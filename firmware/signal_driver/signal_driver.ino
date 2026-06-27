// ── Pins ──────────────────────────────────────────────────────────────────
const int STEERING_PIN = 35;
const int THROTTLE_PIN = 34;
const int WEAPON_PIN   = 18;

const int MOTOR_L_IN1  = 25;
const int MOTOR_L_IN2  = 26;
const int MOTOR_R_IN1  = 27;
const int MOTOR_R_IN2  = 14;
const int WEAPON_IN1   = 2;
const int WEAPON_IN2   = 15;

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
const int DEADBAND         = 5;
const int TURN_MIX_RATIO   = 50;
const int WEAPON_THRESHOLD = 1500;

// ── Failsafe ──────────────────────────────────────────────────────────────
// pulseIn returns 0 on timeout (no signal). A valid RC frame sits ~1000-2000µs,
// so anything outside this band is link loss or a glitch -> stop everything.
const int PULSE_TIMEOUT = 25000;
const int PULSE_MIN_OK  = 900;
const int PULSE_MAX_OK  = 2100;

// ─────────────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== BATTLEBOT STARTING ===");

  pinMode(STEERING_PIN, INPUT);
  pinMode(THROTTLE_PIN, INPUT);
  pinMode(WEAPON_PIN,   INPUT);

  ledcAttachChannel(MOTOR_L_IN1, PWM_FREQ, PWM_RES, 0);
  ledcAttachChannel(MOTOR_L_IN2, PWM_FREQ, PWM_RES, 1);
  ledcAttachChannel(MOTOR_R_IN1, PWM_FREQ, PWM_RES, 2);
  ledcAttachChannel(MOTOR_R_IN2, PWM_FREQ, PWM_RES, 3);
  ledcAttachChannel(WEAPON_IN1,  PWM_FREQ, PWM_RES, 4);
  ledcAttachChannel(WEAPON_IN2,  PWM_FREQ, PWM_RES, 5);

  stopAll();
  Serial.println("Setup complete.");
}

// ── True only for a plausible RC pulse; false on timeout/glitch ───────────
bool validPulse(int pulse) {
  return pulse >= PULSE_MIN_OK && pulse <= PULSE_MAX_OK;
}

// ── Cut all drive and weapon output ───────────────────────────────────────
void stopAll() {
  ledcWrite(MOTOR_L_IN1, 0);
  ledcWrite(MOTOR_L_IN2, 0);
  ledcWrite(MOTOR_R_IN1, 0);
  ledcWrite(MOTOR_R_IN2, 0);
  ledcWrite(WEAPON_IN1,  0);
  ledcWrite(WEAPON_IN2,  0);
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

// ── Convert a validated steering pulse to -100 (left) .. +100 (right) ──────
int steeringFromPulse(int pulse) {
  int steering = map(pulse, STEERING_MIN, STEERING_MAX, -100, 100);
  return applyDeadband(constrain(steering, -100, 100));
}

// ── Convert a validated throttle pulse to -100 (rev) .. +100 (fwd) ─────────
int throttleFromPulse(int pulse) {
  int throttle;
  if (pulse >= THROTTLE_IDLE) {
    throttle = map(pulse, THROTTLE_IDLE, THROTTLE_MAX, 0, 100);
  } else {
    throttle = map(pulse, THROTTLE_MIN, THROTTLE_IDLE, -100, 0);
  }
  return applyDeadband(constrain(throttle, -100, 100));
}

// ── Weapon toggle ─────────────────────────────────────────────────────────
void setWeapon(bool on) {
  ledcWrite(WEAPON_IN1, on ? 255 : 0);
  ledcWrite(WEAPON_IN2, 0);
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
  int steeringPulse = pulseIn(STEERING_PIN, HIGH, PULSE_TIMEOUT);
  int throttlePulse = pulseIn(THROTTLE_PIN, HIGH, PULSE_TIMEOUT);
  int weaponPulse   = pulseIn(WEAPON_PIN,   HIGH, PULSE_TIMEOUT);

  // Failsafe: drive disarms unless both steering and throttle are live.
  if (!validPulse(steeringPulse) || !validPulse(throttlePulse)) {
    stopAll();
    Serial.println("** SIGNAL LOST -- MOTORS DISABLED **");
    delay(20);
    return;
  }

  int  steering = steeringFromPulse(steeringPulse);
  int  throttle = throttleFromPulse(throttlePulse);
  bool weaponOn = validPulse(weaponPulse) && weaponPulse > WEAPON_THRESHOLD;

  mixAndDrive(throttle, steering);
  setWeapon(weaponOn);

  Serial.print("ST: ");    Serial.print(steering);
  Serial.print("  TH: ");  Serial.print(throttle);
  Serial.print("  WPN: "); Serial.println(weaponOn ? "ON" : "OFF");

  delay(20);
}
