/*
 * uistatusbar.c - SDL statusbar.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Andreas Boose <viceteam@t-online.de>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"

#include <stdio.h>

#include "resources.h"
#include "types.h"
#include "ui.h"
#include "uiapi.h"
//#include "uimenu.h"
#include "uistatusbar.h"
#include "videoarch.h"

#include "libretro-core.h"

#include "joystick.h"

/* ----------------------------------------------------------------- */
/* static functions/variables */

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#define MAX_STATUSBAR_LEN           128
#define STATUSBAR_SPEED_POS         0
#define STATUSBAR_PAUSE_POS         4
#define STATUSBAR_DRIVE_POS         24
#define STATUSBAR_DRIVE8_TRACK_POS  26
#define STATUSBAR_DRIVE9_TRACK_POS  31
#define STATUSBAR_DRIVE10_TRACK_POS 36
#define STATUSBAR_DRIVE11_TRACK_POS 41
#define STATUSBAR_TAPE_POS          46
#define STATUSBAR_JOY_POS			0

static char statusbar_text[MAX_STATUSBAR_LEN] = "                                       ";


static int pitch;
static int draw_offset;



static int tape_counter = 0;
static int tape_enabled = 0;
static int tape_motor = 0;
static int tape_control = 0;

static void display_tape(void)
{
    int len;

    if (tape_enabled) {
        len = sprintf(&(statusbar_text[STATUSBAR_TAPE_POS]), "%c%03d%c", (tape_motor) ? '*' : ' ', tape_counter, " >f<R"[tape_control]);
    } else {
        len = sprintf(&(statusbar_text[STATUSBAR_TAPE_POS]), "     ");
    }
    statusbar_text[STATUSBAR_TAPE_POS + len] = ' ';

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}


static void display_joyport(void)
{
    int len;
    char tmpstr[25];
    
    sprintf(tmpstr, "j%d:%2d ", 1, joystick_value[1]);
    sprintf(tmpstr + strlen(tmpstr), "j%d:%2d ", 2, joystick_value[2]);
    sprintf(tmpstr + strlen(tmpstr), "j%d:%2d ", 3, joystick_value[3]);
    sprintf(tmpstr + strlen(tmpstr), "j%d:%2d", 4, joystick_value[4]);
    //Retro_Draw_string(&fake, x+200, y, tmpstr,16,1,1, color_f, color_b);

    len = sprintf(&(statusbar_text[STATUSBAR_JOY_POS]), tmpstr);
    statusbar_text[STATUSBAR_JOY_POS + len] = ' ';

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}



static int per = 0;
static int fps = 0;
static int warp = 0;
static int paused = 0;

static void display_speed(void)
{
    return;
    
    int len;
    char sep = paused ? ('P' | 0x80) : warp ? ('W' | 0x80) : '/';

    len = sprintf(&(statusbar_text[STATUSBAR_SPEED_POS]), "%3d%%%c%2dfps", per, sep, fps);
    statusbar_text[STATUSBAR_SPEED_POS + len] = ' ';

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}

/* ----------------------------------------------------------------- */
/* ui.h */

void ui_display_speed(float percent, float framerate, int warp_flag)
{
    per = (int)(percent + .5);
    if (per > 999) {
        per = 999;
    }

    fps = (int)(framerate + .5);
    if (fps > 99) {
        fps = 99;
    }

    warp = warp_flag;

    display_speed();
}

void ui_display_paused(int flag)
{
    paused = flag;

    display_speed();
}

/* ----------------------------------------------------------------- */
/* uiapi.h */

/* Display a mesage without interrupting emulation */
void ui_display_statustext(const char *text, int fade_out)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: \"%s\", %i\n", __func__, text, fade_out);
#endif
}

/* Drive related UI.  */
void ui_enable_drive_status(ui_drive_enable_t state, int *drive_led_color)
{
    int drive_number;
    int drive_state = (int)state;

    for (drive_number = 0; drive_number < 4; ++drive_number) {
        if (drive_state & 1) {
            ui_display_drive_led(drive_number, 0, 0);
        } else {
            statusbar_text[STATUSBAR_DRIVE_POS + drive_number] = ' ';
        }
        drive_state >>= 1;
    }

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}

void ui_display_drive_track(unsigned int drive_number, unsigned int drive_base, unsigned int half_track_number)
{
    unsigned int track_number = half_track_number / 2;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif

    switch (drive_number) {
        case 1:
            statusbar_text[STATUSBAR_DRIVE9_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE9_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        case 2:
            statusbar_text[STATUSBAR_DRIVE10_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE10_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        case 3:
            statusbar_text[STATUSBAR_DRIVE11_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE11_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
        default:
        case 0:
            statusbar_text[STATUSBAR_DRIVE8_TRACK_POS] = (track_number / 10) + '0';
            statusbar_text[STATUSBAR_DRIVE8_TRACK_POS + 1] = (track_number % 10) + '0';
            break;
    }

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}

/* The pwm value will vary between 0 and 1000.  */
void ui_display_drive_led(int drive_number, unsigned int pwm1, unsigned int led_pwm2)
{
    char c;

#ifdef SDL_DEBUG
    fprintf(stderr, "%s: drive %i, pwm1 = %i, led_pwm2 = %u\n", __func__, drive_number, pwm1, led_pwm2);
#endif

    c = "8901"[drive_number] | ((pwm1 > 500) ? 0x80 : 0);
    statusbar_text[STATUSBAR_DRIVE_POS + (drive_number * 5)] = c;
    statusbar_text[STATUSBAR_DRIVE_POS + (drive_number * 5) + 1] = 'T';

    if (uistatusbar_state & UISTATUSBAR_ACTIVE) {
        uistatusbar_state |= UISTATUSBAR_REPAINT;
    }
}

void ui_display_drive_current_image(unsigned int drive_number, const char *image)
{
printf("d%d -> %s\n",drive_number, image);
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif
}

/* Tape related UI */

void ui_set_tape_status(int tape_status)
{
    tape_enabled = tape_status;

    display_tape();
}

void ui_display_tape_motor_status(int motor)
{
    tape_motor = motor;

    display_tape();
}

void ui_display_tape_control_status(int control)
{
    tape_control = control;

    display_tape();
}

void ui_display_tape_counter(int counter)
{
    if (tape_counter != counter) {
        display_tape();
    }

    tape_counter = counter;
}

void ui_display_tape_current_image(const char *image)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %s\n", __func__, image);
#endif
}

/* Recording UI */
void ui_display_playback(int playback_status, char *version)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i, \"%s\"\n", __func__, playback_status, version);
#endif
}

void ui_display_recording(int recording_status)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i\n", __func__, recording_status);
#endif
}

void ui_display_event_time(unsigned int current, unsigned int total)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i, %i\n", __func__, current, total);
#endif
}

/* Joystick UI */
void ui_display_joyport(BYTE *joyport)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %02x %02x %02x %02x %02x\n", __func__, joyport[0], joyport[1], joyport[2], joyport[3], joyport[4]);
#endif
}

/* Volume UI */
void ui_display_volume(int vol)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s: %i\n", __func__, vol);
#endif
}

/* ----------------------------------------------------------------- */
/* resources */

static int statusbar_enabled;

static int set_statusbar(int val, void *param)
{
    statusbar_enabled = val ? 1 : 0;

    if (statusbar_enabled) {
        uistatusbar_open();
    } else {
        uistatusbar_close();
    }

    return 0;
}

static const resource_int_t resources_int[] = {
    { "SDLStatusbar", 0, RES_EVENT_NO, NULL,
      &statusbar_enabled, set_statusbar, NULL },
    RESOURCE_INT_LIST_END
};

int uistatusbar_init_resources(void)
{
#ifdef SDL_DEBUG
    fprintf(stderr, "%s\n", __func__);
#endif
    return resources_register_int(resources_int);
}


/* ----------------------------------------------------------------- */
/* uistatusbar.h */

int uistatusbar_state = 0;

void uistatusbar_open(void)
{
    uistatusbar_state = UISTATUSBAR_ACTIVE | UISTATUSBAR_REPAINT;
}

void uistatusbar_close(void)
{
    uistatusbar_state = UISTATUSBAR_REPAINT;
}

#include "keyboard.h"
extern unsigned int cur_port;


#include "RSDL_wrapper.h"
#include "libretro-core.h"

#ifdef M16B
extern void Retro_Draw_string(RSDL_Surface *surface, signed short int x, signed short int y, const  char *string,unsigned short maxstrlen,unsigned short xscale, unsigned short yscale, unsigned short fg, unsigned short bg);
#else
extern void Retro_Draw_string(RSDL_Surface *surface, signed short int x, signed short int y, const  char *string,unsigned short maxstrlen,unsigned short xscale, unsigned short yscale, unsigned  fg, unsigned  bg);
#endif

RSDL_Surface fake;

void uistatusbar_draw(void)
{
    int i;
    BYTE c;//, color_f, color_b;
#ifdef M16B
unsigned short int color_f, color_b;
    color_f = 0xffff;
    color_b = 0;
#else
unsigned int color_f, color_b;
    color_f = 0xffffffff;
    color_b = 0;
#endif
    unsigned int line;

  //  color_f = 0xff;
  //  color_b = 0;

    pitch =PITCH;

    char tmpstr[512];

    fake.pixels=&bmp[0];
    fake.h=retroh;
    fake.w=retrow;
    fake.clip_rect.h=retroh;
    fake.clip_rect.w=retrow;
    fake.clip_rect.x=0;
    fake.clip_rect.y=0;
    
    int x, y;
    x=32;
    y=236;

    //sprintf(tmpstr,"joy%d:%2d ",1,joystick_value[1]);
    //sprintf(tmpstr + strlen(tmpstr),"joy%d:%2d",2,joystick_value[2]);
    //Retro_Draw_string(&fake, x+200, y, tmpstr,16,1,1, color_f, color_b);
    display_joyport();

    for (i = 0; i < MAX_STATUSBAR_LEN; ++i) {
        c = statusbar_text[i];

        if (c == 0) {
            break;
        }

        if (c & 0x80) {
		sprintf(tmpstr,"%c",c&0x7f);
		Retro_Draw_string(&fake, x+i*8, y, tmpstr,2,1,1, color_b, color_f);
	        //  uistatusbar_putchar((BYTE)(c & 0x7f), i, 0, color_b, color_f);
        } else {
         	//  uistatusbar_putchar(c, i, 0, color_f, color_b);
		sprintf(tmpstr,"%c",c);
		Retro_Draw_string(&fake, x+i*8, y, tmpstr,2,1,1, color_f, color_b);

        }
    }
}

