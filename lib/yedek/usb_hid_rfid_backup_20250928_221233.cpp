#include "usb_hid_rfid.h"
#include "../src/Helpers/SystemVariables.h"
#include "../src/Helpers/ESPEasy_Storage.h"
#include "../src/DataStructs/ESPEasy_EventStruct.h"
#include "../src/Helpers/StringConverter.h"
#include <HTTPClient.h>
#include "../src/Globals/Settings.h"
#include "../src/DataStructs/ControllerSettingsStruct.h"
#include "../src/Helpers/_CPlugin_Helper.h"

// Global variables
static char   line_buf[64];
static size_t line_len = 0;
HidState g_hid;

// ------------------- HID → ASCII -------------------
static int usage_to_char(uint8_t code) {
  if (code >= 0x1E && code <= 0x26) return '1' + (code - 0x1E); // 1..9
  if (code == 0x27) return '0';
  if (code >= 0x59 && code <= 0x61) return '1' + (code - 0x59); // KP 1..9
  if (code == 0x62) return '0';                                  // KP 0
  if (code == 0x28 || code == 0x58) return '\n';                 // Enter
  return -1;
}

// ------------------- HID Interface Callback -------------------
static void iface_cb(hid_host_device_handle_t dev,
                     const hid_host_interface_event_t event,
                     void *arg) {
  if (event != HID_HOST_INTERFACE_EVENT_INPUT_REPORT) return;

  uint8_t rep[64] = {0};
  size_t got = 0;
  if (hid_host_device_get_raw_input_report_data(dev, rep, sizeof(rep), &got) != ESP_OK || got < 8)
    return;

  const uint8_t *keycodes = &rep[2]; // [mods,reserved,k0..k5]
  for (int i = 0; i < 6; i++) {
    int ch = usage_to_char(keycodes[i]);
    if (ch < 0) continue;

    if (ch == '\n') {
      line_buf[line_len] = 0;
      if (line_len) {
        // Non-blocking serial print - LVGL'yi bloke etmesin
        Serial.printf("[HID] ID: %s\n", line_buf);
        
        // Timestamp oluştur (ISO format)
        String timestamp = String(2025) + "-09-28T" + 
                          (millis() / 3600000 % 24 < 10 ? "0" : "") + String(millis() / 3600000 % 24) + ":" +
                          (millis() / 60000 % 60 < 10 ? "0" : "") + String(millis() / 60000 % 60) + ":" +
                          (millis() / 1000 % 60 < 10 ? "0" : "") + String(millis() / 1000 % 60);
        
        // SystemVariables'a RFID Card ID ve timestamp'ini set et (C11 task'ından önce)
        SystemVariables::setCardID(String(line_buf));
        SystemVariables::setCardTimestamp(timestamp);
        Serial.printf("[HID] Card ID ve timestamp SystemVariables'a kaydedildi: %s - %s\n", line_buf, timestamp.c_str());
        
        // SystemVariable'ın doğru kaydedildiğini kontrol et
        String stored_cardid = SystemVariables::getCardID();
        String stored_timestamp = SystemVariables::getCardTimestamp();
        Serial.printf("[HID] SystemVariable kontrol - CardID: %s, Timestamp: %s\n", stored_cardid.c_str(), stored_timestamp.c_str());
        
        // Kısa delay - SystemVariable'ın işlenmesi için
        vTaskDelay(pdMS_TO_TICKS(50));
      
        
        // C11 Controller'a PDKS API'ye otomatik POST gönder (arka planda)
        sendCardDataToPDKS(String(line_buf));
        
        // Burada RFID card ID'sini işleyebilirsiniz
        // Örnek: ESPEasy Rules sistemi tetikleme
        // String rfid_event = "RFID," + String(line_buf);
        
        line_len = 0;
      }
    } else if (line_len + 1 < sizeof(line_buf)) {
      line_buf[line_len++] = (char)ch;
      line_buf[line_len] = 0;
    }
  }
  
  // CPU yield - diğer task'lara fırsat ver
  vTaskDelay(pdMS_TO_TICKS(1));
}

// ------------------- HID Device Callback -------------------
static void dev_cb(hid_host_device_handle_t dev,
                   const hid_host_driver_event_t event,
                   void *arg) {
  hid_host_dev_params_t prm;
  hid_host_device_get_params(dev, &prm);

  const hid_host_device_config_t cfg = {
    .callback     = iface_cb,
    .callback_arg = nullptr
  };

  switch (event) {
    case HID_HOST_DRIVER_EVENT_CONNECTED: {
      Serial.printf("[HID] CONNECTED proto=%d subcls=%d\n", prm.proto, prm.sub_class);

      if (g_hid.started) { 
        Serial.println("[HID] Already started, ignore duplicate"); 
        return; 
      }
      if (g_hid.opened && g_hid.dev != dev) {
        g_hid = HidState{};
      }

      esp_err_t err;
      if (!g_hid.opened) {
        err = hid_host_device_open(dev, &cfg);
        if (err != ESP_OK) {
          Serial.printf("[HID] Open error: %d (ok if INVALID_STATE)\n", err);
        } else { 
          g_hid.dev = dev; 
          g_hid.opened = true; 
        }
      }

      // Boot protocol setup for keyboard
      if (prm.sub_class == HID_SUBCLASS_BOOT_INTERFACE && prm.proto == HID_PROTOCOL_KEYBOARD) {
        err = hid_class_request_set_protocol(dev, HID_REPORT_PROTOCOL_BOOT);
        if (err != ESP_OK) {
          Serial.printf("[HID] SET_PROTOCOL(BOOT) failed (%d), staying in REPORT mode\n", err);
        } else {
          err = hid_class_request_set_idle(dev, 0, 0);
          if (err != ESP_OK) {
            Serial.printf("[HID] SET_IDLE failed (%d), continuing\n", err);
          }
        }
      }

      if (!g_hid.started) {
        err = hid_host_device_start(dev);
        if (err != ESP_OK) {
          Serial.printf("[HID] Device start error: %d\n", err);
        } else {
          g_hid.started = true;
          Serial.println("[HID] ✅ Device started successfully - Ready for RFID input!");
        }
      }
      break;
    }

    #ifdef HID_HOST_DRIVER_EVENT_DISCONNECTED
    case HID_HOST_DRIVER_EVENT_DISCONNECTED:
    #elif defined(HID_HOST_DRIVER_EVENT_DEV_DISCONNECTED)
    case HID_HOST_DRIVER_EVENT_DEV_DISCONNECTED:
    #endif
      Serial.println("[HID] 🔌 DISCONNECTED - Resetting state");
      g_hid = HidState{};
      line_len = 0;
      break;

    default: 
      break;
  }
}

// ------------------- USB Host Task -------------------
static void usb_host_task(void *me) {
  Serial.println("[USB_HOST] 🚀 Starting USB Host task on Core 0...");
  
  usb_host_config_t cfg = { 
    .skip_phy_setup = false, 
    .intr_flags = ESP_INTR_FLAG_LEVEL1 
  };
  
  esp_err_t ret = usb_host_install(&cfg);
  if (ret != ESP_OK) {
    Serial.printf("[USB_HOST] ❌ USB Host install failed: %d\n", ret);
    vTaskDelete(NULL);
    return;
  }
  
  Serial.println("[USB_HOST] ✅ USB Host installed successfully on Core 0");
  xTaskNotifyGive((TaskHandle_t)me);

  while (true) {
    uint32_t flags = 0;
    usb_host_lib_handle_events(portMAX_DELAY, &flags);
    if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      usb_host_device_free_all();
    }
    
    // CPU yield - diğer task'lara fırsat ver (özellikle LVGL)
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ------------------- Public Functions -------------------
void initUSBHIDRFID() {
  Serial.println("[USB_HID_RFID] Initializing USB HID RFID system...");
  
  // USB host task oluştur - Core 0'a taşı (LVGL ile çakışmaması için)
  xTaskCreatePinnedToCore(usb_host_task, "usb_host", 4096,
                          xTaskGetCurrentTaskHandle(), 8, nullptr, 0); // Priority düşürüldü 15→8
  ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

  // HID host driver başlat - Core 0'a taşı ve priority düşür
  const hid_host_driver_config_t hid_cfg = {
    .create_background_task = true,
    .task_priority = 5,        // Priority düşürüldü 10→5
    .stack_size    = 4096,
    .core_id       = 0,        // Core 1→0 (LVGL ile çakışmaması için)
    .callback      = dev_cb,
    .callback_arg  = nullptr
  };
  
  esp_err_t ret = hid_host_install(&hid_cfg);
  if (ret != ESP_OK) {
    Serial.printf("[USB_HID_RFID] ❌ HID host install failed: %d\n", ret);
    return;
  }
  
  Serial.println("[USB_HID_RFID] ✅ USB HID RFID system initialized on Core 0");
}

void cleanupUSBHIDRFID() {
  Serial.println("[USB_HID_RFID] Cleaning up USB HID RFID system...");
  
  // HID state'i sıfırla
  g_hid = HidState{};
  line_len = 0;
  
  // HID host uninstall
  hid_host_uninstall();
  
  // USB host uninstall  
  usb_host_uninstall();
  
  Serial.println("[USB_HID_RFID] ✅ Cleanup completed");
}

// PDKS API'ye kart verisi gönder (C11 Controller kullanarak)
void sendCardDataToPDKS(const String& cardId) {
    Serial.printf("[PDKS] C11 Controller ile kart verisi gönderiliyor: %s\n", cardId.c_str());
    
    // WiFi bağlantısı kontrol et
    String wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
    if (wifi_status.toInt() < 2) {
        Serial.println("[PDKS] WiFi bağlı değil - C11 gönderimi yapılamıyor");
        return;
    }
    
    // C11 Controller aktif mi kontrol et
    if (!Settings.ControllerEnabled[0]) {
        Serial.println("[PDKS] Controller[0] aktif değil - C11 gönderimi yapılamıyor");
        return;
    }
    
    // String'i heap'te kopyala - task içinde free edilecek
    char* cardId_copy = (char*)malloc(cardId.length() + 1);
    if (cardId_copy == nullptr) {
        Serial.println("[PDKS] Heap allocation başarısız - C11 gönderimi yapılamıyor");
        return;
    }
    strcpy(cardId_copy, cardId.c_str());
    
    // C11 Controller'a POST gönder (background task olarak)
    xTaskCreatePinnedToCore(
        sendCardDataWithC11Task,    // Task fonksiyonu  
        "PDKS_C11_Send",           // Task adı
        4096,                      // Stack size
        (void*)cardId_copy,        // Parametre (kopyalanan card ID)
        1,                         // Priority
        nullptr,                   // Task handle
        0                          // Core 0 (USB HID ile aynı core)
    );
}

// Background task - C11 Controller ile PDKS API'ye gönder
void sendCardDataWithC11Task(void* parameter) {
    char* cardId = (char*)parameter;
    
    Serial.printf("[PDKS_C11_TASK] C11 Controller ile gönderim başlatılıyor - Card ID: %s\n", cardId);
    
    // SystemVariable'ları güncelle - C11 bunları kullanacak
    SystemVariables::setCardID(String(cardId));
    
    // SystemVariable'ın doğru kaydedildiğini tekrar kontrol et
    vTaskDelay(pdMS_TO_TICKS(100)); // Biraz daha bekle
    String stored_cardid = SystemVariables::getCardID();
    Serial.printf("[PDKS_C11_TASK] SystemVariable kontrol - Stored CardID: %s\n", stored_cardid.c_str());
    
    // Client IP'sini al  
    String sys_ip = SystemVariables::getSystemVariable(SystemVariables::IP);
    
    Serial.printf("[PDKS_C11_TASK] SystemVariables güncellendi - CardID: %s, IP: %s\n", 
                  cardId, sys_ip.c_str());
    
    // ESPEasy C11 Controller sistemi kullan
    // Event struct oluştur
    struct EventStruct TempEvent;
    TempEvent.ControllerIndex = 0; // Controller 0 kullan
    TempEvent.idx = 0;
    TempEvent.TaskIndex = 255; // Dummy task index
    TempEvent.sensorType = Sensor_VType::SENSOR_TYPE_STRING;
    
    // C11 Controller'ın kendi queue sistemini kullan
    // Bu sayede C11'in ayarlarındaki HttpMethod, HttpUri, HttpBody kullanılacak
    // HttpBody'de şu template kullanılmalı: {"card_id":"%cardid%","client_ip":"%ip%"}
    
    String dummy_string = ""; // CPlugin_011 String& referans bekliyor
    if (!CPlugin_011(CPlugin::Function::CPLUGIN_PROTOCOL_SEND, &TempEvent, dummy_string)) {
        Serial.println("[PDKS_C11_TASK] ❌ C11 Controller gönderimi başarısız");
        
        // Debug için SystemVariable'ları tekrar kontrol et
        String debug_cardid = SystemVariables::getSystemVariable(SystemVariables::CARDID);
        Serial.printf("[PDKS_C11_TASK] Debug - CARDID SystemVariable: '%s'\n", debug_cardid.c_str());
        
        // LVGL ekranını güncelle - başarısız gönderim
        getUserNameAndUpdateScreen(String(cardId));
    } else {
        Serial.println("[PDKS_C11_TASK] ✅ C11 Controller gönderimi başarılı");
        
        // LVGL ekranını güncelle - başarılı gönderim  
        getUserNameAndUpdateScreen(String(cardId));
    }
    
    Serial.printf("[PDKS_C11_TASK] Task tamamlandı - Card ID: %s\n", cardId);
    
    // Heap memory'yi free et
    free(cardId);
    
    // Task'ı sonlandır  
    vTaskDelete(nullptr);
}

// PDKS API'den user name al ve LVGL ekranını güncelle
void getUserNameAndUpdateScreen(const String& cardId) {
    Serial.printf("[LVGL_UPDATE] Card ID için user name alınıyor: %s\n", cardId.c_str());
    
    // WiFi bağlantısı kontrol et
    String wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
    if (wifi_status.toInt() < 2) {
        Serial.println("[LVGL_UPDATE] WiFi bağlı değil - Offline mode");
        // External function call - custom.cpp'den
        extern void updateLVGLScreen(const String& cardId, const String& userName, bool success);
        updateLVGLScreen(cardId, "WiFi Bağlı Değil", false);
        return;
    }
    
    HTTPClient http;
    http.setTimeout(3000); // 3 saniye timeout
    
    // PDKS API card endpoint URL oluştur - Controller 0 ayarlarından
    String card_url = "";
    if (Settings.ControllerEnabled[0]) {
        // Controller 0 ayarlarını yükle  
        ControllerSettingsStruct controller_settings;
        LoadControllerSettings(0, controller_settings);
        
        IPAddress controller_ip = controller_settings.getIP();
        uint16_t controller_port = controller_settings.Port;
        
        card_url = "http://" + controller_ip.toString() + ":" + String(controller_port) + "/card/" + cardId;
        Serial.printf("[LVGL_UPDATE] Controller[0] URL: %s\n", card_url.c_str());
    } else {
        // Fallback: Custom.h'den ya da varsayılan
        #ifdef PDKS_SERVER_URL
            card_url = String(PDKS_SERVER_URL) + "/card/" + cardId;
        #else
            card_url = "http://192.168.1.100:5000/card/" + cardId; // Varsayılan
        #endif
        Serial.println("[LVGL_UPDATE] Controller[0] deaktif - Varsayılan URL kullanılıyor");
    }
    
    http.begin(card_url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.GET();
    String userName = "Bilinmeyen Kart";
    bool success = false;
    
    if (httpResponseCode == 200) {
        String response = http.getString();
        Serial.printf("[LVGL_UPDATE] ✅ API Response: %s\n", response.c_str());
        
        // JSON response'dan user_name'i parse et (basit string arama)
        int user_name_start = response.indexOf("\"user_name\":\"");
        if (user_name_start != -1) {
            user_name_start += 13; // "user_name":" uzunluğu
            int user_name_end = response.indexOf("\"", user_name_start);
            if (user_name_end != -1) {
                userName = response.substring(user_name_start, user_name_end);
                success = true;
                Serial.printf("[LVGL_UPDATE] ✅ User name bulundu: %s\n", userName.c_str());
            }
        }
    } else if (httpResponseCode > 0) {
        Serial.printf("[LVGL_UPDATE] ❌ HTTP Error: %d\n", httpResponseCode);
        userName = "HTTP Hata: " + String(httpResponseCode);
    } else {
        Serial.printf("[LVGL_UPDATE] ❌ Connection Error: %s\n", http.errorToString(httpResponseCode).c_str());
        userName = "Bağlantı Hatası";
    }
    
    http.end();
    
    // LVGL ekranını güncelle - custom.cpp'deki external function
    extern void updateLVGLScreen(const String& cardId, const String& userName, bool success);
    updateLVGLScreen(cardId, userName, success);
    
    Serial.printf("[LVGL_UPDATE] ✅ LVGL ekran güncelleme tamamlandı: %s -> %s\n", cardId.c_str(), userName.c_str());
}