#include "esp_wifi.h"
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <LiquidCrystal.h>


const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// define temporal space to execute sending comands
unsigned long lastTime = 0;  
unsigned long timerDelay = 100; 

// REPLACE WITH THE MAC Address of your receiver 
uint8_t broadcastAddress[] = {0x2C, 0xF4, 0x32, 0x49, 0xD0, 0x10};

// Variable to store if sending data was successful
String success;

//Structure to store send and receiver data
typedef struct struct_message {
    char Aa[20];
    int Bb;
    bool Cc;
} struct_message;

// Create a struct_message to store received data and the data to be send
struct_message recvData;
struct_message sendData;

esp_now_peer_info_t peerInfo;

// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
    Serial.print(" RSSI: ");
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
// define data structure to get RSSI
typedef struct {
  unsigned frame_ctrl: 16;
  unsigned duration_id: 16;
  uint8_t addr1[6]; /* receiver address */
  uint8_t addr2[6]; /* sender address */
  uint8_t addr3[6]; /* filtering address */
  unsigned sequence_ctrl: 16;
  uint8_t addr4[6]; /* optional */
} wifi_ieee80211_mac_hdr_t;

typedef struct {
  wifi_ieee80211_mac_hdr_t hdr;
  uint8_t payload[0]; /* network data ended with 4 bytes csum (CRC32) */
} wifi_ieee80211_packet_t;

int sinal;

void promiscuous_rx_cb(void *buf, wifi_promiscuous_pkt_type_t type) {
  // All espnow traffic uses action frames which are a subtype of the mgmnt frames so filter out everything else.
  if (type != WIFI_PKT_MGMT)
    return;

  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buf;
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;

  int rssi = ppkt->rx_ctrl.rssi;
  sinal = rssi;
}
 
void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
 
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  esp_wifi_set_promiscuous(true);
  esp_wifi_set_promiscuous_rx_cb(&promiscuous_rx_cb);

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


  //##### LCD ######

  lcd.begin(16, 2);
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
      Serial.println("Seent with success");
      lcd.setCursor(0, 0);
      lcd.print("IRSS: ");
      lcd.setCursor(7, 0);
      lcd.print(sinal);
      Serial.println();
      Serial.println();
      Serial.println();
      Serial.println();
    }
    else {
      Serial.println("Error sending the data");
    }
   }
}




