🎯 Funktionsübersicht – Projekt „ESP32 Mesh-Netzwerk zur Temperaturüberwachung“

Das Projekt realisiert ein skalierbares, sicheres Sensornetzwerk zur Temperaturüberwachung in Gebäuden.
Mehrere ESP32-basierte Sensorknoten erfassen Temperaturdaten und kommunizieren über ein WiFi-Mesh-Netzwerk.
Die Daten werden über LoRaWAN an einen lokalen Server übertragen, gespeichert und visualisiert.

🔐 1. Sensorknoten & Mesh-Kommunikation

Temperaturmessung mit DS1621 (I2C)

Kommunikation der ESP32-Knoten über painlessMesh

Selbstheilendes, dynamisches WiFi-Mesh-Netzwerk

Automatische Knoten-Erkennung und Weiterleitung

📡 2. Stationsknoten mit LoRa-Anbindung

Empfang aggregierter Messdaten aus dem Mesh

Weiterleitung der Daten per LoRa (SPI)

Gateway-nahe Datenkonzentration

Reduzierung der LoRa-Sendeintervalle zur Energieeinsparung

🌍 3. LoRaWAN-Gateway

Raspberry Pi mit WM1302 / LR1302

Betrieb als lokales LoRaWAN-Gateway

GPS-Zeit- und Positionssynchronisation

Weiterleitung der Pakete an den Network Server

🗄️ 4. Backend & Datenverarbeitung

ChirpStack als LoRaWAN Network Server

MQTT (Mosquitto) für Nachrichtenverarbeitung

InfluxDB zur Speicherung von Zeitreihendaten

Docker-basierter, reproduzierbarer Serverbetrieb

📊 5. Visualisierung & Monitoring

Grafana-Dashboards für:

Echtzeit-Temperaturwerte

Historische Verläufe

Vergleich mehrerer Räume

Filterung nach Knoten, Zeit und Standort

Erweiterbar für weitere Sensordaten

🧪 6. Test- & Demonstrationsdaten

Unterstützung von Testknoten

Simulation mehrerer Sensoren

Geeignet für Vorführungen und Entwicklung

⚙️ 7. Skalierbarkeit & Erweiterbarkeit

Beliebig viele ESP32-Sensorknoten integrierbar

Erweiterbar um zusätzliche Sensoren (z. B. Luftfeuchte, CO₂)

Modularer Aufbau (Firmware, Gateway, Backend getrennt)

🧩 Hardwareübersicht
Sensorknoten

ESP32 (BERRYBASE ESP32)

DS1621 Temperatursensor (I2C)

Breadboard & Jumper-Kabel

Stationsknoten

ESP32

DS1621 Temperatursensor

LoRa-Bee-Modul (SPI)

Gateway

Raspberry Pi 4 Model B

LR1302 LoRaWAN HAT

WM1302 Gateway Modul

GPS- & LoRa-Antenne

microSD-Karte

💻 Software & Technologien
Firmware

Arduino IDE

ESP32 Board Package

painlessMesh

I2C / SPI

Server & Backend

ChirpStack

Docker & Docker Compose

Mosquitto (MQTT)

InfluxDB

Grafana

🚀 Einsatzgebiete

🏢 Gebäude- und Raumüberwachung

🌡️ Raumklima-Analyse

⚡ Energieeffizienz-Optimierung

🎓 Forschung & Lehre