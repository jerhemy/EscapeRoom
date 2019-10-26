/*
 * 
 * All the resources for this project: https://github.com/jswaldon/EscapeRoom
 * Created by Jerhemy Waldon
 * 
 */

/*  Pin layout used:
 *  --------------------------------------
 *              MFRC522      Arduino       
 *              Reader/PCD   Uno/101       
 *  Signal      Pin          Pin          
 *  --------------------------------------
 *  RST/Reset   RST          9             
 *  SPI SS 1    SDA(SS)      8
 *  SPI MOSI    MOSI         11 / ICSP-4
 *  SPI MISO    MISO         12 / ICSP-1
 *  SPI SCK     SCK          13 / ICSP-3
 */
 
#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

#define SS_PIN        8
#define RST_PIN       9
#define SENSOR_ID     0x01
#define NO_CARD       "0000000"

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0D, 0xF6, 0xFF };

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
EthernetClient client;
IPAddress server(192, 168, 0, 2);
bool cardSet = false;

String lastCard = NO_CARD;
String newCard = NO_CARD;

void setup() 
{
  Serial.begin(9600);   // Initiate a serial communication
  
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // no point in carrying on, so do nothing forevermore:
    while (true) {
      delay(1);
    }
  }
  
  // print your local IP address:
  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP());
  
  SPI.begin();      // Initiate  SPI bus
  delay(1000);    
  mfrc522.PCD_Init();   // Initiate MFRC522
  delay(4);
  
  mfrc522.PCD_DumpVersionToSerial();
    
  Serial.println("Approximate your card to the reader...");
  Serial.println();
}


void loop() 
{
  EthernetHealthCheck();
  
  // If no cards are detected, exit loop
  if (!mfrc522.PICC_IsNewCardPresent()) 
  {
    Serial.println("No Card Detected");
  } else if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("Same Card");    
  } else {
    Serial.println();
    Serial.print(F("Card UID:"));
    newCard = dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();  
  }
  Serial.print(F("New Card:"));
  Serial.println(newCard);
  Serial.print(F("Last Card:"));
  Serial.println(lastCard); 
   
  if(lastCard != newCard) {
    if (client.connect(server, 1880))
    {
      Serial.println("Connecting to Client...");
      // Make a HTTP request:
      client.print("GET /red_book?id=");
      client.print(newCard);
      client.println(" HTTP/1.1\n");
      client.println("Host: localhost\n");
      client.println("Connection: close");
      client.println();
      lastCard = newCard;
    }
    else {
       Serial.println("Client Connection Failed!");
    }  
  }
  delay(3000);
}

void EthernetHealthCheck() {
  switch (Ethernet.maintain()) {
    case 1:
      //renewed fail
      Serial.println("Error: renewed fail");
      break;

    case 2:
      //renewed success
      Serial.println("Renewed success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    case 3:
      //rebind fail
      Serial.println("Error: rebind fail");
      break;

    case 4:
      //rebind success
      Serial.println("Rebind success");
      //print your local IP address:
      Serial.print("My IP address: ");
      Serial.println(Ethernet.localIP());
      break;

    default:
      //nothing happened
      break;
  }
}
 
/**
   Helper routine to dump a byte array as hex values to Serial.
*/
String dump_byte_array(byte * buffer, byte bufferSize) {
  String rfid_uid = "";
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    String uid_part = String(buffer[i], HEX); 
    rfid_uid += uid_part;
  }
  Serial.println();
  Serial.println(rfid_uid);
  return rfid_uid;
}
