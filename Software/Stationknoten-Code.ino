/****************************************************
 * Projekt:   ESP32 Mesh Temperaturmessung
 * Datei:     Stationknoten.ino
 * Autor:     Rukaia Al Mezal
 * Datum:     17.12.2025
 *
  ****************************************************/



// ===== Region & Chip hier festlegen (KEIN Extra-Header nötig) =====
#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>

// === Mesh + Sensor ===
#include <painlessMesh.h>
#include <Wire.h>
#include <map>
#include <set>
#include <vector>

// ================== OTAA KEYS ==================
// JoinEUI/AppEUI: 0000000000000000  -> little-endian
static const u1_t PROGMEM JOIN_EUI_LE[8] = {
  0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

// DevEUI: 69A8CC13CA13DCB4 -> little-endian eintragen:
static const u1_t PROGMEM DEV_EUI_LE[8] = {
  0xB4,0xDC,0x13,0xCA,0x13,0xCC,0xA8,0x69
};

// AppKey: C1A9B8382C9D4C863EDC45CFE4445241 -> big-endian (NICHT drehen)
static const u1_t PROGMEM APP_KEY[16] = {
  0xC1,0xA9,0xB8,0x38,0x2C,0x9D,0x4C,0x86,
  0x3E,0xDC,0x45,0xCF,0xE4,0x44,0x52,0x41
};

// LMIC Callbacks
void os_getArtEui(u1_t* buf){ memcpy_P(buf, JOIN_EUI_LE, 8); }
void os_getDevEui(u1_t* buf){ memcpy_P(buf, DEV_EUI_LE,  8); }
void os_getDevKey(u1_t* buf){ memcpy_P(buf, APP_KEY,     16); }

// ================== Payload & Sendeintervall ==================
static uint8_t payload[24];              // 6 Bytes genutzt
const unsigned TX_INTERVAL = 60;         // Sekunden
static osjob_t sendjob;

// ================== Pin-Mapping ==================
const lmic_pinmap lmic_pins = {
  .nss = 5,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = { 26, 33, 32 },
};

// ================== Mesh-Konfiguration ==================
#define MESH_PREFIX     "tempMesh"
#define MESH_PASSWORD   "meshPassword"
#define MESH_PORT       5555

Scheduler userScheduler;
painlessMesh mesh;

// === DS1621 (lokaler Sensor in der Station) ===
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int DS1621_ADDRESS = 0x48;

String nodeName = "Station";  // Name dieses Knotens (Gateway-Station)

// === Datenhaltung ===
std::map<uint32_t, float>   latestTemp;    // NodeID -> letzte Temp
std::map<uint32_t, String>  latestName;    // NodeID -> letzter Name
std::vector<uint32_t>       nodeOrder;     // bekannte Nodes (Round-Robin)
size_t rrIndex = 0;

// >>> NEU: FPort-Zuordnung pro Node <<<
std::map<uint32_t, uint8_t> nodeToPort;    // NodeID -> FPort (1..223)
std::set<uint8_t>           usedPorts;

uint8_t reservePort(uint32_t id, uint8_t desired = 0){
  if (nodeToPort.count(id)) return nodeToPort[id];

  // Station bevorzugt Port 1
  if (desired >= 1 && desired <= 223 && !usedPorts.count(desired)) {
    nodeToPort[id] = desired; usedPorts.insert(desired); return desired;
  }
  // freie Ports ab 2 (1 bleibt i.d.R. Station)
  for (uint8_t p = 2; p <= 223; ++p){
    if (!usedPorts.count(p)){
      nodeToPort[id] = p; usedPorts.insert(p); return p;
    }
  }
  // Fallback (sollte nie passieren)
  nodeToPort[id] = 1; usedPorts.insert(1); return 1;
}

// === Sensorinitialisierung ===
void setupSensor() {
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAC); Wire.write(0x00);  // continuous conversion
  Wire.endTransmission();
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xEE);                    // start conversion
  Wire.endTransmission();
}

float readTemperatureC() {
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAA);
  Wire.endTransmission(false);
  Wire.requestFrom(DS1621_ADDRESS, 2);
  int8_t msb = Wire.read();
  uint8_t lsb = Wire.read();
  float t = msb;
  if (lsb & 0x80) t += 0.5;
  return t;
}

// === Helper: neuen/aktualisierten Node speichern + Port vergeben ===
void updateLatest(uint32_t id, float t, const String& name){
  bool isNew = (latestTemp.find(id) == latestTemp.end());
  latestTemp[id] = t;
  latestName[id] = name;
  if (isNew) {
    nodeOrder.push_back(id);
    // Port zuweisen: Station behält 1, andere bekommen freien Port
    uint8_t desired = (id == mesh.getNodeId()) ? 1 : 0;
    uint8_t p = reservePort(id, desired);
    Serial.printf("[PORT] Node %u -> FPort %u\n", id, p);
  }
}

// === Empfang aus Mesh ===
bool parseMeshMessage(const String& msg, String& nameOut, uint32_t& idOut, float& tempOut) {
  int c1 = msg.indexOf(',');
  int c2 = msg.indexOf(',', c1 + 1);
  if (c1 > 0 && c2 > c1) {
    nameOut = msg.substring(0, c1);
    String idStr = msg.substring(c1 + 1, c2);
    String tStr  = msg.substring(c2 + 1);
    idOut = (uint32_t) strtoul(idStr.c_str(), nullptr, 10);
    tempOut = tStr.toFloat();
    return true;
  }
  int s = msg.indexOf(':');
  if (s > 0) {
    nameOut = msg.substring(0, s);
    tempOut = msg.substring(s + 1).toFloat();
    idOut = 0;
    return true;
  }
  return false;
}

void receivedCallback(uint32_t from, String &msg) {
  String sender; uint32_t id; float temp;
  if (!parseMeshMessage(msg, sender, id, temp)) return;
  if (id == 0) id = from;
  updateLatest(id, temp, sender);

  Serial.printf("[MESH] Empfangen von %s (%u): %.1f °C -> gespeichert\n",
                sender.c_str(), id, temp);
}

// === Station sendet eigene Temperatur ins Mesh & speichert sie ===
void sendLocalTempMesh() {
  float t = readTemperatureC();
  String msg = nodeName + "," + String(mesh.getNodeId()) + "," + String(t, 1);
  mesh.sendBroadcast(msg);
  updateLatest(mesh.getNodeId(), t, nodeName);
  Serial.printf("[MESH] Station Local Temp: %.1f °C (NodeID %u)\n", t, mesh.getNodeId());
}
Task sendTask(5000, TASK_FOREVER, &sendLocalTempMesh);

// === Round-Robin Auswahl für LoRa-Sendung ===
static bool havePayloadToSend(uint8_t* out, uint8_t& len, uint32_t& srcId, float& srcTemp, uint8_t& fport) {
  if (nodeOrder.empty()) return false;
  size_t tries = nodeOrder.size();
  while (tries--) {
    if (rrIndex >= nodeOrder.size()) rrIndex = 0;
    uint32_t id = nodeOrder[rrIndex++];
    auto it = latestTemp.find(id);
    if (it == latestTemp.end()) continue;

    srcId = id;
    srcTemp = it->second;

    // FPort pro Node
    fport = nodeToPort.count(id) ? nodeToPort[id] : reservePort(id);

    // 6-Byte Binär: [nodeId LE (4)] [temp_x10 int16 LE (2)]
    int16_t t10 = (int16_t)roundf(srcTemp * 10.0f);
    out[0] = (uint8_t)(srcId & 0xFF);
    out[1] = (uint8_t)((srcId >> 8) & 0xFF);
    out[2] = (uint8_t)((srcId >> 16) & 0xFF);
    out[3] = (uint8_t)((srcId >> 24) & 0xFF);
    out[4] = (uint8_t)(t10 & 0xFF);
    out[5] = (uint8_t)((t10 >> 8) & 0xFF);
    len = 6;
    return true;
  }
  return false;
}

// === LoRa Sendejob ===
static void do_send(osjob_t* j){
  if (LMIC.opmode & OP_TXRXPEND){
    Serial.println(F("OP_TXRXPEND, not sending"));
  } else {
    uint8_t len=0; uint32_t srcId=0; float srcTemp=0; uint8_t fport=1;
    if (havePayloadToSend(payload, len, srcId, srcTemp, fport)) {
      // <<<< entscheidend: FPort je Node
      LMIC_setTxData2(fport, payload, len, 0); // unconfirmed
      Serial.printf("[LORA] Queued: NodeID %u, Temp %.1f °C, %u bytes, FPort %u\n",
                    srcId, srcTemp, len, fport);
    } else {
      Serial.println(F("[LORA] Keine neuen Daten – Slot übersprungen"));
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
    }
  }
}

// === LoRa Events ===
void onEvent(ev_t ev){
  Serial.print(os_getTime()); Serial.print(": ");
  switch(ev){
    case EV_JOINING:   Serial.println(F("EV_JOINING")); break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      LMIC_setLinkCheckMode(0);
      LMIC_setDrTxpow(DR_SF7, 14);
      LMIC_setAdrMode(1);
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE"));
      if (LMIC.txrxFlags & TXRX_ACK) Serial.println(F("Received ack"));
      if (LMIC.dataLen){
        Serial.print(F("Received ")); Serial.print(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
      break;
    default:
      Serial.print(F("Event: ")); Serial.println((unsigned)ev);
      break;
  }
}

// === Setup ===
void setup(){
  Serial.begin(115200);
  delay(2000);
  Serial.println(F("Starting (EU868 / OTAA) + Mesh-Gateway"));

  // Mesh init
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  userScheduler.addTask(sendTask);
  sendTask.enable();

  // Station-Port direkt reservieren (FPort 1)
  reservePort(mesh.getNodeId(), 1);

  // Sensor init
  setupSensor();
  Serial.printf("Station NodeID: %u, Name: %s (FPort %u)\n",
                mesh.getNodeId(), nodeName.c_str(), nodeToPort[mesh.getNodeId()]);

  // LoRa init
  os_init();
  LMIC_reset();

  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);
  LMIC_setDrTxpow(DR_SF12, 14);
  LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100);

  do_send(&sendjob);
}

// === Loop ===
void loop(){
  os_runloop_once();  // LMIC
  mesh.update();      // Mesh
}
