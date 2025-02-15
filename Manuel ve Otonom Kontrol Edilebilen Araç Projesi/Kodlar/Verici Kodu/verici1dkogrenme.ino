#include <EEPROM.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <MPU6050.h>

#define BUTTON1_PIN 2  // Kayıt butonu
#define BUTTON2_PIN 3  // Oynatma butonu
#define LED_PIN 4      // Kayıt sırasında yanacak LED

#define MAX_HAREKET 1365  // 4096 byte / 3 byte (zaman ve hareket) = ~1365 hareket
#define KAYIT_SURESI 60000  // 1 dakika (milisaniye cinsinden)

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};
MPU6050 mpu;

char hareketKomut;
int hareketIndex = 0;
bool kayitAktif = false;
unsigned long kayitBaslangicZamani = 0;

void setup() {
  pinMode(BUTTON1_PIN, INPUT_PULLUP);
  pinMode(BUTTON2_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Wire.begin();
  mpu.initialize();

  radio.begin();
  radio.openWritingPipe(addresses[1]);  // Alıcı adresi
  radio.openReadingPipe(1, addresses[0]);  // Verici adresi
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();
}

void loop() {
  // Kayıt butonuna basıldığında
  if (digitalRead(BUTTON1_PIN) == LOW) {
    delay(50);  // Debounce
    if (!kayitAktif) {
      Serial.println("Kayıt başladı...");
      kayitAktif = true;
      kayitBaslangicZamani = millis();
      digitalWrite(LED_PIN, HIGH);  // LED'i yak
    } else {
      Serial.println("Kayıt durduruldu.");
      kayitAktif = false;
      digitalWrite(LED_PIN, LOW);  // LED'i söndür
      kayitTamamla();
    }
    while (digitalRead(BUTTON1_PIN) == LOW);  // Buton bırakılana kadar bekle
  }

  // Eğer kayıt aktifse hareketleri kaydet
  if (kayitAktif) {
    if (millis() - kayitBaslangicZamani >= KAYIT_SURESI) {  // 1 dakikayı geçtiyse
      Serial.println("Kayıt otomatik olarak durduruldu.");
      kayitAktif = false;
      digitalWrite(LED_PIN, LOW);  // LED'i söndür
      kayitTamamla();
    } else {
      kayitYap();
    }
  }

  // Oynatma butonuna basıldığında
  if (digitalRead(BUTTON2_PIN) == LOW) {
    Serial.println("Kayıt oynatılıyor...");
    digitalWrite(LED_PIN, HIGH);  // LED'i yak
    kayitOynat();
    digitalWrite(LED_PIN, LOW);  // LED'i söndür
    Serial.println("Oynatma tamamlandı.");
    delay(50);  // Debounce
  }

  // Normal hareket gönderme
  hareketKomut = eldivenHareketiAl();  // Eldivenden hareket komutu al
  Serial.print("Hareket: ");
  Serial.println(hareketKomut);  // Seri monitörde hareketi yazdır
  radio.write(&hareketKomut, sizeof(hareketKomut));  // Komutu alıcıya gönder
}

// **Kayıt Fonksiyonu**
void kayitYap() {
  hareketKomut = eldivenHareketiAl();  // Eldivenden hareket komutu al
  if (hareketIndex < MAX_HAREKET) {
    Wire.beginTransmission(0x50);  // AT24C32 I2C adresi
    Wire.write((int)(hareketIndex * 2) >> 8);  // Yüksek byte adres
    Wire.write((int)(hareketIndex * 2) & 0xFF);  // Düşük byte adres
    Wire.write(hareketKomut);  // Hareket komutunu yaz
    Wire.endTransmission();
    delay(5);  // Yazma işlemi için bekleme süresi

    Serial.print("Kaydedilen: ");
    Serial.println(hareketKomut);
    hareketIndex++;
  }

  // Komutu anlık olarak araca gönder
  radio.write(&hareketKomut, sizeof(hareketKomut));

  delay(30);  // Her hareket arasında 30 ms gecikme
}

// **Kayıt Tamamlama Fonksiyonu**
void kayitTamamla() {
  // Kayıt tamamlandıktan sonra son işaretleyici ekle
  Wire.beginTransmission(0x50);
  Wire.write((int)(hareketIndex * 2) >> 8);
  Wire.write((int)(hareketIndex * 2) & 0xFF);
  Wire.write('\0');
  Wire.endTransmission();

  // Araç 2 saniye duracak
  char durKomutu = 'S';
  radio.write(&durKomutu, sizeof(durKomutu));
  delay(2000);  // 2 saniye bekle

  // LED 2 saniye yanıp söner
  manuelModGecis();
}

// **Oynatma Fonksiyonu**
void kayitOynat() {
  hareketIndex = 0;
  char komut;

  while (true) {
    Wire.beginTransmission(0x50);
    Wire.write((int)(hareketIndex * 2) >> 8);
    Wire.write((int)(hareketIndex * 2) & 0xFF);
    Wire.endTransmission();
    Wire.requestFrom(0x50, 1);

    if (Wire.available()) {
      komut = Wire.read();
    }

    if (komut == '\0') {
      break;
    }

    Serial.print("Oynatılan: ");
    Serial.println(komut);

    radio.write(&komut, sizeof(komut));
    hareketIndex++;
    delay(30);  // Hareketler arasında 30 ms gecikme
  }

  // Oynatma bitince manuel moda geçiş
  manuelModGecis();
}

// **Manuel Mod Geçişi İçin LED Yanıp Sönmesi**
void manuelModGecis() {
  delay(2000);  // 2 saniye bekle
  for (int i = 0; i < 2; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
  }
}

// **Eldiven Hareket Algılama**
char eldivenHareketiAl() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  const int threshold = 7000;  // Hassasiyet eşiği
  if (ax > threshold && abs(ay) < threshold) return 'D';  // Düz sağ
  if (ax < -threshold && abs(ay) < threshold) return 'Z';  // Düz sol
  if (ay > threshold) {
    if (ax > threshold) return 'R';  // İleri sağ
    if (ax < -threshold) return 'L';  // İleri sol
    return 'I';  // İleri
  }
  if (ay < -threshold) return 'G';  // Geri
  return 'S';  // Dur
}
