#include <painlessMesh.h>  // Bibliothek für Mesh-Netzwerke
#include <Wire.h>          // I2C-Kommunikation (für DS1621)

//  Mesh-Netzwerk-Konfiguration
#define MESH_PREFIX     "tempMesh"       // Netzwerkname (SSID)
#define MESH_PASSWORD   "meshPassword"   // Passwort
#define MESH_PORT       5555             // Mesh-Port

Scheduler userScheduler;     // Task-Scheduler für Aufgabenplanung
painlessMesh mesh;           // Mesh-Objekt

//  I2C-Pin-Zuordnung für den ESP32
const int SDA_PIN = 21;
const int SCL_PIN = 22;
const int DS1621_ADDRESS = 0x48; // Standardadresse des DS1621

String nodeName = "Node2";  // Name dieses Knotens (für Identifikation)

//  Maps zum Speichern von Knoten-Informationen
std::map<uint32_t, String> nodeNames;        // Node-ID -> Name
std::map<uint32_t, float> nodeTemperatures;  // Node-ID -> Temperatur

//  Sensorinitialisierung: DS1621 konfigurieren
void setupSensor() {
  Wire.begin(SDA_PIN, SCL_PIN); // I2C starten
  // Konfiguration: kontinuierliche Messung einschalten
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAC); Wire.write(0x00); // Config-Register setzen
  Wire.endTransmission();
  
  // Messung starten
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xEE); // Start Conversion
  Wire.endTransmission();
}

//  Temperaturmessung in °C (Celsius)
float readTemperatureC() {
  Wire.beginTransmission(DS1621_ADDRESS);
  Wire.write(0xAA);              // Temperatur-Register lesen
  Wire.endTransmission(false);  // Repeated Start für saubere Kommunikation

  Wire.requestFrom(DS1621_ADDRESS, 2); // 2 Byte lesen

  int8_t msb = Wire.read();     // Ganzzahliger Teil (signed)
  uint8_t lsb = Wire.read();    // Bit 7 = 0.5°C, andere Bits ignorieren

  float temp = msb;
  if (lsb & 0x80) {             // Prüfen, ob 0.5°C hinzukommt
    temp += 0.5;
  }
  return temp;
}

//  Temperatur über das Mesh senden
void sendTemperature() {
  float temp = readTemperatureC();                   // Temperatur lesen
  String msg = nodeName + ":" + String(temp, 1);     // Nachricht formatieren
  mesh.sendBroadcast(msg);                           // An alle Knoten senden
}

//  Callback bei Empfang einer Nachricht von einem anderen Knoten
void receivedCallback(uint32_t from, String &msg) {
  int sep = msg.indexOf(':');       // Trennzeichen suchen
  if (sep <= 0) return;             // Ungültige Nachricht ignorieren

  String sender = msg.substring(0, sep);           // Name extrahieren
  float temp = msg.substring(sep + 1).toFloat();   // Temperatur extrahieren

  nodeNames[from] = sender;        // Namen im Speicher ablegen
  nodeTemperatures[from] = temp;   // Temperatur im Speicher ablegen

  // Ausgabe über die serielle Konsole
  Serial.printf("Empfangen von %s (%u): %.1f °C\n", sender.c_str(), from, temp);
}

//  Aufgabe zum periodischen Senden der Temperatur alle 5 Sekunden
Task sendTask(5000, TASK_FOREVER, &sendTemperature);

void setup() {
  Serial.begin(115200);      // Serielle Schnittstelle starten
  setupSensor();             // DS1621 konfigurieren

  // Mesh initialisieren und debug-Ausgaben aktivieren
  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION);
  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.onReceive(&receivedCallback);  // Empfangen-Callback setzen

  // Sendeaufgabe aktivieren
  userScheduler.addTask(sendTask);
  sendTask.enable();
}

void loop() {
  mesh.update();   // Mesh-Logik ausführen (Verbindungen, Nachrichten etc.)
}
