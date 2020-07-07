#ifndef _SYMBOL_INCLUDE_H_
#define _SYMBOL_INCLUDE_H_

/**********************platform symbols  ***************************/
int vsnprintf(char *str, size_t size, const char *format, va_list ap);
char *setlocale(int category, const char *locale);
extern _ARMABI int memcmp(const void * /*s1*/, const void * /*s2*/,
                          size_t /*n*/) __attribute__((__nonnull__(1, 2)));
extern unsigned int strlen(char *s);
extern _ARMABI size_t strlen(const char * /*s*/) __attribute__((__nonnull__(1)));

/* Support the alias for the __aeabi_memcpy which may
   assume memory alignment.  */
void __aeabi_memcpy4(void *dest, const void *source,
                     size_t n) _ATTRIBUTE((alias("__aeabi_memcpy")));
void __aeabi_memcpy8(void *dest, const void *source,
                     size_t n) _ATTRIBUTE((alias("__aeabi_memcpy")));
/* Support the routine __aeabi_memcpy.  Can't alias to memcpy
   because it's not defined in the same translation unit.  */
void __aeabi_memcpy(void *dest, const void *source, size_t n);
void __rt_memcpy(void *dest, const void *source, size_t n);

/* Support the alias for the __aeabi_memclr which may assume memory alignment.  */
void __aeabi_memclr4(void *dest, size_t n) _ATTRIBUTE((alias("__aeabi_memclr")));
void __aeabi_memclr8(void *dest, size_t n) _ATTRIBUTE((alias("__aeabi_memclr")));
/* Support the routine __aeabi_memclr.  */
void __aeabi_memclr(void *dest, size_t n);
void __rt_memclr(void *dest, size_t n);
void __rt_memclr_w(void *dest, size_t n);

/* Support the alias for the __aeabi_memset which may assume memory alignment.  */
void __aeabi_memset4(void *dest, size_t n, int c) _ATTRIBUTE((alias("__aeabi_memset")));
void __aeabi_memset8(void *dest, size_t n, int c) _ATTRIBUTE((alias("__aeabi_memset")));
/* Support the routine __aeabi_memset.  Can't alias to memset
   because it's not defined in the same translation unit.  */
void __aeabi_memset(void *dest, size_t n, int c);
void _memset(void *dest, size_t n, int c);
void _memset_w(void *dest, size_t n, int c);

void exit(int status);
extern _ARMABI int strcmp(const char * /*s1*/, const char * /*s2*/) __attribute__((__nonnull__(1,
        2)));

/*support for system trace lib*/
extern size_t xFreeBytesRemaining[];
extern size_t xMinimumEverFreeBytesRemaining[];
extern size_t xHeapTotalSize[];
extern BlockLink_t xStart[];
extern TaskHandleList_t *pTaskHandleList;
extern BOOL_PATCH_FUNC patch_osif_os_timer_create;
extern uint16_t uxTimerCreateCount;
extern uint16_t uxTimerDeleteCount;
extern BOOL_PATCH_FUNC patch_osif_os_msg_send_intern;
extern BOOL_PATCH_FUNC patch_vTaskSwitchContext;
extern QueueHandle_t xTimerQueue;
extern BOOL_PATCH_FUNC patch_HardFaultRecord_TryToSave;
extern OTP_STRUCT otp;

/*support export os task signal api*/
extern BaseType_t xTaskGenericNotify(TaskHandle_t xTaskToNotify, uint32_t ulValue,
                                     eNotifyAction eAction,
                                     uint32_t *pulPreviousNotificationValue);
extern BaseType_t xTaskGenericNotifyFromISR(TaskHandle_t xTaskToNotify, uint32_t ulValue,
                                            eNotifyAction eAction, uint32_t *pulPreviousNotificationValue,
                                            BaseType_t *pxHigherPriorityTaskWoken);
extern BOOL_PATCH_FUNC patch_osif_os_task_signal_send;

/*support for hw aes cbc mode apis*/
bool hw_aes_encrypt128(uint32_t *p_in, uint32_t *p_out, uint16_t data_word_len, uint32_t *p_key,
                       uint32_t *p_iv,
                       T_HW_AES_MODE mode);
bool hw_aes_decrypt128(uint32_t *p_in, uint32_t *p_out, uint16_t data_word_len, uint32_t *p_key,
                       uint32_t *p_iv,
                       T_HW_AES_MODE mode);
bool hw_aes_encrypt256(uint32_t *p_in, uint32_t *p_out, uint16_t data_word_len, uint32_t *p_key,
                       uint32_t *p_iv,
                       T_HW_AES_MODE mode);
bool hw_aes_decrypt256(uint32_t *p_in, uint32_t *p_out, uint16_t data_word_len, uint32_t *p_key,
                       uint32_t *p_iv,
                       T_HW_AES_MODE mode);

/*support for retarget printf*/
extern uint32_t log_timestamp_get(void);
extern void LogUartTxChar(const uint8_t ch);
extern uint8_t log_seq_num;

/*support for flash high speed read api*/
extern T_FLASH_DEVICE_INFO flash_device_info;
extern T_FLASH_SPLIT_READ_INFO flash_split_read_info;
extern T_FLASH_DMA_CFG flash_dma_cfg;
bool flash_lock(T_LOCK_TYPE flash_lock_mode);
void flash_unlock(T_LOCK_TYPE flash_lock_mode);
uint32_t flash_split_read(uint32_t start_addr, uint32_t data_len, uint8_t *data);
bool flash_read(uint32_t start_addr, uint32_t data_len, uint8_t *data);

/* support for flash otp api */
extern FlashCB flash_erase_enter;
extern FlashCB flash_erase_exit;
bool flash_get_status_regs(uint8_t *status1, uint8_t *status2, uint8_t *status3);
bool flash_set_status_regs(uint8_t *status1, uint8_t *status2, uint8_t *status3);
bool flash_wait_busy(void);
bool flash_read_internal(uint32_t start_addr, uint32_t data_len, uint8_t *data);
bool flash_write_internal(uint32_t start_addr, uint32_t data_len, uint8_t *data);

/*support for slient ota*/
bool flash_cmd_rx(uint8_t cmd, uint8_t read_len, uint8_t *read_buf);
bool flash_cmd_tx(uint8_t cmd, uint8_t data_len, uint8_t *data_buf);
bool flash_write_enable(void);
uint32_t flash_get_rdid(void);
void flash_auto_write(uint32_t start_addr, uint32_t data);

/******support symbols for bee2_sdk.lib*******/
/*used for app unlock bp wtih flash lock*/
extern T_FLASH_RET flash_set_block_protect(uint8_t bp_lv);
extern T_FLASH_RET flash_sw_protect_unlock_by_addr(uint32_t unlock_addr, uint8_t *old_bp_lv);
extern T_FLASH_RET flash_get_block_protect(uint8_t *bp_lv);
extern uint32_t g_BasePri;
extern T_LOCK_TYPE  flash_lock_flag;

/*support save and trace wdg timeout reset reason to flash */
extern bool flash_erase(T_ERASE_TYPE type, uint32_t addr);
extern void vTaskStatusDump(void);
extern void dump_raw_memory_all(void);
extern void hardfault_print_buffered_log(void);
extern void LOGUARTDriverInit(void);
extern void hardfault_dump(T_HARDFAULT_RECORD *pHardFault_Record);
extern BOOL_PATCH_FUNC Patch_Dump_CPU_Register_and_Memory;
extern uint32_t xTickCount;
extern BOOL_PATCH_FUNC patch_osif_os_sys_time_get;

/* used by flash otp API */
bool spic_wait_busy(void);

/* used by SFDP API */
uint32_t spic_get_dr(T_SPIC_BYTE_NUM byte_num);
void spic_set_dummy_len(uint8_t dummy_len);

/*used for app update flash block protect level*/
void flash_get_query_info(void **query_info);
bool flash_load_query_info(void);
T_FLASH_RET flash_set_top_bottom(bool from_bottom);

/* support symbols for auto_k_rf.lib */
extern void btaon_fast_update(uint16_t offset, uint8_t mask, uint8_t data);
extern uint8_t hci_handle_le_receiver_test(HCI_CMD_PKT *hci_cmd_ptr);
extern uint32_t rtk_read_modem_radio_reg(uint8_t addr, uint8_t type);
extern void rtk_write_modem_radio_reg(uint8_t addr, uint8_t type, uint32_t val);
void (*phy_auto_gated_on)(void);
void (*phy_auto_gated_off)(void);

/*support for opening more os APIs*/
BaseType_t xQueueGenericSend(QueueHandle_t xQueue, const void *const pvItemToQueue,
                             TickType_t xTicksToWait, const BaseType_t xCopyPosition);
BaseType_t xQueueGenericSendFromISR(QueueHandle_t xQueue, const void *const pvItemToQueue,
                                    BaseType_t *const pxHigherPriorityTaskWoken, const BaseType_t xCopyPosition);

void vListInitialise(List_t *const pxList);
void vListInitialiseItem(ListItem_t *const pxItem);
void vListInsert(List_t *const pxList, ListItem_t *const pxNewListItem);
void vListInsertEnd(List_t *const pxList, ListItem_t *const pxNewListItem);
UBaseType_t uxListRemove(ListItem_t *const pxItemToRemove);
uint32_t rtlEnterCritical(void);
void rtlExitCritical(uint32_t ulSavedInterruptStatus);
void *pvPortMalloc(RAM_TYPE ramType, size_t xWantedSize);
void vPortFree(void *pv);

/**********************end platform symbols  **************************/

/*hci_if api for rcu single tone test*/
bool hci_if_open(P_HCI_IF_CALLBACK p_callback);
bool hci_if_close(void);
bool hci_if_write(uint8_t *p_buf, uint32_t len);
bool hci_if_confirm(uint8_t *p_buf);


bool BTIF_VendorGetResponse(uint8_t *pData, uint8_t len);

void gap_register_extend_cb(P_FUN_GAP_EXTEND_CB extend_callback);
extern P_FUN_LE_APP_CB gap_app_cb;
extern P_FUN_GAP_APP_CB gap_common_app_cb;

/*gap_lib.c symbols */
bool client_handle_btif_msg(T_BTIF_UP_MSG *p_msg);

/*gap_vendor_cmd.c symbols */
bool btif_vendor_cmd_req(uint16_t op, uint8_t len, uint8_t *p_param);

/*gap_ext_scan.c symbols */
void gap_send_dev_state(uint16_t cause);
extern T_GAP_DEV_STATE gap_device_state;

bool btif_le_ext_scan_req(T_BTIF_LE_SCAN_MODE mode,
                          T_BTIF_LE_SCAN_FILTER_DUPLICATES filter_duplicates,
                          uint16_t duration, uint16_t period);

bool btif_le_ext_scan_param_set_req(T_BTIF_LOCAL_ADDR_TYPE local_addr_type,
                                    T_BTIF_LE_SCAN_FILTER_POLICY filter_policy,
                                    uint8_t scan_phys,
                                    T_BTIF_LE_EXT_SCAN_PARAM *p_extended_scan_param);
bool btif_buffer_put(void *p_buf);


/*gap_ext_adv.c symbols */
void gap_send_msg_to_app(T_IO_MSG *p_msg);
bool btif_le_ext_adv_enable_req(T_BTIF_LE_EXT_ADV_MODE adv_mode, uint8_t num_of_sets,
                                T_BTIF_LE_EXT_ADV_SET_PARAM *p_adv_set_param);

bool btif_le_ext_adv_param_set_req(uint8_t adv_handle, uint16_t adv_event_prop,
                                   uint32_t primary_adv_interval_min, uint32_t primary_adv_interval_max,
                                   uint8_t primary_adv_channel_map, T_BTIF_LOCAL_ADDR_TYPE own_address_type,
                                   T_BTIF_REMOTE_ADDR_TYPE peer_address_type, uint8_t *p_peer_address,
                                   T_BTIF_LE_ADV_FILTER_POLICY filter_policy, uint8_t tx_power,
                                   T_BTIF_LE_PRIM_ADV_PHY_TYPE primary_adv_phy, uint8_t secondary_adv_max_skip,
                                   T_BTIF_LE_PHY_TYPE secondary_adv_phy, uint8_t adv_sid,
                                   T_BTIF_LE_SCAN_REQ_NOTIFY_TYPE scan_req_notification_enable);

bool btif_le_ext_adv_data_set_req(T_BTIF_LE_EXT_ADV_DATA_TYPE data_type,
                                  uint8_t adv_handle, T_BTIF_LE_ADV_FRAG_OP_TYPE op,
                                  T_BTIF_LE_ADV_FRAG_PREFERENCE_TYPE frag_preference,
                                  uint8_t data_len, uint8_t *p_data);
bool btif_le_modify_adv_set_req(T_BTIF_LE_ADV_SET_OP op, uint8_t adv_handle);
bool btif_le_set_adv_set_rand_addr_req(uint8_t *random_addr, uint8_t adv_handle);

/*gap_credit_based_conn.c symbols */
extern uint16_t gap_param_ds_pool_id;
extern uint16_t gap_param_ds_data_offset;
extern T_LE_LINK *gap_link_table;
void *btif_buffer_get(uint16_t pool_id, uint16_t size, uint16_t offset);

bool le_load_upper_stack_efuse(T_OTP_UPPERSTACK *p_otp_upper);

bool btif_le_credit_based_conn_req(uint16_t link_id, uint16_t le_psm, uint16_t mtu,
                                   uint16_t initial_credits, uint16_t credits_threshold);

bool btif_le_credit_based_conn_cfm(uint16_t link_id, uint16_t channel, uint16_t mtu,
                                   uint16_t initial_credits, uint16_t credits_threshold,
                                   T_BTIF_L2C_LE_CONN_STATUS cause);

bool btif_le_credit_based_disconn_req(uint16_t link_id, uint16_t channel);

bool btif_le_credit_based_disconn_cfm(uint16_t link_id, uint16_t channel);

bool btif_le_send_credit_req(uint16_t link_id, uint16_t channel, uint16_t credits);

bool btif_le_credit_based_data_req(void *p_buf, uint16_t link_id, uint16_t channel,
                                   uint16_t length, uint16_t offset);

bool btif_le_credit_based_data_cfm(uint16_t link_id, uint16_t channel, T_BTIF_CAUSE cause);

bool btif_le_credit_based_security_reg_req(uint16_t le_psm, bool active,
                                           T_BTIF_LE_SECURITY_MODE mode, uint8_t key_size);

bool btif_le_credit_based_psm_reg_req(uint16_t le_psm, uint8_t action);

bool le_link_check_conn_id_internal(const char *p_func_name, uint8_t conn_id);

T_LE_LINK *le_link_find_by_link_id(uint16_t link_id);

/*gap_aox.c symbols */
bool btif_le_set_suppl_test_params_req(bool is_receiver, uint8_t suppl_len, uint8_t suppl_slot_type,
                                       uint8_t num_antena_ids, uint8_t antenna_switching_pattern);

bool btif_le_set_conn_suppl_params_req(bool is_receiver, uint16_t link_id, uint8_t slot_duration,
                                       uint8_t num_antena_ids, uint8_t *p_antenna_id_list);

bool btif_le_conn_suppl_request_enable_req(uint16_t link_id, uint8_t enable,
                                           uint16_t suppl_interval,
                                           uint8_t suppl_len, uint8_t suppl_type);

bool btif_le_conn_suppl_response_enable_req(uint16_t link_id, uint8_t enable);

bool btif_le_read_antenna_info_req(void);

/* gap_lib_patch.c symbols*/
bool ecc_make_key(uint8_t public_key[64], uint8_t private_key[32], uint64_t *random);
void bt_crypto_f4(uint8_t u[32], uint8_t v[32], uint8_t x[16], uint8_t z, uint8_t res[16]);
bool sm_le_sc_local_oob_init(T_BTIF_LOCAL_OOB_DATA *p_local_oob_data);
bool sm_le_sc_peer_oob_init(T_BTIF_PEER_OOB_DATA *p_peer_oob_data);
bool btif_sw_reset_req(uint8_t mode);
bool le_save_key(T_LE_KEY_ENTRY *p_entry, T_GAP_KEY_TYPE key_type, uint8_t key_length,
                 uint8_t *key);
bool btif_just_work_req_cfm(uint8_t *bd_addr, T_BTIF_REMOTE_ADDR_TYPE remote_addr_type,
                            T_BTIF_CAUSE cause);
uint8_t le_get_key(T_LE_KEY_ENTRY *p_entry, T_GAP_KEY_TYPE key_type, uint8_t *key);
bool btif_le_gen_rand_addr(T_BTIF_ADDR_RAND rand_addr_type, uint8_t *p_addr);
bool bond_priority_queue_add(T_DEV_TYPE type, uint8_t idx);
uint32_t imp_flash_load(void *p_data, uint16_t start_offset, uint16_t block_size,
                        uint8_t size, uint8_t idx);
uint32_t flash_load_le_local_csrk(T_LE_LOCAL_CSRK *p_data, uint8_t idx);
uint32_t flash_load_le_local_ltk(T_LE_LOCAL_LTK *p_data, uint8_t idx);
uint32_t flash_load_le_remote_bd(T_LE_REMOTE_BD *p_data, uint8_t idx);
uint32_t flash_load_le_remote_csrk(T_LE_REMOTE_CSRK *p_data, uint8_t idx);
uint32_t flash_load_le_remote_irk(T_LE_REMOTE_IRK *p_data, uint8_t idx);
uint32_t flash_load_le_remote_ltk(T_LE_REMOTE_LTK *p_data, uint8_t idx);
uint8_t flash_read_le_cccd_length(uint8_t idx);
uint32_t imp_flash_save(void *p_data, uint16_t start_offset, uint16_t block_size,
                        uint8_t size, uint8_t idx);
T_LE_KEY_ENTRY *le_allocate_key_entry(uint8_t *bd_addr, T_GAP_REMOTE_ADDR_TYPE bd_type,
                                      uint8_t local_bd_type);
T_LE_KEY_ENTRY *le_find_entry_by_aes(uint8_t *unresolved_addr, uint8_t addr_type);
void le_link_release(T_LE_LINK *p_link);
bool btif_le_disconn_cfm(uint16_t link_id);
void gap_send_conn_state_msg(uint8_t conn_id, T_GAP_CONN_STATE new_state, uint16_t disc_cause);
bool btif_send_cmd(T_BTIF_DOWN_MSG *p_msg, bool alloc);

uint8_t   gap_link_credits;
uint16_t le_fs_remote_bd_offset;
uint16_t le_link_block_size;
uint16_t le_fs_start_offset;

/* auto_k_rf lib symbols*/
extern UINT32(*rtk_read_modem_radio_reg_pi)(UCHAR modem_page, UCHAR addr, UCHAR type);
extern void (*rtk_write_modem_radio_reg_pi)(UCHAR modem_page, UCHAR addr, UCHAR type, UINT16 val);
extern void (*rtk_update_rfc_reg_pi)(UINT8, UINT16, UINT16);

extern uint32_t SystemCpuClock;
#endif
