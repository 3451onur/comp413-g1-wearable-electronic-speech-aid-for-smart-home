#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <letter_29_new_LAST_inferencing.h> // Edge Impulse kütüphanesi

// LED pin tanımları
#define GREEN_LED 13  // Yeşil LED GPIO pini
#define RED_LED 12    // Kırmızı LED GPIO pini
#define BLUE_LED 14   // Mavi LED GPIO pini

#define BUFFER_SIZE EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE

// Wi-Fi bilgileri
const char* ssid = "POCO-Onur"; // Wi-Fi SSID
const char* password = "Erbay216"; // Wi-Fi Şifresi

// Web server
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Gelen direnç verileri için dizi
float resistance[10] = {0}; // R0-R9 için

// Karar mekanizması değişkenleri
String currentWord = "";           // Oluşturulan kelime
String finalWord = "";             // Tamamlanan kelime
String lastFinalWord = "";         // Önceki final kelime
String lastLetter = "";            // Son eklenen harf

unsigned long lastRunTime = 0;     // Son sınıflandırma zamanı
const unsigned long INTERVAL = 1750; // 1.75 saniye

// HTML içeriği
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32 Dashboard</title>
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <style>
        body {
            font-family: 'Arial', sans-serif;
            margin: 0;
            padding: 0;
            background: linear-gradient(to right, #ffffff, #d9e4f5);
            color: #333;
        }
        header {
            background: #37474f;
            color: #eceff1;
            text-align: center;
            padding: 20px 0;
        }
        .container {
            display: flex;
            flex-direction: column;
            align-items: center;
            margin: 20px;
        }
        .message-container {
            display: flex;
            justify-content: space-around;
            flex-wrap: wrap;
            width: 90%;
            margin: 20px auto;
            background: #eceff1;
            border-radius: 8px;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
            padding: 20px;
        }
        .message-card {
            text-align: center;
            margin: 10px;
        }
        .message-card h3 {
            font-size: 22px;
            margin-bottom: 10px;
            color: #37474f;
        }
        .message-card span {
            font-size: 30px;
            font-weight: bold;
            color: #00796b;
        }
        .charts {
            display: flex;
            flex-wrap: wrap;
            justify-content: center;
            gap: 20px;
            margin-top: 30px;
        }
        canvas {
            max-width: 900px;
            height: 500px;
            border: 1px solid #ddd;
            border-radius: 8px;
            background: #ffffff;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
        }
        footer {
            text-align: center;
            padding: 10px 0;
            margin-top: 30px;
            font-size: 14px;
            background: #37474f;
            color: #eceff1;
        }
    </style>
</head>
<body>
    <header>
        <h1>Resistance Measurement</h1>
    </header>
    <div class="container">
        <div class="message-container">
            <div class="message-card">
                <h3>Final Word</h3>
                <span id="finalWord">Waiting...</span>
            </div>
            <div class="message-card">
                <h3>Current Word</h3>
                <span id="currentWord">Waiting...</span>
            </div>
        </div>
        <div class="charts">
            <canvas id="resistanceChart"></canvas>
        </div>
    </div>
    <footer>
        © 2024 WESA Team
    </footer>
<script>
    const ctx = document.getElementById('resistanceChart').getContext('2d');

    const resistanceChart = new Chart(ctx, {
        type: 'bar',
        data: {
            labels: Array.from({ length: 10 }, (_, i) => `R${i + 1}`),
            datasets: [{
                label: 'Resistance (kΩ)',
                data: Array(10).fill(0),
                backgroundColor: [
                    'rgba(54, 162, 235, 0.7)', // Mavi - İlk 5
                    'rgba(54, 162, 235, 0.7)',
                    'rgba(54, 162, 235, 0.7)',
                    'rgba(54, 162, 235, 0.7)',
                    'rgba(54, 162, 235, 0.7)',
                    'rgba(255, 99, 132, 0.7)', // Kırmızı - Son 5
                    'rgba(255, 99, 132, 0.7)',
                    'rgba(255, 99, 132, 0.7)',
                    'rgba(255, 99, 132, 0.7)',
                    'rgba(255, 99, 132, 0.7)'
                ],
                borderWidth: 1
            }]
        },
        options: {
            responsive: true,
            maintainAspectRatio: false,
            scales: {
                y: {
                    min: 0,
                    max: 50,
                    ticks: {
                        stepSize: 5
                    }
                }
            }
        }
    });

    if (!!window.EventSource) {
        const source = new EventSource('/events');
        source.addEventListener('update', function (e) {
            const data = JSON.parse(e.data);

            document.getElementById('finalWord').innerText = data.finalWord || '-';
            document.getElementById('currentWord').innerText = data.currentWord || '';

            if (data.resistance && Array.isArray(data.resistance)) {
                resistanceChart.data.datasets[0].data = data.resistance.map(val => parseFloat(val));
                resistanceChart.update();
            } else {
                console.warn('Invalid resistance data:', data.resistance);
            }
        });
    }
</script>

</body>
</html>
)rawliteral";

// Fonksiyon: JSON verisini işleme
void handleData(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, data);
    if (error) {
        Serial.println("Failed to parse JSON");
        request->send(400, "text/plain", "Invalid JSON");
        return;
    }

    for (int i = 0; i < 10; i++) {
        String key = "R" + String(i + 1);
        if (doc.containsKey(key)) {
            resistance[i] = doc[key];
        }
    }
}

// Fonksiyon: Tahmin işlemi
void classifyAndPredict() {
    float inputBuffer[BUFFER_SIZE] = {
        resistance[0], resistance[1], resistance[2], resistance[3], resistance[4],
        resistance[5], resistance[6], resistance[7], resistance[8], resistance[9]
    };

    signal_t signal;
    numpy::signal_from_buffer(inputBuffer, BUFFER_SIZE, &signal);

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, false);

    if (err != EI_IMPULSE_OK) {
        Serial.printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    float maxScore = 0.0;
    const char* predictedLabel = "";

    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        if (result.classification[ix].value > maxScore) {
            maxScore = result.classification[ix].value;
            predictedLabel = result.classification[ix].label;
        }
    }

    if (String(predictedLabel) != "Invalid" && maxScore >= 0.99) {
        String newLetter = String(predictedLabel);

        if (newLetter != lastLetter) {
            currentWord += newLetter;
            lastLetter = newLetter;
        }
    }
}

// Fonksiyon: Başlatma
void setup() {
    Serial.begin(115200);

    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(BLUE_LED, OUTPUT);

    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BLUE_LED, LOW);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected.");
    Serial.print("Receiver IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", index_html);
    });

    server.on("/data", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL, handleData);

    server.addHandler(&events);

    server.begin();
    Serial.println("HTTP Server started.");
}

// Fonksiyon: Ana döngü
void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastRunTime >= INTERVAL) {
        lastRunTime = currentTime;

        // Tahmin işlemi
        classifyAndPredict();

        bool allLow = true;
        for (int i = 0; i < 10; i++) {
            if (resistance[i] > 10) {
                allLow = false;
                break;
            }
        }

        if (allLow) {
            if (!currentWord.isEmpty()) {
                finalWord = currentWord;
                currentWord = "";
                lastLetter = "";
            }
        }

        // LED kontrolü
        if (finalWord == "AÇY") {
            digitalWrite(GREEN_LED, HIGH);
        } else if (finalWord == "KAPATY") {
            digitalWrite(GREEN_LED, LOW);
        } else if (finalWord == "AÇK") {
            digitalWrite(RED_LED, HIGH);
        } else if (finalWord == "KAPATK") {
            digitalWrite(RED_LED, LOW);
        } else if (finalWord == "AÇM") {
            digitalWrite(BLUE_LED, HIGH);
        } else if (finalWord == "KAPATM") {
            digitalWrite(BLUE_LED, LOW);
        }

        // Dashboard güncelleme
        DynamicJsonDocument jsonDoc(512);
        jsonDoc["finalWord"] = finalWord;
        jsonDoc["currentWord"] = currentWord;

        JsonArray resistanceArray = jsonDoc.createNestedArray("resistance");
        for (int i = 0; i < 10; i++) {
            resistanceArray.add(resistance[i]);
        }

        String jsonString;
        serializeJson(jsonDoc, jsonString);
        events.send(jsonString.c_str(), "update", millis());
    }
}
