#include <Servo.h>
#include "HX711.h"

// PINS
const byte BUTTON_RESET_PESO = 2;
const byte BUTTON_RESET_WEIGHT = 3;
#define LOADCELL_SCK_PIN 4
#define LOADCELL_DOUT_PIN 5
#define LED_ACCEPT 6
#define LED_REJECT 7
#define INDUC_PROX_PIN 8
#define SERVO_RAMP_PIN 9
#define SERVO_PLATE_PIN 10

// SERVO PLATE ANGLE
#define SERVO_PLATE_DEFAULT_ANGLE 88
#define SERVO_PLATE_ACCEPT_ANGLE 180
#define SERVO_PLATE_REJECT_ANGLE 0
#define SERVO_PLATE_DELAY 15

// SERVO RAMP ANGLE
#define SERVO_RAMP_25c_ANGLE 75
#define SERVO_RAMP_5p_ANGLE 104
#define SERVO_RAMP_20p_ANGLE 130

// LOAD CELL
#define WEIGHT_CALIBRATION_FACTOR 8500
#define WEIGHT_DEVIATION 0.75
#define WEIGHT_25c 3.6
#define WEIGHT_5p 7.4
#define WEIGHT_20p 11.5
HX711 scale;

Servo servo_plate;
Servo servo_ramp;

int plate_accept = 0;
int plate_reject = 0;
float peso_value = 0;
String input;

void setup() {
  Serial.begin(9600);
  
  servo_plate.attach(SERVO_PLATE_PIN);
  servo_plate.write(SERVO_PLATE_DEFAULT_ANGLE);
  
  servo_ramp.attach(SERVO_RAMP_PIN);
  servo_ramp.write(SERVO_RAMP_5p_ANGLE);

  pinMode(INDUC_PROX_PIN, INPUT);
  pinMode(BUTTON_RESET_PESO, INPUT);
  pinMode(BUTTON_RESET_WEIGHT, INPUT);

  pinMode(LED_ACCEPT, OUTPUT);
  pinMode(LED_REJECT, OUTPUT);

  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(WEIGHT_CALIBRATION_FACTOR);
  scale.tare();

  attachInterrupt(digitalPinToInterrupt(BUTTON_RESET_PESO), reset_peso, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_RESET_WEIGHT), reset_weight, RISING);

  Serial.println("=======================================");
}

void loop() {

  int induc_prox_detect = digitalRead(INDUC_PROX_PIN);

  Serial.println(scale.get_units(), 3);

  if (induc_prox_detect == 0){
    Serial.println("Metallic object detected");
    
    delay(3000);

    Serial.println(scale.get_units(), 3);

    if ((scale.get_units() < (WEIGHT_25c + WEIGHT_DEVIATION)) && (scale.get_units() > (WEIGHT_25c - WEIGHT_DEVIATION))){
      Serial.println("25 cent coin detected");
      servo_ramp.write(SERVO_RAMP_25c_ANGLE);
      peso_value += 0.25;
      Serial.print("New peso value: ");
      Serial.println(peso_value);
      digitalWrite(LED_ACCEPT, HIGH);
      digitalWrite(LED_REJECT, LOW);
      plate_accept = 1;
    } else if ((scale.get_units() < (WEIGHT_5p + WEIGHT_DEVIATION)) && (scale.get_units() > (WEIGHT_5p - WEIGHT_DEVIATION))){
      Serial.println("5 peso coin detected");
      servo_ramp.write(SERVO_RAMP_5p_ANGLE);
      peso_value += 5;
      Serial.print("New peso value: ");
      Serial.println(peso_value);
      digitalWrite(LED_ACCEPT, HIGH);
      digitalWrite(LED_REJECT, LOW);
      plate_accept = 1;
    } else if ((scale.get_units() < (WEIGHT_20p + WEIGHT_DEVIATION)) && (scale.get_units() > (WEIGHT_20p - WEIGHT_DEVIATION))){
      Serial.println("20 peso coin detected");
      servo_ramp.write(SERVO_RAMP_20p_ANGLE);
      peso_value += 20;
      Serial.print("New peso value: ");
      Serial.println(peso_value);
      digitalWrite(LED_ACCEPT, HIGH);
      digitalWrite(LED_REJECT, LOW);
      plate_accept = 1;
    } else {
      Serial.println("Object rejected");
      digitalWrite(LED_ACCEPT, LOW);
      digitalWrite(LED_REJECT, HIGH);
      plate_reject = 1;
    }
  } else if (scale.get_units() > 10){
      Serial.println("Object rejected");
      digitalWrite(LED_ACCEPT, LOW);
      digitalWrite(LED_REJECT, HIGH);
      plate_reject = 1;
  }
  
  if (plate_accept == 1){
    for (int i = SERVO_PLATE_DEFAULT_ANGLE; i <= SERVO_PLATE_ACCEPT_ANGLE; i += 1) {
      servo_plate.write(i);
      delay(SERVO_PLATE_DELAY);
    }
    delay(1000);
    for (int i = SERVO_PLATE_ACCEPT_ANGLE; i >= SERVO_PLATE_DEFAULT_ANGLE; i -= 1) {
      servo_plate.write(i);
      delay(SERVO_PLATE_DELAY);
    }
  } else if (plate_reject == 1){
    for (int i = SERVO_PLATE_DEFAULT_ANGLE; i >= SERVO_PLATE_REJECT_ANGLE; i -= 1) {
      servo_plate.write(i);
      delay(SERVO_PLATE_DELAY);
    }
    delay(1000);
    for (int i = SERVO_PLATE_REJECT_ANGLE; i <= SERVO_PLATE_DEFAULT_ANGLE; i += 1) {
      servo_plate.write(i);
      delay(SERVO_PLATE_DELAY);
    }
  }
  plate_accept = 0;
  plate_reject = 0;
  digitalWrite(LED_ACCEPT, LOW);
  digitalWrite(LED_REJECT, LOW);

  delay(1000);
}

void reset_peso(){
  peso_value = 0;
  Serial.print("Peso value reset to zero: ");
  Serial.println(peso_value);
}

void reset_weight(){
  Serial.print("Weight reset");
  scale.tare();
}

/*
======================================================================================================================
Logic Flow/Pseudocode:
  once an object has been detected on the receiving platform, the sorting mechanism must begin automatically
  sense weight by load cell
  sense magnetic properties by hall effect sensor
  create coin database --> determine if object is coin or not based on standard weights of 0.25, 5, and 20 peso coin
  
  if coin{
    use green LED to indicate
    rotate servo_plate and servo_ramp to proper coin bin
    add coin value to sum
  } else{
    use red LED to indicate
    rotate servo_plate to reject bin
  }

Special stuff:
  Placing a coin/object on the receiving area while sorting/storing is taking place
    i. The machine may either wait for the current sorting; or
    ii. The machine may stop and send out an error signal; or
    iii. Sort the new coin/object at the same time (harder)

  reset button to reset total coin value
======================================================================================================================
*/