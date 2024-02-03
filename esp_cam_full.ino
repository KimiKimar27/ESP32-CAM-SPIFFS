/*
brownout
xclk
ap
img quality
*/

#include "ESPAsyncWebServer.h"
#include "WiFi.h"
#include "SPIFFS.h"
#include "FS.h"
#include "esp_camera.h"

#include "camera_pins.h"
#include "utils.h"

#define FORMAT_SPIFFS false

const char* ssid = "*";
const char* password = "*";

AsyncWebServer server(80);

void setup() {
  Serial.begin(115200);

  //
  // WIFI SETUP
  //
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  //
  // ROUTES SETUP
  //
  // Download file by going to <ip_addr>/?name=<file_name>
  // Example: 192.168.0.24/?name=hello.txt
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String fileName = request->arg("name");
    if (SPIFFS.exists("/" + fileName)) {
      request->send(SPIFFS, "/" + fileName, "application/octet-stream");
    } else {
      request->send(404, "text/plain", "File not found");
    }
  });
  // Start server
  server.begin();

  //
  // SPIFFS SETUP
  //
  if (!SPIFFS.begin(FORMAT_SPIFFS)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }

  //
  // CAMERA SETUP
  //
  // Camera init
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = 5;
  config.pin_d1 = 18;
  config.pin_d2 = 19;
  config.pin_d3 = 21;
  config.pin_d4 = 36;
  config.pin_d5 = 39;
  config.pin_d6 = 34;
  config.pin_d7 = 35;
  config.pin_xclk = 0;
  config.pin_pclk = 22;
  config.pin_vsync = 25;
  config.pin_href = 23;
  config.pin_sscb_sda = 26;
  config.pin_sscb_scl = 27;
  config.pin_pwdn = 32;
  config.pin_reset = -1;
  config.xclk_freq_hz = 20000000;
  //config.xclk_freq_hz = 10000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  // Start the camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  } else Serial.println("Camera initialized");

  //
  // EXECUTE FUNCTIONS
  //
  capturePhoto(); // Save photo to SPIFFS
  esp_camera_deinit(); // Deinitialize camera

}

void capturePhoto() {
  // Store image in buffer
  Serial.println("Capturing image...");
  camera_fb_t* fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Save the captured image to SPIFFS
  File file = SPIFFS.open("/image.jpg", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
  } else {
    file.write(fb->buf, fb->len);
    Serial.println("Image saved to SPIFFS");
  }
  file.close();

  // Return the frame buffer to the camera
  esp_camera_fb_return(fb);
}

void loop() {}
