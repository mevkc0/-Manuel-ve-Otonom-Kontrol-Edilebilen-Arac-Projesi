#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};  // Adresler
char gelenKomut;

void setup() {
  pinMode(A3, OUTPUT);  // LED'in bağlı olduğu pini çıkış yap
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
    
    if (gelenKomut == 'T') {
      digitalWrite(A3, HIGH);  // LED'i yak
    } else if (gelenKomut == 'Y') {
      digitalWrite(A3, LOW);  // LED'i söndür
    }
  }
}
