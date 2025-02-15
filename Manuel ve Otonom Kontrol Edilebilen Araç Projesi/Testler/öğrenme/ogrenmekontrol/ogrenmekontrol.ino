#include <Wire.h>
#include <EEPROM.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <MPU6050.h>

#define BUTTON1_PIN 2  // Kayıt butonu
#define BUTTON2_PIN 3  // Oynatma butonu
#define LED_PIN 4      // Kayıt sırasında yanacak LED

#define MAX_HAREKET 500  // Maksimum kaydedilecek hareket sayısı
#define KAYIT_SURESI 10000  // 10 saniye (milisaniye cinsinden)

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};
MPU6050 mpu;

char hareketKomut;
int hareketIndex = 0;

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
    Serial.println("Kayıt başladı...");
    digitalWrite(LED_PIN, HIGH);  // LED'i yak
    kayitYap();
    digitalWrite(LED_PIN, LOW);  // LED'i söndür
    Serial.println("Kayıt tamamlandı.");
    delay(200);  // Debounce
  }

  // Oynatma butonuna basıldığında
  if (digitalRead(BUTTON2_PIN) == LOW) {
    Serial.println("Kayıt oynatılıyor...");
    kayitOynat();
    Serial.println("Oynatma tamamlandı.");
    delay(200);  // Debounce
  }

  // Normal hareket gönderme
  hareketKomut = eldivenHareketiAl();  // Eldivenden hareket komutu al
  Serial.print("Hareket: ");
  Serial.println(hareketKomut);  // Seri monitörde hareketi yazdır
  radio.write(&hareketKomut, sizeof(hareketKomut));  // Komutu alıcıya gönder
}

// **Kayıt Fonksiyonu**
void kayitYap() {
  hareketIndex = 0;
  unsigned long baslangicZamani = millis();

  while (millis() - baslangicZamani < KAYIT_SURESI) {
    hareketKomut = eldivenHareketiAl();  // Eldivenden hareket komutu al
    if (hareketIndex < MAX_HAREKET) {
      EEPROM.write(hareketIndex, hareketKomut);  // EEPROM'a kaydet
      Serial.print("Kaydedilen: ");
      Serial.println(hareketKomut);  // Kaydedilen komutu yazdır
      hareketIndex++;
    }

    // Komutu anlık olarak araca gönder
    radio.write(&hareketKomut, sizeof(hareketKomut));  

    delay(45);  // Her hareket arasında 10 ms gecikme
  }

  // Kayıt sonu işaretleyici
  EEPROM.write(hareketIndex, '\0');  // Kayıt sonunu işaretle

  // Araç 2 saniye duracak
  char durKomutu = 'S';
  radio.write(&durKomutu, sizeof(durKomutu));
  delay(2000);  // 2 saniye bekle
}

// **Oynatma Fonksiyonu**
void kayitOynat() {
  hareketIndex = 0;
  char komut;

  // Sürekli oynatma işlemi yapılacak
  while (true) {
    komut = EEPROM.read(hareketIndex);  // EEPROM'dan oku

    if (komut == '\0') {  // Kayıt sonuna ulaşıldığında
      break;
    }

    Serial.print("Oynatılan: ");
    Serial.println(komut);  // Oynatılan komutu yazdır

    // Komutu alıcıya gönder
    radio.write(&komut, sizeof(komut));

    hareketIndex++;
    delay(45);  // Hareketler arasında 10 ms gecikme
  }
}

// **Eldiven Hareket Algılama**
char eldivenHareketiAl() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // Serial ile sensör verilerini yazdırarak kontrol edin
  Serial.print("ax: "); Serial.print(ax);
  Serial.print(" ay: "); Serial.print(ay);
  Serial.print(" az: "); Serial.print(az);
  Serial.print(" gx: "); Serial.print(gx);
  Serial.print(" gy: "); Serial.print(gy);
  Serial.print(" gz: "); Serial.println(gz);

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
