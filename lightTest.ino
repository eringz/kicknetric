#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/*
  INITIATING VALUE FOR INPUT PINS FROM TOGGLE SWITCH AS D2 AND D4
*/
const int D2 = 2;   
const int D4 = 4;      
/*
  CREATING GROUP FOR 13 PINS FOR SIGNAL LIGHTS AND TAIL LIGHTS 
*/
const int pins[] = {5,18,19,21,22,23,13,12,14,27,26,25,32};
const byte pins_length = 12;

// STORING VALUE OF SPEED IN RUNNING AND BLINKING
const int speed = 70; 

/*
  *DEFINING PINS FOR TRANSMITTER IN PIN 16 AND RECEIVER FOR PIN 17
  *DECLARING DISTANCE MEASURMENTS OF LIDAR, SIGNAL STRENGTH AND TEMPERATURE
  *SAVING TO CHECK VALUE, DATA MEASURED BY LIDAR, FRAME HEADER OF DATA PACKAGE AND RECEIVE STATE FOR FRAME
*/
#define RXD2 16
#define TXD2 17
int dist; 
int strength; 
float temprature;
unsigned char check, uart[9]; //CHECK AND DATA
const int HEADER=0x59; // FRAME HEADER
int rec_debug_state = 0x01;// RECEIVE STATE
int i;

void setup() {
  /*
    *SET THE TOGGLE PIN (2) AS INPUT WITH INTERNAL PULL-UP RESISTOR
    *SET THE GROUP OF PINS AS OUTPUT AS LOOP
    *DEFAULT VALUE OF DELAY FOR LIDAR IN 2 SECONDS
    *SETTING THE RATE OF 2 SIGNAL CHANGE TO COMMUNICATING CHANNEL INTO 115200 BITS PER SECOND FOR UART BENEWAKE LIDAR
  */
  // pinMode(D2, INPUT_PULLUP);
  // pinMode(D4, INPUT_PULLUP);
  for (int i = 0; i <= pins_length; i++) {
    pinMode(pins[i], OUTPUT);
  }
  delay(2000);
	Serial.begin(115200);
	Serial2.begin(115200);
}

void loop() {
  /*
    *GETTING THE DISTANCE, STRENGTH AND TEMPERATURE FROM LIDAR SENSOR
    *LIGHTING LAYER OF HAZZARD DEPENDS ON LIDAR DISTANCE
  */
  GET_LIDAR_DATA();
  
  
  /*
    * READING THE TOGGLE SWITCH ACCORDING TO STATE OF DIRECTIONS
    * D2 PIN AS LEFT
    * D4 PIN AS RIGHT
  */
  int LEFT = digitalRead(D2); 
  int RIGHT = digitalRead(D4); 

  /*
    *YELLOW LIGHTS RUNNING TO LEFT WHEN TOGGLE SWITCH IS ON LEFT SIDE
    *YELLOW LIGHTS RUNNING TO RIGHT WHEN TOGGLE SWITCH IS ON RIGHT SIDE
    *NO YELLOW LIGHTS RUNNING WHEN TOGGLE SWITCH IS ON CENTER
  */
  if (LEFT == HIGH) {
    for(int i = 9; i>=0; i--) {
      digitalWrite(pins[i], 0);
      digitalWrite(pins[i-1], 1);
      digitalWrite(pins[i-2], 1);
      delay(speed);
    }
    // for(int i = 0; i<=2; i++){
    //   digitalWrite(pins[i], 1);
    // }
    // delay(speed);
    // for(int i = 0; i<=2; i++){
    //   digitalWrite(pins[i], 0);
    // }
    // delay(speed);
    Serial.print("TURN LEFT");
    Serial.print("\n");
  } 
  else if(RIGHT == HIGH) { 
    for(int i = 0; i<=9; i++) {
      digitalWrite(pins[i], 0);
      digitalWrite(pins[i+1], 1);
      digitalWrite(pins[i+2], 1);
      delay(speed);
    }
    // for(int i = 9; i>=7; i--){
    //   digitalWrite(pins[i], 1);
    // }
    // delay(speed);
    // for(int i = 9; i>=7; i--){
    //   digitalWrite(pins[i], 0);
    // }
    // delay(speed);
    Serial.print("TURN RIGHT");
    Serial.print("\n");
    
  }

  // RED LIGHTS IS ON WHEN IN POWER
  digitalWrite(pins[10], 1);
  digitalWrite(pins[11], 1);
  digitalWrite(pins[12], 1);
}

/*
  *MAKING A FUNCTION THAT GETTING THE DATA FROM LIDAR AND SEND IT TO SERIAL PORT
  *RECEIVING DEBUG STATE FROM 0X01-0X09
*/
void GET_LIDAR_DATA(){
  if (Serial2.available()) { //check if serial port has data input
    if(rec_debug_state == 0x01) {  //the first byte
      uart[0]=Serial2.read();
      if(uart[0] == 0x59) {
        check = uart[0];
        rec_debug_state = 0x02;
      }
    }else if(rec_debug_state == 0x02) {//the second byte
      uart[1]=Serial2.read();
      if(uart[1] == 0x59) {
        check += uart[1];
        rec_debug_state = 0x03;
      }else{
        rec_debug_state = 0x01;
      }
    }else if(rec_debug_state == 0x03) {
      uart[2]=Serial2.read();
      check += uart[2];
      rec_debug_state = 0x04;
    }else if(rec_debug_state == 0x04) {
      uart[3]=Serial2.read();
      check += uart[3];
      rec_debug_state = 0x05;
    }else if(rec_debug_state == 0x05) {
      uart[4]=Serial2.read();
      check += uart[4];
      rec_debug_state = 0x06;
    }else if(rec_debug_state == 0x06) {
      uart[5]=Serial2.read();
      check += uart[5];
      rec_debug_state = 0x07;
    }else if(rec_debug_state == 0x07) {
      uart[6]=Serial2.read();
      check += uart[6];
      rec_debug_state = 0x08;
    }else if(rec_debug_state == 0x08) {
      uart[7]=Serial2.read();
      check += uart[7];
      rec_debug_state = 0x09;
    }else if(rec_debug_state == 0x09) {
      uart[8]=Serial2.read();
      if(uart[8] == check) {
        dist = uart[2] + uart[3]*256;//the distance
        strength = uart[4] + uart[5]*256;//the strength
        temprature = uart[6] + uart[7] *256;//calculate chip temprature
        temprature = temprature/8 - 256; 
        // Serial.print("dist = ");
        // Serial.print(dist); //output measure distance value of LiDAR
        // Serial.print('\n');
        // Serial.print("strength = ");
        // Serial.print(strength); //output signal strength value
        // Serial.print('\n');
        // Serial.print("\t Chip Temprature = ");
        // Serial.print(temprature);
        // Serial.println(" celcius degree"); //output chip temperature of Lidar                                                       
        while(Serial2.available()) {
          Serial2.read();
        } // This part is added becuase some previous packets are there in the buffer so to clear serial buffer and get fresh data.
        delay(100);                                           
      }
      //91.44
      if(dist <= 210) {
          for(int i = 0; i <= 12; i++) {
            digitalWrite(pins[i], 1);
          }
          delay(speed);
          for(int i = 0; i <= 12; i++) {
            digitalWrite(pins[i], 0);
          }
          delay(speed);
        }else if(dist <= 250 && dist > 210) {
          for(int i = 0; i <= 9; i++) {
            digitalWrite(pins[i], 1);
          }
          delay(speed);
          for(int i = 0; i <= 9; i++) {
            digitalWrite(pins[i], 0);
          }
          delay(speed);
        }
      rec_debug_state = 0x01;
    }
  }
}
