#include <Wire.h>
#include "FastLED.h"
#include "ThunderCloudEnums.h"

// How many leds in your strip?
#define NUM_LEDS 150
#define DATA_PIN 6

ColorMode colorMode = HUE;
Mode mode = ON;
Mode lastMode = OFF;
TempestMode tempestMode = WAITING;
// Define the array of leds
CRGB leds[NUM_LEDS];

unsigned int fade_hue[NUM_LEDS];
unsigned int fade_sat[NUM_LEDS];
unsigned int fade_dir = 1;
unsigned int col1=180;
unsigned int col2=185;

// Parameters for a real thunder simulation
unsigned long  _1mn = 60000L;
unsigned long  _2mn = 60000L*2;
unsigned long  _3mn = 60000L*3;
unsigned long  _4mn = 60000L*4;
unsigned long  _5mn = 60000L*5;
unsigned long  _6mn = 60000L*6;
unsigned long  _7mn = 60000L*7;
unsigned long  _8mn = 60000L*8;
unsigned long  _9mn = 60000L*9;
unsigned long  _10mn = 60000L*10;
unsigned long  _11mn = 60000L*11;
unsigned long  _12mn = 60000L*12;
unsigned long  _13mn = 60000L*13;
unsigned long  _14mn = 60000L*14;
unsigned long  _15mn = 60000L*15;


unsigned long STARTING_DURATION = _4mn;
unsigned long GROWING_DURATION = _2mn;
unsigned long TEMPEST_1_S1_DURATION = _1mn;
unsigned long TEMPEST_1_S2_DURATION = _1mn;
unsigned long CALM_S1_DURATION = _2mn;
unsigned long CALM_S2_DURATION = _1mn;
unsigned long TEMPEST_2_S1_DURATION = _1mn;
unsigned long TEMPEST_2_S2_DURATION = _1mn;
unsigned long REDUCING_DURATION = _2mn;
unsigned long ENDING_DURATION = _4mn;

EasingType STARTING_EFFECT = LINEAR;
EasingType GROWING_EFFECT = INSINE;
EasingType TEMPEST_1_S1_EFFECT = OUTCUBIC;
EasingType TEMPEST_1_S2_EFFECT = OUTCUBIC;
EasingType CALM_S1_EFFECT = INSINE;
EasingType CALM_S2_EFFECT = INSINE;
EasingType TEMPEST_2_S1_EFFECT = INCUBIC;
EasingType TEMPEST_2_S2_EFFECT = INQUINT;
EasingType REDUCING_EFFECT = INCUBIC;
EasingType ENDING_EFFECT = LINEAR;

unsigned long STARTING_END = 0L + STARTING_DURATION;
unsigned long GROWING_END = STARTING_END + GROWING_DURATION;
unsigned long TEMPEST_1_S1_END = GROWING_END + TEMPEST_1_S1_DURATION;
unsigned long TEMPEST_1_S2_END = TEMPEST_1_S1_END + TEMPEST_1_S2_DURATION;
unsigned long CALM_S1_END = TEMPEST_1_S2_END + CALM_S1_DURATION;
unsigned long CALM_S2_END = CALM_S1_END + CALM_S2_DURATION;
unsigned long TEMPEST_2_S1_END = CALM_S2_END + TEMPEST_2_S1_DURATION;
unsigned long TEMPEST_2_S2_END = TEMPEST_2_S1_END + TEMPEST_2_S2_DURATION;
unsigned long REDUCING_END = TEMPEST_2_S2_END + REDUCING_DURATION;
unsigned long ENDING_END = REDUCING_END + ENDING_DURATION;


unsigned long  loopTime = 14400000L;//(4H) || 21600000L//(6H) || 86400000(24H)
unsigned long  currentTime = 0L;
unsigned long  previousMillis = 0L;

// Global variable for quick thunder method
unsigned int _hue = 255;
unsigned int _saturation = 255;
unsigned int _intensity = 255;
unsigned int _thunderDelayMin = 10;
unsigned int _thunderDelayMax = 500;
unsigned int _crackDelayMin = 50;
unsigned int _crackDelayMax = 250;
unsigned int _rollingMaxHits = 10;

// Used for make a nice fading without delay
unsigned long hueFadeDelay = 160000L;
unsigned long satFadeDelay = 180000L;
unsigned long valFadeDelay = 200000L;
boolean hueFading = false;
boolean satFading = false;

void setup() {
    delay(2000);
    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
    Serial.begin(115200);
    Wire.begin(9); // Start I2C Bus as a Slave (Device Number 9)
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
    // Here, we set the mode based on the IR signal received. Check the debug log when you press a button on your remote, and  add the hex code here (you need 0x prior to each command to indicate it's a hex value)
    while(Wire.available()){
        delay(5);
        int received = Wire.read();
        Serial.print(F("Receiving IR hex: "));
        Serial.println(received,HEX);
        lastMode = mode;
        
        if(mode!=TEMPEST){
           hueFading = false;
           satFading = false; 
        }  
        
        switch(received){
           case 0xFF:
                setLastMode();
                if(colorMode == HUE){
                   _hue = increasingValue(_hue, 8, 255);
                } else if(colorMode == SATURATION){
                   _saturation = increasingValue(_saturation, 8, 255);
                } else if(colorMode == VALUE){
                   _intensity = increasingValue(_intensity, 8, 255);
                }
                break;  
           case 0x7F:
                 setLastMode();
                 if(colorMode == HUE){                   
                     _hue = decreasingValue(_hue, 8, 16);
                 } else if(colorMode == SATURATION){
                     _saturation = decreasingValue(_saturation, 8, 16);
                 } else if(colorMode == VALUE){
                     _intensity = decreasingValue(_intensity, 8, 16);
                 }
                 break;  
           case 0xBF:
                mode = OFF; break;
            case 0x3F:
                mode = ON; break;
           case 0x1F:
                if(colorMode==HUE){     
                    colorMode = SATURATION;    
                } else if(colorMode==SATURATION){      
                    colorMode = VALUE;   
                } else if(colorMode==VALUE){      
                    colorMode = HUE;   
                }
                Serial.print("colorMode : ");
                Serial.println(colorMode);
                break;
            case 0x2F:
                mode = THUNDER; break;
            case 0xF:
                mode = ACID; break;
            case 0x37:
               if(mode!=FADE1 && mode!=FADE2 && mode!=FADE3) {
                  Serial.print(F("SWITCH FADE MODE 1 !"));
                  mode = FADE1;
                } else if(mode==FADE1){
                  Serial.print(F("SWITCH FADE MODE 2 !"));
                  for(unsigned int i=0;i<NUM_LEDS;i++){
                    fade_hue[i]=random(128,255);
                    fade_sat[i]=random(128,255);
                    if(i & 1 == 1){ 
                       col1=random(1,255);
                    }else{
                       col2=random(185,255);  
                    }
                  }
                  mode = FADE2;
                } else if(mode==FADE2){
                  Serial.print(F("SWITCH FADE MODE 3 !"));
                  for(unsigned int i=0;i<NUM_LEDS;i++){
                    fade_hue[i]=random(0,255);
                    fade_sat[i]=random(128,255);
                    col1=random(90,255);
                  }
                  mode = FADE3;
                } else if (mode==FADE3) {
                  mode = FADE1;
                }  
                break;
            case 0x17:
                if(mode!=TEMPEST){
                    reset();
                    _saturation = 0;
                    _hue = 0;
                    tempestMode = WAITING;
                    previousMillis = millis();
                    currentTime = 0L;
                    mode = TEMPEST;
                } else {
                    if(hueFading && satFading){
                         Serial.print(F("hueFading and satFading OFF "));
                         satFading=false;
                         hueFading=false;
                         _saturation = 0;
                         _hue=0; 
                    } else{
                       if(hueFading && !satFading){
                         Serial.print(F("satFading ON"));
                         satFading = true;
                      }
                      if(!hueFading){
                           Serial.print(F("hueFading ON "));
                           _saturation = 64;
                           hueFading = true;
                      }
                    }
                }
                break;
            case 0xDF:
                mode = RED; 
                _hue = 0;
                _saturation=255;
                _intensity = 255;
                break;
            case 0x5F:
                mode = GREEN; 
                _hue = 96;
                _saturation=255;
                _intensity = 255;
                break;
            case 0x9F:
                mode = BLUE; 
                _hue = 160;
                _saturation=255;
                _intensity = 255;
                break;
            case 0xEF:
                mode = RED1; 
                _hue = 45;
                _saturation = 210;
                _intensity = 255;
                break;
            case 0x6F:
                mode = GREEN1; 
                _hue = 132;
                _saturation = 210;
                _intensity = 255;
                break;
            case 0xAF:
                mode = BLUE1; 
                _hue = 200;
                _saturation = 210;
                _intensity = 255;
                break;
            case 0xCF:
                mode = RED2; 
                _hue = 180;
                _saturation = 210;
                _intensity = 255;
                break;
             case 0x4F:
                mode = GREEN2; 
                _hue = 160;
                _saturation = 210;
                _intensity = 255;
                break;
            case 0x8F:
                mode = BLUE2; 
                _hue = 140;
                _saturation = 210;
                _intensity = 255;
                break;    
            case 0xF7:
                mode = RED3; 
                _hue = 120;
                _saturation = 210;
                _intensity = 255;
                break;
            case 0x77:
                mode = GREEN3; 
                _hue = 100;
                _saturation = 210;
                _intensity = 255;
                break;
             case 0xB7:
                mode = BLUE3; 
                _hue = 80;
                _saturation = 210;
                _intensity = 255;
                break;
              case 0xD7:
                mode = RED4; 
                _hue = 60;
                _saturation = 210;
                _intensity = 255;
                break;  
              case 0x57:
                mode = GREEN4; 
                _hue = 40;
                _saturation = 210;
                _intensity = 255;
                break; 
              case 0x97:
                mode = BLUE4; 
                _hue = 20;
                _saturation = 210;
                _intensity = 255;
                break; 
        }
    }
}

void loop() {
    switch(mode){
        case OFF: reset();break;
        case ON: _on();break;
        case THUNDER: thunder(_intensity, _thunderDelayMin, _thunderDelayMax, _crackDelayMin, _crackDelayMax, _rollingMaxHits);break;
        case ACID: acid_cloud();break;
        case FADE1: colour_fade1();break;
        case FADE2: colour_fade2();break;
        case FADE3: colour_fade3();break;
        case TEMPEST: _tempest();break;
        case WHITE: single_colour();break;
        case RED: single_colour();break;
        case GREEN: single_colour();break;
        case BLUE: single_colour();break;
        case RED1: single_colour();break;
        case GREEN1: single_colour();break;
        case BLUE1: single_colour();break;
        case RED2: double_colour();break;
        case GREEN2: double_colour();break;
        case BLUE2: double_colour();break;
        case RED3: double_colour();break;
        case GREEN3: double_colour();break;
        case BLUE3: double_colour();break;
        case RED4: double_colour();break;
        case GREEN4: double_colour();break;
        case BLUE4: double_colour();break;
    }
}

void setLastMode(){
   if(mode!=TEMPEST){
     lastMode=OFF;
   }
}

void _on(){
   if(lastMode != mode){
       _hue = 0;
       _saturation = 0;
       _intensity = 255;
        lastMode = mode;
        switchOff();
        for(unsigned int i = 0; i < NUM_LEDS; i++) {
           leds[i] = CHSV(200, 100, 120);
           FastLED.show();
           delay(10);
        }
    }
    _hue = 200;
    _saturation = 100;
    _intensity = 120;
    delay(50);
    mode = WHITE;
}

void single_colour(){
    for (unsigned int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV( _hue, _saturation, _intensity);
    }
    if(lastMode != mode){
        FastLED.show();
        lastMode = mode;
    }
    delay(50);
}

void double_colour(){ 
    if(lastMode != mode){
        unsigned int m = NUM_LEDS/2;
        for (unsigned int i=0;i<75;i++){
          leds[i] = CHSV( _hue, _saturation, _intensity);
          delay(30);
          FastLED.show();
        }
        lastMode = mode;
    }
    delay(50);
}


void colour_fade1(){
    Serial.print(F("colour fade 1 : "));
    unsigned int hue=128+127*cos(2*PI/hueFadeDelay*millis());
    unsigned int sat=128+127*cos(2*PI/satFadeDelay*millis());
    unsigned int col=128+127*cos(2*PI/valFadeDelay*millis());
    if(hue<=64){hue=64;}
    if(sat<=64){sat=64;}
    Serial.print(F("hue : "));
    Serial.print(hue);
    Serial.print(F(" - sat : "));
    Serial.print(sat);
    Serial.print(F(" - col : "));
    Serial.println(col);
    for (unsigned int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV(hue, sat, col);
    }
    FastLED.show();
    delay(50);
}

void colour_fade2(){
    Serial.println(F("colour fade 2 : "));
    for(unsigned int i=0;i<NUM_LEDS;i++){
        fade_hue[i]=fade_hue[i]+1;
        fade_sat[i]=fade_sat[i]+1;
        unsigned int j = 1;
        if(i & 1 == 1){
          col1=col1+1;
          j=col1;
        }else{
          col2=col2+1;
          j=col2;
        }
        if(col1>255){
          col1=1;
        }
        if(col2>255){
          col2=1;
        }
        if(fade_hue[i]>255){
          fade_hue[i]=128;
        }
        if(fade_sat[i]>255){
          fade_sat[i]=128;
        }
        Serial.print(F("hue : "));
        Serial.print(fade_hue[i]);
        Serial.print(F(" - sat : "));
        Serial.print(fade_sat[i]);
        Serial.print(F(" - col : "));
        Serial.println(j);
        leds[i] = CHSV(fade_hue[i], fade_sat[i], j);
        FastLED.show();
        delay(500);
    }
}

void colour_fade3(){
    Serial.println(F("colour fade 3 : "));
    unsigned int r = random(50,255);
    if(fade_dir==1 && (r>=col1-10 && r<=col1+10)){
      col1=col1+1;
    }
    if(fade_dir==0 && (r>=col1-10 && r<=col1+10)){
      col1=col1-1; 
    }
    if(col1>=255){
      col1=255;
      fade_dir=0;
    }
    if(col1<=70){
      col1=70;
      fade_dir=1;
    }
    for(unsigned int i=0;i<NUM_LEDS;i++) {
        fade_hue[i]=fade_hue[i]+1;
        fade_sat[i]=fade_sat[i]+1;
        if(fade_hue[i]>255){
          fade_hue[i]=0;
        }
        if(fade_sat[i]>255){
          fade_sat[i]=128;
        }
        if(i==1) {
          Serial.print(F("hue : "));
          Serial.print(fade_hue[i]);
          Serial.print(F(" - sat : "));
          Serial.print(fade_sat[i]);
          Serial.print(F(" - col : "));
          Serial.println(col1);
        }
        leds[i] = CHSV(fade_hue[i], fade_sat[i], col1);
    }
    delay(5);
    FastLED.show();
}

void acid_cloud(){
    for(unsigned int i=0;i<NUM_LEDS;i++){
        if(random(0,100)>90){
            leds[i] = CHSV( random(0,255), _saturation, _intensity);
        }else{
            leds[i] = CHSV(0,0,0);
        }
    }
    FastLED.show();
    delay(random(5,100));
    reset();
}

void _tempest(){
  Serial.print(F("Tempest  : "));
   if(millis() < previousMillis){ // Reinit if millis() reach the 50 days
        previousMillis = 0L;
    }
    if(currentTime >= loopTime){
        currentTime = 0L;
        previousMillis = previousMillis + loopTime;
    } else {
        currentTime = millis() - previousMillis;
    }
    if(tempestMode != WAITING){
      Serial.print(F("CurrentTime : "));
      logTime(currentTime);
      if(hueFading){
        Serial.println("hueFading change ");
        _hue=128+127*cos(2*PI/(hueFadeDelay/10)*millis());
      }
      if(satFading){
        Serial.println("satFading change ");
        _saturation=128+127*cos(2*PI/(satFadeDelay/10)*millis());
        if(_saturation<=90){_saturation=90;}
        if(_saturation>=220){_saturation=220;}
      }
      Serial.println("");
      Serial.print(F("_hue : "));
      Serial.print(_hue);
      Serial.print(F(" - _sat : "));
      Serial.println(_saturation);
    }  
    
  
    unsigned int intensity = 255;
    unsigned int thunderDelayMin = 10;
    unsigned int thunderDelayMax = 500;
    unsigned int crackDelayMin = 50;
    unsigned int crackDelayMax = 250;

    if(currentTime >= 0L && currentTime <= STARTING_END && tempestMode != STARTING){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ STARTING ---------------------------------------------"));
        tempestMode = STARTING;
    } else if(currentTime > STARTING_END && currentTime <= GROWING_END && tempestMode != GROWING){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ GROWING ----------------------------------------------"));
        tempestMode = GROWING;
    } else if(currentTime > GROWING_END && currentTime <= TEMPEST_1_S1_END && tempestMode != TEMPEST_1_S1){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ TEMPEST 1 STEP 1 -------------------------------------"));
        tempestMode = TEMPEST_1_S1;
    }  else if(currentTime > TEMPEST_1_S1_END && currentTime <= TEMPEST_1_S2_END && tempestMode != TEMPEST_1_S2){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ TEMPEST 1 STEP 2 -------------------------------------"));
        tempestMode = TEMPEST_1_S2;
    } else if(currentTime > TEMPEST_1_S2_END && currentTime <= CALM_S1_END && tempestMode != CALM_S1){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ CALM - STARTING --------------------------------------"));
        tempestMode = CALM_S1;
    } else if(currentTime > CALM_S1_END && currentTime <= CALM_S2_END && tempestMode != CALM_S2){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ CALM - ENDING ----------------------------------------"));
        tempestMode = CALM_S2;
    } else if(currentTime > CALM_S2_END && currentTime <= TEMPEST_2_S1_END && tempestMode != TEMPEST_2_S1){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ TEMPEST 2 STEP 1 -------------------------------------"));
        tempestMode = TEMPEST_2_S1;
    } else if(currentTime > TEMPEST_2_S1_END && currentTime <= TEMPEST_2_S2_END && tempestMode != TEMPEST_2_S2){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ TEMPEST 2 STEP 2 -------------------------------------"));
        tempestMode = TEMPEST_2_S2;
    } else if(currentTime > TEMPEST_2_S2_END && currentTime <= REDUCING_END && tempestMode != REDUCING){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ REDUCING ---------------------------------------------"));
        tempestMode = REDUCING;
    } else if(currentTime > REDUCING_END && currentTime <= ENDING_END && tempestMode != ENDING){
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ ENDING -----------------------------------------------"));
        tempestMode = ENDING;
    } else if (currentTime > ENDING_END && tempestMode != WAITING) {
        Serial.println(F(""));
        Serial.println(F("------------------------------------------ WAITING... -------------------------------------------"));
        tempestMode = WAITING;
    } 
    

    switch(tempestMode){
        case STARTING:
          intensity = random(20,80);
          thunderDelayMin = getRandomDelay(5000, 10000,  currentTime, 0L, STARTING_END, STARTING_EFFECT, INCREASING);
          thunderDelayMax = getRandomDelay(10000, 20000, currentTime, 0L, STARTING_END, STARTING_EFFECT,  INCREASING);
          crackDelayMin = getRandomDelay(1000, 2500, currentTime, 0L, STARTING_END, STARTING_EFFECT, INCREASING);
          crackDelayMax = getRandomDelay(3000, 7500, currentTime, 0L, STARTING_END, STARTING_EFFECT, INCREASING);
          logProgression(currentTime, 0L, STARTING_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, STARTING_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(3,6));
          break;
        case GROWING:
          intensity = random(60,150);
          thunderDelayMin = getRandomDelay(500, 3000, currentTime, STARTING_END, GROWING_END, GROWING_EFFECT, INCREASING);
          thunderDelayMax = getRandomDelay(3000, 10000, currentTime, STARTING_END, GROWING_END, GROWING_EFFECT, INCREASING);
          crackDelayMin = getRandomDelay(500, 1000, currentTime, STARTING_END, GROWING_END, GROWING_EFFECT, INCREASING);
          crackDelayMax = getRandomDelay(1000, 3000, currentTime, STARTING_END, GROWING_END, GROWING_EFFECT, INCREASING);
          logProgression(currentTime, STARTING_END, GROWING_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, GROWING_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(4,8));
          break;
        case TEMPEST_1_S1:
          intensity = random(190,255);
          thunderDelayMin = getRandomDelay(10, 500, currentTime, GROWING_END, TEMPEST_1_S1_END, TEMPEST_1_S1_EFFECT, INCREASING);
          thunderDelayMax = getRandomDelay(500, 1000, currentTime, GROWING_END, TEMPEST_1_S1_END, TEMPEST_1_S1_EFFECT, INCREASING);
          crackDelayMin = getRandomDelay(50, 250, currentTime, GROWING_END, TEMPEST_1_S1_END, TEMPEST_1_S1_EFFECT, INCREASING);
          crackDelayMax = getRandomDelay(250, 1000, currentTime, GROWING_END, TEMPEST_1_S1_END, TEMPEST_1_S1_EFFECT, INCREASING);
          logProgression(currentTime, GROWING_END, TEMPEST_1_S1_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, TEMPEST_1_S1_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(6,9));
          break;
        case TEMPEST_1_S2:
          intensity = random(190,255);
          thunderDelayMin = getRandomDelay(10, 500, currentTime, TEMPEST_1_S1_END, TEMPEST_1_S2_END, TEMPEST_1_S2_EFFECT, DECREASING);
          thunderDelayMax = getRandomDelay(500, 1000, currentTime, TEMPEST_1_S1_END, TEMPEST_1_S2_END, TEMPEST_1_S2_EFFECT, DECREASING);
          crackDelayMin = getRandomDelay(50, 250, currentTime, TEMPEST_1_S1_END, TEMPEST_1_S2_END, TEMPEST_1_S2_EFFECT, DECREASING);
          crackDelayMax = getRandomDelay(250, 1000, currentTime, TEMPEST_1_S1_END, TEMPEST_1_S2_END, TEMPEST_1_S2_EFFECT, DECREASING);
          logProgression(currentTime, TEMPEST_1_S1_END, TEMPEST_1_S2_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, TEMPEST_1_S2_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(6,9));
          break;  
        case CALM_S1:
          intensity = random(30,90);
          thunderDelayMin = getRandomDelay(500, 1500, currentTime, TEMPEST_1_S2_END, CALM_S1_END, CALM_S1_EFFECT, DECREASING);
          thunderDelayMax = getRandomDelay(1500, 4000, currentTime, TEMPEST_1_S2_END, CALM_S1_END, CALM_S1_EFFECT, DECREASING);
          crackDelayMin = getRandomDelay(500, 1000, currentTime, TEMPEST_1_S2_END, CALM_S1_END, CALM_S1_EFFECT, DECREASING);
          crackDelayMax = getRandomDelay(1000, 3000, currentTime, TEMPEST_1_S2_END, CALM_S1_END, CALM_S1_EFFECT, DECREASING);
          logProgression(currentTime, TEMPEST_1_S2_END, CALM_S1_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, CALM_S1_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(4,7));
          break;
        case CALM_S2:
          intensity = random(30,90);
          thunderDelayMin = getRandomDelay(500, 1500, currentTime, CALM_S1_END, CALM_S2_END, CALM_S2_EFFECT, INCREASING);
          thunderDelayMax = getRandomDelay(1500, 4000, currentTime, CALM_S1_END, CALM_S2_END, CALM_S2_EFFECT, INCREASING);
          crackDelayMin = getRandomDelay(500, 1000, currentTime, CALM_S1_END, CALM_S2_END, CALM_S2_EFFECT, INCREASING);
          crackDelayMax = getRandomDelay(1000, 3000, currentTime, CALM_S1_END, CALM_S2_END, CALM_S2_EFFECT, INCREASING);
          logProgression(currentTime, CALM_S1_END, CALM_S2_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, CALM_S2_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(4,7));
          break;
        case TEMPEST_2_S1:
          intensity = random(215,255);
          thunderDelayMin = getRandomDelay(10, 500, currentTime, CALM_S2_END, TEMPEST_2_S1_END, TEMPEST_2_S1_EFFECT, INCREASING);
          thunderDelayMax = getRandomDelay(500, 1000, currentTime, CALM_S2_END, TEMPEST_2_S1_END, TEMPEST_2_S1_EFFECT, INCREASING);
          crackDelayMin = getRandomDelay(50, 250, currentTime, CALM_S2_END, TEMPEST_2_S1_END, TEMPEST_2_S1_EFFECT, INCREASING);
          crackDelayMax = getRandomDelay(250, 1000, currentTime, CALM_S2_END, TEMPEST_2_S1_END, TEMPEST_2_S1_EFFECT, INCREASING);
          logProgression(currentTime, CALM_S2_END, TEMPEST_2_S1_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, TEMPEST_2_S1_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(8,10));
          break;
        case TEMPEST_2_S2:
          intensity = random(215,255);
          thunderDelayMin = getRandomDelay(10, 500, currentTime, TEMPEST_2_S1_END, TEMPEST_2_S2_END, TEMPEST_2_S2_EFFECT, DECREASING);
          thunderDelayMax = getRandomDelay(500, 1000, currentTime, TEMPEST_2_S1_END, TEMPEST_2_S2_END, TEMPEST_2_S2_EFFECT, DECREASING);
          crackDelayMin = getRandomDelay(50, 250, currentTime, TEMPEST_2_S1_END, TEMPEST_2_S2_END, TEMPEST_2_S2_EFFECT, DECREASING);
          crackDelayMax = getRandomDelay(250, 1000, currentTime, TEMPEST_2_S1_END, TEMPEST_2_S2_END, TEMPEST_2_S2_EFFECT, DECREASING);
          logProgression(currentTime, TEMPEST_2_S1_END, TEMPEST_2_S2_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, TEMPEST_2_S2_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(8,10));
          break;
        case REDUCING:
          intensity = random(50,130);
          thunderDelayMin = getRandomDelay(500, 3000, currentTime, TEMPEST_2_S2_END, REDUCING_END, REDUCING_EFFECT, DECREASING);
          thunderDelayMax = getRandomDelay(3000, 10000, currentTime, TEMPEST_2_S2_END, REDUCING_END, REDUCING_EFFECT, DECREASING);
          crackDelayMin = getRandomDelay(500, 1000, currentTime, TEMPEST_2_S2_END, REDUCING_END, REDUCING_EFFECT, DECREASING);
          crackDelayMax = getRandomDelay(1000, 3000, currentTime, TEMPEST_2_S2_END, REDUCING_END, REDUCING_EFFECT, DECREASING);
          logProgression(currentTime, TEMPEST_2_S2_END, REDUCING_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, REDUCING_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(3,6));
          break;
        case ENDING:
          intensity = random(20,80);
          thunderDelayMin = getRandomDelay(5000, 10000, currentTime, REDUCING_END, ENDING_END, ENDING_EFFECT, DECREASING);
          thunderDelayMax = getRandomDelay(10000, 20000, currentTime, REDUCING_END, ENDING_END, ENDING_EFFECT, DECREASING);
          crackDelayMin = getRandomDelay(1000, 2500, currentTime, REDUCING_END, ENDING_END, ENDING_EFFECT, DECREASING);
          crackDelayMax = getRandomDelay(3000, 7500, currentTime, REDUCING_END, ENDING_END, ENDING_EFFECT, DECREASING);
          logProgression(currentTime, REDUCING_END, ENDING_END, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, ENDING_EFFECT);
          thunder(intensity, thunderDelayMin, thunderDelayMax, crackDelayMin, crackDelayMax, random(3,7));
          break;
        case WAITING:
          switchOff();
          break;
    }
    delay(50);
}

unsigned int getRandomDelay(unsigned int delayMin, unsigned int delayMax, unsigned long  cTime, unsigned long sTime, unsigned long  rTime, enum EasingType easeEffect, enum TempestType tempestType){
    unsigned int d = 0;
    float progression = 0.01;
    cTime = cTime - sTime;
    rTime = rTime - sTime;
    
    switch(easeEffect){
        case LINEAR: 
          progression = easeLinear(cTime, 0, 100, rTime);
          break;
        case INSINE: 
          progression =  easeInSine(cTime, 0, 100, rTime);
          break;
        case OUTSINE: 
          progression =  easeOutSine(cTime, 0, 100, rTime);
          break;
        case INCUBIC: 
          progression =  easeInCubic(cTime, 0, 100, rTime);
          break;
        case OUTCUBIC: 
          progression =  easeOutCubic(cTime, 0, 100, rTime);
          break;
        case INCIRC: 
          progression =  easeInCirc(cTime, 0, 100, rTime);
          break;
        case INQUINT:
          progression =  easeInQuint(cTime, 0, 100, rTime);
          break;
    }
    
    if(progression>100){progression=100;} 
    
    switch(tempestType){
        case INCREASING:
            d = delayMax - (delayMax - delayMin) * (progression/100);
            break;
        case DECREASING:
            d = delayMin + (delayMax - delayMin) * (progression/100);
            break;
        default: break;
    }
    return d;
}

void thunder(unsigned int intensity, unsigned int thunderDelayMin, unsigned int thunderDelayMax, unsigned int crackDelayMin, unsigned int crackDelayMax, unsigned int rollingMaxHits){
    switch(random(1,3)){
        case 1:
            thunderburst(intensity);
            delay(random(thunderDelayMin,thunderDelayMax));
            break;
        case 2:
            rolling(intensity, rollingMaxHits);
            break;
        case 3:
            crack(intensity);
            delay(random(crackDelayMin,crackDelayMax));
            break;
    }
}

void rolling(unsigned int intensity, unsigned int rollingMaxHits){
    // a simple method where we go through every LED with 1/10 chance of being turned on, up to 10 times, with a random delay between each time
    for(unsigned int r=0;r<random(2,rollingMaxHits);r++){
        for(unsigned int i=0;i<NUM_LEDS;i++){
            if(random(0,100)>90){
                leds[i] = CHSV(_hue, _saturation, intensity);
            }else{
                //dont need reset as we're blacking out other LEDs her
                leds[i] = CHSV(0,0,0);
            }
        }
        FastLED.show();
        delay(random(5,100));
        switchOff();
    }
}

void crack(unsigned int intensity){
    //turn everything white briefly
    for(unsigned int i=0;i<NUM_LEDS;i++) {
        leds[i] = CHSV(_hue, _saturation, intensity);
    }
    FastLED.show();
    delay(random(10,100));
    switchOff();
}

void thunderburst(unsigned int intensity){
    // this thunder works by lighting two random lengths of the strand from 10-20 pixels.
    unsigned int rs1 = random(0,NUM_LEDS/2);
    unsigned int rl1 = random(10,20);
    unsigned int rs2 = random(rs1+rl1,NUM_LEDS);
    unsigned int rl2 = random(10,20);

    //repeat this chosen strands a few times, adds a bit of realism
    for(unsigned int r = 0;r<random(3,6);r++){
        for(unsigned int i=0;i< rl1; i++){
            leds[i+rs1] = CHSV(_hue, _saturation, intensity);
        }
        if(rs2+rl2 < NUM_LEDS){
            for(unsigned int i=0;i< rl2; i++){
                leds[i+rs2] = CHSV(_hue, _saturation, intensity);
            }
        }
        FastLED.show();
        //stay illuminated for a set time
        delay(random(10,50));
        switchOff();
        delay(random(10,50));
    }
}

// Switch off all lights
void switchOff(){
    for (unsigned int i=0;i<NUM_LEDS;i++){
        leds[i] = CHSV( 0, 0, 0);
    }
    FastLED.show();
}

// Reset all global variables and switch off all lights
void reset(){
    _thunderDelayMin = 10;
    _thunderDelayMax = 500;
    _crackDelayMin = 50;
    _crackDelayMax = 250;
    hueFading = false;
    satFading = false;
    switchOff();
}

void logTime(unsigned long t){
    unsigned int minutes = t/60000;
    unsigned int seconds = (t - (minutes*60000))/1000;
    Serial.print(minutes);
    Serial.print(F(" minutes "));
    Serial.print(seconds);
    Serial.print(F(" seconds ")); 
}

void logProgression(unsigned long cTime, unsigned long sTime, unsigned long rTime, unsigned int thunderDelayMin, unsigned int thunderDelayMax, unsigned int crackDelayMin, unsigned int crackDelayMax, enum EasingType easeEffect){
    Serial.print(F(" - Progression : "));
    cTime = cTime - sTime;
    rTime = rTime - sTime;
    float progression = 0.01;
    switch(easeEffect){
        case LINEAR: 
          progression = easeLinear(cTime, 0, 100, rTime);
          break;
        case INSINE: 
          progression =  easeInSine(cTime, 0, 100, rTime);
          break;
        case OUTSINE: 
          progression =  easeOutSine(cTime, 0, 100, rTime);
          break;
        case INCUBIC: 
          progression =  easeInCubic(cTime, 0, 100, rTime);
          break;
        case OUTCUBIC: 
          progression =  easeOutCubic(cTime, 0, 100, rTime);
          break;
        case INCIRC: 
          progression =  easeInCirc(cTime, 0, 100, rTime);
          break;
        case INQUINT:
          progression =  easeInQuint(cTime, 0, 100, rTime);
          break;
    }
    Serial.print(progression);
    Serial.print(F("%  => "));
    Serial.print(thunderDelayMin);
    Serial.print(F(" - "));
    Serial.print(thunderDelayMax);
    Serial.print(F(" - "));
    Serial.print(crackDelayMin);
    Serial.print(F(" - "));
    Serial.println(crackDelayMax);
}

int increasingValue(int val, int inc, int max){
    val = val + inc;
    if(val > max){
        val = max;                 
    }
    return val;
}

int decreasingValue(int val, int dec, int min){
    val = val - dec;                 
    if(val <= min){                   
        val = min;                 
    }
    return val;
}


float easeLinear(long t, int b, int c, long d){float p = (float)t/(float)d; return (float)(c*p + b);}
float easeInSine(long t, int b, int c, long d) {return (float)(-c * (double)cos((float)((float)t/(float)d) * (float)(PI/2)) + c + b);}
float easeOutSine(long t, int b, int c, long d) {return (float)(c * (double)sin((float)((float)t/(float)d) * (float)(PI/2)) + b);};
float easeInCubic(long t, int b, int c, long d) {float p = (float)t/(float)d;return (float)(c*p*p*p + b);};
float easeOutCubic(long t, int b, int c, long d) {float p = (float)t/(float)d;p--;return (float)(c*(p*p*p + 1) + b);};
float easeInCirc(long t, int b, int c, long d) {float p = (float)t/(float)d;return (float)(-c * ((double)sqrt(1 - p*p) - 1) + b);};
float easeInQuint(long t, int b, int c, long d) {float p = (float)t/(float)d; return (float)(c*p*p*p*p*p + b);};
