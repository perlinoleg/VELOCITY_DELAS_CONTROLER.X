/******************************************************************************/
/* Files to Include*/
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

#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"


#define READ_ROM            0x33
#define SKIP_ROM            0xCC
#define WRITE_SCRATCHPAD	0x0F
#define COPY_SCRATCHPAD		0x55
#define READ_MEMORY         0xF0
#define READ_SCRATCHPAD		0xAA
#define SEARCH_ROM          0xF0
#define MACH_ROM            0x55

#define TIP_INIT_TASK           0
#define TIP_READ_DATA_TASK_1    1
#define TIP_READ_DATA_TASK_2    9
#define TIP_READ_ID_TASK        2
#define TIP_VALIDATION          3
#define TIP_WRITE_DATA_TASK     4
#define TIP_LATCH_DATA          5
#define TIP_CONNECTION_SENS     6
#define TIP_WRITE_COMPLETED     7
#define TIP_WAIT_FOR_WRITE      8

u16 current_tip_task = 0;
unsigned long long LastStoredPulseCntr = 0;
u16 WritePulseCounterTimer = 0;
u8 IDData[128];
u8 ApplicatorCalibration[32];

u8 ROM_NO[8];
u8 IDs[2][8];
u8 ID_APP[8];
u8 ID_LIGHTGUIDE[8];
u16 LastDiscrepancy;
u16 LastFamilyDiscrepancy;
u16 LastDeviceFlag;
u8 crc8;
u16 *pntrIDApp;
u16 *pntrIDLg;
u16 TipSensFailed = 0;
u8 TA1, TA2, ES;
u16 pntrID = 0;

extern u16 Trigger;
extern u16 SystemStateToUpdate;
extern u8 ApplicatorCalibration[32];


u16 OWSearch();
void tip_tasks(void);
void Delay_uSxN(u16 uSxN);
unsigned char OW_reset_pulse(void);
void OW_write_bit(unsigned char write_bit);
unsigned char OW_read_bit(void);
void OW_write_byte(unsigned char write_data);
unsigned char OW_read_byte(void);
void drive_OW_low(void);
void drive_OW_high(void);
unsigned char read_OW(void);
void ReadChipIdData(void);
/**********************************************************************
 * Function:        void tip_tasks(void)
 * PreCondition:    None
 * Input:           None
 * Output:          None
 * Overview:	    Dallas id components managment.
 *                  This code generates reset sequence as per the protocol
 ***********************************************************************/
void tip_tasks(void) {
#ifndef bluebox
    u16 i;
    unsigned long long TIPUsedPulseCounter = 0;

    if (Devices.PulseInProgress == 0) {

        if ((Devices.ApplicatorIsConnected == FALSE) || (Devices2.LightGuideReconnected == TRUE)) {
            current_tip_task = TIP_INIT_TASK;
            CommFlags.TipAccepted = FALSE;
            Devices2.LightGuideReconnected = FALSE;
            Devices2.LightGuideIsValid = FALSE;
            Devices2.ApplicatorDataIsValid = FALSE;
            Devices2.ChipIdIsConnected1 = FALSE;
            Devices2.ChipIdIsConnected2 = FALSE;
        }

        switch (current_tip_task) { // to read info from id chip take 15mS aprox.
            case TIP_INIT_TASK:
                if (Devices.ApplicatorIsConnected) {
                    if (!OW_reset_pulse()) {
                        //Tip present
                        current_tip_task = TIP_READ_ID_TASK;
                    } else {
                        //tip not present
                        CommFlags.TipAccepted = FALSE;
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        FaultNumber = FAULT_NO_ID;
                    }
                }
                break;

            case TIP_READ_ID_TASK:
                if (!OW_reset_pulse()) {
                    LastDiscrepancy = 0;
                    LastDeviceFlag = FALSE;
                    LastFamilyDiscrepancy = 0;
                    if (OWSearch()) {
                        for (i = 0; i < 8; i++) IDs[0][i] = ROM_NO[i];
                        Devices2.ChipIdIsConnected1 = TRUE;
                    }
                    if (Devices2.LighGuideIsConnected == TRUE) {
                        if (OWSearch()) {
                            for (i = 0; i < 8; i++) IDs[1][i] = ROM_NO[i];
                            Devices2.ChipIdIsConnected2 = TRUE;
                        }
                    }
                    if (Devices2.ChipIdIsConnected1 || Devices2.ChipIdIsConnected2)current_tip_task = TIP_READ_DATA_TASK_1;
                    else {
                        current_tip_task = TIP_INIT_TASK;
                        Devices2.ChipIdIsConnected1 = FALSE;
                        Devices2.ChipIdIsConnected2 = FALSE;
                    }
                } else {
                    current_tip_task = TIP_INIT_TASK;
                    Devices2.ChipIdIsConnected1 = FALSE;
                    Devices2.ChipIdIsConnected2 = FALSE;
                }
                break;

            case TIP_READ_DATA_TASK_1:
                if (Devices2.ChipIdIsConnected1) {
                    if (!OW_reset_pulse()) {
                        OW_write_byte(MACH_ROM);
                        pntrID = 0;
                        for (i = 0; i < 8; i++) OW_write_byte(IDs[pntrID][i]);
                        ReadChipIdData();
                        current_tip_task = TIP_READ_DATA_TASK_2;
                    } else {
                        current_tip_task = TIP_INIT_TASK;
                        Devices2.ChipIdIsConnected1 = FALSE;
                        Devices2.ChipIdIsConnected2 = FALSE;
                    }
                }
                break;

            case TIP_READ_DATA_TASK_2:
                if (Devices2.ChipIdIsConnected2) {
                    if (!OW_reset_pulse()) {
                        OW_write_byte(MACH_ROM);
                        pntrID = 1;
                        for (i = 0; i < 8; i++) OW_write_byte(IDs[pntrID][i]);
                        ReadChipIdData();
                        //current_tip_task = TIP_VALIDATION;
                    } else {
                        current_tip_task = TIP_INIT_TASK;
                        Devices2.ChipIdIsConnected1 = FALSE;
                        Devices2.ChipIdIsConnected2 = FALSE;
                    }
                } else {
                    Devices2.LightGuideIsValid = FALSE;
                    Devices2.ApplicatorDataIsValid = FALSE;
                    CommFlags.TipAccepted = FALSE;
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_LIGHTGUIDE_ID;
                    current_tip_task = TIP_INIT_TASK;
                }
                if ((Devices2.ChipIdIsConnected1)&&(Devices2.ChipIdIsConnected2)&&(FaultNumber != FAULT_UNKNOWN_LIGHTGUIDE_TYPE)) {
                    current_tip_task = TIP_VALIDATION;
                }
#ifdef bluebox
                if ((Devices2.ChipIdIsConnected1)&&(Devices2.ChipIdIsConnected2)) {
                    current_tip_task = TIP_INIT_TASK;
                } else {
                    Devices2.ChipIdIsConnected1 = FALSE;
                    Devices2.ChipIdIsConnected2 = FALSE;
                }
#endif           
                break;
            case TIP_VALIDATION:
                if (Devices2.ApplicatorDataIsValid == TRUE) {
                    if (Devices2.LightGuideIsValid == TRUE) {
                        current_tip_task = TIP_WAIT_FOR_WRITE;
                    } else {
                        Devices2.LightGuideIsValid = FALSE;
                        Devices2.ApplicatorDataIsValid = FALSE;
                        CommFlags.TipAccepted = FALSE;
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        FaultNumber = FAULT_UNKNOWN_LIGHTGUIDE_TYPE;
                        current_tip_task = TIP_INIT_TASK;
                    }
                } else {
                    Devices2.LightGuideIsValid = FALSE;
                    Devices2.ApplicatorDataIsValid = FALSE;
                    CommFlags.TipAccepted = FALSE;
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_APPLICATOR_ID;
                    current_tip_task = TIP_INIT_TASK;
                }
                break;

            case TIP_WAIT_FOR_WRITE:
                if (Devices.ApplicatorIsConnected) {
                    if ((PulseCounter - LastStoredPulseCntr) >= 100) {
                        if (!Trigger) current_tip_task = TIP_WRITE_DATA_TASK;
                    } else if (WritePulseCounterTimer > 60000) {
                        if (PulseCounter > LastStoredPulseCntr) {
                            if (!Trigger) current_tip_task = TIP_WRITE_DATA_TASK;
                        } else WritePulseCounterTimer = 0;
                    }
                } else {
                    current_tip_task = TIP_INIT_TASK;
                }
                break;

            case TIP_WRITE_DATA_TASK:
                if (!OW_reset_pulse()) {
                    OW_write_byte(MACH_ROM);
                    for (i = 0; i < 8; i++) OW_write_byte(ID_APP[i]);
                    OW_write_byte(WRITE_SCRATCHPAD);
                    OW_write_byte(120); //the address of counter in dallas
                    OW_write_byte(0);
                    OW_write_byte((PulseCounter >> 56) & 0xFF);
                    OW_write_byte((PulseCounter >> 48) & 0xFF);
                    OW_write_byte((PulseCounter >> 40) & 0xFF);
                    OW_write_byte((PulseCounter >> 32) & 0xFF);
                    OW_write_byte((PulseCounter >> 24) & 0xFF);
                    OW_write_byte((PulseCounter >> 16) & 0xFF);
                    OW_write_byte((PulseCounter >> 8) & 0xFF);
                    OW_write_byte(PulseCounter & 0xFF);
                } else TipSensFailed++;

                TIPUsedPulseCounter = 0;
                if (!OW_reset_pulse()) {
                    OW_write_byte(MACH_ROM);
                    for (i = 0; i < 8; i++) OW_write_byte(ID_APP[i]);
                    OW_write_byte(READ_SCRATCHPAD);
                    TA1 = OW_read_byte();
                    TA2 = OW_read_byte();
                    ES = OW_read_byte();

                    for (i = 0; i < 8; i++) {
                        TIPUsedPulseCounter <<= 8;
                        TIPUsedPulseCounter = TIPUsedPulseCounter + OW_read_byte();
                    }

                    if ((TIPUsedPulseCounter != PulseCounter) || (ES != 7)) {
                        TipSensFailed++;
                    } else current_tip_task = TIP_LATCH_DATA;

                } else TipSensFailed++;

                if (TipSensFailed > 3) current_tip_task = TIP_INIT_TASK;
                break;

            case TIP_LATCH_DATA:
                if (!OW_reset_pulse()) {
                    OW_write_byte(MACH_ROM);
                    for (i = 0; i < 8; i++) OW_write_byte(ID_APP[i]);
                    OW_write_byte(COPY_SCRATCHPAD);
                    OW_write_byte(TA1); //the address of counter in dallas
                    OW_write_byte(TA2);
                    OW_write_byte(ES);
                    current_tip_task = TIP_WRITE_COMPLETED;
                    WritePulseCounterTimer = 0;
                    LastStoredPulseCntr = PulseCounter;
                } else current_tip_task = TIP_INIT_TASK;
                break;

            case TIP_WRITE_COMPLETED:
                if (WritePulseCounterTimer > 10) {
                    if (OW_read_byte() == 0xAA) {
                        OW_reset_pulse();
                        current_tip_task = TIP_WAIT_FOR_WRITE;
                    } else {
                        TipSensFailed++;
                        current_tip_task = TIP_WRITE_DATA_TASK;
                    }
                }
                break;
        }
    }
#endif
}

void ReadChipIdData(void) {
    u16 s;
    u8 ID_check_sum;

    if ((SystemState == SYS_STATE_FAULT)&&(FaultNumber == FAULT_UNKNOWN_LIGHTGUIDE_TYPE)) {
        Devices.SystemStatedUpdate = TRUE;
        SystemStateToUpdate = SYS_STATE_INIT;
        FaultNumber = CLEAR;
    }

    OW_write_byte(READ_MEMORY);
    OW_write_byte(0); //address of pulse counter register 0x00-0x03(0x00 is MSB, 0x03 is LSB)
    OW_write_byte(0);
    for (s = 0; s < 128; ++s) IDData[s] = OW_read_byte();
    if (IDData[33] == 0x10) {//ID of applicator get data
        for (s = 0; s < 8; s++) ID_APP[s] = IDs[pntrID][s];
        PulseCounter = IDData[120];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[121];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[122];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[123];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[124];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[125];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[126];
        PulseCounter <<= 8;
        PulseCounter = PulseCounter + IDData[127];
        LastStoredPulseCntr = PulseCounter;
        for (s = 0; s < 32; ++s) {
            ApplicatorCalibration[s] = IDData [s];
            ID_check_sum = ID_check_sum + ApplicatorCalibration[s];
        }
        if (IDData[32] == ID_check_sum)Devices2.ApplicatorDataIsValid = TRUE; //test checksum     
        else {
            Devices2.ApplicatorDataIsValid = FALSE;
        }
    } else if ((IDData[0] == 0x20) || (IDData[0] == 0x40) || (IDData[0] == 0x30)) {
        LgTypeId = IDData[0];
        for (s = 0; s < 8; s++) ID_LIGHTGUIDE[s] = IDs[pntrID][s];
        LgPulseCounter = IDData[10];
        LgPulseCounter <<= 16;
        LgPulseCounter = LgPulseCounter + IDData[11];
        Devices2.LightGuideIsValid = TRUE;
    } else if (IDData[0] == 0) {
        Devices2.LightGuideIsValid = FALSE;
    }
}

u16 OWSearch() {
    u16 id_bit_number;
    u16 last_zero, rom_byte_number, search_result;
    u16 id_bit, cmp_id_bit;
    u8 rom_byte_mask, search_direction;

    // initialize for search
    id_bit_number = 1;
    last_zero = 0;
    rom_byte_number = 0;
    rom_byte_mask = 1;
    search_result = 0;
    crc8 = 0;

    if (!LastDeviceFlag) {
        // 1-Wire reset
        if (OW_reset_pulse()) {
            // reset the search
            LastDiscrepancy = 0;
            LastDeviceFlag = FALSE;
            LastFamilyDiscrepancy = 0;
            return FALSE;
        }

        // issue the search command 
        OW_write_byte(SEARCH_ROM);

        // loop to do the search
        do {
            // read a bit and its complement
            id_bit = OW_read_bit();
            cmp_id_bit = OW_read_bit();

            // check for no devices on 1-wire
            if ((id_bit == 1) && (cmp_id_bit == 1))
                break;
            else {
                // all devices coupled have 0 or 1
                if (id_bit != cmp_id_bit)
                    search_direction = id_bit; // bit write value for search
                else {
                    // if this discrepancy if before the Last Discrepancy
                    // on a previous next then pick the same as last time
                    if (id_bit_number < LastDiscrepancy)
                        search_direction = ((ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
                    else
                        // if equal to last pick 1, if not then pick 0
                        search_direction = (id_bit_number == LastDiscrepancy);

                    // if 0 was picked then record its position in LastZero
                    if (search_direction == 0) {
                        last_zero = id_bit_number;

                        // check for Last discrepancy in family
                        if (last_zero < 9)
                            LastFamilyDiscrepancy = last_zero;
                    }
                }

                // set or clear the bit in the ROM byte rom_byte_number
                // with mask rom_byte_mask
                if (search_direction == 1)
                    ROM_NO[rom_byte_number] |= rom_byte_mask;
                else
                    ROM_NO[rom_byte_number] &= ~rom_byte_mask;

                // serial number search direction write bit
                OW_write_bit(search_direction);

                // increment the byte counter id_bit_number
                // and shift the mask rom_byte_mask
                id_bit_number++;
                rom_byte_mask <<= 1;

                // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
                if (rom_byte_mask == 0) {
                    //docrc8(ROM_NO[rom_byte_number]);  // accumulate the CRC
                    rom_byte_number++;
                    rom_byte_mask = 1;
                }
            }
        } while (rom_byte_number < 8); // loop until through all ROM bytes 0-7

        // if the search was successful then
        if (!((id_bit_number < 65) || (crc8 != 0))) {
            // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
            LastDiscrepancy = last_zero;

            // check for last device
            if (LastDiscrepancy == 0)LastDeviceFlag = TRUE;

            search_result = TRUE;
        }
    }
    if (!search_result || !ROM_NO[0]) {
        LastDiscrepancy = 0;
        LastDeviceFlag = FALSE;
        LastFamilyDiscrepancy = 0;
        search_result = FALSE;
    }

    return search_result;
}

/**********************************************************************
 * Function:        void drive_OW_low (void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   None
 * Overview:		   Configure the OW_PIN as Output and drive the OW_PIN LOW.
 ***********************************************************************/
void drive_OW_low(void) {
    OW_PIN_DIRECTION = OUTPUT;
    OW_WRITE_PIN = LOW;
}

/**********************************************************************
 * Function:        void drive_OW_high (void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   None
 * Overview:		   Configure the OW_PIN as Output and drive the OW_PIN HIGH.
 ***********************************************************************/
void drive_OW_high(void) {
    OW_PIN_DIRECTION = OUTPUT;
    OW_WRITE_PIN = HIGH;
}

/**********************************************************************
 * Function:        unsigned char read_OW (void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   Return the status of OW pin.
 * Overview:		   Configure as Input pin and Read the status of OW_PIN
 ***********************************************************************/
unsigned char read_OW(void) {
    unsigned char read_data = 0;

    OW_WRITE_PIN = INPUT;
    if (HIGH == OW_READ_PIN) read_data = SET;
    else read_data = CLEAR;

    return read_data;
}

/**********************************************************************
 * Function:        unsigned char OW_reset_pulse(void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   Return the Presense Pulse from the slave.
 * Overview:		   Initialization sequence start with reset pulse.
 *				   This code generates reset sequence as per the protocol
 ***********************************************************************/
unsigned char OW_reset_pulse(void) {
    unsigned char presence_detect;

    drive_OW_low(); // Drive the bus low
    Delay_uSxN(480); // delay 480 microsecond (us)
    drive_OW_high(); // Release the bus
    Delay_uSxN(70); // delay 70 microsecond (us)
    presence_detect = read_OW(); //Sample for presence pulse from slave
    Delay_uSxN(410); // delay 410 microsecond (us)
    drive_OW_high(); // Release the bus
    return presence_detect;
}

/**********************************************************************
 * Function:        void OW_write_bit (unsigned char write_data)
 * PreCondition:    None
 * Input:		   Write a bit to 1-wire slave device.
 * Output:		   None
 * Overview:		   This function used to transmit a single bit to slave device.
 *
 ***********************************************************************/
void OW_write_bit(unsigned char write_bit) {
    if (write_bit) {
        //writing a bit '1'
        drive_OW_low(); // Drive the bus low
        Delay_uSxN(6); // delay 6 microsecond (us)
        drive_OW_high(); // Release the bus
        Delay_uSxN(64); // delay 64 microsecond (us)
    } else {
        //writing a bit '0'
        drive_OW_low(); // Drive the bus low
        Delay_uSxN(60); // delay 60 microsecond (us)
        drive_OW_high(); // Release the bus
        Delay_uSxN(10); // delay 10 microsecond for recovery (us)
    }
}

/**********************************************************************
 * Function:        unsigned char OW_read_bit (void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   Return the status of the OW PIN
 * Overview:		   This function used to read a single bit from the slave device.
 *
 ***********************************************************************/

unsigned char OW_read_bit(void) {
    unsigned char read_data;
    //reading a bit
    drive_OW_low(); // Drive the bus low
    Delay_uSxN(6); // delay 6 microsecond (us)
    drive_OW_high(); // Release the bus
    Delay_uSxN(9); // delay 9 microsecond (us)
    read_data = read_OW(); //Read the status of OW_PIN
    Delay_uSxN(55); // delay 55 microsecond (us)
    return read_data;
}

/**********************************************************************
 * Function:        void OW_write_byte (unsigned char write_data)
 * PreCondition:    None
 * Input:		   Send byte to 1-wire slave device
 * Output:		   None
 * Overview:		   This function used to transmit a complete byte to slave device.
 *
 ***********************************************************************/
void OW_write_byte(unsigned char write_data) {
    unsigned char loop;
    for (loop = 0; loop < 8; loop++) {
        OW_write_bit(write_data & 0x01); //Sending LS-bit first
        write_data >>= 1; // shift the data byte for the next bit to send
    }
}

/**********************************************************************
 * Function:        unsigned char OW_read_byte (void)
 * PreCondition:    None
 * Input:		   None
 * Output:		   Return the read byte from slave device
 * Overview:		   This function used to read a complete byte from the slave device.
 *
 ***********************************************************************/
unsigned char OW_read_byte(void) {
    unsigned char loop, result = 0;

    for (loop = 0; loop < 8; loop++) {
        result >>= 1; // shift the result to get it ready for the next bit to receive
        if (OW_read_bit())
            result |= 0x80; // if result is one, then set MS-bit
    }
    return result;
}

/**********************************************************************
 * Function:        void Delay_uSxN(u16 uSxN)
 * PreCondition:    None
 * Input:		   uSxN micro seconds to delay
 * Output:		   None
 * Overview:		   This function used to delay for uSxN micro seconds.
 *
 ***********************************************************************/
void Delay_uSxN(u16 uSxN) {
    IFS1bits.T5IF = 0;
    TMR5 = 0;
    PR5 = uSxN * 40;
    T5CONbits.TON = 1; // Enable Timer1 and start the counter
    while (!IFS1bits.T5IF);
    T5CONbits.TON = 0;
}
