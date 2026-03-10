#include "tbox_common.h"
#include "delay.h"
#include "drv_pin.h"
#include "drv_spi_nor_flash.h"
#include "macros.h"
#include "drv_log.h"

#define NAND_GET_CA(x)    (0x07FFU & ((x) >> 0))
#define NAND_GET_PA(x)    (0xFFFFU & ((x) >> 11))

#define NAND_BYTE_0(x)    (0xFFU & ((x) >> 0))
#define NAND_BYTE_1(x)    (0xFFU & ((x) >> 8))
#define NAND_BYTE_2(x)    (0xFFU & ((x) >> 16))
#define NAND_BYTE_3(x)    (0xFFU & ((x) >> 24))

#define NAND_CHIP_SIZE    0x8000000U
#define NAND_BLOCK_SIZE   0x20000U
#define NAND_PAGE_SIZE    0x800U

#define SR1_ADDRESS       0xA0U
#define SR2_ADDRESS       0xB0U
#define SR3_ADDRESS       0xC0U

#define CMD_RESET_ENABLE  0x66
#define CMD_RESET_DEVICE  0x99
#define CMD_READ_ID       0x9F
#define CMD_READ_STATUS   0x0F
#define CMD_READ_PAGE     0x13
#define CMD_READ_DATA     0x03
#define CMD_ERASE_BLOCK   0xD8
#define CMD_WRITE_ENABLE  0x06
#define CMD_WRITE_DISABLE 0x04
#define CMD_WRITE_STATUS  0x1F
#define CMD_PROGRAM_LOAD  0x02
#define CMD_PROGRAM_RAND  0x84
#define CMD_PROGRAM_EXEC  0x10

static uint8_t tx_buffer[2148];
static uint8_t rx_buffer[2148];

static int32_t nand_write_enable(void)
{
    tx_buffer[0] = CMD_WRITE_ENABLE;
    if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 1, SPI_PCS_2))
    {
        DRV_LOG_E(DRVFLASH, "nand write enable error");
        return -1;
    }
    return 0;
}

static int32_t nand_write_disable(void)
{
    tx_buffer[0] = CMD_WRITE_DISABLE;
    if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 1, SPI_PCS_2))
    {
        DRV_LOG_E(DRVFLASH, "nand write disable error");
        return -1;
    }
    return 0;
}

static int32_t nand_read_status(uint8_t sr, uint8_t *status)
{
    tx_buffer[0] = CMD_READ_STATUS;
    tx_buffer[1] = sr;
    tx_buffer[2] = 0xFFU;
    memset(rx_buffer, 0xFF, 3);
    if (0 == drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 3, SPI_PCS_2))
    {
        *status = rx_buffer[2];
        return 0;
    }
    return -1;
}

static int32_t nand_write_status(uint8_t sr, uint8_t status)
{
    tx_buffer[0] = CMD_WRITE_STATUS;
    tx_buffer[1] = sr;
    tx_buffer[2] = status;
    memset(rx_buffer, 0xFF, 3);
    if (0 == drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 3, SPI_PCS_2))
    {
        return 0;
    }
    return -1;
}

static int32_t nand_wait_ready(void)
{
    int32_t timeout = 500;
    uint8_t status;

    do
    {
        status = 0;
        if (0 == nand_read_status(SR3_ADDRESS, &status))
        {
            if (0U == (status & 1U))
                return 0;
        }
        delay_us(100);
        --timeout;

    } while (timeout > 0);

    return -1;
}

int32_t drv_flash_nand_get_id(uint32_t *id)
{
    int32_t status;

    tx_buffer[0] = CMD_READ_ID;
    tx_buffer[1] = 0xFF;
    status       = drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 5, SPI_PCS_2);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "read id error");
        return -1;
    }
    *id = 0;
    memcpy(&id, &rx_buffer[2], 3);

    return 0;
}

int32_t drv_flash_nand_init(void)
{
    int32_t  status = 0;
    uint8_t  reg    = 0;
    uint32_t id     = 0;

    tx_buffer[0] = CMD_RESET_ENABLE;
    status       = drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 1, SPI_PCS_2);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "reset error");
        return -1;
    }

    tx_buffer[0] = CMD_RESET_DEVICE;
    status       = drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 1, SPI_PCS_2);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "reset error (0x66)");
        return -1;
    }

    tx_buffer[0] = CMD_READ_ID;
    tx_buffer[1] = 0xFF;
    status       = drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 5, SPI_PCS_2);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "read id error");
        return -1;
    }
    memcpy(&id, &rx_buffer[2], 3);

    status = nand_write_status(SR1_ADDRESS, 0x00);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "read sr2 error");
        return -1;
    }

    status = nand_write_status(SR2_ADDRESS, 0x19);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "read sr2 error");
        return -1;
    }

    status = nand_read_status(SR2_ADDRESS, &reg);
    if (status)
    {
        DRV_LOG_E(DRVFLASH, "read sr2 error");
        return -1;
    }

    DRV_LOG_I(DRVFLASH, "nand id: 0x%06X, sr2: 0x%02X\r\n", id, reg);
    return status;
}

int32_t drv_flash_nand_sleep(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 82;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 82;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_HIGH;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
    
    return 0;
}

int32_t drv_flash_nand_wake(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 82;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION3;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 82;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
    
    return 0;
}

int32_t drv_flash_nand_read(uint32_t addr, uint8_t *data, uint32_t data_len)
{

    uint32_t end_addr = addr + data_len;
    uint32_t page_addr, col_addr, read_len;

    if (addr >= NAND_CHIP_SIZE || end_addr > NAND_CHIP_SIZE)
    {
        return -1;
    }

    while (addr < end_addr)
    {
        page_addr = NAND_GET_PA(addr);
        col_addr  = NAND_GET_CA(addr);
        read_len  = MIN_VALUE(NAND_PAGE_SIZE - col_addr, end_addr - addr);

        tx_buffer[0] = CMD_READ_PAGE;
        tx_buffer[1] = 0xFFU;
        tx_buffer[2] = NAND_BYTE_1(page_addr);
        tx_buffer[3] = NAND_BYTE_0(page_addr);

        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 4, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand read command error");
            return -1;
        }

        if (nand_wait_ready())
        {
            DRV_LOG_E(DRVFLASH, "nand read wait error");
            return -1;
        }

        tx_buffer[0] = CMD_READ_DATA;
        tx_buffer[1] = NAND_BYTE_1(col_addr);
        tx_buffer[2] = NAND_BYTE_0(col_addr);
        tx_buffer[3] = 0xFFU;

        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 4 + read_len, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand read data error");
            return -1;
        }
        memcpy(data, &rx_buffer[4], read_len);
        addr += read_len;
        data += read_len;
    }

    return 0;
}

int32_t drv_flash_nand_write(uint32_t addr, uint8_t *data, uint32_t data_len)
{
    uint32_t end_addr = addr + data_len;
    uint32_t page_addr, col_addr, write_len;
    uint8_t  status;

    if (addr >= NAND_CHIP_SIZE || end_addr > NAND_CHIP_SIZE)
    {
        return -1;
    }

    while (addr < end_addr)
    {
        if (nand_write_enable())
        {
            return -1;
        }

        page_addr = NAND_GET_PA(addr);
        col_addr  = NAND_GET_CA(addr);
        write_len = MIN_VALUE(NAND_PAGE_SIZE - col_addr, end_addr - addr);

        tx_buffer[0] = CMD_PROGRAM_RAND;
        tx_buffer[1] = NAND_BYTE_1(col_addr);
        tx_buffer[2] = NAND_BYTE_0(col_addr);

        memcpy(&tx_buffer[3], data, write_len);
        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 3 + write_len, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand write load error");
            return -1;
        }

        tx_buffer[0] = CMD_PROGRAM_EXEC;
        tx_buffer[1] = NAND_BYTE_2(page_addr);
        tx_buffer[2] = NAND_BYTE_1(page_addr);
        tx_buffer[3] = NAND_BYTE_0(page_addr);
        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 4, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand write exec error");
            return -1;
        }

        if (nand_wait_ready())
        {
            DRV_LOG_E(DRVFLASH, "nand write wait error");
            return -1;
        }

        tx_buffer[0] = CMD_READ_STATUS;
        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 2, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand read status error");
            return -1;
        }
        status = rx_buffer[1];
        if (status & 0x0E)
        {
            DRV_LOG_E(DRVFLASH, "nand write verify error, status=0x%02X", status);
            return -1;
        }

        if (nand_write_disable())
        {
            return -1;
        }

        addr += write_len;
        data += write_len;
    }

    return 0;
}

int32_t drv_flash_nand_erase(uint32_t addr, uint16_t n_128KB)
{

    uint32_t page_addr;

    for (uint16_t i = 0; i < n_128KB; i++)
    {
        if (addr >= NAND_CHIP_SIZE)
        {
            return -1;
        }

        if (nand_write_enable())
        {
            return -2;
        }

        page_addr    = NAND_GET_PA(addr);
        tx_buffer[0] = CMD_ERASE_BLOCK;
        tx_buffer[1] = NAND_BYTE_2(page_addr);
        tx_buffer[2] = NAND_BYTE_1(page_addr);
        tx_buffer[3] = NAND_BYTE_0(page_addr);
        if (drv_spi_nor_flash_transfer(tx_buffer, rx_buffer, 4, SPI_PCS_2))
        {
            DRV_LOG_E(DRVFLASH, "nand erase command error");
            return -3;
        }

        if (nand_wait_ready())
        {
            DRV_LOG_E(DRVFLASH, "nand erase wait error");
            return -4;
        }

        if (nand_write_disable())
        {
            return -5;
        }

        addr += NAND_BLOCK_SIZE;
    }

    return 0;
}