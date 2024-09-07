/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-esp32-arduino-ide/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#include <Arduino_JSON.h>

#include <HTTPClient.h> 

// Replace with your network credentials (STATION)  enjoy office
  const char* ssid = "True Enjoy";
  const char* password = "enjoy7777777777";
  String URL = "http://192.168.1.57/espdata_00/upload.php";



// Replace with your network credentials (STATION)  บ้านริมโขง
  // const char* ssid = "ENJMesh";
  // const char* password = "enjoy042611749";

// For uplod data to server , xampp
  //String URL = "http://192.168.0.113/EspData/upload.php";

//Phototype Function
void UploadData2Xampp();

float temperature = 0; 
float humidity = 0;

// Structure example to receive data
// Must match the sender structure
typedef struct struct_message {
  int id;
  float temp;
  float hum;
  int readingId;
} struct_message;


// Create a struct_message called myData
  struct_message incomingReadings;
// Create a Json data package
  JSONVar board;




// callback function that will be executed when data is received
// void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
//   char macStr[18];
//   Serial.print("Packet received from: ");
//   snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
//            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
//   Serial.println(macStr);
//   memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
void OnDataRecv(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
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
  //events.send(jsonString.c_str(), "new_readings", millis());
  
  Serial.printf("Board ID %u: %u bytes\n", incomingReadings.id, len);
  Serial.printf("t value: %4.2f \n", incomingReadings.temp);
  Serial.printf("h value: %4.2f \n", incomingReadings.hum);
  Serial.printf("readingID value: %d \n", incomingReadings.readingId);
  Serial.println();


  temperature = incomingReadings.temp;
  humidity = incomingReadings.hum;
  Serial.print("Temperature"); Serial.println(temperature);
  Serial.print("humidity"); Serial.println(humidity);



}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  //WiFi.setSleep(WIFI_PS_NONE);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Setting as a Wi-Fi Station..");
  }
  Serial.print("Station IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi Channel: ");
  Serial.println(WiFi.channel());
  delay(2000);


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  else {
    Serial.println("esp_now_init  and Connected...");
  }
  delay(2000);

  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));
}
 
void loop() {
  Serial.println("Start of Loop Func.");
  
    if (temperature > 0 & humidity > 0){
      
      UploadData2Xampp();
    }
    else {
      Serial.println("Temp = " + int(temperature));
      Serial.println("Hum = " + int(humidity));
    }
  
  Serial.println("End of Loop Func.");
  delay(5000);

}

//Function for upload data to xampp
void UploadData2Xampp() {
  Serial.println("Have Data to up Upload...");

  String postData = "temperature=" + String(temperature) + "&humidity=" + String(humidity); 

  HTTPClient http; 
  http.begin(URL);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  
  int httpCode = http.POST(postData); 
  String payload = http.getString(); 
  
  Serial.print("URL : "); Serial.println(URL); 
  Serial.print("Data: "); Serial.println(postData); 
  Serial.print("httpCode: "); Serial.println(httpCode); 
  Serial.print("payload : "); Serial.println(payload); 
  Serial.println("--------------------------------------------------");
  delay(5000);

}