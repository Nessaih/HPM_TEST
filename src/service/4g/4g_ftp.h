#ifndef TBOX_4G_FTP_H
#define TBOX_4G_FTP_H

#include "4g_if.h"

void ftp_4g_init(void);

void ftp_4g_download(uint8 *url, uint16 len,
                     uint8 context_id,
                     IF_FTP_4G_DOWNLOAD_CALLBACK call_bak);

void ftp_4g_period(void);

boolean ftp_4g_isdownloading(void);

uint8 ftp_4g_stat(void);


#endif /* TBOX_4G_FTP_H */
