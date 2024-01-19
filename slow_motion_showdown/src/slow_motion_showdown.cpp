/* 
 * Project: Slow Motion Showdown
 * Description: 2 Person game where each person tries to press their button first.
 *              The catch is each person has a motion detector aimed at their button.
 *              If they set off their motion detector, they end the round and give their opponent a point.
 *              The game doubles as a smart controller, changing the lights of the
 *              room when a player wins or loses.
 * Author: Daniel Stromberg
 * Date: 12/15/23 
*/

#include "Particle.h"
#include <IoTClassroom_CNM.h>
#include <Encoder.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <neopixel.h>


const int BULBS[] = {5, 3, 1, 2, 4, 6};     //bulb numbers - [0] is my testing bulb
const int MYWEMO[] = {4, 2, 3, 5, 1};     //outlet numbers - [0] is my testing outlet
const int READYBUTTONPINP1 = A2;
const int READYBUTTONPINP2 = A5;
const int P1BUTTONPIN = A1;
const int P2BUTTONPIN = A0;
const int P1MOTIONPIN = D10;
const int P2MOTIONPIN = D3;
const int AUTOMODEPIN = D17;                //Labeled S2, SCK on the Photon2
const int READYLEDPINS[] = {D7, D6};
const int PLAYERLEDS [] = {D19, D18};
const int OLED_RESET = -1;
const int PIXELCOUNT = 15;
const int WAITING = 0;
const int PLAYING = 1;
const int NOWINNER = 2;
const int WINNER = 3;
const int COUNTINGDOWN = 4;
const int ENCODERMAX = 80;


//CHANGE BELOW CONSTANTS DEPENDING ON Setup
const int numBulbsToUse = 1;    //1-6 bulbs
const int numOutletsToUse = 2;  //1-5 outlets
const bool isWifiOn = true;     //set to false and disable manual SYSTEM_MODE if no wifi
SYSTEM_MODE(MANUAL);
// SYSTEM_MODE(SEMI_AUTOMATIC);    

SYSTEM_THREAD(ENABLED);

Adafruit_SSD1306 p1OLED(OLED_RESET);
Adafruit_SSD1306 p2OLED(OLED_RESET);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button readyButtonP1(READYBUTTONPINP1);
Button readyButtonP2(READYBUTTONPINP2);
Button player1Button(P1BUTTONPIN);
Button player2Button(P2BUTTONPIN);
Button p1Motion(P1MOTIONPIN);
Button p2Motion(P2MOTIONPIN);
Button autoModeSwitch(AUTOMODEPIN);
Encoder myEnc(D4, D5);
Button encoderButton(D15);          //also labeled MO, S0


int currentMillis;
int previousMillis;
int gameMode;
int p1Score = 0;
int p2Score = 0;
int noWinTimer = 0;
int countdownStart = 0;

//Maual Mode variables
int position;
int hueManualColor = 0;
int hueManualBrightness = 100;
bool isSettingColor = true;
bool isFirstManualRun = true;
int lastHueUpdate;


void waitingForPlayers();
void gameOn();
void noWin();
void turnOnOffReadyLEDs(bool onOff);
void showScore();
void countDown();
void gameStartup();
void lightUpBulbs(bool _onOff, int _color, int _brightness);
void turnOnOffWemoSwitches(bool _onOff);
void lightLEDStrip( int _color, bool _onOff = true, int _count = PIXELCOUNT);

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected, 10000);


    //Setup Wifi only if we need it.
    if (isWifiOn){
        WiFi.on();
        WiFi.clearCredentials();
        WiFi.setCredentials("IoTNetwork");
        WiFi.connect();
        while (WiFi.connecting())
        {
            Serial.printf("."); 
        }
        Serial.printf("\n\n");
    }

    p1OLED.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    p1OLED.clearDisplay();
    p1OLED.setTextColor(WHITE);
    p1OLED.setTextSize(2);
    p1OLED.display();

    p2OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    p2OLED.clearDisplay();
    p2OLED.setTextColor(WHITE);
    p2OLED.setTextSize(2);
    p2OLED.display();

    // gameStartup();

    position = myEnc.read();

    pixel.begin();
    pixel.setBrightness(30); 
    lightLEDStrip(0x00FFFF);
    // pixel.setPixelColor(0, 0,255,0);
    // pixel.setPixelColor(1, 0,255,0);
    // pixel.show();
    delay(3000);
    pixel.clear();
    pixel.show();

    for (int i=0; i < 2; i++){
        pinMode(READYLEDPINS[i], OUTPUT);
        digitalWrite(READYLEDPINS[i], LOW);
    }

    pinMode(PLAYERLEDS[0], OUTPUT);
    pinMode(PLAYERLEDS[1], OUTPUT);
    digitalWrite(PLAYERLEDS[0], LOW);
    digitalWrite(PLAYERLEDS[1], LOW);

    
}

void loop() {
    currentMillis = millis();
    static String hueOrBrightness = "hue";

    if(autoModeSwitch.isClicked()){         //switch high is game mode
        Serial.printf("switch flipped");
        gameStartup();
        isFirstManualRun = true;
    }

    if (autoModeSwitch.isPressed()){        //AUTO MODE (PLAYING GAME)
        switch (gameMode){
            case WAITING:
                waitingForPlayers();
                break;
            case PLAYING:
                gameOn();
                break;
            case NOWINNER:
                noWin();
                break;
            case COUNTINGDOWN:
                countDown();
        }
    }
    else {                                //MANUAL CONTROL MODE
        position = myEnc.read();
        if(isFirstManualRun){
            digitalWrite(READYLEDPINS[0], LOW);
            digitalWrite(READYLEDPINS[1], LOW);
            p1OLED.clearDisplay();
            p1OLED.setCursor(0,0);
            p1OLED.setTextSize(2);
            // p1OLED.printf("Set hue &\n");
            // p1OLED.printf("intensity\nwith\nencoder.");
            p1OLED.printf("Now\nadjusting\n");
            p1OLED.printf("%s", hueOrBrightness.c_str());
            Serial.printf("Now adjusting: %s\n", hueOrBrightness.c_str());
            p1OLED.display();
            p2OLED.display();

            isFirstManualRun = false;
        }



        if(encoderButton.isClicked()){      
            isSettingColor = !isSettingColor;
            isFirstManualRun = true;
        }

        if (position > ENCODERMAX)    {    //Cap the encoder position at the encoder max
            myEnc.write(ENCODERMAX);
            position = ENCODERMAX;
        }
        else if (position < 0)    {       //Minimum encoder position is 0
            myEnc.write(0);
            position = 0;
        }

        if(isSettingColor){
            hueManualColor = map(position, 0, ENCODERMAX, 0, 50000);
            hueOrBrightness = "hue";
        }
        else {
            hueManualBrightness = map(position, 0, ENCODERMAX, 0, 255);
            hueOrBrightness = "intensity";
        }

        if((currentMillis - lastHueUpdate) > 500){
            // setHue(BULBS[0], true, hueManualColor, hueManualBrightness, 255);
            lightUpBulbs(true, hueManualColor, hueManualBrightness);
            lastHueUpdate = currentMillis;
        }

        lightLEDStrip(0x00FFFF);
        // pixel.setPixelColor(0,0,255, 255);
        // pixel.setPixelColor(1,0,255, 255);
    }

    pixel.show();

}

//  Blinks LEDs red when someone loses
void noWin(){   
    if((currentMillis - noWinTimer) < 4000){
        if((currentMillis - noWinTimer) % 500 < 250){   //pulse red lights every 250ms
            lightLEDStrip(0xFF0000);
        }
        else{
            pixel.clear();
            pixel.show();
        }

    }
    else{
        showScore();
        lightUpBulbs(false, 0, 0);
        // setHue(BULBS[0], false, 0, 0, 0);        //turn off bulb
        gameMode = WAITING;
    }
}

//  Start the round where each player tries to press their button
void gameOn(){

    lightLEDStrip(0x00FF00);
    // pixel.setPixelColor(0,0,255,0);
    // pixel.setPixelColor(1, 0, 255, 0);
    digitalWrite(PLAYERLEDS[0], HIGH);
    digitalWrite(PLAYERLEDS[1], HIGH);


    //End the game if either player triggers their motion sensor.
    if (p1Motion.isClicked()){          
        Serial.printf("P1 LOSER\n");
        p2Score++;
        turnOnOffWemoSwitches(false);
        // wemoWrite(MYWEMO[0], LOW);
        lightUpBulbs(true, HueRed, 255);
        // setHue(BULBS[0], true, HueRed, 255, 255);     //set bulb red
        noWinTimer = currentMillis;
        gameMode = NOWINNER;
    }
    if (p2Motion.isClicked()){
        Serial.printf("P2 LOSER");
        p1Score++;
        turnOnOffWemoSwitches(false);
        // wemoWrite(MYWEMO[0], LOW);
        lightUpBulbs(true, HueRed, 255);
        // setHue(BULBS[0], true, HueRed, 255, 255);     //set bulb red
        noWinTimer = currentMillis;
        gameMode = NOWINNER;
    }

    //Meanwhile, wait for each player to press their own button
    if(player1Button.isClicked()){
        lightLEDStrip(0xFFDD00);
        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.setTextSize(4);
        p1OLED.printf("GOLD\nWINS!");
        p1OLED.display();
        p2OLED.display();
        p1OLED.setTextSize(2);
        delay(1000);

        p1Score = p1Score +5;

        // pixel.setPixelColor(0,0,0,255);
        // pixel.setPixelColor(1,0,0,255);
        // pixel.show();
        showScore();
        turnOnOffWemoSwitches(false);
        // wemoWrite(MYWEMO[0], LOW);
        lightUpBulbs(true, HueYellow, 200);
        // setHue(BULBS[0], true, HueBlue, 150, 255);        //turn bulb blue
        delay(2000);
        gameMode = WAITING;
        lightUpBulbs(false, HueGreen, 150);
        // setHue(BULBS[0], false, HueGreen, 150, 255);        //turn bulb off
        
    }
    else if (player2Button.isClicked()) {
        lightLEDStrip(0x0000FF);
        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.setTextSize(4);
        p1OLED.printf("BLUE\nWINS!");
        p1OLED.display();
        p2OLED.display();
        p1OLED.setTextSize(2);
        delay(1000);

        p2Score = p2Score +5;
        // pixel.setPixelColor(0,255,255,0);
        // pixel.setPixelColor(1,255,255,0);
        // pixel.show();
        showScore();
        turnOnOffWemoSwitches(false);
        // wemoWrite(MYWEMO[0], LOW);
        lightUpBulbs(true, HueBlue, 200);
        // setHue(BULBS[0], true, HueYellow, 150, 255);        //turn bulb yellow
        delay(2000);
        gameMode = WAITING;
        lightUpBulbs(false, HueGreen, 150);
        // setHue(BULBS[0], false, HueGreen, 150, 255);        //turn bulb off
    }

}

//  Wait for players to press ready buttons while instructing them to do so.
void waitingForPlayers(){
    turnOnOffReadyLEDs(true);
    digitalWrite(PLAYERLEDS[0], LOW);
    digitalWrite(PLAYERLEDS[1], LOW);
    p1OLED.clearDisplay();
    p1OLED.setCursor(0,0);
    p1OLED.printf("Place both");
    p1OLED.printf("hands on\nthe white\nbuttons");
    p1OLED.display();
    p2OLED.display();

    lightLEDStrip(0xFF0000);
    // pixel.setPixelColor(0,255,0,0);
    // pixel.setPixelColor(1, 255, 0, 0);
    

    if (readyButtonP1.isPressed() && readyButtonP2.isPressed()){
        pixel.clear();
        pixel.show();
        turnOnOffReadyLEDs(false);

        countdownStart = currentMillis;
        gameMode = COUNTINGDOWN;        
    }
    pixel.show();
}

//  Give both players a countdown to the start of the round.
void countDown(){
    if((currentMillis - countdownStart) < 5000){
        if((currentMillis - countdownStart) < 2000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(0,0);
            p1OLED.printf("Get ready\nto start\nin...");
            p1OLED.display();
            p2OLED.display();
        }
        else if((currentMillis - countdownStart) < 3000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.setTextSize(9);
            p1OLED.printf("3");
            p1OLED.display();
            p2OLED.display();
        }
        else if((currentMillis - countdownStart) < 4000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.printf("2");
            p1OLED.display();
            p2OLED.display();
        }
        else{
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.printf("1");
            p1OLED.display();
            p2OLED.display();
        }
    }
    else {
        lightLEDStrip(0x00FF00);
        // pixel.setPixelColor(0, 0, 255, 0);
        // pixel.setPixelColor(1, 0, 255, 0);
        // pixel.show();
        // lightUpBulbs(false, HueGreen, 150);
        turnOnOffWemoSwitches(true);
        // wemoWrite(MYWEMO[0], HIGH);
        p1OLED.setTextSize(2);
        p1OLED.clearDisplay();
        p1OLED.display();
        p2OLED.setTextSize(2);
        p2OLED.clearDisplay();
        p2OLED.display();
        gameMode = PLAYING;
    }

}

//Turn on or off all of the ready button LEDs
void turnOnOffReadyLEDs(bool onOff){
    for (int i=0; i < 2; i++){
        digitalWrite(READYLEDPINS[i], onOff);
    }
}

//  Display the scores of the 2 players.
void showScore(){
    p1OLED.setTextSize(2);
    p2OLED.setTextSize(2);
    p1OLED.clearDisplay();
    p1OLED.setCursor(0,0);
    p1OLED.printf("Gold: %i\nBlue: %i", p1Score, p2Score);
    p1OLED.display();
    p2OLED.display();
    delay(2000);
}


//  Setup the game, displaying relevant info on OLED screens
void gameStartup(){
    p1Score = 0;
    p2Score = 0;

    p1OLED.clearDisplay();
    p1OLED.setTextColor(WHITE);
    p1OLED.setTextSize(2);
    p1OLED.display();

    p2OLED.clearDisplay();
    p2OLED.setTextColor(WHITE);
    p2OLED.setTextSize(2);
    p2OLED.display();

    p1OLED.setCursor(0, 10);
    p1OLED.printf("Slow\nMotion\nShowdown");
    p1OLED.display();
    p2OLED.display();       //This just displays the same as p1OLED. Using this to my advantage.
    delay(3000);
    
    p1OLED.clearDisplay();
    p1OLED.setCursor(10, 5);
    p1OLED.setTextSize(3);
    p1OLED.printf("Player");
    p1OLED.setCursor(10, 35);
    p1OLED.printf("1-Gold");
    p1OLED.display();
    p1OLED.setTextSize(2);

    p2OLED.clearDisplay();
    p2OLED.setCursor(10, 5);
    p2OLED.setTextSize(3);
    p2OLED.printf("Player");
    p2OLED.setCursor(10, 35);
    p2OLED.printf("2-Blue");
    p2OLED.display();
    p2OLED.setTextSize(2);

    delay(3000);

    p1OLED.clearDisplay();
    p1OLED.display();

    p2OLED.clearDisplay();
    p2OLED.display();

    lightUpBulbs(false, HueGreen, 150);
    // setHue(BULBS[0], false, HueGreen, 150, 255);   

    gameMode = WAITING;
}

//  Turn on or off up all of the Bulbs in the array.
//  Makes it easy to switch between 1 or many bulbs.
void lightUpBulbs(bool _onOff, int _color, int _brightness){
    for(int i=0; i< numBulbsToUse; i++){
        setHue(BULBS[i], _onOff, _color, _brightness, 255);
    }
}

//  Lights up LED strips in the array, using the color and,
//  optionally, the on/off state and number of LEDs to light.
void lightLEDStrip(int _color, bool _onOff, int _count){
    if(_onOff){
        for(int i=0; i<PIXELCOUNT; i++){
            pixel.setPixelColor(i, _color);
        }
        pixel.show();
    }
    else{
        for(int i=0; i<PIXELCOUNT; i++){
            pixel.clear();
        }
        pixel.show();
    }
}

//Turns on or off all of the wemo switches in the array.
void turnOnOffWemoSwitches(bool _onOff){
    for(int i=0; i < numOutletsToUse; i++){
        wemoWrite(MYWEMO[i], _onOff);
    }
}