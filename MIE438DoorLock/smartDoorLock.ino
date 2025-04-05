#include "pitches.h"
//#include "esp32_camera.ino"  --> Used if library is not part of current sketch
// 
faceDetection doorScan;

//-------------- Def

#define PIN_SPEAKER 13

int ringMelody[] = {
  NOTE_REST, NOTE_E6, NOTE_REST, NOTE_D6, NOTE_REST, NOTE_CS6, NOTE_REST, NOTE_D6, NOTE_REST, NOTE_E6,
  NOTE_REST, NOTE_FS6, NOTE_REST, NOTE_GS6, NOTE_REST, NOTE_E6, NOTE_REST, NOTE_FS6, NOTE_REST, NOTE_E6,
  NOTE_REST, NOTE_D6, NOTE_D6, NOTE_REST, NOTE_CS6, NOTE_REST, NOTE_B5, NOTE_REST, NOTE_CS6, NOTE_REST,
  NOTE_D6, NOTE_REST, NOTE_A6, NOTE_REST, NOTE_E6, NOTE_D6, NOTE_REST, NOTE_CS6, NOTE_REST, NOTE_CS6,
  NOTE_REST, NOTE_D6, NOTE_REST, NOTE_E6, NOTE_FS6, NOTE_FS6, NOTE_REST, NOTE_REST, NOTE_FS6, NOTE_REST,
  NOTE_E6, NOTE_REST, NOTE_D6, NOTE_D6, NOTE_REST, NOTE_B5, NOTE_B5, NOTE_B5, NOTE_REST, NOTE_REST,
  NOTE_D6, NOTE_REST, NOTE_E6, NOTE_REST, NOTE_D6, NOTE_D6, NOTE_REST, NOTE_B5, NOTE_B5, NOTE_REST,
  NOTE_A5, NOTE_REST, NOTE_A3, NOTE_A3, NOTE_A3, NOTE_REST
};

int ringDuration[] = {
  290, 138, 12, 151, 33, 580, 12, 138, 11, 138,
  12, 138, 12, 138, 23, 499, 81, 162, 138, 138,
  12, 149, 1, 312, 138, 162, 162, 140, 129, 149,
  394, 186, 151, 186, 138, 151, 11, 301, 256, 151,
  22, 138, 12, 138, 163, 33, 280, 265, 151, 23,
  138, 12, 149, 1, 115, 22, 127, 1, 127, 47,
  138, 127, 151, 34, 149, 1, 115, 34, 116, 34,
  383, 208, 11, 197, 118, 11
};

void playRingMelody(){
  for (int i  =  0; i!=76; i++){
    int noteDuration = ringDuration[i];
    tone(PIN_SPEAKER, ringMelody[i], noteDuration*0.9);
    delay(noteDuration + 5);
    noTone(PIN_SPEAKER);
  }
}

enum State{
  IDLE,
  SCANNING,
  FACE_DETECTED,
  MOTOR_HANDLER
};

enum Recognized_Substate{
  OWNER,
  KNOWN_INDIVIDUAL
};

enum Motor_Substate{
  LOCKING,
  UNLOCKING
};


State lockState = IDLE;
Recognized_Substate userPermission;
Motor_Substate  motorPhase;
Motor_Substate lastSubstate = LOCKING;


const char* userLabel = "Baqir", *ownerLabel = "Yasin";
const char* gestureRec  = "Wave";


void setup(){
  Serial.begin(115200);
  doorScan.cameraInit();
  pinMode(IN1_PIN, OUTPUT);
  pinMode(IN2_PIN, OUTPUT);
  pinMode(EN_PIN, OUTPUT);
  ledcAttachChannel(EN_PIN, freq, res, pwmChannel);

}

int cameraCheck = 0;


void loop(){
  
  switch (lockState)
  {
    case IDLE:
      ei_printf("Lock State: IDLE | Camera and lock startup......\r\n");
      delay(5000);
      ei_printf("Switching States........\r\n");
      lockState = SCANNING;
      break;

    case SCANNING:
      ei_printf("Lock State: SCANNING | Camera searching for applicable faces......\r\n");
      doorScan.runImpulse();

      if (strcmp(doorScan.label, userLabel) == 0){
        ei_printf("GUEST WITH SPECIAL PERMISSIONS (%s) IDENTIFIED\r\n", userLabel);
        Serial.printf("Free heap before camera: %d\n", ESP.getFreeHeap());
        ledcWrite(EN_PIN, PWM);
        digitalWrite(IN1_PIN, HIGH);
        tone(PIN_SPEAKER, NOTE_G4, 1000);
        delay(2000);
        noTone(PIN_SPEAKER);
        delay(250);
        lockState =  FACE_DETECTED;
        userPermission  = KNOWN_INDIVIDUAL;
      }
      else if (strcmp(doorScan.label, ownerLabel) == 0){
        ei_printf("OWNER IDENTIFIED (%s) \r\n", ownerLabel);
        ledcWrite(EN_PIN, PWM);
        digitalWrite(IN1_PIN, HIGH);
        Serial.printf("Free heap before camera: %d\n", ESP.getFreeHeap());
        tone(PIN_SPEAKER, NOTE_A6, 1000);
        delay(2000);
        noTone(PIN_SPEAKER);
        delay(250);
        lockState = FACE_DETECTED;
        userPermission =  OWNER;
      }
      else{
        ei_printf("No recognized faces identified. \r\n");
      }
      delay(1000);
      break;

    case FACE_DETECTED:
      switch(userPermission)
      {
        case OWNER:
          //Gesture recog
          ei_printf("Owner Detected, scanning for appropriate gesture \r\n");
          if (cameraCheck == 0){
            doorScan.cameraInit();
            delay(2000);
            cameraCheck = 1;
          }
          Serial.printf("Free heap before camera: %d\n", ESP.getFreeHeap());
          delay(200);
          doorScan.runImpulse();
          if (strcmp(doorScan.label, gestureRec) == 0){
            ei_printf("WAVE FOUND \r\n");
            if (lastSubstate != UNLOCKING){
              lockState = MOTOR_HANDLER;
              motorPhase = UNLOCKING;
              cameraCheck = 0;
            }
            else{
              lockState = MOTOR_HANDLER;
              motorPhase = LOCKING;
              cameraCheck = 0;
            }
          }
          delay(1000);
          break;

        case KNOWN_INDIVIDUAL:
          //Gesture recog
          ei_printf("Special Permission Substate \r\n");
          if (cameraCheck == 0){
            doorScan.cameraInit();
            delay(2000);
            cameraCheck = 1;
          }
          Serial.printf("Free heap before camera: %d\n", ESP.getFreeHeap());
          doorScan.runImpulse();
          if (strcmp(doorScan.label, gestureRec) == 0){
            ei_printf("WAVE FOUND \r\n");
            ledcWrite(EN_PIN, PWM);
            digitalWrite(IN1_PIN, HIGH);
            playRingMelody();
            delay(5000);
            lockState = SCANNING;
          }
          delay(1000);
          break;
      }
      break;

    case MOTOR_HANDLER:
      switch(motorPhase)
      {
        case UNLOCKING:
          digitalWrite(IN1_PIN, HIGH);
          digitalWrite(IN2_PIN, LOW);
          ledcWrite(EN_PIN, PWM);
          delay(5000);
          digitalWrite(IN1_PIN, LOW);
          digitalWrite(IN2_PIN, LOW);
          lockState = SCANNING;
          lastSubstate =  UNLOCKING;
          // We will need someway to limit the motor rotation
          break;

        case LOCKING:
          digitalWrite(IN2_PIN, HIGH);
          digitalWrite(IN1_PIN, LOW);
          ledcWrite(EN_PIN, PWM);
          delay(5000);
          digitalWrite(IN1_PIN, LOW);
          digitalWrite(IN2_PIN, LOW);
          lockState = SCANNING;
          lastSubstate =  LOCKING;
          break;
      }
      break;
  }
}