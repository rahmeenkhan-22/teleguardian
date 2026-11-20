
#include <WiFi.h>
#include "ping/ping_sock.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

// ======================================
// TELEGUARDIAN - ESP32 DIAGNOSTIC PROBE
// Arduino IDE 2.3.10
// ESP32 Core 3.3.11
// ======================================

// Enter your own Wi-Fi credentials
const char* WIFI_SSID = "YOUR_WIFI_NAME";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

const int RSSI_SAMPLES = 20;
const int PING_COUNT = 10;
const int SAMPLE_INTERVAL = 1000;

float rssiValues[RSSI_SAMPLES];

SemaphoreHandle_t pingSemaphore;

uint32_t successfulPings = 0;
uint32_t totalPingTime = 0;
uint32_t transmittedPings = 0;
uint32_t receivedPings = 0;

// ======================================
// PING CALLBACKS
// ======================================

void onPingSuccess(esp_ping_handle_t handle, void* args) {
  uint32_t elapsed = 0;

  esp_ping_get_profile(
    handle,
    ESP_PING_PROF_TIMEGAP,
    &elapsed,
    sizeof(elapsed)
  );

  successfulPings++;
  totalPingTime += elapsed;
}

void onPingTimeout(esp_ping_handle_t handle, void* args) {
  // No ICMP reply received for this request.
}

void onPingEnd(esp_ping_handle_t handle, void* args) {
  esp_ping_get_profile(
    handle, ESP_PING_PROF_REQUEST,
    &transmittedPings, sizeof(transmittedPings)
  );

  esp_ping_get_profile(
    handle, ESP_PING_PROF_REPLY,
    &receivedPings, sizeof(receivedPings)
  );

  xSemaphoreGive(pingSemaphore);
}

// ======================================
// WIFI CONNECTION
// ======================================

bool connectWiFi() {
  Serial.println("\nConnecting to Wi-Fi...");

  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected!");

    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());

    return true;
  }

  Serial.println("\nWi-Fi Connection Failed!");
  return false;
}

// ======================================
// RSSI MEASUREMENT
// ======================================

void measureRSSI() {
  Serial.println("\n========== RSSI ANALYSIS ==========");

  float total = 0;
  int validSamples = 0;

  for (int i = 0; i < RSSI_SAMPLES; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi disconnected during sampling.");
      return;
    }

    rssiValues[i] = WiFi.RSSI();
    total += rssiValues[i];
    validSamples++;

    Serial.print("RSSI Sample ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(rssiValues[i]);
    Serial.println(" dBm");

    if (i < RSSI_SAMPLES - 1) {
      delay(SAMPLE_INTERVAL);
    }
  }

  if (validSamples == 0) return;

  float averageRSSI = total / validSamples;
  float squaredDifference = 0;

  for (int i = 0; i < validSamples; i++) {
    float difference = rssiValues[i] - averageRSSI;
    squaredDifference += difference * difference;
  }

  float variance = squaredDifference / validSamples;

  Serial.println("\n--- RSSI RESULTS ---");

  Serial.print("Average RSSI: ");
  Serial.print(averageRSSI, 2);
  Serial.println(" dBm");

  Serial.print("RSSI Variance: ");
  Serial.print(variance, 2);
  Serial.println(" dB^2");

  Serial.print("Signal Quality: ");

  if (averageRSSI >= -50) {
    Serial.println("EXCELLENT");
  } else if (averageRSSI >= -60) {
    Serial.println("GOOD");
  } else if (averageRSSI >= -67) {
    Serial.println("FAIR");
  } else if (averageRSSI >= -75) {
    Serial.println("WEAK");
  } else {
    Serial.println("VERY WEAK");
  }

  Serial.print("Signal Stability: ");

  if (variance < 4) {
    Serial.println("STABLE");
  } else if (variance < 16) {
    Serial.println("MODERATELY STABLE");
  } else {
    Serial.println("UNSTABLE");
  }
}

// ======================================
// GATEWAY ICMP PING
// ======================================

void measureGatewayPing() {
  Serial.println("\n========== GATEWAY PING TEST ==========");

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot ping: Wi-Fi disconnected.");
    return;
  }

  IPAddress gateway = WiFi.gatewayIP();

  ip_addr_t targetAddress = {};

  IP_ADDR4(
    &targetAddress,
    gateway[0],
    gateway[1],
    gateway[2],
    gateway[3]
  );

  successfulPings = 0;
  totalPingTime = 0;
  transmittedPings = 0;
  receivedPings = 0;

  pingSemaphore = xSemaphoreCreateBinary();

  if (pingSemaphore == NULL) {
    Serial.println("Ping semaphore creation failed.");
    return;
  }

  esp_ping_config_t pingConfig = ESP_PING_DEFAULT_CONFIG();

  pingConfig.target_addr = targetAddress;
  pingConfig.count = PING_COUNT;

  esp_ping_callbacks_t callbacks = {};

  callbacks.on_ping_success = onPingSuccess;
  callbacks.on_ping_timeout = onPingTimeout;
  callbacks.on_ping_end = onPingEnd;

  esp_ping_handle_t pingHandle = NULL;

  esp_err_t result = esp_ping_new_session(
    &pingConfig,
    &callbacks,
    &pingHandle
  );

  if (result != ESP_OK) {
    Serial.println("Could not create ping session.");
    vSemaphoreDelete(pingSemaphore);
    pingSemaphore = NULL;
    return;
  }

  Serial.print("Pinging gateway: ");
  Serial.println(gateway);

  esp_ping_start(pingHandle);

  if (xSemaphoreTake(
        pingSemaphore,
        pdMS_TO_TICKS(20000)
      ) == pdTRUE) {

    esp_ping_delete_session(pingHandle);

    Serial.println("\n--- PING RESULTS ---");

    Serial.print("Packets Sent: ");
    Serial.println(transmittedPings);

    Serial.print("Packets Received: ");
    Serial.println(receivedPings);

    float packetLoss = 0;

    if (transmittedPings > 0) {
      packetLoss =
        ((float)(transmittedPings - receivedPings) /
        transmittedPings) * 100.0;
    }

    Serial.print("ICMP Packet Loss: ");
    Serial.print(packetLoss, 1);
    Serial.println("%");

    if (successfulPings > 0) {
      float averageLatency =
        (float)totalPingTime / successfulPings;

      Serial.print("Average Gateway RTT: ");
      Serial.print(averageLatency, 2);
      Serial.println(" ms");

      Serial.print("Latency Interpretation: ");

      if (averageLatency < 20) {
        Serial.println("EXCELLENT");
      } else if (averageLatency < 50) {
        Serial.println("GOOD");
      } else if (averageLatency < 100) {
        Serial.println("FAIR");
      } else {
        Serial.println("SLOW");
      }
    } else {
      Serial.println("Average Gateway RTT: No replies");
    }

    Serial.print("Packet Loss Interpretation: ");

    if (packetLoss == 0) {
      Serial.println("NO LOSS DETECTED");
    } else if (packetLoss <= 10) {
      Serial.println("LOW LOSS");
    } else if (packetLoss <= 30) {
      Serial.println("MODERATE LOSS");
    } else {
      Serial.println("HIGH LOSS");
    }

  } else {
    esp_ping_stop(pingHandle);
    esp_ping_delete_session(pingHandle);
    Serial.println("Ping test timed out.");
  }

  vSemaphoreDelete(pingSemaphore);
  pingSemaphore = NULL;
}

// ======================================
// SETUP
// ======================================

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("\n==================================");
  Serial.println("       TELEGUARDIAN PROBE");
  Serial.println("    Wi-Fi Diagnostic System");
  Serial.println("==================================");

  if (connectWiFi()) {
    measureRSSI();
    measureGatewayPing();
  }

  Serial.println("\n==================================");
  Serial.println("      DIAGNOSTIC TEST COMPLETE");
  Serial.println("==================================");
}

void loop() {
  // One diagnostic cycle per ESP32 reset.
}
