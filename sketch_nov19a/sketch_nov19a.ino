#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

//
// ======= PIN DEFINISI =======
//

// Ultrasonik 1 (Lampu)
#define trigPin1 26
#define echoPin1 34

// Ultrasonik 2 (Palang)
#define trigPin2 4
#define echoPin2 15

// Lampu
#define merah 13
#define hijau 12

// Buzzer
#define Buzzer 27

// Servo gerbang
#define SERVO_PIN 14

// RFID
#define RST_PIN 22
#define SS_PIN 21

MFRC522 rfid(SS_PIN, RST_PIN);
Servo gateServo;

// UID yang benar
String correctUID = "83:4C:FF:02";


//
// ======= FUNGSI =======
//

// Ultrasonik 1
long getDistance1() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);

  long duration = pulseIn(echoPin1, HIGH);
  return duration * 0.034 / 2;
}

// Ultrasonik 2
long getDistance2() {
  digitalWrite(trigPin2, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin2, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin2, LOW);

  long duration = pulseIn(echoPin2, HIGH, 30000);
  return duration * 0.034 / 2;
}

// Bunyi buzzer
void beep() {
  digitalWrite(Buzzer, HIGH);
  delay(100);
  digitalWrite(Buzzer, LOW);
}

//
// ======= SETUP =======
//

void setup() {
  Serial.begin(115200);

  // Ultrasonik 1
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  // Ultrasonik 2
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);

  // Lampu
  pinMode(merah, OUTPUT);
  pinMode(hijau, OUTPUT);

  // Buzzer
  pinMode(Buzzer, OUTPUT);
  digitalWrite(Buzzer, LOW);

  // Servo
  gateServo.attach(SERVO_PIN);
  gateServo.write(0);

  // RFID
  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Sistem Siap!");
}

//
// ======= LOOP =======
//

void loop() {

  //
  // ======= ULTRASONIK 1 → LAMPU MERAH / HIJAU =======
  //

  long jarakLampu = getDistance1();
  Serial.print("Ultrasonik 1 (Lampu): ");
  Serial.println(jarakLampu);

  if (jarakLampu > 10) {
    digitalWrite(hijau, HIGH);
    digitalWrite(merah, LOW);
  } else {
    digitalWrite(hijau, LOW);
    digitalWrite(merah, HIGH);
  }

  //
  // ======= ULTRASONIK 2 → DETEKSI KENDARAAN DEPAN GERBANG =======
  //

  long jarakGerbang = getDistance2();
  Serial.print("Ultrasonik 2 (Gerbang): ");
  Serial.println(jarakGerbang);

  // Jika kendaraan belum dekat, jangan baca RFID
  if (jarakGerbang > 100) {
    gateServo.write(0); // Palang tutup
    delay(200);
    return;
  }

  //
  // ======= BACA RFID =======
  //

  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {

    // Bunyi buzzer setiap kartu terbaca
    beep();

    // Ambil UID
    String UID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      UID += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "");
      UID += String(rfid.uid.uidByte[i], HEX);
      if (i < rfid.uid.size - 1) UID += ":";
    }
    UID.toUpperCase();

    Serial.print("Kartu Terdeteksi: ");
    Serial.println(UID);

    // Cek kartu
    if (UID == correctUID) {
      Serial.println("[RFID] UID Valid → Membuka palang");
      gateServo.write(90);  // Buka palang
      delay(3000);
      gateServo.write(0);   // Tutup lagi
    } else {
      Serial.println("[RFID] UID Salah!");
    }

    rfid.PICC_HaltA();
  }

  delay(200);
}
