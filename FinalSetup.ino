//SD, SPI, Wire libs for geniral communication
#include <SPI.h>
#include <Wire.h>
#include <SD.h>
//graphics libs for screen
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ILI9341.h>
#include <Adafruit_STMPE610.h>

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000

#define MINPRESSURE 10
#define MAXPRESSURE 1000

// The display uses hardware SPI, plus #9 & #10
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

// The STMPE610 uses hardware SPI on the shield, and #8
#define STMPE_CS 8
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

//sd card chip select pin # 
#define SD_CS 4
Sd2Card card;
int screen = 0;
boolean sdCard = true;
unsigned int simTime = 30000;//starts with 30 sec sim time
boolean inLoop = true;
void setup(void) {

  Serial.begin(9600);
  Serial.println(F("FINAL OPERATING PROGRAM"));

  tft.begin();
  if(!ts.begin()) {
    Serial.println(F("NO TOUCHSCREEN DATA!!!"));
  }

  if(!SD.begin(SD_CS)) {
    Serial.println(F("SD CARD NOT DETECTED"));
    sdCard = false;
  }

  tft.setRotation(1);

  Serial.print(tft.height());
  Serial.print(" ");
  Serial.println(tft.width());
  Serial.println(analogRead(A5));
}


void loop()
{
  //display Section
  switch(screen) {
  case 0 :
    tft.fillScreen(ILI9341_BLACK);
    tft.setCursor(tft.width()-250, ((tft.height()/2)-5) );
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(5);
    tft.setTextWrap(true);
    tft.println("HRVHS");
    tft.setTextSize(4);
    tft.setCursor(35, ((tft.height()/2) + 40));
    tft.print("Engeneering");
    delay(1000);
    screen = 1;
  break;
  
  case 1 :
    inLoop = true;
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect(tft.width()-50, 10, 40, 40, ILI9341_BLUE);
    tft.drawChar(tft.width()-42, 12, 'O', ILI9341_WHITE, -1, 5); 

    tft.fillRect((tft.width()/2)-50, (tft.height()/2), 100, 50, ILI9341_GREEN);
    tft.setCursor((tft.width()/2)-42, (tft.height()/2)+12);
    tft.setTextSize(3);
    tft.print("START");

    if(!sdCard) {
      tft.drawRect((tft.width()/4), (tft.height()/4), 165, 25, ILI9341_RED);
      tft.setCursor((tft.width()/4) + 6, (tft.height()/4) + 5);
      tft.setTextColor(ILI9341_RED);
      tft.setTextSize(2);
      tft.print("NO SD CARD!!!");
    }

    //loop for touch BLOCKING
    while(inLoop) {
      TS_Point p;
      if(!ts.bufferEmpty()) {
        p = ts.getPoint();
      }
      if(ts.touched()){

        p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
        p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
        int y = tft.height() - p.x;
        int x = p.y;

        if(x < ((tft.width()/2)-50) + 100 && x > (tft.width()/2)-50) {
          if(y > (tft.height()/2) && y < ((tft.height()/2) + 50)) {
            tft.drawRect((tft.width()/2)-50, (tft.height()/2), 100, 50, ILI9341_RED);
            Serial.println("START_PUSHED"); 
            screen = 3;
            inLoop = false;
          }
        }
        if(x > tft.width()-50 && x < tft.width() - 10) {
          if(y > 10 && y < 50) {
            Serial.println("OPTIONS_PUSHED");
            screen = 2;
            inLoop = false;
          }
        }
      }
    }
    break;

  case 2 :
    inLoop = true;
draw_2:
    tft.fillScreen(ILI9341_BLACK);
    tft.fillRect( 20, 10, 80, 40, ILI9341_BLUE);
    tft.setCursor(25, 20);
    tft.setTextColor(ILI9341_WHITE);
    tft.setTextSize(3);
    tft.println("BACK");

    tft.setCursor(20, 60);
    tft.setTextSize(2);
    tft.println("TIMER SET");

    tft.fillTriangle(150, 65, 165, 55, 165, 75, ILI9341_BLUE);
    tft.setCursor(175, 60);
    tft.println((simTime/30000));
    tft.fillTriangle(245, 65, 230, 55, 230, 75, ILI9341_BLUE);

    while(inLoop) {
      TS_Point p;
      if(!ts.bufferEmpty()) {
        p = ts.getPoint();
      }

      if(ts.touched()){

        p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
        p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
        int y = tft.height() - p.x;
        int x = p.y;

        if(x > 20 && x < 100) {
          if(y > 10 && y < 50) {
            Serial.println("BACK_PUSHED");
            screen = 1;
            inLoop = false;
          }
        }

        if(x > 150 && x < 165) {
          if(y > 55 && y < 75) {
            Serial.println("DOWN_ARROW_PUSHED");
            simTime-=30000;
            goto draw_2;
          }
        }

        if(x > 230 && x < 245) {
          if(y > 55 && y < 75) {
            Serial.println("UP_ARROW_PUSHED");
            simTime+=30000;
            goto draw_2;
          }
        }
      }
    }
    break;

  case 3:
    {
      unsigned long prevTime = millis();
      unsigned long prevTime_2 = millis();
      unsigned long prevTime_3 = millis();
      unsigned long prevTime_4 = millis();

      short inter = 30;
      inLoop = true;
draw_3:
      tft.fillScreen(ILI9341_BLACK);
      tft.fillRect( 20, 10, 80, 40, ILI9341_BLUE);
      tft.setCursor(25, 20);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.println("BACK");

      //DRAW MAIN TEXT

      tft.setCursor(30, 90);
      tft.setTextColor(ILI9341_RED);
      tft.setTextSize(4);
      tft.println("RECORDING...");
      tft.setTextColor(ILI9341_WHITE);

      //DRAW LOADING BAR
      tft.drawRect(30, 130, 210, 20, ILI9341_GREEN);

      unsigned int nLines = simTime/1000;

      int pos = 0;
      byte barPos = 20;
      int sensVal_2 = analogRead(A5);
      int storedVal[nLines + 10][2];
      int storedInc = 0;
      boolean done = true;

      //****main touch loop****
      while(inLoop) {
        int sensVal = analogRead(A5);
        TS_Point p = ts.getPoint();


        unsigned int dispTime = millis() - prevTime;
        unsigned int storedTime = millis() - prevTime_2;
        unsigned int compTime = millis() - prevTime_3;
        unsigned int timeFinal = millis() - prevTime_4;

        if(pos < 21) {
          if(dispTime >= (simTime/20)) {
            prevTime = millis();

            tft.fillRect((barPos+=10), 130, 10, 20, ILI9341_GREEN);
            pos++;
          }
        }

        if(pos >= 21) {
          inLoop = false;
        }
        //TODO: at each COMPRESSION measure the depth and time

          /*if(compTime >= 1000) {  //Slow down the compression check to onece every half second
          prevTime_3 = millis();
          if(sensVal < sensVal_2) {
            if(done) {
              int j = 0;
              storedVal[storedInc][j] = storedTime;
              prevTime_2 = millis();
              Serial.println(storedVal[storedInc][j]);
              j++;
              storedVal[storedInc][j] = sensVal_2;
              Serial.println(storedVal[storedInc][j]);
              storedInc++;
              done = false;
            }
          }
          else{
            sensVal_2 = sensVal;
            done = true;
          }
        }*/

        if(ts.touched()){
          p.x = map(p.x, TS_MINY, TS_MAXY, 0, tft.height());
          p.y = map(p.y, TS_MINX, TS_MAXX, 0, tft.width());
          int y = tft.height() - p.x;
          int x = p.y;

          if(x > 20 && x < 100) {
            if(y > 10 && y < 50) {
              Serial.println("BACK_PUSHED");
              screen = 1;
              inLoop = false;
            }
          }
        }
      }
      inLoop = true;

      tft.fillScreen(ILI9341_BLACK);
      tft.fillRect( 20, 10, 80, 40, ILI9341_BLUE);
      tft.setCursor(25, 20);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(3);
      tft.println("BACK");
      //display main text
      tft.setCursor(30, 90);
      tft.setTextColor(ILI9341_RED);
      tft.setTextSize(4);
      tft.println("WORKING...");
      tft.setTextColor(ILI9341_WHITE);
      File fi = SD.open("Storefile.csv", FILE_WRITE);

      while(inLoop) {
        for(int i = 0; i < (nLines + 10); i++) {
          for(int j = 0; j < 1; j++) {
            fi.print(storedVal[i][j], DEC);
            fi.write(",");
          }
          fi.println();
        }
        
        fi.close();
      }
    }
    break;
  }
}









