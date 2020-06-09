#include <UIPEthernet.h>

EthernetServer server = EthernetServer(80);

int pins[] = {2,3,4,5,6,7,8,9};
int values[] = {0,0,0,0,0,0,0,0};

void setup() {
    Serial.begin(9600);

    uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
    IPAddress myIP(192,168,0,6);


    Ethernet.begin(mac,myIP);

    server.begin();

    Serial.print("IP Address: ");
    Serial.println(Ethernet.localIP());

    for (int i = 0; i < 8; i++) {
        pinMode(pins[i], OUTPUT);
    }
}

void response(EthernetClient &client, String &request) {
    //Serial.print(request);

    if (request.indexOf("/status") != -1) {

    } else {
        int length = request.indexOf("HTTP");
        String qs(request.substring(6, length - 1));

        int comp = request.indexOf("=");
        int amp = request.indexOf("&");
        int pin = request.substring(comp + 1, amp).toInt();
        int value = request.substring(amp + 3, length - 1).toInt();

        digitalWrite(pins[pin], value);
        values[pin] = value;
    }

    client.println("HTTP/1.1 200 OK");
    client.print("Server: Arduino on ");
    client.println(Ethernet.localIP());
    client.println("Content-Type: application/json");
    client.println("Connection: close");
    client.println("");

    client.print("{\"states\":[");
    for (int i = 0; i < 8; i++) {
        client.print("{\"id\":");
        client.print(i);
        client.print(", \"value\":");
        client.print(values[i]);
        client.print("}");
        if (i < 7) {
            client.print(",");
        }
    }
    client.println("]}");

}

void loop() {
  // listen for incoming clients
    EthernetClient client = server.available();

    if (client) {
        //Serial.println("-> New Connection");

        // an http request ends with a blank line
        boolean currentLineIsBlank = true;
        String request;

        while (client.connected()) {
            if (client.available()) {

                char c = client.read();

                Serial.println(request.indexOf("Host:"));
                if (request.indexOf("HTTP/1.") == -1) {
                    request += c;
                }

                if (c == '\n' && currentLineIsBlank) {
                    response(client, request);
                    break;
                }

                if (c == '\n') {
                    // you're starting a new line
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    // you've gotten a character on the current line
                    currentLineIsBlank = false;
                }
            }
        }

        // give the web browser time to receive the data
        delay(10);

        // close the connection:
        client.stop();
        //Serial.println("   Disconnected\n");
    }
}
