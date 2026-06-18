// ── Pins ──────────────────────────────────────────────────────────────────
const int STEERING_PIN = 35;
const int THROTTLE_PIN = 34;

const int MOTOR_L_IN1 = 25;
const int MOTOR_L_IN2 = 26;
const int MOTOR_R_IN1 = 27;
const int MOTOR_R_IN2 = 14;

const int FAULT_PIN = 33;

// ── PWM config ────────────────────────────────────────────────────────────
const int PWM_FREQ = 20000;
const int PWM_RES  = 8;

// ── Receiver calibration (µs) ─────────────────────────────────────────────
const int STEERING_MIN  = 965;
const int STEERING_MAX  = 1882;

const int THROTTLE_MIN  = 1055;
const int THROTTLE_IDLE = 1565;
const int THROTTLE_MAX  = 2000;

// ── Tuning ────────────────────────────────────────────────────────────────
const int DEADBAND       = 5;
const int TURN_MIX_RATIO = 50;

// ─────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== RC CAR STARTING ===");

  pinMode(STEERING_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(FAULT_PIN, INPUT_PULLUP);

  ledcAttachChannel(MOTOR_L_IN1, PWM_FREQ, PWM_RES, 0);
  ledcAttachChannel(MOTOR_L_IN2, PWM_FREQ, PWM_RES, 1);
  ledcAttachChannel(MOTOR_R_IN1, PWM_FREQ, PWM_RES, 2);
  ledcAttachChannel(MOTOR_R_IN2, PWM_FREQ, PWM_RES, 3);

  ledcWrite(MOTOR_L_IN1, 0);
  ledcWrite(MOTOR_L_IN2, 0);

  Serial.println("Setup complete.");
}

// ── Write a signed speed (-100..100) to one motor ─────────────────────────
void setMotor(int pinA, int pinB, int speed) {
  speed = constrain(speed, -100, 100);
  int duty = map(abs(speed), 0, 100, 0, 255);
  if (speed > 0) {
    ledcWrite(pinA, duty);
    ledcWrite(pinB, 0);
  } else if (speed < 0) {
    ledcWrite(pinA, 0);
    ledcWrite(pinB, duty);
  } else {
    ledcWrite(pinA, 0);
    ledcWrite(pinB, 0);
  }
}

// ── Deadband ──────────────────────────────────────────────────────────────
int applyDeadband(int value) {
  return abs(value) <= DEADBAND ? 0 : value;
}

// ── Read steering: -100 (left) .. 0 .. +100 (right) ──────────────────────
int readSteering() {
  int pulse    = pulseIn(STEERING_PIN, HIGH, 25000);
  int steering = map(pulse, STEERING_MIN, STEERING_MAX, -100, 100);
  return applyDeadband(constrain(steering, -100, 100));
}

// ── Read throttle: -100 (full reverse) .. 0 .. +100 (full forward) ────────
int readThrottle() {
  int pulse = pulseIn(THROTTLE_PIN, HIGH, 25000);
  int throttle;
  if (pulse >= THROTTLE_IDLE) {
    throttle = map(pulse, THROTTLE_IDLE, THROTTLE_MAX, 0, 100);
  } else {
    throttle = map(pulse, THROTTLE_MIN, THROTTLE_IDLE, -100, 0);
  }
  return applyDeadband(constrain(throttle, -100, 100));
}

// ── Motor mixing ──────────────────────────────────────────────────────────
void mixAndDrive(int throttle, int steering) {
  int leftSpeed, rightSpeed;

  if (throttle == 0) {
    leftSpeed  =  steering;
    rightSpeed = -steering;
  } else {
    int inner = (throttle * TURN_MIX_RATIO) / 100;
    if (steering > 0) {
      leftSpeed  = throttle;
      rightSpeed = inner;
    } else if (steering < 0) {
      leftSpeed  = inner;
      rightSpeed = throttle;
    } else {
      leftSpeed  = throttle;
      rightSpeed = throttle;
    }
  }

  setMotor(MOTOR_L_IN1, MOTOR_L_IN2, leftSpeed);
  setMotor(MOTOR_R_IN1, MOTOR_R_IN2, rightSpeed);
}

// ─────────────────────────────────────────────────────────────────────────
void loop() {
  int steering = readSteering();
  int throttle  = readThrottle();
  bool fault    = digitalRead(FAULT_PIN) == LOW;

  mixAndDrive(throttle, steering);

  Serial.print("ST: ");     Serial.print(steering);
  Serial.print("  TH: ");   Serial.print(throttle);
  Serial.print("  ULT: ");  Serial.println(fault ? "** FAULT **" : "OK");

  delay(20);
}