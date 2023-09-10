/*********
项目使用S014进行ov5640保存影像到SD卡中
1.使用SdFat库
2.采用SoftSPI进行传输
3.需要进入SdFat库中修改SdFat/SdFatConfig.h，将SPI_DRIVER_SELECT改为2
by Oliver0804
*********/

#include "esp_camera.h"
#include "Arduino.h"

#include <EEPROM.h>            // read and write from flash memory

//SD
#include "SdFat.h"
#if SPI_DRIVER_SELECT == 2  //  

// SD_FAT_TYPE = 0 for SdFat/File as defined in SdFatConfig.h,
// 1 for FAT16/FAT32, 2 for exFAT, 3 for FAT16/FAT32 and exFAT.
#define SD_FAT_TYPE 0
//
// Chip select may be constant or RAM variable.
const uint8_t SD_CS_PIN = 19;
//
// S014 SD 卡配置
const uint8_t SOFT_MISO_PIN = 13;
const uint8_t SOFT_MOSI_PIN = 21;
const uint8_t SOFT_SCK_PIN = 4;

// SdFat software SPI template
SoftSpiDriver<SOFT_MISO_PIN, SOFT_MOSI_PIN, SOFT_SCK_PIN> softSpi;
// Speed argument is ignored for software SPI.
#if ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(0), &softSpi)
#else  // ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(0), &softSpi)
#endif  // ENABLE_DEDICATED_SPI


SdFat sd;
File file;
camera_fb_t * fb = NULL;


#define EEPROM_SIZE 1

// Pin definition for S014
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   5
#define XCLK_GPIO_NUM    15
#define SIOD_GPIO_NUM    22
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      39
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      33
#define Y6_GPIO_NUM      27
#define Y5_GPIO_NUM      12
#define Y4_GPIO_NUM      35
#define Y3_GPIO_NUM      14
#define Y2_GPIO_NUM      2
#define VSYNC_GPIO_NUM   18
#define HREF_GPIO_NUM    36
#define PCLK_GPIO_NUM    26

int pictureNumber = 0;
void initSoftSD(){
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }
  }
void writeTxt2sd(){
    
  if (!file.open("SoftSPI.txt", O_RDWR | O_CREAT)) {
    sd.errorHalt(F("open failed"));
  }
  file.println(F("This line was printed using software SPI."));

  file.rewind();

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();

  Serial.println(F("Done."));
  }
void saveImage2sd(){
    // 在這裡用SdFat庫來保存文件
  pictureNumber = EEPROM.read(0) + 1;
  String path = "/picture" + String(pictureNumber) + ".jpg";
  file = sd.open(path.c_str(), FILE_WRITE);

  if (!file) {
    Serial.println("Failed to open file in writing mode");
  } else {
    file.write(fb->buf, fb->len);
    Serial.printf("Saved file to path: %s\n", path.c_str());
    EEPROM.write(0, pictureNumber);
    EEPROM.commit();
  }
  
  file.close();
  }
void setup() {
 
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  //Serial.println();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 8000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_SXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  // Init SD
  initSoftSD();

  // write txt to sd 
  writeTxt2sd();

  
  
  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  delay(100);
  fb = esp_camera_fb_get();  

  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  
  //save esp32cam image to sd
  saveImage2sd();

  //釋放fb
  esp_camera_fb_return(fb); 
  
  delay(100);
  Serial.println("demo end.");

}

void loop() {

  fb = esp_camera_fb_get();  
  //save esp32cam image to sd
  saveImage2sd();

  //釋放fb
  esp_camera_fb_return(fb); 
  
  delay(5000);
}

#else  // SPI_DRIVER_SELECT
#error SPI_DRIVER_SELECT must be two in SdFat/SdFatConfig.h
#endif  // SPI_DRIVER_SELECT
