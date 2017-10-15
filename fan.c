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
#include "user.h"
#include <dsp.h>

/*================= Macros ===============================*/
#define FAN_INIT_TASK               0
#define FAN_WAIT_FOR_SPEED_UPDATE   1
#define FAN_SPEED_UPDATE            2
#define FAN_OVERTEMPERATURE         3
#define FAN_SYSTEM_FAULT            4
/*================================================*/

/*================= Variables ===============================*/
u16 current_fan_task = 0;
u16 WaterTemperature[5] = {250, 250, 250, 250, 250};
u16 indxWaterTemperature = 0;
u16 TemperatureAdSamplingCounter = 0;
float ActualWaterTemperature = 25.0;

extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
void fan_tasks(void);

/*================================================*/

void fan_tasks(void) {
    u16 i, min_val, max_val, indx_min, indx_max, devider;
    float tVal;

    if (TemperatureAdSamplingCounter > 1000) {
        TemperatureAdSamplingCounter = 0;
        ADCPC0bits.SWTRG0 = 1;
        //if (indxWaterTemperature < 5)ADCPC0bits.SWTRG0 = 1;
    }

    if (indxWaterTemperature == 5) {
        indxWaterTemperature = 0;
        ActualWaterTemperature = 0;

        min_val = WaterTemperature[0];
        max_val = WaterTemperature[0];
        indx_min = 0;
        indx_max = 0;
        for (i = 0; i < 5; i++) {
            if (WaterTemperature[i] > max_val) {
                indx_max = i;
                max_val = WaterTemperature[i];
            }
            if (WaterTemperature[i] < min_val) {
                indx_min = i;
                min_val = WaterTemperature[i];
            }
        }
        devider = 0;
        for (i = 0; i < 5; i++) {
            if ((i != indx_max)&&(i != indx_min)) {
                ActualWaterTemperature = (float) ActualWaterTemperature + WaterTemperature[i];
                devider ++;
            }
        }
        tVal = (float) ActualWaterTemperature / devider;
        ActualWaterTemperature = (float) tVal * 3.2 / 10.0;
        Devices.WaterTemperatureValueIsReady = 1;
    }

    switch (current_fan_task) {
        case FAN_INIT_TASK:
            PDC2 = FAN_PWM_FREQ * 1;
            current_fan_task = FAN_WAIT_FOR_SPEED_UPDATE;
            break;

        case FAN_WAIT_FOR_SPEED_UPDATE:
            if (Devices.WaterTemperatureValueIsReady)current_fan_task = FAN_SPEED_UPDATE;
            break;

        case FAN_SPEED_UPDATE:
#ifndef bluebox            
            Devices.WaterTemperatureValueIsReady = 0;
            current_fan_task = FAN_WAIT_FOR_SPEED_UPDATE;
            if ((ActualWaterTemperature >= 0) && (ActualWaterTemperature <= 0.1)) {
                FaultNumber = FAULT_COOL_SYS_TEMP_SENSOR;
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                current_fan_task = FAN_SYSTEM_FAULT;
            } else if ((ActualWaterTemperature >= 11) && (ActualWaterTemperature <= 33))PDC2 = FAN_PWM_FREQ * 0.65; //* 0.6;
            else if ((ActualWaterTemperature >= 34) && (ActualWaterTemperature <= 38))PDC2 = FAN_PWM_FREQ * 0.75; //* 0.7;
            else if ((ActualWaterTemperature >= 39) && (ActualWaterTemperature <= 43))PDC2 = FAN_PWM_FREQ * 0.85; //* 0.8;
            else if ((ActualWaterTemperature >= 44) && (ActualWaterTemperature <= 48))PDC2 = FAN_PWM_FREQ * 0.95; //* 0.9;
            else if ((ActualWaterTemperature >= 49) && (ActualWaterTemperature <= 53))PDC2 = FAN_PWM_FREQ;
            else if ((ActualWaterTemperature >= 54) && (ActualWaterTemperature <= 65)) {
                FaultNumber = FAULT_COOLING_SYSTEM_OVERTEMPERATURE;
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                current_fan_task = FAN_OVERTEMPERATURE;
            } else if (ActualWaterTemperature >= 66) {
                FaultNumber = FAULT_COOL_SYS_TEMP_SENSOR;
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                current_fan_task = FAN_SYSTEM_FAULT;
            }
#else
            PDC2 = FAN_PWM_FREQ * 0.85;
#endif
            break;
            //
        case FAN_OVERTEMPERATURE:
            if (ActualWaterTemperature <= 44) {
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_STANDBY;
                FaultNumber = CLEAR;
                current_fan_task = FAN_SPEED_UPDATE;
            } else if (ActualWaterTemperature >= 66) {
                FaultNumber = FAULT_COOL_SYS_TEMP_SENSOR;
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_FAULT;
                current_fan_task = FAN_SYSTEM_FAULT;
            }
            break;
            //
        case FAN_SYSTEM_FAULT:
            //shut down system here
            break;
    }
}
