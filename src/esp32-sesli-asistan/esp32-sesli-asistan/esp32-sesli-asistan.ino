#define REPLAY_PIN 32

extern const char* yanitFilePath;

#define RED_LED 21
#define GREEN_LED 13
#define BLUE_LED 22

void setup() {
  Serial.begin(115200);
  pinMode(REPLAY_PIN, INPUT_PULLUP);
  
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  renkAyarla(255,0,0);

  wifiSetup();
  micSetup();
  spkSetup();
}

void renkAyarla(int kirmizi, int yesil, int mavi){ 
  kirmizi = 255-kirmizi;
  yesil = 255-yesil;
  mavi = 255-mavi;
  analogWrite(RED_LED, kirmizi); 
  analogWrite(GREEN_LED, yesil); 
  analogWrite(BLUE_LED, mavi);
}

void loop() {
  wifiLoop();
  micLoop();

  if (digitalRead(REPLAY_PIN) == HIGH) { // Butona basıldıysa
    delay(200); // debounce için kısa bekleme
    playAudio(yanitFilePath);
  }
}