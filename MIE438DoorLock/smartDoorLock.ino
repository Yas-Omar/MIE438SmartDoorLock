
//#include "esp32_camera.ino"  --> Used if library is not part of current sketch
// 
faceDetection doorScan;
gestureDetection gestScan;

//-------------- Def
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
}


State lockState = IDLE;
Recognized_Substate userPermission;
Motor_Substate  motorPhase;
Motor_Substate lastSubstate = LOCKING;


const char* userLabel = "Baqir", *ownerLabel = "Yasin";
const char* userGestures[2] = {"Peace", "OpenPalm"};


void setup(){
  Serial.begin(115200);
  doorScan.cameraInit();
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
        lockState =  FACE_DETECTED;
        userPermission  = KNOWN_INDIVIDUAL;
      }
      else if (strcmp(doorScan.label, ownerLabel) == 0){
        ei_printf("OWNER IDENTIFIED (%s) \r\n", ownerLabel);
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
            gestScan.cameraInit();
            delay(2000);
            cameraCheck = 1;
          }
          gestScan.runImpulse();
          if (strcmp(gestScan.label, userGestures[0]) == 0){
            if (lastSubstate != UNLOCKING){
              lockState = MOTOR_HANDLER;
              motorPhase = UNLOCKING;
              cameraCheck = 0;
            }
            else{
              ei_printf("DOOR IS ALREADY UNLOCKED, PLEASE LOCK BEFORE ATTEMPTING TO UNLOCK \r\n");
            }

          }
          else if (strcmp(gestScan.label, userGestures[1] == 0)){
            if (lastSubstate != LOCKING){
              lockState = MOTOR_HANDLER;
              motorPhase =  LOCKING;
              cameraCheck = 0;
            }
            else{
              ei_printf("DOOR IS ALREADY LOCKED, PLEASE UNLOCK BEFORE ATTEMPTING TO LOCK \r\n");
            }
          }
          delay(1000);
          break;

        case KNOWN_INDIVIDUAL:
          //Gesture recog
          ei_printf("Special Permission Substate \r\n");
          if (cameraCheck == 0){
            gestScan.cameraInit();
            delay(2000);
            cameraCheck = 1;
          }
          gestScan.runImpulse();
          if (strcmp(gestScan.label, userGestures[0])){
            lockState = MOTOR_HANDLER;   // Call to Speaker State will go here.
            cameraCheck = 0;
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
          lockState = SCANNING;
          lastSubstate =  UNLOCKING;
          // We will need someway to limit the motor rotation
          break;

        case LOCKING:
          digitalWrite(IN2_PIN, HIGH);
          digitalWrite(IN1_PIN, LOW);
          ledcWrite(EN_PIN, PWM);
          lockState = SCANNING;
          lastSubstate =  LOCKING;
          break;
      }
      break;
  }
}