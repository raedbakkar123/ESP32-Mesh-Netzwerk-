ESP32 Mesh-Netzwerk zur Temperaturüberwachung in Gebäuden
📌 Projektbeschreibung

Dieses Projekt realisiert ein sicheres und skalierbares Sensornetzwerk zur Erfassung und Visualisierung von Temperaturdaten in Gebäuden.

Mehrere ESP32-basierte Sensorknoten messen Temperaturen und kommunizieren über ein eigenes WiFi-Mesh-Netzwerk miteinander.
Die gesammelten Daten werden über LoRaWAN an einen lokalen Server übertragen, in InfluxDB gespeichert und mit Grafana visualisiert.

Das System ist für den Innenbereich konzipiert und eignet sich u. a. für:

🏢 Gebäudeüberwachung

🌡️ Raumklima-Analyse

⚡ Energieeffizienz-Optimierung

🧭 Systemübersicht
Datenfluss
ESP32-Sensorknoten
→ WiFi-Mesh-Netzwerk
→ ESP32-Station mit LoRa
→ LoRaWAN-Gateway (Raspberry Pi + WM1302)
→ ChirpStack
→ InfluxDB
→ Grafana-Dashboard

🧩 Hardware
Sensorknoten

ESP32 (BERRYBASE ESP32)

DS1621 Temperatursensor (I²C)

Breadboard & Jumper-Kabel

Stationsknoten

ESP32

DS1621 Temperatursensor

LoRa-Bee-Modul (SPI)

Gateway

Raspberry Pi 4 Model B

LR1302 LoRaWAN HAT

WM1302 LoRaWAN Gateway Modul

GPS-Antenne

LoRa-Antenne

microSD-Karte

💻 Software & Technologien
Firmware

Arduino IDE

ESP32 Board Package

painlessMesh (WiFi-Mesh-Netzwerk)

I²C (Temperatursensor)

SPI (LoRa-Modul)

Server & Backend

ChirpStack (LoRaWAN Network Server)

Docker & Docker Compose

Mosquitto (MQTT)

InfluxDB (Zeitreihendatenbank)

Grafana (Datenvisualisierung)

⚙️ Installation
ESP32-Firmware

Arduino IDE installieren

ESP32-Boardpaket hinzufügen

Bibliothek painlessMesh installieren

Firmware auf alle ESP32-Knoten flashen

Gateway & Server

Raspberry Pi OS installieren

Docker & Docker Compose installieren

ChirpStack per Docker starten

SX1302 Packet Forwarder konfigurieren

InfluxDB & Grafana installieren und verbinden

🚀 Funktionen

🌡️ Temperaturmessung in mehreren Räumen

🔗 Selbstheilendes WiFi-Mesh-Netzwerk

📡 Sichere Datenübertragung über LoRaWAN

📊 Echtzeit- und Langzeit-Visualisierung

📈 Skalierbar durch zusätzliche Sensorknoten

📎 Hinweise

Das System ist für lokale Netze konzipiert

Erweiterbar um weitere Sensoren (z. B. Luftfeuchtigkeit, CO₂)

Ideal für Forschungs-, Lehr- und Smart-Building-Projekte