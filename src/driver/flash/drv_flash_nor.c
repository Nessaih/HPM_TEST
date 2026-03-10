#include "tbox_common.h"
#include "api_rtos.h"
#include "drv_log.h"
#include "drv_pin.h"
#include "drv_spi.h"

#define EFS_CMD_JEDEC_ID               (0X9F)
#define EFS_CMD_WRITE_ENABLE           (0X06)
#define EFS_CMD_WRITE_DISABLE          (0X04)
#define EFS_CMD_READ_STATUS_REG1       (0X05)
#define EFS_CMD_READ_STATUS_REG2       (0X35)
#define EFS_CMD_READ_STATUS_REG3       (0X15)
#define EFS_CMD_SECTOR_ERASE           (0X21)
#define EFS_CMD_CHIP_ERASE             (0XC7)
#define EFS_CMD_WRITE_DATA             (0X12)
#define EFS_CMD_READ_DATA              (0X13)
#define EFS_CMD_4BYTES_ADDR            (0XB7)

#define EFS_SECTOR_SIZE                (4096)
#define EFS_PAGE_SIZE                  (256)
#define EFS_PAGE_COUNT                 (EFS_SECTOR_SIZE / EFS_PAGE_SIZE)
#define EFS_BACKUP_SECTOR              (EXFLASH_ADDR_BLOCK6)
#define EFS_TRANSFER_HEAD_SIZE         (5)
#define EFS_TRANSFER_DATA_SIZE         (256)
#define EFS_TRANSFER_BUFF_SIZE         (EFS_TRANSFER_HEAD_SIZE + EFS_TRANSFER_DATA_SIZE)

#define EFS_CAL_PAGE_HEAD_ADDR(addr)   ((addr) & ~0XFFUL)
#define EFS_CAL_SECTOR_HEAD_ADDR(addr) ((addr) & ~0XFFFUL)
#define EFS_CAL_PAGE_SPACE(addr)       (EFS_PAGE_SIZE - ((addr) & 0XFFUL))
#define EFS_CAL_SECTOR_SPACE(addr)     (EFS_SECTOR_SIZE - ((addr) & 0XFFFUL))

#define EFS_STATUS_READY               (0)
#define EFS_STATUS_BUZY                (1)

#define EFS_INVERT32(x)                ((((uint32_t)x) >> 24) | ((((uint32_t)x) & 0x00FF0000) >> 8) | ((((uint32_t)x) & 0x0000FF00) << 8) | (((uint32_t)x) << 24))

#pragma pack(1)
typedef union
{
    uint8_t buffer[EFS_TRANSFER_BUFF_SIZE];
    struct
    {
        uint8_t  cmd;
        uint32_t addr;
        uint8_t  data[EFS_TRANSFER_DATA_SIZE];
    };
} efs_data_t;
#pragma pack()

static efs_data_t efs_txbuf;
static efs_data_t efs_rxbuf;

#define VSE_DEBUG_ENABLE 1

static drv_spi_handle_t nor_spi_handle;
static drv_spi_config_t nor_spi_config = {
    .instance = 1,
    .mode     = DRV_SPI_MODE_0,
    .cs_mode  = DRV_SPI_CS_MODE_AUTO,
    .cs_index = DRV_SPI_CS_INDEX_3,
    .speed    = 2000000UL,
};

static void drv_flash_nor_write_enable(void)
{
    efs_txbuf.cmd = EFS_CMD_WRITE_ENABLE;
    if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, 1))
    {
        DRV_LOG_E(DRVFLASH, "efs write enable error");
    }
}

static void drv_flash_nor_write_disable(void)
{
#if 0
    efs_txbuf.cmd = EFS_CMD_WRITE_DISABLE;
    if (0 != drv_spi_nor_flash_transfer(efs_txbuf.buffer, efs_rxbuf.buffer, 1,SPI_PCS_3))
    {
        DRV_LOG_E(DRVFLASH, "write disable error");
    }
#endif
}

static int32_t drv_flash_nor_read_status_reg1(void)
{

    efs_txbuf.cmd = EFS_CMD_READ_STATUS_REG1;
    if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, 2))
    {
        DRV_LOG_E(DRVFLASH, "read reg1 error");
    }
    return efs_rxbuf.buffer[1];
}

static int32_t drv_flash_nor_wait_ready(uint32_t wait_10ms)
{
    uint8 reg = 0;
    do
    {
        reg = drv_flash_nor_read_status_reg1();
        vTaskDelay(pdMS_TO_TICKS(10));
        wait_10ms--;
    } while ((reg & 0X01) && wait_10ms);

    if (wait_10ms)
        return EFS_STATUS_READY;
    return EFS_STATUS_BUZY;
}

int32_t drv_flash_nor_get_id(uint32_t *id)
{
    if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(20))
        return 1;
    efs_txbuf.cmd  = EFS_CMD_JEDEC_ID;
    efs_txbuf.addr = 0;
    if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, 4))
    {
        DRV_LOG_E(DRVFLASH, "ease sector error");
    }
    *id = 0;
    memcpy(id, &efs_rxbuf.buffer[1], 3);

    return 0;
}

int32_t drv_flash_nor_init(void)
{

    drv_spi_init(&nor_spi_config, &nor_spi_handle);

    drv_flash_nor_write_enable();
    efs_txbuf.cmd = EFS_CMD_4BYTES_ADDR;
    if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, 1))
    {
        DRV_LOG_E(DRVFLASH, "enter 4bytes mode fail");
    }
    drv_flash_nor_write_disable();
    return 0;
}

int32_t drv_flash_nor_sleep(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 63;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 63;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_HIGH;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    return 0;
}

int32_t drv_flash_nor_wake(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 63;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION3;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 63;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    return 0;
}

int32_t drv_flash_nor_erase(uint32_t addr, uint16_t n_4KB)
{
    uint32_t erase_addr;

    for (size_t i = 0; i < n_4KB; i++)
    {
        if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(100))
            return 1;

        erase_addr = addr + (i * EFS_SECTOR_SIZE);

        drv_flash_nor_write_enable();
        efs_txbuf.cmd  = EFS_CMD_SECTOR_ERASE;
        efs_txbuf.addr = EFS_INVERT32(erase_addr);
        if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, 5))
        {
            DRV_LOG_E(DRVFLASH, "ease sector error");
        }
        drv_flash_nor_write_disable();
    }
    return 0;
}

int32_t drv_flash_nor_read(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t rlen;

    while (data_len)
    {
        if (data_len > EFS_PAGE_SIZE)
            rlen = EFS_PAGE_SIZE;
        else
            rlen = data_len;

        if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(100))
            return 1;

        drv_flash_nor_write_enable();
        efs_txbuf.cmd  = EFS_CMD_READ_DATA;
        efs_txbuf.addr = EFS_INVERT32(addr);
        if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, EFS_TRANSFER_HEAD_SIZE + rlen))
        {
            DRV_LOG_E(DRVFLASH, "ease sector error");
        }
        drv_flash_nor_write_disable();

        if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(100))
            return 1;

        memcpy(data, efs_rxbuf.data, rlen);

        data += rlen;
        addr += rlen;
        data_len -= rlen;
    }
    return 0;
}

int32_t drv_flash_nor_write(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t wlen;

    while (data_len)
    {
        wlen = EFS_CAL_PAGE_SPACE(addr);
        wlen = (wlen < data_len) ? wlen : data_len;
        if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(100))
            return 1;

        drv_flash_nor_write_enable();
        efs_txbuf.cmd  = EFS_CMD_WRITE_DATA;
        efs_txbuf.addr = EFS_INVERT32(addr);
        memcpy(efs_txbuf.data, data, wlen);

        if (0 != drv_spi_transfer(&nor_spi_handle, efs_txbuf.buffer, efs_rxbuf.buffer, EFS_TRANSFER_HEAD_SIZE + wlen))
        {
            DRV_LOG_E(DRVFLASH, "efs write data error");
            return 1;
        }
        drv_flash_nor_write_disable();

        if (EFS_STATUS_BUZY == drv_flash_nor_wait_ready(100))
            return 1;

        data += wlen;
        addr += wlen;
        data_len -= wlen;
    }
    return 0;
}