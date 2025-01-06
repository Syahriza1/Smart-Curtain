#define REMOTEXY_MODE__SOFTSERIAL
#include <SoftwareSerial.h>
#define REMOTEXY_SERIAL_RX 3
#define REMOTEXY_SERIAL_TX 2
#define REMOTEXY_SERIAL_SPEED 9600
#define REMOTEXY_ACCESS_PASSWORD "1234"
#include <RemoteXY.h> 
#pragma pack(push, 1)  

uint8_t RemoteXY_CONF[] =
  { 255,3,0,1,0,102,0,19,0,0,0,83,109,97,114,116,32,67,117,114,
  116,97,105,110,0,24,1,106,200,1,1,5,0,12,31,29,45,10,192,30,
  26,65,117,116,111,109,97,116,105,99,0,77,97,110,117,97,108,0,1,13,
  73,24,24,0,2,31,0,1,70,73,24,24,0,2,31,0,67,32,152,40,
  10,93,2,26,129,19,136,75,9,64,17,84,105,110,103,107,97,116,32,75,
  101,99,101,114,97,104,97,110,0 };

struct {
  uint8_t sensor; 
  uint8_t kiri; 
  uint8_t kanan; 
  int8_t persen;
  uint8_t connect_flag;  

} RemoteXY;   
#pragma pack(pop)

#define MOTOR_IN1 5
#define MOTOR_IN2 6
#define MOTOR_EN 9
#define LDR_PIN A0
#define MOTOR_SPEED 100
#define LEVEL_1 600
#define LEVEL_2 200
#define LEVEL_3 0
#define TIME_LEVEL_2 2000
#define TIME_LEVEL_3 2000
#define TIME_LEVEL_4 3000

int ldrValue;
int currentLevel = 1; 
unsigned long lastAdjustTime = 0; 
const int adjustDelay = 2000;    

bool isAutomatic = true;
int previousLevel = 1; 

// Fungsi Setup dan Loop

void setup() {
  RemoteXY_Init();
  Serial.begin(9600);

  pinMode(MOTOR_IN1, OUTPUT);
  pinMode(MOTOR_IN2, OUTPUT);
  pinMode(MOTOR_EN, OUTPUT);
  pinMode(LDR_PIN, INPUT);

  stopMotor();
}

void loop() {
  RemoteXY_Handler();
  ldrValue = analogRead(LDR_PIN);

  int lightPercentage = map(ldrValue, 0, 1023, 100, 0);

  RemoteXY.persen = lightPercentage;

  bool newAutomaticState = (RemoteXY.sensor == 0);
  if (isAutomatic != newAutomaticState) {
    if (newAutomaticState) {
      adjustCurtain(previousLevel);
      currentLevel = previousLevel;
    } else {
      previousLevel = currentLevel;
    }
    isAutomatic = newAutomaticState;
  }

  if (isAutomatic) {
    int newLevel = determineLevel(ldrValue);

    Serial.print("LDR Value: ");
    Serial.print(ldrValue);
    Serial.print(" || Light Percentage: ");
    Serial.print(lightPercentage);
    Serial.print("% || Current Level: ");
    Serial.print(currentLevel);
    Serial.println("");

    if (newLevel != currentLevel && millis() - lastAdjustTime > adjustDelay) {
      adjustCurtain(newLevel);
      currentLevel = newLevel;
      lastAdjustTime = millis();
    }
  } else {
    if (RemoteXY.kanan) {
      moveMotorLeft(5);
    }
    if (RemoteXY.kiri) {
      moveMotorRight(5);
    }
  }
}

// Fungsi Kontrol Motor

void stopMotor() {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, 0);
}

void moveMotorLeft(int duration) {
  digitalWrite(MOTOR_IN1, HIGH);
  digitalWrite(MOTOR_IN2, LOW);
  analogWrite(MOTOR_EN, MOTOR_SPEED);
  delay(duration);
  stopMotor();
}

void moveMotorRight(int duration) {
  digitalWrite(MOTOR_IN1, LOW);
  digitalWrite(MOTOR_IN2, HIGH);
  analogWrite(MOTOR_EN, MOTOR_SPEED);
  delay(duration);
  stopMotor();
}

// Fungsi Logika Kontrol

int determineLevel(int ldrValue) {
  if (ldrValue >= LEVEL_1) {
    return 1;
  } else if (ldrValue >= LEVEL_2) {
    return 2;
  } else {
    return 3;
  }
}

void adjustCurtain(int newLevel) {
  if (currentLevel == 1 && newLevel == 3) {
    moveMotorRight(TIME_LEVEL_4);
  } else if (currentLevel == 3 && newLevel == 1) {
    moveMotorLeft(TIME_LEVEL_4);
  } else if (currentLevel == 1 && newLevel == 2) {
    moveMotorRight(TIME_LEVEL_2);
  } else if (currentLevel == 2 && newLevel == 3) {
    moveMotorRight(TIME_LEVEL_3);
  } else if (currentLevel == 3 && newLevel == 2) {
    moveMotorLeft(TIME_LEVEL_3);
  } else if (currentLevel == 2 && newLevel == 1) {
    moveMotorLeft(TIME_LEVEL_2); 
  }
}
