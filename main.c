//*********************************************************************//
// AC LAB KIT BUCK DUAL OUTPUT CONFIGURATION WITH CASCADED PI  PROGRAM //
//*********************************************************************//

//include files
#include "p30f6010A.h"
#include "main.h"
//

//
unsigned int DCLinkSample = 0;
char PID_Vsample =0;        //voltage PID sampling counter
char PID_Isample = 0;       //current PID sampling counter
unsigned int softStart = 0; //soft start counter

int Ir = 0;
int Iy = 0;
int VDC1 = 0;
int VDC2 = 0;

int Voffset = 0; //2.5V offset
int Ir_offset = 0;
int Iy_offset = 0;
int DCLink_PREvalue = 0;
long int Vr_PREerror = 0;
long int Vy_PREerror = 0; 
long int R_Integral = 0;
long int Y_Integral = 0;

int Vr_PI_value = 0;
int Vy_PI_value = 0;

int VrREF = -(55200/(int)V_Pgain); //initial output voltage 1 ref
int VyREF = -(55200/(int)V_Pgain); //initial output voltage 2 ref
//

//
void PID_Vr(void); // Voltage 1 PI
void PID_Ir(void); // Current 1 PI
void PID_Vy(void); // Voltage 2 PI
void PID_Iy(void); // Current 2 PI
//

int main(void)
{

init(); // call processor inialisation routine

//precharging init
PWMenable = 0; //PWM is off
delay(30);     //delay 30ms
RL1_ON = 1;    //precharging enable
delay(1000);   //delay 1000ms
RL1_ON = 0;    //precharging disable
delay(30);     //delay 30ms
RL2_ON = 1;    //bypass precharging
delay(30);     //delay 30ms

PWM1 = 0;
PWM2 = 0;
PWM3 = 0;
PWMenable = 1; //PWM is on
//precharging init ends

delay(30);     //delay 30ms
Voffset = adc(14); //read 2.5V offset
Ir_offset = adc(8); //read inductor current Ir offset
Iy_offset = adc(9); //read inductor current Iy offset

//resetting PID parameters
DCLink_PREvalue = adc(3); //initial value of dc-link voltage
PID_Vr();
PID_Ir();
PID_Vy();
PID_Iy(); //resetting end

ClrWdt();

    while(1)
	{


//Ir and Iy current peak detect every 0 match of PWM
if(Ipeak_flag)
{
IrCOUNT = PWM1; //T2 period value == half of on time
IyCOUNT = PWM2; //T3 period value == half of on time
IrCOUNT = IrCOUNT>>1;
IyCOUNT = IyCOUNT>>1;
IrTIMER_ON = 1; //T2 on
IyTIMER_ON = 1; //T3 on
Ipeak_flag = 0; //reset PWM1 0 match flag

}
//peak detect end

//Ir peak control
if(T2_Flag)
{
IrTIMER_ON = 0; //T2 off
T2_Flag = 0;
Ir = adc(8); //sample Ir
if(Ir >= (int)Irpeak) PWMenable = 0; //if peak detect pwm is off

} //peak control ends

//Iy peak control
if(T3_Flag)
{
IyTIMER_ON = 0; //T3 off
T3_Flag = 0;
Iy = adc(9); //sample Iy
if(Iy >= (int)Iypeak) PWMenable = 0; //if peak detect pwm is off

} //peak control ends

//loop counters
if(T1us_Flag) 
   {
PID_Vsample++; 
PID_Isample++;
softStart++;
DCLinkSample++;
ClrWdt();
T1us_Flag = 0;

   }
//loop counters end

//DCLink Status 
if(DCLinkSample >= (int)DCLinkCount)
{
VDC1 = adc(3); //read VDC/2
VDC2 = adc(2); //read VDC/2
if((VDC1 >= (int)VDCLink_Trip) || (VDC2 >= (int)VDCLink_Trip))
PWMenable = 1; //disable on if dc link overvoltage
else if ((VDC1 - VDC2) >= (int)VDCBalance) 
PWMenable = 1; //disable on if dc link unbalance
else if ((VDC1 - VDC2) <= -(int)VDCBalance) 
PWMenable = 1; //disable on if dc link unbalance
else Nop();
DCLinkSample = 0;
} //DCLink Status monitor end

if(PID_Vsample >= (char)PID_V_count) //voltage sample
   {
    PID_Vr(); // Voltage 1 PI 
    PID_Vy(); // Voltage 2 PI
    PID_Vsample = 0;

   } //voltage sample end

if(PID_Isample >= (char)PID_I_count) //current sample
   {
    VDC1 = adc(3); //dc link voltage for feedforward correction
    PID_Ir(); //Current 1 PI
    DCLink_PREvalue = VDC1;
    VDC1 = adc(3); //dc link voltage for feedforward correction
    PID_Iy(); //Current 2 PI
    DCLink_PREvalue = VDC1;
    PID_Isample = 0;

   } //current sample end

if(softStart >= (unsigned int)softCount) //soft start
   {
    VrREF++; //output 1 soft start ref
    VyREF++; //output 2 soft start ref
    if(VrREF >= (int)Vr_ref) VrREF = (int)Vr_ref; //final ref setting
    if(VyREF >= (int)Vy_ref) VyREF = (int)Vy_ref; //

    softStart = 0;

   } //soft start end


       } //while end

          } //main end

// ***************************************************************

//Output voltage 1 Vr PI routine *************************************************
void PID_Vr()
{

int currentError = 0;
long int Pterm = 0;
long int Iterm = 0;

int voltage;
long int PI_value = 0;

voltage = adc(5); //read Vr adc channel 5
voltage = (voltage - (Voffset+2))<<1; //subtract 2.5 V offset in feedback

if(voltage <= 0) voltage = 0;

currentError = VrREF - voltage;  // calculate error

Pterm = (long int)currentError*(long int)V_Pgain; //calculate Proportional term = error*Pgain
if(Pterm >= 55200) Pterm = 55200;
if(Pterm <= -55200) Pterm = -55200;

R_Integral = Vr_PREerror + (long int)currentError;  //calculated integrated error term = current error+previous error
if(R_Integral >= 55200) R_Integral = 55200;
if(R_Integral <= -55200) R_Integral = -55200;
Vr_PREerror = R_Integral;

Iterm = R_Integral*(long int)V_Igain; //calculate Integral term = integarted error*Igain
if(Iterm >= 55200) Iterm = 55200;
if(Iterm <= -55200) Iterm = -55200;

PI_value = Pterm + Iterm;  // PI output = Pterm+Iterm
if(PI_value >= 55200) PI_value = 55200;
if(PI_value <= -55200) PI_value = -55200;

PI_value = PI_value>>5; 

Vr_PI_value = (int)PI_value; //Voltage PI output is current reference 1

}
//Vr PI routine ends ********************************************

//Output current 1 Ir PID routine *************************************************
void PID_Ir()
{
int currentError =0;
long int Pterm = 0;

int current;
long int PI_value = 0;

current = adc(8); //read Ir adc channel 8 
current = (current - Ir_offset)<<1; //subtract 2.5 V offset from feedback
if(current <= 0) current = 0;

currentError = Vr_PI_value - current; //calculate current error

Pterm = (long int)currentError*(long int)I_Pgain; //calculate Proportional term = error*Pgain
if(Pterm >= 55200) Pterm = 55200;
if(Pterm <= -55200) Pterm = -55200;

PI_value = Pterm; // PI output = Pterm
if(PI_value >= 55200) PI_value = 55200;
if(PI_value <= -55200) PI_value = -55200;

PI_value = PI_value + 55200;
PI_value = PI_value>>5;

if(VrREF >= (int)Vr_ref)
PI_value =  PI_value - ((VDC1 - DCLink_PREvalue)<<3); //feedforward corection
if(PI_value <= 0) PI_value = 0; //pwm min. value
if(PI_value >= 3450) PI_value = 3450; //pwm max. value

asm("disi #0x3FFF");
PWM1 = (int)PI_value; //program duty cycle 1
asm("disi #0x0000");

}
//Ir PI routine end *********************************************

//Output voltage 2 Vy PI routine *************************************************
void PID_Vy()
{

int currentError = 0;
long int Pterm = 0;
long int Iterm = 0;

int voltage;
long int PI_value = 0;

voltage = adc(6); //read Vy adc channel 6
voltage = (voltage - (Voffset+2))<<1; //subtract 2.5 V offset in feedback

if(voltage <= 0) voltage = 0;

currentError = VyREF - voltage;  // calculate error

Pterm = (long int)currentError*(long int)V_Pgain; //calculate Proportional term = error*Pgain
if(Pterm >= 55200) Pterm = 55200;
if(Pterm <= -55200) Pterm = -55200;

Y_Integral = Vy_PREerror + (long int)currentError; //calculated integrated error term = current error+previous error 
if(Y_Integral >= 55200) Y_Integral = 55200;
if(Y_Integral <= -55200) Y_Integral = -55200;
Vy_PREerror = Y_Integral;

Iterm = Y_Integral*(long int)V_Igain; //calculate Integral term = integarted error*Igain
if(Iterm >= 55200) Iterm = 55200;
if(Iterm <= -55200) Iterm = -55200;

PI_value = Pterm + Iterm;  // PI output = Pterm+Iterm
if(PI_value >= 55200) PI_value = 55200;
if(PI_value <= -55200) PI_value = -55200;

PI_value = PI_value>>5;

Vy_PI_value = (int)PI_value; //Voltage PI output is current reference 2

}
//Vy PI routine ends ********************************************

//Output current 2 Iy PI routine *************************************************
void PID_Iy()
{
int currentError =0;
long int Pterm = 0;

int current;
long int PI_value = 0;

current = adc(9); //read inductor current Iy
current = (current - Iy_offset)<<1; //subtract 2.5 V offset from feedback
if(current <= 0) current = 0;

currentError = Vy_PI_value - current; //calculate current error

Pterm = (long int)currentError*(long int)I_Pgain; //calculate Proportional term = error*Pgain
if(Pterm >= 55200) Pterm = 55200;
if(Pterm <= -55200) Pterm = -55200;

PI_value = Pterm;  // PI output = Pterm
if(PI_value >= 55200) PI_value = 55200;
if(PI_value <= -55200) PI_value = -55200;

PI_value = PI_value + 55200;
PI_value = PI_value>>5;

if(VyREF >= (int)Vy_ref)
PI_value = PI_value - ((VDC1 - DCLink_PREvalue)<<3); //feedforward corection
if(PI_value <= 0) PI_value = 0; //pwm min. value
if(PI_value >= 3450) PI_value = 3450; //pwm max. value

asm("disi #0x3FFF");
PWM2 = (int)PI_value; //program duty cycle 2
asm("disi #0x0000");

}
//Iy PI routine end *********************************************
