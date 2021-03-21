#include "osapi.h"
#include "user_interface.h"
#include "pwm.h"

static os_timer_t ptimer;

uint32 io_info[][3] = {{PERIPHS_IO_MUX_GPIO2_U,FUNC_GPIO2,2},};
uint32 pwm_duty[1] = {0};

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        case FLASH_SIZE_64M_MAP_1024_1024:
            rf_cal_sec = 2048 - 5;
            break;
        case FLASH_SIZE_128M_MAP_1024_1024:
            rf_cal_sec = 4096 - 5;
            break;
        default:
            rf_cal_sec = 0;
            break;
    }
    return rf_cal_sec;
}

//Blinky callback function
void blinky(void *arg)
{
	static uint8_t state = 0;

	if (state) {
		GPIO_OUTPUT_SET(2, 1);
        GPIO_OUTPUT_SET(5,1);
        os_printf("LED OFF. \n");
	} else {
		GPIO_OUTPUT_SET(2, 0);
        GPIO_OUTPUT_SET(5,0);
        os_printf("Led ON. \n");

	}
	state ^= 1;
}

void os_timer_cb(void)//Software timer callback function
{
    bool ascending = true;
	uint32 t,k;
	pwm_set_duty(pwm_duty[0],0);
	pwm_start();
    pwm_duty[0]+=500;
    if(pwm_duty[0]>=10000){
        pwm_duty[0]=0;
    }   
    t=pwm_get_duty(0);
	k=pwm_get_period();
	os_printf("Duty Cycle: %d\r\n",t);
	os_printf("Period: %d\r\n",k);
}

//Enter main() function
void ICACHE_FLASH_ATTR user_init(void)
{
    gpio_init();
    uart_init(115200, 115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());

    // Disable WiFi
    wifi_set_opmode(NULL_MODE);
    
    //GPIO Configuration
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U,FUNC_GPIO5); //setting GPIO 5 as output

    pwm_init(1000,pwm_duty,1,io_info);
    

    //timer creation
    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)os_timer_cb, NULL);
    os_timer_arm(&ptimer, 1000, 1); //Tiempo en ms de encendido y apagado
}
