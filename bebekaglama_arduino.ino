#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <time.h>

// Wi-Fi ve Telegram bilgileri
const char* ssid = "iPhone";
const char* password = "cigdemiphonesi";
const char* botToken = "7563060439:AAFvt2hpHlN83QVW5JH5OODvTpLZUbdm9gA";
const int64_t chatID = 6927457320;

WiFiClientSecure client;
const int micPin = 34;
const int buzzerPin = 26;

unsigned long sonMesajZamani = 0;
unsigned long raporBaslangic = 0;
const unsigned long mesajGecikme = 30000; // 30 saniye
const unsigned long raporSuresi = 3 * 60 * 1000; // 3 dakika

String loglar[100];
int logIndex = 0;

// Setup
void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi bağlı!");
  client.setInsecure();
  configTime(3 * 3600, 0, "pool.ntp.org");

  raporBaslangic = millis();
  sendTelegramMessage("📡 Bebek telsizi devrede!");
}

// Ana döngü
void loop() {
  unsigned long simdi = millis();
  int sesDegeri = analogRead(micPin);
  Serial.print("Ses: "); Serial.println(sesDegeri);

  String tahmin = "";
  if (sesDegeri >= 20 && sesDegeri <= 40) tahmin = "😴 Hafif huzursuzluk (uykusu olabilir)";
  else if (sesDegeri >= 41 && sesDegeri <= 50) tahmin = "🍼 Bebek ağlıyor (acıkmış olabilir)";
  else if (sesDegeri >= 70) tahmin = "🚨 Yoğun ağlama (gaz sancısı, alt kirli olabilir)";

  if (tahmin != "" && simdi - sonMesajZamani >= mesajGecikme) {
    String saat = zamanAl();
    String mesaj = "🔊 [" + saat + "] " + tahmin;
    sendTelegramMessage(mesaj);
    buzzerCal();
    if (logIndex < 100) loglar[logIndex++] = saat + " → " + tahmin;
    sonMesajZamani = simdi;
  }

  if (simdi - raporBaslangic >= raporSuresi) {
    if (logIndex > 0) {
      String rapor = "📊 3 Dakikalık Özet:\n";
      for (int i = 0; i < logIndex; i++) {
        rapor += loglar[i] + "\n";
      }
      sendTelegramMessage(rapor);
      logIndex = 0;
    }
    raporBaslangic = simdi;
  }

  delay(300);
}

// Buzzer çalma
void buzzerCal() {
  digitalWrite(buzzerPin, HIGH);
  delay(1000);
  digitalWrite(buzzerPin, LOW);
}

// Saat bilgisi alma
String zamanAl() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "??:??";
  char buf[20];
  strftime(buf, sizeof(buf), "%H:%M:%S", &timeinfo);
  return String(buf);
}

// Telegram mesajı gönderme
void sendTelegramMessage(String message) {
  if (!client.connect("api.telegram.org", 443)) {
    Serial.println("❌ Telegram bağlantı hatası");
    return;
  }
  String url = "/bot" + String(botToken) + "/sendMessage";
  String veri = "chat_id=" + String(chatID) + "&text=" + message;

  client.println("POST " + url + " HTTP/1.1");
  client.println("Host: api.telegram.org");
  client.println("Connection: close");
  client.println("Content-Type: application/x-www-form-urlencoded");
  client.print("Content-Length: ");
  client.println(veri.length());
  client.println();
  client.print(veri);
  Serial.println("✈️ Telegram mesajı gönderiliyor...");
}
