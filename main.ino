// Declaration of Libraries
//LCD
#include <Wire.h> 

#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4); 

//Button
#include <Button.h>

//Encoder
#include <ClickEncoder.h>

//Timer 1 for encoder
#include <TimerOne.h>

//Save Function
#include <EEPROMex.h>

//Stepper Library
#include <AccelStepper.h>

// Define a stepper and the pins it will use
AccelStepper motorStep1(AccelStepper::FULL2WIRE, 47, 49);
AccelStepper motorStep2(AccelStepper::FULL2WIRE, 43, 41);

//Declaration of Variables
//Rotary Encoder Variables
boolean up = false;
boolean down = false;
boolean middle = false;

ClickEncoder *encoder;
int16_t last, value;

//EEPROMex Variables

//Save to EEPROMex Adresses

byte enterChar[] = {
B10000,
  B10000,
  B10100,
  B10110,
  B11111,
  B00110,
  B00100,
  B00000
};

byte fastChar[] = {
  B10111,
  B10101,
  B10111,
  B00000,
  B00000,
  B00100,
  B01110,
  B11111
};

//LCD Menu Logic
int refreshScreen=0;

const int numOfMainScreens = 4;
const int numOfSettingScreens = 6;
const int numOfmotorSpeed = 6;
const int numOfTestMenu = 5;
int currentScreen = 0;
int currentSettingScreen = 0;
int currentmotorSpeed = 0;
int currentTestMenuScreen = 0;

String screens[numOfMainScreens][2] = {{"Settings","Click to Edit"}, {"Run Auto", "Enter to Run"}, {"Run Manual", "Enter to Run"},{"Test Machine", "Enter to Test"}};
String settings[numOfSettingScreens][2] = {{"Grinding Time","Mins"}, {"Salt", "Grams"},{"Mixing Time", "Min"},{"Press Time", "Min"},{"Motor Speed", "Click to Edit"}, {"Save Settings", "Enter to Save"}};
String TestMenuScreen[numOfTestMenu] = {"Test Conveyor","Test Grinder","Test Mixer","Test Press","Back to Main Menu"};

String motorSpeed[numOfmotorSpeed][2] = {{"Grinder Speed","Hz"}, {"Grinder Press", "Speed"}, {"Salt Speed", "Speed"},{"Press Mixer", "Hz"},{"Conveyor", "Hz"},{"Save Settings", "Enter to Save"}};


double parameters[numOfSettingScreens]={1,1,1,1};
double parametersMaxValue[numOfSettingScreens]={120,1000,60,60};

double parametersMotor[numOfmotorSpeed]={60,4000,4000,40,15,0};
double parametersMaxValueMotor[numOfmotorSpeed]={60,4000,4000,60,60,60};


//Save to EEPROMex Adresses
    int grindTimeAdd = 0;
    int saltTimeAdd = 10;
    int mixTimeAdd = 20;
    int pressTimeAdd = 30;
    int grinderHZAdd = 40;
    int grinderPressSpeedAdd = 50;
    int saltSpeedAdd = 60;
    int pressMixerHZAdd = 70;
    int conveyorHZAdd = 80;

    double grindTime = 0;
    double saltTime = 0;
    double mixTime = 0;
    double pressTime = 0;
    double grinderHZ = 0;
    double grinderPressSpeed = 0;
    double saltSpeed = 0;
    double pressMixerHZ = 0;
    double conveyorHZ = 0;

    void save_setting(){
        grindTime =  parameters[0];
        saltTime = parameters[1];
        mixTime =  parameters[2];
        pressTime =  parameters[3];
        grinderHZ = parametersMotor[0];
        grinderPressSpeed =  parametersMotor[1];
        saltSpeed =  parametersMotor[2];
        pressMixerHZ = parametersMotor[3];
        conveyorHZ =  parametersMotor[4];

        EEPROM.writeDouble(grindTimeAdd, grindTime);
        EEPROM.writeDouble(saltTimeAdd, saltTime);
        EEPROM.writeDouble(mixTimeAdd, mixTime);
        EEPROM.writeDouble(pressTimeAdd, pressTime);
        EEPROM.writeDouble(grinderHZAdd, grinderHZ);
        EEPROM.writeDouble(grinderPressSpeedAdd, grinderPressSpeed);
        EEPROM.writeDouble(saltSpeedAdd, saltSpeed);
        EEPROM.writeDouble(pressMixerHZAdd, pressMixerHZ);
        EEPROM.writeDouble(conveyorHZAdd, conveyorHZ);
        convertRawDataToMillis();
    }

    void load_settings(){
        parameters[0] = EEPROM.readDouble(grindTimeAdd);
        parameters[1] = EEPROM.readDouble(saltTimeAdd);
        parameters[2] = EEPROM.readDouble(mixTimeAdd);
        parameters[3] = EEPROM.readDouble(pressTimeAdd);
        parametersMotor[0] = EEPROM.readDouble(grinderHZAdd);
        parametersMotor[1] = EEPROM.readDouble(grinderPressSpeedAdd);
        parametersMotor[2] = EEPROM.readDouble(saltSpeedAdd);
        parametersMotor[3] = EEPROM.readDouble(pressMixerHZAdd);
        parametersMotor[4] = EEPROM.readDouble(conveyorHZAdd);

        grindTime = EEPROM.readDouble(grindTimeAdd);
        saltTime = EEPROM.readDouble(saltTimeAdd);
        mixTime = EEPROM.readDouble(mixTimeAdd);
        pressTime = EEPROM.readDouble(pressTimeAdd);
        grinderHZ = EEPROM.readDouble(grinderHZAdd);
        grinderPressSpeed = EEPROM.readDouble(grinderPressSpeedAdd);
        saltSpeed = EEPROM.readDouble(saltSpeedAdd);
        pressMixerHZ = EEPROM.readDouble(pressMixerHZAdd);
        conveyorHZ = EEPROM.readDouble(conveyorHZAdd);

        convertRawDataToMillis();
    }

   


bool settingsFlag = false;
bool settingsEditFlag = false;
bool motorSpeedFlag = false;
bool motorEditSpeedFlag = false;
bool testMenuFlag = false;
bool runAutoFlag = false;
bool runManualFlag = false;


// Timer
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

unsigned long grindingTime = 20000;
unsigned long mixingTime = 20000;
unsigned long saltingTime = 20000;
unsigned long pressingTime = 20000;

unsigned long saltMovePerGram = 84;
unsigned long saltMovePerBatch = 0;

unsigned long minRemaining = 0;
unsigned long secRemaining = 0;
unsigned long timerGrinder = 0;



void convertRawDataToMillis(){

    grindingTime = parameters[0] * 60000;
    mixingTime = parameters[2] * 60000;
    saltingTime = parameters[1] * 60000;
    pressingTime = parameters[3] * 60000;

}


// Declaration Run Flags
bool homeConveyorFlag, conveyorFlag, grindingFlag, mixingFlag, saltFlag, pressFlag, dumpFlag = false;
bool pauseBet = false;
bool grindingPositionFlag, mixingPositionFlag, pressingPositionFlag, dumpPositionFlag;
bool MoveToHome, MoveToGrinder, MoveToMixer, MoveToPress, MoveToDump = false;


// Declaration Relays
int forward = A11;
int reverse = A10;

int speed = 5; //PWM Signal
double hz_multiplier = 255/120;

int conveyor = A15;
int grinder = A14;
int mixer = A12;
int pressMixer = A13;
int pressLiquid = A9;

int liquidValve = A8;

bool runallowed = false;

long initial_homingStep1 = -1;
long targetPosition = 0;
long previousPosition = 0;


int grindPressEndS1 =26;
int grindPressEndS2 =31;

// Mixing Declarations
int mixingPressEndS1 = 23;
int mixingPressEndS2 = 25;
bool mixingGoDown, mixingGoUp, mixingTimerFlag = false;

bool grindingTimerFlag = false;

// Declaration Sensors
int grindPosSensor = 30;
int mixPosSensor = 22;
int presPosSensor = 24;
int dumpPosSensor = 32;

int waterSensor = 18;

//Fast Scroll
bool fastScroll = false;

//Test Machine
bool testConveyorFlag, testinitialmove = false;
int curPositionConveyor = 0; // 0 = Floating 1 = Grinding 2 = Mixing 3 = Press 4 = Dumping
int recPosCon = 0;
bool testRevFor = false;
bool testMoveToGrinder, testMoveToMixer, testMoveToPress, testMoveToDump = false;

bool testGrind, testMixer, testPress = false;


// Calibrate Grind Press Variables
bool calibrateGrindPressFlag = false;
unsigned long grindPressDistance = 0;

// Homing Sequence Variables
bool Init_Homing_Sequence, Init_HomingGrindPress,Init_HomingMixerPress, Init_HomingConveyor = false;
bool goup, godown, initialmove = false;
bool mixgoup, mixgodown, mixinitialmove = false;
bool homethemotor = false;
unsigned long stepper1Distance = 0;


//Functions for Rotary Encoder
void timerIsr(){
  encoder->service();
}

void readRotaryEncoder(){
  value += encoder->getValue();
  
  if (value/2 > last) {
    last = value/2;
    down = true;
    //delay(100);
  }else   if (value/2 < last) {
    last = value/2;
    up = true;
    //delay(100);
  }
}

    unsigned long currentTime = millis();
    const unsigned long eventInterval = 1000;
    unsigned long previousTime = 0;

    void refreshScreensEvery1Second(){
        currentTime = millis();
        if (currentTime - previousTime >= eventInterval) {
            
            refreshScreen = 1;
        /* Update the timing for the next time around */
            previousTime = currentTime;
        }
    }

    unsigned long currentTime1 = millis();
    unsigned long grindPressTickTimer = 0;
    unsigned long previousTime1 = 0;
    unsigned long moveEveryTick = 0;
    unsigned long moveEveryTime = 0;
    int counter = 0;

    void pushPressGrindEveryTick(){
        currentTime1 = millis();
        if (currentTime1 - previousTime1 >= grindPressTickTimer) {
            
            if (digitalRead(grindPressEndS2) == 0 || counter >= 9)
            {
                /* code */
            }else{
                moveEveryTime = moveEveryTime + moveEveryTick;
                motorStep1.runToNewPosition(moveEveryTime);
                counter++;
            }
        /* Update the timing for the next time around */
            previousTime1 = currentTime1;
        }
    }



    unsigned long currentTime2 = millis();
    unsigned long SaltTickTimer = 0;
    unsigned long previousTime2 = 0;
    unsigned long saltmoveEveryTick = 0;
    unsigned long saltmoveEveryTime = 0;
    int saltcounter = 0;

    void saltEveryTick(){
        currentTime2 = millis();
        if (currentTime2 - previousTime2 >= SaltTickTimer) {
            
            if (digitalRead(grindPressEndS2) == 0 || counter >= 9)
            {
                /* code */
            }else{
                saltmoveEveryTime = saltmoveEveryTime + saltmoveEveryTick;
                motorStep2.runToNewPosition(saltmoveEveryTime);
                saltcounter++;
            }
        /* Update the timing for the next time around */
            previousTime2 = currentTime2;
        }
    }

void setup()
{
    //Encoder Setup
    encoder = new ClickEncoder(4,3, 2);
    encoder->setAccelerationEnabled(false);
    Timer1.initialize(1000);
    Timer1.attachInterrupt(timerIsr); 
    last = encoder->getValue();

    // Change these to suit your stepper if you want
    motorStep1.setMaxSpeed(4000);
    motorStep1.setAcceleration(4000);

    // Change these to suit your stepperif you want
    motorStep2.setMaxSpeed(1000);
    motorStep2.setAcceleration(1000);

    //Pin Initialize Relay
    
    pinMode(conveyor, OUTPUT);
    digitalWrite(conveyor, HIGH);

    pinMode(grinder, OUTPUT);
    digitalWrite(grinder, HIGH);
     
    pinMode(mixer, OUTPUT);
    digitalWrite(mixer, HIGH);

    pinMode(pressMixer, OUTPUT);
    digitalWrite(pressMixer, HIGH);

    pinMode(pressLiquid, OUTPUT);
    digitalWrite(pressLiquid, HIGH);

    pinMode(forward, OUTPUT);
    digitalWrite(forward, HIGH);

    pinMode(reverse, OUTPUT);
    digitalWrite(reverse, HIGH);

    pinMode(liquidValve, OUTPUT);
    digitalWrite(liquidValve, HIGH);
    //pinMode(conveyor, OUTPUT); // Reserve Pin
    pinMode(speed, OUTPUT);

    //Pin Initialize Limit Switches
    pinMode(grindPressEndS1, INPUT_PULLUP);
    pinMode(grindPressEndS2, INPUT_PULLUP);
    pinMode(mixingPressEndS1, INPUT_PULLUP);
    pinMode(mixingPressEndS2, INPUT_PULLUP);
    pinMode(grindPosSensor, INPUT_PULLUP);
    pinMode(mixPosSensor, INPUT_PULLUP);
    pinMode(presPosSensor, INPUT_PULLUP);
    pinMode(dumpPosSensor, INPUT_PULLUP);

    pinMode(waterSensor, INPUT_PULLUP);

    //LCD Setup
    lcd.init();
    lcd.createChar(0, enterChar);
    lcd.createChar(1, fastChar);
    lcd.clear();
    lcd.backlight();
    lcd.setCursor(0,0);
    lcd.print("********************");
    lcd.setCursor(0,1);
    lcd.print("*");
    lcd.setCursor(3,1);
    lcd.print("PAPAYA RELISH");
    lcd.setCursor(19,1);
    lcd.print("*");
    lcd.setCursor(0,2);
    lcd.print("*");
    lcd.setCursor(6,2);
    lcd.print("MACHINE");
    lcd.setCursor(19,2);
    lcd.print("*");

    lcd.setCursor(0,3);
    lcd.print("********************");

    //Serial Debug
    Serial.begin(9600);

    Init_Homing_Sequence = true;
    Init_HomingGrindPress = true;
   initialmove = true;

   load_settings();
   //save_setting();
}

void loop()

{
    
    if (Init_Homing_Sequence == true)
    {
        getPressGrindDistance();
    }
    else if(Init_HomingMixerPress == true)
    {
        homeMixPress();

    }
    else if(MoveToHome == true)
    {
        homeConveyor();
    }
    else if(Init_Homing_Sequence == false && Init_HomingMixerPress == false && MoveToHome == false)
    {
        currentMillis = millis();
        readRotaryEncoder(); 
        ClickEncoder::Button b = encoder->getButton();
        if (b != ClickEncoder::Open) { 
        switch (b) { 
                case ClickEncoder::Clicked: 
                    middle=true; 
                break;

                case ClickEncoder::DoubleClicked:
                    Serial.println("ClickEncoder::DoubleClicked");
                    refreshScreen=1;
                    if (testMenuFlag)
                    {
                        if (testRevFor == false)
                        {
                            testRevFor = true;

                        }
                        else
                        {
                            testRevFor = false;
                        }
                    }
                    else if (settingsFlag)
                    {
                        if (fastScroll == false)
                        {
                            fastScroll = true;

                        }
                        else
                        {
                            fastScroll = false;
                        }

                    }
                    Serial.println(fastScroll);
                    
                break;
            
            } 
            } 

            //LCD Change Function and Values
            // To Right Rotary
            if (up == 1)
            {
            up= false;
            refreshScreen=1;
            if (settingsFlag == true)
            {
                if (motorSpeedFlag == true)
                {
                    if(motorEditSpeedFlag == true)
                    {
                        if (parametersMotor[currentmotorSpeed] >= parametersMaxValueMotor[currentmotorSpeed]-1 ){
                            parametersMotor[currentmotorSpeed] = parametersMaxValueMotor[currentmotorSpeed];
                        }else
                        {
                            if(currentSettingScreen == 2){
                                parametersMotor[currentmotorSpeed] += 0.1;
                            }else{

                                if (fastScroll == true)
                                {
                                    parametersMotor[currentmotorSpeed] += 10;
                                }
                                else
                                {
                                    parametersMotor[currentmotorSpeed] += 1;
                                }
                                
                                
                            }
                        }
                    }else{
                        
                        if (currentmotorSpeed == numOfmotorSpeed-1) {
                            currentmotorSpeed = 0;
                        }else{
                            currentmotorSpeed++;
                        }
                    }
                }
                
                else
                {
                    if (settingsEditFlag == true)
                    {
                        if (parameters[currentSettingScreen] >= parametersMaxValue[currentSettingScreen]-1 ){
                            parameters[currentSettingScreen] = parametersMaxValue[currentSettingScreen];
                        }else
                        {
                        if(currentSettingScreen == 2){
                            parameters[currentSettingScreen] += 0.1;
                        }else{
                            parameters[currentSettingScreen] += 1;
                        }
                        }
                    }
                    else
                    {
                        if (currentSettingScreen == numOfSettingScreens-1) {
                            currentSettingScreen = 0;
                        }else{
                            currentSettingScreen++;
                        }
                    }
                }
            }
            else if (testMenuFlag == true)
            {
                if (currentTestMenuScreen == numOfTestMenu-1) {
                    currentTestMenuScreen = 0;
                }else{
                    currentTestMenuScreen++;
                }
            }
            else
            {
                if (currentScreen == numOfMainScreens-1) {
                    currentScreen = 0;
                }else{
                    currentScreen++;
                }
            }
        }

            // To Left Rotary
            if (down == 1)
            {
            down = false;
            refreshScreen=1;
            if (settingsFlag == true)
            {
                if (motorSpeedFlag == true)
                {
                    if(motorEditSpeedFlag == true)
                    {
            
                        if (parametersMotor[currentmotorSpeed] <= 0 ){
                            parametersMotor[currentmotorSpeed] = 0;
                        }else
                        {
                            if(currentSettingScreen == 2){
                                parametersMotor[currentmotorSpeed] -= 0.1;
                            }else{
                                parametersMotor[currentmotorSpeed] -= 1;
                            }
                        }
                    }
                    else
                    {
                        if (currentmotorSpeed == 0) {
                            currentmotorSpeed = numOfmotorSpeed-1;
                        }else{
                            currentmotorSpeed--;
                        }
                    }
                }
                
                else
                {
                    if (settingsEditFlag == true)
                    {

                        if (parameters[currentSettingScreen] <= 0 ){
                            parameters[currentSettingScreen] = 0;
                        }else
                        {
                            if(currentSettingScreen == 2){
                                parameters[currentSettingScreen] -= 0.1;
                            }else{
                                parameters[currentSettingScreen] -= 1;
                            }
                        }
                    }
                    else
                    {
                        if (currentSettingScreen == 0) {
                            currentSettingScreen = numOfSettingScreens-1;
                        }else{
                            currentSettingScreen--;
                        }
                    }
                }
            }
            else if (testMenuFlag == true)
            {
                if (currentTestMenuScreen == 0) {
                    currentTestMenuScreen = numOfTestMenu-1;
                }else{
                    currentTestMenuScreen--;
                }
            }
            else
            {
                if (currentScreen == 0) {
                    currentScreen = numOfMainScreens-1;
                }else{
                    currentScreen--;
                }
            }
        }

        // Rotary Button Press
        if (middle==1)
        {
        middle = false;
        refreshScreen=1;

        if (settingsFlag == true)
        {
            if (currentSettingScreen == 5)
            {
                settingsFlag = false;
                save_setting();
            }
            else if (currentSettingScreen == 4)
            {
                if (currentSettingScreen == 4 && motorSpeedFlag == false)
                {
                    motorSpeedFlag = true;
                }
                else if (currentSettingScreen == 4 && motorSpeedFlag == true && currentmotorSpeed == 5)
                {
                    motorSpeedFlag = false;
                    save_setting();
                }
                else
                {

                    if (motorEditSpeedFlag == true)
                    {
                        motorEditSpeedFlag = false;
                    }
                    else
                    {
                        motorEditSpeedFlag = true;
                    }
                    
                }
                
            }
            else
            {
                if (settingsEditFlag == true)
                {
                    settingsEditFlag = false;
                }
                else
                {
                    settingsEditFlag = true;
                }
            }
        }
        else if (runAutoFlag == true)
        {
            runAutoFlag = false;
            // conveyorFlag = true;
            // grindingFlag = true;
        }
        else if (testMenuFlag == true)
        {
            if (currentTestMenuScreen == 0)
            {
                testConveyorFlag = true; testinitialmove = true;
            }
            else if (currentTestMenuScreen == 1)
            {
                if (testGrind == true)
                {
                    testGrind = false;
                    stopAll();
                }
                else
                {
                    testGrind = true;
                    testConveyorFlag = false; testinitialmove = false;
                    testMixer = false;
                    testPress = false;
                }
                
                
            }
            else if (currentTestMenuScreen == 2)
            {
                if (testMixer == true)
                {
                    testMixer = false;
                    stopAll();
                }
                else
                {
                    testGrind = false;
                    testConveyorFlag = false; testinitialmove = false;
                    testMixer = true;
                    testPress = false;
                }
                
                
            }
            else if (currentTestMenuScreen == 3)
            {
                if (testPress == true)
                {
                    testPress = false;
                    stopAll();
                }
                else
                {
                    testGrind = false;
                    testConveyorFlag = false; testinitialmove = false;
                    testMixer = false;
                    testPress = true;
                }
                
            }
            else
            {
                testMenuFlag = false;
                testGrind = false;
                testConveyorFlag = false; testinitialmove = false;
                testMixer = false;
                testPress = false;
            }
            
        }
        else
        {
            if (currentScreen == 0)
            {
                settingsFlag = true;
            }
            else if (currentScreen == 1)
            {
                runAutoFlag = true;
                 digitalWrite(liquidValve, HIGH);
                
                if(digitalRead(grindPosSensor)== 1){
                    MoveToGrinder = true;
                }

                grindingFlag = true;
                grindPressTickTimer = grindingTime / 15;
                motorStep1.runToNewPosition(0);
                counter = 0;
                moveEveryTime = 0;
                previousMillis = currentMillis;
                previousTime1 = currentTime1;
            }
            else if (currentScreen == 2)
            {
                runManualFlag = true;
            }
            else
            {
                testMenuFlag = true;
            }
            
            }
        }

        if (runAutoFlag == true)
        {
            run();
            
            if (grindingTimerFlag == true || mixingTimerFlag == true)
            {
                refreshScreensEvery1Second();
            }
        }

        if (testConveyorFlag == true)
        {
            testMachineConveyor();
            Serial.println("Test Machine");
        }

        if(testGrind == true || testMixer == true || testPress == true)
        {
            testRunAll();
        }

        getPos();
        //motorSpeedWrite();


        currentMillis = millis();

        if(refreshScreen == 1){
        refreshScreen = 0;
        printScreens();
        }
    } // End Bracket Of Homing Sequence
}


//Save Function
void SaveSettings(){

}

void LoadSettings(){

}


void stopAll(){
    digitalWrite(forward, HIGH);
    digitalWrite(reverse, HIGH);
    digitalWrite(conveyor, HIGH);
    digitalWrite(grinder, HIGH);
    digitalWrite(mixer, HIGH);
    digitalWrite(pressMixer, HIGH);
    digitalWrite(pressLiquid, HIGH);
  //  digitalWrite(grinder, HIGH);
}

//LCD Functions
void printScreens(){
    if (settingsFlag == true)
    {
        if (motorSpeedFlag == false)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.print(settings[currentSettingScreen][0]);
            if(currentSettingScreen == 5){
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                lcd.print("Click to Save All");
            
            }else if(currentSettingScreen == 4)
            {
                lcd.setCursor(0,1);
                lcd.print("Edit Hz and Speed");
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(settingsEditFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }
                lcd.setCursor(19,3);

                if (fastScroll == true)
                {
                    lcd.write(1);
                }
            }
            else
            {
                lcd.setCursor(0,1);
                lcd.print(parameters[currentSettingScreen]);
                lcd.print(" ");
                lcd.print(settings[currentSettingScreen][1]);
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(settingsEditFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }

                lcd.setCursor(19,3);
                if (fastScroll == true)
                {
                    lcd.write(1);
                }
    
                
            }
        }
        else if(motorSpeedFlag == true)
        {
            lcd.begin(20,4);
            lcd.clear();
            lcd.print(motorSpeed[currentmotorSpeed][0]);
            if(currentmotorSpeed == 5){
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                lcd.print("Click to Save All");
            }else{
                lcd.setCursor(0,1);
                lcd.print(parametersMotor[currentmotorSpeed]);
                lcd.setCursor(5,1);
                lcd.print(motorSpeed[currentmotorSpeed][1]);
                lcd.setCursor(0,3);
                lcd.write(0);
                lcd.setCursor(2,3);
                if(motorEditSpeedFlag == false){
                    lcd.print("Click to Edit");   
                }else{
                    lcd.print("Click to Save");  
                }
                lcd.setCursor(19,3);
                if (fastScroll == true)
                {
                    lcd.write(1);
                }
            }
        }
    }
    else if (runAutoFlag == true)
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print("Processing Papaya");
        lcd.setCursor(0,1);
        lcd.print("Current Process");
        
        lcd.setCursor(0,2);
            if (grindingFlag == true)
            {
                lcd.print("Grinding");
            }
            else if(mixingFlag == true)
            {
                lcd.print("Mixing");
            }
            else if(pressFlag == true){
                lcd.print("Pressing");
            }
            else if(dumpFlag == true){
                lcd.print("Dumping");
            }
            else
            {
                lcd.print("Waiting for Commands");
            }
        
        lcd.setCursor(0,3);
            if (grindingFlag == true)
            {
                lcd.print("Time : ");
                if (minRemaining >= 1)
                {
                    lcd.print(minRemaining);
                    lcd.print(" Minute");
                }
                else
                {
                    lcd.print(secRemaining);
                    lcd.print(" Seconds");
                }
            }else if(mixingFlag == true){
                lcd.print("Time : ");
                if (minRemaining >= 1)
                {
                    lcd.print(minRemaining);
                    lcd.print(" Minute");
                }
                else
                {
                    lcd.print(secRemaining);
                    lcd.print(" Seconds");
                }
            }else if(pressFlag == true){
                lcd.print("Time : ");
                if (minRemaining >= 1)
                {
                    lcd.print(minRemaining);
                    lcd.print(" Minute");
                }
                else
                {
                    lcd.print(secRemaining);
                    lcd.print(" Seconds");
                }
            }
        
    }
    else if (testMenuFlag == true)
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(TestMenuScreen[currentTestMenuScreen]);
        if (currentTestMenuScreen == 0)
        {

            lcd.setCursor(0,2);
                if (curPositionConveyor == 1)
                {
                    lcd.print("Grinding Pos");
                }
                else if (curPositionConveyor == 2)
                {
                    lcd.print("Mixing Pos");
                }
                else if (curPositionConveyor == 3)
                {
                    lcd.print("Press Pos");
                }else if(curPositionConveyor == 4)
                {
                    lcd.print("Dumping Pos");
                }
                else if(curPositionConveyor == 0)
                {
                    lcd.print("Unidentified Pos");
                }
        }
        else
        {
            lcd.setCursor(0,1);
            if (testGrind == true )
            {
                lcd.print("Grinder is Running");
            }
            else if (testMixer == true )
            {
                lcd.print("Mixer is Running");
            }
            else if (testPress == true)
            {
                lcd.print("Mixer is Running");
            }else
            {

            }
            
        }
    
        if (currentTestMenuScreen == 4)
        {
            lcd.setCursor(0,3);
            lcd.print("Click to Exit Test");
        }
        else if(currentTestMenuScreen == 0)
        {
            
            lcd.setCursor(0,3);
            if(testConveyorFlag == true){
                lcd.print("Wait Until Finish");
            }else{
                lcd.print("Click to Test");
            }
            
        }else{
            lcd.setCursor(0,3);
             lcd.print("Click to Test");
        }
        
      
    }
    else if (runManualFlag == true)
    {
        /* code */
    }
    else
    {
        lcd.begin(20,4);
        lcd.clear();
        lcd.print(screens[currentScreen][0]);
        lcd.setCursor(0,3);
        lcd.write(0);
        lcd.setCursor(2,3);
        lcd.print(screens[currentScreen][1]);
    }
    
    
}

void refreshScreens(){

}


void run(){
    currentMillis = millis();
    if (grindingFlag == true)
    {
        
        if (grindingPositionFlag == true && MoveToGrinder == false)
        {
            analogWrite(speed, parametersMotor[0] * hz_multiplier);
            grindPapaya();

            if (counter <= 9)
            {
                pushPressGrindEveryTick();
            }
        }

        if (MoveToGrinder == true){
            analogWrite(speed, parametersMotor[4] * hz_multiplier);

            grinderConveyor();
            Serial.println("Test On Grinder Conve");
        }
    }
    else if (mixingFlag == true)
    {
        
        if (mixingPositionFlag == true && MoveToMixer == false)
        {
            analogWrite(speed, parametersMotor[3] * hz_multiplier);
            mixing();
           // Serial.println("Test Run Mixer");
        }

        if (MoveToMixer == true){
            analogWrite(speed, parametersMotor[4] * hz_multiplier);
            mixConveyor();
           // Serial.println("Test On Mixer Conve");
        }
    }
    else if (pressFlag == true){
       
        if (pressingPositionFlag == true && MoveToPress == false)
        {
            pressLiquidRun();
        }
        
        if (MoveToPress == true){
             analogWrite(speed, parametersMotor[4] * hz_multiplier);
            pressingConveyor();
        }
        
        Serial.println("test run press");
        
    }
    else if (dumpFlag == true)
    {
        analogWrite(speed, parametersMotor[4] * hz_multiplier);
        if (dumpPositionFlag == true && MoveToDump == false)
        {
            dumpFlag = false;
            runAutoFlag = false;
        }
        if(MoveToDump == true){
            analogWrite(speed, parametersMotor[4] * hz_multiplier);
            dumpingConveyor();
        }
      
    }
    
}
void testmoveconveyorstop(){
        digitalWrite(conveyor, HIGH);
        digitalWrite(forward, HIGH);
        digitalWrite(reverse, HIGH);
}

void testmoveconveyorFWD(){
        digitalWrite(conveyor, LOW);
        digitalWrite(forward, LOW);
        digitalWrite(reverse, HIGH);
}

void testmoveconveyorRVS(){
        digitalWrite(conveyor, LOW);
        digitalWrite(forward, HIGH);
        digitalWrite(reverse, LOW);
}

void testMachineConveyor(){
    if (testConveyorFlag == true)
    {
        if (testinitialmove == true)
        {
            if (grindingPositionFlag == true)
            {
                testMoveToGrinder = true;
                testinitialmove = false;
            }
            else if(mixingPositionFlag == true)
            {
                testMoveToPress = true;
                testinitialmove = false;
            }
            else if(pressingPositionFlag == true)
            {
                testMoveToDump = true;
                testinitialmove = false;
            }
            else if(dumpPositionFlag == true)
            {
                testMoveToGrinder = true;
                testinitialmove = false;
            }else{
                testMoveToGrinder = true;
                testinitialmove = false;
            }
        }
        else
        {
            if (testMoveToGrinder == true)
            {
                if (grindingPositionFlag == true)
                {
                    testmoveconveyorstop();
                    testMoveToGrinder = false;
                    testConveyorFlag = false;
                }
                else
                {
                    testmoveconveyorRVS();
                }
                
            }
            else if(testMoveToMixer == true)
            {
                if (mixingPositionFlag == true)
                {
                    testmoveconveyorstop();
                    testMoveToMixer = false;
                    testConveyorFlag = false;
                }
                else
                {
                    testmoveconveyorFWD();
                }
            }
            else if(testMoveToPress == true)
            {
                if (pressingPositionFlag == true)
                {
                    testmoveconveyorstop();
                    testMoveToPress = false;
                    testConveyorFlag = false;
                }
                else
                {
                    testmoveconveyorFWD();
                }
            }
            else if(testMoveToDump == true)
            {
                if (dumpPositionFlag == true)
                {
                    testmoveconveyorstop();
                    testMoveToDump = false;
                    testConveyorFlag = false;
                }
                else
                {
                    testmoveconveyorFWD();
                }
            }
        }
        
    }
}





void testRunAll(){
    testGrinderRun();
    testPressLiquidRun();
    testMixerRun();
}

void testGrinderRun(){
     if (testGrind == true)
    {
        digitalWrite(grinder, LOW);
        digitalWrite(forward, LOW);
        

    }
    else
    {
        digitalWrite(grinder, HIGH);
        digitalWrite(forward, HIGH);

    }
}

void testMixerRun(){
     if (testMixer == true)
    {
        digitalWrite(mixer, LOW);
        Serial.println("Mixer Running");
        //digitalWrite(forward, LOW);
   
    }
    else
    {
        digitalWrite(mixer, HIGH);
       // digitalWrite(forward, HIGH);

    }
}

void testPressLiquidRun(){
     if (testPress == true)
    {
        digitalWrite(pressLiquid, LOW);
        Serial.println("Press Running");
        //digitalWrite(forward, LOW);
   
    }
    else
    {
        digitalWrite(pressLiquid, HIGH);
       // digitalWrite(forward, HIGH);

    }
}



void getPos(){
    //Grinder
    if (digitalRead(grindPosSensor) == 0)
    {
        grindingPositionFlag = true;
        curPositionConveyor = 1;
        testRevFor = true;
       // Serial.println("Grinder 1");
    }
    else
    {
        grindingPositionFlag = false;
        //curPositionConveyor = 0;
      //  Serial.println("Grinder 0");
    }
    //Mixer
    if (digitalRead(mixPosSensor) == 0)
    {
        mixingPositionFlag = true;
        curPositionConveyor = 2;
      //  Serial.println("Mixer 1");
    }
    else
    {
        mixingPositionFlag = false;
        //curPositionConveyor = 0;
         // Serial.println("Mixer 0");
    }
    //Press
    if (digitalRead(presPosSensor) == 0)
    {
        pressingPositionFlag = true;
        curPositionConveyor = 3;
        //Serial.println("Press 1");
    }
    else
    {
        pressingPositionFlag = false;
       // curPositionConveyor = 0;
      // Serial.println("Press 0");
    }
    //Dump
    if (digitalRead(dumpPosSensor) == 0)
    {
        dumpPositionFlag = true;
        curPositionConveyor = 4;
         testRevFor = false;
         //Serial.println("Dump 1");
    }
    else
    {
        dumpPositionFlag = false;
       // curPositionConveyor = 0;
       // Serial.println("Dump 0");
    }

    if (grindingPositionFlag == false || mixingPositionFlag == false || pressingPositionFlag == false || dumpPositionFlag == false)
    {
        if(digitalRead(grindPosSensor) != 0 && digitalRead(mixPosSensor) != 0 && digitalRead(presPosSensor) != 0 && digitalRead(dumpPositionFlag) != 0){
            curPositionConveyor = 0;
        }
    }

   // Serial.println(curPositionConveyor);

}
//Function for Writing PWM
void motorSpeedWrite(){
    analogWrite(speed, hz_multiplier);
}




void mixing(){
    if (mixingGoDown == true)
    {
        if (digitalRead(mixingPressEndS2) == 0)
        {
            digitalWrite(mixer, LOW);
            digitalWrite(pressMixer, HIGH);
            digitalWrite(forward, HIGH);
            mixingTimerFlag = true;
            mixingGoDown = false;
            previousMillis = currentMillis;
        }
        else
        {
            analogWrite(speed, parametersMotor[3] * hz_multiplier);
            digitalWrite(mixer, LOW);
            digitalWrite(pressMixer, LOW);
            digitalWrite(forward, LOW);
        }
        
    }
    else if (mixingTimerFlag == true)
    {
        if(currentMillis - previousMillis <= mixingTime){
            Serial.println("Timer on");
            digitalWrite(mixer, LOW);
            secRemaining = (((mixingTime- (currentMillis - previousMillis))/ 1000));
            minRemaining = (((mixingTime- (currentMillis - previousMillis))/ 1000)/60);
            Serial.print("Remaining Time:");
            Serial.println(secRemaining);
        }else{
            mixingGoUp = true;
            mixingTimerFlag = false;
            Serial.println("Timer off");
            refreshScreen = 1;
        }
    }
    else if (mixingGoUp == true)
    {
        if (digitalRead(mixingPressEndS1) == 0)
        {
            digitalWrite(mixer, HIGH);
            digitalWrite(pressMixer, HIGH);
            digitalWrite(reverse, HIGH);

            MoveToPress = true;
            pressFlag = true;
           
            mixingGoUp = false;
            mixingFlag = false;
            Serial.println("Press Ready");

        }
        else
        {
            analogWrite(speed, parametersMotor[3] * hz_multiplier);
            digitalWrite(mixer, LOW);
            digitalWrite(pressMixer, LOW);
            digitalWrite(reverse, LOW);
        }
    }
}

void pressLiquidRun(){
    
    if(digitalRead(waterSensor) == 0){
        digitalWrite(pressLiquid, HIGH);

        digitalWrite(liquidValve, LOW);
        pressFlag = false;
        dumpFlag = true;
        MoveToDump = true;
        Serial.println("Press to dump");
        refreshScreen = 1;
    }else{
        digitalWrite(pressLiquid, LOW);
    }
}

/*
void pressLiquidRun(){
    
    if(currentMillis - previousMillis <= pressingTime){
        digitalWrite(pressLiquid, LOW);
        secRemaining = ((pressingTime- (currentMillis - previousMillis))/ 1000);
        minRemaining = (((pressingTime- (currentMillis - previousMillis))/ 1000)/60);
        Serial.print("Remaining Time:");
        Serial.println(secRemaining);
    }else{
        digitalWrite(pressLiquid, HIGH);
        pressFlag = false;
        dumpFlag = true;
        MoveToDump = true;
        Serial.println("Press to dump");
        refreshScreen = 1;
    }
}

*/

// Function for Process

void grindPapaya(){
    if(currentMillis - previousMillis <= grindingTime){
        // Grinder ON
        grindingTimerFlag = true;
        digitalWrite(forward, LOW);
        digitalWrite(grinder, LOW);
        analogWrite(speed, parametersMotor[0] * hz_multiplier);

        secRemaining = ((grindingTime- (currentMillis - previousMillis))/ 1000);
        minRemaining = (((grindingTime- (currentMillis - previousMillis))/ 1000)/60);
        Serial.print("Remaining Time:");
        Serial.println(secRemaining);

    }else{
        // Grinder ON
        digitalWrite(forward, HIGH);
        digitalWrite(grinder, HIGH);
        refreshScreen = 1;
        mixingFlag = true;
        MoveToMixer = true;
        mixingGoDown = true;
        previousMillis = currentMillis;
        motorStep1.runToNewPosition(0);
        counter = 10;
        saltMovePerBatch = parameters[1] * saltMovePerGram;
        motorStep2.runToNewPosition(saltMovePerBatch);
        motorStep2.setCurrentPosition(0);
        grindingTimerFlag = false;
        grindingFlag = false;
    }  
}


// Function For Conveyor Movement


void homeConveyor(){
    analogWrite(speed, parametersMotor[4] * hz_multiplier);
    if (digitalRead(grindPosSensor) == 0)
    {
        // Reverse Home ON
        digitalWrite(reverse, HIGH);
        digitalWrite(conveyor, HIGH);
        MoveToHome = false;
        Serial.println("Test Run Grinder Conveyor Off");
    }else{
        // Reverse Home OFF
        digitalWrite(reverse, LOW);
        digitalWrite(conveyor, LOW);
    }
}

void grinderConveyor(){
    analogWrite(speed, parametersMotor[4] * hz_multiplier);
    if (digitalRead(grindPosSensor) == 0)
    {
        // Reverse Home ON
        digitalWrite(reverse, HIGH);
        digitalWrite(conveyor, HIGH);
        MoveToGrinder = false;
        grindingTimerFlag = true;
        Serial.println("Test Run Grinder Conveyor Off");
         previousMillis = currentMillis;
    }else{
        // Reverse Home OFF
        digitalWrite(reverse, LOW);
        digitalWrite(conveyor, LOW);
    }
}

void mixConveyor(){
    analogWrite(speed, parametersMotor[4] * hz_multiplier);
    if (digitalRead(mixPosSensor) == 0)
    {
        // Reverse Home ON
        digitalWrite(forward, HIGH);
        digitalWrite(conveyor, HIGH);
        MoveToMixer = false;
        Serial.println("Test Off Mixer Conveyor");
    }else{
        // Reverse Home OFF
        digitalWrite(forward, LOW);
        digitalWrite(conveyor, LOW);
    }
}

void pressingConveyor(){
    analogWrite(speed, parametersMotor[4] * hz_multiplier);
     if (digitalRead(presPosSensor) == 0)
    {
        // Reverse Home ON
        digitalWrite(forward, HIGH);
        digitalWrite(conveyor, HIGH);
        previousMillis = currentMillis;
        MoveToPress = false;
        delay(3000);
    }else{
        // Reverse Home OFF
        digitalWrite(forward, LOW);
        digitalWrite(conveyor, LOW);
    }
}

void dumpingConveyor(){
    analogWrite(speed, parametersMotor[4] * hz_multiplier);
     if (digitalRead(dumpPosSensor) == 0)
    {
        // Reverse Home ON
        digitalWrite(forward, HIGH);
        digitalWrite(conveyor, HIGH);
        MoveToDump = false;
    }else{
        // Reverse Home OFF
        digitalWrite(forward, LOW);
        digitalWrite(conveyor, LOW);
        Serial.println("Press to dump Running");
    }
}

// Function that coordinates conveyor movements
void moveConveyor(){
    if (conveyorFlag == true)
    {
        if (grindingFlag == true)
        {
            homeConveyor(); //Home the conveyor
        }
        else if (mixingFlag == true)
        {
            mixConveyor(); //Move to mixing position
        }
        else if (pressFlag == true)
        {
            pressingConveyor(); // Move to Pressing Position
        }
        else if (dumpFlag == true)
        {
            dumpingConveyor(); // Move to dumping position
        }
    }
}

void homeMixPress(){
    // Serial.println("Test Mix Press");
    if (mixinitialmove == true)
    {
        if (digitalRead(mixingPressEndS1) == 0)
        {
            mixinitialmove = false;
            MoveToHome = true;
            Init_HomingMixerPress =false;
             Serial.println("Mix Press Already homed");
        }
        else if (digitalRead(mixingPressEndS2) == 0)
        {
            mixgoup = true;
            mixinitialmove = false;
            
                Serial.println("Mix Press go Up");
        }
        else
        {
            mixgoup = true;
            mixinitialmove = false;
              Serial.println("Mix Press go Up");
        }
        
    }
    else if (mixgoup == true)
    {
        analogWrite(speed, parametersMotor[3] * hz_multiplier);
        if (digitalRead(mixingPressEndS1) == 0)
        {
            digitalWrite(pressMixer, HIGH);
            digitalWrite(reverse, HIGH);
            Init_HomingMixerPress = false;
            MoveToHome = true;
              Serial.println("Mix Press Already homed");
        }else{
            digitalWrite(pressMixer, LOW);
            digitalWrite(reverse, LOW);
        }
        
    }
}



void getPressGrindDistance(){
    
    if (initialmove == true)
    {
       // Serial.println("Initial Move");
        if (digitalRead(grindPressEndS1) == 0)
        {
            godown = true;
            initialmove = false;
            homethemotor= true;
        }
        else if (digitalRead(grindPressEndS2) == 0)
        {
            goup = true;
            initialmove = false;
            homethemotor= true;
        }
        else
        {
            goup = true;
            homethemotor= true;
            initialmove = false;
 
        }
    }

    if (goup == true && homethemotor== true)
    {
       // Serial.println("Go Up");
        if(digitalRead(grindPressEndS1) == 0){
            motorStep1.setCurrentPosition(0);
            initial_homingStep1 = 1;
            godown = true;
            goup = false;
        }else{
            runUP();
        }
    }
    else if (godown == true && homethemotor== true)
    {
       // Serial.println("Go down");
        if(digitalRead(grindPressEndS2) == 0){
            stepper1Distance = motorStep1.currentPosition() - 500;
            Serial.print("Current Position : ");
            Serial.println(stepper1Distance);
            moveEveryTick = stepper1Distance / 10;
            motorStep1.runToNewPosition(0);
            mixinitialmove = true;
            Init_HomingMixerPress = true;
            godown = false;
            homethemotor= false;
            Init_HomingGrindPress = false;
            Init_Homing_Sequence = false;
            Serial.println("Done Homing Press Grind");
        }else{
            runDOWN();
        }
    }
}



void runUP(){
    motorStep1.moveTo(initial_homingStep1);
    motorStep1.run();
    initial_homingStep1--;
   // Serial.println("Homing -");
    delayMicroseconds(100);
}

void runDOWN(){
    motorStep1.moveTo(initial_homingStep1);
    motorStep1.run();
    initial_homingStep1++;
   // Serial.println("Homing +");
    delayMicroseconds(100);
}




void pressGrinderMove(){
    if(runallowed == true){
        if (motorStep1.currentPosition() >= targetPosition)
        {
            targetPosition += 1000;
        }
        else
        {
            motorStep1.setMaxSpeed(4000); //SPEED = Steps / second
            motorStep1.setAcceleration(50); //ACCELERATION = Steps /(second)^2
            motorStep1.runToNewPosition(targetPosition);
            targetPosition -= 500;
            motorStep1.setMaxSpeed(2000); //SPEED = Steps / second
            motorStep1.setAcceleration(2000); //ACCELERATION = Steps /(second)^2
            motorStep1.runToNewPosition(targetPosition);
        // previousPosition = targetPosition;
        }
    }else{
        return 0;
    }
}