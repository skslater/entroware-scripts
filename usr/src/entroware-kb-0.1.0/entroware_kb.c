/*
* entroware_kb.c
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

#include "entroware_kb.h"

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/acpi.h>
#include <linux/dmi.h>
#include <linux/platform_device.h>
#include <linux/input.h>

MODULE_AUTHOR("Entroware <dev@entroware.com>");
MODULE_DESCRIPTION("Entroware Keyboard Driver");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1.0");

// Sysfs Interface for the keyboard state (ON / OFF)
static ssize_t show_state_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%d\n", keyboard.state);
}

static ssize_t set_state_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    unsigned int val;
    int ret = kstrtouint(buffer, 0, &val);

    if (ret)
    {
        return ret;
    }

    val = clamp_t(u8, val, 0, 1);

    set_kb_state(val);

    return ret ? : size;
}

// Sysfs Interface for the colour of the left side (Colour as hexvalue)
static ssize_t show_colour_left_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%06x\n", keyboard.colour.left);
}

static ssize_t set_colour_left_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    return set_colour_region(buffer, size, REGION_LEFT);
}

// Sysfs Interface for the colour of the centre (Colour as hexvalue)
static ssize_t show_colour_centre_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%06x\n", keyboard.colour.centre);
}

static ssize_t set_colour_centre_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    return set_colour_region(buffer, size, REGION_CENTRE);
}

// Sysfs Interface for the colour of the right side (Colour as hexvalue)
static ssize_t show_colour_right_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%06x\n", keyboard.colour.right);
}

static ssize_t set_colour_right_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    return set_colour_region(buffer, size, REGION_RIGHT);
}

// Sysfs Interface for the colour of the extra region (Colour as hexvalue)
static ssize_t show_colour_extra_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%06x\n", keyboard.colour.extra);
}

static ssize_t set_colour_extra_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    return set_colour_region(buffer, size, REGION_EXTRA);
}

// Sysfs Interface for the keyboard brightness (unsigned int)
static ssize_t show_brightness_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%d\n", keyboard.brightness);
}

static ssize_t set_brightness_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    unsigned int val;
    int ret = kstrtouint(buffer, 0, &val);

    if (ret)
    {
        return ret;
    }

    val = clamp_t(u8, val, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
    set_brightness(val);

    return ret ? : size;
}

// Sysfs Interface for the keyboard kbd_colour
static ssize_t show_kbd_colour_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%d\n", keyboard.kbd_colour);
}

static ssize_t set_kbd_colour_fs(struct device *child, struct device_attribute *attr, const char *buffer, size_t size)
{
    unsigned int val;
    int ret = kstrtouint(buffer, 0, &val);

    if (ret)
    {
        return ret;
    }

    val = clamp_t(u8, val, 0, ARRAY_SIZE(kbd_colours) - 1);
    set_kbd_colour(val);

    return ret ? : size;
}

// Sysfs Interface for if the keyboard has extra region
static ssize_t show_hasextra_fs(struct device *child, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, "%d\n", keyboard.has_extra);
}

static int __init entroware_kb_init(void)
{
    int err;
	const char *sys_vendor;

    sys_vendor = dmi_get_system_info(DMI_SYS_VENDOR);

    if (!wmi_has_guid(CLEVO_EVENT_GUID)) 
    {
        ENTROWARE_ERROR("No known WMI event notification GUID found\n");
        return -ENODEV;
    }

    if (!wmi_has_guid(CLEVO_GET_GUID)) 
    {
        ENTROWARE_ERROR("No known WMI control method GUID found\n");
        return -ENODEV;
    }

/*
    if (strcmp(sys_vendor, "Entroware"))
    {
        ENTROWARE_INFO("'%s %s (%s)' Detected\n", dmi_get_system_info(DMI_SYS_VENDOR),
            dmi_get_system_info(DMI_PRODUCT_NAME),
            dmi_get_system_info(DMI_PRODUCT_VERSION));
    }
    else
    {
        return -EPERM;
    }
*/
    

    entroware_platform_device = platform_create_bundle(&entroware_platform_driver, entroware_wmi_probe, NULL, 0, NULL, 0);
    if (unlikely(IS_ERR(entroware_platform_device)))
    {
        ENTROWARE_ERROR("Can not init Platform driver");
        return PTR_ERR(entroware_platform_device);
    }

    err = entroware_input_init();
    if (unlikely(err))
    {
        ENTROWARE_ERROR("Could not register input device\n");
    }	

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_state) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for state\n");
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_colour_left) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for colour left\n");
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_colour_centre) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for colour centre\n");
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_colour_right) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for colour right\n");
    }

    if(set_colour(REGION_EXTRA, KB_COLOUR_DEFAULT) == 0)
    {
        ENTROWARE_DEBUG("Keyboard does not support EXTRA Colour");
        keyboard.has_extra = 0;
    }
    else
    {
        keyboard.has_extra = 1;
        if (device_create_file(&entroware_platform_device->dev, &dev_attr_colour_extra) != 0)
        {
            ENTROWARE_ERROR("Sysfs attribute creation failed for colour extra\n");
        }

        set_colour(REGION_EXTRA, param_colour_extra);
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_extra) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for extra information flag\n");
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_kbd_colour) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for kbd_colour\n");
    }

    if (device_create_file(&entroware_platform_device->dev, &dev_attr_brightness) != 0)
    {
        ENTROWARE_ERROR("Sysfs attribute creation failed for brightness\n");
    }

    keyboard.colour.left = param_colour_left;
    keyboard.colour.centre = param_colour_centre;
    keyboard.colour.right = param_colour_right;
    keyboard.colour.extra = param_colour_extra;

    set_colour(REGION_LEFT,      param_colour_left);
    set_colour(REGION_CENTRE,    param_colour_centre);
    set_colour(REGION_RIGHT,     param_colour_right);

    set_kbd_colour(param_kbd_colour);
    set_brightness(param_brightness);
    set_kb_state(param_state);

    return 0;
}

static void __exit entroware_kb_exit(void)
{
    entroware_input_exit();

    device_remove_file(&entroware_platform_device->dev, &dev_attr_state);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_colour_left);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_colour_centre);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_colour_right);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_extra);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_kbd_colour);
    device_remove_file(&entroware_platform_device->dev, &dev_attr_brightness);

    if(keyboard.has_extra == 1)
    {
        device_remove_file(&entroware_platform_device->dev, &dev_attr_colour_extra);
    }

    platform_device_unregister(entroware_platform_device);

    platform_driver_unregister(&entroware_platform_driver);

    ENTROWARE_DEBUG("exit");
}

static int __init entroware_input_init(void)
{
    int err;

    entroware_input_device = input_allocate_device();
    if (unlikely(!entroware_input_device)) 
    {
        ENTROWARE_ERROR("Error allocating input device\n");
        return -ENOMEM;
    }

    entroware_input_device->name = "Entroware Keyboard";
    entroware_input_device->phys = DRIVER_NAME "/input0";
    entroware_input_device->id.bustype = BUS_HOST;
    entroware_input_device->dev.parent = &entroware_platform_device->dev;

    set_bit(EV_KEY, entroware_input_device->evbit);

    err = input_register_device(entroware_input_device);
    if (unlikely(err)) 
    {
        ENTROWARE_ERROR("Error registering input device\n");
        goto err_free_input_device;
    }

    return 0;

err_free_input_device:
    input_free_device(entroware_input_device);

    return err;
}

static void __exit entroware_input_exit(void)
{
    if (unlikely(!entroware_input_device))
    {
        return;
    }

    input_unregister_device(entroware_input_device);
    {
        entroware_input_device = NULL;
    }
}

static int entroware_wmi_probe(struct platform_device *dev)
{
    int status;

    status = wmi_install_notify_handler(CLEVO_EVENT_GUID, entroware_wmi_notify, NULL);
    ENTROWARE_DEBUG("clevo_xsm_wmi_probe status: (%0#6x)", status);

    if (unlikely(ACPI_FAILURE(status))) 
    {
    	ENTROWARE_ERROR("Could not register WMI notify handler (%0#6x)\n", status);
    	return -EIO;
    }

    entroware_evaluate_method(GET_AP, 0, NULL);

    return 0;
}

static int entroware_wmi_remove(struct platform_device *dev)
{
    wmi_remove_notify_handler(CLEVO_EVENT_GUID);
    return 0;
}

static int entroware_wmi_resume(struct platform_device *dev)
{
    entroware_evaluate_method(GET_AP, 0, NULL);

    return 0;
}

static void entroware_wmi_notify(u32 value, void *context)
{
    u32 event;

    entroware_evaluate_method(GET_EVENT, 0, &event);
	  ENTROWARE_DEBUG("WMI event (%0#6x)\n", event);

    switch(event)
    {
        case WMI_CODE_DECREASE_BACKLIGHT:
            if(keyboard.brightness == BRIGHTNESS_MIN || (keyboard.brightness - STEP_BRIGHTNESS_STEP) < BRIGHTNESS_MIN)
            {
                set_brightness(BRIGHTNESS_MIN);
            }
            else
            {
                set_brightness(keyboard.brightness - STEP_BRIGHTNESS_STEP);
            }

            break;

        case WMI_CODE_INCREASE_BACKLIGHT:
            if(keyboard.brightness == BRIGHTNESS_MAX || (keyboard.brightness + 25) > BRIGHTNESS_MAX)
            {
                set_brightness(BRIGHTNESS_MAX);
            }
            else
            {
                set_brightness(keyboard.brightness + STEP_BRIGHTNESS_STEP);
            }

            break;
            
        case WMI_CODE_NEXT_COLOUR:
            if ((keyboard.kbd_colour + 1) > (ARRAY_SIZE(kbd_colours) - 1))
            {
                set_kbd_colour(keyboard.kbd_colour = 0);
            }
            else
            {
                set_kbd_colour(keyboard.kbd_colour + 1);
            }

//            set_kbd_colour((keyboard.kbd_colour + 1) > (ARRAY_SIZE(kbd_colours) - 1) ? 0 : (keyboard.kbd_colour + 1));

//            ENTROWARE_INFO("    ARRAY_SIZE %ld", ARRAY_SIZE(kbd_colours) - 1);
//            ENTROWARE_INFO("    KBKBD_COLOURValue %d", keyboard.kbd_colour + 1);
            break;

        case WMI_CODE_TOGGLE_STATE:
            set_kb_state(keyboard.state == 0 ? 1 : 0);
            break;

        default:
            break;
    }
}

static void set_kbd_colour(u8 kbd_colour)
{
    ENTROWARE_INFO("colour: %s\n", kbd_colours[kbd_colour].name);

    if(!entroware_evaluate_method(SET_KB_LED, kbd_colours[kbd_colour].key, NULL))
    {
        set_colour(REGION_LEFT,     kbd_colours[kbd_colour].hexvalue);
        set_colour(REGION_CENTRE,   kbd_colours[kbd_colour].hexvalue);
        set_colour(REGION_RIGHT,    kbd_colours[kbd_colour].hexvalue);

        // increment kbd_colour int
        keyboard.kbd_colour = kbd_colour;
    }

    if(kbd_colour == 0)
    {
        set_colour(REGION_LEFT,      keyboard.colour.left);
        set_colour(REGION_CENTRE,    keyboard.colour.centre);
        set_colour(REGION_RIGHT,     keyboard.colour.right);

        if(keyboard.has_extra == 1)
        {
            set_colour(REGION_EXTRA, keyboard.colour.extra);
        }
    }
}

static void set_brightness(u8 brightness)
{
    ENTROWARE_INFO("brightness: %d\n", brightness);
    if (!entroware_evaluate_method(SET_KB_LED, 0xF4000000 | brightness, NULL))
	{
		keyboard.brightness = brightness;
	}
}

static void set_kb_state(u8 state)
{
    u32 cmd = 0xE0000000;
    ENTROWARE_INFO("state: %d\n", state);

    if(state == 0)
    {
        cmd |= 0x003001;
    }
    else
    {
        cmd |= 0x07F001;
    }

    if (!entroware_evaluate_method(SET_KB_LED, cmd, NULL))
    {
        keyboard.state = state;
    }
}

static int entroware_evaluate_method(u32 method_id, u32 arg, u32 *retval)
{
    struct acpi_buffer in  = { (acpi_size) sizeof(arg), &arg };
    struct acpi_buffer out = { ACPI_ALLOCATE_BUFFER, NULL };
    union acpi_object *obj;
    acpi_status status;
    u32 tmp;

    ENTROWARE_DEBUG("evaluate method: %0#4x  IN : %0#6x\n", method_id, arg);

    status = wmi_evaluate_method(CLEVO_GET_GUID, 0x00, method_id, &in, &out);

    if (unlikely(ACPI_FAILURE(status)))
    {
        ENTROWARE_ERROR("evaluate method error");
        goto exit;
    }

    obj = (union acpi_object *) out.pointer;
    if (obj && obj->type == ACPI_TYPE_INTEGER)
    {
        tmp = (u32) obj->integer.value;
    }
    else
    {
        tmp = 0;
    }

    ENTROWARE_DEBUG("%0#4x  OUT: %0#6x (IN: %0#6x)\n", method_id, tmp, arg);

    if (likely(retval))
    {
        *retval = tmp;
    }

    kfree(obj);

exit:
    if (unlikely(ACPI_FAILURE(status)))
    {
        return -EIO;
    }

    return 0;
}

static int set_colour(u32 region, u32 colour)
{
    u32 cset = ((colour & 0x0000FF) << 16) | ((colour & 0xFF0000) >> 8) | ((colour & 0x00FF00) >> 8);
    u32 cmd = region | cset;

    ENTROWARE_DEBUG("Set Colour '%08x' for region '%08x'", colour, region);

    return entroware_evaluate_method(SET_KB_LED, cmd, NULL);
}

static int set_colour_region(const char *buffer, size_t size, u32 region)
{
    u32 val;
    int ret = kstrtouint(buffer, 0, &val);

    if (ret)
    {
        return ret;
    }

    if(!set_colour(region, val))
    {
        switch(region)
        {
            case REGION_LEFT:
                keyboard.colour.left = val;
                break;
            case REGION_CENTRE:
                keyboard.colour.centre = val;
                break;
            case REGION_RIGHT:
                keyboard.colour.right = val;
                break;
            case REGION_EXTRA:
                keyboard.colour.extra = val;
                break;
        }
    }

    return ret ? : size;
}

static int kbd_colour_validator(const char *val, const struct kernel_param *kp)
{
    int kbd_colour = 0;
    int ret;

    ret = kstrtoint(val, 10, &kbd_colour);
    if (ret != 0 || kbd_colour < 0 || kbd_colour > (ARRAY_SIZE(kbd_colours) - 1))
    {
        return -EINVAL;
    }

    return param_set_int(val, kp);
}

static int brightness_validator(const char *val, const struct kernel_param *kp)
{
    int brightness = 0;
    int ret;

    ret = kstrtoint(val, 10, &brightness);
    if (ret != 0 || brightness < BRIGHTNESS_MIN || brightness > BRIGHTNESS_MAX)
    {
        return -EINVAL;
    }

    return param_set_int(val, kp);
}

module_init(entroware_kb_init);
module_exit(entroware_kb_exit);
