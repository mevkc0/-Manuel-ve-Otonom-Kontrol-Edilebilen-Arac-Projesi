#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(8, 7);  // CE, CSN pinleri
const byte addresses[][6] = {"00001", "00002"};  // Alıcı ve verici adresleri

void setup() {
  Serial.begin(115200);
  radio.begin();
  radio.openWritingPipe(addresses[1]);  // Alıcı adresini ayarla
  radio.openReadingPipe(1, addresses[0]);  // Verici adresini ayarla
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();  // Verici moduna geç
}

void loop() {
  if (Serial.available()) {
    char komut = Serial.read();  // Seri monitörden gelen komutu oku
    radio.write(&komut, sizeof(komut));  // Komutu alıcıya gönder

    Serial.print("Gönderilen komut: ");
    Serial.println(komut);
  }
}
