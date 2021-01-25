#if 1

#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <Wire.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#define MINPRESSURE 100
#define MAXPRESSURE 1000

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
const int XP=8,XM=A2,YP=A3,YM=9; //ID=0x9341
const int TS_LEFT=898,TS_RT=240,TS_TOP=485,TS_BOT=620;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

Adafruit_GFX_Button on_btn, off_btn;


int pixel_x, pixel_y;     //Touch_getXY() updates global vars
bool Touch_getXY(void)
{
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);      //restore shared pins
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);   //because TFT control pins
    digitalWrite(XM, HIGH);
    bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
    if (pressed) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width()); //.kbv makes sense to me
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }
    return pressed;
}

#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
char q[6]="Pause";
char r[]="Next";

//---------------------These will be sent to Radio------------------------
float channels[4];
int index = 1;
bool radioOn=true;
//---------------------------------------------------------------------

//prints the current chosen Station
void print1(){
  tft.setCursor(65, 85); 
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.println(channels[index]);
}

void setup(void)
{   
    //Setting up by Screen
    Serial.begin(9600);
    uint16_t ID = tft.readID();
    Serial.print("TFT ID = 0x");
    Serial.println(ID, HEX);
    Serial.println("Calibrate for your Touch Panel");
    if (ID == 0xD3D3) ID = 0x9486; 
    tft.begin(ID);
    tft.setRotation(0);       
    tft.fillScreen(BLACK);
    on_btn.initButton(&tft,  60, 200, 100, 40, WHITE, CYAN, BLACK, q, 2);
    off_btn.initButton(&tft, 180, 200, 100, 40, WHITE, CYAN, BLACK, r, 2);
    on_btn.drawButton(false);
    off_btn.drawButton(false);
    tft.fillRect(40, 80, 160, 40, CYAN);

    channels[0] = 100.6;
    channels[1] = 107.4;
    channels[2] = 108;
    channels[3] = 104.2;

    print1();

    Wire.begin(0x05);
    Wire.onRequest(requestEvent); // register event
}

void requestEvent(){
  if(radioOn){
    Wire.write(index);  
  }else{
    Wire.write(4);      
  }
  Serial.println(index);
  Serial.println(radioOn);
}

void loop(void)
{
    bool down = Touch_getXY();
    on_btn.press(down && on_btn.contains(pixel_x, pixel_y));
    off_btn.press(down && off_btn.contains(pixel_x, pixel_y));
     if (on_btn.justReleased())
        on_btn.drawButton();
    if (off_btn.justReleased())
        off_btn.drawButton();

    //If next button is pressed
    if (on_btn.justPressed()) {
      tft.fillRect(40, 80, 160, 40, CYAN);
      index++;
      index = index % 4;
      print1();       
    }
    //iff play/pause button is pressed
    if (off_btn.justPressed()) { 
          if(radioOn){
          char q[]="Play";
          on_btn.initButton(&tft,  60, 200, 100, 40, WHITE, CYAN, BLACK,q, 2);
          radioOn=false;
          Serial.println("Radio Paused");
         }else{
          char q[]="Pause";
          on_btn.initButton(&tft,  60, 200, 100, 40, WHITE, CYAN, BLACK,q, 2);
          radioOn=true;
          Serial.println("Radio Played");
         }        
        on_btn.drawButton(true);
        off_btn.drawButton(true);
        //tft.fillRect(40, 80, 160, 80, RED);
    }
    
    //you can remove the delay if you want but the touch will be too fast
    delay(5);

    
    //--------------Send station and radioOn to I2C-----------------
   
    //--------------------------------------------------------------
}
#endif
