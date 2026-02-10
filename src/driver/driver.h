#ifndef __DRIVER_H__
#define __DRIVER_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void driver_init(void);

#ifndef __BOOTLOADER__
extern void driver_sleep(void);
extern void driver_wake(void);
#endif

#ifdef __cplusplus
}
#endif

#endif //__DRIVER_H__