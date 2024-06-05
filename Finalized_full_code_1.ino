#include <Wire.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <TinyGPS++.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPU6050.h>
Adafruit_MPU6050 mpu;
#define RELAY_PIN 12
#define BUZZER_PIN 8
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4
SoftwareSerial mySerial(15, 14);
const byte ROW_PINS[KEYPAD_ROWS] = {37, 35, 33, 31};
const byte COL_PINS[KEYPAD_COLS] = {45, 43, 41, 39};
int z=0;


const int FINGERPRINT_RX_PIN = 17;
const int FINGERPRINT_TX_PIN = 18;
const int triggerPin = 9;   // Trigger pin of HC-SR04 connected to Arduino Mega digital pin 2
const int echoPin = 10;      // Echo pin of HC-SR04 connected to Arduino Mega digital pin 3
const int distanceThreshold = 27;  // Threshold distance in centimeters
float lat, lon;
TinyGPSPlus gps;
bool isGPSDataReceived = false;
String googleMapsLink;
Adafruit_Fingerprint fingerprint = Adafruit_Fingerprint(&Serial1);

const byte ROWS = 4;
const byte COLS = 4;
char keymap[ROWS][COLS] = {
  {'D', '#', '0', '*'},
  {'C', '9', '8', '7'},
  {'B', '6', '5', '4'},
  {'A', '3', '2', '1'}
};
Keypad keypad = Keypad(makeKeymap(keymap), ROW_PINS, COL_PINS, KEYPAD_ROWS, KEYPAD_COLS);

LiquidCrystal_I2C lcd(0x27, 20, 4);

String enterPassword = "";
char keyy;

String password = "9999"; // Change this to your desired password
bool motionDetected = false;
long duration;
int distance;
uint8_t id;
bool avai;

void setup() {

   if (!mpu.begin()) {
    while (1) {
      delay(10);
    }
  }
pinMode(RELAY_PIN, OUTPUT);
digitalWrite(RELAY_PIN, LOW);
digitalWrite(LED_BUILTIN, LOW);
pinMode(BUZZER_PIN, OUTPUT);
pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.begin(9600);
  Serial1.begin(57600); // For fingerprint sensor
  Serial2.begin(9600); // For GPS6MV2 sensor
  mySerial.begin(9600);
  mpu.setHighPassFilter(MPU6050_HIGHPASS_0_63_HZ);
  mpu.setInterruptPinLatch(true);    // Keep it latched. Will turn off when reinitialized.
  mpu.setInterruptPinPolarity(true);
  mpu.setMotionInterrupt(false);
 

  lcd.init();
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(6, 1);
  lcd.print("WELCOME");
pinMode(2, INPUT_PULLUP); // Set interrupt pin as input with internal pull-up resistor
  attachInterrupt(digitalPinToInterrupt(2), handleInterrupt, FALLING); 
  delay(3000);
  displayMenu();
}

void enroll(){
if(verifyPassword()){
    lcd.clear();
  lcd.print("Ready to enroll");
  delay(2000);
  lcd.clear();
  lcd.print("Type the ID (1 to 9): ");
  char z;
  String kk;
  while (kk.length() < 1) {
    z = keypad.getKey();
    if (z) {
      kk += z;
      lcd.print(z);
    }
  }
  delay(4000);
  id = z-'0';
  if (id == 0) {// ID #0 not allowed, try again!
     lcd.print("ID #0 not allowed");
     delay(1000);
     return;
  }
  lcd.print("Enrolling ID #");
  lcd.print(id);

  while (!  getFingerprintEnroll() );
}

}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = fingerprint.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.print("Image taken");
      delay(1000);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.print("Communication error");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      delay(1000);
      break;
    default:
      Serial.println("Unknown error");
      delay(1000);
      break;
    }
  }

  // OK success!

  p = fingerprint.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.print("Image converted");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEMESS:
      lcd.print("Image too messy");
      delay(1000);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      lcd.print("Communication error");
      delay(1000);
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      delay(1000);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = fingerprint.getImage();
  }
  lcd.print("ID "); lcd.println(id);
  p = -1;
  lcd.clear();
  lcd.print("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = fingerprint.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.print("Image taken");
      delay(1000);
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = fingerprint.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      delay(1000);
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  lcd.clear();
  lcd.print("Creating model for #");  lcd.print(id);

  p = fingerprint.createModel();
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Prints matched!");
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.clear();
    lcd.print("Communication error");
    delay(1000);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    lcd.clear();
    lcd.print("Fingerprints did not match");
    delay(1000);
    return p;
  } else {
    lcd.clear();
    lcd.print("Unknown error");
    return p;
  }

  lcd.print("ID "); lcd.print(id);
  p = fingerprint.storeModel(id);
  if (p == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Stored!");
    delay(1000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.clear();
    lcd.print("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    lcd.clear();
    lcd.print("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    lcd.clear();
    lcd.print("Error writing to flash");
    return p;
  } else {
    lcd.clear();
    lcd.print("Unknown error");
    return p;
  }

  return true;
}

void displayMenu() {
  lcd.clear();
  lcd.print("A: Unlock");
  lcd.setCursor(0, 1);
  lcd.print("B: Safe Mode");
  lcd.setCursor(0, 2);
  lcd.print("C: Object Detection");
  lcd.setCursor(0, 3);
  lcd.print("D: Settings");
}

void secondMenu(){
  lcd.clear();
  lcd.print("1.Change Password");
  lcd.setCursor(0, 1);
  lcd.print("2.Add Fingerprint");
  lcd.setCursor(0, 2);
  lcd.print("#: Main Menu");
  while(true){
  char keyz = keypad.getKey();
  if (keyz) {
    switch (keyz) {
      case '1':
        newpass();
        break;
      case '2':
        enroll();
        secondMenu();
        break;
      case '#':
        break;          
    }
  break;}
  }
}

void newpass(){
  if (verifyPassword() && verifyFingerprint()) {
    lcd.clear();
    lcd.print("Enter new password:");

    while (enterPassword.length() < 4) {
      keyy = keypad.getKey();
      if (keyy) {
      enterPassword += keyy;
      lcd.print('*');
      }
    }
    password=enterPassword;
    lcd.clear();
    lcd.print("Password changed");
    delay(1000);
  }
}

bool verifyPassword() {
  lcd.clear();
  lcd.print("Enter password:");

  String enteredPassword = "";
  char key;

  while (enteredPassword.length() < 4) {
    key = keypad.getKey();
    if (key) {
      enteredPassword += key;
      lcd.print('*');
    }
  }

  if (enteredPassword == password) {
    lcd.clear();
    lcd.print("Password correct");
    delay(1000);
    return true;
  } else {
    lcd.clear();
    lcd.print("Password incorrect");
    delay(1000);
    return false;
  }
}

bool verifyPassworded() {
  lcd.clear();
  lcd.print("Enter password:");

  String enteredPassword = "";
  char key;

  while (enteredPassword.length() < 4) {
    key = keypad.getKey();
    if (key) {
      enteredPassword += key;
      lcd.print('*');
    }
  }

  if (enteredPassword == password) {
    lcd.clear();
    lcd.print("Password correct");
    delay(1000);
    return true;
  } else {
    lcd.clear();
    lcd.print("Password incorrect");
    delay(1000);
    return false;
    
  }
}

bool verifyFingerprint() {
  lcd.clear();
  lcd.print("Place finger");

  while (fingerprint.getImage() != FINGERPRINT_OK);

  int fingerprintID = fingerprint.image2Tz(1); // Convert the image to a template
  if (fingerprintID == FINGERPRINT_OK) {
    int result = fingerprint.fingerFastSearch(); // Search the database for a match
    if (result == FINGERPRINT_OK) {
      lcd.clear();
      lcd.print("Fingerprint matched");
      delay(1000);
      return true;
    } else {
      lcd.clear();
      lcd.print("Fingerprint not matched");
      delay(1000);
      return false;
    }
  } else {
    lcd.clear();
    lcd.print("Failed to process fingerprint");
    delay(1000);
    return false;
  }
}


void unlockDoor() {
  //digitalWrite(RELAY_PIN, LOW);
  lcd.clear();
  lcd.print("Door unlocked");
  delay(2000);
  digitalWrite(RELAY_PIN, HIGH);
  delay(5000);
  digitalWrite(RELAY_PIN, LOW);
}

void handleUnlock() {
  if (verifyPassword() && verifyFingerprint()) {
    unlockDoor();
    displayMenu();
  }
}

void safeMode() {
  mpu.setMotionDetectionThreshold(3);
  mpu.setMotionDetectionDuration(100);
  lcd.clear();
  lcd.print("Safe Mode");
  lcd.setCursor(0, 3);
  lcd.print("Press B to exit");


  while (true) {
    if (mpu.getMotionInterruptStatus() && !motionDetected) {
      motionDetected = true;
      digitalWrite(LED_BUILTIN, HIGH);
      lcd.setCursor(0, 1);
      lcd.print("Motion Detected!");
digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
       digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
      digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
       digitalWrite(BUZZER_PIN, HIGH);
    delay(500);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    //Serial.println("Motion Detected but GPS not sent");
  while (z==0){
    getGPSLocation();
    break;

    //delay(10000);
    //z=1;
  }
      isGPSDataReceived = false;
      motionDetected=false;
      z=0;
    }// Check motion detection
    // Replace this part with your motion detection code using GY-521 sensor

    if (keypad.getKey() == 'B') {
      mpu.setMotionInterrupt(true);
      if (verifyPassword()) {
        displayMenu();
        mpu.setMotionInterrupt(false);
        break; // Exit motion detection
        
      }else{
      mpu.setMotionInterrupt(false);
      return safeMode();
    }
    }
  }
}

void handleInterrupt() {
  delay(1000);
  Serial.print("interrupt"); // Set the interrupt flag
  mpu.setMotionInterrupt(false);
 while (!isGPSDataReceived) {
    getGPSLocation();
  } 
  exit(0);
}
void getGPSLocation() {
  //Serial.println("Inside GPS Function");
  do {
    //Serial.println("Inside Do While Function");
    if(Serial2.available()>0 && z==0){
      //Serial.println("Inside Do While IF1 Function");
       gps.encode(Serial2.read());
         //Serial.println("Inside Do While IF2 Function");
       if (gps.location.isValid()){
         //Serial.println("Inside Do While IF3 Function");
      Serial.print("Latitude= "); 
    Serial.print(gps.location.lat(), 6);
    lat=gps.location.lat();
    Serial.print(" Longitude= "); 
    lon=gps.location.lng();
    Serial.println(gps.location.lng(), 6);
    googleMapsLink = "https://www.google.com/maps?q=";
        googleMapsLink += String(lat, 6);
        googleMapsLink += ",";
        googleMapsLink += String(lon, 6);
        
        Serial.print("Google Maps Link: ");
        Serial.println(googleMapsLink);
        
        // Send SMS with the Google Maps link
        
        sendSMS(googleMapsLink);
        changeit();   
    } 
      //z=3;
    
    
    

    
    }
     
    }while (Serial2.available() && z==1);
    //z=2;
    }

void changeit(){

  isGPSDataReceived = true;
  motionDetected=false; 
}
      
    
        
  



/*void getGPSLocation() {
  if (Serial2.available()) { 
    if (gps.encode(Serial2.read())) {
      gps.f_get_position(&lat, &lon);
      
      if (!isGPSDataReceived && motionDetected) {
        Serial.print("Position: ");
        Serial.print("Latitude: ");
        Serial.print(lat, 6);
        Serial.print(",");
        Serial.print("Longitude: ");
        Serial.println(lon, 6);
        
        // Construct the Google Maps link
        String googleMapsLink = "https://www.google.com/maps?q=";
        googleMapsLink += String(lat, 6);
        googleMapsLink += ",";
        googleMapsLink += String(lon, 6);
        
        Serial.print("Google Maps Link: ");
        Serial.println(googleMapsLink);
        
        // Send SMS with the Google Maps link
        sendSMS(googleMapsLink);
        
        isGPSDataReceived = true;
        motionDetected=false; // Set the flag to indicate location has been displayed and SMS sent
      }
    }
  }
}*/
/*else{
        isGPSDataReceived = true;
        motionDetected=false;
        return safeMode();
      }*/

void sendSMS(const String& message) {
  z=2;
  mySerial.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode
  delay(1000);  // Delay of 1000 milli seconds or 1 second
  mySerial.println("AT+CMGS=\"+94775747397\"\r"); // Replace x with mobile number
  delay(1000);
  mySerial.println(message);// The SMS text you want to send
  delay(100);
   mySerial.println((char)26);// ASCII code of CTRL+Z
  delay(1000);
}

void checkObjectAvailability() {
  // Trigger the ultrasonic sensor
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);

  // Measure the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance in centimeters
  distance = duration * 0.034 / 2;

  // Check if the object is within the threshold distance
  if (distance <= distanceThreshold) {
    lcd.setCursor(0, 2);
    lcd.print("Object is available.");
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
       digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
  } else {
    lcd.setCursor(0, 2);
    lcd.print("Object is not ");
    lcd.setCursor(0, 3);
    lcd.print("available.");
     digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
       digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(300);
    digitalWrite(BUZZER_PIN, LOW);
    delay(200);

  }

}

void objectDetection() {
if (verifyPassword()){
lcd.clear();
  lcd.print("Object Detection");
delay(1000); 
checkObjectAvailability();
}
  
}

void loop() {
  char key = keypad.getKey();
  if (key) {
    switch (key) {
      case 'A':
        handleUnlock();
        displayMenu();
        break;
      case 'B':
        safeMode();
        displayMenu();
        break;
      case 'C':
        objectDetection();
        displayMenu();
        break;
      case 'D':
        secondMenu();
        displayMenu();
        break;           
    }
  }
}
