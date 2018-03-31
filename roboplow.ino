#include <Sabertooth.h>
#include <PPMReader.h>

// Settings for the RC channels
static const int PLOW_POSN_CHANNEL = 4;
static const int PLOW_HEIGHT_CHANNEL = 3;
static const int TURN_CHANNEL = 1;
static const int DRIVE_CHANNEL = 2;

static const int MAX_CHANNEL = 6;

// PIN settings
static const int FS_iA6B_PIN = 3;
static const int PLOW_POSN_MOTOR_PIN1 = 6;
static const int PLOW_POSN_MOTOR_PIN2 = 7;
static const int PLOW_HEIGHT_MOTOR_PIN1 = 8;
static const int PLOW_HEIGHT_MOTOR_PIN2 = 9;

static const int STATUS_LED = 13;

static const unsigned long SIGNAL_TIMEOUT = 40; // milliseconds

// PPM reader reads data from the RC receiver
PPMReader ppm(FS_iA6B_PIN, MAX_CHANNEL);

// SaberTooth connected to the serial pin. The sabertooth should have 
// dip switches set to packet serial mode
Sabertooth ST(128);

void setup() {

  pinMode(PLOW_POSN_MOTOR_PIN1, OUTPUT);
  pinMode(PLOW_POSN_MOTOR_PIN2, OUTPUT);
  pinMode(PLOW_HEIGHT_MOTOR_PIN1, OUTPUT);
  pinMode(PLOW_HEIGHT_MOTOR_PIN2, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  digitalWrite(STATUS_LED, LOW);
  
  SabertoothTXPinSerial.begin(9600);
  
  // SaberTooth should stop the motors if it does not receive
  // a command within the timeout
  ST.setTimeout(500);
  
  // Needed to enable mix-mode packet commands
  ST.drive(0);
  ST.turn(0);
}

/*
 * Convert a PPM value to a SaberTooth value
 * PPM range is (unsigned long) 1000 to 2000
 * SaberTooth is (int) -127 to 127
 */
static inline int ppmToSaber(unsigned long ppmVal)
{
  if (ppmVal > 2000 || ppmVal < 1000) {
    return 0;
  }
  return (int(ppmVal) - 1500) / 4;
}

/*
 * Convert a PPM value to signals to send to a L298N
 * There is a large deadband in the middle which means
 * that the L298N is only switched on when the PPM values are
 * near the extreme values.
 */
static inline void ppmToL298N(unsigned long ppmVal, int in1, int in2)
{
  if (ppmVal > 1900 && ppmVal < 2001) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  }
  else if (ppmVal > 999 && ppmVal < 1100) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }
  else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
}

static void runMotors() {
    // Send the drive and turn data to the SaberTooth
    ST.drive(ppmToSaber(ppm.latestValidChannelValue(DRIVE_CHANNEL, 0)));
    ST.turn(ppmToSaber(ppm.latestValidChannelValue(TURN_CHANNEL, 0)));

    // Configure the motor controllers
    ppmToL298N(ppm.latestValidChannelValue(PLOW_POSN_CHANNEL, 0), PLOW_POSN_MOTOR_PIN1, PLOW_POSN_MOTOR_PIN2);
    ppmToL298N(ppm.latestValidChannelValue(PLOW_HEIGHT_CHANNEL, 0), PLOW_HEIGHT_MOTOR_PIN1, PLOW_HEIGHT_MOTOR_PIN2);
}

void loop() {
    static unsigned long lastValid = 0;

    unsigned long currentTime = millis();

    if (0 == ppm.channelState(MAX_CHANNEL)) {
        lastValid = currentTime;
        runMotors();
        byte ledState = (currentTime / 1024) & 1;
        digitalWrite(STATUS_LED, ledState);
        ppm.latestValidChannelValue(MAX_CHANNEL, 0);
    }

    unsigned long elapsed = currentTime - lastValid;

    if (elapsed > SIGNAL_TIMEOUT) {
        // Switch all motors off
        ST.drive(0);
        ST.turn(0);
        ppmToL298N(1500, PLOW_POSN_MOTOR_PIN1, PLOW_POSN_MOTOR_PIN2);
        ppmToL298N(1500, PLOW_HEIGHT_MOTOR_PIN1, PLOW_HEIGHT_MOTOR_PIN2);

        // Flash STATUS LED
        digitalWrite(STATUS_LED, (elapsed >> 8) & 1);
    }
    
}
