#include "osapi.h"
#include "user_interface.h"

static os_timer_t ptimer, timer;

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

//Callback Event Handler
void ICACHE_FLASH_ATTR eventHandler(System_Event_t *event)
{ 
    switch(event->event){
    case EVENT_STAMODE_CONNECTED:
        os_printf("Event: EVENT_STAMODE_CONNECTED\n");
    break;
    case EVENT_STAMODE_DISCONNECTED:
        os_printf("Event: EVENT_STAMODE_DISCONNECTED\n");
        os_printf("Disconnect from ssid %s, reason %d\n", event->event_info.disconnected.ssid, event->event_info.disconnected.reason);
    break;
    case EVENT_STAMODE_AUTHMODE_CHANGE:
        os_printf("Event: EVENT_STAMODE_AUTHMODE_CHANGE\n");
    break;
    case EVENT_STAMODE_GOT_IP:
        os_printf("Event: EVENT_STAMODE_GOT_IP\n");
        os_printf("Connected to Wifi! \n");
    break;
    case EVENT_SOFTAPMODE_STACONNECTED:
        os_printf("Event: EVENT_SOFTAPMODE_STACONNECTED\n");
    break;
    case EVENT_SOFTAPMODE_STADISCONNECTED:
        os_printf("Event: EVENT_SOFTAPMODE_STADISCONNECTED\n");
    break;
    default:
        os_printf("Unexpected event: %d\n", event->event);
    break;
    }
}

//Callback Function for the wifi_station_scan Function BUSCAR
/*void ICACHE_FLASH_ATTR scan_cb(void *arg, STATUS status){
        uint8 ssid[32];
        if (status == OK){
            struct bss_info *bss_link = (struct bss_info *) arg;
            while (bss_link != NULL){
                printf("(\"%s\",%d,\""MACSTR"\",%d)\r\n", ssid, bss_link->rssi,
                        MAC2STR(bss_link->bssid), bss_link->channel);
                bss_link = bss_link->next.stqe_next;
            }
        }
        else{
            os_printf("Scan fail!");
        }
}*/

//Wifi Status Check
void ICACHE_FLASH_ATTR status_check(void *arg){

    uint8 status;

    status = wifi_station_get_connect_status();
    os_timer_disarm(&timer); //Disarming the timer so it doesnt repeat if it enters again

    if(status == STATION_GOT_IP)
    {
        os_printf("An IP address has been asigned successfully. Connected to WiFi! \n");
        GPIO_OUTPUT_SET(2, 0);
        os_printf("RED LED OFF. \n");
        GPIO_OUTPUT_SET(5, 0);
        os_printf("Blue Led ON. \n");

    }
    else
    {
        os_printf("Starting again... \n");
        os_timer_setfn(&timer, (os_timer_func_t *)status_check, NULL); //Start again if IP not connected
        os_timer_arm(&timer, 1000, 0); //Wait 500 mseconds
    }
}

//WiFi Test Function
void ICACHE_FLASH_ATTR wifi_test(void){

    //Start of the Wifi configuration flag
    GPIO_OUTPUT_SET(5, 1);
    os_printf("RED LED ON. \n");

    //Cheking WiFis current configuration
    os_printf("WiFi's current configuration is: ");
    switch (wifi_get_opmode())
    {
    case NULL_MODE:
        os_printf("Disabled \n");
        break;
    case STATION_MODE:
        os_printf("Enabled as Station Mode \n");
        break;
    case SOFTAP_MODE:
        os_printf("Enabled as Soft AP Mode \n");
        break;
    case STATIONAP_MODE:
        os_printf("Enabled as Station+SoftAP Mode \n");
        break;
    }

    //Setting Current Wifi OpMode and Configuration
    wifi_set_opmode_current(STATION_MODE);
    os_printf("ESP8266 is now a WiFi Station! \n");
    char ssid[32] = "JET Home";
    char password[64] = "chemundo300665";
    os_printf("Connecting to Wifi... \n");

    //Looking for Nearby APs
    //os_printf("The nearby APs are: %u \n", wifi_station_scan(NULL,scan_cb)); 

    struct station_config stationConf; //Initialization of stationConf as struct
    stationConf.bssid_set = 0; //need not check MAC address of AP

    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 64);
    wifi_station_disconnect();
    wifi_station_set_config_current(&stationConf);//Setting WiFi in current run

    wifi_set_event_handler_cb(eventHandler);
    wifi_station_connect();
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

    //Wifi Test
    system_init_done_cb(wifi_test);

    // Software Timer
    //1. Se desactiva el timer para luego poder activarlo
    os_timer_disarm(&timer);
    //2. Se define la funciÃ³n "sw_timer_cb" para ser llamada cuando el timer se desborde
    os_timer_setfn(&timer, status_check, NULL);
    //3. Se activa el timer de forma repetitiva con un tiempo de 2 segundos
    os_timer_arm(&timer, 5000, 1);

    //blinky timer creation
    /*os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)blinky, NULL);
    os_timer_arm(&ptimer, 2000, 1); //Tiempo en ms de encendido y apagado
    */
}
