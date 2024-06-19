// blynk info
#define BLYNK_TEMPLATE_ID "TMPL69LxtktMi"  // ID template Blynk yang digunakan
#define BLYNK_TEMPLATE_NAME "Door Lock"   // Nama template Blynk untuk pengunci pintu
#define BLYNK_AUTH_TOKEN "PoMGXXRPRJM-xoNEQilm_dZqMC8U-svX"  // Token otentikasi Blynk yang diperlukan

// wifi info
char ssid[] = "yyy";        // Nama jaringan WiFi
char pass[] = "12345678";   // Kata sandi jaringan WiFi

#include <SPI.h>            // Library SPI untuk komunikasi dengan modul perangkat keras
#include <MFRC522.h>        // Library untuk modul RFID
#include <WiFi.h>           // Library untuk manajemen koneksi WiFi
#include <BlynkSimpleEsp32.h>  // Library Blynk untuk ESP32

#define SS_PIN 5            // Pin SDA terhubung ke GPIO 5
#define RST_PIN 22          // Pin RST terhubung ke GPIO 22
#define RELAY_PIN 4         // Pin terhubung ke relay
#define BUZZ_PIN 15         // Pin terhubung ke buzzer
#define BANYAK_USER 3       // Jumlah maksimal kartu yang diizinkan
int blynkRelay = 0;         // Variabel untuk mengontrol status relay melalui Blynk

MFRC522 rfid(SS_PIN, RST_PIN);  // Membuat instance MFRC522 untuk modul RFID

// Define the valid UIDs
byte validUids[BANYAK_USER][4] = {
  { 0xA3, 0xD8, 0x4E, 0xD3 },   // UID kartu RFID yang diizinkan (contoh)
  { 0x21, 0xC2, 0xDB, 0x2E },
  { 0xDC, 0xD0, 0xD6, 0x39 }
};

void setup() {
  Serial.begin(115200);  // Inisialisasi komunikasi serial dengan PC
  while (!Serial)
    ;  // Tunggu port serial tersedia

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);  // Koneksi ke Blynk dengan token otentikasi dan informasi WiFi
  for (int i = 0; i < 7; i++) {
    Serial.print(".");
    delay(250);
  }

  normalBuzz();  // Buzzer memberikan umpan balik normal

  SPI.begin();      // Inisialisasi bus SPI
  rfid.PCD_Init();  // Inisialisasi modul MFRC522

  pinMode(RELAY_PIN, OUTPUT);    // Set pin relay sebagai output
  pinMode(BUZZ_PIN, OUTPUT);     // Set pin buzzer sebagai output
  digitalWrite(BUZZ_PIN, HIGH);  // Pastikan buzzer mati saat awal
  digitalWrite(RELAY_PIN, LOW);  // Pastikan relay mati saat awal

  Serial.println("RFID reader initialized.");
  Serial.print("MFRC522 Software Version: ");
  Serial.println(rfid.PCD_ReadRegister(MFRC522::VersionReg), HEX);
}

bool isUidValid(byte *uid) {
  for (int i = 0; i < BANYAK_USER; i++) {
    if (memcmp(uid, validUids[i], 4) == 0) {
      return true;
    }
  }
  return false;
}

void loop() {
  Blynk.run();  // Jalankan Blynk

  if (blynkRelay == 1) {
    Serial.println("Blynk open active.");
    normalBuzz();   // Buzzer memberikan umpan balik normal
    digitalWrite(RELAY_PIN, HIGH);  // Aktifkan relay
    delay(5000);    // Tunggu 5 detik
    digitalWrite(RELAY_PIN, LOW);   // Matikan relay
    doubleBuzz();   // Buzzer memberikan umpan balik double buzz
    // set blynk close
    Blynk.virtualWrite(V0, 0);  // Reset kontrol relay di Blynk
    blynkRelay = 0;
  }

  // Cari kartu baru
  if ((rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())) {
    // Cetak UID kartu
    Serial.print("Card UID:");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Periksa apakah UID kartu valid
    if (isUidValid(rfid.uid.uidByte)) {
      Serial.println("Valid card detected. Activating relay for 5 seconds.");
      normalBuzz();   // Buzzer memberikan umpan balik normal
      digitalWrite(RELAY_PIN, HIGH);  // Aktifkan relay
      delay(5000);    // Tunggu 5 detik
      digitalWrite(RELAY_PIN, LOW);   // Matikan relay
      doubleBuzz();   // Buzzer memberikan umpan balik double buzz
    } else {
      Serial.println("Invalid card.");
      alertBuzz();    // Buzzer memberikan umpan balik alert buzz
    }

    // Cetak jenis kartu
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.print("PICC Type: ");
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Hentikan PICC (opsional jika ingin terus membaca kartu yang sama)
    rfid.PICC_HaltA();
    // Stop enkripsi pada PCD
    rfid.PCD_StopCrypto1();
  }
}

void doubleBuzz() {
  digitalWrite(BUZZ_PIN, LOW);
  delay(100);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZ_PIN, LOW);
  delay(100);
  digitalWrite(BUZZ_PIN, HIGH);
}

void normalBuzz() {
  digitalWrite(BUZZ_PIN, LOW);
  delay(100);
  digitalWrite(BUZZ_PIN, HIGH);
}

void alertBuzz() {
  digitalWrite(BUZZ_PIN, LOW);
  delay(50);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZ_PIN, LOW);
  delay(50);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZ_PIN, LOW);
  delay(50);
  digitalWrite(BUZZ_PIN, HIGH);
  delay(100);
}

BLYNK_WRITE(V0) {
  blynkRelay = param.asInt();  // Fungsi Blynk untuk mengontrol relay
}
