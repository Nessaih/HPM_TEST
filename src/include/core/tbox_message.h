#ifndef TBOX_MESSAGE_H
#define TBOX_MESSAGE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum tag_tbox_msg_priority
{
    TBOX_MSG_PRIORITY_LOW   = 0U,
    TBOX_MSG_PRIORITY_NORMAL,
    TBOX_MSG_PRIORITY_HIGH,
    TBOX_MSG_PRIORITY_MAX,
}TBOX_MSG_PRIORITY;

typedef enum tag_tbox_msg_type
{
    TBOX_MSG_TYPE_MESSAGE   = 0U,
    TBOX_MSG_TYPE_TOPIC,
    TBOX_MSG_TYPE_MAX,
}TBOX_MSG_TYPE;

typedef struct  tag_tbox_msg_reginfo
{
    CHAR *name;
    UINT8 priority;
    UINT8 type;
    BOOL  enable;
}TBOX_MSG_REGINFO;

typedef struct tag_tbox_msg_data
{
    UINT8 *data;
    UINT16 size;
}TBOX_MSG_DATA;

typedef VOID (*TBOX_MSG_HADNLER)(const CHAR *name, TBOX_MSG_DATA *data);

INT32 tbox_message_register(const TBOX_MSG_REGINFO *reginfo);
VOID tbox_message_unregister(const CHAR *name);
VOID tbox_message_enable(const CHAR *name, BOOL enable);
BOOL tbox_message_is_enabled(const CHAR *name);

INT32 tbox_message_add_handler(const CHAR *name, TBOX_ID dst_id, TBOX_MSG_HADNLER handler);
INT32 tbox_message_send(const CHAR *name,  TBOX_ID src_id, TBOX_ID dst_id, TBOX_MSG_DATA *data);

INT32 tbox_message_subscribe(const CHAR *name, TBOX_ID dst_id, TBOX_MSG_HADNLER handler);
INT32 tbox_message_publish(const CHAR *name, TBOX_MSG_DATA *data);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_MESSAGE_H */