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
#define DIODE_MODE_TASK_WAIT        0
#define DIODE_MODE_TASK_SELECTED    1
#define DIODE_MODE_TASK_ACTIVATED   2
#define DIODE_MODE_TASK_READY       3
#define DIODE_MODE_TASK_STANDBY     4

#define PULSE_TASK_WAIT_FOR_START_PULSE    7
#define PULSE_TASK_WAIT_FOR_END_PULSE      1
#define PULSE_TASK_TEST_IGBT_AFTER_PULSE   2
#define PULSE_TASK_WAIT_OFF_TIME           3
#define PULSE_TASK_WAIT_TRIGGER_RELEASE    4
#define PULSE_TASK_WAIT_FOR_READY          5
#define PULSE_TASK_FAULT                   6 
#define PULSE_TASK_SLIDE_MODE_BUZZER       8 
/*================================================*/

/*================= Variables ===============================*/
u16 current_pulse_fire_task = 0;
u16 current_diode_mode_task = 0;
u16 offTimeCntr = 0;
u16 SystemStateToUpdate = SYS_STATE_INIT;
u16 MeasurePeriod = 1000;
u16 SPIMeasure = 0;
u16 DiodeCurrent1[5];
u16 DiodeCurrent2[5];
u16 MeasuredDiodeVoltage[5];
u16 ActualDiodeCurrent1 = 0;
u16 ActualDiodeCurrent2 = 0;
u16 ActualLddriverCurrent = 0;
u16 MaxAllowedCurrent = 0;
u16 MinAllowedCurrent = 0;
u16 CapacitorAvarageRawValue = 0;

extern u16 PumpVoltage;
extern u16 cntSPIMeasure;
extern u16 Trigger;
extern u16 Pulse_Off_Time;
extern float TecTreatmentTemperature;
extern tPID fooPID;
extern double Setpoint;
extern u16 LddriverCurrent;
extern u16 IndxDiodeCurrent;
extern u16 CoolingLevel;
extern float ActualTecTemperature;
extern u16 SlideModePulseCounter;
extern u16 cnt100uS;
/*================================================*/

/*================= Functions ===============================*/
void system_state_tasks(void);
void SystemCheck(void);
void pulse_fire_tasks(void);
void diode_mode_tasks(void);
void ShutDownIpl(void);
void SystemToStndby(void);
void PowerDeliverySystemOff(void);
void measurments_tasks(void);

extern void pump_tasks(void);
extern void simmer_tasks(void);
extern void capacitors_bank_tasks(void);
extern void app_short_tasks(void);
extern void lihgtguide_short_task(void);
extern void trigger_tasks(void);
extern void fan_tasks(void);
extern void interlock_tasks(void);
extern void fan_tasks(void);
extern void tip_tasks(void);
extern void tec_tasks(void);
extern void discharge_tasks(void);
extern void LDDriver_tasks(void);
extern void SetDriverCurrent(u16 val);
extern unsigned int Spi1RxBuff[8] __attribute__((space(dma)));
extern unsigned int Spi1TxBuff[8] __attribute__((space(dma)));

/*================================================*/

void SystemCheck(void) {
    system_state_tasks();
    app_short_tasks();
    lihgtguide_short_task();
    trigger_tasks();
    tip_tasks();
    fan_tasks();
    pump_tasks();
    diode_mode_tasks();
    pulse_fire_tasks();
    tec_tasks();
    capacitors_bank_tasks();
    LDDriver_tasks();
    interlock_tasks();
    discharge_tasks();
    measurments_tasks();
}

void system_state_tasks(void) {

    if (Devices.SystemStatedUpdate) {
        switch (SystemStateToUpdate) {
            case SYS_STATE_READY:
                if (SystemState == SYS_STATE_INIT)SystemStateToUpdate = SystemState; //prevent from system to enter in ready from init
#ifdef bluebox
                Devices.PumpVoltageUpdate = TRUE;
                PumpVoltage = 920;
                Devices.ChargerIsEnabled = TRUE; // enable charger
                Devices.LddriverIsEnable = TRUE; //enable driver
#endif          
#ifndef release
                //while (1);//TODO for validation
#endif
                break;

            case SYS_STATE_STANDBY:
                //LD_INHIBIT1 = HIGH;
                Devices.PumpIsEnabled = TRUE;
                Setpoint = 15.0;
                TEC_ENABLE = TRUE;
                Devices.TecIsEnabled = TRUE;
                Devices.PumpVoltageUpdate = TRUE;
                PumpVoltage = 500;
                Devices.LddriverIsEnable = FALSE;
                Devices.ChargerIsEnabled = FALSE; // enable charger
                break;

            case SYS_STATE_PENDING:
#ifdef bluebox
                Devices2.LighGuideIsConnected = TRUE;
#endif
                if (Devices2.LighGuideIsConnected) {
                    Devices.PumpVoltageUpdate = TRUE;
                    PumpVoltage = 920;
                    if (CoolingLevel == 25)Setpoint = 7;
                    else if (CoolingLevel == 50)Setpoint = 5;
                    else if (CoolingLevel == 75)Setpoint = 3;
                    else if (CoolingLevel == 100)Setpoint = 1; //set tec temperature
                    Devices.ChargerIsEnabled = TRUE; // enable charger
                    Devices.LddriverIsEnable = TRUE; //enable driver
                    SlideModePulseCounter = 0;
                } else {
                    SystemStateToUpdate = SystemState;
                    Devices2.LighGuideIsConnected = FALSE; //applicator disconnected
                    Devices2.LightGuideIsValid = FALSE;
                }
                break;

            case SYS_STATE_INIT:
                PowerDeliverySystemOff();
                Devices.PumpIsEnabled = FALSE;
                Devices.PumpIsReady = FALSE;
                break;

            case SYS_STATE_TECHMODE:
                PowerDeliverySystemOff();
                Devices.PumpIsEnabled = FALSE;
                Devices.PumpIsReady = FALSE;
                break;

            case SYS_STATE_FAULT:
                PowerDeliverySystemOff();
                if (SystemState != SystemStateToUpdate) {
                    switch (FaultNumber) {
                        case FAULT_COOLING_SYSTEM:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_DIODE_CURRENT:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_APPLICATOR_DISCONNECTED:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_APPLICATOR_ID:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_COOL_SYS_TEMP_SENSOR:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_COOLING_SYSTEM_OVERTEMPERATURE:

                            break;

                        case FAULT_LIGHTGUIDE_DISCONNECTED:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_NO_ID:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_CAPACITOR_CHARGER:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_PUMP_DRIVER:
                            Devices.PumpIsEnabled = FALSE;
                            Devices.PumpIsReady = FALSE;
                            break;

                        case FAULT_INTERLOCK:
                            Setpoint = 15.0;
                            Devices.PumpVoltageUpdate = TRUE;
                            PumpVoltage = 500;
                            break;

                        case FAULT_UNKNOWN_LIGHTGUIDE_TYPE:

                            break;

                        case FAULT_LIGHTGUIDE_ID:

                            break;

                        case FAULT_LIGHTGUIDE_TEMPERATURE:
                            Setpoint = 15.0;
                            Devices.PumpVoltageUpdate = TRUE;
                            PumpVoltage = 500;
                            break;

                        case FAULT_GUI_COMMUNICATION:

                            break;

                        case FAUL_COMM_TIMEOUT:

                            break;
                    }
                }
                break;
        }

        SystemState = SystemStateToUpdate;
        Devices.SystemStatedUpdate = FALSE;
    }
}

/**********************************************************************
 * Function:        void PowerDeliverySystemOff(void) 
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function cut off all power delivery system
 *
 ***********************************************************************/
void PowerDeliverySystemOff(void) {
    SAFE_IGBT_EN = LOW;
    LD_INHIBIT1 = 1;
    LD_EN_PWM = 0;
    T2CONbits.TON = 0;
    T4CONbits.TON = 0;
    CHARGER_ENABLE = HIGH; //disable charger 
    //DISCHARGE = LOW; //start discharge
    Devices.ChargerIsReady = FALSE;
    Devices.ChargerIsEnabled = FALSE;
    Devices.LddriverIsEnable = FALSE;
    TEC_ENABLE = CLEAR;
    Devices.TecIsEnabled = FALSE;
}

/**********************************************************************
 * Function:        void diode_mode_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This manage the system after diode is selescted
 *
 ***********************************************************************/
void diode_mode_tasks(void) {
    switch (current_diode_mode_task) {
        case DIODE_MODE_TASK_WAIT:
            if (SystemState == SYS_STATE_STANDBY) {
                current_diode_mode_task = DIODE_MODE_TASK_SELECTED;
            } else if (SystemState == SYS_STATE_PENDING) {

            }
            break;

        case DIODE_MODE_TASK_SELECTED:
            if (SystemState == SYS_STATE_PENDING) {
                current_diode_mode_task = DIODE_MODE_TASK_ACTIVATED;
            }
            break;

        case DIODE_MODE_TASK_ACTIVATED:
            if ((Devices.PumpIsReady == TRUE)&&(Devices.ChargerIsReady == TRUE)) {
                current_diode_mode_task = DIODE_MODE_TASK_READY;
            } else if (SystemState != SYS_STATE_PENDING) {
                current_diode_mode_task = DIODE_MODE_TASK_WAIT;
            }
            break;

        case DIODE_MODE_TASK_READY:
#ifdef bluebox
            ActualTecTemperature = 5;
#endif
            if (ActualTecTemperature < 13.0) {
                SystemStateToUpdate = SYS_STATE_READY;
                Devices.SystemStatedUpdate = TRUE;
                current_diode_mode_task = DIODE_MODE_TASK_STANDBY;
            } else {
                if (SystemState != SYS_STATE_PENDING) {
                    current_diode_mode_task = DIODE_MODE_TASK_WAIT;
                }
            }
            break;

        case DIODE_MODE_TASK_STANDBY: // wait to end of ready 
            if (SystemState != SYS_STATE_READY) {
                current_diode_mode_task = DIODE_MODE_TASK_WAIT;
            }
            break;
    }
}

/**********************************************************************
 * Function:        void pulse_fire_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This manage the pulse fire
 *
 ***********************************************************************/
void pulse_fire_tasks(void) {
    if (SystemState == SYS_STATE_READY) {//check if system ready
        switch (current_pulse_fire_task) {
            case PULSE_TASK_WAIT_FOR_START_PULSE:
                if (Trigger) {//check if triger pushed and pulse enableed
                    Devices.PulseInProgress = 1; // 
                    Devices2.PulseDone = 0;
                    BUZZ = HIGH;
                    LD_EN_PWM = ENABLE;
                    LED2 = HIGH;
                    PR2 = Pulse_On_Time; // set pulse time
                    TMR2 = 0; //reset timer
                    if (Birst_On_Time) {
                        PR3 = Birst_On_Time; // set pulse time
                        TMR3 = 0; //reset timer
                    }
                    IFS0bits.T2IF = CLEAR; /* reset Timer2 interrupt flag */
                    T2CONbits.TON = 1; // Enable Timer2 and start the counter
                    IFS0bits.T3IF = CLEAR; /* reset Timer2 interrupt flag */
                    T3CONbits.TON = 1; // Enable Timer2 and start the counter
                    TMR4 = 0; //reset timer (timer 4 used for A/D pulse current measurements)
                    IFS1bits.T4IF = CLEAR; /* reset Timer4 interrupt flag */
                    T4CONbits.TON = 1; // Enable Timer4 and start the counter
                    current_pulse_fire_task = PULSE_TASK_WAIT_FOR_END_PULSE;
                } else {
                    //LD_INHIBIT1 = HIGH;
                    LD_EN_PWM = 0;
                    PULSE_PWM = 0;
                    BUZZ = LOW;
                }
                break;

            case PULSE_TASK_WAIT_FOR_END_PULSE:
                if (Devices2.PulseDone) {
                    BUZZ = LOW;
                    current_pulse_fire_task = PULSE_TASK_WAIT_OFF_TIME;
                    //LD_INHIBIT1 = LOW;
                } else {
                    if (Devices2.WrongCurrentDetected) {
                        FaultNumber = FAULT_DIODE_CURRENT; //time out occured 
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        current_pulse_fire_task = PULSE_TASK_FAULT;
                        BUZZ = LOW;
                    }
                }
                break;

            case PULSE_TASK_WAIT_OFF_TIME:
                BUZZ = LOW;
                if (offTimeCntr >= Pulse_Off_Time) {
                    if (Devices2.AutoRepeat) {
                        if (Devices2.SlideModeSelected == TRUE) {
                            if (SlideModePulseCounter >= SlideModePulsePerSquare) {
                                current_pulse_fire_task = PULSE_TASK_SLIDE_MODE_BUZZER;
                                SlideModePulseCounter = 0;
                                cnt100uS = 0;
                                BUZZ = HIGH; // sound buzzer for end of square
                            } else {
                                current_pulse_fire_task = PULSE_TASK_WAIT_FOR_START_PULSE;
                            }
                        } else {
                            current_pulse_fire_task = PULSE_TASK_WAIT_FOR_START_PULSE;
                        }
                    } else current_pulse_fire_task = PULSE_TASK_WAIT_TRIGGER_RELEASE;
                }
                break;

            case PULSE_TASK_SLIDE_MODE_BUZZER:
                if (cnt100uS > 1200) {
                    BUZZ = LOW;
                    current_pulse_fire_task = PULSE_TASK_WAIT_TRIGGER_RELEASE;
                }
                break;

            case PULSE_TASK_WAIT_TRIGGER_RELEASE:
                if (!Trigger)current_pulse_fire_task = PULSE_TASK_WAIT_FOR_START_PULSE;
#ifdef bluebox
                if (SystemState == SYS_STATE_STANDBY)current_pulse_fire_task = PULSE_TASK_WAIT_FOR_START_PULSE;
#endif
                break;
        }
    } else {
        BUZZ = LOW;
        Devices.PulseInProgress = 0;
        current_pulse_fire_task = PULSE_TASK_WAIT_FOR_START_PULSE;
    }
}

/**********************************************************************
 * Function:        void measurments_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function used to measure all spi a/d sensors
 *
 ***********************************************************************/
void measurments_tasks(void) {
    u16 i;
    u16 Val = 0;

    if (CommFlags.SPI1RxDone) {
        switch (SPIMeasure) {
            case 0:
                DMA0CONbits.CHEN = 1; // Enable DMA Channel
                DMA1CONbits.CHEN = 1; // Enable DMA Channel
                LD_SPI_ENABLE = HIGH;
                CS_ADC_LD = LOW;
                DMA0REQbits.FORCE = 1;
                CommFlags.SPI1RxDone = 0;
                IEC0bits.SPI1IE = 1;
                break;

            case DIODE_VOLTAGE:
                if (Devices.PulseInProgress) {
                    for (i = 0; i < 8; i++) {
                        Spi1RxBuff[i] >>= 1;
                        Val = Val + Spi1RxBuff[i];
                    }
                    DiodeVoltage = Val / 8;
                    //DiodeVoltage = (u32) Val * 49 * 798 / 1000; //voltage in mV
                }

                for (i = 0; i < 8; i++)Spi1TxBuff[i] = MCP3002_CH1;
                IEC0bits.SPI1IE = 1;
                DMA0CONbits.CHEN = 1; // Enable DMA Channel
                DMA1CONbits.CHEN = 1; // Enable DMA Channel
                LD_SPI_ENABLE = HIGH;
                CS_ADC_LD = LOW;
                DMA0REQbits.FORCE = 1;
                CommFlags.SPI1RxDone = 0;
                break;

            case CAP_MONITOR:
                if (!Devices.PulseInProgress) {
                    for (i = 0; i < 8; i++) {
                        Spi1RxBuff[i] >>= 1;
                        if ((Spi1RxBuff[i] > 1000) || (Spi1RxBuff[i] == 0)) {
                            Val = Val + CapacitorAvarageRawValue;
                        } else Val = Val + Spi1RxBuff[i];
                    }
                    Val = Val / 8;
                    CapacitorAvarageRawValue = Val;
                    CapVoltage = Val * 48; //48mV = 5V / 1024bit
                    //CapVoltage = CapVoltage * 1515/150000;// Vcap = (val * 151.5k)/1.5k
                    CapVoltage = CapVoltage * 1515 / 150000; //for debug without capacitor bank
                }
                for (i = 0; i < 8; i++)Spi1TxBuff[i] = MCP3002_CH0;
                cntSPIMeasure = 0;
                break;

            default:
                if (cntSPIMeasure > MeasurePeriod)SPIMeasure = 0xFFFF;
                else SPIMeasure = 7;
                break;
        }
        SPIMeasure++;
        //if (SPIMeasure > 6)SPIMeasure = 0;
    }

    if (Devices2.DiodeCurrentMeasurementDone) {
        Devices2.DiodeCurrentMeasurementDone = 0; //reset until next measurements
        IndxDiodeCurrent = 0;
        ActualDiodeCurrent1 = 0;
        for (i = 0; i < 5; i++) {
            ActualDiodeCurrent1 = ActualDiodeCurrent1 + DiodeCurrent1[i];
        }
        ActualDiodeCurrent1 = ActualDiodeCurrent1 / 5;
        ActualDiodeCurrent1 = 512 - ActualDiodeCurrent1;
        ActualDiodeCurrent1 = ActualDiodeCurrent1 * 322 / 660;

        ActualDiodeCurrent2 = 0;
        for (i = 0; i < 5; i++) {
            ActualDiodeCurrent2 = ActualDiodeCurrent2 + DiodeCurrent2[i];
        }
        ActualDiodeCurrent2 = ActualDiodeCurrent2 / 5;
        ActualDiodeCurrent2 = ActualDiodeCurrent2 - 510;
        ActualDiodeCurrent2 = ActualDiodeCurrent2 * 322 / 660;

        ActualLddriverCurrent = ActualDiodeCurrent1 + ActualDiodeCurrent2;
#ifndef release       
        //        ActualLddriverCurrent = 25; //TODO for validation
#endif
#ifndef maincfg_debug       
        if ((ActualLddriverCurrent > MaxAllowedCurrent) || (ActualLddriverCurrent < MinAllowedCurrent)) {
            if ((ActualLddriverCurrent < 160)&&(Trigger == TRUE)&&(Devices.PulseInProgress == 1)) Devices2.WrongCurrentDetected = TRUE;
        } else Devices2.WrongCurrentDetected = FALSE; //debug
#endif
    }
}

