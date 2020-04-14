/*****************************************************************************
​ ​* ​ ​ @file​ ​  		letimer.h
​ * ​ ​ @brief​ ​ 		header file for myGecko.c
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		October 4th, 2019
​ * ​ ​ @version​ ​ 		2.0
*****************************************************************************/

#ifndef SRC_MYGECKO_H_
#define SRC_MYGECKO_H_

#define CONNECTION_INTERVAL_MIN 60
#define CONNECTION_INTERVAL_MAX 60
#define SLAVE_LATENCY 			3
#define TIMEOUT					600
#define ADVERTISING_MIN			400
#define ADVERTISING_MAX			400
#define ALLOW_BONDING			0x01
#define SECURITY_CONFIGURE_FLAGS 0x0F // test
#define ACCEPT_BONDING_REQUEST  0x01
#define ACCEPT_PASSKEY			1

#define TIMER_ID_FACTORY_RESET    77

enum global_GATT_states {
	SERVICE_DISCOVERY,
	CHARACTERISTIC_DISCOVERY,
	NOTIFICATION,
	READ_TEMP,
	PUSH_BUTTON_SERVICE_DISCOVERY,
	PUSH_BUTTON_CHARACTERISTIC_DISCOVERY,
}current_gatt_state;

/// Button state
static PACKSTRUCT(struct button_state{
	// On/Off Server state
	  uint8_t onoff_current;          /**< Current generic on/off value */
	  uint8_t onoff_target;           /**< Target generic on/off value */
});

struct button_state button_state;

uint8_t connection_handle;
uint8_t server_connection_handle;
uint8_t sm_connection_handle;
uint32_t service_handle;
uint32_t push_button_service_handle;
uint16_t characteristic_handle;
uint16_t push_button_characteristic_handle;
uint8_t health_thermometer_uuid[2];
uint8_t push_button_uuid[16];
uint8_t temperature_measurement_char[2];
uint16_t elem_index;
/// Flag for indicating that lpn feature is active
static uint8 lpn_active = 0;
/// number of active Bluetooth connections
static uint8 num_connections = 0;

void gecko_ecen5823_update(uint32_t evt_id, struct gecko_cmd_packet *evt);
void initiate_factory_reset(void);
void init_mesh(void);

#endif /* SRC_MYGECKO_H_ */
