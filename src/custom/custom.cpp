/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <Arduino.h>
#include "lvgl.h"
#include "../usb/usb_hid_rfid.h"  // API connection status için
#include "custom.h"
#include "../Custom.h"
#include "../src/Helpers/SystemVariables.h"
#include <HTTPClient.h>
#include "../src/Globals/Settings.h"
#include "../src/Helpers/ESPEasy_Storage.h"
#include "../src/DataStructs/ControllerSettingsStruct.h"

// Server bağlantı durumu için extern değişken
extern bool serverConnectionStatus;

// PDKS API server sağlık kontrolü fonksiyonu - header declare et
void checkServerHealth();

// Local storage queue işleme fonksiyonu - usb_hid_rfid.cpp'den
extern void processLocalStorageQueue();
extern bool isServerConnected();

// Server bağlantı durumunu güncelleme fonksiyonu
extern void updateServerConnectionStatus(bool connected);

#include "../src/Globals/RuntimeData.h"

// WiFi icon extern declarations
extern const lv_image_dsc_t _wifi_lost_signal_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_low1_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_low2_signal_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_full_signal_RGB565A8_35x35;

// Ready image extern declaration
extern const lv_image_dsc_t _ready_RGB565A8_100x100;

// Saved image extern declaration
extern const lv_image_dsc_t _saved_RGB565A8_96x62;

// GUI mutex for thread-safe LVGL operations
extern SemaphoreHandle_t gui_mutex;

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */

extern lv_ui guider_ui;

// RFID veri transfer için external değişkenler (main.cpp'den)
extern char rfid_data[64];
extern volatile bool rfid_data_ready;
extern SemaphoreHandle_t rfid_data_mutex;

// HTTP Response için değişkenler
char http_response_data[256] = {0};
volatile bool http_response_ready = false;

// LVGL ekran güncelleme fonksiyonu - main.cpp'den taşındı
// Kart bilgileri görüntüleme durumu
static unsigned long last_card_display_time = 0;
static String last_user_name = "";
static bool displaying_card_info = false;
static bool showing_ready_image = false;
static bool showing_saved_image = false;

// Ready image timer
static lv_timer_t* ready_image_timer = nullptr;

// Saved image timer
static lv_timer_t* saved_image_timer = nullptr;

// Health check task control flag
static bool health_check_task_running = false;

// Ortak ekran güncelleme fonksiyonu (tarih, saat, wifi, ip)
void updateSystemInfoWithGui(lv_ui* gui) {
    // Serial.println("[DEBUG] updateSystemInfoWithGui() çalışıyor..."); // DEBUG spam önlendi
    
    if (!gui) {
        Serial.println("[ERROR] GUI pointer NULL!");
        return;
    }
    
    // 1. TARIH VE SAAT GÜNCELLEMESİ - Cache static strings
    static String cached_time = "";
    static String cached_date = "";
    static unsigned long last_update = 0;
    
    // Sadece 1 saniyede bir güncelle (memory optimizasyonu)
    if (millis() - last_update > 1000) {
        cached_time = SystemVariables::getSystemVariable(SystemVariables::SYSTIME);
        
        // Tarih için - daha az sıklıkta güncelle
        String sys_day = SystemVariables::getSystemVariable(SystemVariables::SYSDAY_0);
        String sys_month = SystemVariables::getSystemVariable(SystemVariables::SYSMONTH_0);
        String sys_year = SystemVariables::getSystemVariable(SystemVariables::SYSYEAR);
        cached_date = sys_day + "/" + sys_month + "/" + sys_year;
        
        last_update = millis();
    }
    
    if (gui->screen_main_label_saat) {
      lv_label_set_text(gui->screen_main_label_saat, cached_time.c_str());
    }
    
    if (gui->screen_main_label_tarih) {
      lv_label_set_text(gui->screen_main_label_tarih, cached_date.c_str());
    }
    
    // 2. WIFI DURUM VE SINYAL GÜCÜ GÜNCELLEMESİ - Cache static strings
    static String cached_wifi_status = "";
    static String cached_wifi_rssi = "";
    static String cached_sys_ip = "";
    static unsigned long last_wifi_update = 0;
    
    // WiFi bilgilerini 3 saniyede bir güncelle (daha seyrek)
    if (millis() - last_wifi_update > 3000) {
        cached_wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
        cached_wifi_rssi = SystemVariables::getSystemVariable(SystemVariables::RSSI);
        cached_sys_ip = SystemVariables::getSystemVariable(SystemVariables::IP);
        last_wifi_update = millis();
    }
    
    // Önce tüm WiFi ikonlarını gizle
    lv_obj_add_flag(gui->screen_main_img_wifi_lost_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_low1_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_low2_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
    
    // 3. IP ADRESİ VE WIFI İKON DURUMU - Cached values kullan
    int wifi_status_int = cached_wifi_status.toInt();
    if (wifi_status_int >= 2) {
        // WiFi bağlı ve IP aldı - TCP port durumuna göre renk belirle
        if (gui->screen_main_label_ip_adres) {
          // TCP API bağlantı durumunu kontrol et
          bool apiConnected = getAPIConnectionStatus();
          
          // Server durumu ile birlikte IP göster
          static String ip_text = "";
          ip_text = cached_sys_ip;
          
          if (apiConnected) {
              ip_text += " [O]";  // TCP Server bağlı
              lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);  // Yeşil
          } else {
              ip_text += " [X]";  // TCP Server bağlı değil  
              lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);  // Kırmızı
          }
          lv_label_set_text(gui->screen_main_label_ip_adres, ip_text.c_str());
        }
        
        // RSSI değerine göre WiFi ikonunu göster - Cached value kullan
        int rssi_value = cached_wifi_rssi.toInt();
        if (rssi_value >= -50) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
        } else if (rssi_value >= -65) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
        } else if (rssi_value >= -75) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_low2_signal, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_clear_flag(gui->screen_main_img_wifi_low1_signal, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        // WiFi bağlı değil - AP mode IP göster, kırmızı renk
        if (gui->screen_main_label_ip_adres) {
          lv_label_set_text(gui->screen_main_label_ip_adres, "192.168.4.1 [X]");
          lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);  // Kırmızı
        }
        lv_obj_clear_flag(gui->screen_main_img_wifi_lost_signal, LV_OBJ_FLAG_HIDDEN);
    }
    
    //Serial.println("[DEBUG] updateSystemInfoWithGui() tamamlandı");
}

// Ready image timer callback - 5 saniye sonra resmi gizle
void ready_image_timer_cb(lv_timer_t* timer) {
    // Ready resmi gizle
    if (guider_ui.screen_main_img_ready)
        lv_obj_add_flag(guider_ui.screen_main_img_ready, LV_OBJ_FLAG_HIDDEN);    
    // "KARTINIZI OKUTUNUZ" label'larını tekrar göster (sadece eski büyük olanı)
    if (guider_ui.screen_main_label_kart_okutun)
        lv_obj_clear_flag(guider_ui.screen_main_label_kart_okutun, LV_OBJ_FLAG_HIDDEN);
    // User name label'ını tekrar göster - eğer veri varsa göster, yoksa temizle
    if (guider_ui.screen_main_label_personel_id) {
        lv_obj_clear_flag(guider_ui.screen_main_label_personel_id, LV_OBJ_FLAG_HIDDEN);
        // Eğer geçerli user name bilgisi varsa onu göster, yoksa temizle
        if (displaying_card_info && last_user_name.length() > 0) {
            lv_label_set_text(guider_ui.screen_main_label_personel_id, last_user_name.c_str());
            //Serial.printf("[READY_IMG] User name geri yüklendi: %s\n", last_user_name.c_str());
        } else {
            lv_label_set_text(guider_ui.screen_main_label_personel_id, "");
            //Serial.println("[READY_IMG] User name temizlendi");
        }
    }    
    showing_ready_image = false;    
    // Timer'ı sil
    if (timer) {
        lv_timer_del(timer);
        ready_image_timer = nullptr;
    }    
    //Serial.println("[READY_IMG] ✅ Ready resmi gizlendi, normal ekran geri yüklendi");
}

// Ready resmi gösterme fonksiyonu
void showReadyImage() {
    // Mevcut timer varsa iptal et
    if (ready_image_timer) {
        lv_timer_del(ready_image_timer);
        ready_image_timer = nullptr;
    }    
    // "KARTINIZI OKUTUNUZ" label'larını gizle (hem eski büyük hem yeni küçük)
    if (guider_ui.screen_main_label_kart_okutun)
        lv_obj_add_flag(guider_ui.screen_main_label_kart_okutun, LV_OBJ_FLAG_HIDDEN);
    // User name label'ını gizle
    if (guider_ui.screen_main_label_personel_id)
        lv_obj_add_flag(guider_ui.screen_main_label_personel_id, LV_OBJ_FLAG_HIDDEN);
    // Ready resmini göster
    if (guider_ui.screen_main_img_ready) {
        lv_obj_clear_flag(guider_ui.screen_main_img_ready, LV_OBJ_FLAG_HIDDEN);
        // Resmin ortalanması için pozisyonu ayarla (gerekirse)
        lv_obj_set_align(guider_ui.screen_main_img_ready, LV_ALIGN_CENTER);
    }    
    showing_ready_image = true;    
    // 5 saniye sonra gizlemek için timer oluştur
    ready_image_timer = lv_timer_create(ready_image_timer_cb, 3000, nullptr);
    lv_timer_set_repeat_count(ready_image_timer, 1); // Sadece 1 kez çalış    
    //Serial.println("[READY_IMG] ✅ Ready resmi gösteriliyor, 5 saniye sonra gizlenecek");
}

// Saved image timer callback - 3 saniye sonra resmi gizle
void saved_image_timer_cb(lv_timer_t* timer) {
    Serial.println("[SAVED_IMG_TIMER] ====== Timer callback çalıştı ======");
    
    // Saved resmi gizle
    if (guider_ui.screen_main_img_saved) {
        lv_obj_add_flag(guider_ui.screen_main_img_saved, LV_OBJ_FLAG_HIDDEN);
        Serial.println("[SAVED_IMG_TIMER] ✅ Saved resmi gizlendi");
    } else {
        Serial.println("[SAVED_IMG_TIMER] ❌ HATA: Saved obje NULL!");
    }
    
    // Ready image gösterilmiyorsa normal ekran elementlerini geri yükle
    if (!showing_ready_image) {
        Serial.println("[SAVED_IMG_TIMER] Ready image gösterilmiyor, normal ekranı geri yüklüyor");
        // "KARTINIZI OKUTUNUZ" label'ını tekrar göster
        if (guider_ui.screen_main_label_kart_okutun) {
            lv_obj_clear_flag(guider_ui.screen_main_label_kart_okutun, LV_OBJ_FLAG_HIDDEN);
            Serial.println("[SAVED_IMG_TIMER] Kart okutun label'ı gösterildi");
        }
        // User name label'ını tekrar göster - eğer veri varsa göster, yoksa temizle
        if (guider_ui.screen_main_label_personel_id) {
            lv_obj_clear_flag(guider_ui.screen_main_label_personel_id, LV_OBJ_FLAG_HIDDEN);
            // Eğer geçerli user name bilgisi varsa onu göster, yoksa temizle
            if (displaying_card_info && last_user_name.length() > 0) {
                lv_label_set_text(guider_ui.screen_main_label_personel_id, last_user_name.c_str());
                Serial.printf("[SAVED_IMG_TIMER] User name geri yüklendi: %s\n", last_user_name.c_str());
            } else {
                lv_label_set_text(guider_ui.screen_main_label_personel_id, "");
                Serial.println("[SAVED_IMG_TIMER] User name temizlendi");
            }
        }
    } else {
        Serial.println("[SAVED_IMG_TIMER] Ready image gösteriliyor, normal ekranı geri yükleme atlandı");
    }
    
    showing_saved_image = false;
    Serial.println("[SAVED_IMG_TIMER] showing_saved_image = false olarak ayarlandı");
    
    // Timer'ı sil
    if (timer) {
        lv_timer_del(timer);
        saved_image_timer = nullptr;
        Serial.println("[SAVED_IMG_TIMER] Timer silindi ve nullptr olarak ayarlandı");
    } else {
        Serial.println("[SAVED_IMG_TIMER] ❌ HATA: Timer NULL!");
    }
    
    Serial.println("[SAVED_IMG_TIMER] ========================================");
}

// Saved resmi gösterme fonksiyonu (offline kayıt için)
void showSavedImage() {
    Serial.println("[SAVED_IMG] ====== showSavedImage() çağrıldı ======");
    
    // Mevcut timer varsa iptal et
    if (saved_image_timer) {
        lv_timer_del(saved_image_timer);
        saved_image_timer = nullptr;
        Serial.println("[SAVED_IMG] Önceki timer iptal edildi");
    }    
    
    // Saved resmini sol altta göster
    if (guider_ui.screen_main_img_saved) {
        Serial.println("[SAVED_IMG] ✅ Saved objesi bulundu, gösteriliyor...");
        lv_obj_clear_flag(guider_ui.screen_main_img_saved, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_pos(guider_ui.screen_main_img_saved, 10, 230); // Sol alt: x=10, y=230
        Serial.println("[SAVED_IMG] Pozisyon ayarlandı (10, 230)");
    } else {
        Serial.println("[SAVED_IMG] ❌ HATA: guider_ui.screen_main_img_saved objesi NULL!");
        return;
    }    
    
    showing_saved_image = true;    
    Serial.println("[SAVED_IMG] showing_saved_image = true olarak ayarlandı");
    
    // 3 saniye sonra gizlemek için timer oluştur
    saved_image_timer = lv_timer_create(saved_image_timer_cb, 2000, nullptr);
    lv_timer_set_repeat_count(saved_image_timer, 1); // Sadece 1 kez çalış    
    Serial.println("[SAVED_IMG] ✅ Timer oluşturuldu (2000ms), saved resmi sol altta gösteriliyor");
    Serial.println("[SAVED_IMG] ========================================");
}

// Ortak ekran güncelleme fonksiyonu (tarih, saat, wifi, ip) - eski versiyon
void updateSystemInfo() {
    updateSystemInfoWithGui(&guider_ui);
}

void updateLVGLScreen(const String& cardId, const String& userName, bool success) {
  if (gui_mutex && xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    
    // Sistem bilgilerini güncelle (tarih, saat, wifi, ip)
    updateSystemInfo();
    
    // PERSONEL ID LABEL GÜNCELLEMESİ - Önce user name'i ayarla
    if (guider_ui.screen_main_label_personel_id) {
      if (success && userName.length() > 0) {
        lv_label_set_text(guider_ui.screen_main_label_personel_id, userName.c_str());
        last_card_display_time = millis();
        last_user_name = userName;
        displaying_card_info = true;
        
        //Serial.printf("[LVGL] User name ayarlandı: %s\n", userName.c_str());
      } else {
        lv_label_set_text(guider_ui.screen_main_label_personel_id, "Bilinmeyen Kart");
        last_card_display_time = millis();
        last_user_name = "Bilinmeyen Kart";
        displaying_card_info = true;
        
        //Serial.println("[LVGL] User name ayarlandı: Bilinmeyen Kart");
      }
    }
    
    // Ready resmini göster (bu user name'i geçici olarak gizleyecek)
    //showReadyImage();
    
    xSemaphoreGive(gui_mutex);
  }
}

void custom_init(lv_ui *ui)
{
    /* Add your codes here */
    
    //Serial.println("🔧 Custom init çağrıldı - Timer'lar ana ekran sonrası başlatılacak");
    
    // Timer oluşturma işi loader_anim_complete_cb() fonksiyonuna taşındı
    // Bu sayede logo ekranı bittikten ve main screen yüklendikten sonra timer'lar başlar
}

void loader_anim_cb(lv_obj_t* var, uint32_t value)
{
    lv_bar_set_value(var, value, LV_ANIM_ON);
}

void loader_anim_complete_cb(lv_obj_t* var)
{
    // Ana ekrana geçiş yap
    ui_load_scr_animation(&guider_ui, &guider_ui.screen_main, guider_ui.screen_main_del, &guider_ui.screen_logo_del, setup_scr_screen_main, LV_SCR_LOAD_ANIM_NONE, 2000, 1000, false, true);
    
    // Ana ekran yüklendikten sonra custom timer'ları başlat
    //Serial.println("🔧 Ana ekran yüklendi, Custom timer'lar başlatılıyor...");
    
    // LVGL sistem zaman ve tarih güncellemesi için timer oluştur
    // Her 1000ms (1 saniye) de bir çalışacak ve ui pointer'ını user_data olarak geçir
    lv_timer_t* data_timer = lv_timer_create(data_update_timer_cb, 1000, &guider_ui);
    
    if (data_timer) {
        //Serial.println("[DEBUG] ✅ Timer başarıyla oluşturuldu - Period: 1000 ms (1 saniye)");
        
        // Timer'ı aktif et
        lv_timer_resume(data_timer);
        
        // Timer repeat ayarla (sürekli çalışsın)
        lv_timer_set_repeat_count(data_timer, -1); // Sonsuz tekrar
        
        //Serial.println("[DEBUG] Timer aktif edildi ve sonsuz tekrara ayarlandı");
        
        // İlk çalıştırmayı tetikle (2 saniye sonra - main screen yüklenmesini bekle)
        lv_timer_set_period(data_timer, 2000); // İlk çalışma için 2 saniye
        //Serial.println("[DEBUG] İlk timer callback 2 saniye sonra tetiklenecek");
    } /*else {
        Serial.println("[ERROR] ❌ Timer oluşturulamadı!");
    }*/
}

void data_update_timer_cb(lv_timer_t* timer)
{
    /* Update time and date on the main screen using ESPEasy system variables */
    
    // İlk çalışmadan sonra normal periyoda dön (1 saniye)
    static bool first_run = true;
    if (first_run) {
        lv_timer_set_period(timer, 1000); // Normal 1 saniye periyoduna dön
        first_run = false;
    }
    
    // Timer'dan ui pointer'ını al
    lv_ui* gui = static_cast<lv_ui*>(lv_timer_get_user_data(timer));
    
    if (!gui) {
        return;
    }
    
    // Debug: Timer çalışıyor mu kontrol et
    static int timer_counter = 0;
    timer_counter++;
    
    // LVGL Timer'ları LVGL Task içinde çalıştığı için mutex'e ihtiyaç yoktur
    // Direkt LVGL API'lerini çağırabiliriz
    
    // Sistem bilgilerini güncelle (tarih, saat, wifi, ip)
    updateSystemInfoWithGui(gui);
    
    // Her 10 saniyede bir server bağlantısını test et (timer_counter % 10 == 0)
    static int last_server_test = 0;
    if (timer_counter != last_server_test && timer_counter % 10 == 0) {
        last_server_test = timer_counter;
        
        // Gerçek PDKS API /health endpoint kontrolü
        checkServerHealth();
    }
    
    // Her 60 saniyede bir queue processing yap (timer_counter % 60 == 0)
    static int last_queue_process = 0;
    if (timer_counter != last_queue_process && timer_counter % 60 == 0) {
        last_queue_process = timer_counter;
        
        // Server bağlıysa queue'yu işle
        if (isServerConnected()) {
            //Serial.println("[HEALTH_CHECK] 📤 Server online - Queue processing başlatılıyor");
            
            // Background task olarak queue processing başlat - Stack size artırıldı
            xTaskCreatePinnedToCore(
                healthCheckTask,           // Task fonksiyonu
                "HealthCheckTask",         // Task adı
                8192,                      // Stack size (4096->8192 artırıldı)
                nullptr,                   // Parametre yok
                1,                         // Priority (düşük priority)
                nullptr,                   // Task handle
                0                          // Core 0
            );
        }
    }
    
    // User name display timeout kontrolü (10 saniye sonra temizle)
    if (displaying_card_info && (millis() - last_card_display_time > 10000)) {
        displaying_card_info = false;
        last_user_name = "";
        
        if (gui->screen_main_label_personel_id && !showing_ready_image) {
            lv_label_set_text(gui->screen_main_label_personel_id, "");
        }
    }
}

// PDKS API server sağlık kontrolü fonksiyonu
void checkServerHealth() {
    // Memory check - eğer yeterli memory yoksa atla
    if (ESP.getFreeHeap() < 20000) {
        Serial.println("[SERVER] ⚠️ Memory yetersiz - Server health check atlandı");
        updateServerConnectionStatus(false);
        return;
    }
    
    // WiFi bağlantısı kontrol et
    String wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
    if (wifi_status.toInt() < 2) {
        // WiFi bağlı değil
        updateServerConnectionStatus(false);
        //Serial.println("[SERVER] WiFi bağlı değil - Server offline");
        return;
    }
    
    HTTPClient http;
    http.setTimeout(2000); // 2 saniye timeout
    
    // PDKS API health endpoint URL - Settings.Controller[0] IP ve Port kullan
    String health_url = "";
    if (Settings.ControllerEnabled[0]) {
        // Controller 0 ayarlarını yükle
        ControllerSettingsStruct controller_settings;
        LoadControllerSettings(0, controller_settings);
        
        IPAddress controller_ip = controller_settings.getIP();
        uint16_t controller_port = controller_settings.Port;
        
        health_url = "http://" + controller_ip.toString() + ":" + String(controller_port) + "/health";
        //Serial.printf("[SERVER] Controller[0] ayarları kullanılıyor - IP: %s, Port: %d\n", controller_ip.toString().c_str(), controller_port);
    } else {
        // Fallback: Custom.h'den ya da varsayılan
        #ifdef PDKS_SERVER_URL
            health_url = String(PDKS_SERVER_URL) + "/health";
        #else
            health_url = "http://192.168.1.100:5000/health"; // Varsayılan
        #endif
        //Serial.println("[SERVER] Controller[0] deaktif - Varsayılan URL kullanılıyor");
    }
    
    //Serial.printf("[SERVER] Health check başlatılıyor: %s\n", health_url.c_str());
    
    http.begin(health_url);
    http.addHeader("Content-Type", "application/json");
    
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
        String response = http.getString();
        //Serial.printf("[SERVER] ✅ Health check OK - Response: %s\n", response.c_str());
        
        // JSON response kontrol et (basit string kontrolü)
        if (response.indexOf("\"status\":\"healthy\"") != -1) {
            updateServerConnectionStatus(true);
            //Serial.println("[SERVER] ✅ Server ONLINE - Database bağlı");
        } else {
            updateServerConnectionStatus(false);
            //Serial.println("[SERVER] ❌ Server OFFLINE - Database problemi");
        }
    } else if (httpResponseCode > 0) {
        //Serial.printf("[SERVER] ❌ HTTP Error: %d\n", httpResponseCode);
        updateServerConnectionStatus(false);
    } else {
        //Serial.printf("[SERVER] ❌ Connection Error: %s\n", http.errorToString(httpResponseCode).c_str());
        updateServerConnectionStatus(false);
    }
    
    http.end();
}

// Background health check task - queue processing için
void healthCheckTask(void* parameter) {
    //Serial.println("[HEALTH_TASK] 🔄 Background health check ve queue processing başladı");
    //Serial.printf("[HEALTH_TASK] Free heap: %d bytes\n", ESP.getFreeHeap());
    //Serial.printf("[HEALTH_TASK] Task stack high water mark: %d\n", uxTaskGetStackHighWaterMark(nullptr));
    
    // Memory güvenlik kontrolü - limiti artırdık
    if (ESP.getFreeHeap() < 15000) { // 15KB'den az heap kaldıysa
        //Serial.println("[HEALTH_TASK] ❌ Heap memory yetersiz - Queue processing atlandı");
        health_check_task_running = false; // Flag'i sıfırla
        vTaskDelete(nullptr);
        return;
    }
    
    // Delay ekle - sistem stabilizasyonu için
    vTaskDelay(pdMS_TO_TICKS(500));
    
    // Server bağlantısını tekrar kontrol et
    //Serial.println("[HEALTH_TASK] 🌐 Server bağlantısı kontrol ediliyor...");
    if (isServerConnected()) {
        //Serial.println("[HEALTH_TASK] ✅ Server bağlı - Queue processing yapılıyor");
        
        // İkinci güvenlik kontrolü
        //Serial.printf("[HEALTH_TASK] Pre-queue free heap: %d bytes\n", ESP.getFreeHeap());
        
        // Güvenli queue processing
        processLocalStorageQueue();
        //Serial.println("[HEALTH_TASK] 📤 Queue processing tamamlandı");
        
        //Serial.printf("[HEALTH_TASK] Post-queue free heap: %d bytes\n", ESP.getFreeHeap());
    } /*else {
        Serial.println("[HEALTH_TASK] ❌ Server bağlı değil - Queue processing atlandı");
    }*/
    
    //Serial.println("[HEALTH_TASK] ✅ Task tamamlanıyor");
    //Serial.printf("[HEALTH_TASK] Final stack high water mark: %d\n", uxTaskGetStackHighWaterMark(nullptr));
    
    // Flag'i sıfırla - başka task başlatılabilir
    health_check_task_running = false;
    
    // Task'ı sonlandır
    vTaskDelete(nullptr);
}