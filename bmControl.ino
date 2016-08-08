
/*
 BM Control

 This program allows for easy OSC and Serial control of Black Magic Cameras over SDI.

 	Board Used:
   	* Arduino Ethernet

	Shields used:
 	* Black Magic Arduino SDI Shield

 	3rd Party Libraries Used:
 	* Black Magic SDI Control Library
 	* ArdOSC (**MARK KROPF VERSION: https://github.com/MarkKropf/ArdOSC)

 OSC Message Formatting:
 /bmc/[camera number]/[command] [value(s)]

*/

// Libraries //
#include <SPI.h>
#include <Wire.h>
#include <BMDSDIControl.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

#include <OSCBundle.h>



// Constants //
#define DEFAULT_OSC_RECEIVE_PORT 8888
#define DEFAULT_IP_ADDRESS 192,168,0,66
#define STATUS_LED_PIN 9

// Ethernet Stuff //
EthernetUDP Udp;
IPAddress targetIp(192, 168, 0, 13);
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x63, 0x08};
byte ip[] = {DEFAULT_IP_ADDRESS};
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  sendBuffer[] = "acknowledged...";       // a string to send back


// OSC Stuff //
//OSCServer oscServer;
//OSCClient oscClient;
int oscReceivePort = DEFAULT_OSC_RECEIVE_PORT;
float replyPort = 8000; //where to respond to pings and gets
OSCMessage toSend;


// Camera Communication Stuff //
const int                 shieldAddress = 0x6E;
BMD_SDICameraControl_I2C  sdiCam(shieldAddress);


// Camera State //
float currentAperture;
float currentFocus;
float currentExposure;
int currentGain;


//debug stuff
int debugApt = 0; //testing aperture
byte debugIp[]  = { 192, 168, 0, 13 };

// ---- Main ---- //
void setup() {
  Serial.begin(115200);
  Serial.println("Hi.");
  
  // Network Setup //
  Ethernet.begin(mac, ip);
  Udp.begin(oscReceivePort);
  Serial.println("Enet Enabled.");

  // Camera Setup //
  sdiCam.begin();
  Wire.setClock(400000); // Set max I2C speed
  sdiCam.setOverride(true);
  Serial.println("SDI Enabled.");

  // Top-Level Callbacks //
//  oscServer.addCallback("/ping", &pingPong);
//  oscServer.addCallback("/setReplyPort", &setReplyPort);
//
//  oscServer.addCallback("/bmc/^", &parseBmcMsg);
//  oscServer.addCallback("/bmcRaw/void", &rawVoid);
//  oscServer.addCallback("/bmcRaw/fixed16", &rawFixed16);
//  Serial.println("OSC Set up.");


}

void loop() {
 
  // check for new bundles ///
   OSCBundle bndl;
   int size;
   
   // --- receive a bundle  --- //
   if( (size = Udp.parsePacket())>0) {
     Serial.println("packet");


    while(size--)
        bndl.fill(Udp.read());
        Serial.println("read");

        if(!bndl.hasError()){
           Serial.println("no err");
             if(bndl.size() > 0) {
                static int32_t sequencenumber=0;
                bndl.route("/ping", pingPong);
                Udp.beginPacket(Udp.remoteIP(), replyPort);
                bndl.send(Udp);
                Udp.endPacket();     

             }
        } else{
         Serial.println("err."); 
        }
   }
  // --- //
  
  
  
  Serial.print("Set Aperture to: ");
  Serial.println(debugApt);
  setAperture(0,debugApt);
  debugApt++;
  debugApt = debugApt % 16;
  
//  Udp.beginPacket(debugIp, outPort);
//  toSend.beginMessage("/hi");
//  toSend.addArgInt32(123);
//  oscClient.send(&toSend);
//  Serial.println("sent");
//  toSend.flush();
//  Serial.println("fushed");

  delay(1000);
  Serial.println("delayed");
      

}

void parseBmcMsg( OSCMessage &_msg) {
  
  int camera = 0; //get address 0
//  String addr = _msg.getOSCAddress(); //get command


  if (true) {

  } else {

    Serial.println("[ERR] command not regognized...");
  }

  blink();

}

// ---- Misc ---- //
void pingPong(OSCMessage &_msg, int val) {
  blink();

  Serial.println("pinged.");
//  toSend.setAddress(debugIp,replyPort);
//  toSend.beginMessage("/pong");
//  toSend.addArgString("ok!");
//  oscClient.send(&toSend);
//  toSend.flush();

}

void setReplyPort(OSCMessage &_msg) {
  //replyPort = _msg.getArgInt32(0);
}


// ---- Camera Commands ---- //

void setAperture(int _camera, float _value) {
  // FORMAT: Float (-1 - 16.0)
  currentAperture = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 2, 0, currentAperture );

}

void setFocus(int _camera, float _value) {
  // FORMAT: Float (0.0=near, 1.0=far)
  currentFocus = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, currentFocus ); //

}

void setExposure(int _camera, int _value) {
  // FORMAT: Int32 (time in "us" (sp?))
  currentExposure = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, currentExposure); //

}

void setGain(int _camera, int _value) {
  // FORMAT: Int32 (time in "us" (sp?))
  currentGain = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, currentGain); //

}


// ---- Raw Commands ---- //
void rawVoid(OSCMessage &_msg) {
  int _cam = 1; //camera
  int _cat = 0; //cateogry
  int _param = 0; //parameter

  sdiCam.writeCommandVoid(_cam, _cat, _param);

}

void rawFixed16(OSCMessage *_msg) {

}

// ---- Utility Functions ---- //
void blink() {
  digitalWrite(STATUS_LED_PIN, HIGH);
  delay(50);
  digitalWrite(STATUS_LED_PIN, LOW);
  delay(50);
}

