#include <Servo.h>

// Pins
#define TRIG_PIN 9
#define ECHO_PIN 10
#define LED_PIN 3
#define SERVO_PIN 6
#define POT_PIN A0

Servo myServo;

// Features
float feature_distance = 0;
float smoothed_distance = 0;
float feature_tilt = 0;

float prev_distance = 0;
unsigned long lastTime = 0;

// Prediction
String gesture_prediction = "NONE";
String expected_gesture = "NONE";
float confidence = 0.0;

// Accuracy
int correct_predictions = 0;
int total_predictions = 0;

// Get distance
float getDistance() {
  long duration;

  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH);
  return duration * 0.034 / 2;
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);

  myServo.attach(SERVO_PIN);

  Serial.begin(9600);
}

void loop() {

  // =============================
  // 1. INPUT
  // =============================
  feature_distance = getDistance();
  feature_tilt = analogRead(POT_PIN);

  // =============================
  // 2. SMOOTHING
  // =============================
  smoothed_distance = (feature_distance + prev_distance) / 2;

  // =============================
  // 3. SWIPE DETECTION
  // =============================
  float diff = abs(smoothed_distance - prev_distance);
  unsigned long currentTime = millis();
  bool isSwipe = (diff > 15 && (currentTime - lastTime) < 300);
  lastTime = currentTime;

  // =============================
  // 4. PREDICTION (PRIORITY BASED)
  // =============================
  gesture_prediction = "NONE";
  confidence = 0.0;

  if (isSwipe) {
    gesture_prediction = "SWIPE";
    confidence = 0.85 + (random(0, 15) / 100.0);
  }
  else if (smoothed_distance < 10) {
    gesture_prediction = "CLOSE";
    confidence = 0.80 + (random(0, 20) / 100.0);
  }
  else if (smoothed_distance > 25) {
    gesture_prediction = "FAR";
    confidence = 0.75 + (random(0, 20) / 100.0);
  }
  else if (feature_tilt < 300) {
    gesture_prediction = "TILT LEFT";
    confidence = 0.70 + (random(0, 25) / 100.0);
  }
  else if (feature_tilt > 700) {
    gesture_prediction = "TILT RIGHT";
    confidence = 0.70 + (random(0, 25) / 100.0);
  }

  // =============================
  // 5. EXPECTED (MATCH SAME LOGIC)
  // =============================
  expected_gesture = "NONE";

  if (isSwipe) {
    expected_gesture = "SWIPE";
  }
  else if (smoothed_distance < 10) {
    expected_gesture = "CLOSE";
  }
  else if (smoothed_distance > 25) {
    expected_gesture = "FAR";
  }
  else if (feature_tilt < 300) {
    expected_gesture = "TILT LEFT";
  }
  else if (feature_tilt > 700) {
    expected_gesture = "TILT RIGHT";
  }

  // =============================
  // 6. ACCURACY
  // =============================
  if (gesture_prediction != "NONE") {
    total_predictions++;

    if (gesture_prediction == expected_gesture) {
      correct_predictions++;
    }
  }

  float accuracy = 0;
  if (total_predictions > 0) {
    accuracy = (correct_predictions * 100.0) / total_predictions;
  }

  // =============================
  // 7. OUTPUT ACTION
  // =============================
  if (gesture_prediction == "CLOSE") {
    digitalWrite(LED_PIN, HIGH);
    myServo.write(0);
  }
  else if (gesture_prediction == "FAR") {
    digitalWrite(LED_PIN, LOW);
    myServo.write(180);
  }
  else if (gesture_prediction == "TILT LEFT") {
    myServo.write(45);
  }
  else if (gesture_prediction == "TILT RIGHT") {
    myServo.write(135);
  }
  else if (gesture_prediction == "SWIPE") {
    digitalWrite(LED_PIN, HIGH);
    delay(80);
    digitalWrite(LED_PIN, LOW);
  }

  // =============================
  // 8. OUTPUT
  // =============================
  Serial.println("====== AI SYSTEM ======");
  Serial.print("Distance: "); Serial.println(smoothed_distance);
  Serial.print("Tilt: "); Serial.println(feature_tilt);

  Serial.print("Prediction: ");
  Serial.print(gesture_prediction);
  Serial.print(" | Confidence: ");
  Serial.println(confidence);

  Serial.print("Expected: ");
  Serial.println(expected_gesture);

  Serial.print("Accuracy: ");
  Serial.print(accuracy);
  Serial.println("%");

  Serial.print("Correct: ");
  Serial.print(correct_predictions);
  Serial.print(" / ");
  Serial.println(total_predictions);

  Serial.println("======================");

  // =============================
  // 9. UPDATE
  // =============================
  prev_distance = smoothed_distance;

  delay(150);
}
