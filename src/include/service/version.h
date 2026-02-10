#ifndef VERSION_H
#define VERSION_H

#ifdef __cplusplus
extern "C" {
#endif

#define VERION_MAX_LEN    64
#define VERSION_TYPE_BOOT 0U
#define VERSION_TYPE_APP  1U

const CHAR *version_get(UINT8 type);
VOID  version_print(UINT8 type);
VOID  version_show(VOID);

#ifdef __cplusplus
}
#endif

#endif //VERSION_H