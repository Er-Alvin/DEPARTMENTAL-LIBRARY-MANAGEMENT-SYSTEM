#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
#include <avr/wdt.h>
#include <SoftwareSerial.h>
SoftwareSerial mySerial2(8, 9);   // RX, TX for the second device
#define BORROW_BTN_PIN 2
#define RETURN_BTN_PIN 3
#define CONTINUE_BTN_PIN 4
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Adjust the I2C address if needed
RTC_DS3231 rtc;
File file;
const char* filename1 = "books.csv";
const char *filename2 = "students.csv";
const char *filename3 = "borrow.csv";
const char *filename4 = "records.csv";
const char *filename5 = "late_fine.csv";

const int chipSelect = 10;  // Assuming your SD card is connected to pin 9
String camData;
void setup() {
  Serial.begin(9600);       // Initialize hardware serial
  mySerial2.begin(115200);    // Initialize second software serial
  pinMode(BORROW_BTN_PIN, INPUT);
  pinMode(RETURN_BTN_PIN, INPUT);
  pinMode(CONTINUE_BTN_PIN, INPUT);
  pinMode(2, INPUT); // Set pin 2 as input for EM-18 RFID reader module
  lcd.begin(16, 2);
  lcd.init();
  lcd.clear();
  lcd.backlight();      // Make sure backlight is on
  lcd.print("E&TC Library");
  lcd.setCursor(0, 1);
  lcd.print("Device 2024");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    return;
  }
}
void loop() {
  if (digitalRead(BORROW_BTN_PIN) == HIGH) {
    Borrow();
  }
  if (digitalRead(RETURN_BTN_PIN) == HIGH) {
    Return();
  }
  delay(100);  // Adjust the delay as needed
}
void Borrow() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning QR Code");
  lcd.setCursor(0, 1);
  lcd.print("For Borrow Book");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Continue button");
  while (digitalRead(CONTINUE_BTN_PIN) == LOW) {
    delay(50);
  }
  Serial.println("start the camera\n");
  // here write capture qr decoding coding part
  String camDataString = Bookid(); // coming from the camera module using serial communication
  char* camData = camDataString.c_str();
  String bookID = String(camData);
  String bookName = searchBook(bookID);
  if (bookName.length() > 0) {
    Serial.print("Book Name found: ");
    Serial.println(bookName);
  } else {
    Serial.println("Book ID not found.");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bookID);
  lcd.setCursor(0, 1);
  lcd.print(bookName);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Continue button");
  while (digitalRead(CONTINUE_BTN_PIN) == LOW) {
    delay(50);
  }
  lcd.setCursor(0, 1);
  lcd.print("Tap RFID Card");
  delay(5000);
  String studentName = ""; // Declare studentName here
  delay(1000);
  String content = readRFID();
  if (content != "") {
    Serial.println("UID tag: " + content);
    studentName = searchStudent(content);
    if (studentName != "") {
      Serial.println("Student Name: " + studentName);
    } else {
      Serial.println("Student not found.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error  Error");
      lcd.setCursor(0, 1);
      lcd.print("Try Again");
      delay(5000);
      wdt_enable(WDTO_15MS);
      while (1) {}
    }
  }
  delay(1000); // Delay to avoid reading the same card multiple times
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(studentName);
  lcd.setCursor(0, 1);
  lcd.print(bookName);
  delay(5000);
  // code for the prepration of records in sd card
  DateTime now = rtc.now();
  File dataFile = SD.open(filename3, FILE_WRITE);
  if (dataFile) {
    dataFile.print(content);
    dataFile.print(",");
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(",");
    dataFile.print(studentName);
    dataFile.print(",");
    dataFile.print(bookID);
    dataFile.print(",");
    dataFile.print(bookName);
    dataFile.println();
    dataFile.close();
    Serial.println("Data written to file.");
  } else {
    Serial.println("Error opening file.");
  }
  delay(5000); // Wait for 10 seconds
  SendSMS();
  // reset the nano automatically
  wdt_enable(WDTO_15MS);
  while (1) {}
}
void Return() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning QR Code");
  lcd.setCursor(0, 1);
  lcd.print("For Return Book");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Continue button");
  while (digitalRead(CONTINUE_BTN_PIN) == LOW) {
    delay(50);
  }
  Serial.println("start the camera\n");
  // here write capture qr decoding coding part
  char camData[] = "1173"; // coming from the camera module using serial communication
  String bookID = String(camData);
  String bookName = searchBook(bookID);
  if (bookName.length() > 0) {
    Serial.print("Book Name found: ");
    Serial.println(bookName);
  } else {
    Serial.println("Book ID not found.");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(bookID);
  lcd.setCursor(0, 1);
  lcd.print(bookName);
  delay(5000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press Continue button");
  while (digitalRead(CONTINUE_BTN_PIN) == LOW) {
    delay(50);
  }
  lcd.setCursor(0, 1);
  lcd.print("Tap RFID Card");
  delay(5000);
  String studentName = ""; // Declare studentName here
  delay(1000);
  String content = readRFID();
  if (content != "") {
    Serial.println("UID tag: " + content);
    studentName = searchStudent(content);
    if (studentName != "") {
      Serial.println("Student Name: " + studentName);
    } else {
      Serial.println("Student not found.");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Error  Error");
      lcd.setCursor(0, 1);
      lcd.print("Try Again");
      delay(5000);
      wdt_enable(WDTO_15MS);
      while (1) {}
    }
  }
  delay(1000); // Delay to avoid reading the same card multiple times
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(studentName);
  lcd.setCursor(0, 1);
  lcd.print(bookName);
  delay(5000);
  // code for the prepration of records in sd card
  DateTime now = rtc.now();
  File dataFile = SD.open(filename3, FILE_WRITE);
  if (dataFile) {
    dataFile.print(content);
    dataFile.print(",");
    dataFile.print(now.year(), DEC);
    dataFile.print('/');
    dataFile.print(now.month(), DEC);
    dataFile.print('/');
    dataFile.print(now.day(), DEC);
    dataFile.print(",");
    dataFile.print(studentName);
    dataFile.print(",");
    dataFile.print(bookID);
    dataFile.print(",");
    dataFile.print(bookName);
    dataFile.println();
    dataFile.close();
    Serial.println("Data written to file.");
  } else {
    Serial.println("Error opening file.");
  }
  delay(5000); // Wait for 10 seconds
  SendSMS();
  // reset the nano automatically
  wdt_enable(WDTO_15MS);
  while (1) {}
}
String searchBook(const String& bookID) {
  file = SD.open(filename1);
  if (file) {
    while (file.available()) {
      String row = file.readStringUntil('\n');
      // Split the row into two parts using ","
      int splitIndex = row.indexOf(',');
      String currentBookID = row.substring(0, splitIndex);
      String bookName = row.substring(splitIndex + 1);

      // Remove leading and trailing whitespaces
      currentBookID.trim();
      bookName.trim();

      if (currentBookID.equals(bookID)) {
        file.close();
        return bookName;
      }
    }
    file.close();
  } else {
    Serial.println("Error opening file!");
  }

  return ""; // Return an empty string if book ID is not found
}
String readRFID() {
  String content = "";
  while (Serial.available()) {
    char receivedChar = Serial.read();
    if (receivedChar != '\n') {
      content += receivedChar; // Append the received character to the string
      // Display the received data on the LCD
      lcd.print(receivedChar);
    } else {
      // Newline received, exit the loop
      break;
    }
  }
  return content;
}
String searchStudent(String id) {
  file = SD.open(filename2);
  if (file) {
    while (file.available()) {
      String line = file.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      String studentID = line.substring(0, commaIndex);
      String studentName = line.substring(commaIndex + 1);
      if (studentID == id) {
        file.close();
        return studentName;
      }
    }
    file.close();
  } else {
    Serial.println("Error opening file!");
  }
  return ""; // Return empty string if student not found
}
void SendSMS(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sending SMS");
  lcd.setCursor(0, 1);
  lcd.print("-");
  delay(1000);
  lcd.print("--");
  delay(1000);
  lcd.print("---");
  delay(1000);
  lcd.print("----");
  delay(1000);
  lcd.print("-----");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Thank You");
  lcd.setCursor(0, 1);
  lcd.print("For Using Device");
  Serial.println("Sending SMS");
  delay(1000);
}
String Bookid(){
  while(mySerial2.available()) 
  {
    String receivedData = mySerial2.readStringUntil('\n'); // Read data from the second software serial until newline
    String originalString;
    //Serial.print("satrt camera");
    int index = receivedData.indexOf('-');
    if (index != -1) 
    {
        originalString = receivedData.substring(index + 1); // Extract substring after '-'
        Serial.println(originalString);
        return originalString;
        //off the esp 32 cam module
    }
  }
}