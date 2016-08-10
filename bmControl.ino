
/*
 BM Control

 This program allows for easy OSC and Serial control of Black Magic Cameras over SDI.

 	Board Used:
   	* Arduino Ethernet Rev 3

	Shields used:
 	* Black Magic Arduino SDI Shield

 	3rd Party Libraries Used:
 	* Black Magic SDI Control Library
 	* OSC (CNMAT)

 OSC Message Formatting:
 /bmc/[camera number]/[command] [value(s)]
 
 Licensed under GPL.

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
#define NUM_CAMERAS 2 //number of cameras to be controlled

// Ethernet Stuff //
EthernetUDP Udp;
IPAddress targetIp(192, 168, 0, 13);
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x63, 0x08};
byte ip[] = {DEFAULT_IP_ADDRESS};
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  sendBuffer[] = "acknowledged...";       // a string to send back

// OSC Stuff //
int oscReceivePort = DEFAULT_OSC_RECEIVE_PORT;
float replyPort = 8000; //where to respond to pings and gets

// Camera Communication Stuff //
const int                 shieldAddress = 0x6E;
BMD_SDICameraControl_I2C  sdiCam(shieldAddress);

// Camera State //
struct Camera{
  int id;
  float aperture;
  float focus;
  int exposure;
  int sensorGain;
  int whiteBalance;
  float lift[4];
  float gamma[4];
  float gain[4];
};
Camera cameras[NUM_CAMERAS+1];


//debug stuff
int debugApt = 0; //testing aperture
byte replyIp[]  = { 192, 168, 0, 13 };

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
  
  char address[256];
  _msg.getAddress(address,0);
  int cam = getAddressSegment(address,1).toInt(); //get address segment 1
  String cmd = getAddressSegment(address,2); //get address segment 2
  
  Serial.println(cam);
  Serial.println(cmd);

  if (cmd == "aperture") {
    setAperture(cam, _msg.getFloat(0));
  } else if (cmd == "focus") {
    setFocus(cam, _msg.getFloat(0));
  } else if (cmd == "exposure") {
    setExposure(cam, _msg.getInt(0));
  } else if (cmd == "sensorGain") {
    setSensorGain(cam, _msg.getInt(0));
  } else if (cmd == "whiteBalance") {
    setWhiteBalance(cam, _msg.getInt(0));
  } else if (cmd == "lift") {
    float input[4];
    input[0] = _msg.getFloat(0);
    input[1] = _msg.getFloat(1);
    input[2] = _msg.getFloat(2);
    input[3] = _msg.getFloat(3);
    setLift(cam, input);
  } else if (cmd == "gamma") {
    float input[4];
    input[0] = _msg.getFloat(0);
    input[1] = _msg.getFloat(1);
    input[2] = _msg.getFloat(2);
    input[3] = _msg.getFloat(3);
    setGamma(cam, input);
  }else if (cmd == "gain") {
    float input[4];
    input[0] = _msg.getFloat(0);
    input[1] = _msg.getFloat(1);
    input[2] = _msg.getFloat(2);
    input[3] = _msg.getFloat(3);
    setGain(cam, input);
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
  Udp.beginPacket(replyIp,replyPort);
  reply.send(Udp);
  Udp.endPacket();
  reply.empty();


}

void setReplyIp(OSCMessage &_msg, int val){
  replyIp[0] = _msg.getInt(0);
  replyIp[1] = _msg.getInt(1);
  replyIp[2] = _msg.getInt(2);
  replyIp[3] = _msg.getInt(3);

}

void setReplyPort(OSCMessage &_msg, int val) {
  replyPort = _msg.getInt(0);
}

void getStatus(OSCMessage &_msg, int camNum){
  
  if(true){
    OSCMessage reply("/status");
    reply.add((int32_t)1);
    reply.add("/status/1/exposure").add((int32_t)cameras[1].exposure);
    Udp.beginPacket(replyIp,replyPort);
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
     Serial.println("Packet Received...");


    while(size--)
        bndl.fill(Udp.read());

        if(!bndl.hasError()){
           Serial.println("no err");
             if(bndl.size() > 0) {
                static int32_t sequencenumber=0;
                bndl.route("/ping", pingPong);    
                bndl.route("/bmc", parseBmcMsg);
            
                bndl.route("/bmcRawVoid", writeRawVoid );    
                bndl.route("/getStatus", getStatus);
                bndl.route("/setReplyPort", setReplyPort);
                bndl.route("/setReplyIp", setReplyIp);


             }
        } else{
         Serial.println("Error."); 
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
  cameras[_camera].aperture = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 2, 0, cameras[_camera].aperture );


}

void setFocus(int _camera, float _value) {
  // FORMAT: Float (0.0=near, 1.0=far)
  cameras[_camera].focus = _value;
  sdiCam.writeCommandFixed16(_camera, 0, 0, 0, cameras[_camera].focus ); //

}

void setExposure(int _camera, int _value) {
  // FORMAT: Int32 (time in "us" (sp?))
  cameras[_camera].exposure = _value;
  sdiCam.writeCommandInt32(_camera, 1, 5, 0, cameras[_camera].exposure); //

}

void setSensorGain(int _camera, int _value) {
  // FORMAT: Int8 (1x, 2x, 4x, 8x, 16x gain)
  cameras[_camera].sensorGain = _value;
  sdiCam.writeCommandInt8(_camera, 1, 1, 0, cameras[_camera].sensorGain); //

}

void setWhiteBalance(int _camera, int _value){
  // FORMAT: Int16: White Balance in Kelvin (3200 - 7500)
  cameras[_camera].whiteBalance = _value;
  sdiCam.writeCommandInt16(_camera, 1, 2, 0, cameras[_camera].whiteBalance); //
  
}

void setLift(int _camera, float _rgbl[4]){
  // FORMAT: float [4] RGBL, -4.0 - 4.0
  cameras[_camera].lift[0] = _rgbl[0];
  cameras[_camera].lift[1] = _rgbl[1];
  cameras[_camera].lift[2] = _rgbl[2];
  cameras[_camera].lift[3] = _rgbl[3];

  sdiCam.writeCommandFixed16(_camera, 8, 0, 0, cameras[_camera].lift ); //

}

void setGamma(int _camera, float _rgbl[4]){
  // FORMAT: float [4] RGBL, -4.0 - 4.0
  cameras[_camera].gamma[0] = _rgbl[0];
  cameras[_camera].gamma[1] = _rgbl[1];
  cameras[_camera].gamma[2] = _rgbl[2];
  cameras[_camera].gamma[3] = _rgbl[3];

  sdiCam.writeCommandFixed16(_camera, 8, 1, 0, cameras[_camera].gamma ); //

}

void setGain(int _camera, float _rgbl[4]){
  // FORMAT: float [4] RGBL, -4.0 - 4.0
  cameras[_camera].gain[0] = _rgbl[0];
  cameras[_camera].gain[1] = _rgbl[1];
  cameras[_camera].gain[2] = _rgbl[2];
  cameras[_camera].gain[3] = _rgbl[3];

  sdiCam.writeCommandFixed16(_camera, 8, 1, 0, cameras[_camera].gain ); //

}

// ---- Raw Commands ---- //
void writeRawVoid(OSCMessage &_msg, int val) {
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

String getAddressSegment(String _fullAddress, int segment){ //for getting individual elements of OSC addresses
  int startSlashIndex=0;
  int endSlashIndex;
  
  for(int i=0;i<segment;i++){
    startSlashIndex= _fullAddress.indexOf("/", startSlashIndex+1);
  }
  
  endSlashIndex =  _fullAddress.indexOf("/", startSlashIndex+1);
//  Serial.print("substring: ");
//  Serial.println(_fullAddress.substring(startSlashIndex+1, endSlashIndex));
  return _fullAddress.substring(startSlashIndex+1, endSlashIndex);
  
}

