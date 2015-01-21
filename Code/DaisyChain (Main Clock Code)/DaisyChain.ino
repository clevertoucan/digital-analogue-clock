/* Finished Code for the "Lite Clock" LED Analogue Clock Project.
Group 8 - Joshua Owens, James Cummins, Daniela Perea, Jonathan Schnurr, Anthony Mastradone.
Written by Joshua Owens.

PLEASE NOTE: This code is HIGHLY INDIVIDUALIZED for this project's circuit set-up. A detailed
schematic of the circuit is available here: 
https://drive.google.com/open?id=0B7LfLZSndjtiNDFSQlFpNjVPVlE&authuser=0*/

#include <Time.h> 
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>

bool load;

int dataPin = 6;
int latchPin = 8;
int clockPin = 10;

/*
  up = Useable Pins. 
  op = Other Pins. 
  List of possible pin configurations for each shift register:
  0b00000001 is q0 (a null value, for this project), the next is q1, the first output pin of the shift register, and so on.
  */
  byte up[] = {0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};
  byte op[] = {0b00000001, 0b00000011, 0b00000111, 0b00001111, 0b00011111, 0b00111111, 0b01111111, 0b11111111};
  byte full = 0b11111111;

  bool minuteHasBeenFound;
  bool hourHasBeenFound;
  /*--------------------Ethernet Stuff---------------------------------*/

  byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; 
// NTP Servers:
IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov
// IPAddress timeServer(132, 163, 4, 103); // time-c.timefreq.bldrdoc.gov


const int timeZone = -5;     
//const int timeZone =  1;  // Central European Time
//const int timeZone = -5;  // Eastern Standard Time
//const int timeZone = -4;  // Eastern Daylight Time
//const int timeZone = -8;  // Pacific Standard Time
//const int timeZone = -7;  // Pacific Daylight Time


EthernetUDP Udp;  
unsigned int localPort = 8888;  // local port to listen for UDP packets

/*---------------------Running Code--------------------------------*/

void setup() {
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  int webCheck = Ethernet.maintain();
  if(webCheck == 1){
    setUpConnection();
  }
}

void loop() {
  if(timeStatus() != timeNotSet){
    runClock(minute(), hour());
  }
  else{
    loading();
  }
}

/*----------------------- Different functions for light configurations, time-to-clock conversion, etc------------------*/

void myShift(int num){
  shiftOut(dataPin, clockPin, MSBFIRST, num);
}

//minutes: any integer value between 1 and 60, inclusive. 
//hours: any integer value between 1 and 12, inclusive.
//Passing zero to either argument will result in no pins being lit up.
void runClock(int minutes, int hours){
  digitalWrite(latchPin, LOW);
  // Converts time into position on the clock:
  // LOGIC: Divide minute by the 7 pins available on each shift register
  // and then see if minutes%7 = 0. If it is, then it's 
  // pin 7 in the number'th shift reg you get from minute/7, if it's not, it's 
  // minutes%7'th pin in the number'th shift register you get from minutes/7

  //Minutes
  for(int i = 0; i<11; i++){
    if(i<9){
      if(minutes/7 == i+1 && minutes%7 == 0){
        myShift(up[7]);
      }
      else if(minutes/7 == i && minutes%7 != 0){
        myShift(up[minutes%7]);
      }
      else{
        myShift(up[0]);
      }
    }
  //Hours
  else{
    if((hours/7)+9 == i+1 && hours%7 == 0){
      myShift(up[7]);
    }
    else if((hours/7)+9 == i && hours%7 != 0){
      myShift(up[hours%7]);
    }
    else{
      myShift(up[0]);
    }
  }
}
digitalWrite(latchPin, HIGH);
}

//Other possible runClock; all previous pins are lit up behind current minute and hour.
//Passing a number greater than 60 or 12 in minutes or hours will result in all pins
//on the circuit being lit up. As with the first runClock, passing 0 will result in no
//pins being lit up. 

void runClock1(int minutes, int hours){
  minuteHasBeenFound = false;
  hourHasBeenFound = false;
  digitalWrite(latchPin, LOW);
  // Converts time into position on the clock:
  // LOGIC: Divide minute by the 7 pins available on each shift register
  // and then see if minutes%7 = 0. If it is, then it's 
  // pin 7 in the number'th shift reg you get from minute/7, if it's not, it's 
  // minutes%7'th pin in the number'th shift register you get from minutes/7

  //Minutes
  for(int i = 0; i<11; i++){
    if(i<9){
      if(minutes/7 == i+1 && minutes%7 == 0){
        myShift(op[7]);
        minuteHasBeenFound = true;
      }
      else if(minutes/7 == i && minutes%7 != 0){
        myShift(op[minutes%7]);
        minuteHasBeenFound = true;
      }
      else if(minuteHasBeenFound) {
        myShift(op[0]);
      }
      else{
        myShift(op[7]);
      }
    }
  //Hours
  else{
    if((hours/7)+9 == i+1 && hours%7 == 0){
      myShift(op[7]);
      hourHasBeenFound = true;
    }
    else if((hours/7)+9 == i && hours%7 != 0){
      myShift(op[hours%7]);
      hourHasBeenFound = true;
    }
    else if(hourHasBeenFound){
      myShift(op[0]);
    }
    else{
      myShift(op[7]);
    }
  }
}
digitalWrite(latchPin, HIGH);
}


void flash(){
  digitalWrite(latchPin, HIGH);
  for(int i = 0; i<11; i++){
    myShift(full);
  }
  digitalWrite(latchPin, LOW);
}

void flashBlue(){
  digitalWrite(latchPin, HIGH);
  for(int i = 0; i<9; i++){
    myShift(full);
  }
  digitalWrite(latchPin, LOW);
}

void flashRed(){
  digitalWrite(latchPin, HIGH);
  for(int i = 10; i<11; i++){
    myShift(full);
  }
  digitalWrite(latchPin, LOW);
}

void allOff(){
  runClock(0,0);
}

/*Possible idea for alternative loader:
Save old spot as a variable, do a manual myShift to that 
specific LED location after the old spot moves on to get
the illusion of two dots moving. */
int i = 1;
void loading(){
  if(i >= 61){
    i = 1;
  }
  if(millis()%20 == 0){
    runClock(i,0);
    i+=1;
  }
}

int j = 1;
bool a = true;
void loading1(){
  if(j >= 61){
    j = 60;
    a = false;
  }
  if(j <= 0){
    j = 1;
    a = true;
  }
  if(millis()%200 == 0){
    runClock1(j, 0);
  }
  if(a){
    j++;
  }
  else{
    j--;
  }
}


/*-----------------------NTP Code-----------------*/


void setUpConnection(){

  int webCheck = Ethernet.begin(mac);
  if(webCheck == 0 && timeStatus() == timeNotSet) {
// Clock has no useable time or ethernet connection; errors out
flashRed();
delay(500);
allOff();
}
if(webCheck = 1){
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(86400);
  if(timeStatus() == timeSet){
    for(int i = 0; i =10; i++){
      flashBlue();
      delay(10);
      flashRed();
      delay(10);
    }
  }
}
}



const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 15000) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}