#include "utils.h"

/* Console for low-power suspend/resume */
#if CONFIG_SERIAL
static const struct device *cons = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
#endif

/* ---------------- Sleep ---------------- */
int sleep_mcu(int32_t ms)
{
#if CONFIG_SERIAL
    pm_device_action_run(cons, PM_DEVICE_ACTION_SUSPEND);
#endif
    k_msleep(ms);
#if CONFIG_SERIAL
    pm_device_action_run(cons, PM_DEVICE_ACTION_RESUME);
#endif
return 0;
};