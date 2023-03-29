#include <Arduino.h>
#line 1 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
#include <mcp_can.h>
#include <elapsedMillis.h>

#define SPI_CS_CAN 9

//Setup LEDs
#define OUTER_RIGHT 10    // Outer Brake lights
#define INNER_RIGHT 6  // Inner, flashing, brake lights
#define INNER_LEFT 5  // Inner, flashing, brake lights
#define OUTER_LEFT 3    // Outer Brake lights
#define ANALOG_BRIGHT 0
#define ANALOG_DIM 247

MCP_CAN CAN(SPI_CS_CAN);

boolean OUTER_BRAKE_STATE = LOW;
boolean INNER_BRAKE_STATE = LOW;
boolean previous_brake_state = false;
boolean need_to_flash = false;
boolean brake_state = false;

elapsedMillis flash_timer;  // used to verify correct time between flashes
uint8_t num_flashes;


#line 26 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void brake_on();
#line 33 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void brake_off();
#line 39 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void brake_flash();
#line 53 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void rx_message_routine(unsigned char buf[]);
#line 79 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void setup(void);
#line 100 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void loop(void);
#line 26 "/home/mini/workspace/ironpatch/Arduino_klee/BrakeFlasher_AVR_Vuln/BrakeFlasher_AVR_Vuln.ino"
void brake_on(){
  if (!need_to_flash){
    INNER_BRAKE_STATE = HIGH;}
  OUTER_BRAKE_STATE = HIGH;
}


void brake_off(){
  OUTER_BRAKE_STATE = INNER_BRAKE_STATE = LOW;
  num_flashes = 0;  // reset instrumentation
}


void brake_flash(){
  if (flash_timer > 250){
    flash_timer = 0;
    INNER_BRAKE_STATE = !INNER_BRAKE_STATE;
    num_flashes++;
    if (num_flashes > 5) {
      INNER_BRAKE_STATE = LOW;
      num_flashes = 0;
      need_to_flash = false;
    }
  }
}


void rx_message_routine(unsigned char buf[]){
    // buf[3] is the integer value, 0-255km, buff[2] is the decimal
    int16_t speed_value = (buf[3] << 8) + buf[2];  // <-------- Vulnerability here
    uint8_t brake_switch = (buf[4] & 0b00001100) >> 2;
    Serial.print(" Speed = ");
    Serial.print(speed_value/256);
    Serial.print(", brake =");
    Serial.print(brake_switch);
    Serial.println("]");
    if (brake_switch){
      brake_state = true;
      brake_on();
      if (speed_value > 0 && previous_brake_state != brake_state){  // speed > 0 and brakes were off last
        need_to_flash = true;
        Serial.println("Flashing=true");
      }
    }
    else{
      brake_state = false;
      need_to_flash = false;
      brake_off();
    }
    previous_brake_state = brake_state;
}


void setup(void) {
  pinMode(INNER_RIGHT, OUTPUT);
  pinMode(INNER_LEFT, OUTPUT);
  pinMode(OUTER_RIGHT, OUTPUT);
  pinMode(OUTER_LEFT, OUTPUT);

  //Initialize the CAN channels with autobaud.
  Serial.begin(115200);
  while (CAN_OK != CAN.begin(CAN_250KBPS)){
    Serial.println("CAN BUS Shield init fail");
    Serial.println("Init CAN BUS Shield again");
    delay(100);
  }

  CAN.init_Mask(0,1,0xFFFFFF);  // Disabling lets any CAN message trigger this
  CAN.init_Mask(1,1,0xFFFFFF);
  CAN.init_Filt(0,1,0xFEF100);
  Serial.println("CAN BUS Shield init ok!");
}


void loop(void) {
  unsigned char len = 0;
  unsigned char buf[8];
  unsigned long id = 0;
  unsigned int timer = 0;

  if (CAN_MSGAVAIL == CAN.checkReceive()){
    CAN.readMsgBufID(&id, &len, buf);
    Serial.print("CANFrame=[");
    Serial.print(id);
    Serial.print(",");
    for (int i = 0; i < len; i++ ){
      Serial.print(buf[i]);
      Serial.print(",");
    }

    timer = micros();
    rx_message_routine(buf);
    timer = micros() - timer;
    Serial.print("Rx_routine_timer=");
    Serial.println(timer, DEC);
  }

  if (need_to_flash){
    brake_flash(); }

  analogWrite(INNER_RIGHT, INNER_BRAKE_STATE ? ANALOG_BRIGHT : ANALOG_DIM);
  analogWrite(INNER_LEFT, INNER_BRAKE_STATE ? ANALOG_BRIGHT : ANALOG_DIM);
  analogWrite(OUTER_RIGHT, OUTER_BRAKE_STATE ? ANALOG_BRIGHT : ANALOG_DIM);
  analogWrite(OUTER_LEFT, OUTER_BRAKE_STATE ? ANALOG_BRIGHT : ANALOG_DIM);
}

