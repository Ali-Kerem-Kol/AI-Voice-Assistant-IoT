#include <WiFi.h>
#include <HTTPClient.h>
#include <driver/i2s.h>
#include <SD.h>
#include <FS.h>

// Mikrofon ve buton pinleri
#define I2S_WS 15
#define I2S_SD 4
#define I2S_SCK 2
#define BUTTON_PIN 33

// SD kart pinleri
#define SD_CS 5

extern String sunucuIP;

// Server URL
String serverURL;

// Değişkenler
bool recording = false;
File audioFile2;

void micSetup() {

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(SD_CS, OUTPUT);

  if (!SD.begin(SD_CS)) {
    Serial.println("SD kart başlatılamadı!");
    renkAyarla(255,0,0);
    while (true);
  }
  Serial.println("SD kart hazır.");

  // SD kart başlat
  if (!SD.begin(SD_CS)) {
    Serial.println("SD kart başlatılamadı!");
    while (true);
  }
  Serial.println("SD kart başlatıldı.");

  // I2S yapılandırması
  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_SCK,
    .ws_io_num = I2S_WS,
    .data_out_num = I2S_PIN_NO_CHANGE,
    .data_in_num = I2S_SD
  };
    
  // I2S başlat
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);
}

void micLoop() {
  int buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == HIGH && !recording) {
    // Kayda başla
    Serial.println("Recording started...");
    recording = true;
    audioFile2 = SD.open("/audio.raw", FILE_WRITE);
    if (!audioFile2) {
      Serial.println("SD karta dosya açılamadı!");
      recording = false;
    }
  } else if (buttonState == LOW && recording) {
    // Kaydı durdur ve server'a gönder
    Serial.println("Recording stopped, sending data...");
    recording = false;
    audioFile2.close();
    sendAudioData();
    Serial.println(serverURL);
  }

  if (recording) {
    recordAudioData();
  }
}

void recordAudioData() {
  renkAyarla(0,255,0);
  const int bufferSize = 1024;
  int16_t buffer[bufferSize];
  size_t bytesRead;

  i2s_read(I2S_NUM_0, (void*)buffer, bufferSize, &bytesRead, portMAX_DELAY);
  audioFile2.write((uint8_t*)buffer, bytesRead);
}

void sendAudioData() {
  serverURL = "http://" + sunucuIP + "/esp32-phpserver/upload.php";
  Serial.print("Server URL: ");
  Serial.println(serverURL);

  File file = SD.open("/audio.raw", FILE_READ);
  if (!file) {
    Serial.println("Dosya açılamadı!");
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverURL);
  http.addHeader("Content-Type", "application/octet-stream");

  size_t fileSize = file.size();
  Serial.printf("Gönderilecek dosya boyutu: %d bytes\n", fileSize);

  int httpResponseCode = http.sendRequest("POST", &file, fileSize);

  if (httpResponseCode > 0) {
    Serial.printf("HTTP Response code: %d\n", httpResponseCode);
  } else {
    Serial.printf("Error code: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
  file.close();
  SD.remove("/audio.raw"); // Dosyayı gönderimden sonra sil

  delay(5000);
  renkAyarla(0,0,255);
  spkRun();
}