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
#define WAIT_APPSHORT            0
#define WAIT_APPSHORT_BOUNCING  1
#define APPSHORT_FAULT          2
#define APPSHORT_FAULT_QUIT 3

#define LIGHTGUIDE_WAIT_SHORT            0
#define LIGHTGUIDE_WAIT_SHORT_BOUNCING  1
#define LIGHTGUIDE_FAULT          2
#define LIGHTGUIDE_FAULT_QUIT 3

#define WAIT_TRIGGER        0
#define WAIT_BOUNCING       1
#define WAIT_RELEASE        2
#define TRIGER_NOT_RELEASED 3
#define WAIT_FOR_SECOND_SWITCH 4
#define TRIGGER_FAULT       5
/*================================================*/

/*================= Variables ===============================*/
u16 current_app_short_tasks = 0;
u16 appShortTimeOut = 0;
u16 LightGuideShortTimeOut = 0;
u16 current_trigger_task = 0;
u16 Trigger = 0;
u16 TriggerCounter = 0;
u16 current_lihgtguide_short_tasks = 0;

extern u16 SystemStateToUpdate;
/*================================================*/

/*================= Functions ===============================*/
void lihgtguide_short_task(void);
void app_short_tasks(void);
void trigger_tasks(void);
/*================================================*/

/**********************************************************************
 * Function:        void lightguide_short_task(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function used to manage light guide connection
 *
 ***********************************************************************/
void lihgtguide_short_task(void) {
    if (Devices.ApplicatorIsConnected == TRUE) {
        switch (current_lihgtguide_short_tasks) {
            case LIGHTGUIDE_WAIT_SHORT:
                if (LIGHTGUIDE_SHORT) {
                    LightGuideShortTimeOut = 0; //reset timer
                    current_lihgtguide_short_tasks = LIGHTGUIDE_WAIT_SHORT_BOUNCING;
                } else {
                    Devices2.LighGuideIsConnected = TRUE;
                }
                break;

            case LIGHTGUIDE_WAIT_SHORT_BOUNCING:
                if (LightGuideShortTimeOut > 15) { //delay 5ms for bouncing
                    if (LIGHTGUIDE_SHORT) {//if ipl applicator disconnected or not connected shut off ipl module
                        Devices2.LighGuideIsConnected = FALSE; //lg disconnected
                        Devices2.LightGuideIsValid = FALSE;
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        FaultNumber = FAULT_LIGHTGUIDE_DISCONNECTED;
                        current_lihgtguide_short_tasks = LIGHTGUIDE_FAULT;
                    } else {
                        current_lihgtguide_short_tasks = LIGHTGUIDE_WAIT_SHORT;
                    }
                } else {
                    if (!LIGHTGUIDE_SHORT)current_lihgtguide_short_tasks = LIGHTGUIDE_WAIT_SHORT;
                }
                break;

            case LIGHTGUIDE_FAULT:
                if (!LIGHTGUIDE_SHORT) {
                    LightGuideShortTimeOut = 0; //reset timer
                    current_lihgtguide_short_tasks = LIGHTGUIDE_FAULT_QUIT;
                }
                break;

            case LIGHTGUIDE_FAULT_QUIT:
                if (LightGuideShortTimeOut > 500) {
                    if (!LIGHTGUIDE_SHORT) {
                        if (FaultNumber == FAULT_LIGHTGUIDE_DISCONNECTED) {
                            Devices2.LightGuideReconnected = TRUE;
                            Devices.SystemStatedUpdate = TRUE;
                            SystemStateToUpdate = SYS_STATE_INIT;
                            FaultNumber = CLEAR;
                            LightGuideShortTimeOut = 0; //reset timer
                            Devices.ApplicatorIsConnected = FALSE; //applicator disconnected because we need read chip id again.
                            appShortTimeOut = 0; //reset timer
                            current_app_short_tasks = WAIT_APPSHORT_BOUNCING;
                            current_lihgtguide_short_tasks = WAIT_APPSHORT_BOUNCING;
                        } else current_lihgtguide_short_tasks = LIGHTGUIDE_WAIT_SHORT;
                    }
                }
                break;
        }
    }
}

/**********************************************************************
 * Function:        void app_short_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function used to manage applicator connection 
 *
 ***********************************************************************/
void app_short_tasks(void) {
    if (SystemState != SYS_STATE_TECHMODE) {
        switch (current_app_short_tasks) {
            case WAIT_APPSHORT:
                if (APP_SHORT) {
                    appShortTimeOut = 0; //reset timer
                    current_app_short_tasks = WAIT_APPSHORT_BOUNCING;
                } else {
                    Devices.ApplicatorIsConnected = TRUE;
                }
                break;

            case WAIT_APPSHORT_BOUNCING:
                if (appShortTimeOut > 15) { //delay 5ms for bouncing
                    if (APP_SHORT) {//if applicator disconnected or not connected shut off ipl module
                        Devices.ApplicatorIsConnected = FALSE; //applicator disconnected
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        FaultNumber = FAULT_APPLICATOR_DISCONNECTED;
                        current_app_short_tasks = APPSHORT_FAULT;
                    } else {
                        current_app_short_tasks = WAIT_APPSHORT;
                    }
                } else if (!APP_SHORT)current_app_short_tasks = WAIT_APPSHORT;
                break;

            case APPSHORT_FAULT:
                if (!APP_SHORT) {
                    appShortTimeOut = 0; //reset timer
                    current_app_short_tasks = APPSHORT_FAULT_QUIT;
                }
                break;

            case APPSHORT_FAULT_QUIT:
                if (appShortTimeOut > 500) {
                    if (!APP_SHORT) {
                        if (FaultNumber == FAULT_APPLICATOR_DISCONNECTED) {
                            Devices.SystemStatedUpdate = TRUE;
                            SystemStateToUpdate = SYS_STATE_INIT;
                            FaultNumber = CLEAR;
                            appShortTimeOut = 0; //reset timer
                            current_app_short_tasks = WAIT_APPSHORT_BOUNCING;
                        }
                    }
                }
                break;
        }
    }
}

/**********************************************************************
 * Function:        void trigger_tasks(void)
 * PreCondition:    None
 * Input:	    None
 * Output:	    None
 * Overview:	    This function used to manage applicator trigger
 *
 ***********************************************************************/
void trigger_tasks(void) {
    if ((Devices.ApplicatorIsConnected == TRUE)&&(SystemState != SYS_STATE_FAULT)) {
        if ((Trigger == TRUE)&&(SystemState == SYS_STATE_STANDBY)) {
            current_trigger_task = TRIGER_NOT_RELEASED;
        }

        switch (current_trigger_task) {
            case WAIT_TRIGGER:
                if ((!TRIGGER1) && (!TRIGGER2)) {
                    TriggerCounter = 0; //reset timer
                    current_trigger_task = WAIT_BOUNCING;
                } else {
                    Trigger = 0; //trigger not pushed
                    if (((TRIGGER1) && (!TRIGGER2)) || ((!TRIGGER1) && (TRIGGER2))) {
                        TriggerCounter = 0;
                        current_trigger_task = WAIT_FOR_SECOND_SWITCH;
                    }
                }
                break;

            case WAIT_FOR_SECOND_SWITCH:
                if ((!TRIGGER1) && (!TRIGGER2)) {
                    TriggerCounter = 0; //reset timer
                    current_trigger_task = WAIT_BOUNCING;
                    //Devices.SystemStatedUpdate = TRUE;
                    //SystemStateToUpdate = SYS_STATE_STANDBY;
                    //FaultNumber = CLEAR;
                } else if ((TRIGGER1) && (TRIGGER2)) {
                    current_trigger_task = WAIT_TRIGGER;
                    Trigger = 0; //trigger not pushed{
                    //                Devices.SystemStatedUpdate = TRUE;
                    //                SystemStateToUpdate = SYS_STATE_STANDBY;
                    //                FaultNumber = CLEAR;
                } else {
                    if (TriggerCounter > 3500) {
                        Devices.SystemStatedUpdate = TRUE;
                        SystemStateToUpdate = SYS_STATE_FAULT;
                        FaultNumber = FAULT_TRIGGER_MALFUNCTION;
                        current_trigger_task = TRIGGER_FAULT;
                    }
                }

                break;

            case WAIT_BOUNCING:
                if (TriggerCounter > 15) { //delay 15ms for bouncing
                    if ((!TRIGGER1) && (!TRIGGER2)) {
                        Trigger = 1; // trigger pushed
                        current_trigger_task = WAIT_RELEASE;
                    } else {
                        Trigger = 0; //trigger not pushed
                        current_trigger_task = WAIT_TRIGGER;
                    }
                }
                break;

            case WAIT_RELEASE:
                if ((TRIGGER1) || (TRIGGER2)) {
                    current_trigger_task = WAIT_TRIGGER;
                    Trigger = 0; //trigger not pushed
                }
                break;

            case TRIGER_NOT_RELEASED:
#ifndef bluebox
                if (SystemState == SYS_STATE_PENDING) {
                    current_trigger_task = TRIGGER_FAULT;
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_FAULT;
                    FaultNumber = FAULT_TRIGGER_NOT_RELEASED;
                }
#endif
                if ((TRIGGER1) || (TRIGGER2)) {
                    current_trigger_task = WAIT_TRIGGER;
                    Trigger = 0; //trigger not pushed
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_STANDBY;
                    FaultNumber = CLEAR;
                }
                break;

            case TRIGGER_FAULT:
                if ((TRIGGER1) && (TRIGGER2)) {
                    current_trigger_task = WAIT_TRIGGER;
                    current_trigger_task = WAIT_TRIGGER;
                    Trigger = 0; //trigger not pushed
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_STANDBY;
                    FaultNumber = CLEAR;
                }
                break;
        }
    } else {
        if((SystemState == SYS_STATE_FAULT)&&( FaultNumber == FAULT_TRIGGER_NOT_RELEASED)){
            if ((TRIGGER1) && (TRIGGER2)) {
                    current_trigger_task = WAIT_TRIGGER;
                    Trigger = 0; //trigger not pushed
                    Devices.SystemStatedUpdate = TRUE;
                    SystemStateToUpdate = SYS_STATE_STANDBY;
                    FaultNumber = CLEAR;
                }
        }
    }
}