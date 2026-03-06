#ifndef __GNSS_H__
#define __GNSS_H__

#define GNSS_SVR_CYCLE_INTV   (1*1000U)

#define GNSS_SVR_TIMER_EVENT  "GNSS_SVR_TIMEOUT_EVENT"

extern SemaphoreHandle_t  	 gnss_mutex;

#define GNSS_MUTEX_LOCK()     xSemaphoreTake(gnss_mutex, portMAX_DELAY)
#define GNSS_MUTEX_UNLOCK()   xSemaphoreGive(gnss_mutex)

#endif
