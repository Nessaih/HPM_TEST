#ifndef TBOX_ADSP_IF_H
#define TBOX_ADSP_IF_H

/*辅助调试串口模块，可以同时通知满足shell,fct,上位机通信的需求*/
#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    TBOX_ADSP_TYPE_NONE = -1,      /*无*/
    TBOX_ADSP_TYPE_ADT,       /*辅助调试工具*/
    TBOX_ADSP_TYPE_FCT,       /*FCT*/
    TBOX_ADSP_TYPE_SHELL,     /*SHELL*/
    TBOX_ADSP_TYPE_MAX
}TBOX_ADSP_TYPE;

typedef BOOL (*TBOX_ADSP_IS_MATCH_FUNC)(UINT8 *data, UINT32 len);
typedef VOID (*TBOX_ADSP_CB_FUNC)(UINT8 *data, UINT32 len);
typedef BOOL (*TBOX_ADSP_IS_EXIT_FUNC)(UINT8 *data, UINT32 len);

INT32 tbox_adsp_register(TBOX_ADSP_TYPE type, 
                         TBOX_ADSP_IS_MATCH_FUNC is_match_func, 
                         TBOX_ADSP_CB_FUNC cb_func, 
                         TBOX_ADSP_IS_EXIT_FUNC is_exit_func);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_ADSP_IF_H */