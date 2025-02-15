#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};
char gelenKomut;

const int motorA_IN1 = 3;
const int motorA_IN2 = 4;
const int motorB_IN1 = 5;
const int motorB_IN2 = 6;

const int highSpeed = 250;  // Hızlı hareket için hız
const int turnSpeed = 150;  // Dönüş sırasında düşük hız

void setup() {
  pinMode(motorA_IN1, OUTPUT);
  pinMode(motorA_IN2, OUTPUT);
  pinMode(motorB_IN1, OUTPUT);
  pinMode(motorB_IN2, OUTPUT);

  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(addresses[0]);  // Yazma için adres
  radio.openReadingPipe(1, addresses[1]);  // Okuma için adres
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();  // Alıcı moduna geç
}

void loop() {
  if (radio.available()) {
    radio.read(&gelenKomut, sizeof(gelenKomut));  // Komutu al

    Serial.print("Gelen komut: ");
    Serial.println(gelenKomut);

    switch (gelenKomut) {
      case 'I': ileriGit(); break;
      case 'G': geriGit(); break;
      case 'R': ileriSaga(); break;
      case 'L': ileriSola(); break;
      case 'D': duzSaga(); break;
      case 'Z': duzSola(); break;
      default: dur(); break;
    }
  }
}

void ileriGit() {
  analogWrite(motorA_IN1, highSpeed);
  digitalWrite(motorA_IN2, LOW);
  analogWrite(motorB_IN1, highSpeed);
  digitalWrite(motorB_IN2, LOW);
}

void geriGit() {
  digitalWrite(motorA_IN1, LOW);
  analogWrite(motorA_IN2, highSpeed);
  digitalWrite(motorB_IN1, LOW);
  analogWrite(motorB_IN2, highSpeed);
}

void ileriSaga() {
  analogWrite(motorA_IN1, highSpeed);  // Sol motor tam hız
  digitalWrite(motorA_IN2, LOW);
  analogWrite(motorB_IN1, turnSpeed);  // Sağ motor düşük hız
  digitalWrite(motorB_IN2, LOW);
}

void ileriSola() {
  analogWrite(motorA_IN1, turnSpeed);  // Sol motor düşük hız
  digitalWrite(motorA_IN2, LOW);
  analogWrite(motorB_IN1, highSpeed);  // Sağ motor tam hız
  digitalWrite(motorB_IN2, LOW);
}

void duzSaga() {
  analogWrite(motorA_IN1, highSpeed);  // Sol motor tam hız
  digitalWrite(motorA_IN2, LOW);
  digitalWrite(motorB_IN1, LOW);  // Sağ motor durur
  digitalWrite(motorB_IN2, LOW);
}

void duzSola() {
  digitalWrite(motorA_IN1, LOW);  // Sol motor durur
  digitalWrite(motorA_IN2, LOW);
  analogWrite(motorB_IN1, highSpeed);  // Sağ motor tam hız
  digitalWrite(motorB_IN2, LOW);
}

void dur() {
  digitalWrite(motorA_IN1, LOW);
  digitalWrite(motorA_IN2, LOW);
  digitalWrite(motorB_IN1, LOW);
  digitalWrite(motorB_IN2, LOW);
}
