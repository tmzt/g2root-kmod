#include <linux/module.h>
#include <linux/init.h>

#include <linux/pagemap.h>
#include <linux/unistd.h>

#include <asm/io.h>

static char *hex2string(uint8_t *data, int len)
{
        static char buf[1024];
        int i;

        i = (sizeof(buf) - 1) / 4;
        if (len > i)
                len = i;

        for (i = 0; i < len; i++)
                sprintf(buf + i * 4, "[%02X]", data[i]);

        return buf;
}

static int __init test_init(void) {
    void *virt = NULL;
    uint off = 0;
    uint8_t line[256];
    char *data;
    int i, j;

    printk("test module loaded\n");
    printk("dumping 4K at 0x0004FC00\n");
    printk("\n");

    virt = ioremap(0x0004FC00, PAGE_SIZE);
    for(i = 0; i < 256; i++) {
        for(j = 0; j < 16; j++) {
            line[j] = readb(virt+off);
            off++;
        }
        data = hex2string(&line, 16);
    }

    printk("\n\ndone.\n");
                
	return 0;
}

static void __exit test_exit(void) {
        printk("test module unloaded\n");
}

module_init(test_init);
module_exit(test_exit);

MODULE_LICENSE("GPL");
