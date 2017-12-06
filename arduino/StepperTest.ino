typedef struct {
  byte power;
  boolean reverse;
} phase_step;

typedef struct {
  phase_step a, b;
} step_state;


// set stepper motor configuration
const byte A_DIR_PIN = 2;
const byte A_PWM_PIN = 3;
const byte B_DIR_PIN = 8;
const byte B_PWM_PIN = 9;
const byte TOGGLE_PIN = 4;
const byte DIRECTION_PIN = 5;
const byte MODE_PIN = 6;
const uint8_t SPEED_CONTROL_PIN = A0;

const byte STEPS_PER_REVOLUTION = 200;

const unsigned long UNSIGNED_LONG_MAX_VALUE = 0xFFFFFFFF;

const int INITIAL_STEP_INTERVAL = 2150;
const int MIN_STEP_INTERVAL = 100;
const int MAX_STEP_INTERVAL = 6000;


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

const step_state FULL_STEPS[] {
  {
    .a = {
      .power = 1,
      .reverse = false
    },
    .b = {
      .power = 1,
      .reverse = false
    }
  },
  {
    .a = {
      .power = 1,
      .reverse = true
    },
    .b = {
      .power = 1,
      .reverse = false
    }
  },
  {
    .a = {
      .power = 1,
      .reverse = true
    },
    .b = {
      .power = 1,
      .reverse = true
    }
  },
  {
    .a = {
      .power = 1,
      .reverse = false
    },
    .b = {
      .power = 1,
      .reverse = true
    }
  }
};

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

const step_state HALF_STEPS[] {};

const byte A_PWM_BIT = 3;
const byte A_DIR_BIT = 2;
const byte B_PWM_BIT = 1;
const byte B_DIR_BIT = 0;

const byte CW = 0;
const byte CCW = 1;


boolean active;
byte direction;
int stepInterval;
int previousStepInterval;
unsigned long previousMicros;
unsigned long currentStep;
int inputValue;
byte previousToggleButtonState;
byte previousDirectionButtonState;
byte previousModeButtonState;
step_state *steps;
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
  steps = HALF_STEPS;
  stepCount = sizeof(HALF_STEPS) / sizeof(step_state);
  Serial.print(stepCount);
  stepInterval = INITIAL_STEP_INTERVAL;
  previousStepInterval = stepInterval;
  currentStep = 0;
  previousMicros = micros();
  previousToggleButtonState = HIGH;
  previousDirectionButtonState = HIGH;
  previousModeButtonState = HIGH;
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
  step({.a = {.power = 1, .reverse = false}, .b = {.power = 1, .reverse = false}});
}

void loop () {

  inputValue = analogRead(A0);
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
        stepCount = sizeof(HALF_STEPS);

      } else {

        steps = FULL_STEPS;
        stepCount = sizeof(FULL_STEPS);

      }
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

        step(steps[currentStep++ % length]);

      } else {

        if (currentStep < 8) {
          currentStep = 0xFFFFFFF0 + (currentStep % length);
        }

        step(steps[currentStep-- % length]);
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
