#ifndef USB_HID_RFID_H
#define USB_HID_RFID_H

#include <Arduino.h>

extern "C" {
#include "usb/usb_host.h"
}

#include "hid_host.h"
#include "hid_usage_keyboard.h"

// USB HID RFID fonksiyonları
void initUSBHIDRFID();                    // Public wrapper - 5 saniye delay ile başlatır
void initUSBHIDRFIDDelayed(uint32_t delay_ms); // Özelleştirilebilir delay
void initUSBHIDRFIDNow();                 // Hemen başlatır (eski initUSBHIDRFID)
void cleanupUSBHIDRFID();

// PDKS API fonksiyonları
void sendCardDataToPDKS(const String& cardId);
void sendCardDataWithC11Task(void* parameter);
void getUserNameAndUpdateScreen(const String& cardId, const String& apiResponse, bool success);
void captureAPIResponse(const String& response); // C011 Controller response capture

// Local storage fonksiyonları (offline support)
void saveRFIDDataToLocal(const String& cardId);
bool isServerConnected();
void processLocalStorageQueue();
bool sendQueuedDataToAPI(const String& cardId, const String& deviceIP, const String& timestamp, const String& deviceId);

// API Connectivity fonksiyonları
bool checkAPIConnectivity();
bool getAPIConnectionStatus();                // TCP bağlantı durumunu döndürür
void checkAPIConnectivityPeriodic();
void handleUSBHIDRFIDLoop();
void checkUSBHealthPeriodic();

// HID State struct
struct HidState {
  hid_host_device_handle_t dev = nullptr;
  bool opened  = false;
  bool started = false;
};

extern HidState g_hid;

#endif // USB_HID_RFID_H