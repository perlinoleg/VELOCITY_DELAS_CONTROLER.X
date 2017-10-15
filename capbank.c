#if defined(__XC16__)
#include <xc.h>
#elif defined(__C30__)
#if defined(__dsPIC33E__)
#include <p33Exxxx.h>
#elif defined(__dsPIC33F__)
#include <p33Fxxxx.h>
#endif
#endif

#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */
#include <dsp.h>

/*================= Macros ===============================*/
#define CAPBANK_INIT_TASK       0
#define CAPBANK_WAIT_TASK       1
#define CAPBANK_READY_TASK      2
#define CAPBANK_DISCHARGE_TASK  3
#define CAPBANK_CHARGE_TASK     4
#define CAPBANK_FAULT_TASK      5
/*================================================*/

/*================= Variables ===============================*/
u16 current_capacitors_bank_task = 0;
u32 ChargerTimeCntr = 0;
u16 CalculatedChargeVoltage = 0;
u16 ActualCapacitorVoltage = 0;
u16 MaxChargeLimit = 0;
u16 MinChargeLimit = 0;
u16 OnTimeForChargeCalculation = 0;
u16 DEBUG_MEMORY_CapVoltage = 0;
u16 FlickerTimer = 0;

extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
void SetChargeVoltage(u16 val);
void capacitors_bank_tasks(void);
/*================================================*/

/**********************************************************************
 * Function:        void SetChargeVoltage(u16 val)
 * PreCondition:    None
 * Input:	    val - charge voltage value
 * Output:	    None
 * Overview:	    This function used to calculate charge voltage and send to SPI procedure
 *
 ***********************************************************************/
void SetChargeVoltage(u16 val) {
    val = val * 3.4;
    val <<= 2;
    val = 0x1000 + val;
    while (!CommFlags.SPI1RxDone);
    CS_DAC = LOW;
    SPIReadWriteWord(val);
    CS_DAC = HIGH;
}

/**********************************************************************
 * Function:        void capacitors_bank_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function used to manage charger and capacitors bank
 *
 ***********************************************************************/
void capacitors_bank_tasks(void) {

//#ifdef bluebox
//    if (Devices.ChargerIsEnabled == TRUE) {
//        Devices.DischargeIsEnabled = FALSE;
//        DISCHARGE = HIGH; //stop discharge before charger enable
//        CHARGER_ENABLE = LOW; //star charge capacitors
//        ChargerTimeCntr = 0; //reset counter for charge time out
//        current_capacitors_bank_task = CAPBANK_CHARGE_TASK;
//    } else {
//        CHARGER_ENABLE = HIGH; //disable charger
//        Devices.ChargerIsReady = FALSE;
//    }
//#endif

    switch (current_capacitors_bank_task) {
        case CAPBANK_INIT_TASK:
            if (Devices.ChargerIsEnabled == TRUE) {
                Devices.DischargeIsEnabled = FALSE;
                DISCHARGE = HIGH; //stop discharge before charger enable
                CHARGER_ENABLE = LOW; //star charge capacitors
                ChargerTimeCntr = 0; //reset counter for charge time out
                current_capacitors_bank_task = CAPBANK_CHARGE_TASK;
            } else {
                CHARGER_ENABLE = HIGH; //disable charger
                Devices.ChargerIsReady = FALSE;
            }
            break;

        case CAPBANK_CHARGE_TASK:
#ifdef maincfg_debug
            CapVoltage = 290;
#endif        
      if (Devices.ChargerIsEnabled == TRUE) {
                if (CapVoltage > 250) {
                    Devices.ChargerIsReady = TRUE;
                    current_capacitors_bank_task = CAPBANK_READY_TASK;
                    FlickerTimer = 0;
                } else if (ChargerTimeCntr > 5000) {
                    FaultNumber = FAULT_CAPACITOR_CHARGER; //time out occured 
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    current_capacitors_bank_task = CAPBANK_FAULT_TASK;
                }
            } else {
                CHARGER_ENABLE = HIGH; //disable charger
                Devices.ChargerIsReady = FALSE;
                current_capacitors_bank_task = CAPBANK_INIT_TASK;
            }

            break;

        case CAPBANK_READY_TASK:
#ifdef maincfg_debug
            CapVoltage = 290;
#endif 
#ifdef bluebox
            CapVoltage = 290;
#endif 
            if (Devices.ChargerIsEnabled == FALSE) {
                CHARGER_ENABLE = HIGH; //disable charger
                Devices.ChargerIsReady = FALSE;
                current_capacitors_bank_task = CAPBANK_INIT_TASK;
            } else if (CapVoltage < 230) {
                DEBUG_MEMORY_CapVoltage = CapVoltage;
                FaultNumber = FAULT_CAPACITOR_CHARGER; //time out occured 
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                current_capacitors_bank_task = CAPBANK_FAULT_TASK;
            } else if (CHARGER_ENABLE == LOW) {
                //if (FlickerTimer > 5) {
                if (FlickerTimer > 25) {//probably bad flicker
                    CHARGER_ENABLE ^= 1;
                    FlickerTimer = 0;
                }
            } else if (CHARGER_ENABLE == HIGH) {
                if (FlickerTimer > 8) {
                    CHARGER_ENABLE ^= 1;
                    FlickerTimer = 0;
                }
            }
            break;

        case CAPBANK_FAULT_TASK:

            break;

    }
}
