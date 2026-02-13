#ifndef __GNSS_H__
#define __GNSS_H__

extern SemaphoreHandle_t  	 gnss_mutex;

#define GNSS_MUTEX_LOCK()     xSemaphoreTake(gnss_mutex, portMAX_DELAY)
#define GNSS_MUTEX_UNLOCK()   xSemaphoreGive(gnss_mutex)

#endif
