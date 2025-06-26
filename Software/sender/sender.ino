

#define BLYNK_TEMPLATE_ID "TMPL6w6NBriV1"
#define BLYNK_TEMPLATE_NAME "THIET BI CANH BAO GUI"
#define BLYNK_AUTH_TOKEN "t51NitTmbqL0ZIQSce6630sdXghh_gnT"

#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <WiFiManager.h>

// Khai báo chân cảm biến
const int cambien = D8; // Cảm biến
int lastSensorValue = -1;

void setup()
{
  Serial.begin(115200);

  // Khởi tạo WiFiManager
  WiFiManager wifiManager;

  // Tạo AP cho cấu hình WiFi mới
  if (!wifiManager.autoConnect("ESP8266_AP"))
  {
    Serial.println("Không thể kết nối WiFi.");
    delay(3000);
    ESP.restart();
  }

  Serial.println("Kết nối WiFi thành công.");

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
  Serial.println("Đã kết nối WiFi.");
  Serial.print("Địa chỉ IP ESP8266: ");
  Serial.println(WiFi.localIP());

  pinMode(cambien, INPUT); // Thiết lập chân cảm biến làm input
}

void loop()
{
  Blynk.run();

  // Đọc tín hiệu từ cảm biến
  int sensorValue = digitalRead(cambien);

  if (sensorValue != lastSensorValue)
  {
    lastSensorValue = sensorValue;

    // Gửi tín hiệu lên Blynk
    Blynk.virtualWrite(V3, sensorValue);
    Serial.print("Gửi tín hiệu: ");
    Serial.println(sensorValue ? "HIGH" : "LOW");
  }

  delay(500); // Trễ 500ms trước khi đọc lại
}
