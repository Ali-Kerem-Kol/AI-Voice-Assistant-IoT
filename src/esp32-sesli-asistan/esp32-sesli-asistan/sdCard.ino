#include <SdFat.h>

File audioFile;

void startRecording() {
  String fileName = "/audio.wav";
  audioFile = SD.open(fileName, FILE_WRITE);
  if (!audioFile) {
    Serial.println("SD karta yazma hatası!");
    return;
  }
  Serial.println("Kayıt başladı: " + fileName);
}

void stopRecording() {
  if (audioFile) {
    audioFile.close();
    Serial.println("Kayıt durduruldu ve dosya kapatıldı.");
  }
}

void writeToSD(int16_t* buffer, size_t size) {
  if (audioFile) {
    audioFile.write((uint8_t*)buffer, size);
  }
}