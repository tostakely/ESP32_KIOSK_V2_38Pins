
/*
version history
started code from existing sketches in 09-05-24 
RFID OK Mamaky sy Manoratra
09-07-2024: add SIM800L communication through Serial2
21/03/2025
Nesorina ilay RTC
*/

#include <SPI.h>
#include <MFRC522.h>  
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "waterFlow.h"
#include <RTClib.h>



//DECLARAION DES PIN
const int Electrovanne_pin = 13;
const int waterFlowPin = 32;
const int BP_pin = 33;
const int SS_PIN = 5;
const int RST_PIN = 2;

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
byte nuidPICC[4];
String cardUID;
String VAR_UID = "";
String VAR_ALEATOIRE_1 , VAR_ALEATOIRE_2 ;
String VAR_MONTANT = "";
String VAR_NUM = "";

// Créer une instance de RTC_PCF8563
RTC_PCF8563 rtc;
LiquidCrystal_I2C lcd(0x27,16,2); 
#define RXD2 16
#define TXD2 17
#define USE_SSL true
#define DELAY_MS 0
#define SIM800L Serial2
String deviceID="0381821103";// code to recognize which device is sending data
String date="";
boolean gprs_disconnect();
boolean is_gprs_connected();
boolean waitResponse(String expected_answer="OK", unsigned int timeout=2000);
const String APN  = "telmanet";
const String USER = "";
const String PASS = "";
const String GOOGLE_SHEETS_API_URL  = "https://script.google.com/macros/s/AKfycbxr9yEF-C5c6YXGV-L7W4TLHRVDN3qeNP8DljwKXyFsx8KxU1vjIk8MAM4ALbw8UzP2WQ/exec";
String request_url = "";


String Data = "";
String timestampnow;
char data1[16],data2[16];
byte buffer2[16];
byte buffer1[16];
String data10="";
String data20="";
const int KEY_1 = 5; 
const int KEY_2 = 7; 
const int KEY_3 = 6; 
const int KEY_4 = 2; 
String STRING_MONTANT="";
short HALF=-1;
short VOUS=-1;
int nbSMS;
boolean WRFID=0;
boolean distributionEnCours=0;
boolean cardValidityToday;
unsigned long val4;
long CREDY_CART = -1;
long CREDY_hosoratna_CART;
unsigned long unixTSvalidity;
String TimeStampUnix;
float validT=0;
float JVAL;
long int vola = 0;
int boutonEtat = 0; 

// Constantes pour le calcul du voltage
const int analogPin = 25; // Pin où la tension divisée est lue
const float R1 = 33000.0; // Résistance 33kΩ
const float R2 = 10000.0; // Résistance 10kΩ
const float Vref = 3.3;   // Référence de tension de l'ESP32
const int resolution = 4095; // Résolution ADC de l'ESP32 (12 bits)

unsigned long startDis,endDis,dureeDis;

const float volumeMax = 5;  // 21.5


volatile byte pulseCount;
byte pulse1Sec = 0;
float volumeDistribue = 0.0;  

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
  volumeDistribue += 0.002125;    
}

void affichage_LCD (int ligne , int colonne , bool cls  , String message){
  if ( cls == true ){
    lcd.clear();
    lcd.setCursor(colonne,ligne);
    lcd.print(message);
  }
  else {
    lcd.setCursor(colonne,ligne);
    lcd.print(message);
  }
   
}

///////////////////////////////////////////////////////////////////////////////////////////////

 void MAMAKY(){
        // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

  //-------------------------------------------

 // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  rfid.PICC_DumpDetailsToSerial(&(rfid.uid)); //dump some details about the card

  cardUID = getCardUID();
  Serial.print(F("Card UID: "));
  Serial.println(cardUID);

  //-------------------------------------------

  Serial.print(F("Name: "));

  byte buffer1[18];

  block = 4;
  len = 18;

  //------------------------------------------- GET FIRST NAME
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(rfid.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));

    rfid.PCD_Init();
      return;
  }

  status = rfid.MIFARE_Read(block, buffer1, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(status));

    rfid.PCD_Init();
    delay(1000);
    status = rfid.MIFARE_Read(block, buffer1, &len);
     Serial.print(F("new status: "));
    Serial.println(rfid.GetStatusCodeName(status));
    return;
  }

  //PRINT FIRST NAME
  for (uint8_t i = 0; i < 15; i++)
  {
    if (buffer1[i] != 32)
    {
      //Serial.write(buffer1[i]);
      Data = Data + char(buffer1[i]);
    }
  }
 

  //---------------------------------------- GET LAST NAME

  byte buffer2[18];
  block = 1;

  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 1, &key, &(rfid.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  
    rfid.PCD_Init();
    return;
  }

  status = rfid.MIFARE_Read(block, buffer2, &len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Reading failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
   
    rfid.PCD_Init();
    return;
  }

  //PRINT LAST NAME
  for (uint8_t i = 0; i < 14; i++) {
    //Serial.write(buffer2[i]);
    Data = Data + char(buffer2[i]);
  }


  //-------------------------------------------
   // Data = Data + '\0';
   Data = decrypt(Data, KEY_1);
   Data = decrypt(Data, KEY_2);
   Data = decrypt(Data, KEY_3);
   Data = decrypt(Data, KEY_4);

  // Recherche de l'index de début et de fin de la sous-chaîne "tojonirina"
  int indexDebut = 0;
  int indexFin = indexDebut + 8;  // La sous-chaîne "tojonirina" a une longueur de 10 caractères
  
  // Extraction de la sous-chaîne "tojonirina"
  VAR_UID = Data.substring(indexDebut, indexFin);
  
  // Recherche des index des caractères '*' et '#'
  int indexAsterisque = Data.indexOf("*");
  int indexCroisillon = Data.indexOf("#");
  int indexarobase= Data.indexOf("@");
  
  // Extraction de la sous-chaîne entre '*' et '#' et "@"
  STRING_MONTANT = Data.substring(indexAsterisque + 1, indexCroisillon);
  char charArray[STRING_MONTANT.length() + 1];
  STRING_MONTANT.toCharArray(charArray, sizeof(charArray));
  CREDY_CART = strtol(charArray, NULL, 10);
  TimeStampUnix= Data.substring(indexCroisillon+1,indexarobase);
  char charArray2[TimeStampUnix.length() + 1];
  TimeStampUnix.toCharArray(charArray2, sizeof(charArray2));
  TimeStampUnix = strtol(charArray2, NULL, 10);

  Serial.println("timeStampUnix="+TimeStampUnix);
    // Affichage des résultats
  
  Serial.print("NUMERO CARTE : ");
  Serial.println(VAR_UID);
  Serial.print(F("VOLA : "));
  Serial.println(CREDY_CART);  
  Serial.println(F("\n**End Reading**\n"));
  Data="";
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

   //CheckValidity();
  affichage_LCD (0,4,true,"NATURANO"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  affichage_LCD (1,0,false,"VOLA : "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  affichage_LCD (1,7,false,String (CREDY_CART)); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  affichage_LCD (1,12,false,"   "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  affichage_LCD (1,14,false,"Ar"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////

void MANORATRA(){
  Serial.println(F("manomboka manoratra"));
   rfid.PCD_Init();
   // Prepare key - all keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

  //some variables we need
  byte block;
  byte len;
  MFRC522::StatusCode status;

   VAR_ALEATOIRE_1 = generateRandomString(4);
   VAR_ALEATOIRE_2 = generateRandomString(4);

  //-------------------------------------------

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent()) {
    //Serial.println("not a new card --> return");
    return;
  }

  // Select one of the cards
  if (!rfid.PICC_ReadCardSerial()) {
   // Serial.println ("can not read serial of card--> return");
    return;
  }

    VAR_UID = getCardUID();
    /*
    Serial.print("NUM_UID = "); 
    Serial.println(VAR_UID);
    */
    /* unix_timestamp=validT;
    String UTS=String(unix_timestampNow);  
    */
    String UTS = " ";
    Data = VAR_UID + "*" + CREDY_hosoratna_CART + "#"+ UTS+"@";
    Data = encrypt(Data, KEY_1); 
    Data = encrypt(Data, KEY_2);
    Data = encrypt(Data, KEY_3); 
    Data = encrypt(Data, KEY_4); 
      data10= Data.substring(0, 15);
      data20=Data.substring(15);
  
         data10.getBytes(buffer1, 16);

  for (int i = 0; i < 16; i++)

  
         data20.getBytes(buffer2, 16);

  for (int i = 0; i < 16; i++)
  
  Serial.println(F("**Card Detected:**"));

  //-------------------------------------------

  rfid.PICC_DumpDetailsToSerial(&(rfid.uid)); //dump some details about the card

  //-------------------------------------------

  block = 4;
  len = 16;

  //------------------------------------------- WRITE FIRST NAME
  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)); //line 834 of MFRC522.cpp file
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
   
    rfid.PCD_Init();
    return;
  }

  //byte firstNameData[16] = {'1', '2', '3', '4', '5', '6', '7', '8', '1', '2', '3', '4', '5', '6', '7', '8'};

  status = rfid.MIFARE_Write(block, buffer1, len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Writing failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  
    rfid.PCD_Init();
    return;
  }

  Serial.println(F("First Name written successfully."));

  //---------------------------------------- WRITE LAST NAME

  block = 1;

  status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(rfid.uid)); //line 834
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Authentication failed: "));
    Serial.println(rfid.GetStatusCodeName(status));
  
    rfid.PCD_Init();
    return;
  }

  //byte lastNameData[16] = {'1', '2', '3', '4', '5', '6', '7', '8', '1', '2', '3', '4', '5', '6', '7', '8'};

  status = rfid.MIFARE_Write(block, buffer2, len);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("Writing failed: "));
    Serial.println(rfid.GetStatusCodeName(status));

    rfid.PCD_Init();
    return;
  }

  Serial.println(F("Last Name written successfully."));

  Serial.println(F("\n**End Writing**\n"));
  
        Serial.println("Data 1 ->" + String(data10));
        Serial.println("Data 2 ->" + String(data20));  
    
        
  delay(1000); //change value if you want to write cards faster

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  rfid.PCD_Init();

  rfid.PCD_Init();

}


///////////////////////helper routines/////////////////////
  String printHex(byte *buffer, byte bufferSize) {
  String num = "";
  for (byte i = 0; i < bufferSize; i++) {
    //Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    if (buffer[i] < 0x10){
    num = num + " 0"; 
    }
    else {
    num = num + " ";
    }
    //Serial.print(buffer[i], HEX);
    num = num + String(buffer[i] , HEX);    
  }
  return num;
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }

}
String generateRandomString(int length) {
  String characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
  String randomString = "";
  
  for (int i = 0; i < length; i++) {
    int randomIndex = random(characters.length());
    char randomChar = characters.charAt(randomIndex);
    randomString += randomChar;
  }
  
  return randomString;
}


String encrypt(String input, int key) {
  String output = "";
  int len = input.length();
  for (int i = 0; i < len; i++) {
    char c = input.charAt(i);
    c += key;  // Décalage de la valeur ASCII du caractère
    output += c;
  }
  return output;
}


String decrypt(String input, int key) {
  String output = "";
  int len = input.length();
  for (int i = 0; i < len; i++) {
    char c = input.charAt(i);
    c -= key;  // Décalage inverse de la valeur ASCII du caractère
    output += c;
  }
  return output;
}


String getCardUID() {
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  return uidString;
}


String temps(){
String mess = "";
  // Obtenir l'heure actuelle
  DateTime now = rtc.now();
  
  String jour = String (now.day(), DEC);
  String mois = String (now.month(), DEC);
  String annee = String (now.year(), DEC);
  String heure = String (now.hour(), DEC);
  String Minute = String (now.minute(), DEC);
  String Seconde = String (now.second(), DEC);
  mess = jour + "/"+ mois + "/" + annee + "--" + heure + ":" + Minute + ":" + Seconde;
return mess;

 /*
 
 // Affichage de l'heure obtenue du module RTC sur le moniteur série
 Serial.print("Heure actuelle (RTC) avec convertion int : ");
 Serial.print(valdateDayNow);
 Serial.print("/");
 Serial.print(valdateMonthNow);
 Serial.print("/");
 Serial.print(valdateYearNow); 
 Serial.print(" ");
 Serial.print(valdateHourNow);
 Serial.print(":");
 Serial.println(valdateMinNow);
 */
}


void ELECTROVANNE_OUVERT(){
 Serial.println("Electronne ouvert");
 digitalWrite(Electrovanne_pin,LOW);
}

void ELETROVANNE_FERME(){
 Serial.println("Electronne fermer");
 digitalWrite(Electrovanne_pin,HIGH);
}

void ecxel(){
  String mess = temps();
  String Tension = Tension_Batterie(analogPin,R1,R2);
   request_url = GOOGLE_SHEETS_API_URL;
  request_url += "?";
  request_url += "&date=";
  request_url += mess;
  request_url += "&deviceID=";
  request_url += deviceID;
  request_url += "&batt_voltage=";
  request_url += Tension;
  request_url += "&card_UID=";
  request_url += VAR_UID;
  request_url += "&Montant=";
  request_url += String(CREDY_CART);
  request_url += "&dureeDis=";
  request_url += String(dureeDis);
  sendGPRSdata();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
void sendGPRSdata(){/////////////// you need to create your request URL before calling this function
  if(!is_gprs_connected()){
    gprs_connect();
  }
  SIM800L.println("AT+CGATT=1");
  delay(1000);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Start HTTP connection
  SIM800L.println("AT+HTTPINIT");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Enabling SSL 1.0
  if(USE_SSL == true){
    SIM800L.println("AT+HTTPSSL=1");
    waitResponse();
    delay(500);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Set the HTTP URL - Firebase URL and FIREBASE SECRET
  SIM800L.println("AT+HTTPPARA=\"URL\",\""+request_url+"\"");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //set http action type 0 = GET, 1 = POST, 2 = HEAD
  //+HTTPACTION: 1,603,0 (POST to Firebase failed)
  //+HTTPACTION: 0,200,0 (POST to Firebase successfull)
  SIM800L.println("AT+HTTPACTION=0");
  for (uint32_t start = millis(); millis() - start < 5000;){
    while(SIM800L.available() > 0){
      String response = SIM800L.readString();
      Serial.println(response);
      if(response.indexOf("+HTTPACTION:") > 0){
        goto OutFor;
      }
    }
  }
  OutFor:
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Stop HTTP connection
  SIM800L.println("AT+HTTPTERM");
  waitResponse("OK",1000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  
 // delay(5000);

}
void gprs_connect()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  // attach or detach from GPRS service 
  SIM800L.println("AT+CGATT=1");
  waitResponse("OK",2000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Connecting to GPRS: GPRS - bearer profile 1
  SIM800L.println("AT+SAPBR=3,1,\"Contype\",\"GPRS\"");
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the APN settings for your sim card network provider.
  SIM800L.println("AT+SAPBR=3,1,\"APN\","+APN);
  waitResponse();
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the user name settings for your sim card network provider.
  /*if(USER != ""){
    SIM800L.println("AT+SAPBR=3,1,\"USER\","+USER);
    waitResponse();
    delay(DELAY_MS);
  }
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //sets the password settings for your sim card network provider.
  if(PASS != ""){
    SIM800L.println("AT+SAPBR=3,1,\"PASS\","+PASS);
    waitResponse();
    delay(DELAY_MS);
  }*/
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //after executing the following command. the LED light of 
  //sim800l blinks very fast (twice a second) 
  //enable the GPRS: enable bearer 1
  SIM800L.println("AT+SAPBR=1,1");
  waitResponse("OK", 2000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Get IP Address - Query the GPRS bearer context status
  SIM800L.println("AT+SAPBR=2,1");
  waitResponse("OK",2000);
  delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
}
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Function: is_gprs_connected()
* checks if the gprs connected.
* if IP address equals "0.0.0.0" its mean not connect to internet
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
boolean is_gprs_connected()
{
  SIM800L.println("AT+SAPBR=2,1");
  if(waitResponse("0.0.0.0") == 1) { return false; }

  return true;
}
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
* Function: gprs_disconnect()
* AT+CGATT = 1 modem is attached to GPRS to a network. 
* AT+CGATT = 0 modem is not attached to GPRS to a network
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
boolean gprs_disconnect()
{
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //Disconnect GPRS
  SIM800L.println("AT+CGATT=0");
 // waitResponse("OK",60000);
  //delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  //DISABLE GPRS
  //SIM800.println("AT+SAPBR=0,1");
  //waitResponse("OK",60000);
  //delay(DELAY_MS);
  //MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
  return true;
}
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//Handling AT COMMANDS
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
boolean waitResponse(String expected_answer, unsigned int timeout)
{
  uint8_t x=0, answer=0;
  String response;
  unsigned long previous;
    
  //Clean the input buffer
  while( SIM800L.available() > 0) SIM800L.read();
  
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  previous = millis();
  do{
    affichage_LCD (1,2,false,String(volumeDistribue));
    //if data in UART INPUT BUFFER, reads it
    if(SIM800L.available() != 0){
        char c = SIM800L.read();
        response.concat(c);
        x++;
        //checks if the (response == expected_answer)
        if(response.indexOf(expected_answer) > 0){
            answer = 1;
        }
    }
  }while((answer == 0) && ((millis() - previous) < timeout));
  //NNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN
  affichage_LCD (1,2,false,String(volumeDistribue));
  Serial.println(response);
  return answer;
}

String Tension_Batterie(int pin , float R1 , float R2){
  int analogValue = analogRead(pin); // Lire la valeur analogique
  float voltage = analogValue * (Vref / resolution); // Convertir en tension (0-3.3V)
  float batteryVoltage = voltage * (R1 + R2) / R2; // Calculer la tension de la batterie
  String Tension = String (batteryVoltage);
/*
  // Afficher les valeurs sur le moniteur série
  Serial.print("Valeur analogique: ");
  Serial.print(analogValue);
  Serial.print(" - Tension mesurée: ");
  Serial.print(voltage);
  Serial.print(" V - Tension batterie: ");
  Serial.print(batteryVoltage);
  Serial.println(" V");
  */
  return Tension;
}
void setup() {
  Serial.begin(9600);
  SIM800L.begin(9600,SERIAL_8N1,RXD2,TXD2);
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522
  Wire.begin();
  analogReadResolution(12); // Régle la résolution à 12 bits (0-4095)
  
 /* if (!rtc.begin()) {
    Serial.println("Impossible de trouver le RTC");
    affichage_LCD (0,4,true,"ERRREUR RTC");
    delay(5000);
    while (1);
  }*/
  
  lcd.init();                     
  lcd.backlight();
  pinMode (BP_pin,INPUT_PULLUP);
  pinMode(waterFlowPin,INPUT_PULLUP);
  pinMode(Electrovanne_pin , OUTPUT);
  ELETROVANNE_FERME();

  pulseCount = 0;
  attachInterrupt(digitalPinToInterrupt(waterFlowPin), pulseCounter, FALLING);

  affichage_LCD (0,4,true,"NATURANO"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  affichage_LCD (1,0,false,"INSERER LA CART"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
  
}

void loop() {
  
MAMAKY();

boutonEtat = digitalRead(BP_pin);
    if(boutonEtat == LOW){
      CREDY_hosoratna_CART = CREDY_CART;
      if(CREDY_hosoratna_CART > 50){
         CREDY_hosoratna_CART = CREDY_CART-50;
         MANORATRA();
         startDis=millis();
         ELECTROVANNE_OUVERT();
         distributionEnCours = true;
         affichage_LCD (0,0,true,"RANO : "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
         affichage_LCD (1,10,false,"litres"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
         volumeDistribue=0;
         
        while(distributionEnCours == true){

           affichage_LCD (1,2,false,String(volumeDistribue)); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
           
              if (volumeDistribue >= volumeMax) {
                 distributionEnCours = false;
                 ELETROVANNE_FERME(); 
                 endDis=millis();
                 dureeDis=endDis-startDis;
                 ecxel();
                 volumeDistribue=0;
                 affichage_LCD (0,4,true,"NATURANO"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
                 affichage_LCD (1,0,false,"VOLA : "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
                 affichage_LCD (1,7,false,String (CREDY_hosoratna_CART)); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
                 affichage_LCD (1,12,false,"   "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
                 affichage_LCD (1,14,false,"Ar"); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
              }
             }
           }
      else if(CREDY_CART < 50 && CREDY_CART > 0){ 
        affichage_LCD (1,0,false,"LANY CREDY CART  "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)   
      }
     else{
        affichage_LCD (1,0,false,"TSY MISY CART   "); //affichage_LCD (int ligne , int colonne , bool cls  , String message)
     }
    }
  }

 
