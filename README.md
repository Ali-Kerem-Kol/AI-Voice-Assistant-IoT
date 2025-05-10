# Yapay Zekâ Tabanlı Sesli Asistan

Bu proje, ESP32 mikrodenetleyicisi kullanılarak geliştirilen IoT tabanlı bir sesli yapay zekâ asistanıdır. Cihaz, sesli komutları algılayıp doğal dil işleme ile cevaplar üretir ve sesli olarak yanıt verir.

## Proje Mimarisi
[ESP32] → (Wi-Fi) → [XAMPP + PHP Server (localhost)] → [Azure + Gemini API]
                                              
                                              ↑
                                    Cevapları işleyip .wav üretir


## Özellikler

- **ESP32** ile sesli komut alma
- **Microsoft Azure Speech-to-Text & Text-to-Speech API** ile sesli işlem
- **Google Gemini API** ile yapay zekâ cevabı üretme
- **PHP sunucu** ile veri yönetimi
- **SD kart** üzerine ses dosyası kaydı ve hoparlörle çalma

## Klasör Yapısı

- `src/esp32-sesli-asistan` → ESP32 için yazılan C++ kodları
- `src/esp32-phpServer` → API bağlantıları için PHP dosyaları
- `docs/` → Proje raporu PDF
- `README.md` → Bu dosya

## Donanım

- ESP-VROOM-32
- I2S Mikrofon
- SD Kart Modülü
- Hoparlör + Amfi
- Li-ion Pil ve Şarj Devresi

## Nasıl Çalıştırılır ?
1. XAMPP Kurulumu ve Ayarları
   - 1. XAMPP’i indirin ve kurun.
   - 2. XAMPP Control Panel’i açın.
   - 3. Apache sunucusunu başlatın (MySQL gerekli değil; 80,443).
   - 4. htdocs klasörünü açın (genellikle C:\xampp\htdocs).
   - 5. Bu repodaki php-server klasörünün içeriğini htdocs içine kopyalayın
2. IoT Cihazınızı Wi-Fi ile Bağlayın
   - 1. ESP32 cihazını açın.
   - 2. Üzerindeki Wi-Fi butonuna basın.
   - 3. Telefonunuzdan "Gemini Sesli Asistan" adlı ağa bağlanın.
   - 4. Otomatik olarak açılan sayfada gerekli bilgileri girin
   - 5. Kaydedin ve cihaz yeniden başlasın.
3. Kullanım ve Test
   - 1. Cihaz konuşma moduna geçince bir sesli komut verin.
   - 2. Komut cihazda kaydedilir, yazıya çevrilir ve api.php aracılığıyla sunucuya gönderilir.
   - 3. Gemini API'den alınan cevap Azure Text-to-Speech ile ses dosyasına dönüştürülür.
   - 4. Cihaz http://<bilgisayar_ip>/output.wav adresinden sesi indirip çalar.
4. Uyarılar
   - 1. Cihazınız ile bilgisayarınız aynı Wi-Fi ağına bağlı olmalıdır.
   - 2. php-server/api.php dosyasını açın ve şu kısımları düzenleyin:
      ```php
      $speechKey = "your-azure-speech-key";        // Azure Speech API key
      $geminiApiKey = "your-gemini-api-key";       // Google Gemini API key

## Katkıda Bulunanlar

- Ali Kerem Kol – B221210042  
- Muhammed Baha Bakan – B221210050

---

📄 Daha fazla bilgi için [docs/Rapor.pdf](docs/Rapor.pdf) dosyasını inceleyin.
