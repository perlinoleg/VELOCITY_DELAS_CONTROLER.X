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
#define WAIT_INTERLOCK          0
#define WAIT_INTERLOCK_BOUNCING 1
#define INTERLOCK_FAULT         2

#define DISCHARGE_WAIT_ENABLE   0
#define DISCHARGE_WAIT_DISABLE  1
/*================================================*/

/*================= Variables ===============================*/
u16 current_interlock_task = 0;
u16 InterlockTimeOut = 0;
u16 current_discharge_task = 0;
u16 LddriverCurrent = 0;
u16 current_lddriver_task = 0;

extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
void interlock_tasks(void);
void discharge_tasks(void);
void LDDriver_tasks(void);
void SetDriverCurrent(u16 val);

/*================================================*/

void LDDriver_tasks(void) {
    if (Devices.DriverCurrentUpdate) {
        SetDriverCurrent(LddriverCurrent);
        Devices.DriverCurrentUpdate = FALSE;
    }

    if (Devices.LddriverIsEnable) {
        LD_INHIBIT1 = LOW;
        SAFE_IGBT_EN = HIGH;
    } else {
        LD_INHIBIT1 = HIGH;
        SAFE_IGBT_EN = LOW;
    }

    switch (current_lddriver_task) {

    }
}

void SetDriverCurrent(u16 val) {
    val = val * 334 / 49;
    val <<= 2;
    val = 0x1000 + val;
    while (!CommFlags.SPI1RxDone);
    LD_SPI_ENABLE = HIGH;
    CS_LD_DAC = LOW;
    SPIReadWriteWord(val);
    CS_LD_DAC = HIGH;
    LD_SPI_ENABLE = LOW;
}

void discharge_tasks(void) {
    switch (current_discharge_task) {
        case DISCHARGE_WAIT_ENABLE:
            if (Devices.DischargeIsEnabled) {
                CHARGER_ENABLE = HIGH; //inhibit charger before discharge
                DISCHARGE = LOW;
                current_discharge_task = DISCHARGE_WAIT_DISABLE;
            }
            break;

        case DISCHARGE_WAIT_DISABLE:
            if (!Devices.DischargeIsEnabled) {
                DISCHARGE = HIGH;
                current_discharge_task = DISCHARGE_WAIT_ENABLE;
            }
            break;
    }
}

void interlock_tasks(void) {
    switch (current_interlock_task) {
        case WAIT_INTERLOCK:
            if (INTERLOCK) {
                InterlockTimeOut = 0; //reset timer
                current_interlock_task = WAIT_INTERLOCK_BOUNCING;
            } else {
                Devices.InterlockIsConnected = TRUE;
            }
            break;

        case WAIT_INTERLOCK_BOUNCING:
            if (InterlockTimeOut > 30) { //delay 15ms for bouncing
                if (INTERLOCK) {//if ipl applicator disconnected or not connected shut off ipl module
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_INTERLOCK;
                    Devices.InterlockIsConnected = FALSE;
                    current_interlock_task = INTERLOCK_FAULT;
                } else {
                    current_interlock_task = WAIT_INTERLOCK;
                }
            }
            if (!INTERLOCK)current_interlock_task = WAIT_INTERLOCK;
            break;

        case INTERLOCK_FAULT:
            if (!INTERLOCK) {
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_STANDBY;
                FaultNumber = CLEAR;
                InterlockTimeOut = 0; //reset timer
                current_interlock_task = WAIT_INTERLOCK_BOUNCING;
            } else {
                if (FaultNumber == 0) {
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_INTERLOCK;
                    Devices.InterlockIsConnected = FALSE;
                    current_interlock_task = INTERLOCK_FAULT;
                }
            }
            break;
    }
}