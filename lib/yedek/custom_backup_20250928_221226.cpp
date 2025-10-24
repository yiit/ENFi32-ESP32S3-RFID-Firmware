/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include <Arduino.h>
#include "lvgl.h"
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

// Server bağlantı durumunu güncelleme fonksiyonu
extern void updateServerConnectionStatus(bool connected);

#include "../src/Globals/RuntimeData.h"

// WiFi icon extern declarations
extern const lv_image_dsc_t _wifi_lost_signal_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_low1_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_low2_signal_RGB565A8_35x35;
extern const lv_image_dsc_t _wifi_full_signal_RGB565A8_35x35;

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

// Ortak ekran güncelleme fonksiyonu (tarih, saat, wifi, ip)
void updateSystemInfoWithGui(lv_ui* gui) {
    // Serial.println("[DEBUG] updateSystemInfoWithGui() çalışıyor..."); // DEBUG spam önlendi
    
    if (!gui) {
        Serial.println("[ERROR] GUI pointer NULL!");
        return;
    }
    
    // 1. TARIH VE SAAT GÜNCELLEMESİ
    String sys_time = SystemVariables::getSystemVariable(SystemVariables::SYSTIME);
    if (gui->screen_main_label_saat) {
      lv_label_set_text(gui->screen_main_label_saat, sys_time.c_str());
      //Serial.printf("[DEBUG] Saat label güncellendi: %s\n", sys_time.c_str());
    } /*else {
      Serial.println("[DEBUG] screen_main_label_saat NULL!");
    }*/
    
    String sys_day = SystemVariables::getSystemVariable(SystemVariables::SYSDAY_0);
    String sys_month = SystemVariables::getSystemVariable(SystemVariables::SYSMONTH_0);
    String sys_year = SystemVariables::getSystemVariable(SystemVariables::SYSYEAR);
    
    String sys_date = sys_day + "/" + sys_month + "/" + sys_year;
    if (gui->screen_main_label_tarih) {
      lv_label_set_text(gui->screen_main_label_tarih, sys_date.c_str());
      //Serial.printf("[DEBUG] Tarih label güncellendi: %s\n", sys_date.c_str());
    } /*else {
      Serial.println("[DEBUG] screen_main_label_tarih NULL!");
    }*/
    
    // 2. WIFI DURUM VE SINYAL GÜCÜ GÜNCELLEMESİ
    String wifi_status = SystemVariables::getSystemVariable(SystemVariables::ISWIFI);
    String wifi_rssi = SystemVariables::getSystemVariable(SystemVariables::RSSI);
    String sys_ip = SystemVariables::getSystemVariable(SystemVariables::IP);
    
    /*Serial.printf("[DEBUG] WiFi Status: %s, RSSI: %s, IP: %s\n", 
                  wifi_status.c_str(), wifi_rssi.c_str(), sys_ip.c_str());*/
    
    // Önce tüm WiFi ikonlarını gizle
    lv_obj_add_flag(gui->screen_main_img_wifi_lost_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_low1_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_low2_signal, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
    
    // 3. IP ADRESİ VE WIFI İKON DURUMU
    if (wifi_status.toInt() >= 2) {
        // WiFi bağlı ve IP aldı - gerçek IP göster
        if (gui->screen_main_label_ip_adres) {
          // Server durumu ile birlikte IP göster
          String ip_text = sys_ip;
          if (serverConnectionStatus) {
              ip_text += " [O]";  // Server bağlı
              lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0x00FF00), LV_PART_MAIN | LV_STATE_DEFAULT);  // Yeşil
          } else {
              ip_text += " [X]";  // Server bağlı değil  
              lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0xFF0000), LV_PART_MAIN | LV_STATE_DEFAULT);  // Kırmızı
          }
          lv_label_set_text(gui->screen_main_label_ip_adres, ip_text.c_str());
          //Serial.printf("[DEBUG] IP label güncellendi: %s\n", ip_text.c_str());
        }
        
        // RSSI değerine göre WiFi ikonunu göster
        int rssi_value = wifi_rssi.toInt();
        if (rssi_value >= -50) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
            //Serial.println("[DEBUG] WiFi full signal ikon gösteriliyor");
        } else if (rssi_value >= -65) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_HIDDEN);
            //Serial.println("[DEBUG] WiFi full signal ikon gösteriliyor");
        } else if (rssi_value >= -75) {
            lv_obj_clear_flag(gui->screen_main_img_wifi_low2_signal, LV_OBJ_FLAG_HIDDEN);
            //Serial.println("[DEBUG] WiFi low2 signal ikon gösteriliyor");
        } else {
            lv_obj_clear_flag(gui->screen_main_img_wifi_low1_signal, LV_OBJ_FLAG_HIDDEN);
            //Serial.println("[DEBUG] WiFi low1 signal ikon gösteriliyor");
        }
    } else {
        // WiFi bağlı değil - AP mode IP göster
        if (gui->screen_main_label_ip_adres) {
          lv_label_set_text(gui->screen_main_label_ip_adres, "192.168.4.1");
          lv_obj_set_style_text_color(gui->screen_main_label_ip_adres, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);  // Beyaz
          //Serial.println("[DEBUG] AP mode IP güncellendi: 192.168.4.1");
        }
        lv_obj_clear_flag(gui->screen_main_img_wifi_lost_signal, LV_OBJ_FLAG_HIDDEN);
        //Serial.println("[DEBUG] WiFi lost signal ikon gösteriliyor");
    }
    
    //Serial.println("[DEBUG] updateSystemInfoWithGui() tamamlandı");
}

// Ortak ekran güncelleme fonksiyonu (tarih, saat, wifi, ip) - eski versiyon
void updateSystemInfo() {
    updateSystemInfoWithGui(&guider_ui);
}

void updateLVGLScreen(const String& cardId, const String& userName, bool success) {
  if (gui_mutex && xSemaphoreTake(gui_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
    
    // Sistem bilgilerini güncelle (tarih, saat, wifi, ip)
    updateSystemInfo();
    
    // PERSONEL ID LABEL GÜNCELLEMESİ
    if (guider_ui.screen_main_label_personel_id) {
      if (success && userName.length() > 0) {
        lv_label_set_text(guider_ui.screen_main_label_personel_id, userName.c_str());
        last_card_display_time = millis();
        last_user_name = userName;
        displaying_card_info = true;
        
        //Serial.printf("[LVGL] Ekranda gösteriliyor: %s\n", userName.c_str());
      } else {
        lv_label_set_text(guider_ui.screen_main_label_personel_id, "Bilinmeyen Kart");
        last_card_display_time = millis();
        last_user_name = "Bilinmeyen Kart";
        displaying_card_info = true;
      }
    }
    
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
    
    // Her 3 saniyede bir server bağlantısını test et (timer_counter % 3 == 0)
    static int last_server_test = 0;
    if (timer_counter != last_server_test && timer_counter % 3 == 0) {
        last_server_test = timer_counter;
        
        // Gerçek PDKS API /health endpoint kontrolü
        checkServerHealth();
    }
}

// PDKS API server sağlık kontrolü fonksiyonu
void checkServerHealth() {
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