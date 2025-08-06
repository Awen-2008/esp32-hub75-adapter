#include <Arduino.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include <SD.h>
#include <FS.h>

#define DEBUG_MODE 1  // 1: 开启调试模式，0: 关闭调试模式

// 定义调试打印宏
#if DEBUG_MODE
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINTF(fmt, ...) Serial.printf(fmt, __VA_ARGS__)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(fmt, ...)
#endif

// SPI引脚定义
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCLK 18
#define SD_CS    5   //SS
MatrixPanel_I2S_DMA matrix;

// 这些宏定义是 HUB75 显示屏连接 ESP32 的 gpio 引脚定义
#define R1_PIN  25
#define G1_PIN  26
#define B1_PIN  27

#define R2_PIN  14
#define G2_PIN  17 
#define B2_PIN  13

#define A_PIN   32
#define B_PIN   33 
#define C_PIN   21
#define D_PIN   15
#define E_PIN   2	// required for 1/32 scan panels, set to 0,2,or 12 base your jumper setup
          
#define LAT_PIN 4
#define OE_PIN  22
#define CLK_PIN 16

const char* DISPLAY_TEXT = "Hello, \nWorld!";
const char* DATA_FILE = "/data.txt";
const int LOOP_DELAY = 5000;

bool sdCardAvailable = false;

void setup()
{
  Serial.begin(115200);

  // 初始化SPI总线(因为SD卡需要使用SPI通信)
  SPI.begin(SPI_SCLK, SPI_MISO, SPI_MOSI, SD_CS);

  // 初始化显示屏
  // 注意：在调用 matrix.begin() 之前，必须先初始化 SPI 总线。
  // 如果使用默认引脚，则不需要传递任何参数。

  //matrix.begin();  // Use default pins supplied within ESP32-HUB75-MatrixPanel-I2S-DMA.h
  matrix.begin(R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN );  // or custom pins
  
  // Draw a single white pixel
  //matrix.drawPixel(0,0, matrix.color565(255,0,0));      // 只能在.begin（）之后执行此操作
  matrix.setTextSize(1);

  sdCardAvailable = SD.begin(SD_CS, SPI);
  if(!sdCardAvailable){
      DEBUG_PRINTLN("SD Card not available");
      sdCardAvailable = false;
      return;
  } else {
    sdCardAvailable = true;
    DEBUG_PRINTLN("SD Card initialized successfully");
    // 打印SD卡信息
    DEBUG_PRINTLN("SD Card available");
  
    #ifdef ESP32
      uint8_t cardType = SD.cardType();

      if(cardType == CARD_NONE){
          DEBUG_PRINTLN("No SD card attached");
          return;
      }

      DEBUG_PRINTLN("SD Card Type: ");
      if(cardType == CARD_MMC){
          DEBUG_PRINTLN("MMC");
      } else if(cardType == CARD_SD){
          DEBUG_PRINTLN("SDSC");
      } else if(cardType == CARD_SDHC){
          DEBUG_PRINTLN("SDHC");
      } else {
          DEBUG_PRINTLN("UNKNOWN");
      }

      uint64_t cardSize = SD.cardSize();
      int cardSizeInMB = cardSize/(1024 * 1024);
     
      DEBUG_PRINTF("Card size: %u MB \n", cardSizeInMB);
 
      uint64_t bytesAvailable = SD.totalBytes(); 
      int spaceAvailableInMB = bytesAvailable/(1024 * 1024);
 
      DEBUG_PRINTF("Space available: %u MB \n", spaceAvailableInMB);
 
      uint64_t spaceUsed = SD.usedBytes(); 
      DEBUG_PRINTF("Space used: %u bytes\n", spaceUsed);

    #endif   
  }
}

// 函数 recordNewData() 是用于将新数据记录到指定的文件中
void recordNewData(fs::FS &fs, const char * path, const char * message)
{
  // 打开文件，以追加模式
  File file = fs.open(path, FILE_APPEND);

  // 如果打开文件失败，输出错误信息
  if(!file) {
    DEBUG_PRINTLN("Failed to open file for appending");
    // 尝试重新初始化SD卡
    sdCardAvailable = SD.begin(SD_CS);
    return;
  }

  // 将message写入文件中
  if(file.println(message)) {
    // 如果写入成功，输出成功信息
    DEBUG_PRINTLN("Data appended");
  } else {
    // 如果写入失败，输出失败信息
    DEBUG_PRINTLN("Append failed");
    // 尝试重新初始化SD卡
    sdCardAvailable = SD.begin(SD_CS);
  }
  // 关闭文件
  file.close();
}
void loop()
{
  //matrix.fillScreen(matrix.color565(255,255,255));
  //delay(2000);
  //matrix.fillScreen(matrix.color565(random(256),random(256),random(256)));
  matrix.clearScreen();

  // 设置文本颜色
  matrix.setTextColor(matrix.color565(random(256),random(256),random(256))); 
  // 设置光标位置
  matrix.setCursor(0,20);
  // 打印文本
  matrix.println(DISPLAY_TEXT);
  // 更新显示
  if (sdCardAvailable) {
    recordNewData(SD, DATA_FILE, DISPLAY_TEXT);
  }
  // 延迟5秒
  delay(LOOP_DELAY);
}
