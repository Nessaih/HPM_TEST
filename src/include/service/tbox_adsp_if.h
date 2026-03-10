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

typedef enum
{
    TBOX_ADSP_MATCH_OK   = 0,
    TBOX_ADSP_NOT_MATCH,
    TBOX_ADSP_MATCH_NEED_MORE_DATA
}TBOX_ADSP_MATCH_RESULT;

typedef enum
{
    TBOX_ADSP_PROCESS_OK = 0,
    TBOX_ADSP_NOT_PROCESS,
    TBOX_ADSP_PROCESS_NEED_MORE_DATA
}TBOX_ADSP_PROCESS_RESULT;

typedef TBOX_ADSP_MATCH_RESULT (*TBOX_ADSP_MATCH_FUNC)(UINT8 *data, UINT32 len);
typedef TBOX_ADSP_PROCESS_RESULT (*TBOX_ADSP_PROCESS_FUNC)(UINT8 *data, UINT32 len);
typedef BOOL (*TBOX_ADSP_IS_EXIT_FUNC)(UINT8 *data, UINT32 len);

INT32 tbox_adsp_register(TBOX_ADSP_TYPE type, 
                         TBOX_ADSP_MATCH_FUNC match_func, 
                         TBOX_ADSP_PROCESS_FUNC process_func, 
                         TBOX_ADSP_IS_EXIT_FUNC is_exit_func);

#ifdef __cplusplus
}
#endif

#endif /* TBOX_ADSP_IF_H */