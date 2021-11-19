#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>

#define PIN_HEATER 16
#define PIN_LM35 2

// Setting PWM

const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;


uint8_t macAddress_ESP_0[] = {0x11, 0x12, 0x13, 0x14, 0x15, 0x16};

// Variable to store if sending data was successful
String success1;
String success2;


//Structure example to send data
//Must match the receiver structure
typedef struct esp1Data {
    int heater;
    float lm35;
} esp1Data;

// Untuk Data Masuk dari node esp1 dan esp2
esp1Data nodeEsp0;

// Struct Utama
esp1Data nodeEsp0_1; // Esp 0 <--> Esp1

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
   
    nodeEsp0_1.heater = nodeEsp0.heater; 
    nodeEsp0_1.lm35 = nodeEsp0.lm35;

    Serial.printf("Heater : %d \n", nodeEsp0_1.heater);
    Serial.printf("LM35 : %f \n",  nodeEsp0_1.lm35);
    Serial.println();

    // Kontrol Heater dgn PWM
    ledcWrite(ledChannel, nodeEsp0_1.heater);
  }

  
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  pinMode(PIN_LM35, INPUT);

  // INIT PWM
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(PIN_HEATER, ledChannel);

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
  nodeEsp0_1.lm35 = analogRead(PIN_LM35);

   esp_err_t result = esp_now_send(0, (uint8_t *) &nodeEsp0_1, sizeof(nodeEsp0_1));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }


  delay(50); // Menghilangkan BUG SOFT RESET
}
