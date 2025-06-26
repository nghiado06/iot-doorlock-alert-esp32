/*******************************************************************************
 * @file    receiver.ino
 * @author  Do Duc Nghia
 * @brief   Receiver Device receive signal and do functions
 * @details This file is written for config the receiving device. Receive signal
 *          and do alarming functions.
 *
 * @version 1.0
 * @date    2024-11-17
 * *****************************************************************************
 */

/********************************************************************
 * ====================== [ INCLUDE LIBRARY ] =======================
 ********************************************************************/
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <WiFiManager.h>
#include <HTTPClient.h>

/********************************************************************
 * ======================= [ BLYNK CONFIG ] =========================
 ********************************************************************/
#define BLYNK_TEMPLATE_ID "TMPL60vHn9oDu"
#define BLYNK_TEMPLATE_NAME "THIET BI CANH BAO NHAN"
#define BLYNK_AUTH_TOKEN "PKB8ONZBg_DMCIc0rCRDAOhvsH_wqEfF"

/********************************************************************
 * ========================= [ CODING ] =============================
 ********************************************************************/
// Hàm lấy dữ liệu từ ESP8266 thông qua Blynk Cloud
String getBlynkValue(String token, String pin)
{
  HTTPClient http;
  String url = "http://blynk.cloud/external/api/get?token=" + token + "&" + pin;
  http.begin(url);
  int httpResponseCode = http.GET();
  String payload = httpResponseCode > 0 ? http.getString() : "0"; // Mặc định trả về 0 nếu lỗi
  http.end();
  return payload;
}

// Khai báo chân linh kiện
const int ledRed = 2;       // LED đỏ
const int ledGreen = 4;     // LED xanh
const int buzzer = 14;      // Buzzer
const int CambienDong = 33; // Cảm biến trạng thái đóng

// Biến điều khiển
unsigned long lastBuzzTime = 0; // Thời điểm buzzer được kích hoạt lần cuối
int buzzCount = 0;              // Đếm số lần buzzer đã kêu
bool isBuzzerResting = false;   // Trạng thái nghỉ của buzzer

// Biến trạng thái cảm biến
bool stateDong = false; // Trạng thái cảm biến đóng
bool stateKhoa = false; // Trạng thái nhận qua Blynk

void setup()
{
  Serial.begin(115200);

  // Sử dụng WiFiManager để tự động kết nối WiFi
  WiFiManager wifiManager;
  if (!wifiManager.autoConnect("ESP32_AP"))
  {
    Serial.println("Không thể kết nối WiFi.");
    delay(3000);
    ESP.restart(); // Khởi động lại nếu không kết nối được
  }

  // Kết nối Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, WiFi.SSID().c_str(), WiFi.psk().c_str());
  Serial.println("Kết nối WiFi và Blynk thành công!");

  // Thiết lập các chân linh kiện
  pinMode(ledRed, OUTPUT);
  pinMode(ledGreen, OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(CambienDong, INPUT);

  // Tắt thiết bị ban đầu
  digitalWrite(ledRed, LOW);
  digitalWrite(ledGreen, LOW);
  digitalWrite(buzzer, LOW);
}

void loop()
{
  Blynk.run(); // Chạy Blynk

  // Cập nhật trạng thái từ Blynk Cloud
  String sensorValue = getBlynkValue("t51NitTmbqL0ZIQSce6630sdXghh_gnT", "V3"); // Thay token ESP8266
  stateKhoa = (sensorValue == "1");

  // Đọc trạng thái cảm biến
  stateDong = digitalRead(CambienDong);

  // Biến lưu trạng thái gửi lên Blynk
  String statusMessageDong = "";
  String statusMessageKhoa = "";

  // Xử lý trạng thái cảm biến và thiết bị
  if (stateDong && !stateKhoa)
  {
    digitalWrite(ledRed, LOW);
    digitalWrite(ledGreen, HIGH);
    digitalWrite(buzzer, LOW); // Tắt buzzer
    statusMessageDong = "Ban Da Dong";
    statusMessageKhoa = "Ban Da Khoa";
    buzzCount = 0;           // Reset đếm buzzer
    isBuzzerResting = false; // Hủy trạng thái nghỉ
  }
  else if (!stateDong)
  {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    statusMessageDong = "Ban Chua Dong";
    statusMessageKhoa = "Ban Chua Khoa";
    handleBuzzer(); // Kích hoạt buzzer
  }
  else if (stateDong && stateKhoa)
  {
    digitalWrite(ledRed, HIGH);
    digitalWrite(ledGreen, LOW);
    statusMessageDong = "Ban Da Dong";
    statusMessageKhoa = "Ban Chua Khoa";
    handleBuzzer(); // Kích hoạt buzzer
  }

  // Gửi trạng thái lên Blynk
  Blynk.virtualWrite(V1, statusMessageDong); // Gửi trạng thái đóng
  Blynk.virtualWrite(V2, statusMessageKhoa); // Gửi trạng thái khóa

  // Hiển thị trạng thái qua Serial Monitor
  Serial.println("=== Trạng thái sau xử lý ===");
  Serial.print("CambienDong: ");
  Serial.println(stateDong ? "HIGH" : "LOW");
  Serial.print("CambienKhoa: ");
  Serial.println(stateKhoa ? "HIGH" : "LOW");
  Serial.print("LED Đỏ: ");
  Serial.println(digitalRead(ledRed) ? "BẬT" : "TẮT");
  Serial.print("LED Xanh: ");
  Serial.println(digitalRead(ledGreen) ? "BẬT" : "TẮT");
  Serial.print("Trạng thái đóng: ");
  Serial.println(statusMessageDong);
  Serial.print("Trạng thái khóa: ");
  Serial.println(statusMessageKhoa);
  Serial.print("Buzzer: ");
  Serial.println(digitalRead(buzzer) ? "BẬT" : "TẮT");
  Serial.println("==========================");
  delay(1000);
}

// Hàm điều khiển buzzer
void handleBuzzer()
{
  unsigned long currentTime = millis();

  if (isBuzzerResting)
  {
    if (currentTime - lastBuzzTime >= 10000)
    {
      buzzCount = 0;
      isBuzzerResting = false;
    }
    return;
  }

  if (buzzCount < 5)
  {
    if (currentTime - lastBuzzTime >= 1000)
    {
      digitalWrite(buzzer, HIGH);
      delay(50);
      digitalWrite(buzzer, LOW);
      lastBuzzTime = millis();
      buzzCount++;
      Serial.println("Buzzer đang kêu...");
    }
  }
  else
  {
    isBuzzerResting = true;
    lastBuzzTime = millis();
    Serial.println("Buzzer đang nghỉ...");
  }
}
