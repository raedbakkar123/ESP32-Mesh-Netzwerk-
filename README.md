ESP32 Mesh-Netzwerk zur Temperaturüberwachung in Gebäuden
Projektbeschreibung
Dieses Projekt realisiert ein sicheres, skalierbares Sensornetzwerk zur Erfassung und Visualisierung von Temperaturdaten in Gebäuden.
Mehrere ESP32-basierte Sensorknoten messen Temperaturen und kommunizieren über ein eigenes WiFi-Mesh-Netzwerk miteinander. Die gesammelten Daten werden über LoRa an einen lokalen LoRaWAN-Server (ChirpStack) übertragen, in InfluxDB gespeichert und mit Grafana visualisiert.
Das System ist für den Einsatz in Innenräumen konzipiert und eignet sich z. B. für:
•	Gebäudeüberwachung
•	Raumklima-Analyse
•	Energieeffizienz-Optimierung
________________________________________
Systemübersicht
Datenfluss:
ESP32-Sensorknoten
→ WiFi-Mesh-Netzwerk
→ ESP32-Station mit LoRa
→ LoRaWAN-Gateway (Raspberry Pi + WM1302)
→ ChirpStack
→ InfluxDB
→ Grafana-Dashboard
________________________________________
Hardware
Sensorknoten
•	ESP32 (BERRYBASE ESP32)
•	DS1621 Temperatursensor (I²C)
•	Breadboard & Jumper-Kabel
Stationsknoten
•	ESP32
•	DS1621 Temperatursensor
•	LoRa-Bee-Modul (SPI)

Gateway
•	Raspberry Pi 4 Model B
•	LR1302 LoRaWAN HAT
•	WM1302 LoRaWAN Gateway Modul
•	GPS-Antenne & LoRa-Antenne
•	microSD-Karte
________________________________________
Software & Technologien
Firmware
•	Arduino IDE
•	ESP32 Board Package
•	painlessMesh (WiFi-Mesh-Netzwerk)
•	I²C (Temperatursensor)
•	SPI (LoRa-Modul)
Server & Backend
•	ChirpStack (LoRaWAN Network Server)
•	Docker & Docker Compose
•	Mosquitto (MQTT)
•	InfluxDB (Zeitreihendatenbank)
•	Grafana (Visualisierung)











________________________________________
Installation 
ESP32-Firmware
1.	Arduino IDE installieren
2.	ESP32-Boardpaket hinzufügen
3.	Bibliothek painlessMesh installieren
4.	Firmware auf alle ESP32-Knoten flashen
Gateway & Server
1.	Raspberry Pi OS installieren
2.	Docker & Docker Compose installieren
3.	ChirpStack via Docker starten
4.	Packet Forwarder (SX1302) konfigurieren
5.	InfluxDB & Grafana installieren und verbinden
________________________________________
Funktionen
•	🌡️ Temperaturmessung in mehreren Räumen
•	🔗 Selbstheilendes WiFi-Mesh-Netzwerk
•	📡 Sichere Datenübertragung über LoRaWAN
•	📊 Echtzeit- und Historien-Visualisierung
•	📈 Skalierbar durch zusätzliche Sensorknoten





