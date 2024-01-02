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

const int readyButtonsPIN = A2;
const int P1BUTTONPIN = A0;
const int P2BUTTONPIN = A1;
const int P1MOTIONPIN = D10;
const int P2MOTIONPIN = D7;
const int OLED_RESET = -1;
const int PIXELCOUNT = 2;
const int WAITING = 0;
const int PLAYING = 1;
const int NOWINNER = 2;
const int WINNER = 3;

SYSTEM_MODE(SEMI_AUTOMATIC);
SYSTEM_THREAD(ENABLED);

Adafruit_SSD1306 display(OLED_RESET);
Adafruit_NeoPixel pixel(PIXELCOUNT, SPI1, WS2812B);
Button readyButtons(readyButtonsPIN);
Button player1Button(P1BUTTONPIN);
Button player2Button(P2BUTTONPIN);
Button p1Motion(P1MOTIONPIN);
Button p2Motion(P2MOTIONPIN);


int currentMillis;
int previousMillis;
int gameMode;

void waitingForPlayers();
void gameOn();
void noWin();

void setup() {
    Serial.begin(9600);
    waitFor(Serial.isConnected, 10000);

    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    display.setTextSize(3);
    display.printf("New\nGame");
    display.display();
    display.setTextSize(2);

    pixel.begin();
    pixel.setBrightness(30); 
    pixel.setPixelColor(0, 0,255,0);
    pixel.setPixelColor(1, 0,255,0);
    pixel.show();
    delay(1000);
    pixel.clear();
    pixel.show();
    display.clearDisplay();
    display.display();
    
    gameMode = WAITING;
    
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
    display.clearDisplay();
    display.setCursor(0,0);
    display.setTextSize(4);
    display.printf("Loser");
    display.display();
    display.setTextSize(2);

    pixel.setPixelColor(0,255,0, 0);
    pixel.setPixelColor(1,255,0, 0);
    pixel.show();
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
    gameMode = WAITING;
}

void gameOn(){
    pixel.setPixelColor(0,0,255,0);
    pixel.setPixelColor(1, 0, 255, 0);

    if (p1Motion.isClicked()){
        // gameMode = NOWINNER;
        Serial.printf("LOSER\n");
        noWin();
        
    }

    if(player1Button.isClicked()){
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(4);
        display.printf("BLUE\nWINS!");
        display.display();
        display.setTextSize(2);

        pixel.setPixelColor(0,0,0,255);
        pixel.setPixelColor(1,0,0,255);
        pixel.show();
        delay(1000);
        gameMode = WAITING;
    }
    else if (player2Button.isClicked()) {
        display.clearDisplay();
        display.setCursor(0,0);
        display.setTextSize(4);
        display.printf("GOLD\nWINS!");
        display.display();
        display.setTextSize(2);

        pixel.setPixelColor(0,255,255,0);
        pixel.setPixelColor(1,255,255,0);
        pixel.show();
        delay(1000);
        gameMode = WAITING;
    }

}

void waitingForPlayers(){
    display.clearDisplay();
    display.setCursor(0,0);
    display.printf("Place both hands on the white buttons");
    display.display();

    pixel.setPixelColor(0,255,0,0);
    pixel.setPixelColor(1, 255, 0, 0);
    

    if (readyButtons.isPressed()){
        pixel.clear();
        pixel.show();

        display.clearDisplay();
        display.setCursor(0,0);
        display.printf("Get ready to start in 3... 2... 1...");
        display.display();

        delay(1000);
        pixel.setPixelColor(0, 0, 255, 0);
        pixel.setPixelColor(1, 0, 255, 0);
        pixel.show();
        display.clearDisplay();
        display.display();
        gameMode = PLAYING;
    }
    pixel.show();
}