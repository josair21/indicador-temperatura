#include <Wire.h>
#include "Sodaq_DS3231.h"
#include <UTFT.h>
#include <UTouch.h>
#include <avr/pgmspace.h>
#include <Adafruit_MAX31865.h>
#define RREF 423.9
#define RNOMINAL 100.1
#define XMAX 799
#define YMAX 479
extern uint8_t BigFont[];
extern uint8_t SmallFont[];
extern uint8_t SixteenSegment40x60[];
extern uint8_t Dingbats1_XL[];
extern uint8_t Grotesk24x48[];
extern unsigned int deletebtn[];
extern unsigned int down[];
extern unsigned int right[];
extern unsigned int up[];
char weekDay[][4] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
Adafruit_MAX31865 thermo = Adafruit_MAX31865(4, 5, 6, 7);
//Adafruit_MAX31865 thermo=Adafruit_MAX31865(10);
UTFT myGLCD(SSD1963_800480,38,39,40,41);  //(byte model, int RS, int WR, int CS, int RST, int SER)
UTouch  myTouch( 43, 42, 44, 45, 46);  //byte tclk, byte tcs, byte din, byte dout, byte irq
DateTime dt(2021, 6, 1, 15, 55, 00, 1);
DateTime now;
void setup() {
  thermo.begin(MAX31865_2WIRE);
  Serial.begin(57600);
  Wire.begin();
  rtc.begin();
  //rtc.setDateTime(dt); //Ajustar fecha - hora
  pinMode(8, OUTPUT);  //backlight
  pinMode(3,OUTPUT); 
  digitalWrite(8, HIGH);//on
  myGLCD.InitLCD(1);
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  start_screen(); //pantalla incial
}
/**********************/
void loop(){
  print_datetime();
  print_buttons();
  upper_value();
  delay(1000);
}
void start_screen(){ //Funcion de la pantalla incial al encender
  myGLCD.fillScr(255, 255, 255); //fondo de pantalla blanco
  
  myGLCD.setColor(VGA_RED); //Barra lateral
  myGLCD.fillRect(XMAX - 100, 0, XMAX, 479);
  
  myGLCD.setColor(50, 50, 255); //Barra superior
  myGLCD.fillRect(0, 0, 799, 100);
  
  myGLCD.setBackColor(50, 50, 255);
  myGLCD.setColor(250, 250, 255);
  myGLCD.setFont(Grotesk24x48);
  myGLCD.print("INDICADOR DE TEMPERATURA", 100, 25, 0);
}
void print_datetime(){ //leer e imprimir hora y fecha
  now = rtc.now();
  char date_array[15] = "";
  char time_array[15] = "";
  sprintf(date_array, "%02d/%02d/%02d", now.date(), now.month(), now.year());
  sprintf(time_array, "%3s %02d:%02d", weekDay[now.dayOfWeek()+1], now.hour(), now.minute());
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print(date_array, 510, 120, 0);
  myGLCD.print(time_array, 518, 138, 0);

  myGLCD.setColor(VGA_GREEN);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(490 + i, 110 + i, 690 - i, 160 - i);
  }
}
void print_buttons(){
  
  myGLCD.drawBitmap(732, 136, 32, 32, up, 1);
  myGLCD.drawBitmap(732, 236, 32, 32, down, 1);
  myGLCD.drawBitmap(732, 336, 32, 32, right, 1);
  myGLCD.drawBitmap(732, 436, 32, 32, deletebtn, 1);
}
void upper_value(){
  myGLCD.setColor(VGA_RED);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(490 + i, 200 + i, 690 - i, 300 - i - 30);
  }
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(Grotesk24x48);
  myGLCD.print("08.00 C", 510, 215, 0);
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Alerta superior", 510, 275, 0);
}
