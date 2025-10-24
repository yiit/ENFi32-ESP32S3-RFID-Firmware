/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *screen_logo;
	bool screen_logo_del;
	lv_obj_t *screen_logo_img_grandstarr_logo;
	lv_obj_t *screen_logo_label_version;
	lv_obj_t *screen_logo_bar_loading;
	lv_obj_t *screen_main;
	bool screen_main_del;
	lv_obj_t *screen_main_label_ust_cubuk;
	lv_obj_t *screen_main_label_saat;
	lv_obj_t *screen_main_label_tarih;
	lv_obj_t *screen_main_label_kart_okutun;
	lv_obj_t *screen_main_label_personel_id;
	lv_obj_t *screen_main_img_wifi_lost_signal;
	lv_obj_t *screen_main_label_ip_adres;
	lv_obj_t *screen_main_img_wifi_low1_signal;
	lv_obj_t *screen_main_img_wifi_low2_signal;
	lv_obj_t *screen_main_img_wifi_full_signal;
	lv_obj_t *screen_main_img_server_connected;
	lv_obj_t *screen_main_img_server_disconnected;
	lv_obj_t *screen_main_img_ready;
	lv_obj_t *screen_main_img_saved;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_screen_logo(lv_ui *ui);
void setup_scr_screen_main(lv_ui *ui);
LV_IMAGE_DECLARE(_GRANDSTARRlogo_RGB565A8_420x90);
LV_IMAGE_DECLARE(_wifi_lost_signal_RGB565A8_35x35);
LV_IMAGE_DECLARE(_wifi_low1_RGB565A8_35x35);
LV_IMAGE_DECLARE(_wifi_low2_signal_RGB565A8_35x35);
LV_IMAGE_DECLARE(_wifi_full_signal_RGB565A8_35x35);
LV_IMAGE_DECLARE(_ready_RGB565A8_100x100);
LV_IMAGE_DECLARE(_saved_RGB565A8_96x62);

LV_FONT_DECLARE(lv_font_OswaldRegular_16)
LV_FONT_DECLARE(lv_font_montserratMedium_16)
LV_FONT_DECLARE(lv_font_OswaldRegular_40)
LV_FONT_DECLARE(lv_font_OswaldRegular_30)
LV_FONT_DECLARE(lv_font_Oswald_Reg_20)


#ifdef __cplusplus
}
#endif
#endif
