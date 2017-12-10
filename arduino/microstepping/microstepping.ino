#include <math.h>

#include <microstepping.h>


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

const int INITIAL_STEP_INTERVAL = 2150;
const int MIN_STEP_INTERVAL = 100;
const int MAX_STEP_INTERVAL = 6000;
const byte PHASE_OFFSET_FRACTION = 4;

phase_step getStep (int step, int stepCount) {
  const double fraction =  ((double) step) / stepCount;
  const double number = round((1 + sin(fraction * TWO_PI)) * 255.5);
  const byte power = (byte)(number - 256);
  return {
      .power = 255, //power,
      .reverse = (number < 256),
      .sin = number
  };
}

const phase_step FULL_STEPS[4] = {
  getStep(0, 4),
  getStep(1, 4),
  getStep(2, 4),
  getStep(3, 4)
};

const phase_step HALF_STEPS[16] = {
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

const byte CW = 0;
const byte CCW = 1;


boolean active;
byte direction;
int stepInterval;
int previousStepInterval;
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

  Serial.print("step 2: power: ");
  Serial.print(getStep(2, 4).power);
  Serial.print(" reverse: ");
  Serial.print(getStep(2, 4).reverse);
  Serial.println(getStep(2, 4).sin);
}


void step (step_state state) {
  analogWrite(A_PWM_PIN, state.a.power);
  digitalWrite(A_DIR_PIN, state.a.reverse);

  analogWrite(B_PWM_PIN, state.b.power);
  digitalWrite(B_DIR_PIN, state.b.reverse);
}


void off () {
  step({.a = {.power = 0, .reverse = false}, .b = {.power = 0, .reverse = false}});
}


void on () {
  step({.a = {.power = 128, .reverse = false}, .b = {.power = 128, .reverse = false}});
}

void loop () {

  inputValue = analogRead(SPEED_CONTROL_PIN);
  stepInterval = int(MIN_STEP_INTERVAL + (MAX_STEP_INTERVAL - MIN_STEP_INTERVAL) * (inputValue / double(1023)));

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

        if (currentStep % 1111 == 0) {
          Serial.println("phase_a_step: ");
          Serial.println(phase_a_step);
          Serial.println(steps[phase_a_step].power);
          Serial.println(steps[phase_a_step].reverse);

          Serial.println("phase_b_step: ");
          Serial.println(phase_b_step);
          Serial.println(steps[phase_b_step].power);
          Serial.println(steps[phase_b_step].reverse);
          Serial.println();
        }

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

      if (Serial.available() != 0 && currentStep % 100 == 0 && stepInterval != previousStepInterval) {

        Serial.print(stepInterval);
        Serial.println(" ms mellom hvert step");
        previousStepInterval = stepInterval;
        Serial.print(stepCount);
        Serial.println(" steps");
      }
    }
  }
}
