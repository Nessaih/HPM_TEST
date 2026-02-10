#include <stdbool.h>
#include "Eio_Uart_Hal.h"
#include "drv_pin.h"
#include "cqueue.h"

#include "drv_eio_def.h"

#define EIO_GNSS_INSTANCE       0U
#define EIO_GNSS_CHANNEL        0U
#define EIO_GNSS_IRQN           EIO_IRQn
#define EIO_GNSS_RATE           9600U

#define EIO_GNSS_TX_SIZE         128U
#define EIO_GNSS_RX_SIZE         128U
#define EIO_GNSS_TQ_SIZE         512U
#define EIO_GNSS_RQ_SIZE         2048U

#define EIO_GNSS_RX_STATE_IDLE   0
#define EIO_GNSS_RX_STATE_DATAIN 1

#define EIO_GNSS_TX_STATE_IDLE   0
#define EIO_GNSS_TX_STATE_BUZY   1


typedef struct
{
	volatile int32_t state;
	eio_cb_t		 callback;
	cqueue_t		 queue;
}eio_channel_t;

typedef struct
{
	bool 			init;
	eio_channel_t	rx;
	eio_channel_t   tx;
}eio_device_t;


static int32_t drv_eio_gnss_deinit(void);

static uint8_t      eio_tx_buf[EIO_GNSS_TX_SIZE];
static uint8_t      eio_rx_buf[EIO_GNSS_RX_SIZE];
static uint8_t      eio_tq_buf[EIO_GNSS_TQ_SIZE];
static uint8_t      eio_rq_buf[EIO_GNSS_RQ_SIZE];

static uint32 		drv_eio_gnss_rate;
static eio_device_t eio_device;

static void tx_callback(uint8 instance, Eio_UartEventType event)
{
	uint32		   l = 0;
	Hal_StatusType status;
	eio_device_t  *dev = &eio_device;
	
	NVIC_DisableIRQ(EIO_GNSS_IRQN);
	switch (event)
	{
		case EIO_UART_EVENT_TX_EMPTY:
			break;
		case EIO_UART_EVENT_END_TRANSFER:
			l = cqueue_get(&dev->tx.queue, eio_tx_buf, EIO_GNSS_TX_SIZE);
			if(l > 0U)
			{
				status = Eio_Uart_Hal_SendData(EIO_GNSS_CHANNEL, eio_tx_buf, l);
				if(STATUS_SUCCESS == status)
				{
					dev->tx.state = EIO_GNSS_TX_STATE_BUZY;
				}
				else
				{
					cqueue_put(&dev->tx.queue, eio_tx_buf, l);
				}
			}
			break;
		case EIO_UART_EVENT_ERROR:
			break;
		default:
			break;
	}
	NVIC_EnableIRQ(EIO_GNSS_IRQN);
}

static void rx_callback(uint8 instance, Eio_UartEventType event)
{
	uint32	   l = 0U;
	eio_device_t  *dev = &eio_device;

    if (event == EIO_UART_EVENT_END_TRANSFER)
    {
        Hal_StatusType status = Eio_Uart_Hal_GetReceiveStatus(0, NULL_PTR);
        if (STATUS_SUCCESS == status)
        {
        	l = EIO_GNSS_RX_SIZE;
			l = cqueue_put(&dev->rx.queue, eio_rx_buf, l);
        }

        Eio_Uart_Hal_ReceiveData(EIO_GNSS_CHANNEL, eio_rx_buf, EIO_GNSS_RX_SIZE);
		if((l > 0) && (NULL_PTR != dev->rx.callback))
		{
			dev->rx.callback();
		}
    }
}

static Eio_UartUserConfigType eio_gnss_config = {
    .EioInstance  = EIO_GNSS_INSTANCE,
    .DriverType   = EIO_DRIVER_TYPE_INTERRUPTS,
    .BaudRate     = EIO_GNSS_RATE,
    .BitCount     = 8U,
    .Direction    = EIO_UART_DIRECTION_BOTH,
    .TxDataPin    = 0U,
    .RxDataPin    = 1U,
    .TxCallback   = tx_callback,
    .RxCallback   = rx_callback,
    .TxDmaChannel = 255U,
    .RxDmaChannel = 255U,
};

static void drv_eio_gnss_pin_sleep(void)
{
    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 4;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 4;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 3;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_GPIO;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 3;
    gpio.mode  = GPIO_MODE_OUTPUT;
    gpio.level = GPIO_LEVEL_LOW;

    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);
}

static void drv_eio_gnss_pin_wake(void)
{

    pin_port_cfg_t port = {0};
    pin_gpio_cfg_t gpio = {0};

    port.pin   = 4;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 4;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

    port.pin   = 3;
    port.pull  = PIN_PULL_DISABLE;
    port.mux   = PIN_MUX_OPTION6;
    port.edge  = PIN_INT_DISABLE;
    port.lock  = PIN_LOCK_DISABLE;
    gpio.pin   = 3;
    gpio.mode  = GPIO_MODE_INPUT;
    gpio.level = GPIO_LEVEL_NONE;
    drv_pin_port_config(&port);
    drv_pin_gpio_config(&gpio);

}

int32_t drv_eio_gnss_init(void)
{
	Hal_StatusType status;
	
	if(eio_device.init)
	{
		drv_eio_gnss_deinit();
	}
	
	drv_eio_gnss_rate = EIO_GNSS_RATE;
	eio_device.init	  = false;
		
	eio_device.rx.state = EIO_GNSS_RX_STATE_IDLE;
	cqueue_init(&eio_device.rx.queue, eio_rq_buf, EIO_GNSS_RQ_SIZE);

	eio_device.tx.state = EIO_GNSS_TX_STATE_IDLE;
	cqueue_init(&eio_device.tx.queue, eio_tq_buf, EIO_GNSS_TQ_SIZE);

	drv_pin_set_level(PIN_RESET_GNSS, 1);
	drv_eio_gnss_pin_wake();

    Eio_Uart_Hal_Init(EIO_GNSS_CHANNEL, &eio_gnss_config);
    Eio_Uart_Hal_SetBaudRate(EIO_GNSS_CHANNEL, drv_eio_gnss_rate);
    status = Eio_Uart_Hal_ReceiveData(EIO_GNSS_CHANNEL, eio_rx_buf, EIO_GNSS_RX_SIZE);

    if(STATUS_SUCCESS != status)
    {
    	return -1;
    }

	eio_device.init = true;
	return 0;
}


static int32_t drv_eio_gnss_deinit(void)
{
	drv_pin_set_level(PIN_RESET_GNSS, 0);
	Eio_Uart_Hal_AbortReceivingData(EIO_GNSS_CHANNEL);
	Eio_Uart_Hal_Deinit(EIO_GNSS_CHANNEL);
	drv_eio_gnss_pin_sleep();
	eio_device.init = false;
    return 0;
}

static int32_t drv_eio_gnss_reinit(void)
{
	drv_eio_gnss_deinit();
	drv_eio_gnss_init();
    return 0;
}

int32_t drv_eio_gnss_wake(void)
{
	if(!eio_device.init)
	{
		drv_eio_gnss_init();
	}
	
	return 0;
}

int32_t drv_eio_gnss_sleep(void)
{
	if(eio_device.init)
	{
		drv_eio_gnss_deinit();
	}

	return 0;
}

void drv_eio_gnss_set_rate(uint32 rate)
{
	drv_eio_gnss_rate = rate;
	drv_eio_gnss_reinit();
}

uint32 drv_eio_gnss_get_rate(void)
{
	return drv_eio_gnss_rate;
}


//TODO: 波特率写文件


int32_t drv_eio_gnss_register(eio_cb_t cb)
{
	eio_device.rx.callback = cb;
	return 0;
}

int32_t drv_eio_gnss_tx(const uint8_t *data, uint32_t len)
{
	int32_t  ret = -1;
	uint32_t l   = 0;
	
	if((0U == eio_device.tx.queue.size) || (0U == len))
	{
		return -1;
	}

	if(cqueue_surplus(&eio_device.tx.queue) < len)
	{
		return -1;
	}

	cqueue_put(&eio_device.tx.queue, data, len);
	if(eio_device.init)
	{
		if(EIO_GNSS_TX_STATE_IDLE == eio_device.tx.state)
		{
			l = cqueue_get(&eio_device.tx.queue, eio_tx_buf, EIO_GNSS_TX_SIZE);
			
			Hal_StatusType status = Eio_Uart_Hal_SendData(EIO_GNSS_CHANNEL, eio_tx_buf, l);
			if(STATUS_SUCCESS == status)
			{
				eio_device.tx.state = EIO_GNSS_TX_STATE_BUZY;
                ret                 = (int32_t)l;
			}
			else
			{
				eio_device.tx.state = EIO_GNSS_TX_STATE_IDLE;
			}
		}
	}
	
	return ret;
}

int32_t drv_eio_gnss_rx(uint8_t *data, uint32_t len)
{
	uint32_t l = 0;

	if(0U == eio_device.rx.queue.size)
	{
		return -1;
	}

	l = cqueue_get(&eio_device.rx.queue, data, len);

	return (int32_t)l;
}



