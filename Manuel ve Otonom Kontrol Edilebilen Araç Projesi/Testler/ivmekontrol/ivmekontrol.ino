#include <Wire.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <MPU6050.h>

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};  // Alıcı ve verici adresleri
MPU6050 mpu;
int16_t ax, ay, az, gx, gy, gz;

void setup() {
  Serial.begin(115200);  // Seri iletişimi başlat
  Wire.begin();
  mpu.initialize();

  radio.begin();
  radio.openWritingPipe(addresses[1]);  // Alıcı adresini ayarla
  radio.openReadingPipe(1, addresses[0]);  // Verici adresini ayarla
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();  // Verici moduna geç
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  char komut = 'S';  // Varsayılan dur komutu
  const int threshold = 7000;  // Hassasiyet eşiği

  // Seri Monitöre ax ve ay değerlerini yazdır
  Serial.print("ax: ");
  Serial.print(ax);
  Serial.print(" | ay: ");
  Serial.println(ay);

  // Hareket kombinasyonları
  if (ax > threshold && abs(ay) < threshold) {
    komut = 'D';  // Düz sağ
  } else if (ax < -threshold && abs(ay) < threshold) {
    komut = 'Z';  // Düz sol
  } else if (ay > threshold) {
    if (ax > threshold) komut = 'R';  // İleri-sağ
    else if (ax < -threshold) komut = 'L';  // İleri-sol
    else komut = 'I';  // İleri
  } else if (ay < -threshold) {
    komut = 'G';  // Geri
  } else {
    komut = 'S';  // Dur
  }

  // Komut alıcıya gönderiliyor
  radio.write(&komut, sizeof(komut));  // Komutu alıcıya gönder

  // Gönderilen komutu Seri Monitöre yazdır
  Serial.print("Gönderilen komut: ");
  Serial.println(komut);

  delay(10);  // Bekleme süresi
}
