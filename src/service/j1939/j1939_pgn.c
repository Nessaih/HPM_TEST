

#include "tbox_log.h"
#include "j1939_includes.h"

#define PGN_TABLE_SIZE 64

union PGN_INFO
{
    struct
    {
        uint32_t addr : 8;
        uint32_t pgn  : 24;
    };
    uint32_t value;
};

struct PGN_TABLE
{
    uint32_t       table_count;
    union PGN_INFO table[PGN_TABLE_SIZE];
};

// clang-format off
struct PGN_TABLE pgn_table = {
    .table_count = 18,
    .table       = {
        {.pgn = J1939_PGN_ACK_1                         },
        {.pgn = J1939_PGN_ACK_2                         },
        {.pgn = J1939_PGN_ACK_3                         },
        {.pgn = J1939_PGN_REQ_1                         },
        {.pgn = J1939_PGN_REQ_2                         },
        {.pgn = J1939_PGN_REQ_3                         },
        {.pgn = J1939_PGN_CM_1                          },
        {.pgn = J1939_PGN_CM_2                          },
        {.pgn = J1939_PGN_CM_3                          },
        {.pgn = J1939_PGN_DT_1                          },
        {.pgn = J1939_PGN_DT_2                          },
        {.pgn = J1939_PGN_DT_3                          },
        {.pgn = J1939_PGN_ENGINE_CONSTRUCTION           },
        {.pgn = J1939_PGN_ENGINE_RUN_TIME               },
        {.pgn = J1939_PGN_FUEL_CONSUMPTION              },
        {.pgn = J1939_PGN_INGREDIENT_IDENTIFICATION     },
        {.pgn = J1939_PGN_VIN                           },
        {.pgn = J1939_PGN_DM1                           },
    },
};
// clang-format on


void j1939_pgn_init(void)
{
    uint32_t i, j, m;
    uint32_t temp_value;
    uint32_t count = pgn_table.table_count;

    for (i = 0; i < count - 1; i++)
    {
        m = i;
        for (j = i + 1; j < count; j++)
        {
            if (pgn_table.table[j].pgn < pgn_table.table[m].pgn)
                m = j;
        }

        if (m != i)
        {
            temp_value               = pgn_table.table[i].value;
            pgn_table.table[i].value = pgn_table.table[m].value;
            pgn_table.table[m].value = temp_value;
        }
    }
}

void j1939_pgn_show(void)
{
    uint32_t i;

    for (i = 0; i < pgn_table.table_count; i++)
    {
        LOG_PRINT("%2u addr: %2X    pgn: %u\n", (unsigned int)i, (unsigned int)pgn_table.table[i].addr, (unsigned int)pgn_table.table[i].pgn);
    }
}

bool j1939_pgn_add(uint8_t addr, uint32_t pgn)
{
    uint32_t i, j;

    if (pgn > DL_PGN_MASK)
    {
        return false;
    }

    if (pgn_table.table_count >= PGN_TABLE_SIZE)
    {
        return false;
    }

    for (i = 0; i < pgn_table.table_count; i++)
    {
        if (pgn < pgn_table.table[i].pgn)
            break;
    }

    for (j = pgn_table.table_count; j > i; j--)
    {
        pgn_table.table[j].value = pgn_table.table[j - 1].value;
    }

    pgn_table.table[i].addr = addr;
    pgn_table.table[i].pgn  = pgn;
    pgn_table.table_count++;

    return true;
}

bool j1939_pgn_filter(uint8_t addr, uint32_t pgn)
{
    int32_t l = 0;
    int32_t m = 0;
    int32_t r = pgn_table.table_count - 1;

    while (l <= r)
    {
        m = (l + r) >> 1;
        if (pgn > pgn_table.table[m].pgn)
        {
            l = m + 1;
        }
        else if (pgn < pgn_table.table[m].pgn)
        {
            r = m - 1;
        }
        else
        {
            if (pgn_table.table[m].addr == 0x00 || pgn_table.table[m].addr == addr)
                return true;
            else
                return false;
        }
    }

    return false;
}

void j1939_pgn_req(uint8_t channel, uint32_t pgn, uint8_t dstaddr, uint8_t saaddr)
{
    J1939_TX_MESSAGE_T msg;

    msg.PGN        = REQUEST_PGN;
    msg.priority   = 6;
    msg.dest_addr  = dstaddr;
    msg.sa_addr    = saaddr;
    msg.byte_count = 8;
    msg.channel    = channel;
    msg.data[0]    = pgn;
    msg.data[1]    = pgn >> 8;
    msg.data[2]    = 0xFF;
    msg.data[3]    = 0xFF;
    msg.data[4]    = 0xFF;
    msg.data[5]    = 0xFF;
    msg.data[6]    = 0xFF;
    msg.data[7]    = 0xFF;

    j1939_tl_send(&msg);
}
