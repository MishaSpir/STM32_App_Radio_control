import processing.serial.*;
import controlP5.*;
import java.util.*;


ControlP5 cp5;



enum State {
  IDLE, // константы обычно в UPPER_CASE
    DATA, 
    FIN, 
    ERR
};

// Использование
State state = State.IDLE;  // через имя enum

long last_time = 0;
boolean flag = false;

Serial stm_serial;   
String ports[] = Serial.list();

void setup() {
  size(800, 400);
  //stm_serial = new Serial(this,"/dev/uart2usb", 9600); 


  cp5 = new ControlP5(this);
  cp5.setFont(createFont("Ubuntu Regular", 16));


  cp5.addButton("refresh").linebreak().setPosition(130, 20).setSize(100, 20);
  cp5.addButton("open_port").linebreak().setPosition(130, 50).setSize(100, 20);
  cp5.addButton("close_port").linebreak().setPosition(130, 80).setSize(100, 20);
  cp5.addButton("led_on").linebreak().setPosition(130, 110).setSize(100, 20);
  cp5.addButton("led_off").linebreak().setPosition(130, 140).setSize(100, 20);
  cp5.addScrollableList("comlist").setPosition(0, 0).setSize(120, 100).setItemHeight(19).setBarHeight(20).linebreak().close();
  // .setType(ScrollableList.LIST) // currently supported DROPDOWN and LIST
  
  cp5.addButton("send_angle").linebreak().setPosition(130, 200).setSize(100, 20);

  cp5.addKnob("knob")
    .setRange(0, 1000)
    .setValue(50)
    .setPosition(4, 100)
    .setRadius(50)
    .setDragDirection(Knob.VERTICAL)
    ;
}

void refresh() {
  cp5.get(ScrollableList.class, "comlist").clear();
  String list[] = Serial.list();
  cp5.get(ScrollableList.class, "comlist").addItems(list);
}

String PortName ;
void comlist(int n) {
  PortName = Serial.list()[n];
}

boolean port_is_open = false;
void open_port() {
  stm_serial = new Serial(this, PortName, 9600); 
  stm_serial.clear();
  port_is_open = true;
}

void close_port() {
  stm_serial.clear();
  stm_serial.stop();
  port_is_open = false;
}
void led_on() {
  stm_serial.write("$0,0*");
}

void led_off() {
  stm_serial.write("$0,1*");
}

String str;
String str_angle;
void knob(int val) {
   str = "$1," + val + "*";
   str_angle = "$2," + val + "*";
  if (stm_serial!= null) {
    stm_serial.write(str);
  }
}

void send_angle(){
  stm_serial.write(str_angle);
}


int  BUF_SIZE = 32;
int c = 0;
char[] buf = new char[BUF_SIZE];
void clearBuf() {
  for (int i = 0; i<buf.length; i++) {
    buf[i]='\0';
  }
}


void draw() {
  //write может писать как массив байтов так и отдельные символы
  background(120);
  textSize(15);


  if (port_is_open) {  
    text("Порт открыт", 140, 15);

    if (stm_serial !=null) {   //проверка, существует ли вообще стм_сериал
      if (stm_serial.available()>0) {
        String in_str = stm_serial.readStringUntil('*');
        if (in_str!=null) {
          println(in_str);
          clearBuf();

          //// //КОНЕЧНЫЙ АВТОМАТ
          for (int i = 0; i< in_str.length(); i++) {
            char ch = in_str.charAt(i);
            switch(state) {
            case IDLE:
              if (ch == '$') {
                state = State.DATA;
              }// Принят маркер, переходим к приёму данных
              else {
              }
              break;
            case DATA:
              if (ch == '*') {
                state = State.FIN;
              }//Принят терминатор, заканчиваем приём данных
              else if (ch == '$') {
                state = State.ERR;
              } else {
                buf[c++] = ch;
                c %= BUF_SIZE;
                println(ch);
                //       // usart_send_blocking(USART3,ch);
              }
              break;
            case FIN:
              println("fin");
              c = 0;
              if (ch == '$') {
                state = State.DATA;
              } else {
                state = State.ERR;
              } 
              break;
            case ERR:
              if (ch == '$') {
                state = State.DATA;
              }
              println("err");
              break;
            default:
            }
          }
        }
      }


      text("Принятая строка: ", 150, 300);
      for (int n =0; n <BUF_SIZE; n++  ) {
        if (buf[n]== '\0') continue;

        text(buf[n], 300+10*n, 300);
      }

      String for_split = new String(buf);
      for_split = for_split.replace("\0", ""); // удаляем нулевые символы
      String[] data_arr = for_split.split(",");

      text("Числа после парсинга: ", 120, 340);
      for (int n =0; n <data_arr.length; n++  ) {
        text(data_arr[n], 300+40*n, 340);
      }

      //println(buf[0]);
    }
  } else {
    text("Порт закрыт", 140, 15);
  }




  if (millis() - last_time >= 1000) {
    last_time = millis();
    flag =!flag;
  }
}
