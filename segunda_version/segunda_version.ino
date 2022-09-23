#include <Wire.h>
#include "Sodaq_DS3231.h"
#include <UTFT.h>
#include <UTouch.h>
#include <EEPROM.h>
#include <SPI.h>
#include <avr/pgmspace.h>
#include <Adafruit_MAX31865.h>
#include <pt100rtd.h>
#define RREF 430.0
#define RNOMINAL 100.0
#define XMAX 799
#define YMAX 479
#define myRound(R) round(R * 20.0) / 20.0
#define C2F(c) ((9 * c / 5) + 32)
extern uint8_t BigFont[];
//extern uint8_t SmallFont[];
//extern uint8_t SixteenSegment40x60[];
//extern uint8_t Dingbats1_XL[];
extern uint8_t Grotesk24x48[];
extern uint8_t SevenSegment96x144Num[];
extern unsigned short deletebtn[];
extern unsigned short down[];
extern unsigned short right[];
extern unsigned short up[];
extern unsigned short buzzer_on[];
extern unsigned short buzzer_off[];
unsigned int xTouch, yTouch, k = 0;
unsigned int lastDay, lastMonth, lastYear, lastHour, lastMinute;
unsigned long j = 0, lastMillis = -5000, soundMillis = millis();
float Tmin, Tmax, lastTmin, lastTmax, temperature, Offset, lastOffset;
unsigned int value_edit = 0;
int a, b, c, d, intTemperature, decTemperature, buzz;
char weekDay[][4] = {"Lun", "Mar", "Mie", "Jue", "Vie", "Sab", "Dom"};
Adafruit_MAX31865 max = Adafruit_MAX31865(4, 5, 6, 7);
//Adafruit_MAX31865 thermo=Adafruit_MAX31865(10);
UTFT myGLCD(SSD1963_800,38,39,40,41);  //(byte model, int RS, int WR, int CS, int RST, int SER)
UTouch  myTouch( 43, 42, 44, 45, 46);  //byte tclk, byte tcs, byte din, byte dout, byte irq
pt100rtd PT100 = pt100rtd();
DateTime now;
void setup() {
  EEPROM.get(0, Tmin); EEPROM.get(4, Tmax); EEPROM.get(8, Offset);
  EEPROM.get(12, buzz);
  max.begin(MAX31865_2WIRE);
  Serial.begin(57600);
  Wire.begin();
  rtc.begin();
  //rtc.setDateTime(dt); //Ajustar fecha - hora
  pinMode(8, OUTPUT);  //backlight
  pinMode(3,OUTPUT); //Buzzzzzzzzzzzzzzzzzzzzzz
  digitalWrite(8, HIGH);//on
  myGLCD.InitLCD(1);
  myGLCD.clrScr();
  myTouch.InitTouch();
  myTouch.setPrecision(PREC_MEDIUM);
  start_screen(); //pantalla incial
  print_rectangles();
  print_buzzer();
  Serial.println(buzz);
}
/**********************/
void loop(){
  read_touch();
  print_datetime();
  //print_buttons(); //Dosent make sense for me, estara adentro de start screen
  upper_value();
  lower_value();
  print_offset();
  //print_buzzer();
  sound_buzzer();
  if(millis() - lastMillis > 5000){
    print_temperature();
    lastMillis = millis();
  }
}
void print_offset(){
  int x = 20;
  int y = 375;
  char buff[5] = "";
  char offset_array[15] = "";
  if(value_edit == 4){
    j++;
    if(j % 2 == 0) myGLCD.setColor(VGA_RED);
    else myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+150 - i, y+70 - i);
    }
  }
  else{
    lastOffset = Offset;
    /* En start screen para ahorrar...
    int x = 20;
    int y = 375;
    myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+150 - i, y + 70 - i);
    }
    */
  }
  
  myGLCD.setFont(Grotesk24x48);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setColor(0, 0, 0);
  dtostrf(Offset, 5, 2, buff);
  sprintf(offset_array, "%s", buff);
  myGLCD.print(offset_array, x+18, y+12, 0);
   /*Ahorrar tiempo...
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setColor(0, 0, 0);
  myGLCD.print("CORRECCION", x - 5, y+10+70, 0);
  */
}
void print_buzzer(){//Creo que solo una vez al inciar y al cambiar de estado nomas
  int x = 320;
  int y = 375;
  myGLCD.setColor(0, 0, 0);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(x -1 - i, y -1 - i, x+107 + i, y + 80 + i);
  }
  if(buzz){
    myGLCD.drawBitmap(x, y, 107, 80, buzzer_on, 1);
  }
  else{
    myGLCD.drawBitmap(x, y, 107, 80, buzzer_on, 1); 
    //Se imprime al pulsar el boton
    myGLCD.setColor(255, 0, 0);
    myGLCD.setBackColor(255, 0 ,0);
    for(int i = 0; i < 100; i++){
      //int x = 320;
      //int y = 375;
      myGLCD.drawLine(x + 5 + i, y + 5, x + 107 - i - 5, y + 80 - 5);
    }
  }
}
void sound_buzzer(){
  if(buzz & (temperature < Tmin || Tmax < temperature) & millis() - soundMillis > 1000){
    digitalWrite(3, !digitalRead(3));
    soundMillis = millis();
  }
  else digitalWrite(3, LOW);
  //Serial.println(digitalRead(3));
}
void print_temperature(){
  int x = 15;
  int y = 180;

  uint16_t rtd, ohmsx100 ;
  uint32_t dummy ;
  float ohms, Tlut ;  
  float Tcvd, Tcube, Tpoly, Trpoly ;
  rtd = max.readRTD();
  dummy = ((uint32_t)(rtd << 1)) * 100 * ((uint32_t) floor(RREF)) ;
  dummy >>= 16 ;
  ohmsx100 = (uint16_t) (dummy & 0xFFFF) ;
  // or use exact ohms floating point value.
  ohms = (float)(ohmsx100 / 100) + ((float)(ohmsx100 % 100) / 100.0) ;
  temperature  = PT100.celsius_cvd(ohms) + Offset;
  /*Tlut  = PT100.celsius(ohmsx100) ;     // NoobNote: LUT== LookUp Table
  Tcvd  = PT100.celsius_cvd(ohms) ;         // Callendar-Van Dusen calc
  Tcube = PT100.celsius_cubic(ohms) ;       // Cubic eqn calc
  Tpoly = PT100.celsius_polynomial(ohms) ;        // 5th order polynomial
  Trpoly  = PT100.celsius_rationalpolynomial(ohms) ;  // ugly rational polynomial quotient
  */
  Serial.println(temperature);
  temperature = myRound(temperature);
  a = temperature / 10;
  b = (temperature - a * 10);
  c = temperature * 10 - a * 100 - b *10;
  d = temperature * 100 - a * 1000 - b * 100 - c * 10;
  intTemperature = a*10 + b;
  decTemperature = c*10 + d;
  myGLCD.setColor(0, 255, 0);
  /* Se imprime una sola vez para acelerar el proceso en la funcions starscreen
  int x = 15;
  int y = 180;
  for(int i = -5; i < 5; i++){
    myGLCD.drawRect(x + i, y + i, x + 10 + 4*96+35 - i, y + 10 +154 - i);
  }
  */ 
  if(value_edit == 0){//solo se actualiza la tenmperatura en caso no se edite ningun valor

  //Se imprime una sola vez para acelerar el proceso en la funcions starscreen
    if(temperature < Tmin || Tmax < temperature) myGLCD.setColor(VGA_RED);
    for(int i = -5; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x + 10 + 4*96+35 - i, y + 10 +154 - i);
    }
    
    myGLCD.setFont(Grotesk24x48);
    myGLCD.setColor(0, 0, 0);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.print("Temperatura Actual", x, y - 48 - 10, 0);
    
    
    myGLCD.setFont(SevenSegment96x144Num);
    myGLCD.setColor(0, 0, 0);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(intTemperature, x + 10, y + 10, 2, '0');
  
    myGLCD.setFont(Grotesk24x48);
    myGLCD.setColor(0, 0, 0);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.print(".", x + 10 + 2*96, y + 10 + 144 - 48, 0);
  
    myGLCD.setFont(SevenSegment96x144Num);
    myGLCD.setColor(0, 0, 0);
    myGLCD.setBackColor(255, 255, 255);
    myGLCD.printNumI(decTemperature, x + 10 + 2*96 + 25, y + 10, 2, '0');
  }
}
void print_rectangles(){
  //De upper value
  int x = 480;
  int y = 220; //+70
  myGLCD.setColor(VGA_BLUE);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("ALERTA SUP.", x+15, y+75, 0);
  

  //De print datetime
  x = 480;
  y = 120;//+50
  myGLCD.setColor(VGA_GREEN);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(x + i, y + i, x+200 - i, y+50 - i);
  }

  //De lower value
  x = 480;
  y = 350; //+70
  myGLCD.setColor(VGA_BLUE);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("ALERTA INF.", x+15, y+75, 0);

  
  //De offset
  x = 20;
  y = 375;
  myGLCD.setColor(VGA_BLUE);
  for(int i = 0; i < 5; i++){
    myGLCD.drawRect(x + i, y + i, x+150 - i, y + 70 - i);
  }
  myGLCD.setFont(BigFont);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setColor(0, 0, 0);
  myGLCD.print("CORRECCION", x - 5, y+10+70, 0);
  

  
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

  print_buttons();

  //De print_buzzer
  myGLCD.setColor(0, 255, 0);
  myGLCD.fillRect(305, 365, 10 + 4*96+45, 465);

  //Se translado a print_rectangles
}
void print_datetime(){ //leer e imprimir hora y fecha
  int x = 480;
  int y = 120;//+50
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
    // Dato a editar de acuerdo a K
    switch(k){
      case 0:
        sprintf(date_array, "%02d/%2s/%4s", lastDay, "--", "----");
        sprintf(time_array, "%3s %2s:%2s", weekDay[now.dayOfWeek()], "--", "--");
        break;
      case 1:
        sprintf(date_array, "%2s/%02d/%4s", "--", lastMonth, "----");
        sprintf(time_array, "%3s %2s:%2s", weekDay[now.dayOfWeek()], "--", "--");
        break;
      case 2:
        sprintf(date_array, "%2s/%2s/%02d", "--", "--", lastYear);
        sprintf(time_array, "%3s %2s:%2s", weekDay[now.dayOfWeek()], "--", "--");
        break;
      case 3:
        sprintf(date_array, "%2s/%2s/%4s", "--", "--", "----");
        sprintf(time_array, "%3s %02d:%2s", weekDay[now.dayOfWeek()], lastHour, "--");
        break;
      case 4:
        sprintf(date_array, "%2s/%2s/%4s", "--", "--", "----");
        sprintf(time_array, "%3s %2s:%02d", weekDay[now.dayOfWeek()], "--", lastMinute);
        break;
    }
      
  }
  else{
    /* En start_screen para ahorrar tiempo
    int x = 480;
    int y = 120;//+50
    myGLCD.setColor(VGA_GREEN);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+50 - i);
    }
    */
    lastDay = now.date(); lastMonth = now.month(); lastYear = now.year();
    lastHour = now.hour(); lastMinute = now.minute();
    sprintf(date_array, "%02d/%02d/%02d", now.date(), now.month(), now.year());
    sprintf(time_array, "%3s %02d:%02d", weekDay[now.dayOfWeek()], now.hour(), now.minute());
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
  int y = 220; //+70
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
    /*En start screen para ahorrar tiempo
    int x = 480;
    int y = 220; //+70
    myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }
    */  
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(Grotesk24x48);
  dtostrf(Tmax, 5, 2, buff);
  sprintf(temp_array, "%s C", buff);
  myGLCD.print(temp_array, x+15, y+12, 0);
  //Serial.println(buff);

  /*Ahorrar tiempo...
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Alerta sup.", x+15, y+75, 0);
  */
  
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
    /*En start screen para ahorrar tiempo
    int x = 480;
    int y = 350; //+70
    myGLCD.setColor(VGA_BLUE);
    for(int i = 0; i < 5; i++){
      myGLCD.drawRect(x + i, y + i, x+200 - i, y+70 - i);
    }
    */  
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(Grotesk24x48);
  dtostrf(Tmin, 5, 2, buff);
  sprintf(temp_array, "%s C", buff);
  myGLCD.print(temp_array, x+15, y+12, 0);
  //Serial.println(buff);


  /*Ahorar Tiempo
  myGLCD.setColor(0, 0, 0);
  myGLCD.setBackColor(255, 255, 255);
  myGLCD.setFont(BigFont);
  myGLCD.print("Alerta inf.", x+15, y+75, 0);
  */
}
void read_touch(){
  if(myTouch.dataAvailable()){
    myTouch.read();
    xTouch = myTouch.getX();
    yTouch = myTouch.getY();
    /*Serial.print(xTouch);
    Serial.print(", ");
    Serial.println(yTouch);
    myGLCD.setColor(0, 0, 0);
    myGLCD.fillCircle(xTouch, yTouch, 4);*/
  }
  //Seleccion de valor a editar
  int x = 20;
  int y = 375;
  //Boton de correccion
  if(20 < xTouch && xTouch < 20 + 150){
    if(375 < yTouch && yTouch < 375+70){
      if(value_edit == 0) value_edit = 4;
    }
  }
  //Boton de 
  else if(480 < xTouch && xTouch < 480+200){
    //Alerta inferior
    if(350 < yTouch && yTouch < 350+70){
      if(value_edit == 0) value_edit = 2;
    }
    //Alerta superior
    else if(230 < yTouch && yTouch < 230+70){
      if(value_edit == 0) value_edit = 1;
    }
    //Fecha
    else if(125 < yTouch && yTouch < 125+50){
      if(value_edit == 0){
        value_edit = 3;
        k = 0;
        //

        //
      }
    }
  }
  //Buzz boton
  else if(320 < xTouch && xTouch < 320 + 107){
    if(375 < yTouch && yTouch < 375+80){
      if(buzz){
        buzz = 0;
        EEPROM.put(12, buzz);
        myGLCD.setColor(255, 0, 0);
        myGLCD.setBackColor(255, 0 ,0);
        for(int i = 0; i < 100; i++){
          int x = 320;
          int y = 375;
          myGLCD.drawLine(x + 5 + i, y + 5, x + 107 - i - 5, y + 80 - 5);
    }
      }
      else{
        buzz = 1;
        EEPROM.put(12, buzz);
        /*
        myGLCD.setColor(255, 255, 255);
        myGLCD.setBackColor(255, 255, 255);
        for(int i = 0; i < 100; i++){
          int x = 320;
          int y = 375;
          myGLCD.drawLine(x + 5 + i, y + 5, x + 107 - i - 5, y + 80 - 5);
        }*/
        print_buzzer();
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
      else if(value_edit == 4){
        Offset = lastOffset;
      }
      value_edit = 0;
      print_rectangles();
    }
    if(320 < yTouch && yTouch < 380){ //Next OK
      print_rectangles();
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
      else if(value_edit == 4){
        EEPROM.put(8, Offset);
        value_edit = 0;
        lastMillis = 0;
      }
    }
    if(220 < yTouch && yTouch < 280){//Down
      lastMillis = millis();
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
        else if(k == 1){
          lastMonth--;
        }
        else if(k == 2){
          lastYear--;
        }
        else if(k == 3){
          lastHour--;
        }
        else if(k == 4){
          lastMinute--;
        }
      }
      else if(value_edit == 4){
        Offset -= 0.05;
      }
    }
    if(120 < yTouch && yTouch < 180){//Uppppp
      lastMillis = millis();
      if(value_edit == 1){
        Tmax += 0.05;
      }
      else if(value_edit == 2){
        Tmin += 0.05; 
      }
      else if(value_edit == 3){
        if(k == 0){
          lastDay++;
        }
        else if(k == 1){
          lastMonth++;
        }
        else if(k == 2){
          lastYear++;
        }
        else if(k == 3){
          lastHour++;
        }
        else if(k == 4){
          lastMinute++;
        }
      }
      else if(value_edit == 4){
        Offset += 0.05;
      }
    }
  }
  xTouch = 0; yTouch = 0;
}
