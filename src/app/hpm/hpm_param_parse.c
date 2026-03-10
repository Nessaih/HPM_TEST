#include "tbox_common.h"
#include "tbox_core.h"
#include "hpm_param_parse.h"
#include "tbox_string.h"

static INT32 hpm_fetch_param_hex_to_uint32(const char *hex, UINT32 *value)
{
    if (NULL_PTR == hex || NULL_PTR == value)
    {
        return -1;
    }

    UINT32 val = 0;
    while (*hex)
    {
        val <<= 4;
        if (*hex >= '0' && *hex <= '9')
        {
            val += *hex - '0';
        }
        else if (*hex >= 'A' && *hex <= 'F')
        {
            val += *hex - 'A' + 10;
        }
        else if (*hex >= 'a' && *hex <= 'f')
        {
            val += *hex - 'a' + 10;
        }
        else
        {
            break;
        }
        hex++;
    }
    *value = val;
    return 0;
}

static hpm_fetch_can_rate_e hpm_param_get_can_rate(UINT32 rate)
{
    if (0 == rate)
    {
        return HPM_FETCH_CAN_RATE_0;
    }
    else if (250 == rate)
    {
        return HPM_FETCH_CAN_RATE_250;
    }
    else if (500 == rate)
    {
        return HPM_FETCH_CAN_RATE_500;
    }
    else if (1000 == rate)
    {
        return HPM_FETCH_CAN_RATE_1000;
    }
    return HPM_FETCH_CAN_RATE_0;
}

UINT32 hpm_param_get_baudrate(hpm_fetch_can_rate_e rate)
{
    switch (rate)
    {
    case HPM_FETCH_CAN_RATE_250:
        return 250;
    case HPM_FETCH_CAN_RATE_500:
        return 500;
    case HPM_FETCH_CAN_RATE_1000:
        return 1000;
    default:
        break;
    }
    return 0;
}

VOID hpm_param_skip_white_space(const char **p)
{
    if (NULL_PTR == p || NULL_PTR == *p)
    {
        return;
    }
    while (**p && isspace(**p) && (**p != '\r') && (**p != '\n'))
    {
        (*p)++;
    }
}

VOID hpm_param_skip_blank_line(const char **p)
{
    if (NULL_PTR == p || NULL_PTR == *p)
    {
        return;
    }
    while (**p)
    {
        hpm_param_skip_white_space(p);
        if ((**p == '\r') || (**p == '\n'))
        {
            while ((**p == '\r' || **p == '\n'))
            {
                (*p)++;
            }
        }
        else
        {
            break;
        }
    }
}

hpm_fetch_line_type_e hpm_param_get_line_type(const char *line)
{
    if (NULL_PTR == line)
    {
        return HPM_FETCH_LINE_INVALID;
    }
    const char *p = line;
    if (*p++ != '[')
    {
        return HPM_FETCH_LINE_INVALID;
    }
    hpm_param_skip_white_space(&p);
    char segname[16] = {0};
    UINT8 len = 0;
    while (*p && (*p != ']') && (len < sizeof(segname) - 1))
    {
        segname[len++] = toupper(*p);
        p++;
    }

    if (*p != ']')
    {
        return HPM_FETCH_LINE_INVALID;
    }

    if ('C' == segname[0] && 'O' == segname[1])
    {
        return HPM_FETCH_LINE_CO;
    }
    else if ('E' == segname[0] && 'S' == segname[1])
    {
        return HPM_FETCH_LINE_ES;
    }
    else if ('N' == segname[0] && 'S' == segname[1] && '4' == segname[2])
    {
        return HPM_FETCH_LINE_NS4;
    }
    else if ('N' == segname[0] && 'S' == segname[1] && '6' == segname[2])
    {
        return HPM_FETCH_LINE_NS6;
    }
    return HPM_FETCH_LINE_INVALID;
}

UINT16 hpm_param_get_line(const char **p, char *line, UINT16 len)
{
    UINT16 pos = 0;
    if (NULL_PTR == p || NULL_PTR == *p || NULL_PTR == line || len == 0)
    {
        return 0;
    }
    while (**p && (**p != '\r') && (**p != '\n') && (pos < len - 1))
    {
        line[pos++] = **p;
        (*p)++;
    }
    line[pos] = '\0';
    while ((**p == '\r' || **p == '\n'))
    {
        (*p)++;
    }
    return pos;
}

INT32 hpm_param_split_str(const char **src, char *dst, UINT16 len, char delim)
{
    if (NULL_PTR == src || NULL_PTR == *src || NULL_PTR == dst || len == 0)
    {
        return -1;
    }

    UINT16 pos = 0;
    hpm_param_skip_white_space(src);
    while (**src && (**src != delim) && (pos < len - 1))
    {
        dst[pos++] = **src;
        (*src)++;
    }
    dst[pos] = '\0';
    if (**src == delim)
    {
        (*src)++;
    }
    if (0 == pos)
    {
        return -1;
    }
    return 0;
}

static INT32 hpm_param_parse_co(hpm_fetch_config_t *config, const char *line)
{
    if (NULL_PTR == config || NULL_PTR == line)
    {
        return -1;
    }

    const char *p = line;
    char buf[32] = {0};
    UINT32 val = 0;

    /* sequence */
    if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        MODULE_LOG_E(HPM, "parse sequence error.");
        return -1;
    }
    tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
    config->sequence = val;

    /* interval */
    if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        MODULE_LOG_E(HPM, "parse interval error.");
        return -1;
    }
    tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
    config->interval = val;

    /* baud list (enclosed in square brackets) */
    if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        MODULE_LOG_E(HPM, "parse baud error.");
        return -1;
    }

    /* remove surrounding brackets if they exist */
    char *q = buf;
    if ('[' == *q)
    {
        q++;
    }
    char *end = q + strlen(q);
    if (end > q && ']' == *(end - 1))
    {
        *(--end) = '\0';
    }

    /* three possible rates separated by ':' */
    {
        char sub[32];
        const char *rate_ptr = q;
        if (0 == hpm_param_split_str(&rate_ptr, sub, sizeof(sub), ':'))
        {
            tbox_string_get_num_bylen((UINT8 *)sub, sizeof(sub), &val);
            config->can_rate1 = hpm_param_get_can_rate(val);
        }
        if (0 == hpm_param_split_str(&rate_ptr, sub, sizeof(sub), ':'))
        {
            tbox_string_get_num_bylen((UINT8 *)sub, sizeof(sub), &val);
            config->can_rate2 = hpm_param_get_can_rate(val);
        }
        if (0 == hpm_param_split_str(&rate_ptr, sub, sizeof(sub), ':'))
        {
            tbox_string_get_num_bylen((UINT8 *)sub, sizeof(sub), &val);
            config->can_rate3 = hpm_param_get_can_rate(val);
        }
    }

    if (0 == hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
        config->protocol = val;
    }

    return 0;
}

INT32 hpm_param_parse_node(hpm_fetch_node_t *node, const char *line)
{
    if (NULL_PTR == node || NULL_PTR == line)
    {
        return -1;
    }

    const char *p = line;
    char buf[32] = {0};
    UINT32 val = 0;

    if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        MODULE_LOG_E(HPM, "parse es error.");
        return -1;
    }
    tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
    node->type = val;

    if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
    {
        return -1;
    }
    tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
    if (val > 0)
    {
        node->channel = (val - 1) & DRV_CAN_INS_COUNT;
    }
    else
    {
        node->channel = 0;
    }

    switch (node->type)
    {
    case HPM_FETCH_NODE_SINGLE:
        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->canid = val;
        break;
    case HPM_FETCH_NODE_CONSECTIVE:
        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->canid = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        tbox_string_get_num_bylen((UINT8 *)buf, sizeof(buf), &val);
        node->frame = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->offset = val;
        break;

    case HPM_FETCH_NODE_BDPGN:
        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->sa = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->pgn = val;
        break;

    case HPM_FETCH_NODE_PGN:
        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->sa = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->ta = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->pgn = val;
        break;

    case HPM_FETCH_NODE_UDS:
        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->canid = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->respid = val;

        if (0 != hpm_param_split_str(&p, buf, sizeof(buf), ','))
        {
            MODULE_LOG_E(HPM, "parse es error.");
            return -1;
        }
        hpm_fetch_param_hex_to_uint32(buf, &val);
        node->did = val;
        break;
    default:
        break;
    }
    return 0;
}

static INT32 hpm_param_parse_es(hpm_fetch_config_t *config, const char *line)
{
    if (NULL_PTR == config || NULL_PTR == line)
    {
        return -1;
    }

    if (config->count >= HPM_FETCH_NODE_COUNT)
    {
        MODULE_LOG_E(HPM, "node count overflow");
        return -1;
    }

    hpm_fetch_node_t *node = &config->node[config->count];

    if (0 != hpm_param_parse_node(node, line))
    {
        return -1;
    }

    config->count++;
    return 0;
}

static INT32 hpm_param_parse_ns4(hpm_fetch_config_t *config, const char *line)
{
    UNUSED(config);
    UNUSED(line);
    return 0;
}

static INT32 hpm_param_parse_ns6(hpm_fetch_config_t *config, const char *line)
{
    UNUSED(config);
    UNUSED(line);
    return 0;
}

INT32 hpm_param_parse_config(hpm_fetch_config_t *config, const char *line, hpm_fetch_line_type_e type)
{
    INT32 ret = 0;
    switch (type)
    {
    case HPM_FETCH_LINE_CO:
        ret = hpm_param_parse_co(config, line);
        break;
    case HPM_FETCH_LINE_ES:
        ret = hpm_param_parse_es(config, line);
        break;
    case HPM_FETCH_LINE_NS4:
        ret = hpm_param_parse_ns4(config, line);
        break;
    case HPM_FETCH_LINE_NS6:
        ret = hpm_param_parse_ns6(config, line);
        break;
    default:
        break;
    }
    return ret;
}