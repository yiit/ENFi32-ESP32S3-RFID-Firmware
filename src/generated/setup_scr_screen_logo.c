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
#include "custom/custom.h"


void setup_scr_screen_logo(lv_ui *ui)
{
    //Write codes screen_logo
    ui->screen_logo = lv_obj_create(NULL);
    lv_obj_set_size(ui->screen_logo, 480, 320);
    lv_obj_set_scrollbar_mode(ui->screen_logo, LV_SCROLLBAR_MODE_OFF);

    //Write style for screen_logo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_logo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_logo, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_logo, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_logo_img_grandstarr_logo
    ui->screen_logo_img_grandstarr_logo = lv_image_create(ui->screen_logo);
    lv_obj_set_pos(ui->screen_logo_img_grandstarr_logo, 30, 100);
    lv_obj_set_size(ui->screen_logo_img_grandstarr_logo, 420, 90);
    lv_obj_add_flag(ui->screen_logo_img_grandstarr_logo, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->screen_logo_img_grandstarr_logo, &_GRANDSTARRlogo_RGB565A8_420x90);
    lv_image_set_pivot(ui->screen_logo_img_grandstarr_logo, 50,50);
    lv_image_set_rotation(ui->screen_logo_img_grandstarr_logo, 0);

    //Write style for screen_logo_img_grandstarr_logo, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->screen_logo_img_grandstarr_logo, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->screen_logo_img_grandstarr_logo, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_logo_label_version
    ui->screen_logo_label_version = lv_label_create(ui->screen_logo);
    lv_obj_set_pos(ui->screen_logo_label_version, 360, 285);
    lv_obj_set_size(ui->screen_logo_label_version, 100, 20);
    lv_label_set_text(ui->screen_logo_label_version, "Version : 1.0");
    lv_label_set_long_mode(ui->screen_logo_label_version, LV_LABEL_LONG_WRAP);

    //Write style for screen_logo_label_version, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->screen_logo_label_version, lv_color_hex(0x0c448d), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->screen_logo_label_version, &lv_font_OswaldRegular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->screen_logo_label_version, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->screen_logo_label_version, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_logo_label_version, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes screen_logo_bar_loading
    ui->screen_logo_bar_loading = lv_bar_create(ui->screen_logo);
    lv_obj_set_pos(ui->screen_logo_bar_loading, 30, 239);
    lv_obj_set_size(ui->screen_logo_bar_loading, 420, 14);
    lv_obj_set_style_anim_duration(ui->screen_logo_bar_loading, 1000, 0);
    lv_bar_set_mode(ui->screen_logo_bar_loading, LV_BAR_MODE_SYMMETRICAL);
    lv_bar_set_range(ui->screen_logo_bar_loading, 0, 100);
    lv_bar_set_value(ui->screen_logo_bar_loading, 0, LV_ANIM_OFF);

    //Write style for screen_logo_bar_loading, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_logo_bar_loading, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_logo_bar_loading, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_logo_bar_loading, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_logo_bar_loading, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->screen_logo_bar_loading, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for screen_logo_bar_loading, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->screen_logo_bar_loading, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->screen_logo_bar_loading, lv_color_hex(0x2195f6), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->screen_logo_bar_loading, LV_GRAD_DIR_NONE, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->screen_logo_bar_loading, 10, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //The custom code of screen_logo.


    //Update current screen layout.
    lv_obj_update_layout(ui->screen_logo);

    //Init events for screen.
    events_init_screen_logo(ui);
}
