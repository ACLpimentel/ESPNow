// SENDER-RECEIVER

#include <ESP8266WiFi.h>
#include <espnow.h>

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0x8C, 0xAA, 0xB5, 0x8C, 0xC5, 0xB0};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char Aa[20];
  int Bb;
  bool Cc;
} struct_message;

// Create a struct_message called myData
struct_message sendData;

// Create a struct_message to reeave
struct_message recvData;

unsigned long lastTime = 0;  
unsigned long timerDelay = 100;  // send readings timer

// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.print("Last Packet Send Status: ");
  if (sendStatus == 0){
    Serial.println("Delivery success");
  }
  else{
    Serial.println("Delivery fail");
  }
}

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&recvData, incomingData, sizeof(recvData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(recvData.Aa);
  Serial.print("Int: ");
  Serial.println(recvData.Bb);
  Serial.print("Bool: ");
  Serial.println(recvData.Cc);
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(10);

  Serial.println("Setup done");

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  
  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
}
 
void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    strcpy(sendData.Aa, "THIS IS A 8266");
    sendData.Bb = random(1,20);
    sendData.Cc = false;
    

    // Send message via ESP-NOW
    esp_now_send(broadcastAddress, (uint8_t *) &sendData, sizeof(sendData));

    lastTime = millis();
  }
}

