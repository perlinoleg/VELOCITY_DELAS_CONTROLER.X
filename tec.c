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
#include "termistor.h"


/*================= Macros ===============================*/
#define TEC_PID_INIT                0
#define TEC_TASK_INIT               1
#define TEC_TASK_WORK               2
#define TEC_TASK_PID_IN_PROGRESS    3
#define TEC_TASK_FAULT              4

#define TEC_V_SENS_TIME 300
/*================================================*/

/*================= Variables ===============================*/
u16 TecTemperature[5] = {0, 0, 0, 0, 0};
u16 indxTecTemperature = 0;
u16 TECVoltage = 0;
u16 StoredTECValue = 9;
u16 current_tec_task = 0;
u16 CoolingLevel = 50;
float ActualTecTemperature = 25.0;
float ActualTermistorValue = 10000;
float tVal;
double lastTime;
double Input, Output, Setpoint = 0;
double errSum, lastErr = 0;
double kp, ki, kd;
u16 TecVoltageMonitor = 0;
u16 TecCurrentMonoitor = 0;

extern u16 TecVoltageSensingTimer;
extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
//void TecControl(float Voltage);
void TECPIDInit(void);
void tec_tasks(void);
void SetTECVoltage(float Voltage);
float GetTemperatureFromTable();
void PidCompute(void);

/*================================================*/

void tec_tasks(void) {
    u16 i, min_val, max_val, indx_min, indx_max, devider;

    if (indxTecTemperature == 5) {
        indxTecTemperature = 0;
        ActualTermistorValue = 0;

        min_val = TecTemperature[0];
        max_val = TecTemperature[0];
        indx_min = 0;
        indx_max = 0;
        for (i = 0; i < 5; i++) {
            if (TecTemperature[i] > max_val) {
                indx_max = i;
                max_val = TecTemperature[i];
            }
            if (TecTemperature[i] < min_val) {
                indx_min = i;
                min_val = TecTemperature[i];
            }
        }
        devider = 0;
        for (i = 0; i < 5; i++) {
            if ((i != indx_max)&&(i != indx_min)) {
                ActualTermistorValue = (float) ActualTermistorValue + TecTemperature[i];
                devider++;
            }
        }
        tVal = (float) ActualTermistorValue / devider * 3.2;
        ActualTermistorValue = (float) tVal * 20.5 / ((4600 - tVal) / 1000);
        if ((ActualTermistorValue < 129900)&&(ActualTermistorValue > 674)) {
            ActualTecTemperature = GetTemperatureFromTable();
            Devices.TecTemperatureValueIsReady = 1;
        } else {
            FaultNumber = FAULT_LIGHTGUIDE_SENSOR;
            Devices.SystemStatedUpdate = TRUE;
            SystemStateToUpdate = SYS_STATE_FAULT;
            current_tec_task = TEC_TASK_FAULT;
        }
    }

    if (Devices2.TecVoltageUpdateByVoltage) {
        SetTECVoltage((float) TECVoltage / 1000);
        if (Devices.TecIsEnabled)TEC_ENABLE = SET;
        else TEC_ENABLE = CLEAR;
        Devices2.TecVoltageUpdateByVoltage = FALSE;
    }

    if (Devices.TecIsEnabled == FALSE)current_tec_task = TEC_PID_INIT;
#ifdef maincfg
    switch (current_tec_task) {
        case TEC_PID_INIT:
            kp = 0.7;
            ki = 0.1;
            kd = 0.0;
            current_tec_task = TEC_TASK_INIT;
            break;

        case TEC_TASK_INIT:
            if ((Devices.TecIsEnabled) && (!TECVoltage)) {
                TEC_ENABLE = SET;
                current_tec_task = TEC_TASK_WORK;
            }
            break;

        case TEC_TASK_WORK:
            if (Devices.TecTemperatureValueIsReady) {
                Devices.TecTemperatureValueIsReady = 0;
                if ((ActualTecTemperature > 37) || (ActualTecTemperature< -7.1)) {
                    FaultNumber = FAULT_LIGHTGUIDE_TEMPERATURE;
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    current_tec_task = TEC_TASK_FAULT;
                } else current_tec_task = TEC_TASK_PID_IN_PROGRESS;
            }
            break;

        case TEC_TASK_PID_IN_PROGRESS:
            if (Devices.TecIsEnabled) {
                Input = ActualTecTemperature;
                PidCompute();
                if (Output < -3) {
                    tVal = Output * (-1);
                    if (tVal > 15.6) {
                        tVal = 15.6;
                    }
                    SetTECVoltage(tVal);

                } else {
                    TEC_ENABLE = DISABLE;
                }
                Devices2.InfoDataReadyToSend = 1;
                current_tec_task = TEC_TASK_WORK;
                Nop();
            } else {
                TEC_ENABLE = CLEAR;
                Devices.TecIsEnabled = FALSE;
                current_tec_task = TEC_PID_INIT;
            }
            break;

        case TEC_TASK_FAULT:
            if (ActualTecTemperature < 35) {
                Devices.SystemStatedUpdate = TRUE;
                SystemStateToUpdate = SYS_STATE_STANDBY;
                FaultNumber = CLEAR;
                current_tec_task = TEC_PID_INIT;
            }
            break;
    }
#endif    

    if (TecVoltageSensingTimer > TEC_V_SENS_TIME) {
        TecVoltageSensingTimer = 0;
        while (!CommFlags.SPI1RxDone);
        TEC_SPI_EN = HIGH;
        CS_ADC_TEC = LOW;
        i = SPIReadWriteWord(MCP3002_CH0);
        i >>= 1;
        TecCurrentMonoitor = (TecCurrentMonoitor + i) / 2; //i tec
        CS_ADC_TEC = HIGH;
        TEC_SPI_EN = LOW;

        TEC_SPI_EN = HIGH;
        CS_ADC_TEC = LOW;
        i = SPIReadWriteWord(MCP3002_CH1);
        i >>= 1;
        TecVoltageMonitor = (TecVoltageMonitor + i) / 2; //v tec
        CS_ADC_TEC = HIGH;
        TEC_SPI_EN = LOW;
    }
}

void PidCompute(void) {
    double error = Setpoint - Input;
    errSum += error;
    if (errSum > 120)errSum = 120;
    else if (errSum < -120)errSum = -120;
    double dErr = error - lastErr; /*Compute PID Output*/
    Output = kp * error + ki * errSum + kd * dErr;
    lastErr = error; /*Remember some variables for next time*/
}

void SetTECVoltage(float Voltage) {
    u16 Val;

    Voltage = (21.69 - Voltage) / 4.026 / 0.00488;
    if (Voltage > 1023)Voltage = 1023;
    Val = (int) Voltage;
    Val <<= 2;
    Val = 0x1000 + Val;
    while (!CommFlags.SPI1RxDone);
    TEC_SPI_EN = HIGH;
    CS_TEC_DAC = LOW;
    SPIReadWriteWord(Val);
    CS_TEC_DAC = HIGH;
    TEC_SPI_EN = LOW;
    if (Devices.TecIsEnabled == TRUE)TEC_ENABLE = ENABLE;

}

float GetTemperatureFromTable() {
    float ReturnTemperature;
    u16 i;
    float a, b;

    for (i = 0; i < 56; i++) {
        if (TempTable[i][1] == ActualTermistorValue) {
            ReturnTemperature = TempTable[i][0];
            break;
        } else if (TempTable[i][1] < ActualTermistorValue) {
            a = (TempTable[i - 1][1] - TempTable[i][1]) / (TempTable[i - 1][0] - TempTable[i][0]);
            b = TempTable[i][1]-(a * TempTable[i][0]);
            ReturnTemperature = (ActualTermistorValue - b) / a;
            break;
        }
    }
    return ReturnTemperature;
}

