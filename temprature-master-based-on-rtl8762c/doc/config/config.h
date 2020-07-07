/*============================================================================*
 *                              Bee2 SDK
 *============================================================================*/
/**
 * \addtogroup  Bee2_SDK   Bee2 SDK
 * \defgroup    APP_EXT     APP
 * \brief       Extension support for user application task
 * \ingroup     Bee2_SDK
 * \defgroup    GAP         GAP
 * \brief       General interface accessing to BT stack
 * \ingroup     Bee2_SDK
 * \defgroup    MCU         MCU
 * \brief       MCU related module code
 * \ingroup     Bee2_SDK
 * \defgroup    OS          OSIF
 * \brief       OS abstraction layer implementation
 * \ingroup     Bee2_SDK
 * \defgroup    PLATFORM    Platform
 * \brief       Platform module provides misc functionality for application development
 * \ingroup     Bee2_SDK
 * \defgroup    PROFILE_API Profile
 * \brief       Profile level API
 * \ingroup     Bee2_SDK
 * \defgroup    SAMPLE_APP  Sample App
 * \brief       Sample application for demostrating how to use the library
 * \ingroup     Bee2_SDK
 * \defgroup    RTK_Periph_Driver   IO Driver
 * \brief       Peripheral IO drivers
 * \ingroup     Bee2_SDK
 */

/*============================================================================*
 *                              APP Extension
 *============================================================================*/
/**
 * \addtogroup  APP_EXT
 * \defgroup    APP_MSG     APP Message
 * \ingroup     APP_EXT
 * \defgroup    DFU_API     DFU API Sets
 * \ingroup     APP_EXT
 * \defgroup    DATA_UART_CMD     Data Uart Command Module
 * \ingroup     APP_EXT
 * \defgroup    BLE_PRIV_MODULE   BLE Privacy Management Module
 * \ingroup     APP_EXT
 */



/*============================================================================*
 *                              GAP
 *============================================================================*/
/**
 * \addtogroup  GAP
 * \defgroup    GAP_COMMON_MODULE       GAP Common Module
 * \ingroup     GAP
 * \defgroup    GAP_LE           GAP LE Module
 * \brief       GAP LE parameters and link related functions
 * \ingroup     GAP
 * \defgroup    BTTYPES          BT Types
 * \ingroup     GAP
 */

/**
 * \addtogroup  GAP_LE
 * \defgroup    GAP_MSG_MODULE           GAP Message
 * \ingroup     GAP_LE
 * \defgroup    GAP_PRIVACY_MODULE       GAP LE Privacy
 * \ingroup     GAP_LE
 * \defgroup    Observer_Role            GAP Observer Role
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_STORAGE           GAP LE Storage
 * \ingroup     GAP_LE
 * \defgroup    GAP_BROADCASTER_Role     GAP Broadcaster Role
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_Bond_Manager      GAP LE Bond Manager
 * \ingroup     GAP_LE
 * \defgroup    GAP_CB_MSG_MODULE        GAP Callback Message
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_CONNECTION_MODULE GAP LE Connection Module
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_COMMON            GAP LE Common Module
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_TYPES             GAP LE Related Definitions
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_DTM               GAP LE Direct Test Mode
 * \ingroup     GAP_LE
 * \defgroup    GAP_LE_VENDOR            GAP LE Vendor Extended Function
 * \ingroup     GAP_LE
 */

/**
 * \addtogroup  GAP_COMMON_MODULE
 * \defgroup    GAP_COMMON       GAP Common
 * \ingroup     GAP_COMMON_MODULE
 */

/*============================================================================*
 *                              MCU
 *============================================================================*/
/**
 * \addtogroup  MCU
 * \defgroup    SYSTEM_INIT System Init
 * \ingroup     MCU
 * \defgroup    SYSTEM_RTL876X  System RTL876X
 * \ingroup     MCU
 */

/*============================================================================*
 *                              OSIF APIs
 *============================================================================*/
/**
 * \addtogroup  OS
 * \defgroup    Schedule    Kernel Scheduler
 * \ingroup     OS
 * \defgroup    Synchronization     Inter-Task Communication
 * \ingroup     OS
 * \defgroup    Task    Task Management
 * \ingroup     OS
 * \defgroup    Timer   Timer Management
 * \ingroup     OS
 * \defgroup    Memory  Memory Management
 * \ingroup     OS
 * \defgroup    Message Message Queue
 * \ingroup     OS
 * \defgroup    Pool    Pool Management
 * \ingroup     OS
 * \defgroup    Queue   List Queue
 * \ingroup     OS
 */






/*============================================================================*
 *                              Platform
 *============================================================================*/
/**
 * \addtogroup  PLATFORM
 * \defgroup    MEM_CONFIG Memory Configure
 * \ingroup     PLATFORM
 * \defgroup    PLATFORM_UTILS Platform Utilities
 * \ingroup     PLATFORM
 * \defgroup    RTL876X Rtl876x
 * \ingroup     PLATFORM
 * \defgroup    SHA256_API Sha256
 * \ingroup     PLATFORM
 * \defgroup    TRACE       Trace
 * \ingroup     PLATFORM
 * \defgroup    AES_API AES API Sets
 * \ingroup     PLATFORM
 * \defgroup    APP_SECTION APP Section
 * \ingroup     PLATFORM
 * \defgroup    CRC_FCS CRC Implementation
 * \ingroup     PLATFORM
 * \defgroup    DEBUG_MONITOR Debug Monitor
 * \ingroup     PLATFORM
 * \defgroup    DLPS_PLATFORM DLPS Platform
 * \ingroup     PLATFORM
 * \defgroup    FLASH_DEVICE    Flash Device
 * \ingroup     PLATFORM
 * \defgroup    FTL    Flash Transport Layer
 * \ingroup     PLATFORM
 * \defgroup    HW_AES    Hardware AES
 * \ingroup     PLATFORM
 * \defgroup    APP_DEFINE    APP Define
 * \ingroup     PLATFORM
 * \defgroup    OVERLAY_MANAGER    Overlay Manager
 * \ingroup     PLATFORM
 * \defgroup    RANDOM_GENERATOR    Random Generator
 * \ingroup     PLATFORM
 * \defgroup    BUILD_CONFIGURATION    Build Configuration
 * \ingroup     PLATFORM
 * \defgroup    TEST_MODE    Test Mode
 * \ingroup     PLATFORM
 */









/*============================================================================*
 *                              Profile APIs
 *============================================================================*/
/**
 * \addtogroup  PROFILE_API
 * \defgroup    Bluetooth_Clients Bluetooth Clients
 * \brief       Bluetooth service client implementation
 * \details
  GATT Clients are used to discover, read, write and obtain indications of remote services, as well as
  configuring broadcast of attributes.

  The following scenarios are covered by GATT clients:

  1) Exchanging configuration

  2) Discovery of services and characteristics on a device

  3) Reading a characteristic value

  4) Writing a characteristic value

  5) Receiving notification of a characteristic value

  6) Receiving indication of a characteristic value
 * \ingroup     PROFILE_API
 * \defgroup    Bluetooth_Services Bluetooth Services
 * \brief       Bluetooth services
 * \ingroup     PROFILE_API
 * \defgroup    GATT_SERVER_API GATT Server API
 * \ingroup     PROFILE_API
 * \defgroup    GATT_CLIENT_API GATT Client API
 * \ingroup     PROFILE_API
 */

/**
 * \addtogroup  Bluetooth_Clients
 * \defgroup    ANCS_CLIENT ANCS Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    SIMP_Client Simple BLE Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    BAS_CLIENT Battery Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    GAPS_Client GAP Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    GATTS_Client GATT Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    IAS_Client Ias service client
 * \ingroup     Bluetooth_Clients
 * \defgroup    IPSS_CLIENT Internet Protocol Support Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    KNS_Client Kns service client
 * \ingroup     Bluetooth_Clients
 * \defgroup    DIS_Client Dis service client
 * \ingroup     Bluetooth_Clients
 * \defgroup    LLS_Client lls service client
 * \ingroup     Bluetooth_Clients
 * \defgroup    TPS_Client tps service client
 * \ingroup     Bluetooth_Clients
 * \defgroup    HIDS_CLIENT HID Service Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    GCS_Client General Common Services Client
 * \ingroup     Bluetooth_Clients
 * \defgroup    AMS_CLIENT AMS Client
 * \ingroup     Bluetooth_Clients
 */


/**
 * \addtogroup  Bluetooth_Services
 * \defgroup    SPS Scan Parameters Service
 * \ingroup     Bluetooth_Services
 * \defgroup    TPS Tx Power Service
 * \ingroup     Bluetooth_Services
 * \defgroup    BAS Battery Service
 * \ingroup     Bluetooth_Services
 * \defgroup    DIS Device Information Service
 * \ingroup     Bluetooth_Services
 * \defgroup    HIDS_KB Human Interface Device Service of Keyboard
 * \ingroup     Bluetooth_Services
 * \defgroup    HIDS_RmC HIDS Human Interface Device Service of Remote Controller
 * \ingroup     Bluetooth_Services
 * \defgroup    ATVV ATV Voice Service
 * \ingroup     Bluetooth_Services
 * \defgroup    HIDS_MS Human Interface Device Service of Mouse
 * \ingroup     Bluetooth_Services
 * \defgroup    HIDS Human Interface Device Service
 * \ingroup     Bluetooth_Services
 * \defgroup    IAS Immediate Alert Service
 * \ingroup     Bluetooth_Services
 * \defgroup    LLS Link Loss Service
 * \ingroup     Bluetooth_Services
 * \defgroup    OTA_SERVICE OTA Service
 * \ingroup     Bluetooth_Services
 * \defgroup    SIMP_Service Simple Ble Service
 * \ingroup     Bluetooth_Services
 * \defgroup    GAP_GATT_SERVICE GAP and GATT Inbox Services
 * \ingroup     Bluetooth_Services
 * \defgroup    HTS Health Thermometer Service
 * \ingroup     Bluetooth_Services
 * \defgroup    HRS Heart Rate Service
 * \ingroup     Bluetooth_Services
 * \defgroup    LNS Location And Navigation Service
 * \ingroup     Bluetooth_Services
 * \defgroup    WSS Weight Scale Service
 * \ingroup     Bluetooth_Services
 * \defgroup    RSCS Running Speed and Cadence Service
 * \ingroup     Bluetooth_Services
 * \defgroup    CSCS Cycling Speed and Cadence Service
 * \ingroup     Bluetooth_Services
 * \defgroup    GLS Glucose Service
 * \ingroup     Bluetooth_Services
 * \defgroup    IPSS Internet Protocol Support Service
 * \ingroup     Bluetooth_Services
 * \defgroup    ADS ADS
 * \ingroup     Bluetooth_Services
 * \defgroup    PLXS Pulse Oximeter Service
 * \ingroup     Bluetooth_Services
 */



/*============================================================================*
 *                              Sample App
 *============================================================================*/



/*============================================================================*
 *                              Peripheral Driver
 *============================================================================*/
/**
* \addtogroup  RTK_Periph_Driver
* \defgroup    IO_DLPS IO DLPS
* \ingroup     RTK_Periph_Driver
* \defgroup    IR IR
* \ingroup     RTK_Periph_Driver
* \defgroup    KeyScan KeyScan
* \ingroup     RTK_Periph_Driver
* \defgroup    LPC LPC
* \ingroup     RTK_Periph_Driver
* \defgroup    NVIC NVIC
* \ingroup     RTK_Periph_Driver
* \defgroup    PINMUX PINMUX
* \ingroup     RTK_Periph_Driver
* \defgroup    QDEC QDEC
* \ingroup     RTK_Periph_Driver
* \defgroup    RCC RCC
* \ingroup     RTK_Periph_Driver
* \defgroup    RTC RTC
* \ingroup     RTK_Periph_Driver
* \defgroup    LCD LCD
* \ingroup     RTK_Periph_Driver
* \defgroup    SPI SPI
* \ingroup     RTK_Periph_Driver
* \defgroup    TIM TIM
* \ingroup     RTK_Periph_Driver
* \defgroup    UART UART
* \ingroup     RTK_Periph_Driver
* \defgroup    WATCH_DOG Watch Dog
* \ingroup     RTK_Periph_Driver
* \defgroup    THREE_WIRE_SPI 3WIRE SPI
* \ingroup     RTK_Periph_Driver
* \defgroup    ADC ADC
* \ingroup     RTK_Periph_Driver
* \defgroup    GDMA GDMA
* \ingroup     RTK_Periph_Driver
* \defgroup    GPIO GPIO
* \ingroup     RTK_Periph_Driver
* \defgroup    I2C I2C
* \ingroup     RTK_Periph_Driver
* \defgroup    I2S I2S
* \ingroup     RTK_Periph_Driver
* \defgroup    CODEC CODEC
* \ingroup     RTK_Periph_Driver
* \defgroup    ADC ADC
* \ingroup     RTK_Periph_Driver
* \defgroup    AON_WATCH_DOG AON Watch Dog
* \ingroup     RTK_Periph_Driver
*/





