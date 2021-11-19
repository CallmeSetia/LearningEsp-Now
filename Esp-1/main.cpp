
#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define PIN_LED 18
#define PIN_LDR 2

uint8_t macAddress_ESP_0[] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};


// Variable to store if sending data was successful
String success1;
String success2;


typedef struct esp2Data {
    int led;
    int ldr;
} esp2Data;

// Untuk Data Masuk dari node esp0 
esp2Data nodeEsp0;

// Struct Utama
esp2Data nodeEsp0_2; // Esp 0 <--> Esp2

// Untuk Pewaktuan
long lastWaktu = 0;        // last waktu


String konvertMacKeString(const uint8_t * mac_addr) {
  /*
    Fungsi u/ Konversi Mac Addes (uint8_t) dengan Hexadesimal to String
  */

  char macStrBaru[18]; // 18 Bit Alamat
  // Konversi uint8_t ke char
  snprintf(macStrBaru, sizeof(macStrBaru), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]); 
  
  return String(macStrBaru); // char ke Arduino String
}

void LagiKirimData(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "[info] kirim OKE" : "[info] kirim GAGAL");
}


void LagiTerimaData(const uint8_t * mac_addr, const uint8_t *incomingData, int len) {
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);

  if (String(macStr) == konvertMacKeString(macAddress_ESP_0) ) { // JIka Pengirim Dari ESP1
    memcpy(&nodeEsp0, incomingData, sizeof(nodeEsp0)); // Data Masuk ke Struct esp1
   
    nodeEsp0_2.ldr = nodeEsp0.ldr;
    nodeEsp0_2.led = nodeEsp0.led;

    Serial.printf("LDR : %d \n", nodeEsp0_2.ldr);
    Serial.printf("LED : %d \n", nodeEsp0_2.led);
    Serial.println();

    // HIDUP MATI LED
    if (nodeEsp0_2.led > 0) {
      digitalWrite(PIN_LED, HIGH);
    } 
    else if (nodeEsp0_2.led < 1) {
      digitalWrite(PIN_LED, LOW);
    }

  }

}



void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_LDR, INPUT);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(LagiKirimData);
   
  // register peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  // register first peer  
  memcpy(peerInfo.peer_addr, macAddress_ESP_0, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(LagiTerimaData);
}
 
void loop() {
  nodeEsp0_2.ldr = analogRead(PIN_LDR);

   esp_err_t result = esp_now_send(0, (uint8_t *) &nodeEsp0_2, sizeof(nodeEsp0_2));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }


  delay(50); // Menghilangkan BUG SOFT RESET
}
