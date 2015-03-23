
#include <Adafruit_CC3000.h>
#include <ccspi.h>
#include <SPI.h>
#include <string.h>
#include "utility/debug.h"
#include <Wire.h>
#include <Adafruit_MMA8451.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "network"           // cannot be longer than 32 characters!
#define WLAN_PASS       "password"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define IDLE_TIMEOUT_MS  3000      // Amount of time to wait (in milliseconds) with no data 
                                   // received before closing the connection.  If you know the server
                                   // you're accessing is quick to respond, you can reduce this value.

// What page to grab!
#define WEBSITE      "your.server"
#define PORT         ":5000"
uint32_t ip;


Adafruit_MMA8451 mma = Adafruit_MMA8451();
// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      12

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 20; // delay for half a second

unsigned long time;
bool riding = false;
bool sent = false;

void setup(void)
{
  Serial.begin(115200);
  Serial.println(F("Hello, CC3000!\n")); 

  Serial.print("Free RAM: "); Serial.println(getFreeRam(), DEC);
  
  /* Initialise the module */
  Serial.println(F("\nInitializing..."));
  if (!cc3000.begin())
  {
    Serial.println(F("Couldn't begin()! Check your wiring?"));
    while(1);
  }
  
  // Optional SSID scan
  // listSSIDResults();
  
  Serial.print(F("\nAttempting to connect to ")); Serial.println(WLAN_SSID);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Failed!"));
    while(1);
  }
   
  Serial.println(F("Connected!"));
  
  /* Wait for DHCP to complete */
  Serial.println(F("Request DHCP"));
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }

  ip = 0;
  // Try looking up the website's IP address
  Serial.print(WEBSITE); Serial.print(F(" -> "));
  while (ip == 0) {
    if (! cc3000.getHostByName(WEBSITE, &ip)) {
      Serial.println(F("Couldn't resolve!"));
    }
    delay(500);
  }

  cc3000.printIPdotsRev(ip);
  
  //////////////// light /////////////////////
  
  if (! mma.begin()) {
    Serial.println("Couldnt start");
    while (1);
  }
  Serial.println("MMA8451 found!");
  
  mma.setRange(MMA8451_RANGE_2_G);
  
  Serial.print("Range = "); Serial.print(2 << mma.getRange());  
  Serial.println("G");
  
  pixels.begin();
  
}

void loop(void)
{
  // Read the 'raw' data in 14-bit counts
  mma.read();
  /*Serial.print("X:\t"); Serial.print(mma.x); 
  Serial.print("\tY:\t"); Serial.print(mma.y); 
  Serial.print("\tZ:\t"); Serial.print(mma.z); 
  Serial.println();*/

  float angle_x;
  float angle_y;
  float angle_z;
  
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mma.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
  /*Serial.print("X: \t"); Serial.print(event.acceleration.x); Serial.print("\t");
  Serial.print("Y: \t"); Serial.print(event.acceleration.y); Serial.print("\t");
  Serial.print("Z: \t"); Serial.print(event.acceleration.z); Serial.print("\t");
  Serial.println("m/s^2 ");*/

  /* Get the orientation of the sensor */
  uint8_t o = mma.getOrientation();
  
  switch (o) {
    case MMA8451_PL_PUF: 
      //Serial.println("Portrait Up Front");
      break;
    case MMA8451_PL_PUB: 
      //Serial.println("Portrait Up Back");
      break;    
    case MMA8451_PL_PDF: 
      //Serial.println("Portrait Down Front");
      break;
    case MMA8451_PL_PDB: 
      //Serial.println("Portrait Down Back");
      break;
    case MMA8451_PL_LRF: 
      //Serial.println("Landscape Right Front");
      break;
    case MMA8451_PL_LRB: 
      //Serial.println("Landscape Right Back");
      break;
    case MMA8451_PL_LLF: 
      //Serial.println("Landscape Left Front");
      break;
    case MMA8451_PL_LLB: 
      //Serial.println("Landscape Left Back");
      break;
    }
  //Serial.println();
  delay(20);

  //Serial.print("x= ");
  
  angle_x =atan2(-event.acceleration.y,-event.acceleration.z)*57.2957795+176.7;
  /*
  Serial.print(angle_x);
  Serial.print(" deg");
  Serial.print(" ");
  
  Serial.print("y= ");*/
  
  angle_y =atan2(-event.acceleration.x,-event.acceleration.z)*57.2957795;
  /*
  Serial.print(angle_y);
  Serial.print(" deg");
  Serial.print(" ");
  
  Serial.print("z= ");*/
  
  angle_z =atan2(-event.acceleration.y,-event.acceleration.x)*57.2957795+75;
  /*
  Serial.print(angle_z);
  Serial.print(" deg");
  Serial.print("\n");*/
  
  
  if (angle_x<=345 && angle_x >270) {
    pixels.setPixelColor(0,255,0,0);
    pixels.setPixelColor(1,255,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 345 && angle_x<=348) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,255,102,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 348 && angle_x<=352) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,255,102,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 352 && angle_x<=356) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,255,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
    else if (angle_x >-8 && angle_x<=-4) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,255,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x >355 && angle_x<=359.99) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,255,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
    else if (angle_x < 0 && angle_x>-4) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,255,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x >= 0 && angle_x<=4) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,255,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 4 && angle_x<=8) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,255,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 8 && angle_x<=12) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,255,102,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x > 12 && angle_x<=16) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,255,102,0);
    pixels.setPixelColor(10,0,0,0);
    pixels.setPixelColor(11,0,0,0);
    pixels.show();
  }
  else if (angle_x >= 17 && angle_x <70) {
    pixels.setPixelColor(0,0,0,0);
    pixels.setPixelColor(1,0,0,0);
    pixels.setPixelColor(2,0,0,0);
    pixels.setPixelColor(3,0,0,0);
    pixels.setPixelColor(4,0,0,0);
    pixels.setPixelColor(5,0,0,0);
    pixels.setPixelColor(6,0,0,0);
    pixels.setPixelColor(7,0,0,0);
    pixels.setPixelColor(8,0,0,0);
    pixels.setPixelColor(9,0,0,0);
    pixels.setPixelColor(10,255,0,0);
    pixels.setPixelColor(11,255,0,0);
    pixels.show();
  }
  else {
     for(int i=0;i<NUMPIXELS;i++){

    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0,0,0)); // Moderately bright green color.

    pixels.show();
     }
  }
  if (!sent && ((angle_x > -8 && angle_x < 8) || (angle_x > 352))){
    if (riding) {
      unsigned long now = millis();
      if(now-time > 10000) {
        Serial.println("Riding = 5 sec");
        // 5 sec after riding
        // TODO: send signal to server
        talkToServer();
        sent = true;
      }
    } else {
     Serial.println("Riding = true");
     time = millis();   
     riding = true;
    }
  }
  
}

/**************************************************************************/
/*!
    @brief  Begins an SSID scan and prints out all the visible networks
*/
/**************************************************************************/

void talkToServer() {
  Adafruit_CC3000_Client www = cc3000.connectTCP(ip, 5000);
  if (www.connected()) {
    Serial.println("Talk to server!!!!!");
    www.fastrprint(F("GET /snap?on"));
    www.fastrprint(F(" HTTP/1.1\r\n"));
    www.fastrprint(F("Host: ")); www.fastrprint(WEBSITE); www.fastrprint(PORT); www.fastrprint(F("\r\n"));
    www.fastrprint(F("\r\n"));
    www.println();
  } else {
    Serial.println(F("Connection failed"));    
    return;
  }

  Serial.println(F("-------------------------------------"));
  
  /* Read data until either the connection is closed, or the idle timeout is reached. */ 
  unsigned long lastRead = millis();
  while (www.connected() && (millis() - lastRead < IDLE_TIMEOUT_MS)) {
    while (www.available()) {
      char c = www.read();
      Serial.print(c);
      lastRead = millis();
    }
  }
  www.close();
  Serial.println(F("-------------------------------------"));
  
  /* You need to make sure to clean up after yourself or the CC3000 can freak out */
  /* the next time your try to connect ... */
  Serial.println(F("\n\nDisconnecting"));
  cc3000.disconnect();
}

void listSSIDResults(void)
{
  uint32_t index;
  uint8_t valid, rssi, sec;
  char ssidname[33]; 

  if (!cc3000.startSSIDscan(&index)) {
    Serial.println(F("SSID scan failed!"));
    return;
  }

  Serial.print(F("Networks found: ")); Serial.println(index);
  Serial.println(F("================================================"));

  while (index) {
    index--;

    valid = cc3000.getNextSSID(&rssi, &sec, ssidname);
    
    Serial.print(F("SSID Name    : ")); Serial.print(ssidname);
    Serial.println();
    Serial.print(F("RSSI         : "));
    Serial.println(rssi);
    Serial.print(F("Security Mode: "));
    Serial.println(sec);
    Serial.println();
  }
  Serial.println(F("================================================"));

  cc3000.stopSSIDscan();
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    Serial.println(F("Unable to retrieve the IP Address!\r\n"));
    return false;
  }
  else
  {
    Serial.print(F("\nIP Addr: ")); cc3000.printIPdotsRev(ipAddress);
    Serial.print(F("\nNetmask: ")); cc3000.printIPdotsRev(netmask);
    Serial.print(F("\nGateway: ")); cc3000.printIPdotsRev(gateway);
    Serial.print(F("\nDHCPsrv: ")); cc3000.printIPdotsRev(dhcpserv);
    Serial.print(F("\nDNSserv: ")); cc3000.printIPdotsRev(dnsserv);
    Serial.println();
    return true;
  }
}
