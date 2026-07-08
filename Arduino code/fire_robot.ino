// ============================================================
//  Fire Detection and Suppression Robot
//  Semester 8, 2026 — Elissa Kastoun, Jad Abdallah, Lea Eid
// ============================================================
//
//  FSM States:
//    SEARCHING  — no fire detected, robot is stopped
//    TRACKING   — fire detected (any sensor < FIRE_NEAR),
//                 robot navigates toward it
//    SPRAYING   — fire close (any sensor < FIRE_STOP),
//                 robot stopped, pump ON, servo aimed
//    DONE       — spray duration elapsed, everything off
//
//  Sensor convention: lower analog value = stronger IR = closer fire
// ============================================================

// ---------- Pin assignments ----------
#define enA   10   // L298N Enable A (right side motors)
#define in1    9   // L298N Motor 1 direction
#define in2    8
#define in3    7   // L298N Motor 2 direction
#define in4    6
#define enB    5   // L298N Enable B (left side motors)

#define ir_R  A0   // Right flame sensor
#define ir_F  A1   // Front flame sensor
#define ir_L  A2   // Left  flame sensor

#define SERVO_PIN  A4
#define PUMP_PIN   A5

// ---------- Thresholds (tune these on the bench first) ----------
#define FIRE_DETECT   500   // Any sensor below this → fire present
#define FIRE_STOP     300   // Any sensor below this → stop and spray
#define FIRE_GONE     480   // All sensors above this → fire is out

// ---------- Servo angles ----------
#define SERVO_CENTER  90
#define SERVO_RIGHT   55    // Aimed toward right sensor direction
#define SERVO_LEFT   125    // Aimed toward left  sensor direction

// ---------- Timing ----------
#define SPRAY_PULSE_MS     2000   // Pump ON duration per pulse (ms)
#define SPRAY_GAP_MS        500   // Pump OFF gap between pulses, for a clean
                                  // sensor re-check (ms). Set to 0 for no gap.
#define LOOP_DELAY_MS        20   // Main loop period (ms)

// ---------- Motor speeds (0–255) ----------
#define SPEED_FORWARD   180
#define SPEED_TURN_FAST 180
#define SPEED_TURN_SLOW  90

// ---------- FSM states ----------
enum RobotState {
  SEARCHING,
  TRACKING,
  SPRAYING,
  DONE
};

RobotState state = SEARCHING;

int s_R, s_F, s_L;            // Raw sensor readings
unsigned long sprayStart = 0; // Timestamp of current pulse / gap phase
bool inPulse = false;         // true = pump-ON pulse, false = pump-OFF gap

// ============================================================
//  SETUP
// ============================================================
void setup() {
  Serial.begin(9600);

  pinMode(ir_R, INPUT);
  pinMode(ir_F, INPUT);
  pinMode(ir_L, INPUT);

  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  pinMode(SERVO_PIN, OUTPUT);
  pinMode(PUMP_PIN,  OUTPUT);

  // Center the servo on boot
  for (int angle = 90; angle <= 140; angle += 5) servoPulse(SERVO_PIN, angle);
  for (int angle = 140; angle >= 40; angle -= 5) servoPulse(SERVO_PIN, angle);
  for (int angle = 40; angle <= 90; angle += 5)  servoPulse(SERVO_PIN, angle);

  stopMotors();
  digitalWrite(PUMP_PIN, LOW);

  Serial.println("Robot ready.");
}

// ============================================================
//  LOOP
// ============================================================
void loop() {
  // --- Read sensors ---
  s_R = analogRead(ir_R);
  s_F = analogRead(ir_F);
  s_L = analogRead(ir_L);

  // --- Serial debug ---
  Serial.print("STATE: ");
  Serial.print(stateLabel());
  Serial.print("  |  R: "); Serial.print(s_R);
  Serial.print("  F: ");    Serial.print(s_F);
  Serial.print("  L: ");    Serial.println(s_L);

  // --- FSM transitions & actions ---
  switch (state) {

    // ---- SEARCHING ----------------------------------------
    case SEARCHING:
      stopMotors();
      digitalWrite(PUMP_PIN, LOW);

      if (fireDetected()) {
        Serial.println(">> Fire detected, switching to TRACKING");
        state = TRACKING;
      }
      break;

    // ---- TRACKING -----------------------------------------
    case TRACKING:
      digitalWrite(PUMP_PIN, LOW);

      if (!fireDetected()) {
        // Lost the fire
        Serial.println(">> Fire lost, back to SEARCHING");
        stopMotors();
        state = SEARCHING;
        break;
      }

      if (fireClose()) {
        // Close enough — stop and start the first spray pulse
        Serial.println(">> Fire close, switching to SPRAYING");
        stopMotors();
        aimServo();          // Point nozzle at dominant sensor
        digitalWrite(PUMP_PIN, HIGH);
        sprayStart = millis();
        inPulse = true;      // begin an ON pulse
        state = SPRAYING;
        break;
      }

      // Navigate toward the dominant (lowest) sensor
      driveTowardFire();
      break;

    // ---- SPRAYING -----------------------------------------
    //  Pulsed pump: spray for SPRAY_PULSE_MS, then drop the pump
    //  for SPRAY_GAP_MS and re-read the sensors. If fire is still
    //  close → another pulse; otherwise → DONE.
    case SPRAYING:
      stopMotors();

      if (inPulse) {
        // --- ON pulse: pump on, keep re-aiming at the flame ---
        digitalWrite(PUMP_PIN, HIGH);
        aimServo();

        if (millis() - sprayStart >= SPRAY_PULSE_MS) {
          // Pulse finished → drop pump and start the off-gap
          Serial.println(">> Pulse done, pump off, re-checking...");
          digitalWrite(PUMP_PIN, LOW);
          inPulse = false;
          sprayStart = millis();
        }
      } else {
        // --- OFF gap: pump off so sensors settle, then decide ---
        digitalWrite(PUMP_PIN, LOW);

        if (millis() - sprayStart >= SPRAY_GAP_MS) {
          if (fireClose()) {
            // Still close → fire another pulse
            Serial.println(">> Fire still close, another pulse");
            aimServo();
            digitalWrite(PUMP_PIN, HIGH);
            inPulse = true;
            sprayStart = millis();
          } else {
            // Fire no longer close → done spraying
            Serial.println(">> Fire no longer close, switching to DONE");
            servoPulseNow(SERVO_PIN, SERVO_CENTER);  // Return nozzle to center
            state = DONE;
          }
        }
      }
      break;

    // ---- DONE ---------------------------------------------
    case DONE:
      stopMotors();
      digitalWrite(PUMP_PIN, LOW);

      // Return to searching if all sensors are clear
      if (allClear()) {
        Serial.println(">> All clear, back to SEARCHING");
        state = SEARCHING;
      }
      break;
  }

  delay(LOOP_DELAY_MS);
}

// ============================================================
//  CONDITION HELPERS
// ============================================================
bool fireDetected() {
  return (s_R < FIRE_DETECT || s_F < FIRE_DETECT || s_L < FIRE_DETECT);
}

bool fireClose() {
  return (s_R < FIRE_STOP || s_F < FIRE_STOP || s_L < FIRE_STOP);
}

bool allClear() {
  return (s_R >= FIRE_GONE && s_F >= FIRE_GONE && s_L >= FIRE_GONE);
}

// ============================================================
//  NAVIGATION
//  Lower value = stronger signal = fire in that direction.
//  A dead-band of 30 around the front sensor avoids oscillation:
//  if front is within 30 of the best reading, treat it as centered.
// ============================================================
void driveTowardFire() {
  int best = min(s_R, min(s_F, s_L));  // Strongest detection

  if (s_F <= best + 30) {
    // Fire is roughly centered in front — move forward
    moveForward();
  } else if (s_R < s_L) {
    // Right sensor is stronger — arc right
    arcRight();
  } else {
    // Left sensor is stronger — arc left
    arcLeft();
  }
}

// ============================================================
//  SERVO AIM
//  Points the nozzle toward the dominant sensor direction.
//  Uses servoPulseNow (single pulse, non-blocking sweep).
// ============================================================
void aimServo() {
  int best = min(s_R, min(s_F, s_L));

  if (s_F <= best + 30) {
    servoPulseNow(SERVO_PIN, SERVO_CENTER);
  } else if (s_R < s_L) {
    servoPulseNow(SERVO_PIN, SERVO_RIGHT);
  } else {
    servoPulseNow(SERVO_PIN, SERVO_LEFT);
  }
}

// ============================================================
//  MOTOR COMMANDS
//  All motor functions set enA/enB explicitly so speed is
//  always consistent regardless of call order.
// ============================================================
void moveForward() {
  analogWrite(enA, SPEED_FORWARD);
  analogWrite(enB, SPEED_FORWARD);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void arcRight() {
  // Right side slower → robot curves right while still moving
  analogWrite(enA, SPEED_TURN_SLOW);
  analogWrite(enB, SPEED_TURN_FAST);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void arcLeft() {
  // Left side slower → robot curves left while still moving
  analogWrite(enA, SPEED_TURN_FAST);
  analogWrite(enB, SPEED_TURN_SLOW);
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
}

void stopMotors() {
  analogWrite(enA, 0);
  analogWrite(enB, 0);
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
}

// ============================================================
//  SERVO HELPERS
//
//  servoPulse()    — blocking sweep helper (used only in setup)
//  servoPulseNow() — single non-blocking pulse (used in loop)
//
//  Both bit-bang the servo signal on the analog pin.
//  PWM formula: angle * 11 + 500 microseconds
// ============================================================
void servoPulse(int pin, int angle) {
  int pwm = (angle * 11) + 500;
  digitalWrite(pin, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(pin, LOW);
  delay(50);   // Servo refresh period
}

void servoPulseNow(int pin, int angle) {
  // Send a single pulse without the 50ms blocking delay.
  // Call this inside the main loop so the servo receives
  // repeated pulses at loop rate (~50ms via LOOP_DELAY_MS).
  int pwm = (angle * 11) + 500;
  digitalWrite(pin, HIGH);
  delayMicroseconds(pwm);
  digitalWrite(pin, LOW);
}

// ============================================================
//  DEBUG HELPER
// ============================================================
const char* stateLabel() {
  switch (state) {
    case SEARCHING: return "SEARCHING";
    case TRACKING:  return "TRACKING ";
    case SPRAYING:  return "SPRAYING ";
    case DONE:      return "DONE     ";
    default:        return "UNKNOWN  ";
  }
}
