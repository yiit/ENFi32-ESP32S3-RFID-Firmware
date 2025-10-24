#include "usb_hid_rfid.h"
#include "../src/Helpers/SystemVariables.h"
#include "../src/Helpers/ESPEasy_Storage.h"
#include "../src/DataStructs/ESPEasy_EventStruct.h"
#include "../src/Helpers/StringConverter.h"
#include "../src/ESPEasyCore/ESPEasy_Log.h"
#include <HTTPClient.h>
#include <WiFi.h>
#include "../src/Globals/Settings.h"
#include "../src/DataStructs/ControllerSettingsStruct.h"
#include "../src/Helpers/_CPlugin_Helper.h"
#include <LittleFS.h>

// Forward declaration for CPlugin_011
extern bool CPlugin_011(CPlugin::Function function, struct EventStruct* event, String& string);
#include "../src/custom/custom.h"

// Forward declarations for local functions
void handleBackgroundAPIResponse();

// Global variables
static char   line_buf[64];
static size_t line_len = 0;
HidState g_hid;

// Global response handling for background tasks
String g_lastAPIResponse = "";
bool g_apiResponseReady = false;
String g_currentCardId = "";
String g_lastCapturedResponse = ""; // C011'den yakalanan response

// Response capture callback - C011 Controller tarafından çağrılır
void captureAPIResponse(const String& response) {
    g_lastCapturedResponse = response;
    addLog(LOG_LEVEL_ERROR, String(F("[RESPONSE_CAPTURE] 🎯 Response yakalandı: ")) + response);
}

// API Connectivity tracking
static bool g_apiConnected = false;

// Global API connection status için getter fonksiyonu
bool getAPIConnectionStatus() {
    // UI için daha responsive olmak için her 10 saniyede bir fresh kontrol
    static unsigned long lastUICheck = 0;
    unsigned long now = millis();
    
    // WiFi bağlı değilse direkt false dön, network kontrolü yapma
    if (WiFi.status() != WL_CONNECTED) {
        g_apiConnected = false;
        return false;
    }
    
    if (now - lastUICheck > 10000) { // 10 saniye (daha az sıklıkla)
        lastUICheck = now;
        g_apiConnected = checkAPIConnectivity();
    }
    
    return g_apiConnected;
}
static unsigned long g_lastConnectivityCheck = 0;
static const unsigned long CONNECTIVITY_CHECK_INTERVAL = 60000; // 60 saniye (WiFi'ye daha az yük)

// API connectivity check with TCP connection test (no HTTP requests)
bool checkAPIConnectivity() {
    // Startup debug - Controller ayarlarını kontrol et
    addLog(LOG_LEVEL_INFO, String(F("[API_CHECK] Controller[0] Enable: ")) + String(Settings.ControllerEnabled[0] ? "✅" : "❌") + 
           String(F(", Protocol[0]: ")) + String(Settings.Protocol[0]));
    
    // Controller[0] aktif değilse false dön
    if (Settings.Protocol[0] == 0) {
        return false;
    }
    
    // WiFi bağlı değilse false dön
    if (WiFi.status() != WL_CONNECTED) {
        return false;
    }
    
    // Controller ayarlarından gerçek IP/Port al
    if (!Settings.ControllerEnabled[0]) {
        return false;
    }
    
    ControllerSettingsStruct controller_settings;
    LoadControllerSettings(0, controller_settings);
    
    // IP ve Port bilgilerini al
    IPAddress serverIP = controller_settings.getIP();
    uint16_t serverPort = controller_settings.Port;
    
    // IP adresi geçerli mi kontrol et
    if (serverIP == IPAddress(0, 0, 0, 0)) {
        return false;
    }
    
    // Port geçerli mi kontrol et
    if (serverPort == 0) {
        return false;
    }
    
    // Basit TCP port kontrolü - HTTP request göndermeden
    WiFiClient client;
    client.setTimeout(2000); // 2 saniye timeout (daha hızlı)
    bool connected = client.connect(serverIP, serverPort);
    
    if (connected) {
        client.stop();
        if (!g_apiConnected) {
            addLog(LOG_LEVEL_INFO, String(F("[API_HEALTH] ✅ TCP portu açık: ")) + serverIP.toString() + ":" + String(serverPort));
        }
    } else {
        if (g_apiConnected) {
            addLog(LOG_LEVEL_ERROR, String(F("[API_HEALTH] ❌ TCP portu kapalı: ")) + serverIP.toString() + ":" + String(serverPort));
        }
        // PC'den erişebiliyorsanız ama ESP32'den erişemiyorsanız:
        addLog(LOG_LEVEL_INFO, String(F("[DEBUG_C11] TCP FAIL - IP/Port: ")) + serverIP.toString() + ":" + String(serverPort) + String(F(", Local IP: ")) + WiFi.localIP().toString());
    }
    
    return connected;
}

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
  esp_err_t ret = hid_host_device_get_raw_input_report_data(dev, rep, sizeof(rep), &got);
  
  if (ret != ESP_OK || got < 8) {
    addLog(LOG_LEVEL_INFO, String(F("[HID] USB read error: ")) + String(ret) + String(F(", got: ")) + String(got));
    return;
  }

  const uint8_t *keycodes = &rep[2]; // [mods,reserved,k0..k5]
  for (int i = 0; i < 6; i++) {
    int ch = usage_to_char(keycodes[i]);
    if (ch < 0) continue;

    if (ch == '\n') {
      if (line_len >= sizeof(line_buf) - 1) {
        addLog(LOG_LEVEL_ERROR, F("[HID] Buffer overflow, resetting"));
        line_len = 0;
        return;
      }
      
      line_buf[line_len] = 0;
      if (line_len > 0) {
        // RFID kart ID'si okundu - ESPEasy log'a ekle
        addLog(LOG_LEVEL_INFO, String(F("[HID] RFID Kart ID: ")) + String(line_buf));
        
        // Basit timestamp - millis() kullan
        String timestamp = String(millis());
        
        // SystemVariables'a RFID Card ID ve timestamp'ini set et (C11 task'ından önce)
        SystemVariables::setCardID(String(line_buf));
        SystemVariables::setCardTimestamp(timestamp);
        addLog(LOG_LEVEL_INFO, String(F("[HID] SystemVariables güncellendi - ID: ")) + String(line_buf) + String(F(", Time: ")) + timestamp);
        
        // Kısa delay - SystemVariable'ın işlenmesi için
        vTaskDelay(pdMS_TO_TICKS(50));
      
        
        // C11 Controller'a PDKS API'ye otomatik POST gönder (arka planda)
        addLog(LOG_LEVEL_INFO, String(F("[HID] 📤 API gönderimi başlatılıyor: ")) + String(line_buf));
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
      if (g_hid.started) { 
        addLog(LOG_LEVEL_INFO, F("[HID] Device already connected - ignoring"));
        return; 
      }
      
      addLog(LOG_LEVEL_INFO, String(F("[HID] 🔌 USB cihazı bağlandı - Proto: ")) + String(prm.proto) + String(F(", SubClass: ")) + String(prm.sub_class));
      
      if (g_hid.opened && g_hid.dev != dev) {
        addLog(LOG_LEVEL_INFO, F("[HID] 🔄 Önceki device cleanup yapılıyor"));
        if (g_hid.dev) {
          hid_host_device_close(g_hid.dev);
        }
        g_hid = HidState{};
      }

      esp_err_t err;
      if (!g_hid.opened) {
        err = hid_host_device_open(dev, &cfg);
        if (err != ESP_OK) {
          //Serial.printf("[HID] Open error: %d (ok if INVALID_STATE)\n", err);
        } else { 
          g_hid.dev = dev; 
          g_hid.opened = true; 
        }
      }

      // Boot protocol setup for keyboard - opsiyonel, bazı cihazlarda çalışmayabilir
      if (prm.sub_class == HID_SUBCLASS_BOOT_INTERFACE && prm.proto == HID_PROTOCOL_KEYBOARD) {
        err = hid_class_request_set_protocol(dev, HID_REPORT_PROTOCOL_BOOT);
        if (err == ESP_OK) {
          // Boot protocol başarılı, SET_IDLE dene
          err = hid_class_request_set_idle(dev, 0, 0);
          if (err != ESP_OK) {
            addLog(LOG_LEVEL_INFO, F("[HID] SET_IDLE failed, continuing in boot mode"));
          } else {
            addLog(LOG_LEVEL_INFO, F("[HID] Boot protocol configured successfully"));
          }
        } else {
          // Boot protocol başarısız, report mode'da devam et (normal durum)
          addLog(LOG_LEVEL_INFO, F("[HID] Using report protocol (boot protocol not supported)"));
        }
      }

      if (!g_hid.started) {
        err = hid_host_device_start(dev);
        if (err != ESP_OK) {
          addLog(LOG_LEVEL_ERROR, String(F("[HID] ❌ Device start hatası: ")) + String(err));
        } else {
          g_hid.started = true;
          addLog(LOG_LEVEL_INFO, F("[HID] ✅ RFID cihazı başarıyla başlatıldı - Kart okumaya hazır!"));
        }
      }
      break;
    }

    #ifdef HID_HOST_DRIVER_EVENT_DISCONNECTED
    case HID_HOST_DRIVER_EVENT_DISCONNECTED:
    #elif defined(HID_HOST_DRIVER_EVENT_DEV_DISCONNECTED)
    case HID_HOST_DRIVER_EVENT_DEV_DISCONNECTED:
    #endif
      addLog(LOG_LEVEL_INFO, F("[HID] 🔌 USB cihazı bağlantısı kesildi - Durum sıfırlanıyor"));
      g_hid = HidState{};
      line_len = 0;
      break;

    default: 
      break;
  }
}

// ------------------- USB Host Task -------------------
static void usb_host_task(void *me) {
  addLog(LOG_LEVEL_INFO, F("[USB_HOST] 🚀 USB Host task başlatılıyor..."));
  
  usb_host_config_t cfg = { 
    .skip_phy_setup = false, 
    .intr_flags = ESP_INTR_FLAG_LEVEL1 
  };
  
  esp_err_t ret = usb_host_install(&cfg);
  if (ret != ESP_OK) {
    addLog(LOG_LEVEL_ERROR, String(F("[USB_HOST] ❌ USB Host install başarısız: ")) + String(ret));
    vTaskDelete(NULL);
    return;
  }
  
  addLog(LOG_LEVEL_INFO, F("[USB_HOST] USB Host basariyla kuruldu"));
  xTaskNotifyGive((TaskHandle_t)me);

  while (true) {
    uint32_t flags = 0;
    
    esp_err_t ret = usb_host_lib_handle_events(pdMS_TO_TICKS(1000), &flags);
    
    if (ret != ESP_OK && ret != ESP_ERR_TIMEOUT) {
      addLog(LOG_LEVEL_ERROR, String(F("[USB_HOST] Event handling hatasi: ")) + String(ret));
    }
    
    if (flags & USB_HOST_LIB_EVENT_FLAGS_NO_CLIENTS) {
      usb_host_device_free_all();
      addLog(LOG_LEVEL_INFO, F("[USB_HOST] No clients - devices freed"));
    }
    
    // CPU yield
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}

// ------------------- Public Functions -------------------

// Delayed USB initialization task
void delayedUSBInitTask(void* parameter) {
  uint32_t delay_ms = (uint32_t)parameter;
  
  addLog(LOG_LEVEL_INFO, String(F("[USB_DELAYED] USB HID init delayed ")) + String(delay_ms/1000) + String(F(" seconds...")));
  
  // Belirtilen sure bekle
  vTaskDelay(pdMS_TO_TICKS(delay_ms));
  
  // WiFi baglanti durumunu bekle (ekstra güvenlik)
  int wifiWaitCount = 0;
  while (WiFi.status() != WL_CONNECTED && wifiWaitCount < 10) {
    addLog(LOG_LEVEL_INFO, F("[USB_DELAYED] Waiting for WiFi connection..."));
    vTaskDelay(pdMS_TO_TICKS(2000)); // 2 saniye daha bekle
    wifiWaitCount++;
  }
  
  // USB HID RFID sistemini baslat
  initUSBHIDRFIDNow();
  
  // Task kendini sil
  vTaskDelete(NULL);
}

// Gerçek USB initialization (eskiden initUSBHIDRFID idi)
void initUSBHIDRFIDNow() {
  uint32_t startTime = millis();
  addLog(LOG_LEVEL_INFO, F("[USB_HID_RFID] Initializing USB HID RFID system..."));
  
  // USB host task - basit konfigürasyon
  BaseType_t taskResult = xTaskCreatePinnedToCore(
    usb_host_task, 
    "usb_host", 
    4096,
    xTaskGetCurrentTaskHandle(), 
    5,
    nullptr, 
    1
  );
  
  if (taskResult != pdPASS) {
    addLog(LOG_LEVEL_ERROR, F("[USB_HID_RFID] USB Host task creation failed"));
    return;
  }
  
  ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(3000));

  // HID host driver - basit konfigürasyon
  const hid_host_driver_config_t hid_cfg = {
    .create_background_task = true,
    .task_priority = 5,
    .stack_size    = 4096,
    .core_id       = 0,
    .callback      = dev_cb,
    .callback_arg  = nullptr
  };
  
  esp_err_t ret = hid_host_install(&hid_cfg);
  if (ret != ESP_OK) {
    addLog(LOG_LEVEL_ERROR, String(F("[USB_HID_RFID] HID host install failed: ")) + String(ret));
    return;
  }
  
  addLog(LOG_LEVEL_INFO, String(F("[USB_HID_RFID] USB HID RFID system initialized successfully in ")) + String(millis() - startTime) + String(F("ms")));
}

// Public wrapper - USB HID başlatmayı erteler
void initUSBHIDRFID() {
  initUSBHIDRFIDDelayed(10000); // 10 saniye bekle - WiFi stabilize olsun
}

// Delayed başlatma - özelleştirilebilir delay
void initUSBHIDRFIDDelayed(uint32_t delay_ms) {
  addLog(LOG_LEVEL_INFO, String(F("[USB_DELAYED] USB HID baslatma ")) + String(delay_ms/1000) + String(F(" saniye sonra yapilacak")));
  
  BaseType_t result = xTaskCreate(
    delayedUSBInitTask,
    "usb_delayed_init",
    2048,
    (void*)delay_ms,
    3, // Düşük priority - sistem yükünü azalt
    NULL
  );
  
  if (result != pdPASS) {
    addLog(LOG_LEVEL_ERROR, F("[USB_DELAYED] Delayed init task failed"));
    // Fallback: Hemen baslat
    initUSBHIDRFIDNow();
  }
}

void cleanupUSBHIDRFID() {
  addLog(LOG_LEVEL_INFO, F("[USB_HID_RFID] Cleaning up USB HID RFID system..."));
  
  // HID state reset
  g_hid = HidState{};
  line_len = 0;
  
  // HID host uninstall
  hid_host_uninstall();
  
  // USB host uninstall  
  usb_host_uninstall();
  
  addLog(LOG_LEVEL_INFO, F("[USB_HID_RFID] Cleanup completed"));
}

// PDKS API'ye kart verisi gönder (C11 Controller kullanarak)
void sendCardDataToPDKS(const String& cardId) {
    addLog(LOG_LEVEL_INFO, String(F("[PDKS] 📋 Kart okundu: ")) + cardId);
    
    // WiFi bağlantısı kontrol et - WiFi.status() kullan
    if (WiFi.status() != WL_CONNECTED) {
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS] ❌ WiFi bağlı değil - Local storage'a kaydet")));
        saveRFIDDataToLocal(cardId);
        return;
    }
    
    // API connectivity kontrolü - hem cache'i hem de anlık kontrol
    bool isConnected = checkAPIConnectivity(); // Her zaman fresh kontrol
    if (isConnected) {
        g_apiConnected = true; // Cache'i güncelle
        addLog(LOG_LEVEL_INFO, F("[PDKS] ✅ API bağlantısı başarılı - Online mode"));
    } else {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] ❌ API bağlantısı başarısız - Offline mode"));
    }
    
    if (!isConnected) {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] ⚠️ Server erişilemez - Local kaydet"));
        saveRFIDDataToLocal(cardId);
        return;
    }
    
    // C11 Controller aktif mi kontrol et
    if (!Settings.ControllerEnabled[0]) {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] ❌ C11 Controller aktif değil!"));
        return;
    }
    
    addLog(LOG_LEVEL_INFO, F("[PDKS] 🚀 C11 Controller ile API'ye gönderiliyor..."));
    
    // String'i heap'te kopyala - task içinde free edilecek
    size_t cardIdLen = cardId.length();
    if (cardIdLen == 0 || cardIdLen > 32) { // Reasonable limit
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS] Invalid card ID length: ")) + String(cardIdLen));
        return;
    }
    
    char* cardId_copy = (char*)malloc(cardIdLen + 1);
    if (cardId_copy == nullptr) {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] Heap allocation başarısız - C11 gönderimi yapılamıyor"));
        // Heap durumunu logla
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS] Free heap: ")) + String(ESP.getFreeHeap()) + String(F(" bytes")));
        return;
    }
    strcpy(cardId_copy, cardId.c_str());
    
    addLog(LOG_LEVEL_ERROR, String(F("[PDKS] 🔄 Background task oluşturuluyor - Free heap: ")) + String(ESP.getFreeHeap()));
    
    // C11 Controller'a POST gönder (background task olarak)
    BaseType_t taskResult = xTaskCreatePinnedToCore(
        sendCardDataWithC11Task,    // Task fonksiyonu  
        "PDKS_C11_Send",           // Task adı
        4096,                      // Stack size artırıldı 4096→5120
        (void*)cardId_copy,        // Parametre (kopyalanan card ID)
        1,                         // Priority (low - blocking olmasın)
        nullptr,                   // Task handle
        0                          // Core 0 (USB HID ile aynı core)
    );
    
    if (taskResult != pdPASS) {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] ❌ Background task oluşturulamadı"));
        free(cardId_copy); // Memory leak önle
        return;
    } else {
        addLog(LOG_LEVEL_ERROR, F("[PDKS] ✅ Background task başarıyla oluşturuldu"));
    }
}

// Background task - C11 Controller ile PDKS API'ye gönder
void sendCardDataWithC11Task(void* parameter) {
    char* cardId = (char*)parameter;
    
    // Task başlangıcında hemen log at
    addLog(LOG_LEVEL_ERROR, String(F("[PDKS_C11_TASK] *** TASK BAŞLADI *** - Card ID: ")) + String(cardId));
    Serial.printf("[PDKS_C11_TASK] *** TASK BAŞLADI *** - Card ID: %s\n", cardId);
    
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_C11_TASK] 🔄 Background task başlatıldı - Card ID: ")) + String(cardId));
    
    // SystemVariable'ları güncelle - C11 bunları kullanacak
    SystemVariables::setCardID(String(cardId));
    
    // Kısa delay
    vTaskDelay(pdMS_TO_TICKS(100));
    
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_C11_TASK] 📋 SystemVariables hazırlandı - CardID: ")) + String(cardId));
    
    // ESPEasy C11 Controller sistemi kullan
    // Event struct oluştur
    struct EventStruct TempEvent;
    TempEvent.ControllerIndex = 0; // Controller 0 kullan
    TempEvent.idx = 0;
    TempEvent.TaskIndex = 255; // Dummy task index
    TempEvent.sensorType = Sensor_VType::SENSOR_TYPE_STRING;
    
    // Gönderim öncesi detaylı log
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📤 Kart gönderiliyor: ")) + String(cardId));
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 🌐 Controller[0] kullanılıyor")));
    
    // Controller ayarlarını kontrol et
    if (Settings.Protocol[0] != 0) {
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] Controller Protocol: ")) + String(Settings.Protocol[0]));
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] Controller aktif, gönderim başlatılıyor...")));
    } else {
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS_API] ❌ Controller[0] aktif değil! Protocol: ")) + String(Settings.Protocol[0]));
    }
    
    String dummy_string = ""; // CPlugin_011 String& referans bekliyor
    
    // HTTP isteği yapılmadan önce log
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 🔄 C11 Controller çağrılıyor...")));
    
    // Controller ayarlarını detaylı logla
    if (Settings.ControllerEnabled[0]) {
        ControllerSettingsStruct controller_settings;
        LoadControllerSettings(0, controller_settings);
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📋 Controller IP: ")) + controller_settings.getIP().toString());
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📋 Controller Port: ")) + String(controller_settings.Port));
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📋 Controller Protocol: ")) + String(Settings.Protocol[0]));
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📋 POST Request hazırlanıyor...")));
    }
    
    if (!CPlugin_011(CPlugin::Function::CPLUGIN_PROTOCOL_SEND, &TempEvent, dummy_string)) {
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS_API] ❌ API gönderimi BAŞARISIZ: ")) + String(cardId));
        addLog(LOG_LEVEL_ERROR, F("[PDKS_API] 🔍 Controller ayarlarını kontrol edin"));
        addLog(LOG_LEVEL_ERROR, String(F("[PDKS_API] 🔍 Dummy string response: ")) + dummy_string);
        
        // Global response ayarla - BAŞARISIZ
        g_lastAPIResponse = "";
        g_apiResponseReady = true;
        g_currentCardId = String(cardId);
        
        // LVGL ekranını güncelle - başarısız gönderim
        getUserNameAndUpdateScreen(String(cardId), dummy_string, false);
    } else {
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] ✅ API gönderimi BAŞARILI: ")) + String(cardId));
        addLog(LOG_LEVEL_INFO, F("[PDKS_API] 📥 Sunucu cevabı: HTTP isteği tamamlandı"));
        addLog(LOG_LEVEL_INFO, String(F("[PDKS_API] 📥 Response data: ")) + dummy_string);
        
        // Kısa delay - response'un gelmesini bekle
        vTaskDelay(pdMS_TO_TICKS(100));
        
        // Response C011'den captureAPIResponse() ile gelmiş olmalı
        String actualResponse = g_lastCapturedResponse;
        addLog(LOG_LEVEL_ERROR, String(F("[REAL_RESPONSE] 📥 Captured Response: ")) + actualResponse);
        
        // Response kullanıldıktan sonra temizle
        g_lastCapturedResponse = "";
        
        // Global response ayarla - BAŞARILI ve gerçek response ile  
        g_lastAPIResponse = actualResponse;
        g_apiResponseReady = true;
        g_currentCardId = String(cardId);
        
        // LVGL ekranını güncelle - başarılı gönderim ve gerçek response ile
        getUserNameAndUpdateScreen(String(cardId), actualResponse, true);
    }
    
    addLog(LOG_LEVEL_INFO, String(F("[PDKS_C11_TASK] ✅ Background task tamamlandı - Card ID: ")) + String(cardId));
    
    // Heap memory'yi free et
    free(cardId);
    
    // Task sonunda log
    addLog(LOG_LEVEL_ERROR, F("[PDKS_C11_TASK] *** TASK BİTİYOR ***"));
    Serial.println("[PDKS_C11_TASK] *** TASK BİTİYOR ***");
    
    // Task'ı sonlandır  
    vTaskDelete(nullptr);
}

// PDKS API'den user name al ve LVGL ekranını güncelle
void getUserNameAndUpdateScreen(const String& cardId, const String& apiResponse, bool success) {
    addLog(LOG_LEVEL_INFO, String(F("[LVGL_UPDATE] API yanıtı parse ediliyor: ")) + apiResponse);
    
    // WiFi bağlantısı kontrol et - WiFi.status() kullan
    if (WiFi.status() != WL_CONNECTED) {
        // WiFi bağlı değil - offline mod
        extern void updateLVGLScreen(const String& cardId, const String& userName, bool success);
        updateLVGLScreen(cardId, "WiFi Bağlı Değil", false);
        return;
    }
    
    String userName = "Bilinmiyor";
    bool finalSuccess = success;
    
    // API yanıtını parse et
    if (apiResponse.length() > 0) {
        addLog(LOG_LEVEL_ERROR, String(F("[API_RESPONSE] Ham API yanıtı: ")) + apiResponse);
        
        // JSON format kontrol et: {"Message":"...;USERNAME"}
        if (apiResponse.indexOf("{") >= 0 && apiResponse.indexOf("Message") >= 0) {
            // JSON parse - Message içindeki noktalı virgül sonrası kısmı al
            int messageStart = apiResponse.indexOf("\"Message\":\"");
            if (messageStart >= 0) {
                messageStart += 11; // "Message":" length
                int messageEnd = apiResponse.indexOf("\"", messageStart);
                if (messageEnd > messageStart) {
                    String message = apiResponse.substring(messageStart, messageEnd);
                    addLog(LOG_LEVEL_ERROR, String(F("[API_RESPONSE] Parsed Message: ")) + message);
                    
                    // Noktalı virgül sonrası kullanıcı adını al
                    int semicolonPos = message.lastIndexOf(';');
                    if (semicolonPos >= 0 && semicolonPos < message.length() - 1) {
                        userName = message.substring(semicolonPos + 1);
                        userName.trim(); // Boşlukları temizle
                        addLog(LOG_LEVEL_ERROR, String(F("[API_RESPONSE] Username extracted: ")) + userName);
                        
                        // Başarı durumunu mesaja göre belirle
                        if (message.indexOf("kaydedildi") >= 0 || message.indexOf("başarı") >= 0) {
                            finalSuccess = true;
                            addLog(LOG_LEVEL_ERROR, F("[API_RESPONSE] ✅ Başarılı kayıt tespit edildi"));
                        } else if (message.indexOf("kaydedilemedi") >= 0 || message.indexOf("hata") >= 0) {
                            finalSuccess = false;
                            addLog(LOG_LEVEL_ERROR, F("[API_RESPONSE] ❌ Kayıt hatası tespit edildi"));
                        }
                    } else {
                        addLog(LOG_LEVEL_ERROR, F("[API_RESPONSE] ⚠️ Semicolon bulunamadı veya username yok"));
                    }
                }
            }
        }
        // Plain text format: "Message;USERNAME" 
        else {
            addLog(LOG_LEVEL_ERROR, F("[API_RESPONSE] Plain text formatı tespit edildi"));
            int semicolonPos = apiResponse.lastIndexOf(';');
            if (semicolonPos >= 0 && semicolonPos < apiResponse.length() - 1) {
                userName = apiResponse.substring(semicolonPos + 1);
                userName.trim();
                
                // Ekran için kullanıcı adını temizle ve kısalt
                if (userName.isEmpty()) {
                    userName = "Kart Tanımlı Değil";
                } else {
                    // Eğer çok uzunsa ekran için kısalt
                    if (userName.length() > 18) {
                        userName = userName.substring(0, 15) + "...";
                    }
                }
                
                addLog(LOG_LEVEL_INFO, String(F("[API_RESPONSE] Kullanıcı: ")) + userName);
                
                // Başarı durumunu mesaja göre belirle
                if (apiResponse.indexOf("kaydedildi") >= 0 || apiResponse.indexOf("başarı") >= 0) {
                    finalSuccess = true;
                } else if (apiResponse.indexOf("kaydedilemedi") >= 0 || apiResponse.indexOf("hata") >= 0) {
                    finalSuccess = false;
                }
            } else {
                // Noktalı virgül yoksa API response'u kontrol et
                if (apiResponse.length() > 0) {
                    // API response varsa onu kısaltarak göster
                    userName = apiResponse.length() > 20 ? apiResponse.substring(0, 17) + "..." : apiResponse;
                    addLog(LOG_LEVEL_ERROR, String(F("[API_RESPONSE] Raw response as username: ")) + userName);
                } else {
                    // API response yoksa varsayılan mesaj
                    userName = success ? "Kart Okundu" : "Bağlantı Hatası";
                }
            }
        }
    } else {
        addLog(LOG_LEVEL_ERROR, F("[API_RESPONSE] ⚠️ API yanıtı boş"));
        // API response boşsa da hiç olmazsa success durumuna göre mesaj göster
        if (success) {
            userName = "Kullanıcı Bulunamadı";
        } else {
            userName = "Bağlantı Hatası";
        }
    }
    
    // Final sonuçları log et
    addLog(LOG_LEVEL_ERROR, String(F("========== [CARD_PROCESSING_FINAL] ==========")));
    addLog(LOG_LEVEL_ERROR, String(F("[FINAL_RESULT] Kart ID: ")) + cardId);
    addLog(LOG_LEVEL_ERROR, String(F("[FINAL_RESULT] Kullanıcı: ")) + userName);
    addLog(LOG_LEVEL_ERROR, String(F("[FINAL_RESULT] API Başarı: ")) + String(finalSuccess ? "✅ BAŞARILI" : "❌ BAŞARISIZ"));
    addLog(LOG_LEVEL_ERROR, String(F("[FINAL_RESULT] Orijinal Success: ")) + String(success ? "true" : "false"));
    addLog(LOG_LEVEL_ERROR, String(F("[FINAL_RESULT] API Response Length: ")) + String(apiResponse.length()));
    
    // LVGL ekranını güncelle - custom.cpp'deki external function
    extern void updateLVGLScreen(const String& cardId, const String& userName, bool success);
    updateLVGLScreen(cardId, userName, finalSuccess);
    
    addLog(LOG_LEVEL_ERROR, String(F("[LVGL_UPDATE] ✅ Ekran güncellendi: ")) + cardId + " -> " + userName + " (Final Success: " + String(finalSuccess ? "BAŞARILI" : "BAŞARISIZ") + ")");
    addLog(LOG_LEVEL_ERROR, String(F("============================================")));
}

// Local storage fonksiyonları - Offline RFID data management

// RFID verisini local file'a kaydet (server offline durumunda)
void saveRFIDDataToLocal(const String& cardId) {
    if (!LittleFS.begin()) {
        addLog(LOG_LEVEL_ERROR, F("[LOCAL_STORAGE] ❌ LittleFS mount başarısız!"));
        return;
    }
    
    // Gerçek sistem zamanını al (ESPEasy SystemVariables kullanarak)
    String sys_time = SystemVariables::getSystemVariable(SystemVariables::SYSTIME); // HH:MM:SS format
    String sys_day = SystemVariables::getSystemVariable(SystemVariables::SYSDAY_0);   // DD format 
    String sys_month = SystemVariables::getSystemVariable(SystemVariables::SYSMONTH_0); // MM format
    String sys_year = SystemVariables::getSystemVariable(SystemVariables::SYSYEAR);   // YYYY format
    
    // ISO timestamp formatı oluştur: YYYY-MM-DDTHH:MM:SS
    String timestamp = sys_year + "-" + sys_month + "-" + sys_day + "T" + sys_time;
    
    // Serial.printf("[LOCAL_STORAGE] Gerçek sistem zamanı kullanılıyor: %s\n", timestamp.c_str());
    
    // Device IP'sini al - C11 Controller ayarlarından
    String device_ip = SystemVariables::getSystemVariable(SystemVariables::IP);
    if (device_ip.length() == 0) {
        // C11 Controller IP'sini fallback olarak kullan
        if (Settings.ControllerEnabled[0]) {
            ControllerSettingsStruct controller_settings;
            LoadControllerSettings(0, controller_settings);
            IPAddress local_ip = WiFi.localIP();
            device_ip = local_ip.toString();
        }
        if (device_ip.length() == 0) {
            device_ip = "0.0.0.0"; // Son fallback
        }
    }

    String device_id = SystemVariables::getSystemVariable(SystemVariables::SYSNAME);
    
    addLog(LOG_LEVEL_INFO, String(F("[LOCAL_STORAGE] Device IP: ")) + device_ip);
    
    // Queue dosyasını append modda aç
    File queueFile = LittleFS.open("/rfid_queue.txt", "a");
    if (!queueFile) {
        addLog(LOG_LEVEL_ERROR, F("[LOCAL_STORAGE] ❌ Queue dosyası açılamadı!"));
        return;
    }
    
    // Format: timestamp|card_id|device_id|retry_count
    String queueEntry = device_id + "|" + cardId + "|" + timestamp + "|" + device_ip + "|0\n";
    queueFile.print(queueEntry);
    queueFile.close();
    
    addLog(LOG_LEVEL_ERROR, String(F("[LOCAL_STORAGE] 💾 OFFLINE KAYIT: ")) + cardId + String(F(" -> Local storage'a kaydedildi")));
    
    // LVGL ekranını güncelle - offline kayıt yapıldığını göster
    extern void updateLVGLScreen(const String& cardId, const String& userName, bool success);
    updateLVGLScreen(cardId, "Kaydedildi", false); // false = offline/error rengi (kırmızı/turuncu)
    
    // Saved resmini sol altta göster (3 saniye)
    extern void showSavedImage();
    addLog(LOG_LEVEL_ERROR, F("[LOCAL_STORAGE] 🖼️ Saved image gösteriliyor..."));
    showSavedImage();
    showSavedImage();
    
    // File size kontrolü (max 50KB)
    File checkFile = LittleFS.open("/rfid_queue.txt", "r");
    if (checkFile && checkFile.size() > 51200) { // 50KB
        addLog(LOG_LEVEL_INFO, F("[LOCAL_STORAGE] ⚠️ Queue dosyası çok büyük, temizleme gerekebilir"));
    }
    if (checkFile) checkFile.close();
}

// Server bağlantısı kontrol et (simple ping)
bool isServerConnected() {
    // WiFi kontrolü
    String wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
    if (wifi_status.toInt() < 2) {
        return false; // WiFi bağlı değil
    }
    
    // Controller ayarlarından IP ve Port al
    ControllerSettingsStruct controller_settings;
    if (Settings.ControllerEnabled[0]) {
        LoadControllerSettings(0, controller_settings);
    } else {
        // Controller aktif değil - bağlantı yok
        addLog(LOG_LEVEL_INFO, F("[SERVER_CHECK] Controller[0] aktif değil"));
        return false;
    }

    // TCP connect ile server durumunu test et
    IPAddress controller_ip = controller_settings.getIP();
    uint16_t controller_port = controller_settings.Port;

    WiFiClient sock;
    bool connected = false;
    
    if (sock.connect(controller_ip, controller_port)) {
        connected = true;
        sock.stop();
    }

    return connected;
}// Local queue'daki verileri server'a gönder
void processLocalStorageQueue() {
    // Serial.println("[QUEUE_PROCESSOR] 🔄 Queue processing başlatıldı...");
    // Serial.printf("[QUEUE_PROCESSOR] Free heap at start: %d bytes\n", ESP.getFreeHeap());
    
    // LittleFS güvenlik kontrolü
    if (!LittleFS.begin()) {
        addLog(LOG_LEVEL_ERROR, F("[QUEUE_PROCESSOR] ❌ LittleFS mount başarısız!"));
        return;
    }
    
    // Queue dosyasını kontrol et
    if (!LittleFS.exists("/rfid_queue.txt")) {
        // Serial.println("[QUEUE_PROCESSOR] 📁 Queue dosyası yok, işlem yok");
        return;
    }
    
    // File size kontrolü - çok büyükse işleme
    File checkFile = LittleFS.open("/rfid_queue.txt", "r");
    if (!checkFile) {
        addLog(LOG_LEVEL_ERROR, F("[QUEUE_PROCESSOR] ❌ Queue dosyası açılamadı!"));
        return;
    }
    
    size_t fileSize = checkFile.size();
    checkFile.close();
    
    if (fileSize > 102400) { // 100KB'den büyükse
        addLog(LOG_LEVEL_INFO, String(F("[QUEUE_PROCESSOR] ⚠️ Queue dosyası çok büyük (")) + String(fileSize) + String(F(" bytes) - İşlem atlandı")));
        return;
    }
    
    // Serial.printf("[QUEUE_PROCESSOR] Queue dosya boyutu: %d bytes\n", fileSize);
    
    File queueFile = LittleFS.open("/rfid_queue.txt", "r");
    if (!queueFile) {
        addLog(LOG_LEVEL_ERROR, F("[QUEUE_PROCESSOR] ❌ Queue dosyası ikinci açılış başarısız!"));
        return;
    }
    
    // Serial.println("[QUEUE_PROCESSOR] 🔄 Queue processing başladı...");
    
    String processedLines = "";
    String pendingLines = "";
    int processedCount = 0;
    int errorCount = 0;
    int lineCount = 0;
    
    while (queueFile.available() && lineCount < 50) { // Max 50 line işle
        String line = queueFile.readStringUntil('\n');
        line.trim();
        lineCount++;
        
        if (line.length() == 0) continue;
        
        // Memory kontrolü her 10 line'da bir
        if (lineCount % 10 == 0) {
            if (ESP.getFreeHeap() < 8000) { // 8KB threshold
                addLog(LOG_LEVEL_INFO, F("[QUEUE_PROCESSOR] ⚠️ Memory düşük - Processing durduruluyor"));
                break;
            }
        }
        
        // Line format: device_id|card_id|timestamp|device_ip|retry_count
        int pipe1 = line.indexOf('|');
        int pipe2 = line.indexOf('|', pipe1 + 1);
        int pipe3 = line.indexOf('|', pipe2 + 1);
        int pipe4 = line.indexOf('|', pipe3 + 1);
        
        if (pipe1 == -1 || pipe2 == -1 || pipe3 == -1 || pipe4 == -1) {
            Serial.printf("[QUEUE_PROCESSOR] ⚠️ Invalid format: %s\n", line.c_str());
            continue;
        }
        
        String device_id = line.substring(0, pipe1);
        String card_id = line.substring(pipe1 + 1, pipe2);
        String timestamp = line.substring(pipe2 + 1, pipe3);
        String device_ip = line.substring(pipe3 + 1, pipe4);
        int retry_count = line.substring(pipe4 + 1).toInt();
        
        // Max retry 3 kez
        if (retry_count >= 3) {
            // Serial.printf("[QUEUE_PROCESSOR] ❌ Max retry ulaşıldı: %s\n", card_id.c_str());
            continue; // Bu entry'yi at
        }
        
        // API'ye göndermeyi dene
        if (sendQueuedDataToAPI(card_id, device_ip, timestamp, device_id)) {
            processedCount++;
            // Serial.printf("[QUEUE_PROCESSOR] ✅ Başarılı: %s\n", card_id.c_str());
        } else {
            // Retry count'u artır ve tekrar queue'ya ekle
            retry_count++;
            pendingLines += device_id + "|" + card_id + "|" + timestamp + "|" + device_ip + "|" + String(retry_count) + "\n";
            errorCount++;
            // Serial.printf("[QUEUE_PROCESSOR] ❌ Hata (retry %d): %s\n", retry_count, card_id.c_str());
        }
        
        // CPU yield - watchdog reset önlemi
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    queueFile.close();
    // Serial.printf("[QUEUE_PROCESSOR] File işleme tamamlandı - %d line işlendi\n", lineCount);
    
    // Queue dosyasını güvenli şekilde yeniden yaz
    if (pendingLines.length() > 0) {
        File newQueueFile = LittleFS.open("/rfid_queue.txt", "w");
        if (newQueueFile) {
            newQueueFile.print(pendingLines);
            newQueueFile.close();
            addLog(LOG_LEVEL_INFO, F("[QUEUE_PROCESSOR] 📝 Queue dosyası güncellendi"));
        } else {
            addLog(LOG_LEVEL_ERROR, F("[QUEUE_PROCESSOR] ❌ Queue dosyası yazma hatası!"));
        }
    } else {
        // Queue boş - dosyayı sil
        LittleFS.remove("/rfid_queue.txt");
        addLog(LOG_LEVEL_INFO, F("[QUEUE_PROCESSOR] 🗑️ Queue boş - dosya silindi"));
    }
    
    addLog(LOG_LEVEL_INFO, String(F("[QUEUE_PROCESSOR] ✅ Tamamlandı - İşlenen: ")) + String(processedCount) + String(F(", Hata: ")) + String(errorCount));
    // Serial.printf("[QUEUE_PROCESSOR] Free heap at end: %d bytes\n", ESP.getFreeHeap());
}

// Queue'dan gelen veriyi API'ye gönder - C11 Controller kullanarak
bool sendQueuedDataToAPI(const String& cardId, const String& deviceIP, const String& timestamp, const String& deviceId) {
    // C11 Controller aktif mi kontrol et
    if (!Settings.ControllerEnabled[0]) {
        addLog(LOG_LEVEL_ERROR, F("[API_QUEUE] Controller[0] aktif değil"));
        return false;
    }
    
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📤 Queue data C11 ile gönderiliyor: ")) + cardId);
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 🕐 Timestamp: ")) + timestamp);
    
    // SystemVariable'ları güncelle
    SystemVariables::setCardID(cardId);
    SystemVariables::setCardTimestamp(timestamp);
    
    // ESPEasy C11 Controller sistemi kullan
    struct EventStruct TempEvent;
    TempEvent.ControllerIndex = 0; // Controller 0 kullan
    TempEvent.idx = 0;
    TempEvent.TaskIndex = 255; // Dummy task index
    TempEvent.sensorType = Sensor_VType::SENSOR_TYPE_STRING;
    
    addLog(LOG_LEVEL_INFO, F("[API_QUEUE] 🔄 C11 Controller çağrılıyor..."));
    
    // SystemVariable'ları tekrar logla
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Güncel CARDID: ")) + SystemVariables::getSystemVariable(SystemVariables::CARDID));
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Güncel TIMESTAMP: ")) + SystemVariables::getSystemVariable(SystemVariables::CARDTIMESTAMP));
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Device IP: ")) + deviceIP);
    addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Device ID: ")) + deviceId);
    
    // Controller ayarlarını da logla - hangi endpoint'e gönderdiğini görmek için
    if (Settings.ControllerEnabled[0]) {
        ControllerSettingsStruct controller_settings;
        LoadControllerSettings(0, controller_settings);
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Controller IP: ")) + controller_settings.getIP().toString());
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Controller Port: ")) + String(controller_settings.Port));
        
        // URI ve Body bilgilerini Settings array'lerinden al
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Controller Enabled: ")) + String(Settings.ControllerEnabled[0] ? "true" : "false"));
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 Controller Protocol: ")) + String(Settings.Protocol[0]));
        
        // HTTP Method ve diğer ayarları mümkünse göster
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📋 POST Request hazırlanıyor...")));
    }
    
    String dummy_string = "";
    bool success = CPlugin_011(CPlugin::Function::CPLUGIN_PROTOCOL_SEND, &TempEvent, dummy_string);
    
    if (success) {
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] ✅ Queue data API'ye başarıyla gönderildi: ")) + cardId);
        addLog(LOG_LEVEL_INFO, F("[API_QUEUE] 📥 Sunucu cevabı alındı"));
        addLog(LOG_LEVEL_INFO, String(F("[API_QUEUE] 📥 Response data: ")) + dummy_string);
    } else {
        addLog(LOG_LEVEL_ERROR, String(F("[API_QUEUE] ❌ Queue data API gönderimi başarısız: ")) + cardId);
        addLog(LOG_LEVEL_ERROR, F("[API_QUEUE] 🔍 Controller konfigürasyonunu kontrol edin"));
        addLog(LOG_LEVEL_ERROR, String(F("[API_QUEUE] 🔍 Error response: ")) + dummy_string);
    }
    
    return success;
}

// ------------------- ESPEasy Loop Integration -------------------

// Periyodik API connectivity check - ESPEasy loop'tan çağrılacak
void checkAPIConnectivityPeriodic() {
    unsigned long currentTime = millis();
    
    // 30 saniyede bir kontrol et
    if (currentTime - g_lastConnectivityCheck > CONNECTIVITY_CHECK_INTERVAL) {
        g_lastConnectivityCheck = currentTime;
        
        bool previousStatus = g_apiConnected;
        g_apiConnected = checkAPIConnectivity();
        
        addLog(LOG_LEVEL_INFO, String(F("[API_HEALTH] Periodic check - Previous: ")) + String(previousStatus ? "connected" : "disconnected") + String(F(", Current: ")) + String(g_apiConnected ? "connected" : "disconnected"));
        
        // Durum değişirse queue'yu kontrol et
        if (!previousStatus && g_apiConnected) {
            // API tekrar erişilebilir oldu, offline queue'yu işle
            addLog(LOG_LEVEL_INFO, F("[API_HEALTH] 🔄 API erişilebilir oldu, offline queue kontrol ediliyor..."));
            processLocalStorageQueue();
        }
    }
}

// ESPEasy PLUGIN_FIFTY_PER_SECOND handler
void handleUSBHIDRFIDLoop() {
    // API connectivity check
    checkAPIConnectivityPeriodic();
    
    // Background task response monitoring
    if (g_apiResponseReady) {
        // Bu flag set edildiğinde C11 response'u loglardan parse et
        handleBackgroundAPIResponse();
    }
}

// Background API response handler
void handleBackgroundAPIResponse() {
    static unsigned long responseStartTime = millis();
    static bool waitingForResponse = false;
    
    if (!waitingForResponse) {
        // İlk kez çağrıldı, timer başlat
        responseStartTime = millis();
        waitingForResponse = true;
        addLog(LOG_LEVEL_ERROR, F("[RESPONSE_MONITOR] ⏱️ C11 response bekleniyor..."));
        return;
    }
    
    // 2 saniye timeout
    if (millis() - responseStartTime > 2000) {
        addLog(LOG_LEVEL_ERROR, F("[RESPONSE_MONITOR] ⏰ Response timeout, varsayılan işlemi uygula"));
        
        // Timeout durumunda yine de ekranı güncelle
        if (!g_currentCardId.isEmpty()) {
            getUserNameAndUpdateScreen(g_currentCardId, g_lastAPIResponse, true);
        }
        
        // Reset flags
        g_apiResponseReady = false;
        waitingForResponse = false;
        g_currentCardId = "";
        g_lastAPIResponse = "";
        return;
    }
    
    // Response henüz gelmemiş, bekle
    // Gerçek implementasyonda burada log parsing veya callback sistemi olacak
    // Şimdilik basit timeout ile çalışıyoruz
}

// USB Health check removed to prevent startup issues