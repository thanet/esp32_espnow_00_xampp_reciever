#include <esp_now.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Arduino_JSON.h>
#include <HTTPClient.h>  // Include HTTP client library

// Replace with your network credentials (Wi-Fi station mode)
const char* ssid = "True Enjoy";
const char* password = "enjoy7777777777";

// XAMPP Server Details
const String serverURL = "http://192.168.1.57/espdata_00/upload.php"; // Replace with your server's IP address and script path

// Structure example to receive data
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  unsigned int readingId;
} struct_message;

struct_message incomingReadings;
JSONVar board;

// HTML Page for the Web Server
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<!-- Your HTML content goes here -->
</html>)rawliteral";

// ESP Web Server Class
class ESPWebServer {
  public:
    static AsyncWebServer server;
    static AsyncEventSource events;

    ESPWebServer() {
      server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
      });
      
      events.onConnect([](AsyncEventSourceClient *client){
        if(client->lastId()){
          Serial.printf("Client reconnected! Last message ID: %u\n", client->lastId());
        }
        client->send("hello!", NULL, millis(), 10000);
      });
      server.addHandler(&events);
      server.begin();
    }

    static void sendEvent(const String& message) {
      events.send(message.c_str(), "new_readings", millis());
    }
};

// Initialize static member variables for ESPWebServer
AsyncWebServer ESPWebServer::server(80);
AsyncEventSource ESPWebServer::events("/events");

// ESP-NOW Handler Class
class ESPNowHandler {
  public:
    ESPNowHandler() {
      if (esp_now_init() != ESP_OK) {
        Serial.println("Error initializing ESP-NOW");
        return;
      }
      esp_now_register_recv_cb(OnDataRecv);
    }

    static void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
      char macStr[18];
      Serial.print("Packet received from: ");
      snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
               mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
      Serial.println(macStr);
      memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
      
      board["id"] = incomingReadings.id;
      board["temperature"] = incomingReadings.temp;
      board["humidity"] = incomingReadings.hum;
      board["readingId"] = String(incomingReadings.readingId);
      String jsonString = JSON.stringify(board);
      
      // Now ESPWebServer is fully declared, you can call its static method
      ESPWebServer::sendEvent(jsonString);

      // Print received data to Serial
      Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
      Serial.printf("Temperature: %4.2f \n", incomingReadings.temp);
      Serial.printf("Humidity: %4.2f \n", incomingReadings.hum);
      Serial.printf("Reading ID: %d \n", incomingReadings.readingId);
      Serial.println();

      // Upload data to XAMPP server
      uploadToServer(incomingReadings);
    }

    static void uploadToServer(struct_message data) {
      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(serverURL); 
        http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Set content type to form-urlencoded
        
        // Create data payload to send
        String postData = "id=" + String(data.id) + "&temperature=" + String(data.temp) + "&humidity=" + String(data.hum) + "&readingId=" + String(data.readingId);
        Serial.println("Sending POST request to: " + serverURL);
        Serial.println("POST Data: " + postData);

        int httpResponseCode = http.POST(postData);
        
        if (httpResponseCode > 0) {
          String response = http.getString();
          Serial.println("Server Response: " + response);
        } else {
          Serial.println("Error on sending POST: " + String(httpResponseCode));
          Serial.println("HTTP Error: " + http.errorToString(httpResponseCode));

        }
        http.end(); // Close connection
      } else {
        Serial.println("Not connected to WiFi");
      }
    }
};

// Setup function
void setup() {
  Serial.begin(115200);
  
  // Set device to AP_STA mode
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to Wi-Fi...");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());

  // Initialize ESP-NOW and Web Server
  ESPNowHandler espNowHandler;
  ESPWebServer espWebServer;
}

void loop() {
  static unsigned long lastEventTime = millis();
  static const unsigned long EVENT_INTERVAL_MS = 5000;
  if ((millis() - lastEventTime) > EVENT_INTERVAL_MS) {
    ESPWebServer::sendEvent("ping");
    lastEventTime = millis();
  }
}
