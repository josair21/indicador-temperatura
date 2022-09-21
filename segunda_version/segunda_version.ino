#include <Wire.h>
#include "Sodaq_DS3231.h"
#include <UTFT.h>
#include <UTouch.h>
#include <EEPROM.h>
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
unsigned int xTouch, yTouch, k = 0;
unsigned int lastDay, lastMonth, lastYear, lastHour, lastMinute;
unsigned long j = 0;
float Tmin, Tmax, lastTmin, lastTmax;
unsigned int value_edit = 0;
char weekDay[][4] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
Adafruit_MAX31865 thermo = Adafruit_MAX31865(4, 5, 6, 7);
//Adafruit_MAX31865 thermo=Adafruit_MAX31865(10);
UTFT myGLCD(SSD1963_800480,38,39,40,41);  //(byte model, int RS, int WR, int CS, int RST, int SER)
UTouch  myTouch( 43, 42, 44, 45, 46);  //byte tclk, byte tcs, byte din, byte dout, byte irq
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
  EEPROM.get(0, Tmin); EEPROM.get(4, Tmax);
}
/**********************/
void loop(){
  print_datetime();
  print_buttons();
  upper_value();
  lower_value();
  read_touch();
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
  int x = 480;
  int y = 125;//+50
  now = rtc.now();
  char date_array[15] = "";
  char time_array[15] = "";
  if(value_edit == 3){
    j++;
    if(j % 2 == 0) myGLCD.setColor(VGA_GREEN);
    else myGLCD.setColor(VGA_YELLOW);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+50 - i);
    }
    sprintf(date_array, "%02d/%02d/%02d", lastDay, lastMonth, lastYear);
    sprintf(time_array, "%3s %02d:%02d", weekDay[now.dayOfWeek()+1], lastHour, lastMinute);
  }
  else{
    myGLCD.setColor(VGA_GREEN);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+50 - i);
    }
    lastDay = now.date(); lastMonth = now.month(); lastYear = now.year();
    lastHour = now.hour(); lastMinute = now.minute();
    sprintf(date_array, "%02d/%02d/%02d", now.date(), now.month(), now.year());
    sprintf(time_array, "%3s %02d:%02d", weekDay[now.dayOfWeek()+1], now.hour(), now.minute());
  }
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print(date_array, x+20, y+10, 0);
  myGLCD.print(time_array, x+28, y+28, 0);
}
void print_buttons(){
  
  myGLCD.drawBitmap(732, 136, 32, 32, up, 1);
  myGLCD.drawBitmap(732, 236, 32, 32, down, 1);
  myGLCD.drawBitmap(732, 336, 32, 32, right, 1);
  myGLCD.drawBitmap(732, 436, 32, 32, deletebtn, 1);
}

void upper_value(){ 
  int x = 480;
  int y = 230; //+70
  char buff[5] = "";
  char temp_array[15] = "";
  if(value_edit == 1){
    j++;
    if(j % 2 == 0) myGLCD.setColor(VGA_RED);
    else myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }
  }
  else{
    lastTmax = Tmax;
    myGLCD.setColor(VGA_RED);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }  
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(Grotesk24x48);
  dtostrf(Tmax, 5, 2, buff);
  sprintf(temp_array, "%s C", buff);
  myGLCD.print(temp_array, x+15, y+12, 0);
  Serial.println(buff);
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Alerta sup.", x+15, y+75, 0);
}

void lower_value(){ 
  int x = 480;
  int y = 350; //+70
  char buff[5] = "";
  char temp_array[15] = "";
  if(value_edit == 2){
    j++;
    if(j % 2 == 0) myGLCD.setColor(VGA_RED);
    else myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }
  }
  else{
    lastTmin = Tmin;
    myGLCD.setColor(VGA_RED);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }  
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(Grotesk24x48);
  dtostrf(Tmin, 5, 2, buff);
  sprintf(temp_array, "%s C", buff);
  myGLCD.print(temp_array, x+15, y+12, 0);
  Serial.println(buff);
  
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Alerta inf.", x+15, y+75, 0);
}
void read_touch(){
  if(myTouch.dataAvailable()){
    myTouch.read();
    xTouch = myTouch.getX();
    yTouch = myTouch.getY();
    Serial.print(xTouch);
    Serial.print(", ");
    Serial.println(yTouch);
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillCircle(xTouch, yTouch, 4);
  }
  //Seleccion de valor a editar
  if(480 < xTouch && xTouch < 480+200){
    if(350 < yTouch && yTouch < 350+70){
      if(value_edit == 0) value_edit = 2;
    }
    else if(230 < yTouch && yTouch < 230+70){
      if(value_edit == 0) value_edit = 1;
    }
    else if(125 < yTouch && yTouch < 125+50){
      if(value_edit == 0){
        value_edit = 3;
        k = 0;
      }
    }
  }
  //Botones para subir o bajar
  else if(700 < xTouch && xTouch < 799){ //Close
    if(420 < yTouch && yTouch < 479){
      if(value_edit == 1){
        Tmax = lastTmax;
      }
      else if(value_edit == 2){
        Tmin = lastTmin;
      }
      else if(value_edit == 3){
        k = 0;
      }
      value_edit = 0;
    }
    if(320 < yTouch && yTouch < 380){ //Next OK
      if(value_edit == 1){
        EEPROM.put(4, Tmax);
        value_edit = 0;
      }
      else if(value_edit == 2){
        EEPROM.put(0, Tmin);
        value_edit = 0;
      }
      else if(value_edit == 3){
        if(k >= 5){
          value_edit = 0;
          k = 0;
          DateTime dt(lastYear, lastMonth, lastDay, lastHour, lastMinute, 00, 1);
          rtc.setDateTime(dt);
        }
        else k++;
      }
    }
    if(220 < yTouch && yTouch < 280){//Down
      if(value_edit == 1){
        Tmax -= 0.05;
      }
      else if(value_edit == 2){
        Tmin -= 0.05;
      }
      else if(value_edit == 3){
        if(k == 0){
          lastDay--;
        }
        if(k == 1){
          lastMonth--;
        }
        if(k == 2){
          lastYear--;
        }
        if(k == 3){
          lastHour--;
        }
        if(k == 4){
          lastMinute--;
        }
      }
    }
    if(120 < yTouch && yTouch < 180){//Uppppp
      if(value_edit == 1){
        Tmax += 0.05;
      }
      else if(value_edit == 2){
        Tmin += 0.05; 
      }
      else if(value_edit == 3){
        //None
      }
    }
  }
  xTouch = 0; yTouch = 0;
}
