/*****************************************************************************
​ ​* ​ ​ @file​ ​  		myGecko.c
​ * ​ ​ @brief​ ​ 		Contains BT stack event handler
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		October 4th, 2019
​ * ​ ​ @version​ ​ 		1.0
*****************************************************************************/
#include "gatt_db.h"
#include "native_gecko.h"
#include "main.h"
#include "log.h"
#include "event.h"
#include "em_letimer.h"
#include "myGecko.h"
#include "display.h"
#include "gecko_ble_errors.h"
#include "ble_device_type.h"
#include <math.h>
#include "gpio.h"

/*
 * @func - gattUint32ToFloat
 * @brief - Convert Temp from uint32 to float
 * @parameters - value_start_little_endian - raw temperature value received from server
 * @return - float - Actual temperature
 */
float gattUint32ToFloat(const uint8_t *value_start_little_endian)
{
	int8_t exponent = (int8_t)value_start_little_endian[4];
	uint32_t mantissa = value_start_little_endian[1]+(((uint32_t)value_start_little_endian[2])<<8)+
						(((uint32_t)value_start_little_endian[3])<<16);

	return (float)mantissa*pow(10,exponent);
}

/*
 * @func - gecko_ecen5823_update
 * @brief - Handle BT events
 * @parameters - evt - event to be handled
 * @return - none
 */
void gecko_ecen5823_update(struct gecko_cmd_packet* evt)
{
	gecko_update(evt);

	switch (BGLIB_MSG_ID(evt->header)) {
	 case gecko_evt_system_boot_id: ;

	 	displayPrintf(DISPLAY_ROW_NAME, "Server");

	 	struct gecko_msg_system_get_bt_address_rsp_t *bt_public_addr = gecko_cmd_system_get_bt_address();

	 	displayPrintf(DISPLAY_ROW_BTADDR, "%x.%x.%x.%x.%x.%x", bt_public_addr->address.addr[5],bt_public_addr->address.addr[4],bt_public_addr->address.addr[3],bt_public_addr->address.addr[2],bt_public_addr->address.addr[1],bt_public_addr->address.addr[0]);

	 	bond_status = 0;
	 	BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_delete_bondings());

	 	/* Set tx power to 0 on reset */
	 	gecko_cmd_system_set_tx_power(0);

		/* Set advertising parameters. 100ms advertisement interval.
		 * The first parameter is advertising set handle
		 * The next two parameters are minimum and maximum advertising interval, both in
		 * units of (milliseconds * 1.6).
		 * The last two parameters are duration and maxevents left as default. */
	 	BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_set_advertise_timing(0, ADVERTISING_MIN, ADVERTISING_MAX, 0, 0));

	 	/* Configure security capabilities of device */
		BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_configure(SECURITY_CONFIGURE_FLAGS, sm_io_capability_displayyesno));

		BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_set_bondable_mode(ALLOW_BONDING));

		/* Start general advertising and enable connections. */
		BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable));

		displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
		displayUpdate();
		break;

	 case gecko_evt_le_connection_opened_id: ;

		 connection_handle = evt->data.evt_le_connection_opened.connection;

		 displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
		 displayUpdate();

		 /* Set connection timing parameters */
		 BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_timing_parameters(connection_handle, CONNECTION_INTERVAL_MIN, CONNECTION_INTERVAL_MAX, SLAVE_LATENCY, TIMEOUT, 0x0, 0x0));

		 /* Enable interrupts only when connection open */
		 LETIMER_IntEnable(LETIMER0, (LETIMER_IEN_UF));
		 break;

	 case gecko_evt_sm_confirm_bonding_id:

		 if(evt->data.evt_sm_confirm_bonding.bonding_handle != -1)
			 LOG_INFO("Bonding exists for this connection!\n");
		 else
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_bonding_confirm(evt->data.evt_sm_confirm_bonding.connection, ACCEPT_BONDING_REQUEST));
		 break;

	 case gecko_evt_sm_confirm_passkey_id:

		 sm_connection_handle = evt->data.evt_sm_confirm_passkey.connection;
		 displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey %d", evt->data.evt_sm_confirm_passkey.passkey);
		 displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
		 displayUpdate();
		 break;

	 case gecko_evt_system_external_signal_id:

		 /* Handle button press interrupt */
		 if(((evt->data.evt_system_external_signal.extsignals & BUTTON_PRESSED) == BUTTON_PRESSED))
		 {
			 static uint8_t button_state[1] = {0};

			 /* Condition when not bonded */
			 if(!bond_status)
			 {
				 displayPrintf(DISPLAY_ROW_PASSKEY, "");
				 displayPrintf(DISPLAY_ROW_ACTION, "");
				 displayUpdate();

				 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_passkey_confirm(sm_connection_handle, ACCEPT_PASSKEY));
			 }
			 /* Condition when bonded */
			 else
			 {
				 button_state[0] = !button_state[0];
				 LOG_INFO("Button State: %d\n", button_state[0]);
				 displayPrintf(DISPLAY_ROW_CLIENTADDR, "Button State: %d", button_state[0]);
				 displayUpdate();
				 BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_send_characteristic_notification(
				 				0xFF, gattdb_button_state, 1, button_state));
			 }
		 }
		 /* Handle temp measurement events */
		 else
		 {
			 handle_event_BT(evt->data.evt_system_external_signal.extsignals);

			 /* cmd to get rssi */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_get_rssi(connection_handle));
		 }
		 break;

	 case gecko_evt_sm_bonded_id:

		 if(evt->data.evt_sm_bonded.bonding == 0xFF)
		 {
			 LOG_INFO("Pairing done but w/o bonding!\n");
		 }
		 else
		 {
			 bond_status = 1; //Global var indicates that notification can be sent now to client
			 displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
			 displayUpdate();
			 LOG_INFO("Bonding Successful!\n");
		 }
		 break;

	 case gecko_evt_sm_bonding_failed_id:

		 LOG_INFO("Pairing & Bonding failed due to 0x%x\n",evt->data.evt_sm_bonding_failed.reason);
		 break;

	 case gecko_evt_le_connection_rssi_id: ;
	 	int8_t rssi;
	 	int16_t tx_power;
	 	struct gecko_msg_system_set_tx_power_rsp_t *tx_power_cmd_response;

	 	rssi = evt->data.evt_le_connection_rssi.rssi;

		if(evt->data.evt_le_connection_rssi.status != 0)
		{
			LOG_INFO("Failed to get rssi: %d", evt->data.evt_le_connection_rssi.status);
		}
		else
		{
			//LOG_INFO("RSSI: %d", rssi);

			if(rssi > -35)
				tx_power = -260;
			else if((rssi <= -35) && (rssi > -45))
				tx_power = -200;
			else if((rssi <= -45) && (rssi > -55))
				tx_power = -150;
			else if((rssi <= -55) && (rssi > -65))
				tx_power = -50;
			else if((rssi <= -65) && (rssi > -75))
				tx_power = 0;
			else if((rssi <= -75) && (rssi > -85))
				tx_power = 50;
			else
				tx_power = 80;

			tx_power_cmd_response = gecko_cmd_system_set_tx_power(tx_power);

			//LOG_INFO("Set Tx power: %d", tx_power_cmd_response->set_power);
		}

		break;
	}
}

/*
 * @func - gecko_ecen5823_update_client
 * @brief - Handle BT events on client side
 * @parameters - evt - event to be handled
 * @return - none
 */
void gecko_ecen5823_update_client(struct gecko_cmd_packet* evt)
{
	uint8_t server_addr[] = SERVER_BT_ADDRESS;

	switch (BGLIB_MSG_ID(evt->header)) {
		 case gecko_evt_system_boot_id:

			 displayPrintf(DISPLAY_ROW_NAME, "Client");

			 struct gecko_msg_system_get_bt_address_rsp_t *bt_public_addr = gecko_cmd_system_get_bt_address();

			 displayPrintf(DISPLAY_ROW_BTADDR, "%x.%x.%x.%x.%x.%x", bt_public_addr->address.addr[5],bt_public_addr->address.addr[4],bt_public_addr->address.addr[3],bt_public_addr->address.addr[2],bt_public_addr->address.addr[1],bt_public_addr->address.addr[0]);
			 displayPrintf(DISPLAY_ROW_BTADDR2, "%x.%x.%x.%x.%x.%x", server_addr[5],server_addr[4],server_addr[3],server_addr[2],server_addr[1],server_addr[0]);

			 /* Delete bondings */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_delete_bondings());

			 /* Configure security capabilities of device */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_configure(SECURITY_CONFIGURE_FLAGS, sm_io_capability_displayyesno));
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_set_bondable_mode(ALLOW_BONDING));

			 /* Start discovery */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_general_discoverable));

			 displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
			 displayUpdate();
			 break;

		 case gecko_evt_le_gap_scan_response_id:

			  /* Check if connected to required server and send connection request */
			   if(!strcmp(server_addr, evt->data.evt_le_gap_scan_response.address.addr))
			   {
				   BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address, evt->data.evt_le_gap_scan_response.address_type, le_gap_phy_1m));
			   }
			 break;

		 case gecko_evt_le_connection_opened_id:

			 /* Might need to be moved */
			 health_thermometer_uuid[0] = 0x09;
			 health_thermometer_uuid[1] = 0x18;

			 /* First global gatt state assigned whenever connection opened */
			 current_gatt_state = SERVICE_DISCOVERY; //Might have to move this

			 displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");
			 displayUpdate();

			 server_connection_handle = evt->data.evt_le_connection_opened.connection;

			 /* Set connection timing parameters and discover primary services by uuid */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_le_connection_set_timing_parameters(server_connection_handle, CONNECTION_INTERVAL_MIN, CONNECTION_INTERVAL_MAX, SLAVE_LATENCY, TIMEOUT, 0x0, 0x0));

			 /* Start bonding process */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_increase_security(server_connection_handle));

			 /* Discover temperature service */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_discover_primary_services_by_uuid(server_connection_handle, 2, health_thermometer_uuid)); //To be moved
			 break;

		case gecko_evt_sm_confirm_passkey_id:

			 displayPrintf(DISPLAY_ROW_PASSKEY, "Passkey %d", evt->data.evt_sm_confirm_passkey.passkey);
			 displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");
			 displayUpdate();
			 break;

		case gecko_evt_system_external_signal_id:

			 /* Handle button press interrupt */
			 if(((evt->data.evt_system_external_signal.extsignals & BUTTON_PRESSED) == BUTTON_PRESSED))
			 {
				 displayPrintf(DISPLAY_ROW_PASSKEY, "");
				 displayPrintf(DISPLAY_ROW_ACTION, "");
				 displayUpdate();

				 BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_passkey_confirm(server_connection_handle, ACCEPT_PASSKEY));
			 }
			 break;

		case gecko_evt_sm_bonded_id:

			 if(evt->data.evt_sm_bonded.bonding == 0xFF)
			 {
				 LOG_INFO("Pairing done but w/o bonding!\n");
			 }
			 else
			 {
				 /* Once bonded, discover button press service */
				 push_button_uuid[0] = 0x89;
				 push_button_uuid[1] = 0x62;
				 push_button_uuid[2] = 0x13;
				 push_button_uuid[3] = 0x2D;
				 push_button_uuid[4] = 0x2A;
				 push_button_uuid[5] = 0x65;
				 push_button_uuid[6] = 0xEC;
				 push_button_uuid[7] = 0x87;
				 push_button_uuid[8] = 0x3E;
				 push_button_uuid[9] = 0x43;
				 push_button_uuid[10] = 0xC8;
				 push_button_uuid[11] = 0x38;
				 push_button_uuid[12] = 0x01;
				 push_button_uuid[13] = 0x00;
				 push_button_uuid[14] = 0x00;
				 push_button_uuid[15] = 0x00;

				 /* Set global gatt state for push button service discovery */
				 current_gatt_state = PUSH_BUTTON_SERVICE_DISCOVERY;
				 displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
				 displayUpdate();
				 LOG_INFO("Bonding Successful!\n");
				 BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_discover_primary_services_by_uuid(server_connection_handle, 16, push_button_uuid));
			 }
			 break;

		case gecko_evt_sm_bonding_failed_id:

			 LOG_INFO("Pairing & Bonding failed due to 0x%x\n",evt->data.evt_sm_bonding_failed.reason);
			 break;

		case gecko_evt_gatt_service_id:

			/* Check if service received corresponds to health thermometer */
			if((health_thermometer_uuid[0] == evt->data.evt_gatt_service.uuid.data[0]) && (health_thermometer_uuid[1] == evt->data.evt_gatt_service.uuid.data[1]))
			{
				service_handle = evt->data.evt_gatt_service.service;
			}
			else
			{
				/* Push button service */
				push_button_service_handle = evt->data.evt_gatt_service.service;
			}
			break;

		case gecko_evt_gatt_procedure_completed_id: ;

			enum global_GATT_states next_gatt_state;

			switch(current_gatt_state)
			{
				case SERVICE_DISCOVERY:

					temperature_measurement_char[0] = 0x1C;
					temperature_measurement_char[1] = 0x2A;

					if(evt->data.evt_gatt_procedure_completed.result)
					{
						LOG_INFO("gatt_procedure_completed failed in relation to gatt_service: %x\n",evt->data.evt_gatt_procedure_completed.result);
					}
					else
					{
						/* Discover characteristics by UUID for health thermometer*/
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_discover_characteristics_by_uuid(server_connection_handle, service_handle, 2, temperature_measurement_char));
						next_gatt_state = CHARACTERISTIC_DISCOVERY;
					}
					break;

				case CHARACTERISTIC_DISCOVERY:

					if(evt->data.evt_gatt_procedure_completed.result)
					{
						LOG_INFO("gatt_procedure_completed failed in relation to gatt_characteristic: %x\n",evt->data.evt_gatt_procedure_completed.result);
					}
					else
					{
						/* Set characteristic notif for health thermometer*/
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_set_characteristic_notification(server_connection_handle, characteristic_handle, gatt_indication));
						next_gatt_state = NOTIFICATION;
					}
					break;

				case NOTIFICATION:
					if(evt->data.evt_gatt_procedure_completed.result)
					{
						LOG_INFO("gatt_procedure_completed failed in relation to gatt_set_characteristic_notif: %x\n",evt->data.evt_gatt_procedure_completed.result);
					}
					else
					{
						/* last time gatt_procedure_completed gets called within this connection. Assign a dummy state */
						next_gatt_state = READ_TEMP;
					}
					break;

				case PUSH_BUTTON_SERVICE_DISCOVERY:

					push_button_uuid[12] = 0x02;

					if(evt->data.evt_gatt_procedure_completed.result)
					{
						LOG_INFO("gatt_procedure_completed failed in relation to gatt_service: %x\n",evt->data.evt_gatt_procedure_completed.result);
					}
					else
					{
						/* Discover characteristics by UUID for push button service*/
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_discover_characteristics_by_uuid(server_connection_handle, push_button_service_handle, 16, push_button_uuid));
						next_gatt_state = PUSH_BUTTON_CHARACTERISTIC_DISCOVERY;
					}
					break;

				case PUSH_BUTTON_CHARACTERISTIC_DISCOVERY:
					if(evt->data.evt_gatt_procedure_completed.result)
					{
						LOG_INFO("gatt_procedure_completed failed in relation to gatt_characteristic: %x\n",evt->data.evt_gatt_procedure_completed.result);
					}
					else
					{
						/* Set characteristic notif for push button */
						BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_set_characteristic_notification(server_connection_handle, push_button_characteristic_handle, gatt_indication));
						next_gatt_state = NOTIFICATION;
					}
					break;
			}

			if(current_gatt_state != next_gatt_state)
			{
				LOG_INFO("GATT state machine transitioned from %d to state %d", current_gatt_state, next_gatt_state);
				current_gatt_state = next_gatt_state;
			}
			else
			{
				LOG_INFO("No transition of state. Current State: %d\n", current_gatt_state);
			}
			break;

		case gecko_evt_gatt_characteristic_id:

			/* Check if characteristic received matches temperature measurement characteristic */
			if((temperature_measurement_char[0] == evt->data.evt_gatt_characteristic.uuid.data[0]) && (temperature_measurement_char[1] == evt->data.evt_gatt_characteristic.uuid.data[1]))
			{
				characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
			}
			else
			{
				/* push button characteristic handle */
				push_button_characteristic_handle = evt->data.evt_gatt_characteristic.characteristic;
			}
			break;

		case gecko_evt_gatt_characteristic_value_id: ;

			uint8_t *temp_received;
			float temp_in_c;

			/* Check if characteristic handle is of temperature measurement and then process temp received */
			if(characteristic_handle == evt->data.evt_gatt_characteristic_value.characteristic)
			{
				BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_send_characteristic_confirmation(server_connection_handle));
				displayPrintf(DISPLAY_ROW_CONNECTION, "Handling Indications");
				temp_received = evt->data.evt_gatt_characteristic_value.value.data;
				temp_in_c = gattUint32ToFloat(temp_received);
				displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temperature: %.2f C", temp_in_c);
				displayUpdate();

				LOG_INFO("Temperature Client: %.2f\n", temp_in_c);
			}
			else if(push_button_characteristic_handle == evt->data.evt_gatt_characteristic_value.characteristic)
			{
				/* Receive button press status */
				BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_send_characteristic_confirmation(server_connection_handle));
				temp_received = evt->data.evt_gatt_characteristic_value.value.data;

				if(*temp_received)
				{
					displayPrintf(DISPLAY_ROW_ACTION, "Button Pressed");
					displayUpdate();
					LOG_INFO("Button Pressed");
				}
				else
				{
					displayPrintf(DISPLAY_ROW_ACTION, "Button Released");
					displayUpdate();
					LOG_INFO("Button Released");
				}
			}
			break;

		case gecko_evt_le_connection_closed_id:

			/* delete bondings */
			BTSTACK_CHECK_RESPONSE(gecko_cmd_sm_delete_bondings());

			/* Start discovering again */
			 BTSTACK_CHECK_RESPONSE(gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_general_discoverable));

			 displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");
			 displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
			 displayPrintf(DISPLAY_ROW_ACTION, "");
			 displayUpdate();
			 break;
	}
}

