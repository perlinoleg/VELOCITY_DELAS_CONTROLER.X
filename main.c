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


#include <stdint.h>        /* Includes uint16_t definition                    */
#include <stdbool.h>       /* Includes true/false definition                  */

#include "system.h"        /* System funct/params, like osc/peripheral config */
#include "user.h"          /* User funct/params, such as InitApp              */

/******************************************************************************/
/* Global Variable Declaration                                                */
/******************************************************************************/
extern void comm_tasks(void);
//extern void ADMeasurments(void);
//extern void InitBt(void);
extern void SystemCheck(void);
//extern void BtCommTask(void);
extern u16 SystemStateToUpdate;
/******************************************************************************/
/* Main Program                                                               */

/******************************************************************************/
//int __attribute__((space(prog), address(0xA7FE))) CheckSumResult = 0xE7DE;

int16_t main(void) {

    /* Configure the oscillator for the device */

#ifndef __SIMULATOR
    ConfigureOscillator();
#endif

    InitApp();
#ifdef maincfg  
    MemorySelfTest();
//    if (StoredCheckSum != FlashCheckSum) {
//        Devices.SystemStatedUpdate = TRUE;
//        SystemStateToUpdate = SYS_STATE_FAULT;
//        FaultNumber = FAULT_APPLICATION_CORRUPTED;
//        LED4 = 1;
//    }
#endif

    while (1) {
        ClrWdt();
        LED0 ^= 1;
        comm_tasks();
        SystemCheck();
    }
}

