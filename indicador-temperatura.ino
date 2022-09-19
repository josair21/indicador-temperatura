#include <Wire.h>
#include "Sodaq_DS3231.h"
char weekDay[][4] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
DateTime dt(2021, 6, 1, 15, 55, 00, 1);
#include <UTFT.h>
#include <UTouch.h>
#include <avr/pgmspace.h>

#include <Adafruit_MAX31865.h>
Adafruit_MAX31865 thermo = Adafruit_MAX31865(4, 5, 6, 7);
//Adafruit_MAX31865 thermo=Adafruit_MAX31865(10);
#define RREF 423.9
#define RNOMINAL 100.1

UTFT myGLCD(SSD1963_800480,38,39,40,41);  //(byte model, int RS, int WR, int CS, int RST, int SER)
UTouch  myTouch( 43, 42, 44, 45, 46);  //byte tclk, byte tcs, byte din, byte dout, byte irq
extern uint8_t BigFont[];
extern uint8_t SmallFont[];
extern uint8_t SixteenSegment40x60[];
extern uint8_t Dingbats1_XL[];
extern uint8_t Grotesk24x48[];
//extern unsigned int cloud[];
extern unsigned int calendar[];


int buzz=0,buzzaux=0;
String state="starting";
int x, y;
int xx=800,yy=480;
int digit=0;
int outrangetimes = 0;
bool temp_data;
bool sonido=1;
bool twodigits=0;
float variable_test=5.99,limit_low_aux,limit_up_aux,entero,decimal_1,decimal_2;
float temp, last_temp, up_limit=8.00, low_limit=2.00;
float temp_array[6] = {100,100,100,100,100,100};
int hour_array[6] = {100,100,100,100,100,100};
int minute_array[6] = {100,100,100,100,100,100};
int second_array[6] = {100,100,100,100,100,100};
unsigned long timemillis=millis(),buzzmillis=millis();
char stCurrent[20]="";
int stCurrentLen=0;
char stLast[20]="";
void setup() {
  thermo.begin(MAX31865_2WIRE);
  Serial.begin(57600);
  Wire.begin();
  rtc.begin();
  //rtc.setDateTime(dt); //Adjust date-time as defined 'dt' above
  pinMode(8, OUTPUT);  //backlight
  pinMode(3,OUTPUT); 
  digitalWrite(8, HIGH);//on
  myGLCD.InitLCD(1);
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
    state="refresh";
    background(10);
    log_icon();
    calendar_icon();
    log_values_grill();
    myGLCD.setFont(Grotesk24x48);
    myGLCD.setColor(VGA_WHITE);
    myGLCD.setBackColor(VGA_TRANSPARENT);
    /*myGLCD.print(".",535,178);
    myGLCD.print("C",550,211);*/
    myGLCD.setBackColor(160,160,255);
    myGLCD.setFont(SixteenSegment40x60);
    myGLCD.print("     ",30,200);
    myGLCD.print("     ",CENTER,200);
    myGLCD.print("     ",570,200);
}
/**********************/
void loop() {
  alarma();
  sound();
  if(state=="refresh"){
    state="working";
    background(4);
    log_icon();
    calendar_icon();
    sound_aux();   
  }
  if(state=="working"){
    actual_temp();
    time_date();
    log_values();
    btn_test();
    limits();
  }
  if(state=="numpad_low" || state=="numpad_up"){
    time_date();
    get_low_values();
    get_up_values();
    actual_temp();
    log_values();
  }
}
/*********************/
void sound(){
  if (myTouch.dataAvailable() && state=="working")
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if(0+350<x && x<0+450 && 0+105<y && y<0+185){
        sonido=!sonido;
        sound_aux();
        
      }
      
    }
}
void sound_aux(){
  if(sonido==1){
          myGLCD.setColor(VGA_GREEN);
          myGLCD.fillRoundRect (350, 105, 450, 185);
          myGLCD.setFont(Dingbats1_XL);
          myGLCD.setBackColor(VGA_GREEN);
          myGLCD.setColor(VGA_BLACK);
          myGLCD.print("@",400-16,145-12);
          
        }
        if(sonido==0){
          myGLCD.setColor(VGA_RED);
          myGLCD.fillRoundRect (350, 105, 450, 185);
          myGLCD.setFont(Dingbats1_XL);
          myGLCD.setBackColor(VGA_RED);
          myGLCD.setColor(VGA_BLACK);
          myGLCD.print("@",400-16,145-12);
          myGLCD.setFont(Grotesk24x48);
          myGLCD.setBackColor(VGA_TRANSPARENT);
          myGLCD.print("X",400-12,145-24);
        }
}
void log_values(){ 
  if((temp>up_limit || temp<low_limit) && temp_data==1){
    if(outrangetimes==6)outrangetimes=0;
    outrangetimes=outrangetimes+1;
    temp_data=0;
    int j=100;
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setFont(BigFont);
    for (int i=1;i<=5;i++){
      temp_array[6-i]=temp_array[5-i];
      hour_array[6-i]=hour_array[5-i];
      minute_array[6-i]=minute_array[5-i];
      second_array[6-i]=second_array[5-i];
      }
      temp_array[0]=temp;
      DateTime now = rtc.now();
      hour_array[0]=now.hour();
      minute_array[0]=now.minute();
      second_array[0]=now.second();
    for(int i=0;i<=5;i++){
      if(temp_array[i]!=100){
        myGLCD.setFont(BigFont);
        if(temp_array[i]<10){myGLCD.print("     ",10+j*(i+1),381);myGLCD.printNumF(temp_array[i],2,18+j*(i+1),381);}
        if(temp_array[i]>=10)myGLCD.printNumF(temp_array[i],2,10+j*(i+1),381);
        myGLCD.setFont(SmallFont);
        if(hour_array[i]<10){
          myGLCD.print("0",12+j*(i+1),430);
          myGLCD.printNumI(hour_array[i],20+j*(i+1),430);
        }
        else  myGLCD.printNumI(hour_array[i],12+j*(i+1),430);
        if(minute_array[i]<10){
          myGLCD.print("0",45+j*(i+1),430);
          myGLCD.printNumI(minute_array[i],53+j*(i+1),430);
        }
        else  myGLCD.printNumI(minute_array[i],45+j*(i+1),430);
        if(second_array[i]<10){
          myGLCD.print("0",73+j*(i+1),430);
          myGLCD.printNumI(second_array[i],81+j*(i+1),430);
        }
        else  myGLCD.printNumI(second_array[i],73+j*(i+1),430);
      }
    }
  }
}
void log_values_grill(){
  int j=100;
  for (int i=1;i<7;i++){
    myGLCD.setBackColor(VGA_WHITE);
    myGLCD.setColor(255,255,255);
    myGLCD.fillRoundRect(i*j,330,j+j*i,360);
    myGLCD.fillRoundRect(i*j,360,j+j*i,420);
    myGLCD.fillRoundRect(i*j,420,j+j*i,450);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.drawRoundRect(i*j,330,j+j*i,360);
    myGLCD.drawRoundRect(i*j,360,j+j*i,420);
    myGLCD.drawRoundRect(i*j,420,j+j*i,450);
    myGLCD.setFont(BigFont);
    myGLCD.printNumI(i,40+j*i,338);
    myGLCD.print("--.--",10+j*i,381);
    //myGLCD.print("C",80+j*i,381);
    myGLCD.setBackColor(VGA_TRANSPARENT);
    //myGLCD.print(".",72+j*i,370);
    myGLCD.print(":",26+j*i,428);
    myGLCD.print(":",59+j*i,428);
    myGLCD.setFont(SmallFont);
    myGLCD.print("--",12+j*i,430);
    myGLCD.print("--",45+j*i,430);
    myGLCD.print("--",73+j*i,430);
  }
}
void background(int z){
  for (int x=0;x<z;x++){
    myGLCD.setColor(20*x, 20*x, 255);
    myGLCD.fillRect(0,x*48,800,(x+1)*48);
  }
}
void calendar_icon(){
  myGLCD.setColor(VGA_WHITE);
  myGLCD.fillRoundRect (550-260, 30, 770-260, 94);
  myGLCD.drawBitmap(702-260,30,64,64,calendar);
}
void time_date(){
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(VGA_WHITE);
  myGLCD.setColor(VGA_BLACK);
  DateTime now = rtc.now(); 
  if(now.date()<10){myGLCD.printNumI(0,565-260,40);
  myGLCD.printNumI(now.date(),565-260+myGLCD.getFontXsize(),40);}
  else myGLCD.printNumI(now.date(),565-260,40);
  myGLCD.print("/",565-260+2*myGLCD.getFontXsize(),40);
  if(now.month()<10){myGLCD.printNumI(0,565-260+3*+myGLCD.getFontXsize(),40);
  myGLCD.printNumI(now.month(),565-260+4*myGLCD.getFontXsize(),40);}
  else myGLCD.printNumI(now.month(),565-260+3*+myGLCD.getFontXsize(),40);
  myGLCD.print("/",565-260+5*myGLCD.getFontXsize(),40);
  if(now.year()<10){myGLCD.printNumI(0,565-260+6*+myGLCD.getFontXsize(),40);
  myGLCD.printNumI(now.year(),565-260+7*myGLCD.getFontXsize(),40);}
  else myGLCD.printNumI(now.year()-2000,565-260+6*+myGLCD.getFontXsize(),40);
 // myGLCD.print("/",565+2*myGLCD.getFontXsize(),40);
  if(now.hour()<10){myGLCD.printNumI(0,565-260,70);
  myGLCD.printNumI(now.hour(),565+myGLCD.getFontXsize()-260,70);}
  else myGLCD.printNumI(now.hour(),565-260,70);
  myGLCD.print(":",565+2*myGLCD.getFontXsize()-260,70);
  if(now.minute()<10){myGLCD.printNumI(0,565+3*+myGLCD.getFontXsize()-260,70);
  myGLCD.printNumI(now.minute(),565+4*myGLCD.getFontXsize()-260,70);}
  else myGLCD.printNumI(now.minute(),565+3*+myGLCD.getFontXsize()-260,70);
  myGLCD.print(":",565+5*myGLCD.getFontXsize()-260,70);
  if(now.second()<10){myGLCD.printNumI(0,565+6*+myGLCD.getFontXsize()-260,70);
  myGLCD.printNumI(now.second(),565+7*myGLCD.getFontXsize()-260,70);}
  else myGLCD.printNumI(now.second(),565+6*+myGLCD.getFontXsize()-260,70);
}
void actual_temp(){
  if(millis()-timemillis>4000){
    bool lasttwodigits;
    float a = 0.0008, b = 0.9656, c = 3.0243;
    temp=thermo.temperature(RNOMINAL,RREF);
    temp = a * temp * temp + b * temp + c;
    Serial.print("temp: ");
    Serial.println(temp);
    myGLCD.setFont(SixteenSegment40x60);
    myGLCD.setColor(VGA_BLACK);
    myGLCD.setBackColor(160,160,255);
    //temp = (random(0,3000)/100.0);
    if(temp>up_limit || temp<low_limit){
      myGLCD.setColor(255,0,0);
      buzz=5;
      
    }
    else buzz--;
    if(buzz<=0)buzz=0;
    lasttwodigits=twodigits;
    if(temp<10.0)twodigits=0;
    else twodigits=1;
    if(twodigits!=lasttwodigits)myGLCD.print("     ",CENTER,200);
    myGLCD.printNumF(temp,1,CENTER,200);
    timemillis=millis();
    temp_data=1;
    }
}


void log_icon(){
  myGLCD.setFont(Dingbats1_XL);
  myGLCD.setColor(VGA_WHITE);
  myGLCD.setBackColor(VGA_TRANSPARENT);
  myGLCD.print("i",40,49+30);
  myGLCD.print("i",760-32,49+30);
  myGLCD.setFont(Grotesk24x48);
  myGLCD.print("SET",85,39+30);
  myGLCD.print("SET",715-24-45,39+30);
  myGLCD.drawRoundRect (30, 30+30, 200, 94+30);
  myGLCD.drawRoundRect (600, 30+30, 770, 94+30);
}

void btn_test(){
  if (myTouch.dataAvailable())
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      if((0+30)<x && x<(0+200) && (0+60)<y && y<(0+124)){
      frame_color(30,30+30,200,94+30);
      draw_btn_low();
      }
      if((0+600)<x && x<(0+770) && (0+60)<y && y<(0+124)){
      frame_color(600,30+30,770,30+94);
      draw_btn_up();
      }
    }
}
void frame_color(int x1, int y1, int x2, int y2){
  myGLCD.setColor(255, 0, 0);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
  while (myTouch.dataAvailable())
    myTouch.read();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (x1, y1, x2, y2);
}
void limits(){
  myGLCD.setColor(VGA_GRAY);
  myGLCD.setBackColor(160,160,255);
  myGLCD.setFont(SixteenSegment40x60);
  myGLCD.printNumF(low_limit,2,50,200);
  myGLCD.printNumF(up_limit,2,590,200);
}
void draw_btn_low(){
  state="numpad_low";
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setFont(BigFont);
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 10, 60+(x*60), 60);
    myGLCD.printNumI(x+1, 27+(x*60), 27);
  }
// Draw the center row of buttons
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60), 70, 60+(x*60), 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60), 70, 60+(x*60), 120);
    if (x<4)
      myGLCD.printNumI(x+6, 27+(x*60), 87);
  }
  myGLCD.print("0", 267, 87);
// Draw the lower row of buttons
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10, 130, 150, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10, 130, 150, 180);
  myGLCD.print("Clear", 40, 147);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (160, 130, 300, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (160, 130, 300, 180);
  myGLCD.print("Enter", 190, 147);
  myGLCD.setBackColor (0, 0, 0);
}
void draw_btn_up(){
  int g=490;
  state="numpad_up";
  myGLCD.setBackColor(VGA_BLUE);
  myGLCD.setColor(VGA_BLACK);
  myGLCD.setFont(BigFont);
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60)+g, 10, g+60+(x*60), 60);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60)+g, 10, g+60+(x*60), 60);
    myGLCD.printNumI(x+1, 27+(x*60)+g, 27);
  }
// Draw the center row of buttons
  for (x=0; x<5; x++)
  {
    myGLCD.setColor(0, 0, 255);
    myGLCD.fillRoundRect (10+(x*60)+g, 70, 60+(x*60)+g, 120);
    myGLCD.setColor(255, 255, 255);
    myGLCD.drawRoundRect (10+(x*60)+g, 70, 60+(x*60)+g, 120);
    if (x<4)
      myGLCD.printNumI(x+6, 27+(x*60)+g, 87);
  }
  myGLCD.print("0", 267+g, 87);
// Draw the lower row of buttons
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (10+g, 130, 150+g, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (10+g, 130, 150+g, 180);
  myGLCD.print("Clear", 40+g, 147);
  myGLCD.setColor(0, 0, 255);
  myGLCD.fillRoundRect (160+g, 130, 300+g, 180);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRoundRect (160+g, 130, 300+g, 180);
  myGLCD.print("Enter", 190+g, 147);
  myGLCD.setBackColor (0, 0, 0);
}
void get_low_values(){
  if (myTouch.dataAvailable() && state=="numpad_low")
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      
      if ((y>=0+10) && (y<=0+60))  // Upper row
      {
        if ((x>=0+10) && (x<=0+60))  // Button: 1
        {
          frame_color(10, 10, 60, 60);
          number_pressed(1);

        }
        if ((x>=0+70) && (x<=0+120))  // Button: 2
        {
          frame_color(70, 10, 120, 60);
          number_pressed(2);
        }
        if ((x>=0+130) && (x<=0+180))  // Button: 3
        {
          frame_color(130, 10, 180, 60);
          number_pressed(3);
        }
        if ((x>=0+190) && (x<=0+240))  // Button: 4
        {
          frame_color(190, 10, 240, 60);
          number_pressed(4);
        }
        if ((x>=0+250) && (x<=0+300))  // Button: 5
        {
          frame_color(250, 10, 300, 60);
          number_pressed(5);
        }
      }

      if ((y>=0+70) && (y<=0+120))  // Center row
      {
        if ((x>=0+10) && (x<=0+60))  // Button: 6
        {
          frame_color(10, 70, 60, 120);
          number_pressed(6);
        }
        if ((x>=0+70) && (x<=0+120))  // Button: 7
        {
          frame_color(70, 70, 120, 120);
          number_pressed(7);
        }
        if ((x>=0+130) && (x<=0+180))  // Button: 8
        {
          frame_color(130, 70, 180, 120);
          number_pressed(8);
        }
        if ((x>=0+190) && (x<=0+240))  // Button: 9
        {
          frame_color(190, 70, 240, 120);
          number_pressed(9);
        }
        if ((x>=0+250) && (x<=0+300))  // Button: 0
        {
          frame_color(250, 70, 300, 120);
          number_pressed(0);
        }
      }

      if ((y>=0+130) && (y<=0+180))  // Upper row
      {
        if ((x>=0+10) && (x<=0+150))  // Button: Clear
        {
          frame_color(10, 130, 150, 180);
          digit=0;
        }
        if ((x>=0+160) && (x<=0+300))  // Button: Enter
        {
          frame_color(160, 130, 300, 180);
          digit=0;
          low_limit=limit_low_aux;
          }
          state="refresh";
        }
      }
    }
void get_up_values(){
  int g=490;
  if (myTouch.dataAvailable() && state=="numpad_up")
    {
      myTouch.read();
      x=myTouch.getX();
      y=myTouch.getY();
      
      if ((y>=0+10) && (y<=0+60))  // Upper row
      {
        if ((x>=0+10+g) && (x<=0+60+g))  // Button: 1
        {
          frame_color(10+g, 10, 60+g, 60);
          number_pressed(1);

        }
        if ((x>=0+70+g) && (x<=0+120+g))  // Button: 2
        {
          frame_color(70+g, 10, 120+g, 60);
          number_pressed(2);
        }
        if ((x>=0+130+g) && (x<=0+180+g))  // Button: 3
        {
          frame_color(130+g, 10, 180+g, 60);
          number_pressed(3);
        }
        if ((x>=0+190+g) && (x<=0+240+g))  // Button: 4
        {
          frame_color(190+g, 10, 240+g, 60);
          number_pressed(4);
        }
        if ((x>=0+g+250) && (x<=0+g+300))  // Button: 5
        {
          frame_color(250+g, 10, 300+g, 60);
          number_pressed(5);
        }
      }

      if ((y>=0+70) && (y<=0+120))  // Center row
      {
        if ((x>=0+10+g) && (x<=0+g+60))  // Button: 6
        {
          frame_color(g+10, 70, g+60, 120);
          number_pressed(6);
        }
        if ((x>=0+70+g) && (x<=0+120+g))  // Button: 7
        {
          frame_color(g+70, 70, g+120, 120);
          number_pressed(7);
        }
        if ((x>=0+130+g) && (x<=0+180+g))  // Button: 8
        {
          frame_color(130+g, 70, 180+g, 120);
          number_pressed(8);
        }
        if ((x>=0+190+g) && (x<=0+240+g))  // Button: 9
        {
          frame_color(190+g, 70, 240+g, 120);
          number_pressed(9);
        }
        if ((x>=0+250+g) && (x<=0+300+g))  // Button: 0
        {
          frame_color(250+g, 70, 300+g, 120);
          number_pressed(0);
        }
      }

      if ((y>=0+130) && (y<=0+180))  // Upper row
      {
        if ((x>=0+10+g) && (x<=0+g+150))  // Button: Clear
        {
          frame_color(10+g, 130, 150+g, 180);
          digit=0;
        }
        if ((x>=0+160+g) && (x<=0+300+g))  // Button: Enter
        {
          frame_color(160+g, 130, 300+g, 180);
          digit=0;
          up_limit=limit_up_aux;
          }
          state="refresh";
        }
      }
    }
    
 void number_pressed(int b){
  myGLCD.setColor(VGA_WHITE);
      myGLCD.setBackColor(160,160,255);
      myGLCD.setFont(SixteenSegment40x60);
      if(state=="numpad_low"){
  if(digit==2){
            decimal_2=b/100.0;
            limit_low_aux=entero+decimal_1+decimal_2;
          }
          if(digit==1){
            digit=2;
            decimal_1=b/10.0;
            limit_low_aux=entero+decimal_1;
          }
          if(digit==0){
            digit=1;
            entero=b;
            limit_low_aux=entero;
            
          }
          myGLCD.printNumF(limit_low_aux,digit,50,200);
}
if(state=="numpad_up"){
  if(digit==2){
            decimal_2=b/100.0;
            limit_up_aux=entero+decimal_1+decimal_2;
          }
          if(digit==1){
            digit=2;
            decimal_1=b/10.0;
            limit_up_aux=entero+decimal_1;
          }
          if(digit==0){
            digit=1;
            entero=b;
            limit_up_aux=entero;
            
          }
          myGLCD.printNumF(limit_up_aux,digit,590,200);
}

} 

void alarma(){
  if(buzz!=0 && sonido){
    if(millis()-buzzmillis>=500){
      buzzmillis=millis();
      digitalWrite(3,!digitalRead(3));
    }
  }
  else digitalWrite(3,0);
}
