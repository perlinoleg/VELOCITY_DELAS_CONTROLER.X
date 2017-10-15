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
//#include "tip.c"
#include <dsp.h>

/*================= Macros ===============================*/
#define CMD_ACK                     0x41
#define CMD_NACK                    0x14
#define CMD_STATUS_REQUEST          0x01
#define CMD_CHECKSUM_REQUEST        0x02
#define CMD_CHECKSUM_RESPONCE       0x12
#define CMD_STATUS_RESPONCE         0x33
#define CMD_SET_SYSTEM_STATE        0x04
#define CMD_FAN_CONTROL             0x45
#define CMD_PUMP_CONTROL            0x44
#define CMD_SIMMER_CONTROL          0x46
#define CMD_TEC_CONTROL             0x47
#define CMD_CAP_DISCHARGE           0x48
#define CMD_WORK_PARAMETERS         0x49
#define CMD_SW_VERSION_REQUEST      0x05
#define CMD_SW_VERSION              0x50
#define CMD_BOOT_MODE               0x55
#define CMD_CHARGER_CONTROL         0x20
#define CMD_SYS_MODE                0x09
#define CMD_PULSE_PARAMETERS        0x51 //from ipl not used in hpdl
#define CMD_REQUEST_MEASURMENTS     0x70
#define CMD_SEND_MEASURMENTS        0x75
#define CMD_APP_STATUS_REQUEST      0xA1
#define CMD_APP_STATUS_RESPONCE     0x1A
#define CMD_LASER_PULSE_PARAMETERS  0x61
#define CMD_TECHMODE_STATUS_REQUEST 0x1B
#define CMD_TECHMODE_STATUS_ANSWER  0xB1
#define CMD_LDDRIVER_CONTROL        0x31
#define CMD_HPDL_PULSE_PARAMETER    0x32
#define CMD_INFO_DATA               0xFF
#define CMD_LIGHTGUIDE_PARAMETER_REQUEST    0x25
#define CMD_LIGHTGUIDE_PARAMETER_RESPONCE   0x52
#define CMD_OPERATION_MODE          0x53

#define CMD_HW_STATUS_REQUEST       0xCC
#define CMD_HW_STATUS_RESPONCE      0xCD
/*========================================================*/

/*================= Variables ===============================*/
u8 RXbuffer[256]; //rx bufer
u8 indxRXbuffer = 0; //index of rx buffer
u8 TXbuffer[256]; //tx bufer
u8 indxTXbuff; //index of tx buffer
u16 FrameLen = 0; // farme data len to transmite
u16 tx100uS;
u16 NoCommunicationTimer = 0;

extern u8 ApplicatorCalibration[32];
extern u16 PumpVoltage;
extern u16 LddriverCurrent;
extern u16 TECVoltage;
extern unsigned long long PulseCounter;
extern u16 SystemMode;
extern u16 Trigger;
extern u16 SystemStateToUpdate;
extern u16 CoolingLevel;
extern u16 MaxAllowedCurrent;
extern u16 MinAllowedCurrent;
extern float ActualTecTemperature;
extern float tVal;
extern float ActualWaterTemperature;
/*==========================================================*/

/*================= Functions ===============================*/
u8 TxHeadCheckSum(void);
u8 TxFrameCheckSum(u8 tFrameLen);
u8 RxHeadCheckSum(void);
u8 RxFrameCheckSum(u8 tFrameLen);
void SendFrame(u8 tFrameLen, u8 tCommand);
void ExecuteReceivedCommand(u16 valCMD);

/*========================================================*/

/**********************************************************************
 * Function:        void comm_tasks(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Overview:	    communication managment.
 *
 ***********************************************************************/
void comm_tasks(void) {

    if (U1STAbits.OERR) {//if error clear flag
        U1STAbits.OERR = CLEAR;
        indxRXbuffer = 0;
    }

    if (NoCommunicationTimer > 3000) {//No communication for 3s initiate fault 22

    }

    if (CommFlags.ReadyToSendFlag) {//check if transmition frame ready to be transmitted
        if (tx100uS > 6) {//wait 6mS at least before send a respponce
            CommFlags.ReadyToSendFlag = 0; //reset transmite flag
            indxTXbuff = 0; //reset index of txbuffer
            while (indxRXbuffer); //here to prevent collision i have to check if i receive
            while (DIR); //or transmite something
            DIR = TX_MODE; // put rs485 transceiver to tx mode
            while (!U1STAbits.TRMT); //check if all TX buffer was sent
            U1TXREG = TXbuffer[0]; // move firs byte to uart
        }
    }

    if (CommFlags.FrameReceivedFlag) {//check if frame received
        indxRXbuffer = 0; //frame received, preapare rx buffer to receive
        CommFlags.FrameReceivedFlag = 0; //reset frame receive flag
        if (RxFrameCheckSum(RXbuffer[6] + 6) == RXbuffer[RXbuffer[6] + 7]) {//check tfarme check sum
            ExecuteReceivedCommand(RXbuffer[7]); //check sum correct, execute received command
        } else {
            ExecuteReceivedCommand(CMD_NACK); //the check sum incorrect send NAck to master
        }
    }
#ifdef __BLACKBOX
    if ((Devices2.InfoDataReadyToSend)&&(!indxRXbuffer)) {
        Devices2.InfoDataReadyToSend = 0;
        TXbuffer[8] = HIBYTE(HIWORD(ActualTecTemperature));
        TXbuffer[9] = LOBYTE(HIWORD(ActualTecTemperature));
        TXbuffer[10] = HIBYTE(LOWORD(ActualTecTemperature));
        TXbuffer[11] = LOBYTE(LOWORD(ActualTecTemperature));
        TXbuffer[12] = HIBYTE(HIWORD(tVal));
        TXbuffer[13] = LOBYTE(HIWORD(tVal));
        TXbuffer[14] = HIBYTE(LOWORD(tVal));
        TXbuffer[15] = LOBYTE(LOWORD(tVal));
        SendFrame(9, CMD_INFO_DATA); //send ack
    }
#endif
}

/**********************************************************************
 * Function:        void SendFrame(u8 tFrameLen, u8 tCommand)
 * PreCondition:    None
 * Input:	    tFrameLen - frame lentdh
 *                  tCommand - frame command
 * Output:	    None
 * Overview:	    This function used to initiate frame sending
 *
 ***********************************************************************/
void SendFrame(u8 tFrameLen, u8 tCommand) {
    tx100uS = 0; //clear timer
    FrameLen = tFrameLen + 8; //calculate frame len
    //indxTXbuff = 0; //reset index of txbuffer
    TXbuffer[6] = tFrameLen; //frame lenght
    TXbuffer[7] = tCommand; //CMD 
    TXbuffer[tFrameLen + 7] = TxFrameCheckSum(tFrameLen + 6); //calculate frame check sum
#ifndef release
    //TXbuffer[tFrameLen + 7] = 0x01; //TODO for validation
#endif
    CommFlags.ReadyToSendFlag = 1; //set transit flag
}

/**********************************************************************
 * Function:        u8 TxFrameCheckSum(u8 tFrameLen)
 * PreCondition:    None
 * Input:	    tFrameLen - frame lentdh
 * Output:	    CheckSum - check sum
 * Overview:	    This function used to calculate check sum of frame to send.
 *
 ***********************************************************************/
u8 TxFrameCheckSum(u8 tFrameLen) {
    u8 CheckSum = 0;
    u16 i;
    for (i = 6; i < (tFrameLen + 1); ++i) {
        CheckSum = CheckSum + TXbuffer[i];
    }
    return CheckSum;
}

/**********************************************************************
 * Function:        u8 RxFrameCheckSum(u8 tFrameLen)
 * PreCondition:    None
 * Input:	    tFrameLen - frame lentdh
 * Output:	    CheckSum - check sum
 * Overview:	    This function used to calculate check sum of received frame.
 *
 ***********************************************************************/
u8 RxFrameCheckSum(u8 tFrameLen) {
    u8 CheckSum = 0;
    u16 i;
    for (i = 6; i < (tFrameLen + 1); ++i) {
        CheckSum = CheckSum + RXbuffer[i];
    }
#ifndef release
    //CheckSum = 0x01; //TODO for validation
#endif
    return CheckSum;
}

/**********************************************************************
 * Function:        void ExecuteReceivedCommand(u16 valCMD)
 * PreCondition:    None
 * Input:	    valCMD - command code
 * Output:	    None
 * Overview:	    This function used to execute received command.
 *
 ***********************************************************************/
void ExecuteReceivedCommand(u16 valCMD) {
    u8 tIndx;

    switch (valCMD) {
        case CMD_STATUS_REQUEST:
            TXbuffer[8] = LgTypeId;
            TXbuffer[9] = (PulseCounter >> 56) & 0xFF;
            TXbuffer[10] = (PulseCounter >> 48) & 0xFF;
            TXbuffer[11] = (PulseCounter >> 40) & 0xFF;
            TXbuffer[12] = (PulseCounter >> 32) & 0xFF;
            TXbuffer[13] = (PulseCounter >> 24) & 0xFF;
            TXbuffer[14] = (PulseCounter >> 16) & 0xFF;
            TXbuffer[15] = (PulseCounter >> 8) & 0xFF;
            TXbuffer[16] = PulseCounter & 0xFF;
            TXbuffer[17] = Trigger;
            TXbuffer[18] = SystemState;
            TXbuffer[19] = FaultNumber;
            SendFrame(13, CMD_STATUS_RESPONCE); //send ack
            break;

        case CMD_PUMP_CONTROL:
            PumpVoltage = RXbuffer[9];
            PumpVoltage <<= 8;
            PumpVoltage += RXbuffer[10];
            Devices.PumpVoltageUpdate = TRUE;
            if (RXbuffer[8]) {
                Devices.PumpIsEnabled = TRUE;
            } else {
                Devices.PumpIsEnabled = FALSE;
                PumpVoltage = 0;
            }
            SendFrame(1, CMD_ACK); //send ack
            break;

        case CMD_BOOT_MODE:
            SendFrame(1, CMD_ACK); //send ack
            while (tx100uS < 4);
            CommFlags.ReadyToSendFlag = 0;
            while (indxRXbuffer); //here to prevent collision i have to check if i receive
            while (DIR); //or transmite something
            DIR = TX_MODE;
            U1STAbits.UTXEN = ENABLE;
            U1TXREG = TXbuffer[0];
            while (DIR == HIGH);
            //add pump off here
            Reset();
            break;

        case CMD_SW_VERSION_REQUEST:
            TXbuffer[8] = SW_VERSION_MAJOR;
            TXbuffer[9] = SW_VERSION_MINOR;
            TXbuffer[10] = SW_VERSION_BUILD;
            SendFrame(4, CMD_SW_VERSION);
            break;

        case CMD_SET_SYSTEM_STATE:
            if ((SystemState != SYS_STATE_FAULT) || (RXbuffer[8] == SYS_STATE_TECHMODE)) {
                SystemStateToUpdate = RXbuffer[8];
                Devices.SystemStatedUpdate = TRUE;
                SendFrame(1, CMD_ACK); //send ack
            } else {
                if (SystemState != SYS_STATE_FAULT) {
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_GUI_COMMUNICATION;
                    Devices.SystemStatedUpdate = TRUE;
                }
                SendFrame(1, CMD_NACK); //send ack
            }
            break;

        case CMD_TEC_CONTROL:
            TECVoltage = RXbuffer[9];
            TECVoltage <<= 8;
            TECVoltage += RXbuffer[10];
            if (TECVoltage > 15600)TECVoltage = 15600;
            Devices2.TecVoltageUpdateByVoltage = TRUE;
            if (RXbuffer[8]) {
                Devices.TecIsEnabled = TRUE;
            } else {
                Devices.TecIsEnabled = FALSE;
            }
            SendFrame(1, CMD_ACK); //send ack
            break;

        case CMD_CAP_DISCHARGE:
            if (RXbuffer[8]) {
                Devices.DischargeIsEnabled = TRUE;
            } else {
                Devices.DischargeIsEnabled = FALSE;
            }
            SendFrame(1, CMD_ACK); //send ack
            break;

        case CMD_CHARGER_CONTROL:
            if (RXbuffer[8]) {
                Devices.ChargerIsEnabled = TRUE;
            } else {
                Devices.ChargerIsEnabled = FALSE;
            }
            SendFrame(1, CMD_ACK); //send ack
            break;

        case CMD_HPDL_PULSE_PARAMETER:
            if (RXbuffer[16] < 150) { //check if critical parameters in allowable limits
                Pulse_On_Time = RXbuffer[8];
                Pulse_On_Time <<= 8;
                Pulse_On_Time += RXbuffer[9];

                Pulse_Off_Time = RXbuffer[10];
                Pulse_Off_Time <<= 8;
                Pulse_Off_Time += RXbuffer[11];

                if (Pulse_Off_Time) {
                    Pulse_Off_Time = Pulse_Off_Time - Pulse_On_Time;
                    //Pulse_Off_Time = Pulse_Off_Time;
                    Devices2.AutoRepeat = 1;
                } else {
                    Devices2.AutoRepeat = 0;
                    Pulse_Off_Time = 1000;
                }

                Pulse_On_Time = ((u32) (Pulse_On_Time)*10000) / 64; //= ((Xms-1) * 10000)/64
                Pulse_On_Time = Pulse_On_Time - 144; //offset for one shot

                Birst_On_Time = RXbuffer[12];
                Birst_On_Time <<= 8;
                Birst_On_Time += RXbuffer[13];

                Birst_Off_Time = RXbuffer[14];
                Birst_Off_Time <<= 8;
                Birst_Off_Time += RXbuffer[15];

                //Birst_Off_Time = Birst_Off_Time - Birst_On_Time;
                Birst_Off_Time = ((u32) (Birst_Off_Time)*10000) / 64;

                Birst_On_Time = ((u32) (Birst_On_Time)*10000) / 64; //= ((Xms-1) * 10000)/64
                Birst_On_Time = Birst_On_Time - 144; //offset for one shot

                LddriverCurrent = RXbuffer[16];
                Devices.DriverCurrentUpdate = TRUE;
                CoolingLevel = RXbuffer[17];
                SendFrame(1, CMD_ACK); //send ack

                MaxAllowedCurrent = LddriverCurrent + LddriverCurrent * 0.30;
                MinAllowedCurrent = LddriverCurrent - LddriverCurrent * 0.30;
            } else SendFrame(1, CMD_NACK);
#ifdef bluebox
            if ((RXbuffer[8] < 255)&&(RXbuffer[11] < 150)) { //check if critical parameters in allowable limits
                Pulse_On_Time = ((u32) (RXbuffer[8])*10000) / 64; //= ((Xms-1) * 10000)/64
                Pulse_On_Time = Pulse_On_Time - 144; //offset for one shot
                Pulse_Off_Time = RXbuffer[9];
                Pulse_Off_Time <<= 8;
                Pulse_Off_Time += RXbuffer[10];

                if (Pulse_Off_Time) {
                    Pulse_Off_Time = Pulse_Off_Time - RXbuffer[8];
                    //Pulse_Off_Time = Pulse_Off_Time;
                    Devices2.AutoRepeat = 1;
                } else {
                    Devices2.AutoRepeat = 0;
                    Pulse_Off_Time = 1000;
                }
                LddriverCurrent = RXbuffer[11];
                Devices.DriverCurrentUpdate = TRUE;
                CoolingLevel = RXbuffer[12];
                SendFrame(1, CMD_ACK); //send ack

                MaxAllowedCurrent = LddriverCurrent + LddriverCurrent * 0.30;
                MinAllowedCurrent = LddriverCurrent - LddriverCurrent * 0.30;
            } else SendFrame(1, CMD_NACK);
#endif
            break;

        case CMD_OPERATION_MODE:
            if (RXbuffer[8] == 3) {
                //slide mode chosen
                Devices2.SlideModeSelected = TRUE;
                SlideModePulsePerSquare = RXbuffer[9];
                SlideModePulsePerSquare <<= 8;
                SlideModePulsePerSquare += RXbuffer[10];
            } else {
                Devices2.SlideModeSelected = FALSE;
            }
            SendFrame(1, CMD_ACK); //send ack
            break;

        case CMD_LIGHTGUIDE_PARAMETER_REQUEST:
            for (tIndx = 0; tIndx < 32; tIndx++) {
                TXbuffer[tIndx + 8] = ApplicatorCalibration[tIndx];
            }
            SendFrame(33, CMD_LIGHTGUIDE_PARAMETER_RESPONCE);
            break;

        case CMD_HW_STATUS_REQUEST:
            TXbuffer[8] = Trigger;
            TXbuffer[9] = INTERLOCK;
            TXbuffer[10] = APP_SHORT;
            TXbuffer[11] = LIGHTGUIDE_SHORT;
            TXbuffer[12] = FLOW_SWITCH;
            TXbuffer[13] = (char) ActualWaterTemperature;
            TXbuffer[14] = (char) ActualTecTemperature;
            if ((Devices2.ChipIdIsConnected1)&&(Devices2.ChipIdIsConnected2)) {
                TXbuffer[15] = 0x01;
            } else TXbuffer[15] = 0x00;
            SendFrame(9, CMD_HW_STATUS_RESPONCE);
            break;
#ifdef bluebox 
        case CMD_LDDRIVER_CONTROL:
            if (RXbuffer[8]) {
                Devices.LddriverIsEnable = TRUE;
            } else {
                Devices.LddriverIsEnable = FALSE;
            }
            LddriverCurrent = RXbuffer[9];
            LddriverCurrent <<= 8;
            LddriverCurrent += RXbuffer[10];
            Devices.DriverCurrentUpdate = TRUE;
            SendFrame(1, CMD_ACK); //send ack
            break;
#endif          

        case CMD_CHECKSUM_REQUEST:
            TXbuffer[8] = HIBYTE(FlashCheckSum);
            TXbuffer[9] = LOBYTE(FlashCheckSum);
            SendFrame(3, CMD_CHECKSUM_RESPONCE);
            break;

        default:
            SendFrame(1, CMD_NACK); //send nack command is not recognised
    }
}
