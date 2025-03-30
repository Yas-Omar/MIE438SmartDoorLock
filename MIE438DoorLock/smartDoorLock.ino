//#include "esp32_camera.ino"  --> Used if library is not part of current sketch
// 
faceDetection doorScan;

//-------------- Def
enum State{
  IDLE,
  SCANNING,
  FACE_DETECTED,
  PLACEHOLDER
};

enum Recognized_Substate{
  OWNER,
  KNOWN_INDIVIDUAL
};

State lockState = IDLE;
Recognized_Substate userPermission;
const char* userLabel = "Baqir", *ownerLabel = "Yasin";


void setup(){
  Serial.begin(115200);
  doorScan.cameraInit();
}


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
          ei_printf("Owner Substate \r\n");
          lockState = PLACEHOLDER;
          break;

        case KNOWN_INDIVIDUAL:
          //Gesture recog
          ei_printf("Special Permission Substate \r\n");
          lockState = PLACEHOLDER;
          break;
      }
      break;

    case PLACEHOLDER:
      //PLACEHOLDER FOR MOTOR CONTROL (WE NEED TWO STATES HERE, ONE FOR LOCKING AND ONE FOR UNLOCKING)
      ei_printf("PLACEHOLDER REACHED, WORKS. CODE NEEDS TO BE RESET. \r\n");
      delay(2000);
      break;
  }
}