/* gas_valve_stepper_roaster.ino
   Gas roaster controller — valve actuated by LIN 4118L stepper (DM542)
   - AccelStepper for drum and valve steppers
   - PID controlling valve position based on bean temp
   - Homing to closed-limit microswitch
   - Ignition sequence with solenoid (NC) and spark igniter
   - Flame detection + lockout
   Note: configure constants below for your hardware (microstep, gear ratio, pins).
*/

#include <SPI.h>
#include <Adafruit_MAX31856.h>
#include <PID_v1.h>
#include <AccelStepper.h>

// ---------------- CONFIGURATION ----------------
// Motor/drive configuration
const int MOTOR_STEPS_PER_REV = 200;    // full steps per rev (1.8° motor)
const int MICROSTEPPING = 16;           // set on DM542 dip switches (1,2,4,8,16)
const float VALVE_GEAR_RATIO = 5.0;     // gearbox ratio (1.0 if direct drive). Adjust if using 5:1 gearbox.

// Derived
const int VALVE_STEPS_PER_REV = MOTOR_STEPS_PER_REV * MICROSTEPPING * VALVE_GEAR_RATIO; // microsteps/rev with gearbox
const int VALVE_OPEN_STEPS = (int)(VALVE_STEPS_PER_REV * 0.25 + 0.5); // steps for 90° open

// PID tuning (start conservative and tune)
double Kp = 30.0, Ki = 0.3, Kd = 100.0;
const unsigned long PID_INTERVAL_MS = 200;

// Valve pilot and mapping
const int PILOT_PERCENT = 6;   // percent open used for pilot/ignite (6% is a starting point)
const int CLOSED_STEPPER_POSITION = 0; // after homing we set closed=0
// ------------------------------------------------

// ---------------- PINS ----------------
// SPI: SCK = 13, MISO = 12, MOSI = 11 (MOSI is unused by MAX31856)
const int CS_BEAN = 10;
const int CS_EXH = 9;

// Drum stepper (driver PUL-/DIR- style)
const int DRUM_STEP_PIN = 3;
const int DRUM_DIR_PIN  = 4;
const int DRUM_ENABLE_PIN = A0; // optional enable (use analog pin as digital)

// Valve stepper
const int VALVE_STEP_PIN = 2;
const int VALVE_DIR_PIN  = 7;
const int VALVE_HOME_PIN = A2; // limit switch (homing switch) - active LOW when pressed

// Fans / solenoid / igniter / flame / safety
const int FAN_PWM_PIN = 5;
const int SOLENOID_PIN = 6;   // MOSFET/relay drives NC solenoid
const int IGNITER_PIN = 11;   // MUST NOT conflict with SPI usage timing — used only during ignition pulses
const int FLAME_PIN = 8;      // flame detector input (HIGH when flame present) - invert depending on sensor
const int ESTOP_PIN = A1;     // emergency stop (active LOW)
const int RESET_PIN = A3;     // reset lockout button (active LOW)

// ---------------- HARDWARE OBJECTS ----------------
Adafruit_MAX31856 tc_bean(13, 12, CS_BEAN); // SCK, MISO, CS
Adafruit_MAX31856 tc_exh(13, 12, CS_EXH);

AccelStepper drumStepper(AccelStepper::DRIVER, DRUM_STEP_PIN, DRUM_DIR_PIN);
AccelStepper valveStepper(AccelStepper::DRIVER, VALVE_STEP_PIN, VALVE_DIR_PIN);

// PID control for bean temperature -> valve percent
double Setpoint = 200.0; // target bean temp (C)
double Input = 20.0;
double Output = 0.0;     // 0..100 percent
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

// Timing / ignition
const unsigned long IGNITION_TIMEOUT_MS = 6000;
const unsigned long IGNITER_PULSE_MS = 100;
const unsigned long IGNITER_INTERVAL_MS = 400;
const unsigned long SOLENOID_WARM_MS = 350;

// State variables
bool flamePresent = false;
bool lockout = false;
unsigned long lastPidTime = 0;

// Valve state tracking
int valveTargetSteps = 0; // absolute step target (closed=0 .. open=VALVE_OPEN_STEPS)

// ---------------- setup/loop ----------------
void setup() {
  Serial.begin(115200);
  delay(300);
  pinMode(FAN_PWM_PIN, OUTPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  pinMode(IGNITER_PIN, OUTPUT);
  pinMode(FLAME_PIN, INPUT);
  pinMode(VALVE_HOME_PIN, INPUT_PULLUP); // assume switch closes to GND when pressed (active LOW)
  pinMode(ESTOP_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  digitalWrite(SOLENOID_PIN, LOW);
  digitalWrite(IGNITER_PIN, LOW);
  analogWrite(FAN_PWM_PIN, 0);

  // Thermocouple init
  tc_bean.begin();
  tc_exh.begin();

  // Stepper config
  drumStepper.setMaxSpeed(1000.0);
  drumStepper.setAcceleration(400.0);

  valveStepper.setMaxSpeed(800.0);  // tune slow for valve homing and fine control
  valveStepper.setAcceleration(400.0);

  // PID
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 100); // 0..100 percent
  myPID.SetSampleTime(PID_INTERVAL_MS);

  // Home valve on startup (no gas required)
  Serial.println("Homing valve (be sure home switch installed).");
  homeValve();

  // Set valve to closed position explicitly
  valveStepper.setCurrentPosition(0);
  valveTargetSteps = 0;
  valveStepper.moveTo(0);
  while (valveStepper.distanceToGo() != 0) valveStepper.run();

  Serial.println("Setup complete.");
}

void loop() {
  // Safety: E-Stop
  if (digitalRead(ESTOP_PIN) == LOW) {
    Serial.println("E-STOP active!");
    lockout = true;
    emergencyShutdown();
    delay(1000);
    return;
  }

  // Manual reset clears lockout
  if (lockout && digitalRead(RESET_PIN) == LOW) {
    Serial.println("Manual reset pressed. Clearing lockout.");
    lockout = false;
  }

  // Read temps
  double beanTemp = readBeanTemp();
  Input = beanTemp;
  flamePresent = readFlame();

  // Simple user request: if setpoint > Input + 2 then request heat. (You will supply proper UI to start roast.)
  bool requestHeat = (Setpoint > Input + 2.0) && !lockout;

  if (requestHeat && !flamePresent) {
    // ignition routine
    bool ok = attemptIgnition();
    if (!ok) {
      Serial.println("Ignition failed: lockout.");
      lockout = true;
      emergencyShutdown();
      return;
    }
  }

  if (flamePresent && !lockout) {
    // PID control loop (timed)
    unsigned long now = millis();
    if (now - lastPidTime >= PID_INTERVAL_MS) {
      myPID.Compute();
      // Map Output (0..100) to valve steps
      int target = (int)round((Output / 100.0) * VALVE_OPEN_STEPS);
      setValvePosition(target);
      lastPidTime = now;
    }
  } else {
    // No flame: ensure valve closed (or leave solenoid to hardware close)
    setValvePosition(0);
  }

  // If flame lost unexpectedly while requestHeat active -> attempt relight limited times (basic)
  if (!flamePresent && requestHeat && !lockout) {
    Serial.println("Flame lost — attempting relight.");
    bool ok = attemptIgnition();
    if (!ok) {
      lockout = true;
      emergencyShutdown();
      return;
    }
  }

  // Drum rotation continuous
  drumStepper.setSpeed(200.0); // steps/sec (tune based on microstep/gear)
  drumStepper.runSpeed();

  // Fan: simple constant or map to exhaust temp if you like
  analogWrite(FAN_PWM_PIN, 150); // ~60%

  // small loop delay
  delay(10);
}

// ---------------- supporting functions ----------------
double readBeanTemp() {
  double t = tc_bean.readCelsius();
  if (isnan(t)) {
    Serial.println("Bean thermocouple error");
    return Input; // retain previous to avoid wild PID changes
  }
  return t;
}

bool readFlame() {
  // Adjust if your sensor is active LOW
  int raw = digitalRead(FLAME_PIN);
  return (raw == HIGH);
}

void energizeSolenoid(bool on) {
  // Drive solenoid MOSFET/relay
  digitalWrite(SOLENOID_PIN, on ? HIGH : LOW);
}

// homing: move slowly toward closed switch until VALVE_HOME_PIN reads LOW (pressed)
void homeValve() {
  const long HOMING_STEP = -4; // step direction to move toward closed; negative chosen assuming closed is lower position
  const int HOMING_SPEED = 150; // microsteps/sec approximate
  // Move away a small amount first (safe)
  valveStepper.setMaxSpeed(200);
  valveStepper.setAcceleration(200);
  valveStepper.moveTo(-VALVE_OPEN_STEPS); // try to move to a safe known position first (if physical)
  unsigned long start = millis();
  unsigned long timeout = 5000;
  while (valveStepper.distanceToGo() != 0 && millis() - start < timeout) valveStepper.run();

  // Now step slowly toward closed position until we hit switch
  valveStepper.setMaxSpeed(100);
  valveStepper.setAcceleration(200);
  // If home is active low (pressed -> LOW)
  long stepsCounter = 0;
  while (digitalRead(VALVE_HOME_PIN) == HIGH) { // while not pressed
    valveStepper.move(HOMING_STEP);
    long targetTimeout = millis() + 10;
    while (valveStepper.distanceToGo() != 0 && millis() < targetTimeout) valveStepper.run();
    stepsCounter++;
    if (stepsCounter > (VALVE_OPEN_STEPS * 4)) { // something wrong — too many steps
      Serial.println("Homing timeout or no limit switch detected!");
      break;
    }
  }
  // Now we are at closed switch. Set position to zero.
  valveStepper.setCurrentPosition(0);
  valveStepper.stop();
  Serial.println("Valve homed to closed.");
}

// move valve to absolute step position (0 = closed, VALVE_OPEN_STEPS = fully open)
void setValvePosition(int steps) {
  if (steps < 0) steps = 0;
  if (steps > VALVE_OPEN_STEPS) steps = VALVE_OPEN_STEPS;
  if (steps == valveTargetSteps) return; // no change
  valveTargetSteps = steps;
  valveStepper.moveTo(steps);
  // Move in small increments to maintain responsiveness; do not block
  // Call run() in loop to let it move
  unsigned long moveTimeout = millis() + 200; // allow small time
  while (valveStepper.distanceToGo() != 0 && millis() < moveTimeout) valveStepper.run();
}

// ignition: energize solenoid, move valve to pilot opening, pulse igniter until flame detected
bool attemptIgnition() {
  Serial.println("Attempting ignition sequence...");
  // 1) energize solenoid (open physically)
  energizeSolenoid(true);
  delay(SOLENOID_WARM_MS);

  // 2) open valve to pilot percent
  int pilotSteps = (int)round((PILOT_PERCENT / 100.0) * VALVE_OPEN_STEPS);
  setValvePosition(pilotSteps);
  delay(200);

  // 3) pulse igniter while checking flame
  unsigned long start = millis();
  unsigned long lastPulse = 0;
  while (millis() - start < IGNITION_TIMEOUT_MS) {
    if (millis() - lastPulse >= IGNITER_INTERVAL_MS) {
      digitalWrite(IGNITER_PIN, HIGH);
      delay(IGNITER_PULSE_MS);
      digitalWrite(IGNITER_PIN, LOW);
      lastPulse = millis();
    }
    if (readFlame()) {
      Serial.println("Flame detected. Ignition OK.");
      // Slightly increase opening to help PID control (PID will adjust next cycle)
      setValvePosition(max(pilotSteps, pilotSteps + 2));
      digitalWrite(IGNITER_PIN, LOW);
      return true;
    }
    delay(50);
  }

  // timeout — fail
  Serial.println("Ignition timed out. Shutting down solenoid.");
  digitalWrite(IGNITER_PIN, LOW);
  energizeSolenoid(false);
  setValvePosition(0);
  return false;
}

// emergency shutdown: kill igniter, de-energize solenoid, move valve to closed (best effort)
void emergencyShutdown() {
  Serial.println("Emergency shutdown activated.");
  digitalWrite(IGNITER_PIN, LOW);
  energizeSolenoid(false);  // NC solenoid will close gas
  // Attempt to close valve stepper (best effort) — homing switch will be triggered if present
  setValvePosition(0);
  // stop drum
  drumStepper.stop();
  analogWrite(FAN_PWM_PIN, 0); // stop fan (or keep on for cooling as desired)
  // remain locked out until reset
}
