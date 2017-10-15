/******************************************************************************/
/* System Level #define Macros                                                */
/******************************************************************************/
#ifdef maincfg
#define SW_VERSION_MAJOR      1
#define SW_VERSION_MINOR      1
#define SW_VERSION_BUILD      81
#endif
/* Microcontroller MIPs (FCY) */
#define SYS_FREQ        80000000L
#define FCY             SYS_FREQ/2
#define BAUDRATE        115200

/******************************************************************************/
/* System Function Prototypes                                                 */
/******************************************************************************/

/* Custom oscillator configuration funtions, reset source evaluation
functions, and other non-peripheral microcontroller initialization functions
go here. */

void ConfigureOscillator(void); /* Handles clock switching/osc initialization */
