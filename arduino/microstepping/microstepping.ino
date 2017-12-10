#include <math.h>

#include "./microstepping.h"


// set stepper motor configuration
#define A_DIR_PIN 2
#define A_PWM_PIN 3
#define B_DIR_PIN 8
#define B_PWM_PIN 9

#define TOGGLE_PIN 4
#define DIRECTION_PIN 5
#define MODE_PIN 6

#define SPEED_CONTROL_PIN A0

#define STEPS_PER_REVOLUTION 200

#define UNSIGNED_LONG_MAX_VALUE 0xFFFFFFFF
#define CW 1
#define CCW 0

#define INITIAL_STEP_INTERVAL 50
#define MIN_STEP_INTERVAL 50
#define MAX_STEP_INTERVAL 6000
#define PHASE_OFFSET_FRACTION 4

phase_step getStep (unsigned int step, unsigned int stepCount) {
  double fraction =  ((double) step) / stepCount; // + ((double) step) /  (stepCount * 2.0);
  double number = round((cos(fraction * TWO_PI)) * 255.0);
  unsigned char power = (unsigned char) abs(number);
  return {
      .power = power,
      .reverse = (number < 0),
      .sin = number
  };
}

/*
phase_step FULL_STEPS[4] = {
  getStep(0, 4),
  getStep(1, 4),
  getStep(2, 4),
  getStep(3, 4)
};


phase_step HALF_STEPS[8] = {
  getStep(0, 8),
  getStep(1, 8),
  getStep(2, 8),
  getStep(3, 8),
  getStep(4, 8),
  getStep(5, 8),
  getStep(6, 8),
  getStep(7, 8)
};
*/

phase_step HALF_STEPS[16] = {
  getStep(0, 16),
  getStep(1, 16),
  getStep(2, 16),
  getStep(3, 16),
  getStep(4, 16),
  getStep(5, 16),
  getStep(6, 16),
  getStep(7, 16),
  getStep(8, 16),
  getStep(9, 16),
  getStep(10, 16),
  getStep(11, 16),
  getStep(12, 16),
  getStep(13, 16),
  getStep(14, 16),
  getStep(15, 16)
};

phase_step FULL_STEPS[64] = {
  getStep(0, 64),
  getStep(1, 64),
  getStep(2, 64),
  getStep(3, 64),
  getStep(4, 64),
  getStep(5, 64),
  getStep(6, 64),
  getStep(7, 64),
  getStep(8, 64),
  getStep(9, 64),
  getStep(10, 64),
  getStep(11, 64),
  getStep(12, 64),
  getStep(13, 64),
  getStep(14, 64),
  getStep(15, 64),
  getStep(16, 64),
  getStep(17, 64),
  getStep(18, 64),
  getStep(19, 64),
  getStep(20, 64),
  getStep(21, 64),
  getStep(22, 64),
  getStep(23, 64),
  getStep(24, 64),
  getStep(25, 64),
  getStep(26, 64),
  getStep(27, 64),
  getStep(28, 64),
  getStep(29, 64),
  getStep(30, 64),
  getStep(31, 64),
  getStep(32, 64),
  getStep(33, 64),
  getStep(34, 64),
  getStep(35, 64),
  getStep(36, 64),
  getStep(37, 64),
  getStep(38, 64),
  getStep(39, 64),
  getStep(40, 64),
  getStep(41, 64),
  getStep(42, 64),
  getStep(43, 64),
  getStep(44, 64),
  getStep(45, 64),
  getStep(46, 64),
  getStep(47, 64),
  getStep(48, 64),
  getStep(49, 64),
  getStep(50, 64),
  getStep(51, 64),
  getStep(52, 64),
  getStep(53, 64),
  getStep(54, 64),
  getStep(55, 64),
  getStep(56, 64),
  getStep(57, 64),
  getStep(58, 64),
  getStep(59, 64),
  getStep(60, 64),
  getStep(61, 64),
  getStep(62, 64),
  getStep(63, 64)
};


/*
    full steps:
    both coils are active in each step
    only the polarities change
    high torque, low precision
    more vibrations - loud operation

const byte FULL_STEPS[] {
  B1111,
  B1011,
  B1010,
  B1110
};
*/

/*
    half steps:
    each coil has three states: inactive, active with positive polarity, active with negative polarity
    reduced torque, high precision
    less vibrations - silent operation

const byte HALF_STEPS[] {
  B1111,
  B0111,
  B1011,
  B1001,
  B1010,
  B0010,
  B1110,
  B1100
};
*/

boolean active;
byte direction;
unsigned long stepInterval;
unsigned long previousStepInterval;
unsigned long previousMicros;
unsigned long currentStep;
unsigned int phaseOffset;
int inputValue;
byte previousToggleButtonState;
byte previousDirectionButtonState;
byte previousModeButtonState;
phase_step *steps;
byte stepCount;
double voltage; // analog voltage from speed control potmeter


void setup () {
  // start serial communication to see whats happening beside your motor
  Serial.begin(9600);
  pinMode(A_PWM_PIN, OUTPUT);
  pinMode(A_DIR_PIN, OUTPUT);
  pinMode(B_PWM_PIN, OUTPUT);
  pinMode(B_DIR_PIN, OUTPUT);

  pinMode(TOGGLE_PIN, INPUT_PULLUP);
  pinMode(DIRECTION_PIN, INPUT_PULLUP);
  pinMode(MODE_PIN, INPUT_PULLUP);

  direction = CW;
  active = false;
  steps = FULL_STEPS;
  stepCount = sizeof(FULL_STEPS) / sizeof(phase_step);
  phaseOffset = stepCount / PHASE_OFFSET_FRACTION;
  stepInterval = INITIAL_STEP_INTERVAL;
  previousStepInterval = stepInterval;
  currentStep = 0;
  previousMicros = micros();
  previousToggleButtonState = HIGH;
  previousDirectionButtonState = HIGH;
  previousModeButtonState = HIGH;
}


void step (step_state state) {
  /*
  Serial.print(state.a.power * (state.a.reverse ? 1.0 : -1.0));
  Serial.print(" ");
  Serial.println(state.b.power * (state.b.reverse ? 1.0 : -1.0));
  */
  
  analogWrite(A_PWM_PIN, state.a.power);
  digitalWrite(A_DIR_PIN, state.a.reverse);

  analogWrite(B_PWM_PIN, state.b.power);
  digitalWrite(B_DIR_PIN, state.b.reverse);
}


void off () {
  step({.a = {.power = 0, .reverse = false, .sin = 0}, .b = {.power = 0, .reverse = false, .sin = 0}});
}


void on () {
  step({.a = {.power = 128, .reverse = false, .sin = 0}, .b = {.power = 128, .reverse = false, .sin = 0}});
}

void loop () {

  inputValue = analogRead(SPEED_CONTROL_PIN);
  stepInterval = (int) (MIN_STEP_INTERVAL + (MAX_STEP_INTERVAL - MIN_STEP_INTERVAL) * (inputValue / double(1023)));

  byte toggleButtonState = digitalRead(TOGGLE_PIN);

  if (toggleButtonState != previousToggleButtonState) {

    if (toggleButtonState == LOW) {
      active = !active;
    }

    previousToggleButtonState = toggleButtonState;
  }

  byte directionButtonState = digitalRead(DIRECTION_PIN);


  if (directionButtonState != previousDirectionButtonState) {
    if (directionButtonState == LOW) {
      direction = (direction == CW) ? CCW : CW;
    }

    previousDirectionButtonState = directionButtonState;
  }

  byte modeButtonState = digitalRead(MODE_PIN);

  if (modeButtonState != previousModeButtonState) {

    if (modeButtonState == LOW) {

      if (steps == FULL_STEPS) {

        steps = HALF_STEPS;
        stepCount = sizeof(HALF_STEPS) / sizeof(phase_step);

      } else {

        steps = FULL_STEPS;
        stepCount = sizeof(FULL_STEPS) / sizeof(phase_step);
      }

      phaseOffset = stepCount / PHASE_OFFSET_FRACTION;
    }

    previousModeButtonState = modeButtonState;
  }


  if (active) {
    // calculate time since last step
    unsigned long currentMicros = micros();
    unsigned long diff = (currentMicros > previousMicros) ? (currentMicros - previousMicros) : (UNSIGNED_LONG_MAX_VALUE - previousMicros + currentMicros);

    byte length = stepCount;

    if (diff >= stepInterval) {
      if (direction == CW) {

        if (currentStep > UNSIGNED_LONG_MAX_VALUE - 10) {
          currentStep = currentStep % length;
        }

        unsigned int phase_a_step = currentStep % length;
        unsigned int phase_b_step = (phase_a_step + phaseOffset) % length;

     
          //Serial.print(steps[phase_a_step].power * (steps[phase_a_step].reverse ? 1 : -1));
          //Serial.print(" ");
          /*
          Serial.print((float) steps[phase_a_step].power * (steps[phase_a_step].reverse ? -1 : 1));
          Serial.print(" ");
          Serial.println((float) steps[phase_b_step].power * (steps[phase_b_step].reverse ? -1 : 1));
        */

        step({.a = steps[phase_a_step], .b = steps[phase_b_step]});
        currentStep++;

      } else {

        if (currentStep < 8) {
          currentStep = 0xFFFFFFF0 + (currentStep % length);
        }

        if (currentStep % 1111 == 0) {  
          Serial.println("feil vei");
        }
        //step(steps[currentStep-- % length]);
      }

      previousMicros = currentMicros;


      if (stepInterval != previousStepInterval) {
      
        //Serial.print(stepInterval);
        //Serial.println(" ms mellom hvert step");
        previousStepInterval = stepInterval;
        //Serial.print(stepCount);
        //Serial.println(" steps");
      }

    }
  }
}
