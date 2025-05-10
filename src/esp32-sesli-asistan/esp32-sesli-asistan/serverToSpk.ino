#include <WiFi.h>
#include <HTTPClient.h>
#include <SdFat.h>
#include <SPI.h>
#include <driver/i2s.h>

// Sunucu bilgileri
const char* yanitFilePath = "/yanit.wav"; // SD kart üzerindeki dosya yolu
extern String sunucuIP;

String downloadURL;

// SD kart pinleri
#define SD_CS 5

// I2S pinleri
#define I2S_LRC 14  // LRC pini
#define I2S_BCLK 27 // BCLK pini
#define I2S_DIN 26  // DIN pini

void spkSetup() {
    pinMode(SD_CS, OUTPUT);

    if (!SD.begin(SD_CS)) {
        Serial.println("SD kart baslatilamadi!");
        while (true);
    }
    Serial.println("SD kart hazir.");

  // I2S ayarları
  i2s_config_t i2s_config2 = {
      .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 16000,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_I2S_MSB,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_BCLK,
      .ws_io_num = I2S_LRC,
      .data_out_num = I2S_DIN,
      .data_in_num = I2S_PIN_NO_CHANGE
  };
  
  // I2S başlat
  i2s_driver_install(I2S_NUM_1, &i2s_config2, 0, NULL);
  i2s_set_pin(I2S_NUM_1, &pin_config);
  i2s_set_clk(I2S_NUM_1, 16000, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

void spkRun() {
    if (downloadResponse()) {
        playAudio(yanitFilePath);
    }
}

bool downloadResponse() {
    downloadURL = "http://" + sunucuIP + "/esp32-phpserver/yanit.wav";

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        http.begin(downloadURL);
        int httpResponseCode = http.GET();

        if (httpResponseCode == HTTP_CODE_OK) {
            // Dosya oluşturma
            File file = SD.open(yanitFilePath, FILE_WRITE);
            if (!file) {
                Serial.println("Dosya SD kartta olusturulamadi.");
                http.end();
                return false;
            }

            // HTTP yanıt akışını al
            WiFiClient* stream = http.getStreamPtr();
            size_t totalBytesRead = 0; // İndirilen toplam bayt miktarını takip eder
            uint8_t buffer[512]; // Daha büyük bir tampon boyutu kullanın

            while (http.connected() && (totalBytesRead < http.getSize() || http.getSize() == -1)) {
                size_t availableBytes = stream->available();
                if (availableBytes > 0) {
                    size_t bytesToRead = availableBytes > sizeof(buffer) ? sizeof(buffer) : availableBytes;
                    size_t bytesRead = stream->readBytes(buffer, bytesToRead);
                    file.write(buffer, bytesRead);
                    totalBytesRead += bytesRead;
                } else {
                    delay(1); // Akış boş ise biraz bekle
                }
            }

            file.close();
            Serial.printf("Yanit dosyasi basariyla indirildi. Toplam %zu bayt.\n", totalBytesRead);
            http.end();
            return true;
        } else {
            Serial.printf("HTTP istegi basarisiz: %d\n", httpResponseCode);
            http.end();
        }
    } else {
        Serial.println("WiFi baglantisi kesildi.");
    }

    return false;
}

void playAudio(const char* filePath) {
    if (!SD.exists(filePath)) {
        Serial.println("Çalınacak dosya bulunamadı.");
        return;
    }

    File audioFile = SD.open(filePath);
    if (!audioFile) {
        Serial.println("Ses dosyası açılamadı.");
        return;
    }
  renkAyarla(225,245,7);
  Serial.print("Download URL: ");
  Serial.println(downloadURL);
  Serial.println("[BİLGİ] WAV dosyası çalınıyor...");
  uint8_t buffer[1024];
  while (audioFile.available()) {
      size_t bytesRead = audioFile.read(buffer, sizeof(buffer));
      size_t bytesWritten = 0;
      i2s_write(I2S_NUM_1, buffer, bytesRead, &bytesWritten, portMAX_DELAY);

      if (bytesWritten < bytesRead) {
          Serial.printf("[UYARI] I2S yazma hatası! %d bayt yazıldı, %d bayt okundu.\n", bytesWritten, bytesRead);
      }
    }
    audioFile.close();
    Serial.println("[BİLGİ] Oynatma tamamlandı!");
  renkAyarla(0,0,255);
}