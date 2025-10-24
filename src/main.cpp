#ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic error "-Wall"
#else // ifdef CONTINUOUS_INTEGRATION
# pragma GCC diagnostic warning "-Wall"
#endif // ifdef CONTINUOUS_INTEGRATION

#include <Arduino.h>

// Include this as first, to make sure all defines are active during the entire compile.
// See: https://www.letscontrolit.com/forum/viewtopic.php?f=4&t=7980
// If Custom.h build from Arduino IDE is needed, uncomment #define USE_CUSTOM_H in ESPEasy_common.h
#include "ESPEasy_common.h"
#include "ESPEasy-Globals.h"
#include "src/Globals/Settings.h"
#include "src/Helpers/SystemVariables.h"

#include "Custom.h"

// USB HID RFID modülü
#include "usb/usb_hid_rfid.h"

// Custom LVGL init fonksiyonu
#include "custom/custom.h"

// Global variables
struct EventStruct TempEvent;

// Server bağlantı durumu - custom.cpp için
bool serverConnectionStatus = false;

// Server bağlantı durumunu güncelleme fonksiyonu
void updateServerConnectionStatus(bool connected) {
    serverConnectionStatus = connected;
    SystemVariables::setServerStatus(connected);
    //Serial.printf("[MAIN] Server bağlantı durumu güncellendi: %s\n", connected ? "BAĞLI" : "BAĞLI DEĞİL");
}


#ifdef ESP_NOW_ACTIVE
#include "src/Globals/Settings.h"
#include <ctype.h> // isdigit fonksiyonu için

void updatePeerStatus(const uint8_t *mac_addr, bool active) {
  for (int i = 0; i < peerStatusCount; i++) {
    if (memcmp(peerStatusList[i].mac, mac_addr, 6) == 0) {
      peerStatusList[i].active = active;
      return;
    }
  }

  if (peerStatusCount < MAX_PAIRED_DEVICES) {
    memcpy(peerStatusList[peerStatusCount].mac, mac_addr, 6);
    peerStatusList[peerStatusCount].active = active;
    peerStatusCount++;
  }
}

void LoadPairedMac() {
  preferences.begin("espnow", true);
  int macDataLength = preferences.getBytesLength("paired_mac");
  pairedDeviceCount = macDataLength / 6;
  if (pairedDeviceCount > MAX_PAIRED_DEVICES) pairedDeviceCount = MAX_PAIRED_DEVICES;
  if (pairedDeviceCount > 0) {
    preferences.getBytes("paired_mac", pairedMacList, macDataLength);
    isPaired = true;
    for (int i = 0; i < pairedDeviceCount; i++) {
      esp_now_peer_info_t peerInfo = {};
      memcpy(peerInfo.peer_addr, pairedMacList[i], 6);
      peerInfo.channel = 0;
      peerInfo.encrypt = false;
      esp_now_add_peer(&peerInfo);
      updatePeerStatus(pairedMacList[i], false);
    }
  } else {
    isPaired = false;
  }
  preferences.end();
}

void RemovePairedMac(const char* macStr) {
  uint8_t mac[6];
  sscanf(macStr, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
         &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);

  int index = -1;
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], mac, 6) == 0) {
      index = i;
      break;
    }
  }

  if (index >= 0) {
    // listedeki öğeyi kaydırarak sil
    for (int i = index; i < pairedDeviceCount - 1; i++) {
      memcpy(pairedMacList[i], pairedMacList[i + 1], 6);
    }
    pairedDeviceCount--;

    // preferences’a gerçekten yaz
    preferences.begin("espnow", false);
    if (pairedDeviceCount > 0) {
      preferences.putBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);
    } else {
      preferences.remove("paired_mac");  // hepsi silindiyse key'i kaldır
    }
    preferences.end();

    // RAM'den ve esp-now'dan da çıkar
    esp_now_del_peer(mac);
    Serial.println("🧹 MAC başarıyla silindi.");
  } else {
    Serial.println("⚠️ MAC adresi listede bulunamadı.");
  }
}

void addToDiscoveredList(const uint8_t *mac_addr) {
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], mac_addr, 6) == 0) return;
  }
  for (int i = 0; i < discoveredCount; i++) {
    if (memcmp(discoveredMacList[i], mac_addr, 6) == 0) return;
  }
  if (discoveredCount < MAX_PAIRED_DEVICES) {
    memcpy(discoveredMacList[discoveredCount], mac_addr, 6);
    discoveredCount++;
  }
}

void SavePairedMac(const uint8_t *newMac) {
  preferences.begin("espnow", false);
  pairedDeviceCount = preferences.getBytesLength("paired_mac") / 6;
  if (pairedDeviceCount > MAX_PAIRED_DEVICES) pairedDeviceCount = MAX_PAIRED_DEVICES;
  preferences.getBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);

  bool alreadyExists = false;
  for (int i = 0; i < pairedDeviceCount; i++) {
    if (memcmp(pairedMacList[i], newMac, 6) == 0) {
      alreadyExists = true;
      break;
    }
  }

  if (!alreadyExists) {
    if (pairedDeviceCount < MAX_PAIRED_DEVICES) {
      memcpy(pairedMacList[pairedDeviceCount], newMac, 6);
      pairedDeviceCount++;
    } else {
      for (int i = 0; i < MAX_PAIRED_DEVICES - 1; i++) {
        memcpy(pairedMacList[i], pairedMacList[i + 1], 6);
      }
      memcpy(pairedMacList[MAX_PAIRED_DEVICES - 1], newMac, 6);
    }
  }

  preferences.putBytes("paired_mac", pairedMacList, pairedDeviceCount * 6);
  preferences.end();

  updatePeerStatus(newMac, true);
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, newMac, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(newMac)) {
    esp_now_add_peer(&peerInfo);
  }

  Serial.println("✅ Eşleşmiş MAC adresi kaydedildi.");
}

void PrintMacAddress(const uint8_t *mac) {
  Serial.print("MAC Adresi: ");
  for (int i = 0; i < 6; i++) {
    Serial.printf("%02X", mac[i]);
    if (i < 5) Serial.print(":");
  }
  Serial.println();
}

String macToStr(const uint8_t *mac) {
  char buf[18];
  snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void AddPeer(const uint8_t *mac_addr) {
  if (esp_now_is_peer_exist(mac_addr)) {
    Serial.println("ℹ️ Peer zaten kayıtlı.");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_addr, 6);
  peerInfo.channel = 0;         // aynı kanal
  peerInfo.encrypt = false;     // şifreleme kullanmıyoruz

  esp_err_t result = esp_now_add_peer(&peerInfo);
  if (result == ESP_OK) {
    Serial.print("✅ Peer eklendi: ");
    PrintMacAddress(mac_addr);
  } else {
    Serial.printf("❌ Peer eklenemedi! Hata kodu: %d\n", result);
  }
}


void OnDataRecv(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len) {
  const uint8_t *mac_addr = esp_now_info->src_addr;
  addToDiscoveredList(mac_addr);  
  hataTimer_espnow_l = millis();
  char receivedData[len + 1];
  memcpy(receivedData, data, len);
  receivedData[len] = '\0';

  if (strcmp(receivedData, "PAIR_REQUEST") == 0) {
    Serial.println("Eslesme Istegi Alindi!");
    const char *response = "PAIR_RESPONSE";
	  esp_now_send(mac_addr, (uint8_t *)response, strlen(response));
    isPaired = true;
	  SavePairedMac(mac_addr);
	  AddPeer(mac_addr);
    return;
  }

  else if (strcmp(receivedData, "PAIR_DEL") == 0) {
    Serial.println("🧹 Pair silme komutu alındı!");
    String macStr = macToStr(mac_addr);
    RemovePairedMac(macStr.c_str());
  }
  
  else if (strcmp(receivedData, "PAIR_RESPONSE") == 0) {
    Serial.println("Eslesme Yaniti Alindi!");
    isPaired = true;
	  SavePairedMac(mac_addr);
	  PrintMacAddress(mac_addr);
    return;
  }

  else if (strcmp(receivedData, "inddara") == 0) {
    Serial.println("DARA!");
	  digitalWrite(4,LOW);
	  delay(1000);
	  digitalWrite(4,HIGH);
  }
  else {
	  String formattedText = String(receivedData);
	  //Serial.print("Gelen veri: ");
	  Serial.println(receivedData);
    //XML_NET_C = formattedText.c_str();
    //dtostrf(XML_NET_S.toFloat(), (ExtraTaskSettings.TaskDeviceValueBit[0] - ExtraTaskSettings.TaskDeviceValueBas[0]), ExtraTaskSettings.TaskDeviceValueDecimals[0], XML_NET_C);
    XML_NET_S = formattedText;
    //strncpy(XML_NET_C, formattedText.c_str(), sizeof(XML_NET_C));
    strlcpy(XML_NET_C, formattedText.c_str(), sizeof(XML_NET_C)); // Espressif destekli

    webapinettartim = isaret_f * XML_NET_S.toFloat();

    //XML_NET_C[sizeof(XML_NET_C)-1] = '\0';
	  // Gelen veriyi sabit olarak LED ekrana yazdır
	  // display.displayText(receivedData.c_str(), PA_CENTER, 0, 0, PA_NO_EFFECT, PA_NO_EFFECT); // Sabit pozisyon belirle
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  updatePeerStatus(mac_addr, status == ESP_NOW_SEND_SUCCESS);
}

void SendData(String data) {
  if (!isPaired || pairedDeviceCount == 0) return;

  for (int i = 0; i < pairedDeviceCount; i++) {
    esp_err_t result = esp_now_send(pairedMacList[i], (uint8_t *)data.c_str(), data.length());
    updatePeerStatus(pairedMacList[i], result == ESP_OK);
  }
}
#endif

#define TFT_HOR_RES   480
#define TFT_VER_RES   320
//#define TFT_VER_RES   272

#ifdef USE_CUSTOM_H

// make the compiler show a warning to confirm that this file is inlcuded
  # warning "**** Using Settings from Custom.h File ***"
#endif // ifdef USE_CUSTOM_H
#define PLUGIN_SET_GENERIC_ESP32
// Needed due to preprocessor issues.
#ifdef PLUGIN_SET_GENERIC_ESP32
  # ifndef ESP32
    #  define ESP32
  # endif // ifndef ESP32
#endif // ifdef PLUGIN_SET_GENERIC_ESP32

/**************** LVGL AKTİF **************/
#ifdef LVGL_ACTIVE
SemaphoreHandle_t gui_mutex;

/*#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>*/
#include <ArduinoJson.h>
#include "generated/gui_guider.h"
lv_ui guider_ui;

/* Display flushing */
void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map) {
  uint32_t w = lv_area_get_width(area);
  uint32_t h = lv_area_get_height(area);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  
  // px_map'ı RGB565 formatına çevir
  uint16_t *buffer = (uint16_t *)px_map;
  tft.pushPixels(buffer, w * h, true);

  tft.endWrite();
  lv_display_flush_ready(disp);
}

/*unsigned long lastTouchTime = 0;

void adjustScreenBrightness() {
    if (millis() - lastTouchTime > 60000) {  // 60 saniye dokunma yoksa parlaklığı düşür
        tft.setBrightness_lgfx(50);
    } else {
        tft.setBrightness_lgfx(255);
    }
}*/

/*Read the touchpad*/
void my_touchpad_read(lv_indev_t *indev, lv_indev_data_t *data) {
  uint16_t touchX, touchY;
  bool touched = tft.getTouch(&touchX, &touchY);

  if (!touched) {
      data->state = LV_INDEV_STATE_RELEASED;
  } else {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = touchX;
      data->point.y = touchY;
      //lastTouchTime = millis();  // Dokunma algılandı, parlaklığı arttır
  }
  //adjustScreenBrightness();
}

void display_init() {
  //tft.begin();
  tft.init();
  tft.initDMA();
  tft.setRotation(3);
  //tft.setRotation(2);
  tft.setBrightness(255);
  tft.fillScreen(TFT_WHITE);
}

// Minimum RAM limiti, 4KB kritik (ESP32 için güvenli alt sınır)
#define CRITICAL_FREE_MEMORY 4096      // 4KB
#define WARNING_FREE_MEMORY  80000     // 20KB (uyarı eşiği)

void optimizeHeap() {
    const size_t freeMem = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    const size_t minMem  = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);

    #if CONFIG_SPIRAM
    const size_t freePsram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    const size_t minPsram  = heap_caps_get_minimum_free_size(MALLOC_CAP_SPIRAM);
    #else
    const size_t freePsram = 0, minPsram = 0;
    #endif

    // Debug sadece kritik durumda log yap - RAM tasarrufu
    /*if (freeMem < WARNING_FREE_MEMORY) {
        Serial.printf("[MEM] Free: %u, Min: %u | PSRAM Free: %u, Min: %u\n", freeMem, minMem, freePsram, minPsram);
    }*/

    // **KRİTİK RAM OUT DURUMU**: Donma riskinde hemen resetle!
    if (minMem < CRITICAL_FREE_MEMORY || freeMem < CRITICAL_FREE_MEMORY
        #if CONFIG_SPIRAM
        || minPsram < CRITICAL_FREE_MEMORY || freePsram < CRITICAL_FREE_MEMORY
        #endif
        ) {
        //Serial.println("‼️ Kritik RAM! Otomatik reset atılıyor...");
        delay(2000); // logun gitmesine izin ver
        ESP.restart();
    }

    // **UYARI EŞİĞİ**: RAM düşüşteyse optimizasyon yap
    if (freeMem < WARNING_FREE_MEMORY) {
        // Mutex timeout ile al - deadlock önleme
        if (xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            lv_obj_t *scr = lv_scr_act();
            if (scr) {
                uint32_t child_count = lv_obj_get_child_cnt(scr);
                for (uint32_t i = 0; i < child_count; i++) {
                    lv_obj_t *child = lv_obj_get_child(scr, i);
                    if (child && lv_obj_has_flag(child, LV_OBJ_FLAG_HIDDEN)) {
                        lv_obj_del(child); // sadece gizli olanları sil
                        //Serial.println("🧹 Gizli LVGL obje silindi.");
                        break; // Bir seferde bir obje sil - stability
                    }
                }
            }
            xSemaphoreGive(gui_mutex);
        }
        
        // LVGL memory defragmentation
        lv_mem_monitor_t mon;
        lv_mem_monitor(&mon);
        if (mon.free_cnt > 50) { // Çok fazla parça varsa
            if (xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
                lv_obj_clean(lv_scr_act());
                xSemaphoreGive(gui_mutex);
                //Serial.println("🧹 LVGL memory defragmentation yapıldı.");
            }
        }
    }
}

void Task_Memory_Manager(void *pvParameters) {
  while (true) {
      optimizeHeap(); // Hafıza kontrolünü tek bir fonksiyona taşıdık
      
      // Adaptif bekleme - RAM durumuna göre
      const size_t freeMem = heap_caps_get_free_size(MALLOC_CAP_8BIT);
      uint32_t delay_ms = 30000; // Default 30 saniye
      
      if (freeMem < WARNING_FREE_MEMORY) {
          delay_ms = 10000; // RAM düşükse 10 saniye
      } else if (freeMem > 200000) {
          delay_ms = 60000; // RAM bol ise 1 dakika
      }
      
      vTaskDelay(pdMS_TO_TICKS(delay_ms));
  }
}
#endif

/**************** LVGL AKTİF **************/

/****************************************************************************************************************************\
 * Arduino project "ESP Easy" © Copyright www.letscontrolit.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * IDE download    : https://www.arduino.cc/en/Main/Software
 * ESP8266 Package : https://github.com/esp8266/Arduino
 *
 * Source Code     : https://github.com/ESP8266nu/ESPEasy
 * Support         : http://www.letscontrolit.com
 * Discussion      : http://www.letscontrolit.com/forum/
 *
 * Additional information about licensing can be found at : http://www.gnu.org/licenses
 \*************************************************************************************************************************/

// This file incorporates work covered by the following copyright and permission notice:

/****************************************************************************************************************************\
 * Arduino project "Nodo" © Copyright 2010..2015 Paul Tonkes
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
 * You received a copy of the GNU General Public License along with this program in file 'License.txt'.
 *
 * Voor toelichting op de licentievoorwaarden zie    : http://www.gnu.org/licenses
 * Uitgebreide documentatie is te vinden op          : http://www.nodo-domotica.nl
 * Compiler voor deze programmacode te downloaden op : http://arduino.cc
 \*************************************************************************************************************************/

// Must be included after all the defines, since it is using TASKS_MAX
#include "_Plugin_Helper.h"

// Plugin helper needs the defined controller sets, thus include after 'define_plugin_sets.h'
#include "src/Helpers/_CPlugin_Helper.h"


#include "src/ESPEasyCore/ESPEasy_setup.h"
#include "src/ESPEasyCore/ESPEasy_loop.h"


#ifdef PHASE_LOCKED_WAVEFORM
# include <core_esp8266_waveform.h>
#endif // ifdef PHASE_LOCKED_WAVEFORM

#if FEATURE_ADC_VCC
ADC_MODE(ADC_VCC);
#endif // if FEATURE_ADC_VCC



#ifdef CORE_POST_2_5_0

/*********************************************************************************************\
* Pre-init
\*********************************************************************************************/
void preinit();
void preinit() {
  system_phy_set_powerup_option(3);
  // Global WiFi constructors are not called yet
  // (global class instances like WiFi, Serial... are not yet initialized)..
  // No global object methods or C++ exceptions can be called in here!
  // The below is a static class method, which is similar to a function, so it's ok.
  #ifndef CORE_POST_3_0_0
  //ESP8266WiFiClass::preinitWiFiOff();
  #endif

  // Prevent RF calibration on power up.
  // TD-er: disabled on 2021-06-07 as it may cause several issues with some boards.
  // It cannot be made a setting as we can't read anything of our own settings.
  //system_phy_set_powerup_option(RF_NO_CAL);
}

#endif // ifdef CORE_POST_2_5_0

/**************** LVGL AKTİF **************/
#ifdef LVGL_ACTIVE

#define BIT_0 (1 << 0)
#define BIT_1 (1 << 1)

EventGroupHandle_t eventGroup;

void Task_ESPEasy(void *pvParameters) {
  while (true) 
  {  
    xEventGroupSetBits(eventGroup, BIT_1);
    ESPEasy_loop();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
  vTaskDelete(NULL);
}

void Task_LVGL(void *pvParameters) {
  unsigned long lastTick = millis();
  unsigned long lastTimerCheck = millis();
  unsigned long debugPrint = millis();
  const int LVGL_REFRESH_PERIOD_MS = 20;  // 5ms -> 20ms (mutex contention'ı azalt)
  const int TIMER_CHECK_PERIOD_MS = 100; // 20ms -> 100ms (timer check sıklığı azalt)
  const int DEBUG_PRINT_INTERVAL = 5000; // 5 saniyede bir debug
  
  //Serial.println("[LVGL] 🚀 LVGL Task BAŞLATILDI - Core: " + String(xPortGetCoreID()));
  
  while (true) {
    unsigned long now = millis();
    
    // 🔹 Debug print - 5 saniyede bir yaşadığını göster
    if (now - debugPrint >= DEBUG_PRINT_INTERVAL) {
      //Serial.printf("[LVGL] ❤️ LVGL Task ÇALIŞIYOR - Uptime: %lu ms, Core: %d\n", now, xPortGetCoreID());
      debugPrint = now;
    }
    
    // 🔹 Tick güncelleme - her zaman yap
    unsigned long elapsed = now - lastTick;
    if (elapsed > 0) {
      lv_tick_inc(elapsed);
      lastTick = now;
    }
    
    // 🔹 Timer handler - daha az sıklıkla çalıştır (RAM tasarrufu)
    if (now - lastTimerCheck >= TIMER_CHECK_PERIOD_MS) {
      // Mutex'i timeout ile al - donma önleme
      if (xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        lv_timer_handler();
        xSemaphoreGive(gui_mutex);
        lastTimerCheck = now;
      } else {
        // Mutex alınamazsa küçük gecikme ekle
        //Serial.println("[LVGL] ⚠️ Mutex timeout - skipping frame");
        vTaskDelay(pdMS_TO_TICKS(2));
      }
    }
    
    // 🔹 Adaptif gecikme - CPU yükünü azalt
    vTaskDelay(pdMS_TO_TICKS(LVGL_REFRESH_PERIOD_MS));
  }
  vTaskDelete(NULL);
}

#include "src/ESPEasyCore/ESPEasyWifi.h"

#endif
/**************** LVGL AKTİF **************/

#ifdef ESP_NOW_ACTIVE
void ESP_NOW() {

esp_now_register_send_cb(OnDataSent);

LoadPairedMac();

if (!isPaired && !preferences.getBytesLength("paired_mac")) {
    Serial.println("Eslesmis Cihaz Yok. Eslesme Bekleniyor."); 
} else {
    Serial.println("Eslesmis cihaz bulundu:");
    
    // 'i' degiskenini 0 olarak baslat ve kayıtlı cihaz sayısına gore dongu yap
    int maxDeviceCount = sizeof(pairedMacList) / sizeof(pairedMacList[0]);
    for (int i = 0; i < pairedDeviceCount && i < maxDeviceCount; i++) {
        if (pairedMacList[i] != nullptr) { // Gecerli MAC adresi kontrolu
            AddPeer(pairedMacList[i]); // Eslesmis cihazı peer olarak ekle
            Serial.printf("Peer eklendi: Cihaz %d\n", i);
        } else {
            Serial.printf("Gecersiz MAC adresi: Cihaz %d\n", i);
        }
    }
  }
  
    // Gelen veri için callback fonksiyonu
    esp_now_register_recv_cb(OnDataRecv);
  
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;

    // esp_now_add_peer çağrısını değişkene ata
    esp_err_t err = esp_now_add_peer(&peerInfo);

    if (esp_now_add_peer(&peerInfo) == ESP_OK) {
        Serial.println("Eslesme Istegi Basariyla Gonderildi.");
    } else {
        Serial.printf("Eslesme Istegi Gonderilemedi. Hata: %d (0x%X)\n", (int)err, (unsigned)err);
    }
}
#endif


void setup()
{
  ESPEasy_setup();

  /**************** LVGL AKTİF **************/
  #ifdef LVGL_ACTIVE

  gui_mutex = xSemaphoreCreateMutex();
  if (!gui_mutex) { Serial.println("Mutex fail!"); ESP.restart(); }

  // LVGL başlat
  display_init();
  lv_init();

  // Bellek tahsisi (DMA destekli, **MALLOC_CAP_DMA** kullan!)
  //draw_buf_1 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM);
  draw_buf_1 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
  draw_buf_2 = (lv_color_t *)heap_caps_malloc(DRAW_BUF_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);

  if (!draw_buf_1 || !draw_buf_2) {
      Serial.println("❌ LVGL Buffer allocation FAILED. Restarting...");
      delay(1000);
      ESP.restart();
  }

  // Ekran tanımlama
  lv_display_t *disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf_1, draw_buf_2, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
  //lv_display_set_buffers(disp, draw_buf_1, NULL, DRAW_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);

  //lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Dokunmatik girişini tanımla
  lv_indev_t *indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  lv_indev_set_disp(indev, disp); // dokunmatik giriş doğru ekranla eşleştirilsin

  setup_ui(&guider_ui);

  // Custom LVGL initialiaztion - Timer ve callback setup
  //Serial.println("🔧 Custom LVGL başlatılıyor...");
  //custom_init(&guider_ui);
  //Serial.println("✅ Custom LVGL başlatıldı");

  eventGroup = xEventGroupCreate();
  if (eventGroup == nullptr) {
    printf("Event group creation failed\n");
    return; // Bellek yetersizse çıkış yap
  }

  Serial.println("🔧 LVGL Task oluşturuluyor...");
  if (xTaskCreatePinnedToCore(Task_LVGL,    // Pointer to the task entry function.
                          "Task_LVGL",  // A descriptive name for the task.
                          1024 * 32,    // The size of the task stack specified as the number of bytes
                          NULL,         // Pointer that will be used as the parameter for the task being created.
                          2,            // Priority artırıldı 1→2 (USB HID'den yüksek olsun)
                          NULL,         // Used to pass back a handle by which the created task can be referenced.
                          1)           // The core to which the task is pinned to, or tskNO_AFFINITY if the task has no core affinity.
                          !=pdPASS) {
                          printf("❌ Failed to create LVGL Loop task\n");
                          ESP.restart();
  } else {
      Serial.println("✅ LVGL Task başarıyla oluşturuldu - Core 1, Priority 2");
  }

  Serial.println("🔧 ESPEasy Task oluşturuluyor...");
  if (xTaskCreatePinnedToCore(Task_ESPEasy,
                          "Task_ESPEasy",
                          1024 * 8,
                          NULL,
                          3,            // Priority artırıldı 2→3
                          NULL,
                          0)
                          != pdPASS) {
                          printf("❌ Failed to create ESP Easy task\n");
                          ESP.restart();
  } else {
      Serial.println("✅ ESPEasy Task başarıyla oluşturuldu - Core 0, Priority 3");
  }

  if (xTaskCreatePinnedToCore(Task_Memory_Manager,
                          "Task_Memory_Manager",
                          4096,
                          NULL,
                          1,
                          NULL,
                          0) != pdPASS) {
                          printf("Failed to create Memory Manager task\n");
                          ESP.restart();
  }
  
  // ✅ USB HID RFID sistemi başlat
  initUSBHIDRFID();

  //ledcAttachChannel(tft.pins.buzzer.buzzer_pin, 2000, 10, 3);
  //ledcWriteChannel(3, 0);
  #endif
  /**************** LVGL AKTİF **************/
}

void loop() {
  #ifndef LVGL_ACTIVE
    ESPEasy_loop();
  #endif
}
