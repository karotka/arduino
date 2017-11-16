#include <ESP8266WiFi.h>

const char* ssid = "sGuest-PK2";
const char* password = "sHost321";

String ip;
String rssi;
String bssid;

char buffer[100];

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(10);

    // Connect to WiFi network
    Serial.print("\nConnect to ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("done.");

    // Start the server
    server.begin();
    Serial.print("Server started at http://");

    ip = WiFi.localIP().toString();
    Serial.print(ip);
    Serial.println("/");
}

void loop() {
    // Check if a client has connected
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    // Wait until the client sends some data
    Serial.println("New client");
    while (!client.available()) {
        delay(1);
    }

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Return the response
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println(""); //  do not forget this one

    rssi = WiFi.RSSI();
    bssid = WiFi.BSSIDstr();
    sprintf(buffer,
            "IP: %s,\n"
            "SSID: %s,\n"
            "RSSI: %s,\n"
            "BSSID: %s,\n", ip.c_str(), ssid, rssi.c_str(), bssid.c_str());
    client.println(buffer);

    delay(1);
    Serial.println("Client disonnected\n");
}
