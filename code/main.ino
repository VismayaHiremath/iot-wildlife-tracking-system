#include <TinyGPS++.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;

// GPS on UART1
HardwareSerial gpsSerial(1);

// GSM on UART2
HardwareSerial gsmSerial(2);

#define RXD1 16
#define TXD1 17

#define RXD2 26
#define TXD2 27

// Home location (set your coordinates)
float homeLat = 12.9716;
float homeLng = 77.5946;

float safeRadius = 100.0; // meters

void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXD1, TXD1);
  gsmSerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Serial.println("System Started...");
}

void loop() {

  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
  }

  if (gps.location.isUpdated()) {

    float lat = gps.location.lat();
    float lng = gps.location.lng();

    Serial.print("Lat: ");
    Serial.println(lat);
    Serial.print("Lng: ");
    Serial.println(lng);

    float distance = calculateDistance(homeLat, homeLng, lat, lng);

    Serial.print("Distance: ");
    Serial.println(distance);

    if (distance > safeRadius) {
      Serial.println("ALERT: Animal out of safe zone!");
      sendSMS(lat, lng);
      delay(10000); // avoid spam
    }
  }
}

float calculateDistance(float lat1, float lon1, float lat2, float lon2) {
  float R = 6371000; // Earth radius in meters

  float dLat = radians(lat2 - lat1);
  float dLon = radians(lon2 - lon1);

  float a = sin(dLat/2) * sin(dLat/2) +
            cos(radians(lat1)) * cos(radians(lat2)) *
            sin(dLon/2) * sin(dLon/2);

  float c = 2 * atan2(sqrt(a), sqrt(1-a));
  return R * c;
}

void sendSMS(float lat, float lng) {
  gsmSerial.println("AT+CMGF=1");
  delay(1000);

  gsmSerial.println("AT+CMGS=\"+911234567890\""); // replace number
  delay(1000);

  gsmSerial.print("Alert! Animal moved out of safe zone.\n");
  gsmSerial.print("Location: https://maps.google.com/?q=");
  gsmSerial.print(lat, 6);
  gsmSerial.print(",");
  gsmSerial.print(lng, 6);

  delay(1000);
  gsmSerial.write(26); // CTRL+Z
}
