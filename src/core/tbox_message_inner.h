#ifndef TBOX_MESSAGE_INNER_H
#define TBOX_MESSAGE_INNER_H

#include "tbox_message.h"

#ifdef __cplusplus
extern "C" {
#endif

/*message*/
INT32 tbox_message_init(VOID);
VOID tbox_message_deinit(VOID);
VOID tbox_message_reset(VOID);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MESSAGE_INNER_H */