#ifndef __PGN_H__
#define __PGN_H__

#define J1939_REQ_SA                        0x21
#define J1939_REQ_DA                        0x00

// PF < 240 : PGN with address
#define J1939_PGN_ACK_1                     0xE800
#define J1939_PGN_ACK_2                     0xE8D8
#define J1939_PGN_ACK_3                     0xE8FF

#define J1939_PGN_REQ_1                     0xEA00
#define J1939_PGN_REQ_2                     0xEAD8
#define J1939_PGN_REQ_3                     0xEAFF

#define J1939_PGN_DT_1                      0xEB00
#define J1939_PGN_DT_2                      0xEBD8
#define J1939_PGN_DT_3                      0xEBFF

#define J1939_PGN_CM_1                      0xEC00
#define J1939_PGN_CM_2                      0xECD8
#define J1939_PGN_CM_3                      0xECFF

// PF >= 240 : PGN with extension

#define J1939_PGN_ENGINE_CONSTRUCTION       0xFEE3
#define J1939_PGN_ENGINE_RUN_TIME           0xFEE5
#define J1939_PGN_FUEL_CONSUMPTION          0xFEE9
#define J1939_PGN_INGREDIENT_IDENTIFICATION 0xFEEB
#define J1939_PGN_VIN                       0xFEEC
#define J1939_PGN_DM1                       0xFECA

#define J1939_PGN_DM2                       0xFECB // not uesd
#define J1939_PGN_DM5                       0xFECE // not uesd
#define J1939_PGN_DM19                      0xD300 // not uesd
#define J1939_PGN_DM20                      0xC200 // not uesd
#define J1939_PGN_DM26                      0xFDB8 // not uesd

void j1939_pgn_init(void);
void j1939_pgn_show(void);
bool j1939_pgn_add(uint8_t addr, uint32_t pgn);
bool j1939_pgn_filter(uint8_t addr, uint32_t pgn);
void j1939_pgn_req(uint8_t channel, uint32_t pgn, uint8_t dstaddr,uint8_t saaddr);

#endif //__PGN_H__