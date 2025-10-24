#ifndef ESPEASY_GLOBALS_H_
#define ESPEASY_GLOBALS_H_



#include "ESPEasy_common.h"


//#include <FS.h>



//enable reporting status to ESPEasy developers.
//this informs us of crashes and stability issues.
// not finished yet!
// #define FEATURE_REPORTING  1

//Select which plugin sets you want to build.
//These are normally automaticly set via the Platformio build environment.
//If you use ArduinoIDE you might need to uncomment some of them, depending on your needs
//If you dont select any, a version with a minimal number of plugins will be biult for 512k versions.
//(512k is NOT finsihed or tested yet as of v2.0.0-dev6)


//build all plugins that still are being developed and are broken or incomplete
//#define PLUGIN_BUILD_DEV

//add this if you want SD support (add 10k flash)
//#define FEATURE_SD 1







/*
// TODO TD-er: Declare global variables as extern and construct them in the .cpp.
// Move all other defines in this file to separate .h files
// This file should only have the "extern" declared global variables so it can be included where they are needed.
//
// For a very good tutorial on how C++ handles global variables, see:
//    https://www.fluentcpp.com/2019/07/23/how-to-define-a-global-constant-in-cpp/
// For more information about the discussion which lead to this big change:
//    https://github.com/letscontrolit/ESPEasy/issues/2621#issuecomment-533673956
*/





/*********************************************************************************************\
 * pinStatesStruct
\*********************************************************************************************/
/*
struct pinStatesStruct
{
  pinStatesStruct() : value(0), plugin(0), index(0), mode(0) {}
  uint16_t value;
  uint8_t plugin;
  uint8_t index;
  uint8_t mode;
} pinStates[PINSTATE_TABLE_MAX];
*/




extern boolean printToWeb;
extern String printWebString;
extern boolean printToWebJSON;


//struct RTC_cache_handler_struct;


// FIXME TD-er: Must move this to some proper class (ESPEasy_Scheduler ?)
extern unsigned long timermqtt_interval;


extern unsigned long lastSend;
extern unsigned long lastWeb;

extern unsigned long wdcounter;
extern unsigned long timerAwakeFromDeepSleep;


#if FEATURE_ADC_VCC
extern float vcc;
#endif



extern bool shouldReboot;
extern bool firstLoop;

// This is read from the settings at boot.
// Even if this setting is changed, you need to reboot to activate the changes.
extern boolean UseRTOSMultitasking;

/*#include "HX711.h"
extern HX711 scale;*/

#include <MD5Builder.h>
extern MD5Builder _md5;

extern bool WebLisansIn;
extern int WebLisansInTimer;
extern int WebLisanseyzInTimer;

extern unsigned long randomNumber;
extern double timeunix;

extern  char kfont1[4];
extern  char kfont2[7];
extern  char kfont3[4];
extern  char kfont4[7];
extern  char kfont5[4];
extern  char kfont6[4];

extern  char font1[7];
extern  char font2[7];
extern  char font3[7];
extern  char font4[7];
extern  char font5[7];
extern  char font6[7];
extern  char sol[4];
extern  char orta[4];
extern  char sag[4];
extern  char beyaz[4];
extern  char siyah[4];
extern  char acik[4];
extern  char koyu[4];
extern  char kes[4];
extern  char logo[16];
extern  char cekmece[7];

extern  char hata_beep[5];
extern  char okey_beep[5];

extern  char qrkodbas[22];
//extern  extern  char qrkodbas[24]  = {27, 64, 10,  29, 40, 107, 48, 103,  7, 29, 40, 107, 48, 105, 72, 29, 40, 107, 48, 128, 254, 94};
extern  char qrkodson[10];
extern  char CR[2];
extern  char LF[2];
extern  char etiketcal[9];

extern String tartimString_s;
extern String XML_TARIH_S;
extern String XML_SAAT_S;
extern String XML_TARIH_V;
extern String XML_SAAT_V;
extern String XML_NET_S;
extern String XML_NET_V;
extern String XML_DARA_S;
extern String XML_DARA_V;
extern String XML_BRUT_S;
extern String XML_BRUT_V;
extern String XML_NET_S_2;
extern String XML_DARA_S_2;
extern String XML_BRUT_S_2;
extern String XML_ADET_S;
extern String XML_ADET_GRAMAJ_S;
extern String XML_URUN_NO_S;
extern String XML_URUN_ISMI_S;
extern String XML_URUN_KOD_S;
extern String XML_BARKOD_S;
extern String XML_BIRIM_FIYAT_S;
extern String XML_TUTAR_S;
extern String XML_SNO_S;
extern String XML_DURUS_ZAMANI_S;

extern String XML_SAYAC_1_S;
extern String XML_SAYAC_1_SONSUZ_S;
extern String XML_SAYAC_1_GECIKME_S;
extern String XML_SAYAC_2_S;
extern String XML_SAYAC_2_SONSUZ_S;
extern String XML_SAYAC_2_GECIKME_S;
extern String XML_SAYAC_3_S;
extern String XML_SAYAC_3_SONSUZ_S;
extern String XML_SAYAC_3_GECIKME_S;
extern String XML_SAYAC_4_S;
extern String XML_SAYAC_4_SONSUZ_S;
extern String XML_SAYAC_4_GECIKME_S;
extern String XML_SICAKLIK_S;

extern String XML_SERI_NO_S;
extern String XML_FIS_NO_S;
extern String XML_TOP_NET_S;
extern String XML_TOP_DARA_S;
extern String XML_TOP_BRUT_S;
extern String XML_TOP_ADET_S;
extern String XML_STABIL_S;
extern String XML_FORMUL1_S;
extern String XML_FORMUL2_S;
extern String XML_FORMUL1_KATSAYI_S;
extern String XML_FORMUL2_KATSAYI_S;
extern String XML_RFIDKOD_S;
extern String XML_FIS_BASLIK1_S;
extern String XML_FIS_BASLIK2_S;
extern String XML_FIS_BASLIK3_S;
extern String XML_FIS_BASLIK4_S;
extern String XML_EAN8_S;
extern String XML_EAN13_S;
extern String XML_QRKOD_S;

extern String XML_V0;
extern String XML_V1;
extern String XML_V2;
extern String XML_V3;
extern String XML_V4;
extern String XML_V5;
extern String XML_V6;
extern String XML_V7;
extern String XML_V8;
extern String XML_V9;
extern String XML_V10;
extern String XML_V11;
extern String XML_V12;
extern String XML_V13;
extern String XML_V14;
extern String XML_V15;
extern String XML_V16;
extern String XML_V17;
extern String XML_V18;
extern String XML_V19;
extern String XML_V20;
extern String XML_V21;
extern String XML_V22;
extern String XML_V23;
extern String XML_V24;
extern String XML_V25;
extern String XML_V26;
extern String XML_V27;
extern String XML_V28;
extern String XML_V29;

extern String XML_FIRMA_ISMI_S;
extern String XML_MUSTERI_ISMI_S;
extern String XML_PLAKA_NO_S;
extern String XML_OPERATOR_ISMI_S;

extern String XML_MESAJ1_S;
extern String XML_MESAJ2_S;
extern String XML_MESAJ3_S;
extern String XML_MESAJ4_S;
extern String XML_MESAJ5_S;
extern String XML_MESAJ6_S;
extern String XML_MESAJ7_S;
extern String XML_MESAJ8_S;
extern String XML_MESAJ9_S;

extern String kopya_etiket;

//extern String XML_DATA[30];

extern char XML_NET_C[9];
extern char XML_DARA_C[9];
extern char XML_BRUT_C[9];
extern char XML_NET_C_2[9];
extern char XML_DARA_C_2[9];
extern char XML_BRUT_C_2[9];
extern char XML_ADET_C[9];
extern char XML_ADET_GRAMAJ_C[9];
extern char XML_TOP_NET_C[11];
extern char XML_TOP_DARA_C[11];
extern char XML_TOP_BRUT_C[11];
extern char XML_SNO_C[4];
extern char XML_SAYAC_1_C[7];
extern char XML_SAYAC_2_C[7];
extern char XML_SAYAC_3_C[7];
extern char XML_SAYAC_4_C[7];
extern char XML_SERI_NO_C[9];
extern char XML_SIRA_NO_C[9];
extern char XML_FIS_NO_C[9];
extern char XML_BARKOD_C[6];
extern char XML_FORMUL1_C[7];
extern char XML_FORMUL2_C[7];
extern char XML_FORMUL1_KATSAYI_C[9];
extern char XML_FORMUL2_KATSAYI_C[9];
extern char yazdir_c[2048];
extern char yazdir_xml[2048];
extern char XML_INPUT_PIN_C[4];
extern char XML_OUTPUT_PIN_C[4];
extern char XML_webapinettartim_son_C[9];
extern char XML_webapinettartim_C[9];

extern float isaret_f;
extern float isaret_f_2;
extern float webapinettartim;
extern float webapinettartim_son;
extern float webapidaratartim;
extern float webapibruttartim;
extern float webapinettartim_2;
extern float webapidaratartim_2;
extern float webapibruttartim_2;
extern float webapiadet;
extern float webapiadetgr;
extern int webapiurunno;
extern float webapibfiyat;
extern float webapitutar;
extern float StabilTartim_f;
extern float webapikatsayi1;
extern float webapikatsayi2;

extern float ind_sifir_degeri;
extern float ind_dara_degeri;
extern float eyz_dara_degeri;

extern String IRDA_DATA_S;

extern unsigned long hataTimer_l;
extern unsigned long hataTimer_espnow_l;
extern unsigned long gosterTimer_l;
extern unsigned long stabilTimer_l;
extern unsigned long button_basildi;

extern uint32_t sno;
extern int bluetooth_mod;
extern float top_net;
extern float top_dara;
extern float top_brut;
extern long fis_no;
extern uint32_t seri_no;

extern String message0;
extern String message1;
extern String message2;
extern String message3;
extern String message4;
extern String message5;
extern String message6;
extern String message7;
extern String message8;
extern String message9;
extern String data_s;
extern String tartimdata_s;
extern String paketVeri_s;

extern String toplam_detay;

#define MAX_M1_ENTRIES 30  // Kaç adet ürün olduğunu bilmiyorsak, makul bir sınır belirleyelim
#define MAX_LINE_LENGTH 32  // Her satır için maksimum uzunluk

/*extern String urunadi[129];
extern String urunkod[129];
extern String urunnet[129];
extern String urunadet[129];
//extern String M1[30];
extern char M1[MAX_M1_ENTRIES][MAX_LINE_LENGTH];  // M1 dizisini statik olarak tanımlıyoruz
extern char M2[MAX_M1_ENTRIES][MAX_LINE_LENGTH];  // M1 dizisini statik olarak tanımlıyoruz
extern char M3[MAX_M1_ENTRIES][MAX_LINE_LENGTH];  // M1 dizisini statik olarak tanımlıyoruz
extern char M4[MAX_M1_ENTRIES][MAX_LINE_LENGTH];  // M1 dizisini statik olarak tanımlıyoruz*/

extern String stabilTartim_s;

extern int artyaz;
extern int topyaz;
extern int hayvan_modu;
extern uint8_t topla_i;

extern bool oto_yazdir;
extern bool escpos_mod;
extern unsigned long escpos_time;
extern bool tspl_mod;
extern unsigned long tspl_time;
extern bool WifiAPMode;

extern String options2[10];

extern bool karakter_195;
extern bool karakter_196;
extern bool karakter_197;

extern bool Client_Mod;

extern bool sd_aktif;

extern unsigned long lastTickMillis;

/***************LVGL***************/
#ifndef LGFX_H
#define LGFX_H
//#include <LovyanGFX.hpp>
#define LGFX_USE_V1
#endif
#include <lvgl.h>

#include "PanelLan.h"
extern PanelLan tft;

/*#include "gfx_conf.h"
extern LGFX tft;*/

// Ekran çözünürlüğü

#define TFT_ROTATION  LV_DISPLAY_ROTATION_1

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
//#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (16 / 8))
//#define DRAW_BUF_SIZE  (TFT_HOR_RES * 20)  // 20 satır yerine daha büyük bir değer varsa düşürebilirsin.
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10)
extern lv_color_t *draw_buf_1;
extern lv_color_t *draw_buf_2;
//***************LVGL***************/

extern bool bul_buton_aktif;
extern bool edit_buton_aktif;
extern bool kaydet_buton_aktif;

extern bool net_label;

#define MAKTIF_BUTTON_COUNT 8

extern bool M_buton_aktif[MAKTIF_BUTTON_COUNT]; 

extern uint8_t ADDR_M_BUTON_AKTIF[];

#define SEC_M_BUTON_COUNT 8
#define SEC_F_BUTON_COUNT 8

extern uint8_t sec_M_buton[SEC_M_BUTON_COUNT];
extern uint8_t sec_F_buton[SEC_F_BUTON_COUNT];

extern uint8_t ADDR_SEC_M_BUTON[];
extern uint8_t ADDR_SEC_F_BUTON[];

extern uint8_t sec_URUN_buton;

// 📌 Seçili buton bilgilerini saklayan değişkenler
extern uint8_t selected_m_btn_no;
extern uint8_t selected_i;

extern uint8_t m_btn_no;
extern uint8_t f_btn_no;

extern char mesaj_labels[8][31][33]; // Buton isimlerini saklayan dizi
extern char urun_labels[101][49];
extern char urun_kod_labels[101][14];
extern uint8_t urun_tipi[101];
extern uint16_t urun_skt[101];
extern uint8_t urun_etiket_format[101];
extern float urun_dara[101];
extern float urun_birim_gramaj[101];
extern int8_t urun_barkod_format[101];
extern char urun_msg[101][49];

extern float urun_net[255];
extern uint8_t urun_adet[255];

void beep(uint16_t freq, uint32_t duration);

#include <EEPROM.h>

#define EEPROM_SIZE 32  // EEPROM toplam boyutu
#define EEPROM_START_ADDR 200 // EEPROM başlangıç adresi

// 📌 EEPROM adreslerini tanımlamak için ENUM
enum EEPROM_Address : uint8_t {
    ADDR_SEC_MBUTON_NO = EEPROM_START_ADDR, // 0. byte
    ADDR_M1_BUTON_AKTIF,  // 1. byte
    ADDR_M2_BUTON_AKTIF,  // 2. byte
    ADDR_M3_BUTON_AKTIF,  // 3. byte
    ADDR_M4_BUTON_AKTIF,  // 4. byte
    ADDR_M5_BUTON_AKTIF,  // 5. byte
    ADDR_M6_BUTON_AKTIF,  // 6. byte
    ADDR_M7_BUTON_AKTIF,  // 7. byte
    ADDR_M8_BUTON_AKTIF,  // 8. byte
    ADDR_SEC_M1_BUTON,    // 9. byte
    ADDR_SEC_M2_BUTON,    // 10. byte
    ADDR_SEC_M3_BUTON,    // 11. byte
    ADDR_SEC_M4_BUTON,    // 12. byte
    ADDR_SEC_M5_BUTON,    // 13. byte
    ADDR_SEC_M6_BUTON,    // 14. byte
    ADDR_SEC_M7_BUTON,    // 15. byte
    ADDR_SEC_M8_BUTON,    // 16. byte
    ADDR_SEC_F1_BUTON,    // 17. byte
    ADDR_SEC_F2_BUTON,    // 18. byte
    ADDR_SEC_F3_BUTON,    // 19. byte
    ADDR_SEC_F4_BUTON,    // 20. byte
    ADDR_SEC_F5_BUTON,    // 21. byte
    ADDR_SEC_F6_BUTON,    // 22. byte
    ADDR_SEC_F7_BUTON,    // 23. byte
    ADDR_SEC_F8_BUTON,    // 24. byte
    ADDR_SEC_URUN_BUTON,  // 25. byte
    ADDR_NET_LABEL,
  // IMPORTANT: ADDR_MAX must be 1 past the last valid address. Previous '+ 26' made ADDR_NET_LABEL invalid (>= ADDR_MAX)
  ADDR_MAX = EEPROM_START_ADDR + 27 // 27 byte ayrıldı (0..26 offset kullanılabilir)
};

void writeEEPROM(EEPROM_Address addr, uint8_t value);
uint8_t readEEPROM(EEPROM_Address addr);

extern char json_temp_buf[64]; // Maks 64 karakterlik geçici kullanım


#ifdef ESP_NOW_ACTIVE
#include <esp_now.h>
#include <Preferences.h>

extern uint8_t espnow_led;

#define espnow_hata "Eslesmis cihaz yok, veri gonderilemiyor."
#define MAX_PAIRED_DEVICES 6

// Degiskenler
extern Preferences preferences;
extern uint8_t pairedMacList[MAX_PAIRED_DEVICES][6];  // Eslesmis cihazin MAC adresi
extern int pairedDeviceCount; // Eslesmis cihaz sayısını tutar

struct PeerStatus {
  uint8_t mac[6];
  bool active;
};

extern PeerStatus peerStatusList[MAX_PAIRED_DEVICES];
extern int peerStatusCount;

// Tarama sonucu geçici cihaz listesi
extern uint8_t discoveredMacList[MAX_PAIRED_DEVICES][6];
extern int discoveredCount;

// Broadcast adresini belirle
extern uint8_t broadcastAddress[];
extern esp_now_peer_info_t peerInfo;

extern unsigned long buttonPressStartTime; // Butona basılma baslangıc zamanı
extern bool buttonPressed;

extern bool isPaired; // Eslesme durumu

void StartPairing();
void EspnowSendKomut(const char *komut);
void EspnowSendData(String data);
#endif

#endif /* ESPEASY_GLOBALS_H_ */
