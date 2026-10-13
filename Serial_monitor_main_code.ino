#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
#define buzzer 2
#define servoPin 3

// Updated UID for the keychain: C3 48 A6 36
String UID = "C3 48 A6 36"; 
byte lock = 0;
bool welcomePrinted = false;  // Flag to control welcome message display

Servo servo;
LiquidCrystal_I2C lcd(0x27, 16, 2);
MFRC522 rfid(SS_PIN, RST_PIN);

void displayWelcome() {
  lcd.clear();
  Serial.println("LCD: Screen cleared.");
  
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  Serial.println("LCD: [4,0] Welcome!");
  
  lcd.setCursor(1, 1);
  lcd.print("Put your card");
  Serial.println("LCD: [1,1] Put your card");
  
  welcomePrinted = true;
}

void setup() {
  Serial.begin(9600);
  servo.attach(servoPin);
  servo.write(70);
  
  lcd.init();
  lcd.backlight();
  
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(buzzer, OUTPUT);
  
  // Initially, display the welcome message
  displayWelcome();
}

void loop() {
  // If no card is present and the welcome message hasn't been (re)printed, show it.
  if (!rfid.PICC_IsNewCardPresent()) {
    if (!welcomePrinted) {
      displayWelcome();
    }
    delay(100); // small delay to avoid spamming the loop
    return;
  } else {
    // Reset the flag once a card is detected so we can update the welcome message later.
    welcomePrinted = false;
  }
  
  if (!rfid.PICC_ReadCardSerial())
    return;
  
  // Card found: proceed with scanning
  lcd.clear();
  Serial.println("LCD: Screen cleared.");
  
  lcd.setCursor(0, 0);
  lcd.print("Scanning");
  Serial.println("LCD: [0,0] Scanning");
  
  Serial.print("NUID tag is :");
  String ID = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    lcd.print(".");
    
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
    
    Serial.print(" ");
    if (rfid.uid.uidByte[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(rfid.uid.uidByte[i], HEX);
    delay(300);
  }
  Serial.println();
  
  ID.toUpperCase();
  
  // Process recognized card for lock/unlock
  if (ID.substring(1) == UID && lock == 0) {
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    servo.write(50);   // Move servo to the locked position
    delay(100);
    
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    
    lcd.setCursor(0, 0);
    lcd.print("Locked");
    Serial.println("LCD: [0,0] Locked");
    
    delay(1500);
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    lock = 1;
    displayWelcome();
    
  } else if (ID.substring(1) == UID && lock == 1) {
    digitalWrite(buzzer, HIGH);
    delay(300);
    digitalWrite(buzzer, LOW);
    servo.write(110);  // Move servo to the unlocked position
    delay(100);
    
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    
    lcd.setCursor(0, 0);
    lcd.print("Unlocked");
    Serial.println("LCD: [0,0] Unlocked");
    
    delay(1500);
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    lock = 0;
    displayWelcome();
    
  } else {
    // Wrong card detected; give error indication.
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    Serial.println("LCD: [0,0] Wrong card!");
    
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(200);
    
    delay(1500);
    lcd.clear();
    Serial.println("LCD: Screen cleared.");
    displayWelcome();
  }
}
