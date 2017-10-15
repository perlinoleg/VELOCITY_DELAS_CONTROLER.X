/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

/* Device header file */
#if defined(__XC16__)
#include <xc.h>
#elif defined(__C30__)
#if defined(__dsPIC33E__)
#include <p33Exxxx.h>
#elif defined(__dsPIC33F__)
#include <p33Fxxxx.h>
#endif
#endif

#include <stdint.h>        /* Includes uint16_t definition   */
#include <stdbool.h>       /* Includes true/false definition */
#include "user.h"

/******************************************************************************/
/* Interrupt Vector Options                                                   */
/******************************************************************************/
/*                                                                            */
/* Refer to the C30 (MPLAB C Compiler for PIC24F MCUs and dsPIC33F DSCs) User */
/* Guide for an up to date list of the available interrupt options.           */
/* Alternately these names can be pulled from the device linker scripts.      */
/*                                                                            */
/* dsPIC33F Primary Interrupt Vector Names:                                   */
/*                                                                            */
/* _INT0Interrupt      _C1Interrupt                                           */
/* _IC1Interrupt       _DMA3Interrupt                                         */
/* _OC1Interrupt       _IC3Interrupt                                          */
/* _T1Interrupt        _IC4Interrupt                                          */
/* _DMA0Interrupt      _IC5Interrupt                                          */
/* _IC2Interrupt       _IC6Interrupt                                          */
/* _OC2Interrupt       _OC5Interrupt                                          */
/* _T2Interrupt        _OC6Interrupt                                          */
/* _T3Interrupt        _OC7Interrupt                                          */
/* _SPI1ErrInterrupt   _OC8Interrupt                                          */
/* _SPI1Interrupt      _DMA4Interrupt                                         */
/* _U1RXInterrupt      _T6Interrupt                                           */
/* _U1TXInterrupt      _T7Interrupt                                           */
/* _ADC1Interrupt      _SI2C2Interrupt                                        */
/* _DMA1Interrupt      _MI2C2Interrupt                                        */
/* _SI2C1Interrupt     _T8Interrupt                                           */
/* _MI2C1Interrupt     _T9Interrupt                                           */
/* _CNInterrupt        _INT3Interrupt                                         */
/* _INT1Interrupt      _INT4Interrupt                                         */
/* _ADC2Interrupt      _C2RxRdyInterrupt                                      */
/* _DMA2Interrupt      _C2Interrupt                                           */
/* _OC3Interrupt       _DCIErrInterrupt                                       */
/* _OC4Interrupt       _DCIInterrupt                                          */
/* _T4Interrupt        _DMA5Interrupt                                         */
/* _T5Interrupt        _U1ErrInterrupt                                        */
/* _INT2Interrupt      _U2ErrInterrupt                                        */
/* _U2RXInterrupt      _DMA6Interrupt                                         */
/* _U2TXInterrupt      _DMA7Interrupt                                         */
/* _SPI2ErrInterrupt   _C1TxReqInterrupt                                      */
/* _SPI2Interrupt      _C2TxReqInterrupt                                      */
/* _C1RxRdyInterrupt                                                          */
/*                                                                            */
/* dsPIC33E Primary Interrupt Vector Names:                                   */
/*                                                                            */
/* _INT0Interrupt     _IC4Interrupt      _U4TXInterrupt                       */
/* _IC1Interrupt      _IC5Interrupt      _SPI3ErrInterrupt                    */
/* _OC1Interrupt      _IC6Interrupt      _SPI3Interrupt                       */
/* _T1Interrupt       _OC5Interrupt      _OC9Interrupt                        */
/* _DMA0Interrupt     _OC6Interrupt      _IC9Interrupt                        */
/* _IC2Interrupt      _OC7Interrupt      _PWM1Interrupt                       */
/* _OC2Interrupt      _OC8Interrupt      _PWM2Interrupt                       */
/* _T2Interrupt       _PMPInterrupt      _PWM3Interrupt                       */
/* _T3Interrupt       _DMA4Interrupt     _PWM4Interrupt                       */
/* _SPI1ErrInterrupt  _T6Interrupt       _PWM5Interrupt                       */
/* _SPI1Interrupt     _T7Interrupt       _PWM6Interrupt                       */
/* _U1RXInterrupt     _SI2C2Interrupt    _PWM7Interrupt                       */
/* _U1TXInterrupt     _MI2C2Interrupt    _DMA8Interrupt                       */
/* _AD1Interrupt      _T8Interrupt       _DMA9Interrupt                       */
/* _DMA1Interrupt     _T9Interrupt       _DMA10Interrupt                      */
/* _NVMInterrupt      _INT3Interrupt     _DMA11Interrupt                      */
/* _SI2C1Interrupt    _INT4Interrupt     _SPI4ErrInterrupt                    */
/* _MI2C1Interrupt    _C2RxRdyInterrupt  _SPI4Interrupt                       */
/* _CM1Interrupt      _C2Interrupt       _OC10Interrupt                       */
/* _CNInterrupt       _QEI1Interrupt     _IC10Interrupt                       */
/* _INT1Interrupt     _DCIEInterrupt     _OC11Interrupt                       */
/* _AD2Interrupt      _DCIInterrupt      _IC11Interrupt                       */
/* _IC7Interrupt      _DMA5Interrupt     _OC12Interrupt                       */
/* _IC8Interrupt      _RTCCInterrupt     _IC12Interrupt                       */
/* _DMA2Interrupt     _U1ErrInterrupt    _DMA12Interrupt                      */
/* _OC3Interrupt      _U2ErrInterrupt    _DMA13Interrupt                      */
/* _OC4Interrupt      _CRCInterrupt      _DMA14Interrupt                      */
/* _T4Interrupt       _DMA6Interrupt     _OC13Interrupt                       */
/* _T5Interrupt       _DMA7Interrupt     _IC13Interrupt                       */
/* _INT2Interrupt     _C1TxReqInterrupt  _OC14Interrupt                       */
/* _U2RXInterrupt     _C2TxReqInterrupt  _IC14Interrupt                       */
/* _U2TXInterrupt     _QEI2Interrupt     _OC15Interrupt                       */
/* _SPI2ErrInterrupt  _U3ErrInterrupt    _IC15Interrupt                       */
/* _SPI2Interrupt     _U3RXInterrupt     _OC16Interrupt                       */
/* _C1RxRdyInterrupt  _U3TXInterrupt     _IC16Interrupt                       */
/* _C1Interrupt       _USB1Interrupt     _ICDInterrupt                        */
/* _DMA3Interrupt     _U4ErrInterrupt    _PWMSpEventMatchInterrupt            */
/* _IC3Interrupt      _U4RXInterrupt     _PWMSecSpEventMatchInterrupt         */
/*                                                                            */
/* For alternate interrupt vector naming, simply add 'Alt' between the prim.  */
/* interrupt vector name '_' and the first character of the primary interrupt */
/* vector name.  There is no Alternate Vector or 'AIVT' for the 33E family.   */
/*                                                                            */
/* For example, the vector name _ADC2Interrupt becomes _AltADC2Interrupt in   */
/* the alternate vector table.                                                */
/*                                                                            */
/* Example Syntax:                                                            */
/*                                                                            */
/* void __attribute__((interrupt,auto_psv)) <Vector Name>(void)               */
/* {                                                                          */
/*     <Clear Interrupt Flag>                                                 */
/* }                                                                          */
/*                                                                            */
/* For more comprehensive interrupt examples refer to the C30 (MPLAB C        */
/* Compiler for PIC24 MCUs and dsPIC DSCs) User Guide in the                  */
/* <C30 compiler instal directory>/doc directory for the latest compiler      */
/* release.  For XC16, refer to the MPLAB XC16 C Compiler User's Guide in the */
/* <XC16 compiler instal directory>/doc folder.                               */
/*                                                                            */
/******************************************************************************/
/* Interrupt Routines                                                         */
/******************************************************************************/
u8 HeaderCheckSum;
u8 FrameCheckSum;
u16 BytesToEndOfFrame;
u16 CommTimeOutCount = 0;
u16 cnt100uS = 0;
u16 cntSPIMeasure = 0;
u16 TecPidTimer = 0;
u16 PumpVoltageSensingTimer = 0;
u16 TecVoltageSensingTimer = 0;
u16 IndxDiodeCurrent = 0;
u16 SlideModePulseCounter = 0;

extern u8 RXbuffer[256]; //rx bufer
extern u8 indxRXbuffer; //index of rx buffer
extern u8 TXbuffer[256]; //tx bufer
extern u8 indxTXbuff; //index of tx buffer
extern u16 FrameLen;
extern u16 DelayCounter;
extern u16 offTimeCntr;
extern u32 ChargerTimeCntr;
extern u16 tx100uS;
extern u16 TriggerCounter;
extern u16 PumpTimer;
extern u16 SimmerTimer;
extern u16 FanTimer;
extern u16 InterlockTimeOut;
extern u16 SPIMeasure;
extern u16 Pulse_On_Time;
extern u16 appShortTimeOut;
extern u16 LightGuideShortTimeOut;
extern u16 btIndxTx;
extern u16 btIndxRx;
extern u16 CharsToSend;
extern unsigned char bttx_buffer[8];
extern unsigned char BtRx_Buffer[8];

extern u16 SimmerMonitor[5];
extern u16 CapacitorMonitor[5];
extern u16 DiodeCurrent1[5];
extern u16 DiodeCurrent2[5];
extern u16 MeasuredDiodeVoltage[5];
extern u16 TempSensorApplicator[5];
extern u16 TempSensorAmbient[5];
extern u16 TecTemperature[5];
extern u16 indxTecTemperature;
extern u16 WaterTemperature[5];
extern u16 ADMeasurementsDone;
extern u16 WritePulseCounterTimer;
extern u16 TemperatureAdSamplingCounter;
extern u16 indxWaterTemperature;
extern u16 LddriverCurrentOffTime;
extern u16 FlickerTimer;
extern u16 TipTimeout;
extern u16 SpiTimeout;
extern u16 NoCommunicationTimer;

void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    static u16 mSecCounter = 0;

    IFS0bits.T1IF = CLEAR; /* reset Timer 1 interrupt flag */

    if (mSecCounter++ >= 9) {//to count in 1mS
        mSecCounter = 0;
        TriggerCounter++;
        PumpTimer++;
        TemperatureAdSamplingCounter++;
        appShortTimeOut++;
        LightGuideShortTimeOut++;
        InterlockTimeOut++;
        WritePulseCounterTimer++;
        tx100uS++;
        CommTimeOutCount++;
        if (CommTimeOutCount > 2)indxRXbuffer = 0;
        PumpVoltageSensingTimer++;
        TecVoltageSensingTimer++;
        ChargerTimeCntr++;
        FlickerTimer++;
        offTimeCntr++;
        SpiTimeout++;
        cnt100uS++;
        NoCommunicationTimer++;
        LED6 ^= 1;
    }
    cntSPIMeasure++;
}

void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = CLEAR; /* reset Timer 1 interrupt flag */
    Devices.PulseInProgress = 0; //end of pulse
    Devices2.PulseDone = 1;
    LED2 = 0;
    PulseCounter++;
    SlideModePulseCounter++;
    T2CONbits.TON = 0;
    T3CONbits.TON = 0;
    IFS0bits.T3IF = CLEAR; /* reset Timer 3 interrupt flag */
    //LD_INHIBIT1 = HIGH;
    LD_EN_PWM = 0;
    PULSE_PWM = 0;
    offTimeCntr = 0;
}

void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = CLEAR; /* reset Timer 3 interrupt flag */
    if (Devices.PulseInProgress) {
        Devices.PulseInProgress = 0;
        PR3 = Birst_Off_Time; // set pulse time
        TMR3 = 0; //reset timer
        LD_EN_PWM = 0;
        PULSE_PWM = 0;
    } else {
        Devices.PulseInProgress = 1;
        LD_EN_PWM = ENABLE;
        PR3 = Birst_On_Time; // set pulse time
        TMR3 = 0; //reset timer
        TMR4 = 0; //reset timer (timer 4 used for A/D pulse current measurements)
        IFS1bits.T4IF = CLEAR; /* reset Timer4 interrupt flag */
        T4CONbits.TON = 1; // Enable Timer4 and start the counter
    }
}

void __attribute__((interrupt, no_auto_psv)) _T4Interrupt(void) {
    IFS1bits.T4IF = CLEAR; /* reset Timer 1 interrupt flag */
    if (Devices.PulseInProgress)ADCPC0bits.SWTRG1 = 1;
    else T4CONbits.TON = 0;
}

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = CLEAR;
    RXbuffer[indxRXbuffer++] = U1RXREG;
    CommTimeOutCount = 0;
    if (indxRXbuffer == 1) {
        if (RXbuffer[0] != 0x55)
            indxRXbuffer = 0;
    } else if (indxRXbuffer == 2) {
        if (RXbuffer[1] == HPDL_ADDRESS)HeaderCheckSum = RXbuffer[1]; // the real string next row for debug only
            //if ((RXbuffer[1] == IPL_ADDRESS)||(RXbuffer[1] == 0x20))HeaderCheckSum = RXbuffer[1];//TODO for debug
        else indxRXbuffer = 0;
    } else if (indxRXbuffer == 6) {
        HeaderCheckSum = HeaderCheckSum + RXbuffer[2] + RXbuffer[3] + RXbuffer[4];
        if (RXbuffer[5] != HeaderCheckSum)indxRXbuffer = 0;
    } else if (indxRXbuffer == 7) {
        BytesToEndOfFrame = RXbuffer[6] + 8;
    } else if (indxRXbuffer == BytesToEndOfFrame) {
        CommFlags.FrameReceivedFlag = 1;
        NoCommunicationTimer = 0;
        LED3 ^= 1;
    }
}

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = CLEAR;
    indxTXbuff++;
    while (!U1STAbits.TRMT); //check if all TX buffer was sent
    if (indxTXbuff == FrameLen) {
        while (!U1STAbits.TRMT); //check if all TX buffer was sent
        //U1STAbits.UTXEN = DISABLE;
        DIR = RX_MODE;
    } else {
        while (!U1STAbits.TRMT); //check if all TX buffer was sent
        U1TXREG = TXbuffer[indxTXbuff]; //send byte
    }
}
//
//void __attribute__((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
//    IFS1bits.U2RXIF = CLEAR;
//    BtRx_Buffer[btIndxRx++] = U2RXREG;
//    if (btIndxRx > 7)btIndxRx = 0;
//
//}
//
//void __attribute__((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
//
//    IFS1bits.U2TXIF = CLEAR;
//    btIndxTx++;
//    while (!U2STAbits.TRMT); //check if all TX buffer was sent
//    if (btIndxTx < CharsToSend) {
//        U2TXREG = bttx_buffer[btIndxTx]; //send byte
//    }
//}

void __attribute__((interrupt, no_auto_psv)) _DMA0Interrupt(void) {
    IFS0bits.DMA0IF = 0; // Clear the DMA0 Interrupt Flag;
}

void __attribute__((interrupt, no_auto_psv)) _DMA1Interrupt(void) {
    IFS0bits.DMA1IF = 0; // Clear the DMA0 Interrupt Flag
    IEC0bits.SPI1IE = 0;
    CommFlags.SPI1RxDone = 1;
    CS_ADC_LD = HIGH;
    LD_SPI_ENABLE = LOW;
}

void __attribute__((interrupt, no_auto_psv)) _SPI1Interrupt(void) {
    IFS0bits.SPI1IF = 0;
    switch (SPIMeasure) {
        case DIODE_VOLTAGE:
            CS_ADC_LD = HIGH;
            Nop();
            Nop();
            Nop();
            CS_ADC_LD = LOW;
            break;

        case CAP_MONITOR:
            CS_ADC_LD = HIGH;
            Nop();
            Nop();
            Nop();
            CS_ADC_LD = LOW;
            break;
    }

}

void __attribute__((interrupt, no_auto_psv)) _ADCP0Interrupt(void) {
    WaterTemperature[indxWaterTemperature++] = ADCBUF0; // Read AN0 conversion result    
    TecTemperature[indxTecTemperature++] = ADCBUF1;
    IFS6bits.ADCP0IF = 0; // Clear ADC Pair 0 Interrupt Flag
    if (indxWaterTemperature < 5)ADCPC0bits.SWTRG0 = 1;
}

void __attribute__((interrupt, no_auto_psv)) _ADCP1Interrupt(void) {
    DiodeCurrent1[IndxDiodeCurrent] = ADCBUF2; // Read AN0 conversion result
    DiodeCurrent2[IndxDiodeCurrent] = ADCBUF3; // Read AN1 conversion result
    IFS6bits.ADCP1IF = 0; // Clear ADC Pair 0 Interrupt Flag
    ADCPC1bits.SWTRG2 = 1;
}

void __attribute__((interrupt, no_auto_psv)) _ADCP2Interrupt(void) {
    MeasuredDiodeVoltage[IndxDiodeCurrent] = ADCBUF4; // Read AN1 conversion result
    MeasuredDiodeVoltage[IndxDiodeCurrent++] = ADCBUF5; // Read AN0 conversion result
    IFS7bits.ADCP2IF = 0; // Clear ADC Pair 0 Interrupt Flag
    if (IndxDiodeCurrent < 5)ADCPC0bits.SWTRG1 = 1;
    else Devices2.DiodeCurrentMeasurementDone = 1;
}

//void __attribute__((interrupt, no_auto_psv)) _ADCP4Interrupt(void) {
//    TECTemperature[indxADCBuffer] = ADCBUF9; // Read AN0 conversion result
//    indxADCBuffer++;
//    if (indxADCBuffer > 4) {
//        indxADCBuffer = 0;
//        ADMeasurementsDone = 1;
//    }
//    IFS7bits.ADCP4IF = 0; // Clear ADC Pair 0 Interrupt Flag
//}