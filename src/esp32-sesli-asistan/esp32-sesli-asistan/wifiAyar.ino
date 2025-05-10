#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <DNSServer.h>

const uint8_t DNS_PORT = 53;
DNSServer dnsServer;

String sunucuIP = "";

#define BUTTON_PIN 12

// Wi-Fi ayarları
const char *ssid = "Gemini Sesli Asistan";
const char *password = "123456789";

// Web sunucu
AsyncWebServer server(80);

// Wi-Fi bilgilerini EEPROM'dan okuma
String storedSSID;
String storedPassword;
String storedIP;

void wifiSetup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // EEPROM'u başlat
  EEPROM.begin(512);

  // Wi-Fi bilgilerini EEPROM'dan oku
  storedSSID = readWiFiData(0);
  storedPassword = readWiFiData(32); // Şifrenin yazıldığı başlangıç adresi
  storedIP = readWiFiData(64); // IP'nin yazıldığı başlangıç adresi

  // Eğer Wi-Fi bilgileri varsa, bu bilgileri logla ve bağlan
  if (storedSSID.length() > 0 && storedPassword.length() > 0 && storedIP.length() > 0) {
    Serial.println("EEPROM'dan okunan Wi-Fi bilgileri:");
    Serial.print("SSID: ");
    Serial.println(storedSSID);
    Serial.print("Şifre: ");
    Serial.println(storedPassword);

    sunucuIP = storedIP;

    // Wi-Fi'a bağlanmayı dene
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
    Serial.println("Wi-Fi bağlanıyor...");
    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startAttemptTime >= 20000) { // 20 saniye deneme süresi
        Serial.println("Wi-Fi bağlantısı başarısız.");
        break;
      }
      delay(100);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Bağlantı başarılı!");
      renkAyarla(0,0,255);
      Serial.println("IP: " + WiFi.localIP().toString());
    } else {
      Serial.print("Wi-Fi bağlantı durumu: ");
      renkAyarla(255,0,0);
      Serial.println(WiFi.status()); // Hata kodunu yazdır
      startAccessPoint();
    }
  } else {
    // Wi-Fi bilgileri yoksa AP moduna geç
    startAccessPoint();
    renkAyarla(245,90,7);
  }

  // Web sunucusuna Wi-Fi ayarlarını alma sayfası ekle
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = R"rawliteral(
      <html lang="tr-TR">
      <head>
          <meta charset="UTF-8">
          <meta name="viewport" content="width=device-width, initial-scale=1.0">
          <title> Gemini Sesli Asistan </title>
          <style>
              * {margin: 0; padding: 0;}
              div.kapsayici {
                  width: 100%;
                  height: 100%;
                  display: flex;
                  align-items: center;
                  justify-content: center;
              }
              div.kapsayici div.form {
                  width: 300px;
                  height: auto;            
              }
              div.kapsayici div.form form {
                  width: 300px;
                  height: auto;
                  display: flex;
                  flex-direction: column;
                  align-items: center;
              }
              div.kapsayici div.form form label {
                  font-family: Arial,Helvetica Neue,Helvetica,sans-serif; 
                  font-size: 18px;
              }
              div.kapsayici div.form form input[type=text], div.kapsayici div.form form input[type=password] {
                  width: 100%;
                  margin: 5px 0px;
                  padding: 10px;
                  line-height: 14px;
                  font-family: Arial,Helvetica Neue,Helvetica,sans-serif; 
                  font-size: 14px;
                  border: 1px solid black;
                  border-radius: 5px;
              }
              div.kapsayici div.form form input[type=submit] {
                  width: 30%;
                  padding: 10px;
                  line-height: 14px;
                  font-family: Arial,Helvetica Neue,Helvetica,sans-serif; 
                  font-size: 14px;
                  cursor: pointer;
                  border: 1px solid black;
                  border-radius: 5px;
                  background-color: white;
                  margin-bottom: 5px;
              }
              div.kapsayici div.form form input[type=submit]:hover {
                  background-color: black;
                  color: white;
              }
          </style>
      </head>
      <body>
          <div class="kapsayici">
              <div class="form">
                  <form action="/save" method="POST">
                      <label> Kablosuz Ağ Adı </label>
                      <input type="text" name="ssid">
                      <label> Kablosuz Ağ Şifresi </label>
                      <input type="password" name="password">
                      <label> Sunucu IP (Test) </label>
                      <input type="text" name="ip">
                      <input type="submit" value="Gönder">
                  </form>
              </div>
          </div>
      </body>
      </html>
    )rawliteral";
    request->send(200, "text/html", html);
  });

  // Wi-Fi ayarlarını kaydetme
  server.on("/save", HTTP_POST, [](AsyncWebServerRequest *request){
    String newSSID = request->arg("ssid");
    String newPassword = request->arg("password");
    String newSunucuIP = request->arg("ip");
    
    saveWiFiData(newSSID, 0);
    saveWiFiData(newPassword, 32); // Şifreyi EEPROM'un 32. adresinden başlat
    saveWiFiData(newSunucuIP, 64); // IP'yi EEPROM'un 64. adresinden başlat

    request->send(200, "text/html", "<meta charset=\"UTF-8\"><h1>Kablosuz ağ bilgileri kaydedildi. ESP32 yeniden başlatılacak.</h1>");
    delay(2000);
    ESP.restart();
  });

  server.begin();
}

void startAccessPoint() {
  Serial.println("AP modunda başlatılıyor...");
  WiFi.softAP(ssid, password);
  Serial.println("Access Point başlatıldı");
  Serial.println("IP: " + WiFi.softAPIP().toString());
  
  // DNS server'ı başlat ve tüm domainleri ESP32'nin IP adresine yönlendir
  dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());

  // Captive portal yönlendirme
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->redirect("/");
  });
}

String readWiFiData(int addr) {
  String data = "";
  for (int i = addr; i < addr + 32; i++) {
    char c = EEPROM.read(i);
    if (c == '\0') break; // Null terminator'ı gördüğümüzde okumayı durdur
    data += c;
  }
  return data;
}

void saveWiFiData(String data, int addr) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.write(addr + data.length(), '\0'); // Null terminator ekle
  EEPROM.commit();
}

void clearWiFiData() {
  for (int i = 0; i < 96; i++) { // 32 byte SSID ve 32 byte şifre ve 32 byte IP için 96 byte
    EEPROM.write(i, 0); // EEPROM'dan tüm verileri sıfırla
  }
  EEPROM.commit();
}

void wifiLoop()
{
  if (digitalRead(BUTTON_PIN) == HIGH) { // Butona basıldıysa
    delay(200); // debounce için kısa bekleme
    clearWiFiData(); // Wi-Fi bilgilerini sil
    ESP.restart(); // ESP32'yi yeniden başlat
  }
  
  dnsServer.processNextRequest();
}