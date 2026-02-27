#ifndef __HPM_NAT_H__
#define __HPM_NAT_H__

INT32 hpm_net_init(UINT8 seq);

BOOL hpm_net_is_ready(VOID);

INT32 hpm_net_connect(UINT8 *ip, UINT16 port);

INT32 hpm_net_disconnect(VOID);

INT32 hpm_net_send(UINT8 *data, UINT16 len);

#endif
