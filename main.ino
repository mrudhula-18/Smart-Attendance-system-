#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// Pin definitions based on your specified wiring
// Keep these as they are the pins your MFRC522 module's SS and RST are connected to
#define RST_PIN D1 // Your specified RST pin (GPIO15)
#define SS_PIN D8  // Your specified SDA (SS) pin (GPIO16)

// IMPORTANT: Changed BUZZER pin.
// You had BUZZER defined on the same pin as RST (GPIO15).
// This will cause conflicts. Using D2 (GPIO4) as an example.
// Make sure you connect your buzzer to this new pin.
#define BUZZER D2 // Changed BUZZER pin to D2 (GPIO4)

// SPI Pins - We no longer need to define these explicitly for SPI.begin()
// because we are using the default pins.
// #define SPI_SCK_PIN 14  // D5 is GPIO14
// #define SPI_MISO_PIN 12  // D6 is GPIO12
// #define SPI_MOSI_PIN 13  // D7 is GPIO13

// The MFRC522 constructor still needs to know which pins are used for SS and RST
MFRC522 mfrc522(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

int blockNum = 2; // The block you want to read from

// bufferLen should be exactly 18 bytes for MIFARE_Read (16 data + 2 status)
byte bufferLen = 18;
byte readBlockData[18];

String card_holder_name_string; // Renamed to avoid conflict with the function
const String sheet_url = "https://script.google.com/macros/s/AKfycbxC6ErTfZpiKzVisGHfvnR64z4kVGXUGSRKlgeRcDvL2r15bKAfTHRzGMeN6g_4IHYqSg/exec?name="; // Replace with your Google Sheet URL

#define WIFI_SSID "Galaxy A21s7B2C" // Replace with your WiFi SSID
#define WIFI_PASSWORD "dwho1430" // Replace with your WiFi Password

// Default key for MIFARE Classic (often all FFs or all 00s)
// Initialize with a common default key. You might need to try the other if this doesn't work.
void setupKey() {
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF; // Common default key
    // key.keyByte[i] = 0x00; // Another common default key - uncomment and try if FF doesn't work
  }
}

void setup() {
  // Start the Serial Monitor
  Serial.begin(9600);
  // Add delays to allow the Serial Monitor to connect
  delay(1000);
  Serial.println("Serial communication started!");

  // Configure the BUZZER pin
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW); // Ensure buzzer is off initially

  // Connect to WiFi
  Serial.print("Connecting to AP");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500); // Increased delay slightly for connection
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  // Initialize the SPI bus using default pins
  // Removed arguments to use the default SCK, MISO, and MOSI pins
  SPI.begin();
  Serial.println("SPI initialized with default pins.");

  // Initialize the MFRC522 module
  // The MFRC522 library will use the SS_PIN and RST_PIN defined in the constructor
  mfrc522.PCD_Init();
  Serial.println("MFRC522 initialized.");

  // Setup the default key for authentication
  setupKey(); // Call the setupKey function
  Serial.println("Default authentication key setup.");

  Serial.println("\nPlace a MIFARE Classic card on the reader.");
}

void loop() {
  // ... (rest of your loop function remains the same) ...
  // (The code for card detection, reading, buzzer, and HTTP request)
  // ... (Refer to the previous response for the complete loop function) ...

  // Check if a new card is present
  if (!mfrc522.PICC_IsNewCardPresent()) {
    delay(100); // Small delay to prevent flooding the loop
    return; // If no card is detected, keep checking
  }

  // If we have a new card, try to read its serial (UID)
  if (!mfrc522.PICC_ReadCardSerial()) {
    return; // If we cannot read the card serial, try again
  }

  Serial.print("\n");
  Serial.println("**Card Detected**");
  Serial.print(F("Card UID:"));

  // Print the UID of the card
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.print("\n");

  // Show card type
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));
  Serial.print("\n");

  // Proceed to read data from the card
  Serial.println(F("Attempting to read from Data Block..."));
  ReadDataFromBlock(blockNum, readBlockData);

  // Print the data read from block
  Serial.print("\n");
  Serial.print(F("Data read from Block "));
  Serial.print(blockNum);
  Serial.print(F(" --> "));
  // Print only the first 16 bytes as they contain the data
  for (int j = 0; j < 16; j++) {
    Serial.write(readBlockData[j]);
  }
  Serial.print("\n");

  // Trigger the buzzer
int buzzerFrequency1 = 1000; // Example frequency in Hz
  int buzzerFrequency2 = 1500; // Another example frequency

  tone(BUZZER, buzzerFrequency1); // Start the first tone
  delay(200); // Play for 200 milliseconds
  noTone(BUZZER); // Stop the tone
  delay(200); // Pause
  tone(BUZZER, buzzerFrequency2); // Start the second tone
  delay(200); // Play for 200 milliseconds
  noTone(BUZZER); // Stop the tone


  // Send data to Google Sheet via HTTP GET request
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    // WARNING: Using setInsecure() disables certificate validation.
    // Only use this if you understand the security implications and trust the server.
    // For production, you should properly validate the certificate.
    client.setInsecure();

    // Convert the read data (byte array) to a String for the URL
    // Be careful with null termination and character encoding here.
    // String((char*)readBlockData) treats the byte array as a C-style string.
    // The data from the RFID card should ideally be null-terminated if it's a string.
    // Added a check to ensure the first 16 bytes are treated as a string.
    card_holder_name_string = sheet_url + "?data=" + String((char*)readBlockData).substring(0, 16); // Added "?data=" and substring
    card_holder_name_string.trim(); // Remove leading/trailing whitespace
    // URL encode the data if it contains special characters (spaces, etc.)
    // This is important for GET requests.
    card_holder_name_string.replace(" ", "%20"); // Simple example for spaces

    Serial.print("Constructed URL: ");
    Serial.println(card_holder_name_string);

    HTTPClient https;
    Serial.print(F("[HTTPS] begin...\n"));

    // Ensure the URL is valid and the client can connect
    if (https.begin(client, card_holder_name_string)) {
      Serial.print(F("[HTTPS] GET...\n"));
      // start connection and send HTTP header
      int httpCode = https.GET();
      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been sent and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
        // You can read the response payload here if needed
        // String payload = https.getString();
        // Serial.println(payload);
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }
      https.end(); // Close connection
    } else {
      Serial.printf("[HTTPS] Unable to connect to %s\n", card_holder_name_string.c_str());
    }
  } else {
    Serial.println("WiFi not connected. Skipping HTTP request.");
  }

  // Halt PICC (card) to prevent multiple readings of the same card
  mfrc522.PICC_HaltA();
  // Stop encryption if it was started
  mfrc522.PCD_StopCrypto1();

  Serial.println("\n--------------------\n"); // Separator for next card detection
  delay(1000); // Delay before checking for the next card
}


// Function to read data from the card block
void ReadDataFromBlock(int blockNum, byte readBlockData[]) {
  // Authenticate before reading
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Read Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Read Authentication success.");
  }

  // Read the data
  // The bufferLen variable needs to be a pointer when passed to MIFARE_Read
  byte actualBufferLen = bufferLen; // Use a local variable for the pointer
  status = mfrc522.MIFARE_Read(blockNum, readBlockData, &actualBufferLen); // Pass address of local variable
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Reading Block ");
    Serial.print(blockNum);
    Serial.print(" failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.print("Block ");
    Serial.print(blockNum);
    Serial.println(" was read successfully.");
  }
  // Ensure the read data is null-terminated if you intend to treat it as a string.
  // This prevents issues when converting to String for the HTTP request.
  // Assuming the data in the block is meant to be a string.
  if (actualBufferLen > 0 && actualBufferLen <= 16) { // Check bounds
      readBlockData[actualBufferLen] = '\0'; // Null terminate the string
  } else if (actualBufferLen > 16) { // If buffer is larger than 16 (due to status bytes), null terminate at 16
       readBlockData[16] = '\0';
  }
}

// The original code did not have a WriteDataToBlock function,
// but I'll include a placeholder/commented-out version for completeness
/*
void WriteDataToBlock(int blockNum, byte blockData[]) {
  // Authenticate before writing
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, blockNum, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Write Authentication failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.println("Write Authentication success.");
  }

  // Write the data
  status = mfrc522.MIFARE_Write(blockNum, blockData, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Writing to Block ");
    Serial.print(blockNum);
    Serial.print(" failed: ");
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  } else {
    Serial.print("Data was written into Block ");
    Serial.print(blockNum);
    Serial.println(" successfully.");
  }
}
*/