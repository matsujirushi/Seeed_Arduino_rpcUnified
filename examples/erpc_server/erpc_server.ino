#include "Seeed_erpcUnified.h"

T_APP_RESULT ble_gap_callback(uint8_t cb_type, void *p_cb_data)
{
  T_APP_RESULT result = APP_RESULT_SUCCESS;
  T_LE_CB_DATA *p_data = (T_LE_CB_DATA *)p_cb_data;
  switch (cb_type)
  {
  case GAP_MSG_LE_SCAN_INFO:
  {
    Serial.printf("GAP_MSG_LE_SCAN_INFO:adv_type 0x%x\n\r", p_data->p_le_scan_info->adv_type);
    Serial.printf("bd_addr %02x:%02x:%02x:%02x:%02x:%02x\n\r",
                  (p_data->p_le_scan_info->bd_addr)[5],
                  (p_data->p_le_scan_info->bd_addr)[4],
                  (p_data->p_le_scan_info->bd_addr)[3],
                  (p_data->p_le_scan_info->bd_addr)[2],
                  (p_data->p_le_scan_info->bd_addr)[1],
                  (p_data->p_le_scan_info->bd_addr)[0]);
    Serial.printf("remote_addr_type %d, rssi %d, data_len %d\n\r",
                  p_data->p_le_scan_info->remote_addr_type,
                  p_data->p_le_scan_info->rssi,
                  p_data->p_le_scan_info->data_len);
  }
  break;
  }
  return result;
}
extern void ble_handle_gap_msg(T_IO_MSG *p_gap_msg);
extern T_APP_RESULT ble_gatt_client_callback(T_CLIENT_ID client_id, uint8_t conn_id, void *p_data);

uint8_t conn_id = 0xff;

/** @brief  GAP - scan response data (max size = 31 bytes) */
static const uint8_t scan_rsp_data[] =
{
    0x03,                             /* length */
    GAP_ADTYPE_APPEARANCE,            /* type="Appearance" */
    LO_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
    HI_WORD(GAP_GATT_APPEARANCE_UNKNOWN),
};

/** @brief  GAP - Advertisement data (max size = 31 bytes, best kept short to conserve power) */
static const uint8_t adv_data[] =
{
    /* Flags */
    0x02,             /* length */
    GAP_ADTYPE_FLAGS, /* type="Flags" */
    GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,
    /* Service */
    0x03,             /* length */
    GAP_ADTYPE_16BIT_COMPLETE,
    LO_WORD(0x180F),
    HI_WORD(0x180F),
    /* Local name */
    0x0F,             /* length */
    GAP_ADTYPE_LOCAL_NAME_COMPLETE,
    'B', 'L', 'E', '_', 'P', 'E', 'R', 'I', 'P', 'H', 'E', 'R', 'A', 'L',
};
char v1_user_descr[] = "V1 read characteristic";
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial)
  {
  };
  delay(2000);
  Serial.printf("ble_init\n\r");
  ble_init();
  ble_server_init(BLE_SERVER_MAX_APPS);
  le_register_app_cb(ble_gap_callback);
  le_register_msg_handler(ble_handle_gap_msg);
  le_register_gattc_cb(ble_gatt_client_callback);

  le_adv_set_param(GAP_PARAM_ADV_DATA, sizeof(adv_data), (void *)adv_data);
	le_adv_set_param(GAP_PARAM_SCAN_RSP_DATA, sizeof(scan_rsp_data), (void *)scan_rsp_data);

  ble_service_t srcv;

	srcv.uuid_length = UUID_16BIT_SIZE;
	uint16_t srcv_uuid = 0x180F;
	memcpy(&(srcv.uuid), &srcv_uuid, 2);
	srcv.is_primary = true;

	uint8_t srcv_app_id = ble_create_service(srcv);
	Serial.printf("srcv_app_id: %d\n\r", srcv_app_id);

	ble_char_t CHAR;
	CHAR.uuid_length = UUID_16BIT_SIZE;
	uint16_t CHAR_uuid = 0x2A19;
	memcpy(&(CHAR.uuid), &CHAR_uuid, 2);
	CHAR.properties = (GATT_CHAR_PROP_READ | GATT_CHAR_PROP_NOTIFY);
  CHAR.permissions = GATT_PERM_READ;
	uint8_t char_handle1 = ble_create_char(srcv_app_id, CHAR);
	Serial.printf("char_handle1: %d\n\r", char_handle1);
	ble_desc_t desc;

	desc.flags = ATTRIB_FLAG_VALUE_INCL | ATTRIB_FLAG_CCCD_APPL;
	desc.uuid_length = UUID_16BIT_SIZE;
	uint16_t desc_uuid = 0x2902;
	uint16_t default_vlaue = 0x0000;
	memcpy(&(desc.uuid), &desc_uuid, 2);
	memcpy(&(desc.uuid[2]), &default_vlaue, 2);
	desc.p_value = NULL;
	desc.vlaue_length = 2;
	desc.permissions =   (GATT_PERM_READ | GATT_PERM_WRITE) ;
	uint8_t desc_handle1 = ble_create_desc(srcv_app_id, char_handle1, desc);
	Serial.printf("desc_handle1: %d\n\r", desc_handle1);

  ble_desc_t desc2;
	desc2.flags = ATTRIB_FLAG_VOID | ATTRIB_FLAG_ASCII_Z;
	desc2.uuid_length = UUID_16BIT_SIZE;
	uint16_t desc2_uuid = 0x2901;
  memcpy(&(desc2.uuid), &desc2_uuid, 2);
	desc2.p_value = (uint8_t *)v1_user_descr;
	desc2.vlaue_length = (sizeof(v1_user_descr) - 1);
	desc2.permissions =   (GATT_PERM_READ) ;
	uint8_t desc_handle2 = ble_create_desc(srcv_app_id, char_handle1, desc2);
  Serial.printf("desc_handle2: %d\n\r", desc_handle2);
  
  ble_service_start(srcv_app_id);

  ble_start();

  delay(2000);
	le_adv_start();


}

void loop()
{
  delay(1000);
  Serial.printf(".");
  // rpc_le_scan_start();
  // delay(10000);
  // rpc_le_scan_stop();
  // delay(5000);
}
