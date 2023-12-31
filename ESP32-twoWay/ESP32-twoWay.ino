#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>


// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x2C, 0xF4, 0x32, 0x49, 0xD0, 0x10};

// Variable to store if sending data was successful
String success;

//Structure example to send data
//Must match the receiver structure
typedef struct struct_message {
    char Aa[20];
    int Bb;
    bool Cc;
} struct_message;

// Create a struct_message called BME280Readings to hold sensor readings
struct_message recvData;
struct_message sendData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 100; 


esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
 
  Serial.print("Bytes received: ");
  Serial.println(len);

  Serial.print("Char: ");
  Serial.println(recvData.Aa);
  Serial.print("Int: ");
  Serial.println(recvData.Bb);
  Serial.print("Bool1: ");
  Serial.println(recvData.Cc);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  esp_now_init();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}
 
void loop() {

 
  // Set values to send
   if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    strcpy(sendData.Aa, "THIS IS A 32");
    sendData.Bb = random(1,20);
    sendData.Cc = false;

    // Send message via ESP-NOW
    esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));

    lastTime = millis();
    
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
   }
}




