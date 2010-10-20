#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/console.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/pm.h>
#include <linux/proc_fs.h>
#include <linux/suspend.h>
#include <linux/reboot.h>
#include <linux/earlysuspend.h>
#include <linux/pm_qos_params.h>
#include <mach/msm_iomap.h>
#include <mach/system.h>
#include <asm/io.h>

//include "../arch/arm/mach-msm/smd_private.h"
#include "smd_private1.h"
#include "../arch/arm/mach-msm/smd_rpcrouter.h"
#include "../arch/arm/mach-msm/acpuclock.h"
#include "../arch/arm/mach-msm/proc_comm.h"
#include "../arch/arm/mach-msm/clock.h"
#include "../arch/arm/mach-msm/spm.h"
#include "../arch/arm/mach-msm/pm.h"
#ifdef CONFIG_HAS_WAKELOCK
#include <linux/wakelock.h>
#endif

#define A11S_CLK_SLEEP_EN (MSM_GCC_BASE + 0x020)
#define A11S_PWRDOWN      (MSM_ACC_BASE + 0x01c)
#define A11S_SECOP        (MSM_TCSR_BASE + 0x038)

/* Set magic number in SMEM for power collapse state */
#define HTC_POWER_COLLAPSE_ADD  (MSM_SHARED_RAM_BASE + 0x000F8000 + 0x000007F8)
#define HTC_POWER_COLLAPSE_MAGIC_NUM    (HTC_POWER_COLLAPSE_ADD - 0x04)
unsigned int magic_num;

#if defined(CONFIG_MSM_N_WAY_SMD)
#define DEM_MASTER_BITS_PER_CPU             6

/* Power Master State Bits - Per CPU */
#define DEM_MASTER_SMSM_RUN \
        (0x01UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))
#define DEM_MASTER_SMSM_RSA \
        (0x02UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))
#define DEM_MASTER_SMSM_PWRC_EARLY_EXIT \
        (0x04UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))
#define DEM_MASTER_SMSM_SLEEP_EXIT \
        (0x08UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))
#define DEM_MASTER_SMSM_READY \
        (0x10UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))
#define DEM_MASTER_SMSM_SLEEP \
        (0x20UL << (DEM_MASTER_BITS_PER_CPU * SMSM_STATE_APPS))

/* Power Slave State Bits */
#define DEM_SLAVE_SMSM_RUN                  (0x0001)
#define DEM_SLAVE_SMSM_PWRC                 (0x0002)
#define DEM_SLAVE_SMSM_PWRC_DELAY           (0x0004)
#define DEM_SLAVE_SMSM_PWRC_EARLY_EXIT      (0x0008)
#define DEM_SLAVE_SMSM_WFPI                 (0x0010)
#define DEM_SLAVE_SMSM_SLEEP                (0x0020)
#define DEM_SLAVE_SMSM_SLEEP_EXIT           (0x0040)
#define DEM_SLAVE_SMSM_MSGS_REDUCED         (0x0080)
#define DEM_SLAVE_SMSM_RESET                (0x0100)
#define DEM_SLAVE_SMSM_PWRC_SUSPEND         (0x0200)

#define PM_SMSM_WRITE_STATE	SMSM_STATE_APPS_DEM
#define PM_SMSM_READ_STATE	SMSM_STATE_POWER_MASTER_DEM

#define PM_SMSM_WRITE_RUN	DEM_SLAVE_SMSM_RUN
#define PM_SMSM_READ_RUN	DEM_MASTER_SMSM_RUN
#else
#define PM_SMSM_WRITE_STATE     SMSM_STATE_APPS
#define PM_SMSM_READ_STATE      SMSM_STATE_MODEM

#define PM_SMSM_WRITE_RUN       SMSM_RUN
#define PM_SMSM_READ_RUN        SMSM_RUN
#endif

extern int msm_pm_collapse(void);
extern int msm_arch_idle(void);
extern void msm_pm_collapse_exit(void);

uint32_t(*smsm_get_state)(enum smsm_state_item) = (void *)0xc0043684;
int(*smsm_change_state)(enum smsm_state_item, uint32_t, uint32_t) = (void *)0xc0043700;
void(*smsm_print_sleep_info)(unsigned) = (void *)0xc00447d0;
int(*__cpu_early_init)(void) = (void *)0xc003deb8;

static int
msm_pm_wait_state(uint32_t wait_all_set, uint32_t wait_all_clear,
                  uint32_t wait_any_set, uint32_t wait_any_clear)
{
	int i;
	uint32_t state;

	for (i = 0; i < 100000; i++) {
		state = smsm_get_state(PM_SMSM_READ_STATE);
		if (((wait_all_set || wait_all_clear) && 
		     !(~state & wait_all_set) && !(state & wait_all_clear)) ||
		    (state & wait_any_set) || (~state & wait_any_clear))
			return 0;
		udelay(1);
	}
	pr_err("msm_pm_wait_state(%x, %x, %x, %x) failed %x\n",	wait_all_set,
		wait_all_clear, wait_any_set, wait_any_clear, state);
	return -ETIMEDOUT;
}

static void apps_sleep(void) {
    uint32_t saved_vector[2];
    //unsigned int sleep_delay = 192000*5;
    static uint32_t *msm_pm_reset_vector;
	uint32_t enter_state;
	uint32_t enter_wait_set = 0;
	uint32_t enter_wait_clear = 0;
	uint32_t exit_state;
//	uint32_t exit_wait_clear = 0;
	uint32_t exit_wait_any_set = 0;

    int ret;

	enter_state = DEM_SLAVE_SMSM_SLEEP;
	enter_wait_set = DEM_MASTER_SMSM_SLEEP;
	exit_state = DEM_SLAVE_SMSM_SLEEP_EXIT;
	exit_wait_any_set = DEM_MASTER_SMSM_SLEEP_EXIT;

    ret = smsm_change_state(PM_SMSM_WRITE_STATE, PM_SMSM_WRITE_RUN, enter_state);
    printk("smsm_change_state result: %d\n", ret);

    ret = msm_pm_wait_state(enter_wait_set, enter_wait_clear, 0, 0);
    printk("msm_pm_wait_state result: %d\n", ret);

//    msm_enter_prep_hw();
    writel(1, A11S_PWRDOWN);
    writel(4, A11S_SECOP);
    printk("A11S_PWRDOWN: %.8x\n", readl(A11S_PWRDOWN));
    printk("A11S_SECOP: %.8x\n", readl(A11S_SECOP));

    printk("msm_sleep(): enter "
           "A11S_CLK_SLEEP_EN %x, A11S_PWRDOWN %x, "
           "smsm_get_state %x\n", readl(A11S_CLK_SLEEP_EN),
           readl(A11S_PWRDOWN), smsm_get_state(PM_SMSM_READ_STATE));

    magic_num = 0xAAAA1111;
    writel(magic_num, HTC_POWER_COLLAPSE_MAGIC_NUM);

    smsm_print_sleep_info(0);
    saved_vector[0] = msm_pm_reset_vector[0];
    saved_vector[1] = msm_pm_reset_vector[1];
    msm_pm_reset_vector[0] = 0xE51FF004; /* ldr pc, 4 */
    msm_pm_reset_vector[1] = virt_to_phys(msm_pm_collapse_exit);
    printk(KERN_INFO "msm_sleep(): vector %x %x -> "
           "%x %x\n", saved_vector[0], saved_vector[1],
           msm_pm_reset_vector[0], msm_pm_reset_vector[1]);
};

static int __init pm_init(void) {
    printk("pm1 module loaded\n");
    printk("attempting apps sleep\n");
    apps_sleep();
    return 0;
};

static void __exit pm_exit(void) {
    printk("pm1 module unloaded\n");
};

module_init(pm_init);
module_exit(pm_exit);

MODULE_LICENSE("GPL");
















