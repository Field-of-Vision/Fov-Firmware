
/********************************************************************************/
//Stepper Motor Setup//
/********************************************************************************/
//Y Axis Stepper
#include <AccelStepper.h>
#include <MultiStepper.h>

#define ENABLE_Y    19
#define DIR_Y       13
#define STEP_Y      14
#define motorInterfaceType 1
AccelStepper stepper_Y = AccelStepper(motorInterfaceType, STEP_Y, DIR_Y);

//X Axis Stepper
#define ENABLE_X    23
#define DIR_X       32
#define STEP_X      33
AccelStepper stepper_X = AccelStepper(motorInterfaceType, STEP_X, DIR_X);

MultiStepper steppers;
long positionMove[2];

//Conversion for axis coordinates to steps
#define xConvert 224

//Normal Pulley
#define yConvert 270

//Larger Pulley
//#define yConvert 413.5

void stepperSetup(){
  pinMode(ENABLE_X, OUTPUT);
  pinMode(ENABLE_Y, OUTPUT);
  digitalWrite(ENABLE_X, HIGH);
  digitalWrite(ENABLE_Y, HIGH);
      // Configure each stepper
  stepper_X.setMaxSpeed(15000);
  stepper_Y.setMaxSpeed(15000);

  // Then give them to MultiStepper to manage
  steppers.addStepper(stepper_X);
  steppers.addStepper(stepper_Y);
}

 int xSpd = 12500;
 int ySpd = 12500;

//Homing Sequence Variables
//X AXIS
long initial_homing_X=-1;  // Used to Home Stepper at startup
//Y AXIS
long initial_homing_Y=1;  // Used to Home Stepper at startup


//End Stop Hall Sensor
#define  hall_X   36
#define  hall_Y   38

void hallSensorsSetup(){
  pinMode(hall_X, INPUT);
  pinMode(hall_Y, INPUT);  
}


/********************************************************************************/
//AWS Server Setup//
/********************************************************************************/
#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"


#define AWS_IOT_PUBLISH_TOPIC   "esp32/globalTest"
#define AWS_IOT_SUBSCRIBE_TOPIC "PreMatch/1"

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager


//wifi manager creates local network "FOV_Board1" which allows user to setup wifi connection
//Board saves last connected network details and will try to connect to that network first
//If that connection fails it returns to local network to setup new wifi connection
void wifiManagerSetup(){
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    //wm.resetSettings();
    bool res;

    //Change what credentials of local network generated by the board
    
    // res = wm.autoConnect(); // auto generated AP name from chipid
     res = wm.autoConnect("FOV_Board1"); // anonymous ap
    //res = wm.autoConnect("FOV_Board1","fov123");

    if(!res) {
        Serial.println("Failed to connect");
        //ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected");
    }
  }
  

void connectAWS()
{
  wifiManagerSetup();

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.print("Connecting to AWS IOT");

  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }

  if(!client.connected()){
    Serial.println("AWS IoT Timeout!");
    return;
  }

  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);

  Serial.println("AWS IoT Connected!");
}

void publishMessage(){
 
  StaticJsonDocument<200> doc;
  //doc["Match Request"] = "Manchester_City_QPR_41042.json";
  doc["Match Request"] = "fixed_pattern.json";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  Serial.println(jsonBuffer);
  Serial.println("Message sent to " + String(AWS_IOT_PUBLISH_TOPIC));
 // Serial.println(12345);
}
//{
//"timestamp":1.25,
//"x":60,
//"y":29,
//"possession":0,
//"pass":0,
//"receive":0,
//"home goal":0,
//"away goal":0,
//"out":0
//},
//EXAMPLE INCOMING MESSAGE:
//{"timestamp": 75.5, "x": 16, "y": 37, "possession": 1, "pass": 0, "receive": 0, "home goal": 0, "away goal": 0, "out": 0}

int lastPoss = 0;
int vibeMode;
float prevX = 0;
float prevY = 0;

void messageHandler(char* topic, byte* payload, unsigned int length) {
  Serial.print("incoming: ");
  Serial.println(topic);

  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  float timestamp = doc["timestamp"];
  int xReceived = doc["x"];
  float yReceived = doc["y"];
  int possession = doc["possession"];
  int pass = doc["pass"];
  
  int received = doc["receive"];
  int homeGoal = doc["home Goal"];
  int awayGoal = doc["away Goal"] ;
  int out = doc["out"] ;

  //Serial.println(message);
  
  Serial.print("Timestamp: "); Serial.println(timestamp);
  Serial.print("X Coord: "); Serial.println(xReceived);
  Serial.print("Y Coord: "); Serial.println(yReceived);
  Serial.print("Possession: "); Serial.println(possession);
  Serial.print("Pass: "); Serial.println(pass);
  Serial.print("Received: "); Serial.println(received);
  Serial.print("Home Goal: "); Serial.println(homeGoal);
  Serial.print("Away Goal: "); Serial.println(awayGoal);
  Serial.print("Out: "); Serial.println(out);

  //Vibration Mode, vibration response depending on what event has occured
// if(lastPoss != possession)vibeMode = 0;  
// if(lastPoss != possession)vibeMode = 6;  
//  else if(pass == 1)        vibeMode = 1; //board motor 2`    
//  else if(received == 1)    vibeMode = 2; //board motor 1
//  else if(homeGoal == 1)    vibeMode = 3; //both motors
//  else if(awayGoal == 1)    vibeMode = 4; // both motors
//  else if(out == 1)         vibeMode = 5; //both motors 
//  else                      vibeMode = 6; //No Vibe

  if (pass == 1 && possession == 1) vibeMode = 0;
  else if (pass == 1 && possession == 0) vibeMode = 1;
  else if (received == 1 && possession == 1) vibeMode = 2;
  else if (received == 1 && possession == 0) vibeMode = 3;
  else if (homeGoal == 1) vibeMode = 4;
  else if (awayGoal == 1) vibeMode = 5;
  else vibeMode = 6;
  
  lastPoss = possession;
  Serial.print("Vibe Mode: "); Serial.println(vibeMode);
  if(vibeMode != 6)coreSetup();

//  Function to calculate required speed of movement
  //based on distance between last coord and next coord
  speedCalc(prevX, prevY , xReceived, yReceived);
  //Function to move stepper motors to next position
  moveStepsToPos(xReceived, yReceived);
  prevX = xReceived;
  prevY = yReceived;

}

  void speedCalc(float x1, float y1, float x2, float y2){ //calculate required stepper speed based on distance the ball has the travel within an alotted time
    float t = 0.25; //timeframe to complete movement
    
    //adjust coordinates back to mm
    x1 = x1*1.875;
    x2 = x2*1.875;
    y1 = y1*2.5;
    y2 = y2*2.5;
    float d = sqrt((sq(x2 - x1))+(sq(y2-y1))); //distance to next coordinate
    float s = d/t; //in mm/s //speed needed to get to next point within allowed timeframe 
    
    xSpd = (s*166); //convert speed  mm/s to stepper speed
    ySpd = (s*166);

    //Top speed is 20000 before motors start jamming
    if(xSpd > 20000){ xSpd = 20000; ySpd = 20000;}
    Serial.print("Speed Calulated: "); Serial.println(xSpd);
     
    }


//////////////////////////
//Vibration Motor Setup//
/////////////////////////

//Vibration Motor in pitch
#define VIB_GPIO1   9
#define PWM1_Ch     0
#define PWM1_Res    8
#define PWM1_Freq   1000

//Vibration motor on body
#define VIB_GPIO2  21
#define PWM2_Ch    1
#define PWM2_Res   8
#define PWM2_Freq  1000

int PWM1_DutyCycle = 0; 
int PWM2_DutyCycle = 0;

void pwmPinsSetup(){
  
  //Setup Motor 1
  ledcAttachPin(VIB_GPIO1, PWM1_Ch);
  ledcSetup(PWM1_Ch, PWM1_Freq, PWM1_Res);
  //Set up Motor 2
  ledcAttachPin(VIB_GPIO2, PWM2_Ch);
  ledcSetup(PWM2_Ch, PWM2_Freq, PWM2_Res);

  ledcWrite(PWM1_Ch, 0);
  ledcWrite(PWM2_Ch, 0);
  
  }


///////////////////////////////////////////////////////////////////////////
//Vibration Motor On Second Core
///////////////////////////////////////////////////////////////////////////
//Vibration functionality is handled by ESP32's second core to provide multitasking

//Multitasking RTOS Cores
TaskHandle_t PWMVibe;

void coreSetup(){
  xTaskCreatePinnedToCore(
    pwmMotor,    // Function that should be called
    "Motor PWM",  // Name of the task (for debugging)
    10000,            // Stack size (bytes)
    NULL,            // Parameter to pass
    1,               // Task priority
    &PWMVibe,             // Task handle
    0                //Select Which Core
);

}
//1 = body, 2 = pitch
//vibration response depending on events
void pwmMotor( void * pvParametersm ){

  //VibeMode = 0 HOME PASS#########################
  if(vibeMode == 0){    
      ledcWrite(PWM1_Ch, 200);
      delay(300);
      ledcWrite(PWM1_Ch, 0);}

  //VibeMode = AWAY PASS#############################
  if(vibeMode == 1){    
      ledcWrite(PWM2_Ch, 200);
      delay(300);
      ledcWrite(PWM2_Ch, 0);}

  //VibeMode = 2 HOME RECEIVE#########################
//  if(vibeMode == 2){    
//      ledcWrite(PWM2_Ch, 200);
//      delay(25);
//      ledcWrite(PWM2_Ch, 0);
//      delay(25);
//      ledcWrite(PWM2_Ch, 200);
//      delay(25);
//      ledcWrite(PWM2_Ch, 0);
//      }

  if(vibeMode == 2){    
      ledcWrite(PWM1_Ch, 200);
      delay(100);
      ledcWrite(PWM1_Ch, 0);}

  //VibeMode = 3 AWAY RECEIVE#########################
//  if(vibeMode == 3){    
//      ledcWrite(PWM1_Ch, 200);
//      ledcWrite(PWM2_Ch, 200);
//      delay(200);
//      ledcWrite(PWM1_Ch, 0);
//      ledcWrite(PWM2_Ch, 0);
//      }

  if(vibeMode == 3){    
      ledcWrite(PWM2_Ch, 200);
      delay(100);
      ledcWrite(PWM2_Ch, 0);}

  //VibeMode = 4 Away Goal Scored
  if(vibeMode == 4){    
      ledcWrite(PWM1_Ch, 200);
      ledcWrite(PWM2_Ch, 200);
      delay(25);
      ledcWrite(PWM1_Ch, 0);
      ledcWrite(PWM2_Ch, 0);
      delay(25);
      ledcWrite(PWM1_Ch, 200);
      ledcWrite(PWM2_Ch, 200);
      delay(25);
      ledcWrite(PWM1_Ch, 0);
      ledcWrite(PWM2_Ch, 0);
      
      }

  //VibeMode = HOME GOAL
  if(vibeMode == 5){    
      ledcWrite(PWM1_Ch, 200);
      ledcWrite(PWM2_Ch, 200);
      delay(50);
      ledcWrite(PWM1_Ch, 0);
      ledcWrite(PWM2_Ch, 0);
      }


      vTaskDelete(NULL);
}



void setup() {
  Serial.begin(9600);
  stepperSetup();
  hallSensorsSetup();
  pwmPinsSetup();
  homeSteppers();
  delay(1000);
  connectAWS();
  //publishMessage();
  Serial.println("ready to receive");
  delay(250);
}

//Period which board checks for new messages from server
int period = 140;
unsigned long time_now = 0;

void loop() {
 // publishMessage();
     if(millis() >= time_now + period){
        time_now += period;
        client.loop();
    }
 
}


void homeSteppers(){
/////////////////////////////////////////////////////////////////////////////////////////
// X AXIS HOMING
////////////////////////////////////////////////////////////////////////////////////////
  float homingSpd = 15000.0;
 Serial.println("Homing X Axis");
   digitalWrite(ENABLE_X, LOW);
   digitalWrite(ENABLE_Y, HIGH);
   delay(5);  // Wait for EasyDriver wake up

   //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper_X.setMaxSpeed(homingSpd);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper_X.setAcceleration(homingSpd);  // Set Acceleration of Stepper
 

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper X is Homing . . . . . . . . . . . ");

  while (digitalRead(hall_X)) {  // Make the Stepper move CCW until the switch is activated 
    //Serial.println(digitalRead(home_switch));
    stepper_X.moveTo(initial_homing_X);  // Set the position to move to
    initial_homing_X--;  // Decrease by 1 for next move if needed
    stepper_X.run();  // Start moving the stepper
    delay(1);
}

  stepper_X.setCurrentPosition(0);  // Set the current position as zero for now
  stepper_X.setMaxSpeed(homingSpd);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper_X.setAcceleration(homingSpd);  // Set Acceleration of Stepper
  initial_homing_X=-1;

  while (!digitalRead(hall_X)) { // Make the Stepper move CW until the switch is deactivated
    stepper_X.moveTo(initial_homing_X);  
    stepper_X.run();
    initial_homing_X++;
    delay(1);
  }
  
  stepper_X.setCurrentPosition(0);
  Serial.println("Homing X Axis Completed");


/////////////////////////////////////////////////////////////////////////////////////////
// Y AXIS HOMING
////////////////////////////////////////////////////////////////////////////////////////   
  Serial.println("Homing Y Axis");
   digitalWrite(ENABLE_Y, LOW);
   digitalWrite(ENABLE_X, HIGH);
   delay(3);  // Wait for EasyDriver wake up

   //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper_Y.setMaxSpeed(homingSpd);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper_Y.setAcceleration(homingSpd);  // Set Acceleration of Stepper
 

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper Y is Homing . . . . . . . . . . . ");

  while (digitalRead(hall_Y)) {  // Make the Stepper move CCW until the switch is activated 
    //Serial.println(digitalRead(home_switch));
    stepper_Y.moveTo(initial_homing_Y);  // Set the position to move to
    initial_homing_Y++;  // Decrease by 1 for next move if needed
    stepper_Y.run();  // Start moving the stepper
    delay(1);
}

  stepper_Y.setCurrentPosition(0);  // Set the current position as zero for now
  stepper_Y.setMaxSpeed(homingSpd);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper_Y.setAcceleration(homingSpd);  // Set Acceleration of Stepper
  initial_homing_Y=1;

  while (!digitalRead(hall_Y)) { // Make the Stepper move CW until the switch is deactivated
    stepper_Y.moveTo(initial_homing_Y);  
    stepper_Y.run();
    initial_homing_Y--;
    delay(1);
  }
  
  stepper_Y.setCurrentPosition(0);
  Serial.println("Homing Y Axis Completed");

  moveStepsToPos(4, 1);
  stepper_X.setCurrentPosition(0);
  stepper_Y.setCurrentPosition(0);
  
  digitalWrite(ENABLE_X, HIGH);
  digitalWrite(ENABLE_Y, HIGH);
  
  }


  void moveStepsToPos(long x, long y){
  stepper_X.setMaxSpeed(xSpd);
  stepper_Y.setMaxSpeed(ySpd);
  //Serial.println("Moving Steppers!");
  digitalWrite(ENABLE_X,LOW);
  digitalWrite(ENABLE_Y,LOW);
  positionMove[0] = x * xConvert;
  positionMove[1] = -y * yConvert;
  steppers.moveTo(positionMove);
  steppers.runSpeedToPosition(); // Blocks until all are in position
  //delay(1);
  //digitalWrite(ENABLE_X,HIGH);
  //digitalWrite(ENABLE_Y,HIGH);

}
