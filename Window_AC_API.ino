#include <ESP8266WiFi.h>
#include <aWOT.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#define IR_LED 4  // ESP8266 GPIO pin to use. Recommended: 4 (D2).

IRsend irsend(IR_LED);

//set your wifi SSID and pasword here
const char* ssid = "";
const char* password = "";


//need to know the min and max temp the AC supports so we can properly reset it at start
const int minTemp = 61;
const int maxTemp = 86;

//variable for AC temp as it changes over time
int acTemp = 72;


//these are the amount of times the Mode button needs pressed from one Mode to reach the other
const int toCool = 3;
const int toAuto = 1;

/*
 * Need to update all of this so the power state is internally tracked, status simply returns "state" as an enum
 * OFF = 0, HEAT = 1, COOL = 2, AUTO = 3
 * Then setting state is simply an enum/int. 
 * setMode code will need updated to check current state of both the last state (auto/cool) and on/off power before changing
 * but status should always reflect the int
 */

//this AC unit automatically changes fan speed from "auto" to "low" when changed to fan or dehum.
//changing from cool to auto is fine, but auto to cool means the fan needs changed. this int defines how many times to cycle speed
const int fanSpeed = 1;

enum States { OFF = 0, HEAT = 1, COOL = 2, AUTO = 3 };
States currentState = OFF;
States previousState = AUTO;//necessary to track state changes when turning "off", we need to know whether it was on cool or auto previously

//tracks the current state of the AC power separately to retain mode
bool acPower = false;

//default mode it assumes we're in. keep this lowercase
//String currentMode = "auto";

//IR codes, these are for a Haier HWR05XCM-L
const unsigned long IR_POWER = 0x19F69867;
const unsigned long IR_MODE = 0x19F610EF;
const unsigned long IR_UP = 0x19F6A05F;
const unsigned long IR_DOWN = 0x19F6906F;
const unsigned long IR_SPEED = 0x19F620DF;
const unsigned int IR_DELAY = 20;//delay between sending commands



WiFiServer server(80);
WebApp app;

#pragma region power
void togglePower()
{
  //invert power state
  //acPower = !acPower;
  if (currentState == OFF)
  {
    //restore previous state/mode for tracking purposes, need to remember if it was on cool/auto
    currentState = previousState;
  }
  else
  {
    //before turning off we need to save the current state/mode to make sure it's back to auto/cool when we restore power
    previousState = currentState;
    currentState = OFF;
  }
  
  irsend.sendNEC(IR_POWER, 32);
  delay(IR_DELAY);  
}

void powerCmd(Request &req, Response &res) {
  togglePower();
  statusCmd(req,res);
}
#pragma endregion power

#pragma region mode
void modeCmd(Request &req, Response &res) {

  int modeVal = atoi(req.route("modeVal"));
  //char * modeVal = req.route("modeVal");
  //String newMode(modeVal);

  setMode(modeVal);
  
  statusCmd(req,res);
}

//takes in a string for mode, of auto or cool, turns the AC on if it is off and loops through modes until the proper one is achieved
void setMode(int mode)
{
  int modeLoop = 0;
  bool changeFan = false;
  //lower case to ensure user can send any form
//  mode.toLowerCase();

  //toggle power for these modes, which will also set the previous state in place
  //this is necessary so it doesn't accidentally change to a different state in the next steps
  if ((mode == 3 || mode == 2) && currentState == OFF)
  {
    togglePower();  
  }

  if (mode == 3 && currentState != AUTO)
  {   
    //set to auto
    currentState = AUTO;
    modeLoop = toAuto;
  }
  else if (mode == 2 && currentState != COOL)
  {
    //set to cool
    currentState = COOL;
    modeLoop = toCool;
    changeFan = true; //fan speed needs changed after mode
  }
  else if (mode == 0 && currentState != OFF)
  {
    //if the input is for 0 (off) and power is on, turn it off
    togglePower();
    return;
  }
  else
  {
    //input was "heat" or something else, ignore
    return;
  }

//  if (acPower == false)
//  {
//    //if the power is off we need it on before we can proceed
//    togglePower();
//  }

  //loop through modes 10 times so we don't hit an infinite loop
  //each loop first checks if we're already on the mode we need, then sends an IR code to change mode
  for(int i=1; i<=modeLoop; i++)
  {
    irsend.sendNEC(IR_MODE, 32);
    delay(IR_DELAY);
  }

  //because of how the AC changes fan speed to "low" when cycling modes, this determines if we need to set it back
  if (changeFan)
  {
    for(int i=1; i<=fanSpeed; i++)
    {
      irsend.sendNEC(IR_SPEED, 32);
      delay(IR_DELAY);
    }
  }
}
#pragma endregion mode

#pragma region temperature
void tempCmd(Request &req, Response &res) {
  int tempVal = atoi(req.route("tempVal"));
  setTemp(tempVal);
  
  statusCmd(req,res);
}

//input is degrees to increase on AC, it simply loops through that value and sends the IR_UP signal each time
void tempUp(int degrees)
{
  for(int i=1; i<=degrees; i++){
    //increase temperature variable for tracking
    acTemp++;
    irsend.sendNEC(IR_UP, 32);
    delay(IR_DELAY);
  }
}

//input is degrees to decrease on AC, it simply loops through that value and sends the IR_DOWN signal each time
void tempDown(int degrees)
{
  for(int i=1; i<=degrees; i++){
    //decrease temperature variable
    acTemp--;
    irsend.sendNEC(IR_DOWN, 32);
    delay(IR_DELAY);
  }
}

//determines whether the new temp is higher or lower and uses up/down temp as needed
void setTemp(int newTemp)
{  
  //need to verify new temp isn't outside of range of max/min
  //as well as determine if we're going up or down
  if (newTemp > acTemp && newTemp <= maxTemp)
  {
    if (currentState == OFF)
    {
      togglePower();
    }
    
    //calculate temp difference and pass to tempUp
    newTemp = newTemp - acTemp;
    tempUp(newTemp);
  }
  else if (newTemp < acTemp && newTemp >= minTemp)
  {
    if (currentState == OFF)
    {
      togglePower();
    }

    //calculate temp diference and pass to tempDown
    newTemp = acTemp - newTemp;
    tempDown(newTemp);
  }
}

//the top temp is 86, bottom is 61 on the AC, we need to loop 25 times, changing temp down each time
//once at the bottom we can set the temp up to whatever value is needed
void resetTemp()
{
  togglePower();
  delay(IR_DELAY);
  //calculate the temp difference
  int resetLoop = maxTemp - minTemp;
  //loop that many times, sending the down signal so we have a starting place to count up from
  for(int i=1; i<=resetLoop; i++){
    irsend.sendNEC(IR_DOWN, 32);
    delay(IR_DELAY);
  }
  //temporarily hold the acTemp which is our default value  
  int tmpTemp = acTemp;
  //set acTemp to new value, which should be the minimum
  acTemp = minTemp;

  //now that we know what value the AC is at, we can tell it to set back to the default value
  setTemp(tmpTemp);
}
#pragma endregion temperature

void statusCmd(Request &req, Response &res) {
  res.success("application/json");
  String currentStatus;

  currentStatus = "{\"Temperature\":";
  currentStatus += (int)acTemp;
  currentStatus += ",\"State\":";
  currentStatus += (int)currentState;
  currentStatus += "}\n";
  res.print(currentStatus);
  Serial.print(currentStatus);
}

void setup() {

  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  
  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());


  pinMode(IR_LED, OUTPUT);

  //resetTemp();

  // mount the handler to the default router
  app.get("/power", &powerCmd);
  app.get("/state/:modeVal", &modeCmd);
  app.get("/temp/:tempVal", &tempCmd);
  app.get("/status", &statusCmd);

}

void loop(){
  WiFiClient client = server.available();
  
  if (client.connected()) {
    app.process(&client);
  }
}
