/*
* Copyright 2025 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#if LV_USE_GUIDER_SIMULATOR && LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "custom/custom.h"

static void screen_logo_event_handler (lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    switch (code) {
    case LV_EVENT_SCREEN_LOAD_START:
    {
        lv_obj_set_y(guider_ui.screen_logo_bar_loading, 230);
        ui_animation(guider_ui.screen_logo_bar_loading, 5, 5, 0, 100, &lv_anim_path_linear, 0, 0, 0, 0, (lv_anim_exec_xcb_t)loader_anim_cb, NULL, (lv_anim_completed_cb_t)loader_anim_complete_cb, NULL);
        break;
    }
    case LV_EVENT_SCREEN_UNLOAD_START:
    {
        lv_obj_set_y(guider_ui.screen_logo_bar_loading, 230);
        break;
    }
    default:
        break;
    }
}

void events_init_screen_logo (lv_ui *ui)
{
    lv_obj_add_event_cb(ui->screen_logo, screen_logo_event_handler, LV_EVENT_ALL, ui);
}


void events_init(lv_ui *ui)
{

}
