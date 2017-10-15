/******************************************************************************/
/* Files to Include                                                           */
/******************************************************************************/

#define _user

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

#include <stdint.h>          /* For uint16_t definition                       */
#include <stdbool.h>         /* For true/false definition                     */
#include "system.h"
#include "user.h"

/******************************************************************************/
/* User Functions                                                             */
/******************************************************************************/
unsigned int Spi1RxBuff[8] __attribute__((space(dma)));
unsigned int Spi1TxBuff[8] __attribute__((space(dma))) = {MCP3002_CH0, MCP3002_CH0, MCP3002_CH0, MCP3002_CH0, MCP3002_CH0, MCP3002_CH0, MCP3002_CH0, MCP3002_CH0};

extern u8 TXbuffer[256]; //tx bufer

typedef struct PROG_POINTER_TAG {

    union {
        __prog__ u16* pointer;

        struct {
            u16 offset;
            u8 tlbpag;
        };
    };
} PROG_POINTER;

u8 MemorySelfTest(void) {
    u8 Result = 0;
    u16 CS = 0;
    u16 wMemory;
    PROG_POINTER MemAddress;
    u32 length = 0;

    length >>= 1;
    MemAddress.pointer = 0;
    while (length < 0x1FF) {
        ClrWdt();
        TBLPAG = MemAddress.tlbpag;
        wMemory = __builtin_tblrdl(MemAddress.offset);
        Result = LOBYTE(wMemory);
        CS = CS + Result;
        Result = HIBYTE(wMemory);
        CS = CS + Result;
        wMemory = __builtin_tblrdh(MemAddress.offset);
        Result = LOBYTE(wMemory);
        CS = CS + Result;
        MemAddress.pointer++;
        length += 2;
    }
    while (length < 0xFFF) {
        ClrWdt();
        CS = CS + 0xFF;
        CS = CS + 0xFF;
        CS = CS + 0xFF;
        MemAddress.pointer++;
        length += 2;
    }

    while (length < 0xA7FE) {
        ClrWdt();
        TBLPAG = MemAddress.tlbpag;
        wMemory = __builtin_tblrdl(MemAddress.offset);
        Result = LOBYTE(wMemory);
        CS = CS + Result;
        Result = HIBYTE(wMemory);
        CS = CS + Result;
        wMemory = __builtin_tblrdh(MemAddress.offset);
        Result = LOBYTE(wMemory);
        CS = CS + Result;
        MemAddress.pointer++;
        length += 2;
    }

    TBLPAG = MemAddress.tlbpag;
    StoredCheckSum = __builtin_tblrdl(MemAddress.offset);

    CS = CS + 0xFF;
    CS = CS + 0xFF;
    CS = CS + 0xFF;
    MemAddress.pointer++;
    length += 2;

    while (length < 0xABFF) {
        ClrWdt();
        CS = CS + 0xFF;
        CS = CS + 0xFF;
        CS = CS + 0xFF;
        MemAddress.pointer++;
        length += 2;
    }

    MemAddress.pointer = (__prog__ u16*) (0xF80000);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0x0F;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF80004);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0x07;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF80006);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0x87;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF80008);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0xC7;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF8000A);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0xDF;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF8000C);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0x67;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //MemAddress.pointer++;

    MemAddress.pointer = (__prog__ u16*) (0xF8000E);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0xE3;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;

    MemAddress.pointer = (__prog__ u16*) (0xF80010);
    TBLPAG = MemAddress.tlbpag;
    wMemory = __builtin_tblrdl(MemAddress.offset);
    wMemory = wMemory & 0x3F;
    Result = LOBYTE(wMemory);
    CS = CS + Result;
    //    Result = HIBYTE(wMemory);
    //    CS = CS + Result;
    //PulseCounter = (u32) CS;
    FlashCheckSum = CS;
    return Result;
}

void InitApp(void) {
    SRbits.IPL = 0; //global interrupt enable for main application
    /*--------------------------------------------------------*/
    /* Timers Init*/
    T1CON = 0; // Timer reset
    IPC0bits.T1IP = 6; // Timer1 Interrupt priority level=7
    IFS0bits.T1IF = 0; // Reset Timer1 interrupt flag
    IEC0bits.T1IE = 1; // Enable Timer1 interrupt
    TMR1 = 0;
    PR1 = 3999; // Timer1 period register = 25nS x 4000 = 100uS
    T1CONbits.TON = 1; // Enable Timer1 and start the counter

    //
    T2CON = 0; // Timer reset
    T2CONbits.TCKPS = 3; // pescaler 1:64 (25nS * 256 = 6.4uS)
    IPC1bits.T2IP = 7; // Timer1 Interrupt priority level=7
    IFS0bits.T2IF = 0; // Reset Timer1 interrupt flag
    IEC0bits.T2IE = 1; // Enable Timer1 interrupt
    TMR2 = 0;
    
    T3CON = 0; // Timer reset
    T3CONbits.TCKPS = 3; // pescaler 1:64 (25nS * 256 = 6.4uS)
    IPC2bits.T3IP = 7; // Timer1 Interrupt priority level=7
    IFS0bits.T3IF = 0; // Reset Timer1 interrupt flag
    IEC0bits.T3IE = 1; // Enable Timer1 interrupt
    TMR3 = 0;

    
    T4CON = 0; // Timer reset
    T4CONbits.TCKPS = 2; // pescaler 1:64 (25nS * 64 = 1.6uS)
    IPC6bits.T4IP = 6; // Timer1 Interrupt priority level=7
    IFS1bits.T4IF = 0; // Reset Timer1 interrupt flag
    IEC1bits.T4IE = 1; // Enable Timer1 interrupt
    PR4 = 1000; // interrupt every 1.6mS
    TMR4 = 0;

    //timer 5 used for one wire communication
    T5CON = 0; // Timer reset
    IPC7bits.T5IP = 3; // Timer1 Interrupt priority level=4
    IFS1bits.T5IF = 0; // Reset Timer1 interrupt flag
    IEC1bits.T5IE = 0; // Enable Timer1 interrupt
    TMR5 = 0;
    PR5 = 40; // Timer1 period register = 19 x 25nS = 0.5uS
    /*--------------------------------------------------------*/

    /*--------------------------------------------------------*/
    /* Ports Init*/
    PORTA = CLEAR;
    LATA = CLEAR;

    TRISAbits.TRISA0 = INPUT; //interlock
    TRISAbits.TRISA1 = INPUT; //flow switch
    TRISAbits.TRISA2 = OUTPUT;
    BUZZ = LOW;
    TRISAbits.TRISA3 = OUTPUT;
    pLDAC = LOW;
    TRISAbits.TRISA4 = OUTPUT;
    tLDAC = LOW;
    TRISAbits.TRISA5 = OUTPUT;
    DIR = RX_MODE;
    TRISAbits.TRISA7 = OUTPUT;
    DISCHARGE = HIGH;
    TRISAbits.TRISA9 = OUTPUT;
    LED6 = OFF;
    TRISAbits.TRISA10 = OUTPUT;
    LED7 = OFF;
    //TRISAbits.TRISA14 = OUTPUT;
    //LDAC = LOW;
    TRISAbits.TRISA15 = OUTPUT;
    TEC_ENABLE = DISABLE;

    PORTB = CLEAR;
    LATB = CLEAR;

    //#ifndef __MPLAB_DEBUGGER_ICD3
    TRISBbits.TRISB6 = OUTPUT;
    TRISBbits.TRISB7 = OUTPUT;
    //#endif
    TRISBbits.TRISB12 = OUTPUT;
    LED2 = OFF;
    TRISBbits.TRISB13 = OUTPUT;
    LED3 = OFF;
    TRISBbits.TRISB14 = OUTPUT;
    LED4 = OFF;
    TRISBbits.TRISB15 = OUTPUT;
    LED5 = OFF;

    PORTC = CLEAR;
    LATC = CLEAR;
    TRISCbits.TRISC3 = OUTPUT;
    BT_AUTO_PAIRING = OFF;
    TRISCbits.TRISC13 = OUTPUT;
    PUMP_SPI_EN = DISABLE;
    TRISCbits.TRISC14 = INPUT;
    //CS_ADC_PUMP = HIGH;

    PORTD = CLEAR;
    LATD = CLEAR;

    TRISDbits.TRISD0 = OUTPUT;
    pCS_DAC = HIGH;
    TRISDbits.TRISD1 = INPUT; //TRIGGER2
    TRISDbits.TRISD2 = INPUT; //TRIGGER1
    TRISDbits.TRISD4 = OUTPUT;
    LD_INHIBIT1 = HIGH;
    TRISDbits.TRISD8 = OUTPUT;
    CS_TEC_DAC = HIGH;
    TRISDbits.TRISD9 = OUTPUT;
    CS_ADC_TEC = HIGH;
    TRISDbits.TRISD10 = OUTPUT;
    TEC_SPI_EN = DISABLE;
    TRISDbits.TRISD11 = OUTPUT;
    PUMP_SPI_EN = SET;
    TRISDbits.TRISD13 = OUTPUT; //TODO HW DEBUG change after task done
    CS_LD_DAC = HIGH;
    TRISDbits.TRISD14 = OUTPUT;
    LED0 = OFF;
    TRISDbits.TRISD15 = OUTPUT; //UART TX
    LED1 = OFF;

    PORTE = CLEAR;
    LATE = CLEAR;

    TRISEbits.TRISE0 = OUTPUT;
    //SAFE_IGBT = LOW;
    TRISEbits.TRISE1 = OUTPUT;
    PULSE_PWM = LOW;
    TRISEbits.TRISE3 = OUTPUT;
    FAN_PWM = HIGH;
    TRISEbits.TRISE4 = OUTPUT;
    TRISEbits.TRISE5 = INPUT;
    TRISEbits.TRISE7 = INPUT;

    PORTF = CLEAR;
    LATF = CLEAR;

    TRISFbits.TRISF1 = OUTPUT;
    BT_MASTER_RESET = LOW;
    TRISFbits.TRISF2 = INPUT;
    TRISFbits.TRISF3 = OUTPUT; //UART TX

    PORTG = CLEAR;
    LATG = CLEAR;

    TRISGbits.TRISG0 = OUTPUT;
    BT_AUTO_MASTER = OFF;
    TRISGbits.TRISG1 = OUTPUT;
    BT_BAUD_RATE = OFF;
    TRISGbits.TRISG2 = OUTPUT;
    LD_SPI_ENABLE = HIGH;
    TRISGbits.TRISG3 = OUTPUT;
    CS_DAC = HIGH;
    TRISGbits.TRISG6 = INPUT;
    TRISGbits.TRISG8 = OUTPUT;
    CS_ADC_LD = HIGH;
    TRISGbits.TRISG9 = OUTPUT;
    CHARGER_ENABLE = HIGH;
    TRISGbits.TRISG15 = OUTPUT;
    SAFE_IGBT_EN = LOW;
    /*--------------------------------------------------------*/

    /*-------------------------------------------------------------------------------------------------------------------*/
    /* UART Init*/
    U1MODEbits.UARTEN = 0; // Bit15 TX, RX DISABLED, ENABLE at end of func
    //U1MODEbits.notimplemented;	// Bit14
    U1MODEbits.USIDL = 0; // Bit13 Continue in Idle
    U1MODEbits.IREN = 0; // Bit12 No IR translation
    U1MODEbits.RTSMD = 1; // Bit11 Simplex Mode
    //U1MODEbits.notimplemented;	// Bit10
    U1MODEbits.UEN = 0; // Bits8,9 TX,RX enabled, CTS,RTS not
    U1MODEbits.WAKE = 0; // Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0; // Bit6 No Loop Back
    U1MODEbits.ABAUD = 0; // Bit5 No Autobaud (would require sending '55')
    U1MODEbits.URXINV = 0; // Bit4 IdleState = 1  (for dsPIC)
    U1MODEbits.BRGH = 0; // Bit3 16 clocks per bit period
    //U1MODEbits.PDSEL = 0; // Bits1,2 8bit, No Parity
    U1MODEbits.PDSEL = 0; // 9 bits
    U1MODEbits.STSEL = 0; // Bit0 One Stop Bit

    U1BRG = (FCY / (16 * BAUDRATE));
    //U1BRG = (FCY / (16 * BAUDRATE)) - 1; //to match veriscite we need increase the value by one
    // Load all values in for U1STA SFR
    U1STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    U1STAbits.UTXINV = 0; //Bit14 N/A, IRDA config
    U1STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15
    //U1STAbits.notimplemented = 0;	//Bit12
    U1STAbits.UTXBRK = 0; //Bit11 Disabled
    U1STAbits.UTXEN = 0; //Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0; //Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0; //Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0; //Bits6,7 Int. on character recieved
    //U1STAbits.ADDEN = 0; //Bit5 Address Detect Disabled
    U1STAbits.ADDEN = 0; //Bit5 Address Detect Enabled
    U1STAbits.RIDLE = 0; //Bit4 *Read Only Bit*
    U1STAbits.PERR = 0; //Bit3 *Read Only Bit*
    U1STAbits.FERR = 0; //Bit2 *Read Only Bit*
    U1STAbits.OERR = 0; //Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0; //Bit0 *Read Only Bit*
    IPC2bits.U1RXIP = 5; // High Range Interrupt Priority level, no urgent reason
    IPC3bits.U1TXIP = 5; //interrup priority level 3(1 - lowest, 7 - highest)
    //IPC16bits.U1EIP=6;

    //IEC4bits.U1EIE=ENABLE;
    IFS0bits.U1TXIF = 0; // Clear the Transmit Interrupt Flag
    IEC0bits.U1TXIE = 1; // Enable Transmit Interrupts
    IFS0bits.U1RXIF = 0; // Clear the Recieve Interrupt Flag
    IEC0bits.U1RXIE = 1; // Enable Recieve Interrupts

    U1MODEbits.UARTEN = 1; // And turn the peripheral on
    U1STAbits.UTXEN = ENABLE;

    /*-------------------------------------------------------*/
    TXbuffer[0] = 0x55; //SYNC
    TXbuffer[1] = 0x10; //to MASTER
    TXbuffer[2] = HPDL_ADDRESS; //from IPL controler
    TXbuffer[3] = 0; //RRQ not required
    TXbuffer[4] = 0xFF;
    TXbuffer[5] = TXbuffer[1] + TXbuffer[2] + TXbuffer[3] + TXbuffer[4];
    /*-------------------------------------------------------*/
    //
    //    U2MODEbits.UARTEN = 0; // Bit15 TX, RX DISABLED, ENABLE at end of func
    //    U2MODEbits.USIDL = 0; // Bit13 Continue in Idle
    //    U2MODEbits.IREN = 0; // Bit12 No IR translation
    //    U2MODEbits.RTSMD = 0; // Bit11 Simplex Mode
    //    U2MODEbits.UEN = 0; // Bits8,9 TX,RX enabled, CTS,RTS not
    //    U2MODEbits.WAKE = 0; // Bit7 No Wake up (since we don't sleep here)
    //    U2MODEbits.LPBACK = 0; // Bit6 No Loop Back
    //    U2MODEbits.ABAUD = 0; // Bit5 No Autobaud (would require sending '55')
    //    U2MODEbits.URXINV = 0; // Bit4 IdleState = 1  (for dsPIC)
    //    U2MODEbits.BRGH = 0; // Bit3 16 clocks per bit period
    //    U2MODEbits.PDSEL = 0; // 9 bits
    //    U2MODEbits.STSEL = 0; // Bit0 One Stop Bit
    //    U2BRG = (FCY / (16 * BAUDRATE)) - 1;
    //    // Load all values in for U1STA SFR
    //    U2STAbits.UTXISEL1 = 0; //Bit15 Int when Char is transferred (1/2 config!)
    //    U2STAbits.UTXINV = 0; //Bit14 N/A, IRDA config
    //    U2STAbits.UTXISEL0 = 0; //Bit13 Other half of Bit15
    //    //U1STAbits.notimplemented = 0;	//Bit12
    //    U2STAbits.UTXBRK = 0; //Bit11 Disabled
    //    U2STAbits.UTXEN = 0; //Bit10 TX pins controlled by periph
    //    U2STAbits.UTXBF = 0; //Bit9 *Read Only Bit*
    //    U2STAbits.TRMT = 0; //Bit8 *Read Only bit*
    //    U2STAbits.URXISEL = 0; //Bits6,7 Int. on character recieved
    //    //U1STAbits.ADDEN = 0; //Bit5 Address Detect Disabled
    //    U2STAbits.ADDEN = 1; //Bit5 Address Detect Enabled
    //    U2STAbits.RIDLE = 0; //Bit4 *Read Only Bit*
    //    U2STAbits.PERR = 0; //Bit3 *Read Only Bit*
    //    U2STAbits.FERR = 0; //Bit2 *Read Only Bit*
    //    U2STAbits.OERR = 0; //Bit1 *Read Only Bit*
    //    U2STAbits.URXDA = 0; //Bit0 *Read Only Bit*
    //    IPC7bits.U2RXIP = 2; // High Range Interrupt Priority level, no urgent reason
    //    IPC7bits.U2TXIP = 2; //interrup priority level 3(1 - lowest, 7 - highest)
    //    //IPC16bits.U1EIP=6;
    //
    //    //IEC4bits.U1EIE=ENABLE;
    //    IFS1bits.U2TXIF = 0; // Clear the Transmit Interrupt Flag
    //    IEC1bits.U2TXIE = 1; // Enable Transmit Interrupts
    //    IFS1bits.U2RXIF = 0; // Clear the Recieve Interrupt Flag
    //    IEC1bits.U2RXIE = 1; // Enable Recieve Interrupts
    //
    //    U2MODEbits.UARTEN = 1; // And turn the peripheral on
    //    U2STAbits.UTXEN = ENABLE;
    /*-------------------------------------------------------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* DMA SPI Init*/
    DMA0CON = 0x2001;
    DMA0CNT = 7;
    DMA0REQ = 0x00A;

    DMA0PAD = (volatile unsigned int) &SPI1BUF;
    DMA0STA = __builtin_dmaoffset(Spi1TxBuff);

    IPC1bits.DMA0IP = 4;
    IFS0bits.DMA0IF = 0; // Clear DMA interrupt
    IEC0bits.DMA0IE = 1; // Enable DMA interrupt
    DMA0CONbits.CHEN = 1; // Enable DMA Channel
    /*##########################################*/
    DMA1CON = 0x0001;
    DMA1CNT = 7;
    DMA1REQ = 0x00A;

    DMA1PAD = (volatile unsigned int) &SPI1BUF;
    DMA1STA = __builtin_dmaoffset(Spi1RxBuff);

    IPC3bits.DMA1IP = 4;
    IFS0bits.DMA1IF = 0; // Clear DMA interrupt
    IEC0bits.DMA1IE = 1; // Enable DMA interrupt

    DMA1CONbits.CHEN = 1; // Enable DMA Channel
    /*-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* SPI Init*/
    //IFS0bits.SPI1IF = 0; // Clear the Interrupt Flag
    IEC0bits.SPI1IE = 0; // Disable the Interrupt
    // SPI1CON1 Register Settings
    SPI1CON1bits.DISSCK = 0; // Internal Serial Clock is Enabled
    SPI1CON1bits.DISSDO = 0; // SDOx pin is controlled by the module
    SPI1CON1bits.MODE16 = 1; // Communication is word-wide (16 bits)
    SPI1CON1bits.SMP = 0; // Input data is sampled at the middle of data output time
    SPI1CON1bits.CKE = 0; // Serial output data changes on transition from Idle clock state to active clock state
    SPI1CON1bits.SSEN = 0; //SS pin used by module
    SPI1CON1bits.CKP = 1; // Idle state for clock is a low level; active state is a high level
    SPI1CON1bits.MSTEN = 1; // Master mode Enabled
    SPI1CON1bits.SPRE = 3; //secondary prescaler 1:1
    SPI1CON1bits.PPRE = 2; //primary prescaler 4:1
    // Interrupt Controller Settings
    SPI1STATbits.SPIEN = 1; // Enable SPI module
    IPC2bits.SPI1IP = 6;
    IFS0bits.SPI1IF = 0; // Clear the Interrupt Flag
    IEC0bits.SPI1IE = 0; // Disable the Interrupt
    /*-----------------------------------------------------------------------*/

    /*-----------------------------------------------------------------------*/
    /* PWM Init*/
    PTCON2bits.PCLKDIV = 6;
    PWMCON1bits.ITB = 1;
    PWMCON1bits.DTC = 2;
    PWMCON1bits.MDCS = 0;
    PHASE1 = 100;
    SPHASE1 = 100;
    IOCON1bits.PMOD = 3; //independed
    IOCON1bits.PENH = 0;
    IOCON1bits.PENL = 0;
    PDC1 = 50;
    SDC1 = 50;

    PWMCON2bits.ITB = 1;
    PWMCON2bits.DTC = 2;
    PHASE2 = FAN_PWM_FREQ;
    IOCON2bits.PMOD = 3; //independed
    IOCON2bits.PENH = 0;
    IOCON2bits.POLH = 1;
    PDC2 = FAN_PWM_FREQ * 0.8; //100% duty cicle
    IOCON2bits.PENH = 1; //fan pwm
    PTCONbits.PTEN = 1;

    /*-------------------------------------------------------------------------------------------------------------------*/

    /*-------------------------------------------------------*/
    /* A/D Init*/
    ADCONbits.FORM = 0; // Output in Integer Format
    ADCONbits.EIE = 1; // Enable Early Interrupt
    ADCONbits.ORDER = 0; // Normal Order of Conversion
    ADCONbits.SEQSAMP = 0; // Simultaneous Sampling
    ADCONbits.ASYNCSAMP = 1; // Asynchronous Sampling
    ADCONbits.SLOWCLK = 0; // High Frequency Clock Input
    ADCONbits.ADCS = 5; // Clock Divider Selection
    ADCPC0bits.TRGSRC0 = 0b00001; // individual Trigger Selected
    ADCPC0bits.TRGSRC1 = 0b00001; // individual Primary Trigger Selected
    ADCPC1bits.TRGSRC2 = 0b00001; // Time Primary Trigger Selected
    //ADCPC2bits.TRGSRC2 = 0b01100; // Time Primary Trigger Selected
    //    ADCPC4bits.TRGSRC8 = 0b01100; // Time Primary Trigger Selected
    //    ADCPC4bits.TRGSRC9 = 0b01100; // Time Primary Trigger Selected
    ADPCFGbits.PCFG0 = 0; // AN0 is configured as analog input Temperature #1
    ADPCFGbits.PCFG1 = 0; // AN1 is configured as analog input Temperature #2

    ADPCFGbits.PCFG2 = 0; // AN2 is configured as analog input Diode Current #1
    ADPCFGbits.PCFG3 = 0; // AN3 is configured as analog input Diode Current #2
    ADPCFGbits.PCFG4 = 0; // AN4 is configured as analog input
    ADPCFGbits.PCFG5 = 0; // AN5 is configured as analog input
    //    ADPCFGbits.PCFG8 = 0; // AN8 is configured as analog input
    //    ADPCFGbits.PCFG9 = 0; // A93 is configured as analog input
    IPC27bits.ADCP0IP = 0x04; // Set ADC Pair 0 Interrupt Priority (Level 4)
    IFS6bits.ADCP0IF = 0; // Clear ADC Pair 0 Interrupt Flag
    IEC6bits.ADCP0IE = 1; // Enable ADC Pair 0 Interrupt

    IPC27bits.ADCP1IP = 0x04; // Set ADC Pair 1 Interrupt Priority (Level 4
    IFS6bits.ADCP1IF = 0; // Clear ADC Pair 1 Interrupt Flag
    IEC6bits.ADCP1IE = 1; // Enable ADC Pair 1 Interrupt

    IPC28bits.ADCP2IP = 0x04; // Set ADC Pair 2 Interrupt Priority (Level 4)
    IFS7bits.ADCP2IF = 0; // Clear ADC Pair 3 Interrupt Flag
    IEC7bits.ADCP2IE = 1; // Enable ADC Pair 4 Interrupt
    //    IPC28bits.ADCP4IP = 0x04; // Set ADC Pair 2 Interrupt Priority (Level 4)
    //    IFS7bits.ADCP4IF = 0; // Clear ADC Pair 3 Interrupt Flag
    //    IEC7bits.ADCP4IE = 1; // Enable ADC Pair 4 Interrupt
    ADCONbits.ADON = 1; // Enable ADC Module
    /*-------------------------------------------------------*/

}

u16 SPIReadWriteWord(u16 WordVar) {
    u16 DemoResult;

    DemoResult = SPI1BUF; // dummy read of the SPI1BUF register to clear the SPIRBF flag
    SPI1BUF = WordVar; // write the data out to the SPI peripheral
    while (!SPI1STATbits.SPIRBF); // wait for the data to be sent out
    DemoResult = SPI1BUF;
    return DemoResult;
}


