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


const int BULBS[] = {5, 3, 1, 2, 4, 6};
const int MYWEMO[] = {4, 5, 3, 2, 1};     //outlet number
const int READYBUTTONSPIN = A2;
const int P1BUTTONPIN = A0;
const int P2BUTTONPIN = A1;
const int P1MOTIONPIN = D10;
const int P2MOTIONPIN = D3;
const int READYLEDPINS[] = {D7, D6, D5, D4};
const int PLAYERLEDS [] = {D19, D18};
const int OLED_RESET = -1;
const int PIXELCOUNT = 2;
const int WAITING = 0;
const int PLAYING = 1;
const int NOWINNER = 2;
const int WINNER = 3;

// SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_MODE(MANUAL);
SYSTEM_THREAD(ENABLED);

Adafruit_SSD1306 p1OLED(OLED_RESET);
Adafruit_SSD1306 p2OLED(OLED_RESET);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button readyButtons(READYBUTTONSPIN);
Button player1Button(P1BUTTONPIN);
Button player2Button(P2BUTTONPIN);
Button p1Motion(P1MOTIONPIN);
Button p2Motion(P2MOTIONPIN);


int currentMillis;
int previousMillis;
int gameMode;
int p1Score = 0;
int p2Score = 0;

void waitingForPlayers();
void gameOn();
void noWin();
void turnOnOffReadyLEDs(bool onOff);
void showScore();

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected, 10000);

    //comment out below if turnin off wifi
    WiFi.on();
    WiFi.clearCredentials();
    WiFi.setCredentials("IoTNetwork");
    WiFi.connect();
    while (WiFi.connecting())
    {
        Serial.printf(".");
    }
    Serial.printf("\n\n");

    p1OLED.begin(SSD1306_SWITCHCAPVCC, 0x3D);
    p1OLED.clearDisplay();
    p1OLED.setTextColor(WHITE);
    p2OLED.display();

    p2OLED.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    p2OLED.clearDisplay();
    p2OLED.setTextColor(WHITE);
    p2OLED.display();



    p1OLED.setCursor(40, 5);
    p1OLED.setTextSize(3);
    p1OLED.printf("p1\n");
    p1OLED.setCursor(30, 35);
    p1OLED.printf("Gold");
    p1OLED.display();
    p1OLED.setTextSize(2);

    p2OLED.clearDisplay();
    p2OLED.setCursor(40, 5);
    p2OLED.setTextSize(3);
    p2OLED.printf("p2\n");
    p2OLED.setCursor(30, 35);
    p2OLED.printf("Blue");
    p2OLED.display();
    p2OLED.setTextSize(2);

    pixel.begin();
    pixel.setBrightness(30); 
    pixel.setPixelColor(0, 0,255,0);
    pixel.setPixelColor(1, 0,255,0);
    pixel.show();
    delay(10000);
    pixel.clear();
    pixel.show();

    p1OLED.clearDisplay();
    p1OLED.display();

    p2OLED.clearDisplay();
    p2OLED.display();

    setHue(BULBS[0], false, HueGreen, 150, 255);        //turn bulb yellow

    
    gameMode = WAITING;

    for (int i=0; i < 4; i++){
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
    }

    pixel.show();

}

void noWin(){
    // p1OLED.clearDisplay();
    // p1OLED.setCursor(5,15);
    // p1OLED.setTextSize(4);
    // p1OLED.printf("Loser");
    // p1OLED.display();
    // p1OLED.setTextSize(2);

    pixel.setPixelColor(0,255,0, 0);
    pixel.setPixelColor(1,255,0, 0);
    pixel.show();
    setHue(BULBS[0], true, HueRed, 255, 255);     //set bulb red

    delay(500);

    pixel.clear();
    pixel.show();
    delay(500);

    pixel.setPixelColor(0,255,0, 0);
    pixel.setPixelColor(1,255,0, 0);
    pixel.show();
    delay(500);

    pixel.clear();
    pixel.show();
    delay(500);
    setHue(BULBS[0], false, 0, 0, 0);        //turn off bulb
    gameMode = WAITING;
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
        showScore();
        wemoWrite(MYWEMO[0], LOW);
        noWin();
    }

    if (p2Motion.isClicked()){
        Serial.printf("P2 LOSER");
        p1Score++;
        showScore();
        wemoWrite(MYWEMO[0], LOW);
        noWin();
    }

    //Meanwhile, wait for each player to press their own button
    if(player1Button.isClicked()){
        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.setTextSize(4);
        p1OLED.printf("BLUE\nWINS!");
        p1OLED.display();
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
    p1OLED.printf("Place both hands on the white buttons");
    p1OLED.display();

    pixel.setPixelColor(0,255,0,0);
    pixel.setPixelColor(1, 255, 0, 0);
    

    if (readyButtons.isPressed()){
        pixel.clear();
        pixel.show();
        turnOnOffReadyLEDs(false);

        p1OLED.clearDisplay();
        p1OLED.setCursor(0,0);
        p1OLED.printf("Get ready to start in 3... 2... 1...");
        p1OLED.display();

        delay(1000);
        pixel.setPixelColor(0, 0, 255, 0);
        pixel.setPixelColor(1, 0, 255, 0);
        pixel.show();
        setHue(BULBS[0], false, 0x00FF00, 150, 255);        //turn bulb green
        wemoWrite(MYWEMO[0], HIGH);
        p1OLED.clearDisplay();
        p1OLED.display();
        gameMode = PLAYING;
    }
    pixel.show();
}

void turnOnOffReadyLEDs(bool onOff){
    for (int i=0; i < 4; i++){
        digitalWrite(READYLEDPINS[i], onOff);
    }
}

void showScore(){
    p1OLED.setTextSize(2);
    p1OLED.clearDisplay();
    p1OLED.setCursor(0,0);
    p1OLED.printf("Player 1: %i\nPlayer 2: %i", p1Score, p2Score);
    p1OLED.display();
}