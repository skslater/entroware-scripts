/*
* entroware_kb.h
*
* Copyright (C) 2018 Entroware <dev@entroware.com>
* Copyright (C) 2018 TUXEDO Computers GmbH <tux@tuxedocomputers.com>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef _ENTROWARE_KB_H
#define _ENTROWARE_KB_H

#define DRIVER_NAME "entroware_kb"
#define pr_fmt(fmt) DRIVER_NAME ": " fmt

#include <linux/list.h>
#include <linux/platform_device.h>
#include <linux/module.h>

#define __ENTROWARE_PR(lvl, fmt, ...) do { pr_##lvl(fmt, ##__VA_ARGS__); } while (0)
#define ENTROWARE_INFO(fmt, ...) __ENTROWARE_PR(info, fmt, ##__VA_ARGS__)
#define ENTROWARE_ERROR(fmt, ...) __ENTROWARE_PR(err, fmt, ##__VA_ARGS__)
#define ENTROWARE_DEBUG(fmt, ...) __ENTROWARE_PR(debug, "[%s:%u] " fmt, __func__, __LINE__, ##__VA_ARGS__)

#define BRIGHTNESS_MIN                  0
#define BRIGHTNESS_MAX                  255
#define BRIGHTNESS_DEFAULT              BRIGHTNESS_MAX

#define KB_KBD_COLOUR_DEFAULT           0   // DEFAULT Mode

#define CLEVO_EVENT_GUID                "ABBC0F6B-8EA1-11D1-00A0-C90629100000"
#define CLEVO_EMAIL_GUID                "ABBC0F6C-8EA1-11D1-00A0-C90629100000"
#define CLEVO_GET_GUID                  "ABBC0F6D-8EA1-11D1-00A0-C90629100000"

#define REGION_LEFT                     0xF0000000
#define REGION_CENTRE                   0xF1000000
#define REGION_RIGHT                    0xF2000000
#define REGION_EXTRA                    0xF3000000

#define KEYBOARD_BRIGHTNESS             0xF4000000

#define COLOUR_WHITE                     0xFFFFFF
#define COLOUR_BLUE                      0x0000FF
#define COLOUR_CYAN                      0x00FFFF
#define COLOUR_RED                       0xFF0000
#define COLOUR_GREEN                     0x00FF00
#define COLOUR_YELLOW                    0xFFFF00
#define COLOUR_MAGENTA                   0xFF00FF

#define KB_COLOUR_DEFAULT                COLOUR_WHITE
#define DEFAULT_KBD_COLOUR               0 

// Method IDs for CLEVO_GET
#define GET_EVENT                       0x01
#define GET_AP                          0x46
#define SET_KB_LED                      0x67

// WMI Codes
#define WMI_CODE_DECREASE_BACKLIGHT     0x81
#define WMI_CODE_INCREASE_BACKLIGHT     0x82
#define WMI_CODE_NEXT_COLOUR            0x83
#define WMI_CODE_TOGGLE_STATE           0x9F

#define STEP_BRIGHTNESS_STEP            85

// Module Parameter Values
//static bool 

// Sysfs Interface Methods
// Sysfs Interface for the keyboard state (ON / OFF)
static ssize_t show_state_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_state_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);



// Sysfs Interface for the colour of the left side (Colour as hexvalue)
static ssize_t show_colour_left_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_colour_left_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for the colour of the centre (Colour as hexvalue)
static ssize_t show_colour_centre_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_colour_centre_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for the colour of the right side (Colour as hexvalue)
static ssize_t show_colour_right_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_colour_right_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for the colour of the extra region (Colour as hexvalue)
static ssize_t show_colour_extra_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_colour_extra_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for the keyboard brightness (unsigned int)
static ssize_t show_brightness_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_brightness_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for the keyboard kbd_colour
static ssize_t show_kbd_colour_fs(struct device *child, struct device_attribute *attr, char *buffer);
static ssize_t set_kbd_colour_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size);

// Sysfs Interface for if the keyboard has extra region
static ssize_t show_hasextra_fs(struct device *child, struct device_attribute *attr, char *buffer);

// Keyboard struct
static struct
{
    u8 has_extra;
    u8 state;

    struct 
    {
        u32 left;
        u32 centre;
        u32 right;
        u32 extra;
    } colour;

    u8 brightness;
    u8 kbd_colour;
} keyboard = {
    .has_extra = 0,
    .kbd_colour = DEFAULT_KBD_COLOUR,
    .state = 1,
    .brightness = BRIGHTNESS_DEFAULT,
    .colour =  {
        .left = KB_COLOUR_DEFAULT,
        .centre = KB_COLOUR_DEFAULT,
        .right = KB_COLOUR_DEFAULT,
        .extra = KB_COLOUR_DEFAULT,
    }
};

static struct
{
    u8 key;
    int hexvalue;
    const char *const name;
}kbd_colours[] = {
    { .key = 0,   .hexvalue = COLOUR_WHITE,    .name = "white" },
    { .key = 1,   .hexvalue = COLOUR_BLUE,     .name = "blue" },
    { .key = 2,   .hexvalue = COLOUR_CYAN,     .name = "cyan" },
    { .key = 3,   .hexvalue = COLOUR_RED,      .name = "red" },
    { .key = 4,   .hexvalue = COLOUR_GREEN,    .name = "green" },
    { .key = 5,   .hexvalue = COLOUR_YELLOW,   .name = "yellow" },
    { .key = 6,   .hexvalue = COLOUR_MAGENTA,  .name = "magenta" }
};


struct platform_device *entroware_platform_device;
static struct input_dev *entroware_input_device;

// Init and Exit methods
static int __init entroware_kb_init(void);
static void __exit entroware_kb_exit(void);

static int __init entroware_input_init(void);
static void __exit entroware_input_exit(void);

// Methods for controlling the Keyboard
static void set_brightness(u8 brightness);
static void set_kb_state(u8 state);
static void set_kbd_colour(u8 kbd_colour);
static int set_colour(u32 region, u32 colour);

static int set_colour_region(const char *buffer, size_t size, u32 region);

static int entroware_wmi_remove(struct platform_device *dev);
static int entroware_wmi_resume(struct platform_device *dev);
static int entroware_wmi_probe(struct platform_device *dev);
static void entroware_wmi_notify(u32 value, void *context);

static int entroware_evaluate_method(u32 method_id, u32 arg, u32 *retval);

static struct platform_driver entroware_platform_driver = {
    .remove = entroware_wmi_remove,
    .resume = entroware_wmi_resume,
    .driver = {
        .name  = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
};

// Param Validators
static int kbd_colour_validator(const char *val, const struct kernel_param *kp);
static const struct kernel_param_ops param_ops_kbd_colour_ops = {
	.set	= kbd_colour_validator,
	.get	= param_get_int,
};

static int brightness_validator(const char *val, const struct kernel_param *kp);
static const struct kernel_param_ops param_ops_brightness_ops = {
	.set	= brightness_validator,
	.get	= param_get_int,
};

// Params Variables
static uint param_colour_left = KB_COLOUR_DEFAULT;
module_param_named(colour_left, param_colour_left, uint, S_IRUSR);
MODULE_PARM_DESC(colour_left, "Colour for the Left Section");

static uint param_colour_centre = KB_COLOUR_DEFAULT;
module_param_named(colour_centre, param_colour_centre, uint, S_IRUSR);
MODULE_PARM_DESC(colour_centre, "Colour for the Centre Section");

static uint param_colour_right = KB_COLOUR_DEFAULT;
module_param_named(colour_right, param_colour_right, uint, S_IRUSR);
MODULE_PARM_DESC(colour_right, "Colour for the Right Right");

static uint param_colour_extra = KB_COLOUR_DEFAULT;
module_param_named(colour_extra, param_colour_extra, uint, S_IRUSR);
MODULE_PARM_DESC(colour_extra, "Colour for the Extra Section");

static ushort param_kbd_colour = DEFAULT_KBD_COLOUR;
module_param_cb(kbd_colour, &param_ops_kbd_colour_ops, &param_kbd_colour, S_IRUSR);
MODULE_PARM_DESC(kbd_colour, "Set the keyboard colour");

static ushort param_brightness = BRIGHTNESS_DEFAULT;
module_param_cb(brightness, &param_ops_brightness_ops, &param_brightness, S_IRUSR);
MODULE_PARM_DESC(brightness, "Set the Keyboard Brightness");

static bool param_state = true;
module_param_named(state, param_state, bool, S_IRUSR);
MODULE_PARM_DESC(state, "Set the State of the Keyboard TRUE = ON | FALSE = OFF");

// Sysfs device Attributes
static DEVICE_ATTR(state,           0644, show_state_fs,           set_state_fs);
static DEVICE_ATTR(colour_left,     0644, show_colour_left_fs,     set_colour_left_fs);
static DEVICE_ATTR(colour_centre,   0644, show_colour_centre_fs,   set_colour_centre_fs);
static DEVICE_ATTR(colour_right,    0644, show_colour_right_fs,    set_colour_right_fs);
static DEVICE_ATTR(colour_extra,    0644, show_colour_extra_fs,    set_colour_extra_fs);
static DEVICE_ATTR(brightness,      0644, show_brightness_fs,      set_brightness_fs);
static DEVICE_ATTR(kbd_colour,      0644, show_kbd_colour_fs,      set_kbd_colour_fs);
static DEVICE_ATTR(extra,           0444, show_hasextra_fs,        NULL);

#endif
