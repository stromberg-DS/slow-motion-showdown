/* 
 * Project: Slow Motion Showdown
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
const int MYWEMO[] = {4, 5, 3, 2, 1};     //outlet numbers - [0] is my testing outlet
const int READYBUTTONPINP1 = A2;
const int READYBUTTONPINP2 = A5;
const int P1BUTTONPIN = A0;
const int P2BUTTONPIN = A1;
const int P1MOTIONPIN = D10;
const int P2MOTIONPIN = D3;
const int AUTOMODEPIN = D17;                //Labeled S2, SCK on the Photon2
const int READYLEDPINS[] = {D7, D6};
const int PLAYERLEDS [] = {D19, D18};
const int OLED_RESET = -1;
const int PIXELCOUNT = 2;
const int WAITING = 0;
const int PLAYING = 1;
const int NOWINNER = 2;
const int WINNER = 3;
const int COUNTINGDOWN = 4;

//set to false and disable manual SYSTEM_MODE if no wifi
const bool isWifiOn = false;
// SYSTEM_MODE(MANUAL);
SYSTEM_MODE(SEMI_AUTOMATIC);    

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


int currentMillis;
int previousMillis;
int gameMode;
int p1Score = 0;
int p2Score = 0;
int noWinTimer = 0;
int countdownStart = 0;


void waitingForPlayers();
void gameOn();
void noWin();
void turnOnOffReadyLEDs(bool onOff);
void showScore();
void countDown();

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

    pixel.begin();
    pixel.setBrightness(30); 
    pixel.setPixelColor(0, 0,255,0);
    pixel.setPixelColor(1, 0,255,0);
    pixel.show();
    delay(3000);
    pixel.clear();
    pixel.show();

    p1OLED.clearDisplay();
    p1OLED.display();

    p2OLED.clearDisplay();
    p2OLED.display();

    setHue(BULBS[0], false, HueGreen, 150, 255);   

    
    gameMode = WAITING;

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

    if (autoModeSwitch.isPressed()){
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
    } else {
        pixel.setPixelColor(0,0,255, 255);
        pixel.setPixelColor(1,0,255, 255);
    }

    pixel.show();

}

void noWin(){   
    if((currentMillis - noWinTimer) < 4000){
        showScore();
        if((currentMillis - noWinTimer) % 500 < 250){   //pulse red lights every 250ms
            pixel.setPixelColor(0,255,0, 0);
            pixel.setPixelColor(1,255,0, 0);
            pixel.show();
        } else{
            pixel.clear();
            pixel.show();
        }

    } else{
        setHue(BULBS[0], false, 0, 0, 0);        //turn off bulb
        gameMode = WAITING;
    }
}

void gameOn(){
    pixel.setPixelColor(0,0,255,0);
    pixel.setPixelColor(1, 0, 255, 0);


    digitalWrite(PLAYERLEDS[0], HIGH);
    digitalWrite(PLAYERLEDS[1], HIGH);


    //End the game if either player triggers their motion sensor.
    if (p1Motion.isClicked()){          
        Serial.printf("P1 LOSER\n");
        p2Score++;
        wemoWrite(MYWEMO[0], LOW);
        setHue(BULBS[0], true, HueRed, 255, 255);     //set bulb red
        noWinTimer = currentMillis;
        gameMode = NOWINNER;
    }

    if (p2Motion.isClicked()){
        Serial.printf("P2 LOSER");
        p1Score++;
        wemoWrite(MYWEMO[0], LOW);
        setHue(BULBS[0], true, HueRed, 255, 255);     //set bulb red
        noWinTimer = currentMillis;
        gameMode = NOWINNER;
    }

    //Meanwhile, wait for each player to press their own button
    if(player1Button.isClicked()){
        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.setTextSize(4);
        p1OLED.printf("BLUE\nWINS!");
        p1OLED.display();
        p2OLED.display();
        p1OLED.setTextSize(2);
        delay(1000);

        p1Score = p1Score +5;

        pixel.setPixelColor(0,0,0,255);
        pixel.setPixelColor(1,0,0,255);
        pixel.show();
        showScore();
        wemoWrite(MYWEMO[0], LOW);
        setHue(BULBS[0], true, HueBlue, 150, 255);        //turn bulb blue
        delay(2000);
        gameMode = WAITING;
        setHue(BULBS[0], false, HueGreen, 150, 255);        //turn bulb off
        
    }
    else if (player2Button.isClicked()) {
        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.setTextSize(4);
        p1OLED.printf("GOLD\nWINS!");
        p1OLED.display();
        p2OLED.display();
        p1OLED.setTextSize(2);
        delay(1000);

        p2Score = p2Score +5;

        pixel.setPixelColor(0,255,255,0);
        pixel.setPixelColor(1,255,255,0);
        pixel.show();
        showScore();
        wemoWrite(MYWEMO[0], LOW);
        setHue(BULBS[0], true, HueYellow, 150, 255);        //turn bulb yellow
        delay(2000);
        gameMode = WAITING;
        setHue(BULBS[0], false, HueGreen, 150, 255);        //turn bulb off
    }

}

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

    pixel.setPixelColor(0,255,0,0);
    pixel.setPixelColor(1, 255, 0, 0);
    

    if (readyButtonP1.isPressed() && readyButtonP2.isPressed()){
        pixel.clear();
        pixel.show();
        turnOnOffReadyLEDs(false);

        countdownStart = currentMillis;
        gameMode = COUNTINGDOWN;        
    }
    pixel.show();
}

void countDown(){
    if((currentMillis - countdownStart) < 5000){
        if((currentMillis - countdownStart) < 2000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(0,0);
            p1OLED.printf("Get ready\nto start\nin...");
            p1OLED.display();
            p2OLED.display();
        } else if((currentMillis - countdownStart) < 3000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.setTextSize(9);
            p1OLED.printf("3");
            p1OLED.display();
            p2OLED.display();
        } else if((currentMillis - countdownStart) < 4000){
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.printf("2");
            p1OLED.display();
            p2OLED.display();
        } else{
            p1OLED.clearDisplay();
            p1OLED.setCursor(40,0);
            p1OLED.printf("1");
            p1OLED.display();
            p2OLED.display();
        }
    } else {
        pixel.setPixelColor(0, 0, 255, 0);
        pixel.setPixelColor(1, 0, 255, 0);
        pixel.show();
        setHue(BULBS[0], false, 0x00FF00, 150, 255);        //turn bulb green
        wemoWrite(MYWEMO[0], HIGH);
        p1OLED.clearDisplay();
        p1OLED.display();
        p2OLED.clearDisplay();
        p2OLED.display();
        gameMode = PLAYING;
    }

}

void turnOnOffReadyLEDs(bool onOff){
    for (int i=0; i < 2; i++){
        digitalWrite(READYLEDPINS[i], onOff);
    }
}

void showScore(){
    p1OLED.setTextSize(2);
    p1OLED.clearDisplay();
    p1OLED.setCursor(0,0);
    p1OLED.printf("Player 1: %i\nPlayer 2: %i", p1Score, p2Score);
    p1OLED.display();
    p2OLED.display();
}