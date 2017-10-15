/******************************************************************************/
/* User Level #define Macros                                                  */
/******************************************************************************/

typedef unsigned char u8;
typedef unsigned int u16;
typedef unsigned long u32;

#ifdef bluebox
#define SW_VERSION_MAJOR      1
#define SW_VERSION_MINOR      0
#define SW_VERSION_BUILD      2
#endif

#define LOBYTE(w)((char)(w))
#define HIBYTE(w)((char)(((int)(w) >> 8) & 0xFF))

#define LOWORD(w)((int)(w))
#define HIWORD(w)((int)(((long)(w) >> 16) & 0xFFFF))

#define ENABLE 	1
#define DISABLE 0

#define SET 	1 
#define RST     0 

#define ON 	1
#define OFF 	0

#define HIGH 	1
#define LOW 	0

#define TRUE	1
#define FALSE	0

#define CLEAR	0

#define RX_MODE 0
#define TX_MODE 1

#define INPUT 	1
#define OUTPUT	0

#define MCP3002_CH0 0xC000
#define MCP3002_CH1 0xE000

#define SYS_STATE_INIT      0x01
#define SYS_STATE_STANDBY   0x10
#define SYS_STATE_PENDING   0x15
#define SYS_STATE_READY     0x20
#define SYS_STATE_TREATMENT 0x30
#define SYS_STATE_FAULT     0x0F
#define SYS_STATE_TECHMODE  0x55

#define FAULT_GUI_COMMUNICATION         21  //15
#define FAUL_COMM_TIMEOUT               22  //16
#define FAULT_INTERLOCK                 24 //18
#define FAULT_APPLICATOR_DISCONNECTED   26 //1A
#define FAULT_COOL_SYS_TEMP_SENSOR      27 //1B
#define FAULT_COOLING_SYSTEM            28 //1C
#define FAULT_COOLING_SYSTEM_OVERTEMPERATURE 30 //0x1E
#define FAULT_DIODE_CURRENT             32 //0x20
#define FAULT_TRIGGER_MALFUNCTION       34 //0x22
#define FAULT_APPLICATOR_ID             35 //0x23
#define FAULT_LIGHTGUIDE_DISCONNECTED   37 //0x25
#define FAULT_LIGHTGUIDE_ID             38 //0x26
#define FAULT_NO_ID                     39 //0x27
#define FAULT_TRIGGER_NOT_RELEASED      40 //0x28
#define FAULT_CAPACITOR_CHARGER         41 //0x29
#define FAULT_PUMP_DRIVER               42 //0x2A
#define FAULT_UNKNOWN_LIGHTGUIDE_TYPE   43 //0x2B
#define FAULT_APPLICATION_CORRUPTED     44 //0x2C
#define FAULT_LIGHTGUIDE_TEMPERATURE    45 //0x2D
#define FAULT_LIGHTGUIDE_SENSOR         46 //0x2E

#define DIODE_VOLTAGE            1
#define CAP_MONITOR           2

#define Reset() {__asm__ volatile ("reset");}

#define FLOW_SWITCH     PORTAbits.RA0
#define INTERLOCK       PORTAbits.RA1
#define BUZZ            LATAbits.LATA2
#define pLDAC           LATAbits.LATA3
#define tLDAC           LATAbits.LATA4
#define DIR             LATAbits.LATA5
#define DISCHARGE       LATAbits.LATA7
#define LED6            LATAbits.LATA9
#define LED7            LATAbits.LATA10
//#define LDAC            LATAbits.LATA14
#define TEC_ENABLE      LATAbits.LATA15

#define LED2            LATBbits.LATB12
#define LED3            LATBbits.LATB13
#define LED4            LATBbits.LATB14
#define LED5            LATBbits.LATB15

#define PWM_SAFE_IGBT  IOCON1bits.PENL
#define LD_EN_PWM      IOCON1bits.PENH

#define BT_AUTO_PAIRING LATCbits.LATC3
#define CS_ADC_PUMP     LATCbits.LATC13
//#define PUMP_SPI_EN     LATCbits.LATC14

#define pCS_DAC         LATDbits.LATD0
#define TRIGGER2        PORTDbits.RD1
#define TRIGGER1        PORTDbits.RD2
#define LD_INHIBIT1     LATDbits.LATD4
#define CS_TEC_DAC      LATDbits.LATD8
#define CS_ADC_TEC      LATDbits.LATD9
#define TEC_SPI_EN      LATDbits.LATD10
#define PUMP_SPI_EN     LATDbits.LATD11
#define CS_LD_DAC       LATDbits.LATD13
#define LED0            LATDbits.LATD14
#define LED1            LATDbits.LATD15

#define PULSE_PWM        LATEbits.LATE1
#define FAN_PWM         LATEbits.LATE3
#define LIGHTGUIDE_SHORT PORTEbits.RE7

#define BT_MASTER_RESET LATFbits.LATF1

#define BT_AUTO_MASTER  LATGbits.LATG0
#define BT_BAUD_RATE    LATGbits.LATG1
#define LD_SPI_ENABLE   LATGbits.LATG2
#define CS_DAC          LATGbits.LATG3
#define APP_SHORT       PORTGbits.RG6
#define CS_ADC_LD       LATGbits.LATG8
#define CHARGER_ENABLE  LATGbits.LATG9
#define SAFE_IGBT_EN    LATGbits.LATG15

#define OW_WRITE_PIN        TRISGbits.TRISG7
#define OW_PIN_DIRECTION     LATGbits.LATG7
#define OW_READ_PIN         PORTGbits.RG7

#define FAN_PWM_FREQ    31000
#define HPDL_ADDRESS     0x90

#define LONG_PULSE_CURRENT      50
#define SHORT_PULSE_CURRENT     120
#

typedef struct PULSE_PARAMETER_TAG {
    u16 PULSE_E;
    u16 PULSE_T;
    u16 PULSE_V;
} PULSE_PARAMETER;

#ifdef _user

/******************************************************************************/
/* User Structers                                                             */

/******************************************************************************/
struct strCommFlags {
    u16 FrameReceivedFlag : 1;
    u16 ReadyToSendFlag : 1;
    u16 SPI1RxDone : 1;
    u16 TipAccepted : 1;
} CommFlags = {0, 0, 1, 0};

struct sDevices {
    u16 TecIsEnabled : 1;
    u16 FanIsEnabled : 1;
    u16 PumpIsEnabled : 1;
    u16 PumpIsReady : 1;
    u16 PumpVoltageUpdate : 1;
    u16 WaterTemperatureValueIsReady : 1;
    u16 TecTemperatureValueIsReady : 1;
    u16 PulseInProgress : 1;
    u16 SystemStatedUpdate : 1;
    u16 ApplicatorIsConnected : 1;
    u16 InterlockIsConnected : 1;
    u16 ChargerIsEnabled : 1;
    u16 ChargerIsReady : 1;
    u16 DriverCurrentUpdate : 1;
    u16 LddriverIsEnable : 1;
    u16 DischargeIsEnabled : 1;
} Devices = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

struct sDevices2 {
    u16 TecLevelUpdate : 1;
    u16 TecVoltageUpdateByVoltage : 1;
    u16 AutoRepeat : 1;
    u16 InfoDataReadyToSend : 1;
    u16 LighGuideIsConnected : 1;
    u16 ChipIdIsConnected1 : 1;
    u16 ChipIdIsConnected2 : 1;
    u16 DiodeCurrentMeasurementDone : 1;
    u16 WrongCurrentDetected : 1;
    u16 LightGuideReconnected : 1;
    u16 LightGuideIsValid : 1;
    u16 ApplicatorDataIsValid : 1;
    u16 PulseDone : 1;
    u16 SlideModeSelected : 1;
    //u16 IsLongPulse : 1; //if  0 then short pulse if 1 then long pulse
} Devices2 = {0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
/******************************************************************************/

/******************************************************************************/
/* User Variables                                                             */
/******************************************************************************/
u16 FaultNumber = 0;
u16 SystemState = SYS_STATE_INIT;
u16 Pulse_On_Time = 6250; //10ms default
u16 Pulse_Off_Time = 19900;
u16 Birst_On_Time = 0;
u16 Birst_Off_Time = 0;
unsigned long long PulseCounter = 0;
u32 LgPulseCounter = 0;
u16 LgTypeId = 0;
u16 DiodeVoltage = 0;
u32 CapVoltage = 0;
u16 FlashCheckSum = 0;
u16 StoredCheckSum = 0;
u16 SlideModePulsePerSquare = 0;
/******************************************************************************/

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/
void InitApp(void); /* I/O and Peripheral Initialization */
u16 SPIReadWriteWord(u16 WordVar);
void FaultStateDeclare(u16 FaultValue);
void FaultStateNonFatalErrorsReset(void);
u8 MemorySelfTest(void);
/******************************************************************************/

#else

extern PULSE_PARAMETER EenerjyTable[42];
/******************************************************************************/
/* User Structers                                                             */

/******************************************************************************/
extern struct strCommFlags {
    u16 FrameReceivedFlag : 1;
    u16 ReadyToSendFlag : 1;
    u16 SPI1RxDone : 1;
    u16 TipAccepted : 1;
} CommFlags;

extern struct sDevices {
    u16 TecIsEnabled : 1;
    u16 FanIsEnabled : 1;
    u16 PumpIsEnabled : 1;
    u16 PumpIsReady : 1;
    u16 PumpVoltageUpdate : 1;
    u16 WaterTemperatureValueIsReady : 1;
    u16 TecTemperatureValueIsReady : 1;
    u16 PulseInProgress : 1;
    u16 SystemStatedUpdate : 1;
    u16 ApplicatorIsConnected : 1;
    u16 InterlockIsConnected : 1;
    u16 ChargerIsEnabled : 1;
    u16 ChargerIsReady : 1;
    u16 DriverCurrentUpdate : 1;
    u16 LddriverIsEnable : 1;
    u16 DischargeIsEnabled : 1;
} Devices;

extern struct sDevices2 {
    u16 TecLevelUpdate : 1;
    u16 TecVoltageUpdateByVoltage : 1;
    u16 AutoRepeat : 1;
    u16 InfoDataReadyToSend : 1;
    u16 LighGuideIsConnected : 1;
    u16 ChipIdIsConnected1 : 1;
    u16 ChipIdIsConnected2 : 1;
    u16 DiodeCurrentMeasurementDone : 1;
    u16 WrongCurrentDetected : 1;
    u16 LightGuideReconnected : 1;
    u16 LightGuideIsValid : 1;
    u16 ApplicatorDataIsValid : 1;
    u16 PulseDone : 1;
    u16 SlideModeSelected : 1;
} Devices2;

/******************************************************************************/

/******************************************************************************/
/* User Variables                                                             */
/******************************************************************************/
extern u16 FaultNumber;
extern u16 SystemState;
extern u16 SystemMode;
extern u16 Pulse_On_Time; //= ((Xms-1) * 10000)/16
extern u16 Pulse_Off_Time;
extern u16 Birst_On_Time;
extern u16 Birst_Off_Time;
extern unsigned long long PulseCounter;
extern u32 LgPulseCounter;
extern u16 LgTypeId;
extern u16 DiodeVoltage;
extern u32 CapVoltage;
extern u16 FlashCheckSum;
extern u16 StoredCheckSum;
extern u16 SlideModePulsePerSquare;
/******************************************************************************/

/******************************************************************************/
/* User Function Prototypes                                                   */
/******************************************************************************/
extern void InitApp(void); /* I/O and Peripheral Initialization */
extern u16 SPIReadWriteWord(u16 WordVar);
extern void FaultStateDeclare(u16 FaultValue);
extern void FaultStateNonFatalErrorsReset(void);
extern u8 MemorySelfTest(void);
/******************************************************************************/

#endif
