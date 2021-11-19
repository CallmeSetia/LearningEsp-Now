#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

uint8_t macAddress_ESP_1[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
uint8_t macAddress_ESP_2[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};


// Variable to store if sending data was successful
String success1;
String success2;


//Structure example to send data
//Must match the receiver structure
typedef struct esp1Data {
    int heater;
    float lm35;
} esp1Data;

typedef struct esp2Data {
    int led;
    int ldr;
} esp2Data;

// Untuk Data Masuk dari node esp1 dan esp2
esp1Data nodeEsp1;
esp2Data nodeEsp2;

// Struct Utama
esp1Data nodeEsp0_1; // Esp 0 <--> Esp1
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

  if (String(macStr) == konvertMacKeString(macAddress_ESP_1) ) { // JIka Pengirim Dari ESP1
    memcpy(&nodeEsp1, incomingData, sizeof(nodeEsp1)); // Data Masuk ke Struct esp1
   
    nodeEsp0_1.heater = nodeEsp1.heater; 
    nodeEsp0_1.lm35 = nodeEsp1.lm35;

    Serial.printf("Heater : %d \n", nodeEsp0_1.heater);
    Serial.printf("LM35 : %f \n",  nodeEsp0_1.lm35);
    Serial.println();
  }

  else if (String(macStr) == konvertMacKeString(macAddress_ESP_2)) { // JIka Pengirim Dari ESP2
    memcpy(&nodeEsp2, incomingData, sizeof(nodeEsp2)); // Data Masuk ke Struct esp1
   
    nodeEsp0_2.ldr = nodeEsp2.ldr;
    nodeEsp0_2.led = nodeEsp2.led;

    Serial.printf("LDR : %d \n", nodeEsp0_2.ldr);
    Serial.printf("LED : %d \n", nodeEsp0_2.led);
    Serial.println();
  }
}



void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

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
  memcpy(peerInfo.peer_addr, macAddress_ESP_1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // register second peer  
  memcpy(peerInfo.peer_addr, macAddress_ESP_2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }


  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(LagiTerimaData);
}
 
void loop() {
  unsigned long waktuSekarang = millis();

  if(waktuSekarang - lastWaktu > 5000) { // Per 5 Detik ESP-0 membaca nilai LM35 (dari ESP-1) dan membaca nilai LDR (dari ESP-2)
    // update Pewaktuan
    lastWaktu = waktuSekarang;   

    /*
      Berdasarkan data LM35 yang dibaca oleh ESP-0, 
      Jika LM35<40.0 maka ESP-0 mengirim data ke ESP-1 supaya Heater1 hidup 100% atau datanya 255, 
      jika LM35>60.0 maka ESP-0 mengirim data ke ESP-1 supaya Heater1 mati 100% atau datanya 0, 
      jika LM35 antara 40 dan 60 maka data untuk Heater1 adalah proporsional antara 255-0, 
      contohnya ketika LM35=50 maka data=127 (lihat gambar di lampiran).
    */
    // float bacaLm35 = nodeEsp0_1.lm35;
    
    if (nodeEsp0_1.lm35 < 40.0) {
      nodeEsp0_1.heater = 100; // Heater1 hidup 100% atau datanya 255
      esp_err_t result = esp_now_send(macAddress_ESP_1, (uint8_t *) &nodeEsp0_1, sizeof(nodeEsp0_1));
      if (result == ESP_OK) {
        Serial.println("Sent OK");
      }
      else {
        Serial.println("Sent Gagal");
      }
    }
    else if (nodeEsp0_1.lm35  > 60.0) {
      nodeEsp0_1.heater = 0; // Heater1 mati 100% atau datanya 0
      esp_err_t result = esp_now_send(macAddress_ESP_1, (uint8_t *) &nodeEsp0_1, sizeof(nodeEsp0_1));
      if (result == ESP_OK) {
        Serial.println("Sent OK");
      }
      else {
        Serial.println("Sent Gagal");
      }
    }
    else {
      map(nodeEsp0_1.lm35, 40, 60, 255, 0); //  jika LM35 antara 40 dan 60 maka data untuk Heater1 adalah proporsional antara 255-0
      esp_err_t result = esp_now_send(macAddress_ESP_1, (uint8_t *) &nodeEsp0_1, sizeof(nodeEsp0_1));
      
      if (result == ESP_OK) {
        Serial.println("Sent OK");
      }
      else {
        Serial.println("Sent Gagal");
      }
    }

    /*
      Berdasarkan data LDR yang dibaca ESP-0, 
      jika LDR<100 maka ESP-0 mengirim data ke ESP-2 supaya LED on, 
      jika LDR>800 maka ESP-0 mengirim data ke ESP-2 supaya LED off. 
      Jadi yang melakukan pembandingan data LDR dengan 100 atau 800 bukan program di ESP-2 tapi di ESP-0, 
      ESP-2 hanya menerima perintah on/off dari ESP-0.
    */ 

    if (nodeEsp0_2.ldr < 100) {
      nodeEsp0_2.led = 1; // ESP-2 supaya LED on
      esp_err_t result = esp_now_send(macAddress_ESP_2, (uint8_t *) &nodeEsp0_2, sizeof(nodeEsp0_2));
      if (result == ESP_OK) {
        Serial.println("Sent OK");
      }
      else {
        Serial.println("Sent Gagal");
      }
    }
    else if (nodeEsp0_2.ldr > 800) {
      nodeEsp0_2.led = 0; // Heater1 mati 100% atau datanya 0
      esp_err_t result = esp_now_send(macAddress_ESP_2, (uint8_t *) &nodeEsp0_2, sizeof(nodeEsp0_2));
      if (result == ESP_OK) {
        Serial.println("Sent OK");
      }
      else {
        Serial.println("Sent Gagal");
      }
    }
  }
  delay(50); // Menghilangkan BUG SOFT RESET
}
