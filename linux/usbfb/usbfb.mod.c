#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x9b800190, "module_layout" },
	{ 0x1f76b17, "device_remove_file" },
	{ 0x2d2e3cb9, "device_remove_bin_file" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0x9924c496, "__usb_get_extra_descriptor" },
	{ 0xd6ee688f, "vmalloc" },
	{ 0x15692c87, "param_ops_int" },
	{ 0xe809f812, "framebuffer_release" },
	{ 0x1c0e6274, "dev_set_drvdata" },
	{ 0x487f831, "fb_find_best_display" },
	{ 0x5f722efd, "malloc_sizes" },
	{ 0xc9561772, "fb_destroy_modelist" },
	{ 0xa1b759ce, "fb_add_videomode" },
	{ 0x8968d9bb, "fb_sys_read" },
	{ 0x2addc0be, "down_interruptible" },
	{ 0x1976aa06, "param_ops_bool" },
	{ 0xfb0e29f, "init_timer_key" },
	{ 0xe4b05d73, "mutex_unlock" },
	{ 0x999e8297, "vfree" },
	{ 0xe2abc877, "sys_copyarea" },
	{ 0x510336a, "usb_get_descriptor" },
	{ 0x3744cf36, "vmalloc_to_pfn" },
	{ 0xf97456ea, "_raw_spin_unlock_irqrestore" },
	{ 0x43b23f34, "usb_deregister" },
	{ 0x50eedeb8, "printk" },
	{ 0x20c55ae0, "sscanf" },
	{ 0x7a890c8, "fb_alloc_cmap" },
	{ 0x5152e605, "memcmp" },
	{ 0x51ef16a8, "register_framebuffer" },
	{ 0x2da418b5, "copy_to_user" },
	{ 0x16305289, "warn_slowpath_null" },
	{ 0x432c627e, "mutex_lock" },
	{ 0xf05ffa15, "fb_var_to_videomode" },
	{ 0xdd1a2871, "down" },
	{ 0xe3a846b8, "usb_free_coherent" },
	{ 0xa598e29c, "vesa_modes" },
	{ 0x2616b23a, "device_create_file" },
	{ 0x98b71c6, "fb_dealloc_cmap" },
	{ 0xf01ebb70, "usb_submit_urb" },
	{ 0x708a1153, "kmem_cache_alloc" },
	{ 0xda8af7ad, "fb_find_nearest_mode" },
	{ 0x29f2a053, "sys_fillrect" },
	{ 0x77edf722, "schedule_delayed_work" },
	{ 0x79a3b6ae, "sys_imageblit" },
	{ 0xc06b9161, "fb_sys_write" },
	{ 0xff9ca065, "fb_edid_to_monspecs" },
	{ 0x1e9d44a0, "device_create_bin_file" },
	{ 0x1dc36131, "fb_destroy_modedb" },
	{ 0x21fb443e, "_raw_spin_lock_irqsave" },
	{ 0x49799b97, "framebuffer_alloc" },
	{ 0xcdef7632, "fb_deferred_io_cleanup" },
	{ 0xe7aad1db, "fb_deferred_io_init" },
	{ 0x37a0cba, "kfree" },
	{ 0xe2fa3293, "remap_pfn_range" },
	{ 0x2e60bace, "memcpy" },
	{ 0xc4554217, "up" },
	{ 0x1bc6e1ed, "usb_register_driver" },
	{ 0xb81960ca, "snprintf" },
	{ 0xba8a2c75, "usb_alloc_coherent" },
	{ 0x33d169c9, "_copy_from_user" },
	{ 0x9d9c6fce, "dev_get_drvdata" },
	{ 0xc906fe05, "usb_free_urb" },
	{ 0xb326f2e4, "down_timeout" },
	{ 0xc22de927, "usb_alloc_urb" },
	{ 0x934d8b7f, "fb_class" },
	{ 0x65f3ad9a, "fb_videomode_to_var" },
	{ 0xc23c4099, "unregister_framebuffer" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=usbcore,fb_sys_fops,syscopyarea,sysfillrect,sysimgblt";

MODULE_ALIAS("usb:v1C40p04DCd*dc*dsc*dp*ic*isc*ip*");
