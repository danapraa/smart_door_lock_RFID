// blynk info
#define BLYNK_TEMPLATE_ID "TMPL69LxtktMi"
#define BLYNK_TEMPLATE_NAME "Door Lock"
#define BLYNK_AUTH_TOKEN "PoMGXXRPRJM-xoNEQilm_dZqMC8U-svX"

// wifi info
char ssid[] = "yyy";
char pass[] = "12345678";

#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

#define SS_PIN 5     // SDA pin connected to GPIO 5
#define RST_PIN 22   // RST pin connected to GPIO 223
#define RELAY_PIN 4  // Pin connected to the relay
#define BUZZ_PIN 15
#define BANYAK_USER 3
int blynkRelay = 0;

MFRC522 rfid(SS_PIN, RST_PIN);  // Create MFRC522 instance


// Define the valid UIDs
byte validUids[BANYAK_USER][4] = {
  { 0xA3, 0xD8, 0x4E, 0xD3 },
  { 0x21, 0xC2, 0xDB, 0x2E },
  { 0xDC, 0xD0, 0xD6, 0x39 }
};

void setup() {
  Serial.begin(115200);  // Initialize serial communications with the PC
  while (!Serial)
    ;  // Wait for serial port to be available

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  for (int i = 0; i < 7; i++) {
    Serial.print(".");
    delay(250);
  }

  normalBuzz();

  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  pinMode(RELAY_PIN, OUTPUT);    // Set relay pin as output
  pinMode(BUZZ_PIN, OUTPUT);     // Set buzzer pin as output
  digitalWrite(BUZZ_PIN, HIGH);  // Ensure relay is off initially
  digitalWrite(RELAY_PIN, LOW);  // Ensure relay is off initially

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
  Blynk.run();

  if (blynkRelay == 1) {
    Serial.println("Blynk open active.");
    normalBuzz();
    digitalWrite(RELAY_PIN, HIGH);  // Turn on relay
    delay(5000);                    // Wait for 5 seconds
    digitalWrite(RELAY_PIN, LOW);   // Turn off relay
    doubleBuzz();
    // set blynk close
    Blynk.virtualWrite(V0, 0);
    blynkRelay = 0;
  }

  // Look for new cards
  if ((rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial())) {
    // Print Card UID
    Serial.print("Card UID:");
    for (byte i = 0; i < rfid.uid.size; i++) {
      Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      Serial.print(rfid.uid.uidByte[i], HEX);
    }
    Serial.println();

    // Check if the card UID is valid
    if (isUidValid(rfid.uid.uidByte)) {
      Serial.println("Valid card detected. Activating relay for 5 seconds.");
      normalBuzz();
      digitalWrite(RELAY_PIN, HIGH);  // Turn on relay
      delay(5000);                    // Wait for 5 seconds
      digitalWrite(RELAY_PIN, LOW);   // Turn off relay
      doubleBuzz();
    } else {
      Serial.println("Invalid card.");
      alertBuzz();
    }

    // Print Card type
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.print("PICC Type: ");
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Halt PICC (this command is optional if you want to keep reading the same card)
    rfid.PICC_HaltA();
    // Stop encryption on PCD
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
  blynkRelay = param.asInt();
}