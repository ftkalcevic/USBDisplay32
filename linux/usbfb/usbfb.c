/*
 * usbfb.c -- Framebuffer driver for DisplayLink USB controller
 *
 * Copyright (C) 2009 Roberto De Ioris <roberto@unbit.it>
 * Copyright (C) 2009 Jaya Kumar <jayakumar.lkml@gmail.com>
 * Copyright (C) 2009 Bernie Thompson <bernie@plugable.com>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License v2. See the file COPYING in the main directory of this archive for
 * more details.
 *
 * Layout is based on skeletonfb by James Simmons and Geert Uytterhoeven,
 * usb-skeleton by GregKH.
 *
 * Device-specific portions based on information from Displaylink, with work
 * from Florian Echtler, Henrik Bjerregaard Pedersen, and others.
 */

#define pr_fmt(fmt) "usbfb: " fmt

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/usb.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/fb.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/prefetch.h>
#include <linux/delay.h>
#include <linux/version.h> /* many users build as module against old kernels*/
#include <linux/sysfs.h>
#include "usbfb.h"

#define ITDB02_43
//#define ITDB02_32W
#ifdef ITDB02_43
    #define DISPLAY_X 		480
    #define DISPLAY_Y 		272
    #define DISPLAY_X_MM	93
    #define DISPLAY_Y_MM	52
    #define SCANLINE_LEN (DISPLAY_X*2)
    #define ENDPOINT_NO 2
#endif


#ifdef ITDB02_32W
    #define DISPLAY_X 		400
    #define DISPLAY_Y 		240
    #define DISPLAY_X_MM	72
    #define DISPLAY_Y_MM	44
    #define SCANLINE_LEN (DISPLAY_X*2)
    #define ENDPOINT_NO 2
#endif


static struct fb_fix_screeninfo dlfb_fix = {
	.id =           "usbfb",
	.type =         FB_TYPE_PACKED_PIXELS,
	.visual =       FB_VISUAL_TRUECOLOR,
	.xpanstep =     0,
	.ypanstep =     0,
	.ywrapstep =    0,
	.accel =        FB_ACCEL_NONE,
};

static const u32 usbfb_info_flags = FBINFO_DEFAULT | FBINFO_READS_FAST |
		FBINFO_VIRTFB |
        	FBINFO_FLAG_DEFAULT |
		FBINFO_HWACCEL_IMAGEBLIT | FBINFO_HWACCEL_FILLRECT |
		FBINFO_HWACCEL_COPYAREA | FBINFO_MISC_ALWAYS_SETPAR;

/*
 * Match the VID/PID of the USBDisplay32 device.
 */
static struct usb_device_id id_table[] = {
	{.idVendor = 0x1C40,
         .idProduct = 0x04DC,
         .bInterfaceClass = 0xff,
         .bInterfaceSubClass = 0x00,
         .bInterfaceProtocol = 0x00,
	 .match_flags = USB_DEVICE_ID_MATCH_VENDOR |
		USB_DEVICE_ID_MATCH_PRODUCT /* |
                USB_DEVICE_ID_MATCH_INT_CLASS |
                USB_DEVICE_ID_MATCH_INT_SUBCLASS |
                USB_DEVICE_ID_MATCH_INT_PROTOCOL*/,
	},
	{},
};
MODULE_DEVICE_TABLE(usb, id_table);

/* module options */
static bool console = false; /* Allow fbcon to open framebuffer */
static bool fb_defio = true;  /* Detect mmap writes using page faults */
static bool shadow = true; /* Optionally disable shadow framebuffer */
static int backlight = 80; /* Default backlight intensity 0-100 */


DEFINE_SEMAPHORE(register_fb_lock);
static void lock_register_fb(void)
{
    pr_err("lock_register_fb\n");
    down( &register_fb_lock );
    pr_err("lock_register_fb:down\n");
}
static void unlock_register_fb(void)
{
    pr_err("unlock_register_fb\n");
    up( &register_fb_lock );
    pr_err("unlock_register_fb:up\n");
}
EXPORT_SYMBOL_GPL(unlock_register_fb);
EXPORT_SYMBOL_GPL(lock_register_fb);

/* dlfb keeps a list of urbs for efficient bulk transfers */
static void dlfb_urb_completion(struct urb *urb);
static struct urb *dlfb_get_urb(struct dlfb_data *dev);
static int dlfb_submit_urb(struct dlfb_data *dev, struct urb * urb, size_t len);
static int dlfb_alloc_urb_list(struct dlfb_data *dev, int count, size_t size);
static void dlfb_free_urb_list(struct dlfb_data *dev);


static int dlfb_set_bootloader( struct dlfb_data *dev )
{
	char *buf;
	char *wrptr;
	int retval = 0;
	int writesize;
	struct urb *urb;

	if (!atomic_read(&dev->usb_active))
		return -EPERM;

	urb = dlfb_get_urb(dev);
	if (!urb)
		return -ENOMEM;

	buf = (char *) urb->transfer_buffer;
        wrptr = buf;

	*wrptr++ = 0x8D;
	*wrptr++ = 0x70;
	*wrptr++ = 0x74;
	*wrptr++ = 0x84;
	*wrptr++ = 0x19;
	*wrptr++ = 0x68;

	writesize = wrptr - buf;

	retval = dlfb_submit_urb(dev, urb, writesize);

	return retval;
}

static int dlfb_set_backlight(struct dlfb_data *dev,
			      int backlight )
{
	char *buf;
	char *wrptr;
	int retval = 0;
	int writesize;
	struct urb *urb;

	if (!atomic_read(&dev->usb_active))
		return -EPERM;

	urb = dlfb_get_urb(dev);
	if (!urb)
		return -ENOMEM;

	buf = (char *) urb->transfer_buffer;
        wrptr = buf;

	*wrptr++ = 0x87;
	*wrptr++ = backlight;

	writesize = wrptr - buf;

	retval = dlfb_submit_urb(dev, urb, writesize);

	return retval;
}

static int dlfb_ops_mmap(struct fb_info *info, struct vm_area_struct *vma)
{
	unsigned long start = vma->vm_start;
	unsigned long size = vma->vm_end - vma->vm_start;
	unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
	unsigned long page, pos;

	if (offset + size > info->fix.smem_len)
		return -EINVAL;

	pos = (unsigned long)info->fix.smem_start + offset;

	pr_notice("mmap() framebuffer addr:%lu size:%lu\n",
		  pos, size);

	while (size > 0) {
		page = vmalloc_to_pfn((void *)pos);
		if (remap_pfn_range(vma, start, page, PAGE_SIZE, PAGE_SHARED))
			return -EAGAIN;

		start += PAGE_SIZE;
		pos += PAGE_SIZE;
		if (size > PAGE_SIZE)
			size -= PAGE_SIZE;
		else
			size = 0;
	}

	//vma->vm_flags |= VM_DONTEXPAND | VM_DONTDUMP;	/* avoid to swap out this VMA */
	return 0;
}

int dlfb_handle_damage(struct dlfb_data *dev, int x, int y,
	       int width, int height, char *data)
{
	int i,j, ret;
	char *cmd;
	cycles_t start_cycles, end_cycles;
	int bytes_sent = 0;
	int bytes_identical = 0;
	struct urb *urb;

    // pr_info("Handle Damage %d,%d, %dx%d\n", x,y, width, height );

	start_cycles = get_cycles();

	if ((width <= 0) ||
	    (x + width > dev->info->var.xres) ||
	    (y + height > dev->info->var.yres))
		return -EINVAL;

	if (!atomic_read(&dev->usb_active))
		return 0;

	urb = dlfb_get_urb(dev);
	if (!urb)
            return 0;
	cmd = urb->transfer_buffer;
	bytes_sent = 0;
        

        *cmd++ = 0x8b;
        *cmd++ = (x >> 8) & 0xFF;
        *cmd++ = x & 0xFF;
        *cmd++ = (y >> 8) & 0xFF;
        *cmd++ = y & 0xFF;
        *cmd++ = (width >> 8) & 0xFF;
        *cmd++ = width & 0xFF;
        *cmd++ = (height >> 8) & 0xFF;
        *cmd++ = height & 0xFF;
        bytes_sent = 9;
        ret = dlfb_submit_urb(dev, urb, bytes_sent);
         
	urb = dlfb_get_urb(dev);
	if (!urb)
            return 0;
	cmd = urb->transfer_buffer;
	bytes_sent = 0;

        for (i = y; i < y + height ; i++) 
        {
            const int line_offset = dev->info->fix.line_length * i;
            const int byte_offset = line_offset + (x * BPP);

            char *pixels = (char *) dev->info->fix.smem_start;
            pixels += byte_offset;
            for ( j = 0; j < width; j++ )
            {
                char b2 = *(pixels++);
                char b1 = *(pixels++);
                *cmd++ = b1;
                *cmd++ = b2;
                bytes_sent += 2;

                if ( bytes_sent >= MAX_TRANSFER )
                {
                    // Full buffer
                    ret = dlfb_submit_urb(dev, urb, bytes_sent);

                    urb = dlfb_get_urb(dev);
	            if (!urb)
                    {
                        pr_warn( "No urb to complete damage transfer\n" );
                        return 0;
                    }   
	            cmd = urb->transfer_buffer;
                    bytes_sent = 0;
                }
            }
	}
        if ( bytes_sent > 0 )
            ret = dlfb_submit_urb(dev, urb, bytes_sent);
        else
            dlfb_urb_completion(urb);

	atomic_add(bytes_sent, &dev->bytes_sent);
	atomic_add(bytes_identical, &dev->bytes_identical);
	atomic_add(width*height*2, &dev->bytes_rendered);
	end_cycles = get_cycles();
	atomic_add(((unsigned int) ((end_cycles - start_cycles)
		    >> 10)), /* Kcycles */
		   &dev->cpu_kcycles_used);

	return 0;
}

static ssize_t dlfb_ops_read(struct fb_info *info, char __user *buf,
			 size_t count, loff_t *ppos)
{
	ssize_t result = -ENOSYS;

#if defined CONFIG_FB_SYS_FOPS || defined CONFIG_FB_SYS_FOPS_MODULE
	result = fb_sys_read(info, buf, count, ppos);
#endif

	return result;
}

/*
 * Path triggered by usermode clients who write to filesystem
 * e.g. cat filename > /dev/fb1
 * Not used by X Windows or text-mode console. But useful for testing.
 * Slow because of extra copy and we must assume all pixels dirty.
 */
static ssize_t dlfb_ops_write(struct fb_info *info, const char __user *buf,
			  size_t count, loff_t *ppos)
{
	ssize_t result;
	struct dlfb_data *dev = info->par;
	u32 offset = (u32) *ppos;

#if defined CONFIG_FB_SYS_FOPS || defined CONFIG_FB_SYS_FOPS_MODULE

	result = fb_sys_write(info, buf, count, ppos);

	if (result > 0) {
		int start = max((int)(offset / info->fix.line_length) - 1, 0);
		int lines = min((u32)((result / info->fix.line_length) + 1),
				(u32)info->var.yres);

		dlfb_handle_damage(dev, 0, start, info->var.xres,
			lines, info->screen_base);
	}
#endif

	return result;
}

/* hardware has native COPY command (see libdlo), but not worth it for fbcon */
static void dlfb_ops_copyarea(struct fb_info *info,
				const struct fb_copyarea *area)
{

	struct dlfb_data *dev = info->par;

#if defined CONFIG_FB_SYS_COPYAREA || defined CONFIG_FB_SYS_COPYAREA_MODULE

	sys_copyarea(info, area);

	dlfb_handle_damage(dev, area->dx, area->dy,
			area->width, area->height, info->screen_base);
#endif

}

static void dlfb_ops_imageblit(struct fb_info *info,
				const struct fb_image *image)
{
	struct dlfb_data *dev = info->par;

#if defined CONFIG_FB_SYS_IMAGEBLIT || defined CONFIG_FB_SYS_IMAGEBLIT_MODULE

	sys_imageblit(info, image);

	dlfb_handle_damage(dev, image->dx, image->dy,
			image->width, image->height, info->screen_base);

#endif

}

static void dlfb_ops_fillrect(struct fb_info *info,
			  const struct fb_fillrect *rect)
{
	struct dlfb_data *dev = info->par;

#if defined CONFIG_FB_SYS_FILLRECT || defined CONFIG_FB_SYS_FILLRECT_MODULE

	sys_fillrect(info, rect);

	dlfb_handle_damage(dev, rect->dx, rect->dy, rect->width,
			      rect->height, info->screen_base);
#endif

}

#ifdef CONFIG_FB_DEFERRED_IO
/*
 * NOTE: fb_defio.c is holding info->fbdefio.mutex
 *   Touching ANY framebuffer memory that triggers a page fault
 *   in fb_defio will cause a deadlock, when it also tries to
 *   grab the same mutex.
 */
static void dlfb_dpy_deferred_io(struct fb_info *info,
				struct list_head *pagelist)
{
	struct page *cur;
	struct fb_deferred_io *fbdefio = info->fbdefio;
	struct dlfb_data *dev = info->par;
    int ymin, ymax;
    bool bFirst = true;
    int bytes = 0;

	//pr_notice("D\n");
                //return;
	if (!fb_defio)
		return;

	if (!atomic_read(&dev->usb_active))
		return;
	atomic_add(1, &dev->deferred_calls);

    // Measure the damage area, then call standard damage routine.
    // Each page is 4k (2048 pixels wide), so we always do full lines
	list_for_each_entry(cur, &fbdefio->pagelist, lru) 
    {
        int start = cur->index << PAGE_SHIFT;
        int end = start + PAGE_SIZE;

	    //pr_notice("%d %d\n", start, end );
    
        int ystart = start / SCANLINE_LEN;
        int yend = (end / SCANLINE_LEN)+1;

        bytes += PAGE_SIZE;

        if ( bFirst )
        {
            ymin = ystart;
            ymax = yend;
            bFirst = false;
        }
        else
        {
            if ( ystart < ymin )
                ymin = ystart;
            if ( yend > ymax )
                ymax = yend;
        }
	}
	atomic_add(bytes, &dev->deferred_bytes);

    if ( bFirst )
    {
        // Didn't find anything
    }
    else
    {
        if ( ymin < 0 )
            ymin = 0;
        if ( ymax >= DISPLAY_Y )
            ymax = DISPLAY_Y-1;
	    //pr_notice("y %d %d\n", ymin, ymax );
		dlfb_handle_damage( dev, 0, ymin, DISPLAY_X, ymax-ymin+1,
			                info->screen_base);
    }

return;
}

#endif

static int dlfb_get_edid(struct dlfb_data *dev, char *edid, int len)
{
	int i;

        static char usbdisplay32_edid[128] =
{
    // Header
    0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, // 0-7
    0, 0,                                           // 8-9
    0, 0,                                           // 10-11
    0, 0, 0, 0,                                     // 12-15
    0,                                              // 16
    0,                                              // 17
    1,                                              // 18
    3,                                              // 19
    // Basic Display
    0x80,                                           // 20 
    10,                                             // 21
    6,                                              // 22
    100,                                            // 23
    0x18,                                           // 24
    // Chromaticity
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                   // 25-34
    // Established timing
    0,                                              // 35
    0,                                              // 36
    0,                                              // 37
    // Standard timing
    (DISPLAY_X/8)-31, 0,                                          // 38-39
    1, 1,                                           // 40-41
    1, 1,                                           // 42-43
    1, 1,                                           // 44-45
    1, 1,                                           // 46-47
    1, 1,                                           // 48-49
    1, 1,                                           // 50-51
    1, 1,                                           // 52-53
    // desc 1 - timing detail
    1,0,
    DISPLAY_X & 0xFF,
    0,
    (DISPLAY_X & 0x0F00)>>4,
    DISPLAY_Y & 0xFF,
    0,
    (DISPLAY_Y & 0x0F00)>>4,
    0,
    0,
    0,
    DISPLAY_X_MM,
    DISPLAY_Y_MM,
    0,
    0,
    0,
    0x18,
    // desc 2 - Range Limits
    0,0,0,
    0xFD,
    0,
    1,1,1,1,10,
    0,0,0,0,0,0,0,0,
    // desc 3 - monitor name
    0,0,0,
    0xFC,
    0,
    'U','S','B','D','i','s','p','l','a','y','3','2','\n',
    // desc 4 - serial #
    0,0,0,
    0xFF,
    0,
    '1','\n',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
    0
};
        char sum = 0;

	pr_info("get_edid\n");
	pr_info("sizeof static_edid=%d\n", sizeof(usbdisplay32_edid) );

        for ( i = 0; i < sizeof(usbdisplay32_edid); i++ )
        {
            edid[i] = usbdisplay32_edid[i];
            sum += edid[i];
        }
        edid[127] = -sum;

	return len;
}

static int dlfb_ops_ioctl(struct fb_info *info, unsigned int cmd,
				unsigned long arg)
{

	struct dlfb_data *dev = info->par;

	if (!atomic_read(&dev->usb_active))
		return 0;

	/* TODO: Update X server to get this from sysfs instead */
	if (cmd == DLFB_IOCTL_RETURN_EDID) {
		void __user *edid = (void __user *)arg;
		if (copy_to_user(edid, dev->edid, dev->edid_size))
			return -EFAULT;
		return 0;
	}

	/* TODO: Help propose a standard fb.h ioctl to report mmap damage */
	if (cmd == DLFB_IOCTL_REPORT_DAMAGE) {
		struct dloarea area;

		if (copy_from_user(&area, (void __user *)arg,
				  sizeof(struct dloarea)))
			return -EFAULT;

		/*
		 * If we have a damage-aware client, turn fb_defio "off"
		 * To avoid perf imact of unnecessary page fault handling.
		 * Done by resetting the delay for this fb_info to a very
		 * long period. Pages will become writable and stay that way.
		 * Reset to normal value when all clients have closed this fb.
		 */
#ifdef CONFIG_FB_DEFERRED_IO
		if (info->fbdefio)
			info->fbdefio->delay = DL_DEFIO_WRITE_DISABLE;
#endif
		if (area.x < 0)
			area.x = 0;

		if (area.x > info->var.xres)
			area.x = info->var.xres;

		if (area.y < 0)
			area.y = 0;

		if (area.y > info->var.yres)
			area.y = info->var.yres;

		dlfb_handle_damage(dev, area.x, area.y, area.w, area.h,
			   info->screen_base);
	}

	return 0;
}

/* taken from vesafb */
static int
dlfb_ops_setcolreg(unsigned regno, unsigned red, unsigned green,
	       unsigned blue, unsigned transp, struct fb_info *info)
{
	int err = 0;

	if (regno >= info->cmap.len)
		return 1;

	if (regno < 16) {
                /* 0:5:6:5 */
                ((u32 *) (info->pseudo_palette))[regno] =
                    ((red & 0xf800)) |
                    ((green & 0xfc00) >> 5) | ((blue & 0xf800) >> 11);
	}

	return err;
}

/*
 * It's common for several clients to have framebuffer open simultaneously.
 * e.g. both fbcon and X. Makes things interesting.
 * Assumes caller is holding info->lock (for open and release at least)
 */
static int dlfb_ops_open(struct fb_info *info, int user)
{
	struct dlfb_data *dev = info->par;

	/*
	 * fbcon aggressively connects to first framebuffer it finds,
	 * preventing other clients (X) from working properly. Usually
	 * not what the user wants. Fail by default with option to enable.
	 */
	if ((user == 0) && (!console))
		return -EBUSY;

	/* If the USB device is gone, we don't accept new opens */
	if (dev->virtualized)
		return -ENODEV;

	dev->fb_count++;

	kref_get(&dev->kref);

#ifdef CONFIG_FB_DEFERRED_IO
	if (fb_defio && (info->fbdefio == NULL)) {
		/* enable defio at last moment if not disabled by client */

		struct fb_deferred_io *fbdefio;

		fbdefio = kmalloc(sizeof(struct fb_deferred_io), GFP_KERNEL);

		if (fbdefio) {
			fbdefio->delay = DL_DEFIO_WRITE_DELAY;
			fbdefio->deferred_io = dlfb_dpy_deferred_io;
		}

		info->fbdefio = fbdefio;
		fb_deferred_io_init(info);
	}
#endif

	pr_notice("open /dev/fb%d user=%d fb_info=%p count=%d\n",
	    info->node, user, info, dev->fb_count);

	return 0;
}

/*
 * Called when all client interfaces to start transactions have been disabled,
 * and all references to our device instance (dlfb_data) are released.
 * Every transaction must have a reference, so we know are fully spun down
 */
static void dlfb_free(struct kref *kref)
{
	struct dlfb_data *dev = container_of(kref, struct dlfb_data, kref);

	/* this function will wait for all in-flight urbs to complete */
	if (dev->urbs.count > 0)
		dlfb_free_urb_list(dev);

	if (dev->backing_buffer)
		vfree(dev->backing_buffer);

	kfree(dev->edid);

	pr_warn("freeing dlfb_data %p\n", dev);

	kfree(dev);
}

static void dlfb_release_urb_work(struct work_struct *work)
{
	struct urb_node *unode = container_of(work, struct urb_node,
					      release_urb_work.work);

	up(&unode->dev->urbs.limit_sem);
}

static void dlfb_free_framebuffer_work(struct work_struct *work)
{
	struct dlfb_data *dev = container_of(work, struct dlfb_data,
					     free_framebuffer_work.work);
	struct fb_info *info = dev->info;
	int node = info->node;

	unregister_framebuffer(info);

	if (info->cmap.len != 0)
		fb_dealloc_cmap(&info->cmap);
	if (info->monspecs.modedb)
		fb_destroy_modedb(info->monspecs.modedb);
	if (info->screen_base)
		vfree(info->screen_base);

	fb_destroy_modelist(&info->modelist);

	dev->info = 0;

	/* Assume info structure is freed after this point */
	framebuffer_release(info);

	pr_warn("fb_info for /dev/fb%d has been freed\n", node);

	/* ref taken in probe() as part of registering framebfufer */
	kref_put(&dev->kref, dlfb_free);
}

/*
 * Assumes caller is holding info->lock mutex (for open and release at least)
 */
static int dlfb_ops_release(struct fb_info *info, int user)
{
	struct dlfb_data *dev = info->par;

	dev->fb_count--;

	/* We can't free fb_info here - fbmem will touch it when we return */
	if (dev->virtualized && (dev->fb_count == 0))
		schedule_delayed_work(&dev->free_framebuffer_work, HZ);

#ifdef CONFIG_FB_DEFERRED_IO
	if ((dev->fb_count == 0) && (info->fbdefio)) {
		fb_deferred_io_cleanup(info);
		kfree(info->fbdefio);
		info->fbdefio = NULL;
		info->fbops->fb_mmap = dlfb_ops_mmap;
	}
#endif

	pr_warn("released /dev/fb%d user=%d count=%d\n",
		  info->node, user, dev->fb_count);

	kref_put(&dev->kref, dlfb_free);

	return 0;
}

/*
 * Check whether a video mode is supported by the DisplayLink chip
 * We start from monitor's modes, so don't need to filter that here
 */
static int dlfb_is_valid_mode(struct fb_videomode *mode,
		struct fb_info *info)
{
	//struct dlfb_data *dev = info->par;

	if (mode->xres != DISPLAY_X ||
            mode->yres != DISPLAY_Y) {
		pr_warn("%dx%d beyond chip capabilities\n",
		       mode->xres, mode->yres);
		return 0;
	}

	pr_info("%dx%d valid mode\n", mode->xres, mode->yres);

	return 1;
}

static void dlfb_var_color_format(struct fb_var_screeninfo *var)
{
	const struct fb_bitfield red = { 11, 5, 0 };
	const struct fb_bitfield green = { 5, 6, 0 };
	const struct fb_bitfield blue = { 0, 5, 0 };

	var->bits_per_pixel = 16;
	var->red = red;
	var->green = green;
	var->blue = blue;
}

static int dlfb_ops_check_var(struct fb_var_screeninfo *var,
				struct fb_info *info)
{
	struct fb_videomode mode;

	/* TODO: support dynamically changing framebuffer size */
	if ((var->xres * var->yres * 2) > info->fix.smem_len)
		return -EINVAL;

	/* set device-specific elements of var unrelated to mode */
	dlfb_var_color_format(var);

	fb_var_to_videomode(&mode, var);

	if (!dlfb_is_valid_mode(&mode, info))
		return -EINVAL;

	return 0;
}

static int dlfb_ops_set_par(struct fb_info *info)
{
	struct dlfb_data *dev = info->par;
	int result=0;
	u16 *pix_framebuffer;
	int i;

	pr_notice("set_par mode %dx%d\n", info->var.xres, info->var.yres);

        dlfb_set_backlight( dev, atomic_read(&dev->backlight_intensity) );

	//result = dlfb_set_video_mode(dev, &info->var);

	if ((result == 0) && (dev->fb_count == 0)) {

		/* paint greenscreen */

		pix_framebuffer = (u16 *) info->screen_base;
		for (i = 0; i < info->fix.smem_len / 2; i++)
			pix_framebuffer[i] = 0x37e6;

		dlfb_handle_damage(dev, 0, 0, info->var.xres, info->var.yres,
				   info->screen_base);
	}

	return result;
}


/*
 * In order to come back from full DPMS off, we need to set the mode again
 */
static int dlfb_ops_blank(int blank_mode, struct fb_info *info)
{
    return 0;
}

static struct fb_ops dlfb_ops = {
	.owner = THIS_MODULE,
	.fb_read = dlfb_ops_read,
	.fb_write = dlfb_ops_write,
	.fb_setcolreg = dlfb_ops_setcolreg,
	.fb_fillrect = dlfb_ops_fillrect,
	.fb_copyarea = dlfb_ops_copyarea,
	.fb_imageblit = dlfb_ops_imageblit,
	.fb_mmap = dlfb_ops_mmap,
	.fb_ioctl = dlfb_ops_ioctl,
	.fb_open = dlfb_ops_open,
	.fb_release = dlfb_ops_release,
	.fb_blank = dlfb_ops_blank,
	.fb_check_var = dlfb_ops_check_var,
	.fb_set_par = dlfb_ops_set_par,
};


/*
 * Assumes &info->lock held by caller
 * Assumes no active clients have framebuffer open
 */
static int dlfb_realloc_framebuffer(struct dlfb_data *dev, struct fb_info *info)
{
	int retval = -ENOMEM;
	int old_len = info->fix.smem_len;
	int new_len;
	unsigned char *old_fb = info->screen_base;
	unsigned char *new_fb;
	unsigned char *new_back = 0;

	pr_warn("Reallocating framebuffer. Addresses will change!\n");

	new_len = info->fix.line_length * info->var.yres;

	if (PAGE_ALIGN(new_len) > old_len) {
		/*
		 * Alloc system memory for virtual framebuffer
		 */
		new_fb = vmalloc(new_len);
		if (!new_fb) {
			pr_err("Virtual framebuffer alloc failed\n");
			goto error;
		}

		if (info->screen_base) {
			memcpy(new_fb, old_fb, old_len);
			vfree(info->screen_base);
		}

		info->screen_base = new_fb;
		info->fix.smem_len = PAGE_ALIGN(new_len);
		info->fix.smem_start = (unsigned long) new_fb;
		info->flags = usbfb_info_flags;

/*
 * For a range of kernels, must set these to workaround bad logic
 * that assumes all framebuffers are using PCI aperture.
 * If we don't do this, when we call register_framebuffer, fbmem.c will
 * forcibly unregister other framebuffers with smem_start of zero.  And
 * that's most of them (VESA, EFI, etc).
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32) && \
	LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 34))
		info->aperture_base = info->fix.smem_start;
		info->aperture_size = info->fix.smem_len;
#endif

		/*
		 * Second framebuffer copy to mirror the framebuffer state
		 * on the physical USB device. We can function without this.
		 * But with imperfect damage info we may send pixels over USB
		 * that were, in fact, unchanged - wasting limited USB bandwidth
		 */
		if (shadow)
			new_back = vzalloc(new_len);
		if (!new_back)
			pr_info("No shadow/backing buffer allocated\n");
		else {
			if (dev->backing_buffer)
				vfree(dev->backing_buffer);
			dev->backing_buffer = new_back;
		}
	}

	retval = 0;

error:
	return retval;
}

/*
 * 1) Get EDID from hw, or use sw default
 * 2) Parse into various fb_info structs
 * 3) Allocate virtual framebuffer memory to back highest res mode
 *
 * Parses EDID into three places used by various parts of fbdev:
 * fb_var_screeninfo contains the timing of the monitor's preferred mode
 * fb_info.monspecs is full parsed EDID info, including monspecs.modedb
 * fb_info.modelist is a linked list of all monitor & VESA modes which work
 *
 * If EDID is not readable/valid, then modelist is all VESA modes,
 * monspecs is NULL, and fb_var_screeninfo is set to safe VESA mode
 * Returns 0 if successful
 */
static int dlfb_setup_modes(struct dlfb_data *dev,
			   struct fb_info *info,
			   char *default_edid, size_t default_edid_size)
{
	int i;
	const struct fb_videomode *default_vmode = NULL;
	int result = 0;
	char *edid;
	int tries = 3;

	if (info->dev) /* only use mutex if info has been registered */
		mutex_lock(&info->lock);

	edid = kmalloc(EDID_LENGTH, GFP_KERNEL);
	if (!edid) {
		result = -ENOMEM;
		goto error;
	}

	fb_destroy_modelist(&info->modelist);
	memset(&info->monspecs, 0, sizeof(info->monspecs));

	/*
	 * Try to (re)read EDID from hardware first
	 * EDID data may return, but not parse as valid
	 * Try again a few times, in case of e.g. analog cable noise
	 */
	while (tries--) {

		i = dlfb_get_edid(dev, edid, EDID_LENGTH);

		if (i >= EDID_LENGTH)
			fb_edid_to_monspecs(edid, &info->monspecs);

		if (info->monspecs.modedb_len > 0) {
			dev->edid = edid;
			dev->edid_size = i;
			break;
		}
	}

	/* If that fails, use a previously returned EDID if available */
	if (info->monspecs.modedb_len == 0) {

		pr_err("Unable to get valid EDID from device/display\n");

		if (dev->edid) {
			fb_edid_to_monspecs(dev->edid, &info->monspecs);
			if (info->monspecs.modedb_len > 0)
				pr_err("Using previously queried EDID\n");
		}
	}

	/* If that fails, use the default EDID we were handed */
	if (info->monspecs.modedb_len == 0) {
		if (default_edid_size >= EDID_LENGTH) {
			fb_edid_to_monspecs(default_edid, &info->monspecs);
			if (info->monspecs.modedb_len > 0) {
				memcpy(edid, default_edid, default_edid_size);
				dev->edid = edid;
				dev->edid_size = default_edid_size;
				pr_err("Using default/backup EDID\n");
			}
		}
	}

	/* If we've got modes, let's pick a best default mode */
	if (info->monspecs.modedb_len > 0) {

		for (i = 0; i < info->monspecs.modedb_len; i++) {
			if (dlfb_is_valid_mode(&info->monspecs.modedb[i], info))
				fb_add_videomode(&info->monspecs.modedb[i],
					&info->modelist);
			else {
				if (i == 0)
					/* if we've removed top/best mode */
					info->monspecs.misc
						&= ~FB_MISC_1ST_DETAIL;
			}
		}

		default_vmode = fb_find_best_display(&info->monspecs,
						     &info->modelist);
	}

	/* If everything else has failed, fall back to safe default mode */
	if (default_vmode == NULL) {

		struct fb_videomode fb_vmode = {0};

		/*
		 * Add the standard VESA modes to our modelist
		 * Since we don't have EDID, there may be modes that
		 * overspec monitor and/or are incorrect aspect ratio, etc.
		 * But at least the user has a chance to choose
		 */
		for (i = 0; i < VESA_MODEDB_SIZE; i++) {
			if (dlfb_is_valid_mode((struct fb_videomode *)
						&vesa_modes[i], info))
				fb_add_videomode(&vesa_modes[i],
						 &info->modelist);
		}

		/*
		 * default to resolution safe for projectors
		 * (since they are most common case without EDID)
		 */
		fb_vmode.xres = 800;
		fb_vmode.yres = 600;
		fb_vmode.refresh = 60;
		default_vmode = fb_find_nearest_mode(&fb_vmode,
						     &info->modelist);
	}

	/* If we have good mode and no active clients*/
	if ((default_vmode != NULL) && (dev->fb_count == 0)) {

		fb_videomode_to_var(&info->var, default_vmode);
		dlfb_var_color_format(&info->var);

		/*
		 * with mode size info, we can now alloc our framebuffer.
		 */
		memcpy(&info->fix, &dlfb_fix, sizeof(dlfb_fix));
		info->fix.line_length = info->var.xres *
			(info->var.bits_per_pixel / 8);

		result = dlfb_realloc_framebuffer(dev, info);

	} else
		result = -EINVAL;

error:
	if (edid && (dev->edid != edid))
		kfree(edid);

	if (info->dev)
		mutex_unlock(&info->lock);

	return result;
}

static ssize_t metrics_bytes_rendered_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->bytes_rendered));
}

static ssize_t metrics_bytes_identical_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->bytes_identical));
}

static ssize_t metrics_bytes_sent_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->bytes_sent));
}

static ssize_t metrics_deferred_calls_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->deferred_calls));
}

static ssize_t metrics_deferred_bytes_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->deferred_bytes));
}


static ssize_t metrics_cpu_kcycles_used_show(struct device *fbdev,
				   struct device_attribute *a, char *buf) {
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->cpu_kcycles_used));
}

static ssize_t edid_show(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
			struct file *filp,
#endif
			struct kobject *kobj, struct bin_attribute *a,
			char *buf, loff_t off, size_t count) {
	struct device *fbdev = container_of(kobj, struct device, kobj);
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;

	if (dev->edid == NULL)
		return 0;

	if ((off >= dev->edid_size) || (count > dev->edid_size))
		return 0;

	if (off + count > dev->edid_size)
		count = dev->edid_size - off;

	pr_info("sysfs edid copy %p to %p, %d bytes\n",
		dev->edid, buf, (int) count);

	memcpy(buf, dev->edid, count);

	return count;
}

static ssize_t edid_store(
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
			struct file *filp,
#endif
			struct kobject *kobj, struct bin_attribute *a,
			char *src, loff_t src_off, size_t src_size) {
	struct device *fbdev = container_of(kobj, struct device, kobj);
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;

	/* We only support write of entire EDID at once, no offset*/
	if ((src_size != EDID_LENGTH) || (src_off != 0))
		return 0;

	dlfb_setup_modes(dev, fb_info, src, src_size);

	if (dev->edid && (memcmp(src, dev->edid, src_size) == 0)) {
		pr_info("sysfs written EDID is new default\n");
		dlfb_ops_set_par(fb_info);
		return src_size;
	} else
		return 0;
}

static ssize_t metrics_reset_store(struct device *fbdev,
			   struct device_attribute *attr,
			   const char *buf, size_t count)
{
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
pr_info("metrics_reset_store\n");

	atomic_set(&dev->bytes_rendered, 0);
	atomic_set(&dev->bytes_identical, 0);
	atomic_set(&dev->bytes_sent, 0);
	atomic_set(&dev->deferred_calls, 0);
	atomic_set(&dev->deferred_bytes, 0);
	atomic_set(&dev->cpu_kcycles_used, 0);

	return count;
}

static ssize_t backlight_store(struct device *fbdev,
			       struct device_attribute *attr,
			       const char *buf, size_t count)
{
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;

	int intensity = 0;
pr_info("backlight_store\n");
	if ( sscanf( buf, "%d", &intensity ) > 0 )
	{	
	    if ( intensity < 0 )
		intensity = 0;
	    else if ( intensity > 100 )
		intensity = 100;
	    atomic_set(&dev->backlight_intensity, intensity);
            dlfb_set_backlight( dev, atomic_read(&dev->backlight_intensity) );
	}

	return count;
}

static ssize_t bootloader_store(struct device *fbdev,
			        struct device_attribute *attr,
			        const char *buf, size_t count)
{
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
pr_info("bootloader_store\n");

        dlfb_set_bootloader( dev );

	return count;
}


static ssize_t backlight_show(struct device *fbdev,
			      struct device_attribute *a, char *buf) 
{
	struct fb_info *fb_info = dev_get_drvdata(fbdev);
	struct dlfb_data *dev = fb_info->par;
pr_info("backlight_show\n");
	return snprintf(buf, PAGE_SIZE, "%u\n",
			atomic_read(&dev->backlight_intensity));
}


static struct bin_attribute edid_attr = {
	.attr.name = "edid",
	.attr.mode = 0666,
	.size = EDID_LENGTH,
	.read = edid_show,
	.write = edid_store
};

static struct device_attribute fb_device_attrs[] = {
	__ATTR_RO(metrics_bytes_rendered),
	__ATTR_RO(metrics_bytes_identical),
	__ATTR_RO(metrics_bytes_sent),
	__ATTR_RO(metrics_deferred_calls),
	__ATTR_RO(metrics_deferred_bytes),
	__ATTR_RO(metrics_cpu_kcycles_used),
	__ATTR(metrics_reset, S_IWUSR, NULL, metrics_reset_store),
	__ATTR(backlight_intensity, 0644, backlight_show, backlight_store),
	__ATTR(bootloader, S_IWUSR, NULL, bootloader_store),
};

/*
 * This is necessary before we can communicate with the display controller.
 */
static int dlfb_select_std_channel(struct dlfb_data *dev)
{
return 0;
/*
	int ret;
	u8 set_def_chn[] = {	   0x57, 0xCD, 0xDC, 0xA7,
				0x1C, 0x88, 0x5E, 0x15,
				0x60, 0xFE, 0xC6, 0x97,
				0x16, 0x3D, 0x47, 0xF2  };

	ret = usb_control_msg(dev->udev, usb_sndctrlpipe(dev->udev, 0),
			NR_USB_REQUEST_CHANNEL,
			(USB_DIR_OUT | USB_TYPE_VENDOR), 0, 0,
			set_def_chn, sizeof(set_def_chn), USB_CTRL_SET_TIMEOUT);
	return ret;
    */
}

static int dlfb_parse_vendor_descriptor(struct dlfb_data *dev,
					struct usb_interface *interface)
{
	char *desc;
	char *buf;
	char *desc_end;

	int total_len = 0;

pr_info("dlfb_parse_vendor_descriptor" );
	buf = kzalloc(MAX_VENDOR_DESCRIPTOR_SIZE, GFP_KERNEL);
	if (!buf)
		return false;
	desc = buf;

	total_len = usb_get_descriptor(interface_to_usbdev(interface),
					0x5f, /* vendor specific */
					0, desc, MAX_VENDOR_DESCRIPTOR_SIZE);

	/* if not found, look in configuration descriptor */
	if (total_len < 0) {
		if (0 == usb_get_extra_descriptor(interface->cur_altsetting,
			0x5f, &desc))
			total_len = (int) desc[0];
	}

	if (total_len > 5) {
		pr_info("vendor descriptor length:%x data:%02x %02x %02x %02x" \
			"%02x %02x %02x %02x %02x %02x %02x\n",
			total_len, desc[0],
			desc[1], desc[2], desc[3], desc[4], desc[5], desc[6],
			desc[7], desc[8], desc[9], desc[10]);

		if ((desc[0] != total_len) || /* descriptor length */
		    (desc[1] != 0x5f) ||   /* vendor descriptor type */
		    (desc[2] != 0x01) ||   /* version (2 bytes) */
		    (desc[3] != 0x00) ||
		    (desc[4] != total_len - 2)) /* length after type */
			goto unrecognized;

		desc_end = desc + total_len;
		desc += 5; /* the fixed header we've already parsed */

		while (desc < desc_end) {
			u8 length;
			u16 key;

			key = *((u16 *) desc);
			desc += sizeof(u16);
			length = *desc;
			desc++;

			switch (key) {
			case 0x0200: { /* max_area */
				u32 max_area;
				max_area = le32_to_cpu(*((u32 *)desc));
				pr_warn("DL chip limited to %d pixel modes\n",
					max_area);
				dev->sku_pixel_limit = max_area;
				break;
			}
			default:
				break;
			}
			desc += length;
		}
	} else {
		pr_info("vendor descriptor not available (%d)\n", total_len);
	}

	goto success;

unrecognized:
	/* allow usbfb to load for now even if firmware unrecognized */
	pr_err("Unrecognized vendor firmware descriptor\n");

success:
	kfree(buf);
	return true;
}
static int dlfb_usb_probe(struct usb_interface *interface,
			const struct usb_device_id *id)
{
	struct usb_device *usbdev;
	struct dlfb_data *dev = 0;
	struct fb_info *info = 0;
	int retval = -ENOMEM;
	int i;

	/* usb initialization */

	usbdev = interface_to_usbdev(interface);
pr_err("Probing - VID:%04X PID:%04X\n", usbdev->descriptor.idVendor, usbdev->descriptor.idProduct );

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (dev == NULL) {
		err("dlfb_usb_probe: failed alloc of dev struct\n");
		goto error;
	}

	/* we need to wait for both usb and fbdev to spin down on disconnect */
	kref_init(&dev->kref); /* matching kref_put in usb .disconnect fn */
	kref_get(&dev->kref); /* matching kref_put in free_framebuffer_work */

	dev->udev = usbdev;
	dev->gdev = &usbdev->dev; /* our generic struct device * */
	usb_set_intfdata(interface, dev);

	pr_info("%s %s - serial #%s\n",
		usbdev->manufacturer, usbdev->product, usbdev->serial);
	pr_info("vid_%04x&pid_%04x&rev_%04x driver's dlfb_data struct at %p\n",
		usbdev->descriptor.idVendor, usbdev->descriptor.idProduct,
		usbdev->descriptor.bcdDevice, dev);
	pr_info("console enable=%d\n", console);
	pr_info("fb_defio enable=%d\n", fb_defio);
	pr_info("shadow enable=%d\n", shadow);

	dev->sku_pixel_limit = DISPLAY_X * DISPLAY_Y; /* default to maximum */

	if (!dlfb_parse_vendor_descriptor(dev, interface)) {
		pr_err("firmware not recognized. Assume incompatible device\n");
		goto error;
	}

	if (!dlfb_alloc_urb_list(dev, WRITES_IN_FLIGHT, MAX_TRANSFER)) {
		retval = -ENOMEM;
		pr_err("dlfb_alloc_urb_list failed\n");
		goto error;
	}

	/* We don't register a new USB class. Our client interface is fbdev */

	/* allocates framebuffer driver structure, not framebuffer memory */
	info = framebuffer_alloc(0, &interface->dev);
	if (!info) {
		retval = -ENOMEM;
		pr_err("framebuffer_alloc failed\n");
		goto error;
	}

	dev->info = info;
	info->par = dev;
	info->pseudo_palette = dev->pseudo_palette;
	info->fbops = &dlfb_ops;

	retval = fb_alloc_cmap(&info->cmap, 256, 0);
	if (retval < 0) {
		pr_err("fb_alloc_cmap failed %x\n", retval);
		goto error;
	}

	INIT_DELAYED_WORK(&dev->free_framebuffer_work,
			  dlfb_free_framebuffer_work);

	INIT_LIST_HEAD(&info->modelist);

	retval = dlfb_setup_modes(dev, info, NULL, 0);
	if (retval != 0) {
		pr_err("unable to find common mode for display and adapter\n");
		goto error;
	}

	/* ready to begin using device */

	atomic_set(&dev->backlight_intensity, backlight);
	atomic_set(&dev->usb_active, 1);
	dlfb_select_std_channel(dev);

	dlfb_ops_check_var(&info->var, info);
	dlfb_ops_set_par(info);

{
int count = 0;
do
{
pr_err("13: info->dev=%p register count=%d\n", info->dev, count);
        lock_register_fb();
	retval = register_framebuffer(info);
        unlock_register_fb();
pr_err("13a: info->device=%p\n", info->device);
pr_err("13b: fb_class=%p\n", fb_class);
pr_err("14: info->dev=%p\n", info->dev);
	if (retval < 0) {
		pr_err("register_framebuffer failed %d\n", retval);
		goto error;
	}
pr_err("14a: retval=%d\n", retval);

pr_err("15: info->dev=%p\n", info->dev);
count++;
} while ( info->dev == NULL && count < 2 );
} 
if ( info->dev == NULL )
{
    pr_err("Failed to register framebuffer\n");
    goto error;
}
pr_err("fbdevice=%s\n", dev_name(info->dev));

	for (i = 0; i < ARRAY_SIZE(fb_device_attrs); i++) {
pr_err("device_create_file %s\n", fb_device_attrs[i].attr.name );
		retval = device_create_file(info->dev, &fb_device_attrs[i]);
		if (retval) {
			pr_err("device_create_file failed %d\n", retval);
			goto err_del_attrs;
		}
	}

pr_err("16: info->dev=%p\n", info->dev);
	retval = device_create_bin_file(info->dev, &edid_attr);
	if (retval) {
		pr_err("device_create_bin_file failed %d\n", retval);
		goto err_del_attrs;
	}

	pr_info("USBDisplay32 device /dev/fb%d attached. %dx%d resolution."
			" Using %dK framebuffer memory\n", info->node,
			info->var.xres, info->var.yres,
			((dev->backing_buffer) ?
			info->fix.smem_len * 2 : info->fix.smem_len) >> 10);
	return 0;

err_del_attrs:
pr_err("removing devices\n");
	for (i -= 1; i >= 0; i--)
		device_remove_file(info->dev, &fb_device_attrs[i]);

error:
	if (dev) {
pr_err("Releasing memory\n");

		if (info) {
pr_err("dealloc_cmap\n");
			if (info->cmap.len != 0)
				fb_dealloc_cmap(&info->cmap);
pr_err("destroy_modedb\n");
			if (info->monspecs.modedb)
				fb_destroy_modedb(info->monspecs.modedb);
pr_err("free screen_base\n");
			if (info->screen_base)
				vfree(info->screen_base);

pr_err("destroy modelist\n");
			fb_destroy_modelist(&info->modelist);

pr_err("release info\n");
			framebuffer_release(info);
		}

		if (dev->backing_buffer)
			vfree(dev->backing_buffer);

		kref_put(&dev->kref, dlfb_free); /* ref for framebuffer */
		kref_put(&dev->kref, dlfb_free); /* last ref from kref_init */

		/* dev has been deallocated. Do not dereference */
	}

	return retval;
}

static void dlfb_usb_disconnect(struct usb_interface *interface)
{
	struct dlfb_data *dev;
	struct fb_info *info;
	int i;

	dev = usb_get_intfdata(interface);
	info = dev->info;

	pr_info("USB disconnect starting\n");

	/* we virtualize until all fb clients release. Then we free */
	dev->virtualized = true;

	/* When non-active we'll update virtual framebuffer, but no new urbs */
	atomic_set(&dev->usb_active, 0);

	/* remove usbfb's sysfs interfaces */
	for (i = 0; i < ARRAY_SIZE(fb_device_attrs); i++)
		device_remove_file(info->dev, &fb_device_attrs[i]);
	device_remove_bin_file(info->dev, &edid_attr);

	usb_set_intfdata(interface, NULL);

	/* if clients still have us open, will be freed on last close */
	if (dev->fb_count == 0)
		schedule_delayed_work(&dev->free_framebuffer_work, 0);

	/* release reference taken by kref_init in probe() */
	kref_put(&dev->kref, dlfb_free);

	/* consider dlfb_data freed */

	return;
}

static struct usb_driver dlfb_driver = {
	.name = "usbfb",
	.probe = dlfb_usb_probe,
	.disconnect = dlfb_usb_disconnect,
	.id_table = id_table,
};

static int __init dlfb_module_init(void)
{
	int res;

	res = usb_register(&dlfb_driver);
	if (res)
		err("usb_register failed. Error number %d", res);
        sema_init(&register_fb_lock,1);

	return res;
}

static void __exit dlfb_module_exit(void)
{
	usb_deregister(&dlfb_driver);
}

module_init(dlfb_module_init);
module_exit(dlfb_module_exit);

static void dlfb_urb_completion(struct urb *urb)
{
	struct urb_node *unode = urb->context;
	struct dlfb_data *dev = unode->dev;
	unsigned long flags;

	/* sync/async unlink faults aren't errors */
	if (urb->status) {
		if (!(urb->status == -ENOENT ||
		    urb->status == -ECONNRESET ||
		    urb->status == -ESHUTDOWN)) {
			pr_err("%s - nonzero write bulk status received: %d\n",
				__func__, urb->status);
			atomic_set(&dev->lost_pixels, 1);
		}
	}

	urb->transfer_buffer_length = dev->urbs.size; /* reset to actual */

	spin_lock_irqsave(&dev->urbs.lock, flags);
	list_add_tail(&unode->entry, &dev->urbs.list);
	dev->urbs.available++;
	spin_unlock_irqrestore(&dev->urbs.lock, flags);

	/*
	 * When using fb_defio, we deadlock if up() is called
	 * while another is waiting. So queue to another process.
	 */
	if (fb_defio)
		schedule_delayed_work(&unode->release_urb_work, 0);
	else
		up(&dev->urbs.limit_sem);
}

static void dlfb_free_urb_list(struct dlfb_data *dev)
{
	int count = dev->urbs.count;
	struct list_head *node;
	struct urb_node *unode;
	struct urb *urb;
	int ret;
	unsigned long flags;

	pr_notice("Waiting for completes and freeing all render urbs\n");

	/* keep waiting and freeing, until we've got 'em all */
	while (count--) {

		/* Getting interrupted means a leak, but ok at shutdown*/
		ret = down_interruptible(&dev->urbs.limit_sem);
		if (ret)
			break;

		spin_lock_irqsave(&dev->urbs.lock, flags);

		node = dev->urbs.list.next; /* have reserved one with sem */
		list_del_init(node);

		spin_unlock_irqrestore(&dev->urbs.lock, flags);

		unode = list_entry(node, struct urb_node, entry);
		urb = unode->urb;

		/* Free each separately allocated piece */
		usb_free_coherent(urb->dev, dev->urbs.size,
				  urb->transfer_buffer, urb->transfer_dma);
		usb_free_urb(urb);
		kfree(node);
	}

}

static int dlfb_alloc_urb_list(struct dlfb_data *dev, int count, size_t size)
{
	int i = 0;
	struct urb *urb;
	struct urb_node *unode;
	char *buf;

//pr_info("alloc_urb_list count=%d, size=%d\n", count, size);
	spin_lock_init(&dev->urbs.lock);

	dev->urbs.size = size;
	INIT_LIST_HEAD(&dev->urbs.list);

	while (i < count) {
		unode = kzalloc(sizeof(struct urb_node), GFP_KERNEL);
		if (!unode)
			break;
		unode->dev = dev;

		INIT_DELAYED_WORK(&unode->release_urb_work,
			  dlfb_release_urb_work);

		urb = usb_alloc_urb(0, GFP_KERNEL);
		if (!urb) {
			kfree(unode);
			break;
		}
		unode->urb = urb;

		buf = usb_alloc_coherent(dev->udev, MAX_TRANSFER, GFP_KERNEL,
					 &urb->transfer_dma);
		if (!buf) {
			kfree(unode);
			usb_free_urb(urb);
			break;
		}

		/* urb->transfer_buffer_length set to actual before submit */
		usb_fill_bulk_urb(urb, dev->udev, usb_sndbulkpipe(dev->udev, ENDPOINT_NO),
			buf, size, dlfb_urb_completion, unode);
		urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

		list_add_tail(&unode->entry, &dev->urbs.list);

		i++;
	}

	sema_init(&dev->urbs.limit_sem, i);
	dev->urbs.count = i;
	dev->urbs.available = i;

	pr_notice("allocated %d %d byte urbs\n", i, (int) size);

	return i;
}

static struct urb *dlfb_get_urb(struct dlfb_data *dev)
{
	int ret = 0;
	struct list_head *entry;
	struct urb_node *unode;
	struct urb *urb = NULL;
	unsigned long flags;

	/* Wait for an in-flight buffer to complete and get re-queued */
	ret = down_timeout(&dev->urbs.limit_sem, GET_URB_TIMEOUT);
	if (ret) {
		atomic_set(&dev->lost_pixels, 1);
		pr_warn("wait for urb interrupted: %x available: %d\n",
		       ret, dev->urbs.available);
		goto error;
	}

	spin_lock_irqsave(&dev->urbs.lock, flags);

	BUG_ON(list_empty(&dev->urbs.list)); /* reserved one with limit_sem */
	entry = dev->urbs.list.next;
	list_del_init(entry);
	dev->urbs.available--;

	spin_unlock_irqrestore(&dev->urbs.lock, flags);

	unode = list_entry(entry, struct urb_node, entry);
	urb = unode->urb;

error:
	return urb;
}

static int dlfb_submit_urb(struct dlfb_data *dev, struct urb *urb, size_t len)
{
	int ret;

	//pr_info("len=%d,size=%d\n", len, dev->urbs.size);
	BUG_ON(len > dev->urbs.size);

	urb->transfer_buffer_length = len; /* set to actual payload len */
	ret = usb_submit_urb(urb, GFP_KERNEL);
	if (ret) {
		dlfb_urb_completion(urb); /* because no one else will */
		atomic_set(&dev->lost_pixels, 1);
		pr_err("usb_submit_urb error %x\n", ret);
	}
	return ret;
}

module_param(console, bool, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
MODULE_PARM_DESC(console, "Allow fbcon to open framebuffer");

module_param(fb_defio, bool, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
MODULE_PARM_DESC(fb_defio, "Page fault detection of mmap writes");

module_param(shadow, bool, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
MODULE_PARM_DESC(shadow, "Shadow vid mem. Disable to save mem but lose perf");

module_param(backlight, int, S_IWUSR | S_IRUSR | S_IWGRP | S_IRGRP);
MODULE_PARM_DESC(backlight, "Set backlight 0-100");

MODULE_AUTHOR("Roberto De Ioris <roberto@unbit.it>, "
	      "Jaya Kumar <jayakumar.lkml@gmail.com>, "
	      "Bernie Thompson <bernie@plugable.com>");
MODULE_DESCRIPTION("USBDisplay32 kernel framebuffer driver");
MODULE_LICENSE("GPL");

