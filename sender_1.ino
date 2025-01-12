#include <WiFi.h>
#include <HTTPClient.h>

// Wi-Fi Bilgileri
const char* ssid = "POCO-Onur";               // Wi-Fi SSID
const char* password = "Erbay216";            // Wi-Fi Şifresi
const char* receiverUrl = "http://192.168.63.163/data"; // Receiver'ın IP adresi ve endpoint

// Direnç ölçüm pinleri ve sabitleri
const int PA[] = {16, 18, 23, 26, 32};       // PA pinleri
const int PB[] = {17, 19, 25, 27, 33};       // PB pinleri
const float R_A[] = {10000.0F, 10000.0F, 10000.0F, 10000.0F, 10000.0F};
const float R_B[] = {20000.0F, 20000.0F, 20000.0F, 20000.0F, 20000.0F};
const float C[] = {10.0F * 1e-6F, 10.0F * 1e-6F, 10.0F * 1e-6F, 10.0F * 1e-6F, 10.0F * 1e-6F};

float resistance[5]; // Direnç sonuçları

// Direnç hesaplama fonksiyonu
float calculateResistance(float R_A, float R_B, float C, float delta_T) {
  float numerator = (R_A + R_B) * delta_T;
  float denominator = ((R_A + R_B) * C * log((R_A + R_B) / R_B)) - delta_T;
  return (denominator == 0) ? -1 : (numerator / denominator) / 1000.0F; // Direnç (kΩ cinsinden)
}

// Direnç ölçüm fonksiyonu
float measureResistance(const int PA, const int PB, float R_A, float R_B, float C) {
  unsigned long startTime = millis();
  unsigned long TA = 0, TB = 0;
  bool TAmeasured = false, TBmeasured = false;

  pinMode(PA, OUTPUT);
  digitalWrite(PA, LOW);
  delay(1);
  digitalWrite(PA, HIGH);
  delay(20);
  digitalWrite(PA, LOW);
  pinMode(PA, INPUT);
  pinMode(PB, INPUT);

  while (!TAmeasured || !TBmeasured) {
    if (!TAmeasured && digitalRead(PA) == LOW) {
      TA = millis() - startTime;
      TAmeasured = true;
    }
    if (!TBmeasured && digitalRead(PB) == LOW) {
      TB = millis() - startTime;
      TBmeasured = true;
    }
  }

  if (TA != 0 && TB != 0) {
    float delta_T = (TA - TB) * 1e-3;
    return calculateResistance(R_A, R_B, C, delta_T);
  }
  return -1; // Hata durumu
}

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Sender1...");

  // Wi-Fi ağına bağlan
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  Serial.print("Sender1 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Tüm dirençleri ölç
  for (int i = 0; i < 5; i++) {
    resistance[i] = measureResistance(PA[i], PB[i], R_A[i], R_B[i], C[i]);
  }

  // Seri monitöre direnç verilerini yazdır
  Serial.println("Measured Resistance Values (Sender1):");
  for (int i = 0; i < 5; i++) {
    Serial.printf("R%d: %.2f kΩ\n", i + 1, resistance[i]);
  }

  // HTTP POST ile veri gönder
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(receiverUrl);
    http.addHeader("Content-Type", "application/json");

    // JSON veri oluştur
    String jsonData = String("{\"R1\":") + resistance[0] +
                      ",\"R2\":" + resistance[1] +
                      ",\"R3\":" + resistance[2] +
                      ",\"R4\":" + resistance[3] +
                      ",\"R5\":" + resistance[4] + "}";

    int httpResponseCode = http.POST(jsonData);

    if (httpResponseCode > 0) {
      Serial.println("Data sent successfully");
    } else {
      Serial.print("Error sending data: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi not connected. Data not sent.");
  }

  delay(50); // 50ms gecikme
}
