
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
float cam1_currentAperture;
float cam1_currentFocus;
float cam1_currentExposure;
int cam1_currentGain;

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


}

// --- OSC Messages --- //
void parseBmcMsg(OSCMessage &_msg, int offset) {
  
  int camera = 0; //get address 0
  //  String addr = _msg.getOSCAddress(); //get command


  if (_msg.fullMatch("/bmc/1/aperture",0)) {
    setAperture(1, _msg.getInt(0));
  } else if (_msg.fullMatch("/bmc/1/focus",0)) {
    setFocus(1, _msg.getFloat(0));
  } else if (_msg.fullMatch("/bmc/1/exposure",0)) {
    setExposure(1, _msg.getInt(0));
  }else {

    Serial.println("[ERR] command not regognized...");
    char buff[16];
    _msg.getAddress(buff,0); 
    Serial.println(buff);

  }

  blink();

}

void pingPong(OSCMessage &_msg, int val) {
  blink();

  Serial.println("pinged.");
  OSCMessage reply("/pong");
  reply.add((int32_t)1);
  Udp.beginPacket(debugIp,replyPort);
  reply.send(Udp);
  Udp.endPacket();
  reply.empty();


}

void setReplyPort(OSCMessage &_msg) {
  //replyPort = _msg.getArgInt32(0);
}

void getStatus(OSCMessage &_msg, int camNum = 1){
  
  if(true){
    OSCMessage reply("/status");
    reply.add((int32_t)1);
    reply.add("/status/1/exposure").add((int32_t)cam1_currentExposure);
    Udp.beginPacket(debugIp,replyPort);
    reply.send(Udp);
    Udp.endPacket();
    reply.empty();
  
  }
  
}

// ---- LOOP ---- //

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
                bndl.route("/bmc", parseBmcMsg);    
                bndl.route("/getStatus", getStatus);
             }
        } else{
         Serial.println("err."); 
        }
   }
  // --- //
  
  
    //  DEBUGGINs //
//  Serial.print("Set Aperture to: ");
//  Serial.println(debugApt);
//  setAperture(0,debugApt);
//  debugApt++;
//  debugApt = debugApt % 16;
//
//  delay(1000);
//  Serial.println("delayed");

}



// ---- Camera Commands ---- //

void setAperture(int _camera, float _value) {
  // FORMAT: Float (-1 - 16.0)
  cam1_currentAperture = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 2, 0, cam1_currentAperture );

}

void setFocus(int _camera, float _value) {
  // FORMAT: Float (0.0=near, 1.0=far)
  cam1_currentFocus = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, cam1_currentFocus ); //

}

void setExposure(int _camera, int _value) {
  // FORMAT: Int32 (time in "us" (sp?))
  cam1_currentExposure = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, cam1_currentExposure); //

}

void setGain(int _camera, int _value) {
  // FORMAT: 
  cam1_currentGain = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, cam1_currentGain); //

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

