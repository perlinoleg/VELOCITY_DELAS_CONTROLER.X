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
#define PUMP_STARTUP            0//
#define PUMP_ON_TASK            1
#define WAIT_FOR_FLOW           2
#define CHECK_FLOW              3//
#define WAIT_FLOW_BOUNCING      4//
#define PUMP_OFF_TASK           5//
#define PUMP_SYSTEM_FAULT       6//
#define WAIT_FOR_STARTUP        7//
#define INIT_PUMP_TASK          8//
#define PUMP_TECHMODE           9
#define PUMP_WAIT_TECHMODE      10

#define PUMP_V_SENS_TIME        200//sens pump voltage every 2sec
/*================================================*/

/*================= Variables ===============================*/
u16 PumpVoltage = 0;
u16 current_pump_task = INIT_PUMP_TASK;
u16 PumpTimer = 0;
u16 PumpVoltageMonitor = 818;
u16 SpiTimeout = 0;

extern u16 PumpVoltageSensingTimer;
extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
void SetPumpVoltage(u16 Voltage);
void pump_tasks(void);
void PumpFault(void);

extern u16 SPIReadWriteWord(u16 WordVar);

/*================================================*/

/**********************************************************************
 * Function:        void SetPumpVoltage(u16 Voltage)
 * PreCondition:    None
 * Input:	    Voltage - set volage for pump driver
 * Output:	    None
 * Overview:	    This function used to calculate and send to SPI module the voltage for pump driver
 *
 ***********************************************************************/
void SetPumpVoltage(u16 Voltage) {
    SpiTimeout = 0;
    Voltage <<= 2;
    Voltage = 0x1000 + Voltage;
    while ((!CommFlags.SPI1RxDone) || (SpiTimeout < 49));
    PUMP_SPI_EN = HIGH;
    pCS_DAC = LOW;
    if (PumpVoltage > 1023)PumpVoltage = 1023;
    SPIReadWriteWord(Voltage);
    pCS_DAC = HIGH;
    PUMP_SPI_EN = LOW;
}

/**********************************************************************
 * Function:        void SetPumpVoltage(u16 Voltage)
 * PreCondition:    None
 * Input:	    Voltage - set volage for pump driver
 * Output:	    None
 * Overview:	    This function used to calculate and send to SPI module the voltage for pump driver
 *
 ***********************************************************************/
void pump_tasks(void) {
    u16 val;
    
    if (SystemState == SYS_STATE_TECHMODE)current_pump_task = PUMP_WAIT_TECHMODE;

    if (Devices.PumpVoltageUpdate) {
        if (!Devices.PumpIsEnabled)PumpVoltage = 0;
        SetPumpVoltage(PumpVoltage);
        Devices.PumpVoltageUpdate = FALSE;
    }

    switch (current_pump_task) {
        case INIT_PUMP_TASK:
            PUMP_SPI_EN = HIGH;
            CS_ADC_PUMP = LOW;
            val = SPIReadWriteWord(MCP3002_CH1);
            CS_ADC_PUMP = HIGH;
            PUMP_SPI_EN = LOW;
            SetPumpVoltage(0);
            current_pump_task = WAIT_FOR_STARTUP;
            break;

        case WAIT_FOR_STARTUP:
            if (Devices.PumpIsEnabled) {
                SetPumpVoltage(940); //set pump max speed
                SetPumpVoltage(940); //set pump max speed
                PumpTimer = 0; //reset timer
                current_pump_task = PUMP_STARTUP;
            } else {
#ifndef maincfg_debug
                if (!FLOW_SWITCH)PumpFault();
#endif     
                #ifndef maincfg_debug
                if (!FLOW_SWITCH)PumpFault();
#endif 
            }
            break;

        case PUMP_STARTUP:
            if (!FLOW_SWITCH) {
                if (!Devices.PumpIsEnabled) {//check if need pump off
                    SetPumpVoltage(0);
                    Devices.PumpIsReady = FALSE;
                    PumpTimer = CLEAR;
                    current_pump_task = PUMP_OFF_TASK;
                } else {
                    SetPumpVoltage(580);
                    Devices.PumpIsReady = TRUE;
                    current_pump_task = CHECK_FLOW;
                }
            } else if (PumpTimer > 3000)current_pump_task = PUMP_SYSTEM_FAULT;
            break;

        case CHECK_FLOW:
            if (!Devices.PumpIsEnabled) {//check if need pump off
                SetPumpVoltage(0);
                Devices.PumpIsReady = FALSE;
                PumpTimer = CLEAR;
                current_pump_task = PUMP_OFF_TASK;
            } else {
                if (FLOW_SWITCH) {//no water
                    PumpTimer = 0; //reset timer
                    current_pump_task = WAIT_FLOW_BOUNCING; //check after bouncing 
                }
            }
            break;

        case WAIT_FLOW_BOUNCING:
            if (!FLOW_SWITCH) {
                if (!Devices.PumpIsEnabled) {//check if need pump off
                    SetPumpVoltage(0);
                    Devices.PumpIsReady = FALSE;
                    PumpTimer = CLEAR;
                    current_pump_task = PUMP_OFF_TASK;
                } else {
                    Devices.PumpIsReady = TRUE;
                    current_pump_task = CHECK_FLOW;
                }
            } else if (PumpTimer > 35)PumpFault();
            break;

        case PUMP_OFF_TASK:
            if (FLOW_SWITCH)current_pump_task = INIT_PUMP_TASK;
            else {
                if (PumpTimer > 1500)PumpFault();
                if (Devices.PumpIsEnabled) current_pump_task = INIT_PUMP_TASK;
            }
            break;

        case PUMP_SYSTEM_FAULT:
            if (SystemState != SYS_STATE_FAULT) {
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                FaultNumber = FAULT_COOLING_SYSTEM;
                SetPumpVoltage(0);
            }
            Devices.PumpIsReady = FALSE;
            Devices.PumpIsEnabled = FALSE;
            break;
            
        case PUMP_WAIT_TECHMODE:
            if (Devices.PumpIsEnabled) {
                SetPumpVoltage(940); //set pump max speed
                SetPumpVoltage(940); //set pump max speed
                current_pump_task = PUMP_TECHMODE;
            }
            break;
            
        case PUMP_TECHMODE:
            
            break;
    }

    if (PumpVoltageSensingTimer > PUMP_V_SENS_TIME) {
        PumpVoltageSensingTimer = 0;
        while (!CommFlags.SPI1RxDone);
        PUMP_SPI_EN = HIGH;
        CS_ADC_PUMP = LOW;
        val = SPIReadWriteWord(MCP3002_CH1);
        val >>= 1;
        PumpVoltageMonitor = (PumpVoltageMonitor + val) / 2; //v pump is 24vdc r divider 10k*24/(49.9k+10k)=4V
        CS_ADC_PUMP = HIGH;
        PUMP_SPI_EN = LOW;
        if (PumpVoltageMonitor < 700) {
            PumpFault();
            FaultNumber = FAULT_PUMP_DRIVER;
        }
    }

//#ifdef bluebox
//    if (Devices.PumpVoltageUpdate) {
//        SetPumpVoltage(PumpVoltage);
//        Devices.PumpVoltageUpdate = FALSE;
//    }
//#endif
}

void PumpFault(void) {
    Devices.SystemStatedUpdate = TRUE;
    SystemStateToUpdate = SYS_STATE_FAULT;
    FaultNumber = FAULT_COOLING_SYSTEM;
    SetPumpVoltage(0);
    Devices.PumpIsReady = FALSE;
    Devices.PumpIsEnabled = FALSE;
    PumpTimer = CLEAR;
    current_pump_task = PUMP_SYSTEM_FAULT;
}
