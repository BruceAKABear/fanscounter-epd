/**
 * 粉丝计数器/天气预报主控程序
 * 
 * 蓝牙UUID生成网址为：https://www.uuidgenerator.net/
 * 
*/

//将蓝牙头文件包含进来
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <GxEPD2_BW.h>

// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V
GxEPD2_BW <GxEPD2_290, GxEPD2_290::HEIGHT> display(GxEPD2_290(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/FreeMonoBold12pt7b.h>
#include <Fonts/FreeMonoBold24pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include <Fonts/FreeSans9pt7b.h>

#include <Fonts/Org_01.h>
#include "FS.h"
#include "SPIFFS.h"
#include <ArduinoJson.h>
#include "custom_image.h"
//包含wifi
#include <WiFi.h>

//自定义配置
#define BTNAME "BLabfanscounter"
#define SERVICE_UUID "71db3107-6be5-4c0b-ad8e-731df7069e62"
#define READ_CHARACTERISTIC_UUID "8b3aed0e-a5a0-4e7f-9965-d0f3d26a1b36"
#define WRITE_CHARACTERISTIC_UUID "2bbd7b92-5bc8-441c-a69a-5764d4f3d1a8"
#define BILIBILI_FANS_GET_URL "https://api.bilibili.com/x/relation/stat?vmid=402729300"
#define REBOOT_PIN 10
//是否配置标识
boolean configed = false;
//工作模式，1为粉丝计数器模式，2为天气预报模式
int workType = 1;
//wifi ssid
char *ssid = "daxiongjia_2.4G";
//wifi 密码
char *password = "dengyi19911114";
//wifi连接标识
boolean wifiConnected = false;
//bilibili用户id
String biliUserId = "";
//YouTube用户key
String youtubeKey = "";
//配置json
String configJson = "123";
//系统是否有异常
boolean systemHasError = false;

//3. 定义写数据回调函数
class MyCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            //遍历所有字节
            for (int i = 0; i < value.length(); i++) {
                configJson += value[i];
            }
            Serial.print("---" + configJson);
        }
    }
};

/**
 * 初始化函数
 * 在系统启动时，只会执行一次
 *
 * 执行顺序：1.启动蓝牙->2.读取配置文件->3.显示logo页面->4.获取信息显示界面
*/
void setup() {
    Serial.begin(115200);
    display.init(115200);
    pinMode(REBOOT_PIN, OUTPUT);
    //1.启动蓝牙
    initBLE();
    initWifiAndShowConnectingWifi();
    showTianqiMainPage();
    //2.读取配置文件
    //    if (SPIFFS.begin()) {
    //        Serial.println("文件系统挂载成功");
    //        // 3.显示logo页面
    //        showLogo();
    //        //4.读物配置文件
    //        readProperties();
    //        if (!configed) {
    //            showNotConfig();
    //        }
    //        initAndConnectWIFI();
    //        //初始化显示主页
    //        initShowMainPage();
    //    } else {
    //        //文件系统如果初始化失败，提示用户
    //        Serial.println("文件系统挂载失败");
    //        systemHasError = true;
    //        showSystemError();
    //    }
}

void loop() {
    //只有在系统没有异常时才进行循环
    if (!systemHasError) {
        //如果工作模式为粉丝计数器模式
        if (workType == 1) {
            //1.请求bilibili粉丝数据，数据请求成功以后局部更新
            //2.请求youtube粉丝数据，数据请求成功以后局部更新
        }
        //如果粉丝计数器是天气预报模式
        if (workType == 2) {
        }
    }
}

/**
 * 软件重启系统方法
 */
void rebootSystem() {
    digitalWrite(REBOOT_PIN, LOW);
}

/**
 * 初始化蓝牙方法
 *
*/
void initBLE() {
    //1. 蓝牙广播名字
    BLEDevice::init(BTNAME);
    //2. 创建蓝牙服务
    BLEServer *pServer = BLEDevice::createServer();
    //3. 创建蓝牙主服务
    BLEService *pService = pServer->createService(SERVICE_UUID);
    //4. 创建读取特征
    BLECharacteristic *readCharacteristic = pService->createCharacteristic(
            READ_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_READ);
    //5. 创建写特征
    BLECharacteristic *writeCharacteristic = pService->createCharacteristic(
            WRITE_CHARACTERISTIC_UUID,
            BLECharacteristic::PROPERTY_WRITE);
    //6. 设置读取的值
    readCharacteristic->setValue("123456");
    //设置写数据回调
    writeCharacteristic->setCallbacks(new MyCallbacks());
    //7. 启动服务
    pService->start();
    //8. 设置广播信息
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("蓝牙初始化完成!");
}
/**
 * 显示wifi连接中
*/
void initWifiAndShowConnectingWifi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setRotation(1);
        display.drawBitmap(108, 95, font_wang, 16, 16, GxEPD_BLACK);
        display.drawBitmap(124, 95, font_luo, 16, 16, GxEPD_BLACK);
        display.drawBitmap(140, 95, font_lian, 16, 16, GxEPD_BLACK);
        display.drawBitmap(156, 95, font_jie, 16, 16, GxEPD_BLACK);
        display.drawBitmap(172, 95, font_zhong, 16, 16, GxEPD_BLACK);
        display.setRotation(2);
        display.drawBitmap(10, 113, image_wifi, 70, 70, GxEPD_BLACK);
    } while (display.nextPage());
    //设置局部更新区域
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);
    uint16_t box_x = 188;
    uint16_t box_y = 103;
    uint16_t box_w = 70;
    uint16_t box_h = 20;
    uint16_t cursor_x = box_x;
    uint16_t cursor_y = box_y + 0;
    uint8_t showTime = 1;
    display.setPartialWindow(box_x, box_y, box_w, box_h);
    display.setFont(&FreeMonoBold12pt7b);
    while (WiFi.status() != WL_CONNECTED) {
        display.setCursor(cursor_x, cursor_y);
        display.firstPage();
        if (showTime > 5) {
            showTime = 1;
            display.setCursor(cursor_x, cursor_y);
        } else {
            for (int i = 1; i <= showTime; ++i) {
                Serial.println(i);
                display.print(".");
            }
            delay(500);
            showTime++;
        }
        display.nextPage();
    }
    //wifi连接上以后才会进到这一步
    wifiConnected = true;

}

/**
 * 初始化显示主页
 */
void initShowMainPage() {
    if (workType == 1) {
        //worktype==1粉丝计数器模式
        showFensiMainPage();
    } else if (workType == 2) {
        //worktype==2天气预报模式
        showTianqiMainPage();
    }
}

/**
 * 显示主页方法
 *
 * 主页显示延迟3秒，显示logo图片，同时显示标语
*/
void showLogo() {
    display.setFullWindow();
    display.firstPage();
    do {
        display.setRotation(1);
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(58, 104, font_yong, 16, 16, GxEPD_BLACK);
        display.drawBitmap(74, 104, font_yuan, 16, 16, GxEPD_BLACK);
        display.drawBitmap(92, 104, font_bao, 16, 16, GxEPD_BLACK);
        display.drawBitmap(108, 104, font_chi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(124, 104, font_hao, 16, 16, GxEPD_BLACK);
        display.drawBitmap(140, 104, font_qi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(156, 104, font_xin, 16, 16, GxEPD_BLACK);
        display.drawBitmap(172, 104, font_he, 16, 16, GxEPD_BLACK);
        display.drawBitmap(188, 104, font_chuang, 16, 16, GxEPD_BLACK);
        display.drawBitmap(204, 104, font_zao, 16, 16, GxEPD_BLACK);
        display.drawBitmap(220, 104, font_li, 16, 16, GxEPD_BLACK);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(124, 96);
        display.println("BLab");
        display.setRotation(2);
        display.drawBitmap(5, 113, image_logo, 70, 70, GxEPD_BLACK);
    } while (display.nextPage());
    //保持3秒
    delay(3000);
}

/**
 * 读取配置文件,同时将配置文件解析出来赋值给全局变量
 *
 * 如果配置文件存在则解析出配置，如果配置文件不存在提示用户配置
 *
*/
void readProperties() {
    //检查文件是否存在
    boolean exist = SPIFFS.exists("/config.json");
    if (exist) {
        configed = true;
        File file = SPIFFS.open("/config.json", "r");
        String data = file.readString();
        configJson = data;
    } else {
        Serial.println("配置文件不存在");
    }
}

/**
 * 显示系统错误方法，次方法主要在系统初始化时获取配置文件时错误使用
 * 如果获取配置文件挂载文件系统时失败时提示
 */
void showSystemError() {
    display.setTextColor(GxEPD_BLACK);
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setRotation(1);
        display.drawBitmap(116, 95, font_xi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(132, 95, font_tong, 16, 16, GxEPD_BLACK);
        display.drawBitmap(148, 95, font_cuo, 16, 16, GxEPD_BLACK);
        display.drawBitmap(164, 95, font_wu, 16, 16, GxEPD_BLACK);
        display.setRotation(2);
        display.drawBitmap(10, 113, image_error, 70, 70, GxEPD_BLACK);

    } while (display.nextPage());
}

void writeProperties(String jsonstring) {
}

/**
 * 显示未配置页面方法
*/
void showNotConfig() {
    display.setFullWindow();
    display.firstPage();
    do {
        display.fillScreen(GxEPD_WHITE);
        display.setRotation(1);
        display.drawBitmap(108, 95, font_qing, 16, 16, GxEPD_BLACK);
        display.drawBitmap(124, 95, font_pei, 16, 16, GxEPD_BLACK);
        display.drawBitmap(140, 95, font_zhi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(156, 95, font_xi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(172, 95, font_tong, 16, 16, GxEPD_BLACK);
        display.setRotation(2);
        display.drawBitmap(10, 113, image_set, 70, 70, GxEPD_BLACK);
    } while (display.nextPage());
    //死循环监听是否配置成功
    while (true) {
        if (configed) {
            return;
        }
    }
}


/***
 * 粉丝计数器模式主页
 */
void showFensiMainPage() {
    display.setFullWindow();
    display.firstPage();
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);
    do {
        display.fillScreen(GxEPD_WHITE);
        //显示时间日期
        display.setFont(&FreeMonoBold12pt7b);
        display.setCursor(5, 20);
        display.println("2019-10-11");
        display.setFont(&FreeMonoBold9pt7b);
        display.setCursor(150, 20);
        display.println("11:11");
        display.drawLine(0, 28, 296, 28, GxEPD_BLACK);
        display.drawLine(100, 28, 100, 128, GxEPD_BLACK);
        //画分类线
        display.drawBitmap(108, 30, image_youtube, 47, 47, GxEPD_BLACK);
        display.drawLine(100, 78, 296, 78, GxEPD_BLACK);
        display.drawBitmap(108, 80, image_bilibili, 47, 47, GxEPD_BLACK);

        //显示粉丝数
        display.setFont(&FreeMonoBold12pt7b);
        //查询youtube粉丝数
        display.setCursor(185, 60);
        display.println(".");
        //查询bilibili粉丝数
        display.setCursor(185, 113);
        display.println("2600000");
        /* code */
    } while (display.nextPage());
}

/**
 * 天气预报模式主页
 */
void showTianqiMainPage() {
    display.setFullWindow();
    display.firstPage();
    display.setTextColor(GxEPD_BLACK);
    display.setRotation(1);
    do {
        display.fillScreen(GxEPD_WHITE);
        display.drawBitmap(40, 5, image_qing, 70, 70, GxEPD_BLACK);
        //日期时间
        display.setFont(&Org_01);
        display.setCursor(35, 86);
        display.print("2020-01-28 16:38");

        display.drawLine(150, 0, 150, 80, GxEPD_BLACK);
        display.drawLine(150, 40, 296, 40, GxEPD_BLACK);
        display.drawLine(150, 80, 296, 80, GxEPD_BLACK);
        //地址
        display.drawBitmap(14, 100, font16_jia, 24, 22, GxEPD_BLACK);
        display.drawBitmap(34, 100, font16_xing, 24, 22, GxEPD_BLACK);
        //竖隔线
        display.drawLine(60, 95, 60, 124, GxEPD_BLACK);
        //天气
        display.drawBitmap(68, 96, font9_qing, 16, 12, GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);
        display.setCursor(65, 122);
        display.print("25");
        display.drawBitmap(87, 112, font9_sheshidu, 16, 12, GxEPD_BLACK);
        display.setCursor(100, 122);
        display.print("-");
        display.print("30");
        display.drawBitmap(133, 112, font9_sheshidu, 16, 12, GxEPD_BLACK);
        /*
         * ----------------预报信息------------------------------------
         * */
        //明天
        display.drawBitmap(160, 5, font9_ming, 16, 12, GxEPD_BLACK);
        display.drawBitmap(174, 5, font9_tian, 16, 12, GxEPD_BLACK);
        display.setFont(&Org_01);
        display.setCursor(160, 35);
        display.print("2020-01-29");
        display.drawBitmap(250, 4, image_2020_qing, 20, 20, GxEPD_BLACK);
        //后天
        display.drawBitmap(160, 45, font9_hou, 16, 12, GxEPD_BLACK);
        display.drawBitmap(174, 45, font9_tian, 16, 12, GxEPD_BLACK);
        display.setFont(&Org_01);
        display.setCursor(160, 75);
        display.print("2020-01-29");
        display.drawBitmap(250, 44, image_2020_yin, 20, 20, GxEPD_BLACK);
        //实时温度湿度
        display.drawBitmap(160, 100, font_wen, 16, 16, GxEPD_BLACK);
        display.drawBitmap(176, 100, font_du, 16, 16, GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);
        display.setCursor(190, 113);
        display.print("25");
        display.drawBitmap(212, 105, font9_sheshidu, 16, 12, GxEPD_BLACK);

        display.drawBitmap(228, 100, font_shi, 16, 16, GxEPD_BLACK);
        display.drawBitmap(244, 100, font_du, 16, 16, GxEPD_BLACK);
        display.setFont(&FreeMono9pt7b);
        display.setCursor(260, 113);
        display.print("30");

    } while (display.nextPage());
}
