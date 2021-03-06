
/*
 BM Control
 Copyright (c) 2016 Jesse Garrison

 This program allows for easy OSC and Serial control of Black Magic Cameras over SDI.

 	Board Used:
   	* Arduino Ethernet Rev 3

	Shield used:
 	* Black Magic Arduino SDI Shield

 	3rd Party Libraries Used:
 	* Black Magic SDI Control Library
 	* OSC (CNMAT)

 OSC Message Formatting:
 /bmc/[camera number]/[command] [value(s)]

 Licensed under GPL 3.
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
IPAddress targetIp(192,168,0,255);
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x63, 0x69};
byte ip[] = {DEFAULT_IP_ADDRESS};
char packetBuffer[256];  //buffer to hold incoming packet,
char  sendBuffer[] = "acknowledged...";       // a string to send back

// OSC Stuff //
int oscReceivePort = DEFAULT_OSC_RECEIVE_PORT;
float replyPort = 8000; //where to respond to pings and gets
byte replyIp[]  = { 192,168,0,255 };

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
  float hue;
  float saturation;
};
Camera cameras[NUM_CAMERAS+1];


//debug stuff
int debugApt = 0; //testing aperture

// ---- Main ---- //
void setup() {
//  Serial.begin(115200);
//  Serial.println("Hi.");

  // Network Setup //
  Ethernet.begin(mac, ip);
  Udp.begin(oscReceivePort);
//  Serial.println("Enet Enabled.");

  // Camera Setup //
  sdiCam.begin();
  Wire.setClock(400000); // Set max I2C speed
  sdiCam.setOverride(true);
//  Serial.println("SDI Enabled.");


}

// --- OSC Messages --- //
void parseBmcMsg(OSCMessage &_msg, int offset) {

  char address[256];
  _msg.getAddress(address,0);
  int cam = getAddressSegment(address,1).toInt(); //get address segment 1
  String cmd = getAddressSegment(address,2); //get address segment 2

//  Serial.println(cam);
//  Serial.println(cmd);


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
  } else if(cmd== "hue"){
    setHue(cam, _msg.getFloat(0));
  } else if (cmd=="saturation"){
    setSat(cam, _msg.getFloat(0));

  }else {

//    Serial.println("[ERR] command not regognized...");
    char buff[16];
    _msg.getAddress(buff,0);
//    Serial.println(buff);

  }

}

void bigMess(OSCMessage &_msg, int val){
  char address[256];
  _msg.getAddress(address,0);
  int cam = getAddressSegment(address,1).toInt(); //get address segment 1
  
 // float thisFloat = _msg.getFloat(0);
   setFocus(cam, _msg.getFloat(0));
    setAperture(cam, _msg.getFloat(1));
//  setExposure(cam, _msg.getInt(0));
    setSat(cam, _msg.getFloat(2));
    setHue(cam, _msg.getFloat(3));
    setSensorGain(cam, _msg.getInt(4));
    setWhiteBalance(cam, _msg.getFloat(5));
    
    
//  char msgString[256];
//  _msg.getString(0,msgString,256 );
//  float thisFloat = getAddressSegment(msgString,0).toFloat();
//   setFocus(cam, thisFloat);
//   thisFloat = getAddressSegment(msgString,1).toFloat();
//    setAperture(cam, thisFloat);
////  setExposure(cam, _msg.getInt(0));
//     thisFloat = getAddressSegment(msgString,2).toFloat();
//    setSat(cam, thisFloat);
//    thisFloat = getAddressSegment(msgString,3).toFloat();
//    setHue(cam, thisFloat);
//    float thisInt = getAddressSegment(msgString,4).toInt();
//
//    setSensorGain(cam, thisInt);
//    thisInt = getAddressSegment(msgString,4).toInt();
//    setWhiteBalance(cam, thisInt);

//    float lift[4];
//    lift[0] = thisFloat;
//    lift[1] = thisFloat;
//    lift[2] = thisFloat;
//    lift[3] = thisFloat;
//    setLift(cam, lift);
//
//    float gamma[4];
//    gamma[0] = _msg.getFloat(8);
//    gamma[1] = _msg.getFloat(8);
//    gamma[2] = _msg.getFloat(8);
//    gamma[3] = _msg.getFloat(8);
//    setGamma(cam, gamma);
//    float gain[4];
//    gain[0] = _msg.getFloat(8);
//    gain[1] = _msg.getFloat(8);
//    gain[2] = _msg.getFloat(8);
//    gain[3] = _msg.getFloat(8);
//    setGain(cam, gain);
//       Serial.print("got here: ");
     //  Serial.println(_msg.getFloat(1));


  
}

void pingPong(OSCMessage &_msg, int val) {
//  blink();

//  Serial.println("pinged.");
  OSCMessage reply("/pong");
//  reply.add((int32_t)1);
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

//void getStatus(OSCMessage &_msg, int camNum){
//    camNum = _msg.getInt(0);
//    OSCBundle reply;
//    //reply.beginMessage(makeStatusAddr(camNum,"aperture"));
//    reply.add(makeStatusAddr(camNum,"aperture")).add((float)cameras[camNum].aperture);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"focus")).add((float)cameras[camNum].focus);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"exposure")).add((int32_t)cameras[camNum].exposure);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"sensorGain")).add((int32_t)cameras[camNum].sensorGain);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"whiteBalance")).add((int32_t)cameras[camNum].whiteBalance);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"lift")).add((int32_t)cameras[camNum].lift[0]).add((float)cameras[camNum].lift[1]).add((float)cameras[camNum].lift[2]).add((float)cameras[camNum].lift[3]);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"gamma")).add((int32_t)cameras[camNum].gamma[0]).add((float)cameras[camNum].gamma[1]).add((float)cameras[camNum].gamma[2]).add((float)cameras[camNum].gamma[3]);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"gain")).add((int32_t)cameras[camNum].gain[0]).add((float)cameras[camNum].gain[1]).add((float)cameras[camNum].gain[2]).add((float)cameras[camNum].gain[3]);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//    reply.add(makeStatusAddr(camNum,"whiteBalance")).add((int32_t)cameras[camNum].hue);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//    
//    reply.add(makeStatusAddr(camNum,"whiteBalance")).add((int32_t)cameras[camNum].saturation);
//    Udp.beginPacket(replyIp,replyPort);
//    reply.send(Udp);
//    Udp.endPacket();
//    reply.empty();
//
//}

// ---- LOOP ---- //

void loop() {

  // check for new bundles ///
   OSCBundle bndl;
   int size;

   // --- receive a bundle  --- //
   if( (size = Udp.parsePacket())>0) {
//     Serial.println("Packet Received...");


    while(size--){
        bndl.fill(Udp.read());
//        Serial.print(".");
    }

        if(!bndl.hasError()){
//           Serial.println("no err");
             if(bndl.size() > 0) {
                static int32_t sequencenumber=0;
                bndl.route("/ping", pingPong);
                bndl.route("/bmc", parseBmcMsg);

                bndl.route("/bmcRawVoid", writeRawVoid );
         //       bndl.route("/getStatus", getStatus);
                bndl.route("/setReplyPort", setReplyPort);
                bndl.route("/setReplyIp", setReplyIp);

                bndl.route("/bigMess", bigMess);

//                Serial.println("routed.");
             }
        } else{
//         Serial.println("Error.");
        }
//        Serial.print(".");
   }
  // --- //

 // blink();

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

void setHue(int _camera, float _value){
  cameras[_camera].hue = _value;
  float colorCombined[2] = {cameras[_camera].hue, cameras[_camera].saturation};
  sdiCam.writeCommandFixed16(_camera,8,6,0,colorCombined);
}

void setSat(int _camera, float _value){
  cameras[_camera].saturation = _value;
  float colorCombined[2] = {cameras[_camera].hue, cameras[_camera].saturation};
  sdiCam.writeCommandFixed16(_camera,8,6,0,colorCombined);
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

char* makeStatusAddr(int _camNum, String _variable){
  char buffer[256];
  String addr("/status/");
  addr+=_camNum;
  addr+="/";
  addr+=_variable;

  addr.toCharArray(buffer, 256);
//  Serial.println(buffer);

  return buffer;

}
