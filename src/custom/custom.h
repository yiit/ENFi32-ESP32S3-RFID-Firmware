/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "generated/gui_guider.h"

void custom_init(lv_ui *ui);

void slider_adjust_img_cb(lv_obj_t * img, int32_t brightValue, int16_t hueValue);

void loader_anim_cb(lv_obj_t* var, uint32_t value);

void loader_anim_complete_cb(lv_obj_t* var);

void data_update_timer_cb(lv_timer_t* timer);

void healthCheckTask(void* parameter);

void ready_image_timer_cb(lv_timer_t* timer);

void showReadyImage();

void saved_image_timer_cb(lv_timer_t* timer);

void showSavedImage();

#ifdef __cplusplus
}

// C++ fonksiyonları (Arduino String kullanır)
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <map>
void updateLVGLScreen(const String& cardId, const String& userName, bool success);
void updateSystemInfoWithGui(lv_ui* gui);
void updateSystemInfo();
String getUserNameFromAPISync(const String& cardId);
#endif

#endif /* EVENT_CB_H_ */
