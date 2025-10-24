/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_screen_main(lv_ui *ui)
{
    //Write codes screen_main
    ui->screen_main = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_main, 480, 320);
    lv_obj_set_scrollbar_mode(ui->screen_main, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_main, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_main, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_ust_cubuk
    ui->screen_main_label_ust_cubuk = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_ust_cubuk, 0, 0);
    lv_obj_set_size(ui->screen_main_label_ust_cubuk, 480, 45);
    lv_label_set_text(ui->screen_main_label_ust_cubuk, "");
    lv_label_set_long_mode(ui->screen_main_label_ust_cubuk, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_ust_cubuk, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_ust_cubuk, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_ust_cubuk, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_ust_cubuk, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_ust_cubuk, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_ust_cubuk, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_main_label_ust_cubuk, lv_color_hex(0x0c448d), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_main_label_ust_cubuk, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_ust_cubuk, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_saat
    ui->screen_main_label_saat = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_saat, 280, 15);
    lv_obj_set_size(ui->screen_main_label_saat, 100, 20);
    lv_label_set_text(ui->screen_main_label_saat, "00:00:00");
    lv_label_set_long_mode(ui->screen_main_label_saat, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_saat, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_saat, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_saat, &lv_font_Oswald_Reg_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_saat, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_saat, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_saat, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_tarih
    ui->screen_main_label_tarih = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_tarih, 380, 15);
    lv_obj_set_size(ui->screen_main_label_tarih, 100, 20);
    lv_label_set_text(ui->screen_main_label_tarih, "01-01-2025\n");
    lv_label_set_long_mode(ui->screen_main_label_tarih, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_tarih, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_tarih, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_tarih, &lv_font_Oswald_Reg_20, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_tarih, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_tarih, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_tarih, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_kart_okutun
    ui->screen_main_label_kart_okutun = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_kart_okutun, 40, 216);
    lv_obj_set_size(ui->screen_main_label_kart_okutun, 400, 45);
    lv_label_set_text(ui->screen_main_label_kart_okutun, "KARTINIZI OKUTUNUZ");
    lv_label_set_long_mode(ui->screen_main_label_kart_okutun, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_kart_okutun, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_kart_okutun, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_kart_okutun, &lv_font_OswaldRegular_40, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_kart_okutun, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_kart_okutun, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_kart_okutun, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_personel_id
    ui->screen_main_label_personel_id = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_personel_id, 0, 55);
    lv_obj_set_size(ui->screen_main_label_personel_id, 480, 40);
    lv_label_set_text(ui->screen_main_label_personel_id, "");
    lv_label_set_long_mode(ui->screen_main_label_personel_id, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_personel_id, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_personel_id, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_personel_id, &lv_font_OswaldRegular_30, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_personel_id, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_personel_id, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_personel_id, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_wifi_lost_signal
    ui->screen_main_img_wifi_lost_signal = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_wifi_lost_signal, 5, 5);
    lv_obj_set_size(ui->screen_main_img_wifi_lost_signal, 35, 35);
    lv_obj_add_flag(ui->screen_main_img_wifi_lost_signal, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_wifi_lost_signal, &_wifi_lost_signal_RGB565A8_35x35);
    lv_image_set_pivot(ui->screen_main_img_wifi_lost_signal, 50,50);
    lv_image_set_rotation(ui->screen_main_img_wifi_lost_signal, 0);

    //Write style for screen_main_img_wifi_lost_signal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_wifi_lost_signal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_wifi_lost_signal, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_label_ip_adres
    ui->screen_main_label_ip_adres = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_label_ip_adres, 140, 15);
    lv_obj_set_size(ui->screen_main_label_ip_adres, 105, 20);
    lv_label_set_text(ui->screen_main_label_ip_adres, "IP ADRESi");
    lv_label_set_long_mode(ui->screen_main_label_ip_adres, LV_LABEL_LONG_WRAP);

    //Write style for screen_main_label_ip_adres, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_label_ip_adres, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_label_ip_adres, &lv_font_OswaldRegular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_label_ip_adres, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_label_ip_adres, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_label_ip_adres, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_wifi_low1_signal
    ui->screen_main_img_wifi_low1_signal = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_wifi_low1_signal, 5, 5);
    lv_obj_set_size(ui->screen_main_img_wifi_low1_signal, 35, 35);
    lv_obj_add_flag(ui->screen_main_img_wifi_low1_signal, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_wifi_low1_signal, &_wifi_low1_RGB565A8_35x35);
    lv_image_set_pivot(ui->screen_main_img_wifi_low1_signal, 50,50);
    lv_image_set_rotation(ui->screen_main_img_wifi_low1_signal, 0);

    //Write style for screen_main_img_wifi_low1_signal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_wifi_low1_signal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_wifi_low1_signal, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_wifi_low2_signal
    ui->screen_main_img_wifi_low2_signal = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_wifi_low2_signal, 5, 5);
    lv_obj_set_size(ui->screen_main_img_wifi_low2_signal, 35, 35);
    lv_obj_add_flag(ui->screen_main_img_wifi_low2_signal, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_wifi_low2_signal, &_wifi_low2_signal_RGB565A8_35x35);
    lv_image_set_pivot(ui->screen_main_img_wifi_low2_signal, 50,50);
    lv_image_set_rotation(ui->screen_main_img_wifi_low2_signal, 0);

    //Write style for screen_main_img_wifi_low2_signal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_wifi_low2_signal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_wifi_low2_signal, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_wifi_full_signal
    ui->screen_main_img_wifi_full_signal = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_wifi_full_signal, 5, 5);
    lv_obj_set_size(ui->screen_main_img_wifi_full_signal, 35, 35);
    lv_obj_add_flag(ui->screen_main_img_wifi_full_signal, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_main_img_wifi_full_signal, &_wifi_full_signal_RGB565A8_35x35);
    lv_image_set_pivot(ui->screen_main_img_wifi_full_signal, 50,50);
    lv_image_set_rotation(ui->screen_main_img_wifi_full_signal, 0);

    //Write style for screen_main_img_wifi_full_signal, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_main_img_wifi_full_signal, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_main_img_wifi_full_signal, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_server_connected
    ui->screen_main_img_server_connected = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_server_connected, 45, 5);
    lv_obj_set_size(ui->screen_main_img_server_connected, 20, 20);
    lv_label_set_text(ui->screen_main_img_server_connected, "🌐");
    lv_obj_add_flag(ui->screen_main_img_server_connected, LV_OBJ_FLAG_HIDDEN);
    
    //Write style for screen_main_img_server_connected, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_img_server_connected, lv_color_hex(0x00ff00), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_img_server_connected, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_img_server_connected, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_img_server_connected, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_img_server_connected, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_img_server_connected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_server_disconnected
    ui->screen_main_img_server_disconnected = lv_label_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_server_disconnected, 45, 5);
    lv_obj_set_size(ui->screen_main_img_server_disconnected, 20, 20);
    lv_label_set_text(ui->screen_main_img_server_disconnected, "❌");
    lv_obj_add_flag(ui->screen_main_img_server_disconnected, LV_OBJ_FLAG_HIDDEN);
    
    //Write style for screen_main_img_server_disconnected, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_main_img_server_disconnected, lv_color_hex(0xff0000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_main_img_server_disconnected, &lv_font_montserratMedium_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_main_img_server_disconnected, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_main_img_server_disconnected, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_main_img_server_disconnected, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_main_img_server_disconnected, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_saved
    ui->screen_main_img_saved = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_saved, 10, 230);  // Sol alt konumda (x=10, y=230)
    lv_obj_set_size(ui->screen_main_img_saved, 96, 62);
    lv_image_set_src(ui->screen_main_img_saved, &_saved_RGB565A8_96x62);
    lv_obj_add_flag(ui->screen_main_img_saved, LV_OBJ_FLAG_HIDDEN);  // Başlangıçta gizli
    
    //Write style for screen_main_img_saved, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_main_img_saved, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_saved, true, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_main_img_ready
    ui->screen_main_img_ready = lv_image_create(ui->screen_main);
    lv_obj_set_pos(ui->screen_main_img_ready, 190, 110);  // Orta konumda (480/2 - 100/2 = 190, 320/2 - 100/2 = 110)
    lv_obj_set_size(ui->screen_main_img_ready, 100, 100);
    lv_image_set_src(ui->screen_main_img_ready, &_ready_RGB565A8_100x100);
    lv_obj_add_flag(ui->screen_main_img_ready, LV_OBJ_FLAG_HIDDEN);  // Başlangıçta gizli
    
    //Write style for screen_main_img_ready, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->screen_main_img_ready, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_clip_corner(ui->screen_main_img_ready, true, LV_PART_MAIN|LV_STATE_DEFAULT);
    //The custom code of screen_main.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_main);

}
