#include <WiFi.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

const char* ssid = "Please add your WIFI SSID";
const char* passwd = "Please add your WIFI PASSWORD";
const char* url = "https://www.saucedemo.com/";
const char* FileFormat = "/FileFormat.bin";

void connect_to_wifi(){
  WiFi.mode(WIFI_STA);  //connection to Wifi access point....ESP will not act as access point
  WiFi.begin(ssid, passwd);
  Serial.println("Connecting to Wi-Fi !");

  int tryDelay = 1000;
  int numberofTries = 20;

  while (true){
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL: Serial.println("[WiFi] SSID not found..."); break;
      case WL_CONNECT_FAILED:
        Serial.println("[WiFi] Failed - WiFi not connected! Reason: ");
        return;
        break;
      case WL_CONNECTION_LOST: Serial.println("[WiFi] Connection lost..."); break;
      case WL_SCAN_COMPLETED: Serial.println("[WiFi] Scan is Completed..."); break;
      case WL_DISCONNECTED: Serial.println("[WiFi] Wifi is disconnected...."); break;
      case WL_CONNECTED: 
        Serial.println("[WiFi] WiFi is connected successfully...");
        Serial.print("WiFi local IP Address: ");
        Serial.println(WiFi.localIP());
        return;
        break;
      default:
        Serial.print("[WiFi] WiFi status: ");
        Serial.println(WiFi.status());
        break;
    }

    delay(tryDelay);

    if (numberofTries <= 0){
      Serial.println("[WiFi] Failed to connect to WiFi...");
      WiFi.disconnect();
      return;
    }
    else{
      numberofTries --;
    }
  }
}

void writetoSPIFFS(const char* FileFormat, const char* url){
  //Now, writing to a file storing HTTP URL Request on a file path
  WiFiClientSecure *wcs = new WiFiClientSecure;
  wcs->setInsecure(); //This is for Public Servers which do not have certificate validation
  HTTPClient http;
  http.begin(*wcs, url);
  int httpResponseCode = http.GET();

  size_t totalBytes = 0;
  uint8_t buffer[4096]; // Using a fixed 4KB buffer
  
  unsigned long start = millis();
  if (httpResponseCode == 200) {

    Serial.println("writing stream to file");
    File file = SPIFFS.open(FileFormat, "w");
    // WiFiClient& stream = http.getStream();

    int contentLength = http.getSize();
    Serial.print("Content length: ");
    Serial.println(contentLength);

  WiFiClient* stream = http.getStreamPtr();

  while (http.connected() && (contentLength == -1 || totalBytes < contentLength)) {  //To stop the possibility of infinite loop
    // Check for available data
    int available = stream->available();
    if (available > 0) {
      // Read up to buffer size
      int bytesRead = stream->readBytes(buffer, min(available, (int)sizeof(buffer)));
    
      // Write to file
     file.write(buffer, bytesRead);
      totalBytes += bytesRead;
    
      // Print progress every 50KB
      if (totalBytes % (50 * 1024) < 4096) {
        Serial.printf("Downloaded: %d bytes", totalBytes);
        if (contentLength > 0) {
          Serial.printf(" (%.1f%%)\n", (totalBytes * 100.0) / contentLength);
        } else {
        Serial.println();
        }
      }
    } else {
    delay(1); // Small delay if no data available
    }
}

    unsigned long end = millis();

    file.close();
    float no_of_seconds = (end - start)/1000.00;  //provides number of seconds
    float kilo_bytes = totalBytes/1024.00;
    float speed_KB = kilo_bytes/no_of_seconds;
    float kilo_bits = (totalBytes * 8.0)/1000.0;
    float speed_Kb = kilo_bits/no_of_seconds;
    Serial.printf("Speed in KiloBytes per second: %.2f", speed_KB); 
    Serial.println();
    Serial.printf("Speed in Kilobits per second: %.2f", speed_Kb);
    Serial.println();
    Serial.println("done");
    Serial.println("stream written to file");
  }
  else{
    Serial.print("Http error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  // return totalBytes;
  delete wcs;
}

void readfile(const char* FileFormat){
  //This is reading of file
  File r = SPIFFS.open(FileFormat, "r");
  if(!r){
    Serial.println("Could not open file....");
  }
  else{
    Serial.println("File loaded");
  }
  
  // Serial.println("File Content: ");  ////This section can be uncommented if we wish to read through entire file that is loaded
  // while (r.available()){  
  //   Serial.write(r.read());
  // }
  
  r.close();
  Serial.printf("total = %u used = %u \n", (unsigned)SPIFFS.totalBytes(), (unsigned)SPIFFS.usedBytes());  //Notice here printf is written
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); //115200 as baud rate
  connect_to_wifi();
  Serial.print("RSSI: ");
  Serial.println(WiFi.RSSI());   //RSSI stands for Received Signal Strength Indicator and it stands for 

  if(!SPIFFS.begin(true)){
    Serial.println("An error has occurred while mounting SPIFFS");
    while(1);
  }
  
  writetoSPIFFS(FileFormat, url);
  delay(2000);

  readfile(FileFormat);
  delay(2000);

}

void loop() {
}
