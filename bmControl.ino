
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
#include <ArdOSC.h>

// Constants //
#define DEFAULT_OSC_RECEIVE_PORT 8888
#define DEFAULT_IP_ADDRESS 192,168,0,168
#define STATUS_LED_PIN 13

// Ethernet Stuff //
IPAddress targetIp(192,168,0,13);
byte mac[] = {0x90, 0xA2, 0xDA, 0x10, 0x41, 0x80};
IPAddress ip(DEFAULT_IP_ADDRESS);
char packetBuffer[UDP_TX_PACKET_MAX_SIZE];  //buffer to hold incoming packet,
char  sendBuffer[] = "acknowledged...";       // a string to send back


// OSC Stuff //
OSCServer oscServer;
unsigned int oscReceivePort = DEFAULT_OSC_RECEIVE_PORT;   
float replyPort=8000; //where to respond to pings and gets

// Camera Communication Stuff //
const int                 shieldAddress = 0x6E;
BMD_SDICameraControl_I2C  sdiCam(shieldAddress);


// Camera State //
float currentAperture;
float currentFocus;


// ---- Main ---- //
void setup(){
	Serial.begin(11522);

	// Network Setup //
	Ethernet.begin(mac,ip);
	oscServer.begin(oscReceivePort);

	// Camera Setup //
	sdiCam.begin();
  	Wire.setClock(400000); // Set max I2C speed
  	sdiCam.setOverride(true);



	// Top-Level Callbacks //
	oscServer.addCallback("/ping", &ping);
	oscServer.addCallback("/setReplyPort", &setReplyPort);

	oscServer.addCallback("/bmc/^", &parseMsg);

	oscServer.addCallback("/bmcRaw/void", &rawVoid);
	oscServer.addCallback("/bmcRaw/fixed16", &rawFixed16);


}

void loop(){
	
}

void parseMsg( OSCMessage *_msg){
	int camera = 0; //get address 0
	String cmd = "hello"; //get command


	if(cmd=="hello"){

		} else{

			Serial.println("[ERR] command not regognized...");
		}

	blink();

}

// ---- Misc ---- //
void ping(OSCMessage *_msg){

	OSCMessage reply;
	reply.setOSCAddress("/pong");
	reply.setIpAddress(_msg.getIpAddress());
	reply.setPortNumber(replyPort);
	reply.addArdString("ok!");
	blink();

}

void setReplyPort(OSCMessage *_msg){
	replyPort = _msg.getArgInt32(0);
}


// ---- Camera Commands ---- //

void setAperture(int _camera, float _value){
	// FORMAT: Float (-1 - 16.0)
	currentAperture = _value;
	sdiCam.writeCommandFixed16(_camera,0,2,0, currentAperture ); 

}

void setFocus(int _camera, float _value){
	// FORMAT: Float (0.0=near, 1.0=far)
	currentFocus = _value;
	sdiCam.writeCommandFixed16(_camera,0,0,0, currentFocus ); //

}

// ---- Raw Commands ---- //
void rawVoid(OSCMessage *_msg){
	int _cam = 1; //camera
	int _cat = 0; //cateogry
	int _param = 0; //parameter

	sdiCam.writeCommandVoid(_cam, _cat, _param);

}

void rawFixed16(OSCMessage *_msg){

}

// ---- Utility Functions ---- //
void blink(){
	digitalWrite(STATUS_LED_PIN, HIGH);
  	delay(50);
  	digitalWrite(STATUS_LED_PIN, LOW);
  	delay(50);
}

