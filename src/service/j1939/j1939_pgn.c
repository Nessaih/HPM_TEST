

#include "j1939_includes.h"
#include "tbox_log.h"

#define PGN_TABLE_CNT 64

struct PGN_TABLE
{
    uint32_t count;
    uint32_t pgn[PGN_TABLE_CNT];
};

// clang-format off
struct PGN_TABLE pgn_table =
    {
        .count = 18,
        .pgn = {
            J1939_PGN_ACK_1,
            J1939_PGN_ACK_2,
            J1939_PGN_ACK_3,
            J1939_PGN_REQ_1,
            J1939_PGN_REQ_2,
            J1939_PGN_REQ_3,
            J1939_PGN_CM_1,
            J1939_PGN_CM_2,
            J1939_PGN_CM_3,
            J1939_PGN_DT_1,
            J1939_PGN_DT_2,
            J1939_PGN_DT_3,
            J1939_PGN_ENGINE_CONSTRUCTION,
            J1939_PGN_ENGINE_RUN_TIME,
            J1939_PGN_FUEL_CONSUMPTION,
            J1939_PGN_INGREDIENT_IDENTIFICATION,
            J1939_PGN_VIN,
            J1939_PGN_DM1}
    };
// clang-format on

static void PGN_sort(uint32_t array[], uint32_t count)
{
    uint32_t i, j, m, t;

    for (i = 0; i < count - 1; i++)
    {
        m = i;
        for (j = i + 1; j < count; j++)
        {
            if (array[j] < array[m])
                m = j;
        }

        if (m != i)
        {
            t        = array[i];
            array[i] = array[m];
            array[m] = t;
        }
    }
}

void j1939_pgn_init(void)
{
    PGN_sort(pgn_table.pgn, pgn_table.count);
}

void j1939_pgn_show(void)
{
    uint32_t i;

    for (i = 0; i < pgn_table.count; i++)
    {
        LOG_PRINT("%2u  PGN = %u\n", (unsigned int)i, (unsigned int)pgn_table.pgn[i]);
    }
}

bool j1939_pgn_add(uint32_t pgn)
{
    uint32_t i, j;

    if (pgn > 0x7FFFF)
    {
        return false;
    }

    if (pgn_table.count >= PGN_TABLE_CNT)
    {
        return false;
    }

    for (i = 0; i < pgn_table.count; i++)
    {
        if (pgn < pgn_table.pgn[i])
            break;
    }

    for (j = pgn_table.count; j > i; j--)
    {
        pgn_table.pgn[j] = pgn_table.pgn[j - 1];
    }

    pgn_table.pgn[i] = pgn;
    pgn_table.count++;

    return true;
}

bool j1939_pgn_filter(uint32_t pgn)
{
    int32_t l = 0;
    int32_t m = 0;
    int32_t r = pgn_table.count - 1;

    while (l <= r)
    {
        m = (l + r) >> 1;
        if (pgn > pgn_table.pgn[m])
            l = m + 1;
        else if (pgn < pgn_table.pgn[m])
            r = m - 1;
        else
            return true;
    }
    return false;
}

void j1939_pgn_req(uint32_t channel, uint32_t pgn, uint8_t dstaddr,uint8_t saaddr)
{
    J1939_TX_MESSAGE_T msg;

    msg.PGN        = REQUEST_PGN;
    msg.priority   = 6;
    msg.dest_addr  = dstaddr;
	msg.sa_addr	   = saaddr;
    msg.byte_count = 8;
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
