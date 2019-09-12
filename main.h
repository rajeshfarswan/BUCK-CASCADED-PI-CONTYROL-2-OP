// user defined register and function definitions //

#include "p30f6010A.h"

#define RL1_ON PORTGbits.RG0
#define RL2_ON PORTGbits.RG1
#define RL3_ON PORTGbits.RG2
#define RL4_ON PORTGbits.RG3
#define RL5_ON PORTGbits.RG6

#define PWM1 PDC1 //R PWM
#define PWM2 PDC2 //Y PWM
#define PWM3 PDC3 //B PWM

#define T1us_Flag IFS0bits.T1IF //every 1.5us flag

#define PID_V_count 33 //20.2Khz sampling
#define PID_I_count 12 //55.55Khz sampling
#define DCLinkCount 1000 //1.5ms sampling

#define PWM_PERIOD PTPER 
#define Converstion_Done ADCON1bits.DONE
#define Start_Converstion ADCON1bits.SAMP

#define CH0_MULA ADCHSbits.CH0SA
#define CH0_MULB ADCHSbits.CH0SB

#define PWMenable PTCONbits.PTEN

#define Ipeak_flag IFS2bits.PWMIF

#define VDCLink_Trip 905 //1023counts == 200V // dc link voltage trip
#define VDCBalance 100 //Dc link unbalace limit @20V // dc link voltage unbalance limit

#define Irpeak 900 //5A limit //peak current limit
#define Iypeak 900 //5A limit

#define V_Pgain 16  // PI gain parameters
#define V_Igain 1   // 

#define I_Pgain 36  //

#define Vr_ref 826 //1count == .218volts //max voltage ref 920 //826
#define Vy_ref 917 //1count == .218volts //max voltage ref 920 //917
                                         
#define softCount 500 //0.75mS //soft start counter

#define IrTIMER_ON T2CONbits.TON //timer2 for peak current detect
#define IrCOUNT PR2 
#define T2_Flag IFS0bits.T2IF //flag for Ir peak detect

#define IyTIMER_ON T3CONbits.TON //timer3 for peak current detect
#define IyCOUNT PR3
#define T3_Flag IFS0bits.T3IF //flag for Iy peak detect

// User Function Definitions

int adc(char); // adc function

void delay(unsigned int); //delay function

void init(void); //processor initialisation routine
