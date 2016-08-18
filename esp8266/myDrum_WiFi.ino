#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include "TimeLib.h"

const char* ssid = "GVT-CIMD";
const char* password = "cimD1980";

#define SERVER_PORT 35000

unsigned int packetSize;
#define PACKET_MAX_SIZE 48
char packetBuffer[PACKET_MAX_SIZE];
IPAddress ipRemote;


#define localPort 4097
#define NTP_PACKET_SIZE 48
byte NTP_packetBuffer[NTP_PACKET_SIZE];
#define NTP_POL_TIME 3600
#define TIMEZONE -3
IPAddress timeServer(132, 163, 4, 101);

byte sendBuffer[] = {
	0b11100011,          // LI, Version, Mode.
	0x0,                 // Stratum unspecified.
	0x6,                 // Polling interval
	0xEC,                // Clock precision.
	0x0, 0x0, 0x0, 0x0 }; // Reference ...

WiFiUDP UDP;

void printTime() {

	Serial.print("Time: ");

	Serial.print(day());
	Serial.print("/");
	Serial.print(month());
	Serial.print("/");
	Serial.print(year());
	Serial.print("   ");

	Serial.print(hour());
	Serial.print(":");
	Serial.print(minute());
	Serial.print(":");
	Serial.println(second());
	Serial.println("");
}

time_t getNtpTime() {
	//WiFiUDP udp;
	UDP.begin(localPort);
	while (UDP.parsePacket() > 0); // discard any previously received packets
	for (int i = 0; i < 5; i++) { // 5 retries.
		sendNTPpacket(&UDP);
		uint32_t beginWait = millis();
		while (millis() - beginWait < 1000) {
			if (UDP.parsePacket()) {
				Serial.println("");
				Serial.println("Receive NTP Response");
				UDP.read(NTP_packetBuffer, NTP_PACKET_SIZE);
				// Extract seconds portion.
				unsigned long highWord = word(NTP_packetBuffer[40], NTP_packetBuffer[41]);
				unsigned long lowWord = word(NTP_packetBuffer[42], NTP_packetBuffer[43]);
				unsigned long secSince1900 = highWord << 16 | lowWord;
				UDP.flush();
				return secSince1900 - 2208988800UL + TIMEZONE * SECS_PER_HOUR;
			}
			delay(10);
		}
	}
	Serial.println("");
	Serial.println("No NTP Response :-(");
	return 0; // return 0 if unable to get the time
}

void sendNTPpacket(WiFiUDP *u) {
	// Zeroise the buffer.
	memset(NTP_packetBuffer, 0, NTP_PACKET_SIZE);
	memcpy(NTP_packetBuffer, sendBuffer, 16);

	if (u->beginPacket(timeServer, 123)) {
		u->write(NTP_packetBuffer, NTP_PACKET_SIZE);
		u->endPacket();
	}
}

void setup()
{

	Serial.begin(115200);

	Serial.println();
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);

	WiFi.mode(WIFI_STA);
	WiFi.hostname("myDrum");
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	UDP.begin(SERVER_PORT);


	setSyncProvider(getNtpTime);
	setSyncInterval(NTP_POL_TIME);

	printTime();

}

void loop()
{
	String packetString;

	packetSize = UDP.parsePacket();
	if (packetSize) {
		//Serial.print("readPacket: "); Serial.println(packetSize);
		UDP.read(packetBuffer, PACKET_MAX_SIZE);
		//ipRemote = UDP.remoteIP();

		packetString = String(packetBuffer);

		Serial.print(packetString);
	}

	memset(packetBuffer, 0, PACKET_MAX_SIZE);
	packetString = "";
	packetSize = 0;

}
