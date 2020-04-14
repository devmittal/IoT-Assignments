/*****************************************************************************
​ ​* ​ ​ @file​ ​  		myGecko.c
​ * ​ ​ @brief​ ​ 		Contains BT stack event handler
​ * ​ ​ @Author(s)​  	​​Devansh Mittal
​ * ​ ​ @Date​ ​​ 		October 4th, 2019
​ * ​ ​ @version​ ​ 		2.0
 *   @References 	soc-btmesh-light example project from SI BT mesh SDK
 *   				soc-btmesh-switch example project from SI BT mesh SDK
*****************************************************************************/
#include "gatt_db.h"
#include "native_gecko.h"
#include "main.h"
#include "log.h"
#include "em_letimer.h"
#include "myGecko.h"
#include "display.h"
#include "gecko_ble_errors.h"
#include "ble_device_type.h"
#include "ble_mesh_device_type.h"
#include <math.h>
#include <string.h>
#include "gpio.h"
#include "event.h"
#include "mesh_generic_model_capi_types.h"
#include "mesh_lib.h"

/*
 * @func - onoff_update
 * @brief - Update generic on/off state.
 * @parameters - element_index Server model element index
 * 				 remaining_ms The remaining time in milliseconds.
 * @return - Status of the update operation
 */
static errorcode_t onoff_update(uint16_t element_index, uint32_t remaining_ms)
{
	struct mesh_generic_state current, target;

	current.kind = mesh_generic_state_on_off;
	current.on_off.on = button_state.onoff_current;

	target.kind = mesh_generic_state_on_off;
	target.on_off.on = button_state.onoff_target;

	return mesh_lib_generic_server_update(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID,
										element_index,
										&current,
										&target,
										remaining_ms);
}

/*
 * @func - onoff_update_and_publish
 * @brief - Update generic on/off state and publish model state to the network.
 * @parameters - element_index Server model element index
 * 				 remaining_ms The remaining time in milliseconds.
 * @return - Status of the update operation
 */
static errorcode_t onoff_update_and_publish(uint16_t element_index,
                                            uint32_t remaining_ms)
{
	errorcode_t e;

	e = onoff_update(element_index, remaining_ms);

	if (e == bg_err_success) {
	e = mesh_lib_generic_server_publish(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID,
										element_index,
										mesh_generic_state_on_off);
	}

	return e;
}

/***************************************************************************//**
 * This function process the requests for the generic on/off model.
 *
 * @param[in] model_id       Server model ID.
 * @param[in] element_index  Server model element index.
 * @param[in] client_addr    Address of the client model which sent the message.
 * @param[in] server_addr    Address the message was sent to.
 * @param[in] appkey_index   The application key index used in encrypting the request.
 * @param[in] request        Pointer to the request structure.
 * @param[in] transition_ms  Requested transition time (in milliseconds).
 * @param[in] delay_ms       Delay time (in milliseconds).
 * @param[in] request_flags  Message flags. Bitmask of the following:
 *                           - Bit 0: Nonrelayed. If nonzero indicates
 *                                    a response to a nonrelayed request.
 *                           - Bit 1: Response required. If nonzero client
 *                                    expects a response from the server.
 ******************************************************************************/
static void onoff_request(uint16_t model_id,
                          uint16_t element_index,
                          uint16_t client_addr,
                          uint16_t server_addr,
                          uint16_t appkey_index,
                          const struct mesh_generic_request *request,
                          uint32_t transition_ms,
                          uint16_t delay_ms,
                          uint8_t request_flags)
{
	LOG_INFO("ON/OFF request: requested state=<%s>\r\n", request->on_off ? "Pressed" : "Released");
	if (button_state.onoff_current == request->on_off)
	{
	    LOG_INFO("Request for current state received; no op\r\n");
	}
	else
	{
		LOG_INFO("Button change to <%s>\r\n", request->on_off ? "Pressed" : "Released");

		if (transition_ms == 0 && delay_ms == 0)
		{ // Immediate change
			button_state.onoff_current = request->on_off;
			button_state.onoff_target = request->on_off;

			request->on_off ? displayPrintf(DISPLAY_ROW_ACTION, "Button Pressed") : displayPrintf(DISPLAY_ROW_ACTION, "Button Released");
		}
	}
	onoff_update_and_publish(element_index, 0);
}

/***************************************************************************//**
 * This function is a handler for generic on/off change event.
 *
 * @param[in] model_id       Server model ID.
 * @param[in] element_index  Server model element index.
 * @param[in] current        Pointer to current state structure.
 * @param[in] target         Pointer to target state structure.
 * @param[in] remaining_ms   Time (in milliseconds) remaining before transition
 *                           from current state to target state is complete.
 ******************************************************************************/
static void onoff_change(uint16_t model_id,
                         uint16_t element_index,
                         const struct mesh_generic_state *current,
                         const struct mesh_generic_state *target,
                         uint32_t remaining_ms)
{
	if (current->on_off.on != button_state.onoff_current)
	{
		LOG_INFO("on-off state changed %u to %u\r\n", button_state.onoff_current, current->on_off.on);

		button_state.onoff_current = current->on_off.on;
	}
	else
	{
		LOG_INFO("dummy onoff change - same state as before\r\n");
	}
}

void lpn_init(void)
{
	// Do not initialize LPN if lpn is currently active
	// or any GATT connection is opened
	if (lpn_active || num_connections)
		return;

	BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_lpn_init());

	lpn_active = 1;
	LOG_INFO("LPN Initialized\n");

	// Configure the lpn with following parameters:
	// - Minimum friend queue length = 2
	// - Poll timeout = 5 seconds
	BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_lpn_configure(2, 5 * 1000));

	BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_lpn_establish_friendship(0));
}

void lpn_deinit(void)
{
	uint16 result;

	if (!lpn_active)
		return; // lpn feature is currently inactive

	// Terminate friendship if exist
	BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_lpn_terminate_friendship());
	// turn off lpn feature
	BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_lpn_deinit());
	lpn_active = 0;
	LOG_INFO("LPN deinitialized\r\n");
}

/*
 * @func - init_mesh
 * @brief - Initialize Server/Client parameters.
 * @parameters - void.
 * @return - void.
 */
void init_mesh(void)
{
	if(DeviceUsesClientModel())
	{
		BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_generic_client_init());
	}

	if(DeviceUsesServerModel())
	{
		BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_generic_server_init());
	}

	if(DeviceIsOnOffPublisher())
	{
		mesh_lib_init(malloc,free,8);
	}

	if(DeviceIsOnOffSubscriber())
	{
		uint16_t _primary_elem_index = 0;

		mesh_lib_init(malloc,free,9);
		BTSTACK_CHECK_RESPONSE(gecko_cmd_mesh_friend_init());
		button_state.onoff_current = 0;
		mesh_lib_generic_server_register_handler(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID, 0, onoff_request, onoff_change);
		onoff_update_and_publish(_primary_elem_index, 0);
	}
}

/***************************************************************************//**
 * This function is called to initiate factory reset. Factory reset may be
 * initiated by keeping one of the WSTK pushbuttons pressed during reboot.
 * Factory reset is also performed if it is requested by the provisioner
 * (event gecko_evt_mesh_node_reset_id).
 ******************************************************************************/
void initiate_factory_reset(void)
{
	LOG_INFO("factory reset\r\n");
	displayPrintf(DISPLAY_ROW_ACTION, "Factory Reset");

	/* perform a factory reset by erasing PS storage. This removes all the keys and other settings
	 that have been configured for this node */
	gecko_cmd_flash_ps_erase_all();
	// reboot after a small delay
	gecko_cmd_hardware_set_soft_timer(2 * 32768, TIMER_ID_FACTORY_RESET, 1);
}

/*
 * @func - gecko_ecen5823_update
 * @brief - Handle BTMesh events
 * @parameters - evt - event to be handled
 * @return - none
 */
void gecko_ecen5823_update(uint32_t evt_id, struct gecko_cmd_packet *evt)
{
	handle_gecko_event(evt_id, evt);

	switch (evt_id) {
	 case gecko_evt_system_boot_id: ;

	 	 /* Factory reset device */
	 	if((!GPIO_PinInGet(PB0_port, PB0_pin)) || (!GPIO_PinInGet(PB1_port, PB1_pin)))
	 	{
	 		initiate_factory_reset();
	 	}
	 	else
	 	{
	 		/* Set attributes to init device */
			char name[20];

			struct gecko_msg_system_get_bt_address_rsp_t *bt_public_addr = gecko_cmd_system_get_bt_address();
			displayPrintf(DISPLAY_ROW_BTADDR, "%x.%x.%x.%x.%x.%x", bt_public_addr->address.addr[5],bt_public_addr->address.addr[4],bt_public_addr->address.addr[3],bt_public_addr->address.addr[2],bt_public_addr->address.addr[1],bt_public_addr->address.addr[0]);

			if(DeviceIsOnOffPublisher())
			{
				displayPrintf(DISPLAY_ROW_NAME, "Publisher");
				sprintf(name, "5823Pub %02x:%02x", bt_public_addr->address.addr[1],bt_public_addr->address.addr[0]);
			}
			else
			{
				displayPrintf(DISPLAY_ROW_NAME, "Subscriber");
				sprintf(name, "5823Sub %02x:%02x", bt_public_addr->address.addr[1],bt_public_addr->address.addr[0]);
			}
			LOG_INFO("Device name: '%s'\r\n", name);
			BTSTACK_CHECK_RESPONSE(gecko_cmd_gatt_server_write_attribute_value(gattdb_device_name, 0, strlen(name), (uint8_t *)name));
	 	}
		break;

	 case gecko_evt_mesh_node_initialized_id:

		 /* Beaconing if not provisioned done in gecko_main.c */
		 if (evt->data.evt_mesh_node_initialized.provisioned)
		 {
			 LOG_INFO("Already provisioned\n");
			 elem_index = 0;
			 init_mesh();
			 if(DeviceIsOnOffPublisher())
			 {
				 lpn_init();
			 }
		 }
		 break;
	 case gecko_evt_mesh_node_provisioning_started_id:

		 displayPrintf(DISPLAY_ROW_ACTION, "Provisioning");
		 break;

	 case gecko_evt_mesh_node_provisioned_id:

		 elem_index = 0;
		 init_mesh();
		 displayPrintf(DISPLAY_ROW_ACTION, "Provisioned");
		 break;

	 case gecko_evt_mesh_node_provisioning_failed_id:

		 displayPrintf(DISPLAY_ROW_ACTION, "Provision Fail");
		 break;

	 case gecko_evt_mesh_generic_server_client_request_id:

		 if(DeviceUsesServerModel())
			 mesh_lib_generic_server_event_handler(evt);
		 break;

	 case gecko_evt_mesh_generic_server_state_changed_id:

		 if(DeviceUsesServerModel())
			 mesh_lib_generic_server_event_handler(evt);
		 break;

	 case gecko_evt_le_connection_opened_id:

		 num_connections++;
		 displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

		 if(DeviceIsOnOffPublisher())
			 lpn_deinit();
		 break;

	 case gecko_evt_le_connection_closed_id:

		 if (num_connections > 0)
		 {
			 if (--num_connections == 0)
			 {
				 displayPrintf(DISPLAY_ROW_CONNECTION, "");
				 if(DeviceIsOnOffPublisher())
					 lpn_init(); // initialize lpn when there is no active connection
			 }
		 }
		 break;

	 case gecko_evt_mesh_node_reset_id:

	       initiate_factory_reset();
	       break;

	 case gecko_evt_system_external_signal_id: ;

		 struct mesh_generic_request req;
		 static uint32_t trid = 0;

		 /* Handle display update timer interrupt */
		 if(((evt->data.evt_system_external_signal.extsignals & DISPLAY_UPDATE) == DISPLAY_UPDATE))
		 {
			 displayUpdate();
		 }

		 /* If publisher and button pressed, send data to network */
		 if(DeviceUsesClientModel())
		 {
			 if(((evt->data.evt_system_external_signal.extsignals & BUTTON_PRESSED) == BUTTON_PRESSED))
			 {
				 uint16_t resp;
				 req.kind = mesh_generic_request_on_off;
				 req.on_off = GPIO_PinInGet(PB0_port, PB0_pin) ? MESH_GENERIC_ON_OFF_STATE_OFF : MESH_GENERIC_ON_OFF_STATE_ON;

				 trid++;

				 resp = mesh_lib_generic_client_publish(
					 MESH_GENERIC_ON_OFF_CLIENT_MODEL_ID,
					 elem_index,
					 trid,
					 &req,
					 0,
					 0,
					 0
					 );

				 if (resp)
				 {
					 LOG_INFO("gecko_cmd_mesh_generic_client_publish failed,code %x\r\n", resp);
				 }
				 else
				 {
					 LOG_INFO("Request sent, trid = %u\r\n", trid);
				 }
			 }
		 }
		 break;

	 case gecko_evt_hardware_soft_timer_id:
		switch (evt->data.evt_hardware_soft_timer.handle)
		{
			case TIMER_ID_FACTORY_RESET:
				// reset the device to finish factory reset
				gecko_cmd_system_reset(0);
				break;
		}
		break;
	}
}
