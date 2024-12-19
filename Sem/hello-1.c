#include <linux/init.h>
#include <linux/module.h>
#include <linux/printk.h>

static int __init custom_init(void) {
    pr_info("Hello world?\n");
    return 0;
}

static int __exit custom_exit(void) {
    pr_info("Goodbye world?\n");
}

module_init(custom_init);
module_exit(custom_exit);

MODULE_LICENSE("GPL");