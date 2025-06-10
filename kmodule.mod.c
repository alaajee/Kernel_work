#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_MITIGATION_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

KSYMTAB_FUNC(create_Serveur, "", "");
KSYMTAB_FUNC(client_handle, "", "");
KSYMTAB_FUNC(treat_task, "", "");
KSYMTAB_FUNC(work_cpu, "", "");
KSYMTAB_FUNC(net_cpu, "", "");

SYMBOL_CRC(create_Serveur, 0x95524e49, "");
SYMBOL_CRC(client_handle, 0xf1de0b42, "");
SYMBOL_CRC(treat_task, 0x61f77034, "");
SYMBOL_CRC(work_cpu, 0x51fd7246, "");
SYMBOL_CRC(net_cpu, 0x906cdd67, "");

static const char ____versions[]
__used __section("__versions") =
	"\x18\x00\x00\x00\xed\x25\xcd\x49"
	"alloc_workqueue\0"
	"\x18\x00\x00\x00\x36\xf2\xb6\xc5"
	"queue_work_on\0\0\0"
	"\x14\x00\x00\x00\x81\x90\x8a\xbd"
	"kernel_bind\0"
	"\x10\x00\x00\x00\xba\x0c\x7a\x03"
	"kfree\0\0\0"
	"\x1c\x00\x00\x00\x6e\x64\xf7\xb3"
	"kthread_should_stop\0"
	"\x18\x00\x00\x00\x98\x2a\x26\xfe"
	"kernel_accept\0\0\0"
	"\x18\x00\x00\x00\xee\x92\x89\x8d"
	"kernel_recvmsg\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x18\x00\x00\x00\x95\xde\x72\x3d"
	"wake_up_process\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x14\x00\x00\x00\x55\x70\xa8\x6a"
	"init_net\0\0\0\0"
	"\x1c\x00\x00\x00\x63\xa5\x03\x4c"
	"random_kmalloc_seed\0"
	"\x20\x00\x00\x00\xdd\x04\xd0\x33"
	"kernel_sock_shutdown\0\0\0\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x20\x00\x00\x00\xfa\xea\x96\x1a"
	"kthread_create_on_node\0\0"
	"\x18\x00\x00\x00\xf6\xd1\x77\x41"
	"kernel_listen\0\0\0"
	"\x1c\x00\x00\x00\xf0\x14\xc5\x56"
	"sock_create_kern\0\0\0\0"
	"\x20\x00\x00\x00\xee\xfb\xb4\x10"
	"__kmalloc_cache_noprof\0\0"
	"\x18\x00\x00\x00\x14\x91\xff\x61"
	"sock_release\0\0\0\0"
	"\x18\x00\x00\x00\x77\x57\xef\x3d"
	"kernel_sendmsg\0\0"
	"\x18\x00\x00\x00\x1d\x07\x60\x20"
	"kmalloc_caches\0\0"
	"\x18\x00\x00\x00\xde\x9f\x8a\x25"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "9AF04BD1B62D811CE338125");
