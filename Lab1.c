#include <stdint.h>
#include <stdbool.h>
#include "../inc/tm4c123gh6pm.h"
#include "BSP.h"
#include "Profile.h"
#include "Texas.h"
#include "CortexM.h"
#include "Display.h"
#include "randomvalue.h"
#include "functiongame.h"
#define 	BLUE_MASK 		0x04


#define COLOR LCD_GREEN
#define BGCOLOR     LCD_BLACK
#define AXISCOLOR   LCD_ORANGE
#define MAGCOLOR    LCD_YELLOW
#define EWMACOLOR   LCD_CYAN
#define SOUNDCOLOR  LCD_CYAN
#define LIGHTCOLOR  LCD_LIGHTGREEN
#define TOPTXTCOLOR LCD_WHITE
#define TOPNUMCOLOR LCD_ORANGE
#define LIGHT_MAX 200000
#define LIGHT_MIN 0
uint16_t x1,y1;
int16_t x2=100, y2= 100;
uint8_t select1;
uint32_t Time, Magnitude = 0;
uint32_t LightData;
uint16_t random;
int flag ;
int count =1;
bool game;

int count1=0 ;
int j ;
//extern car_on_road enemy[2];
//extern xSemaphoreHandle car_generation;
//extern const unsigned char Enemy_car[];
extern unsigned volatile int First_enemy;
unsigned int ran=0;





void drawaxes(void)
{
 BSP_LCD_FillRect(30, 40, 50, 50, COLOR);  
}

void Timer0B_Init(unsigned long period)
{   
 volatile uint32_t ui32Loop; 
 SYSCTL_RCGC1_R |= SYSCTL_RCGC1_TIMER0; // activate timer0
 ui32Loop = SYSCTL_RCGC1_R;				// Do a dummy read to insert a few cycles after enabling the peripheral.
 TIMER0_CTL_R &= ~0x100;     // disable timer0A during setup
 TIMER0_CFG_R = 0x00000004;       // configure for 32-bit timer mode
 TIMER0_TBMR_R = 0x00000002;      // configure for periodic mode, default down-count settings
 TIMER0_TBILR_R = period-1;       // reload value
 NVIC_PRI5_R &= ~0x000000E0; 	 // configure Timer0A interrupt priority as 0
 NVIC_EN0_R |= 0x100000;     // enable interrupt 19 in NVIC (Timer0A)
 TIMER0_IMR_R |= 0x00000100;      // arm timeout interrupt
 TIMER0_CTL_R |= 0x00000100;      // enable timer0A
}

void Timer2A_Init(unsigned long period)
{   
	volatile uint32_t ui32Loop; 
	SYSCTL_RCGCTIMER_R|= 0x04; // activate timer0
  ui32Loop = SYSCTL_RCGC1_R;				// Do a dummy read to insert a few cycles after enabling the peripheral.
  TIMER2_CTL_R &= ~0x00000001;     // disable timer0A during setup
  TIMER2_CFG_R |= 0x00000004;       // configure for 32-bit timer mode
  TIMER2_TAMR_R |= 0x00000002;      // configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;       // reload value
	NVIC_PRI5_R &= ~0xE0000000; 	 // configure Timer2A interrupt priority as 0
  NVIC_EN0_R |= 0x800000;     // enable interrupt 23 in NVIC (Timer2A)
	TIMER2_IMR_R |= 0x00000001;      // arm timeout interrupt
  TIMER2_CTL_R |= 0x00000001;      // enable timer2A
}

void Timer0B_Handler(void)
{
		// acknowledge flag for Timer0A
		TIMER0_ICR_R |= 0x00000100; 
	
		// Toggle the blue LED.
    GPIO_PORTF_DATA_R ^=BLUE_MASK;

}

//---------------- Task0  ----------------
// Joystick initialization 
// *********Task0_Init*********
// Inputs:  none
// Outputs: none

void Task0_Init(void)
{
 BSP_Joystick_Init();
}

void Task0(void)
{
 BSP_Joystick_Input(&x1,&y1,&select1);
}
//---------------- Task2 ----------------
uint32_t Task2Failures;  // number of times Light wasn't ready
// *********Task2_Init*********
// initializes light sensor
// Task2 measures light intensity
// Inputs:  none
// Outputs: none
void Task2_Init(void)
{
 Task2Failures = 0;
 BSP_LightSensor_Init();
 LightData = BSP_LightSensor_Input();
 BSP_LightSensor_Start();
}

void Task2(void)
{
 TExaS_Task2();     // record system time in array, toggle virtual logic analyzer
 Profile_Toggle2(); // viewed by the logic analyzer to know Task2 started
 if(BSP_LightSensor_End(&LightData)==0)
 {
  Task2Failures++;      // should have been ready
 }
 BSP_LightSensor_Start(); // start measurement for next time
}

//---------------- Task3 measures light ----------------
// The Display on the LCD screen and starts the game when button is pressed 
// *********Task3_Init*********
// Inputs:  none
// Outputs: int
void Task3_Init(void)
{
  BSP_Button1_Init();   
}

int Task3(void)
{
 // static uint8_t prev1 = 0, prev2 = 0;
 uint8_t current;
 TExaS_Task3();     // record system time in array, toggle virtual logic analyzer
 Profile_Toggle3(); // viewed by the logic analyzer to know Task3 started
 BSP_LCD_DrawBitmap(10,110 ,logo1, 100,55);
 //BSP_Buzzer_Set(0);
 current = BSP_Button1_Input();
 if(current == 0)
 {
  BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
  BSP_LCD_DrawString(5, 4, "Get Set Go!!!", TOPTXTCOLOR);
  return 1;
 }
 else 
 {
	BSP_LCD_DrawBitmap(10,110 ,logo1, 100,55);
	//BSP_LCD_DrawString(5, 3, "Hi", TOPTXTCOLOR);
	return 0;
 }
}

//---------------- Task4 measures light ----------------
// Updates the player car according to the joystick value
// *********Task4_Init*********
// Inputs:  none
// Outputs: none
void Task4_Init(void)
{
 BSP_LCD_Init();
}

void Task4(void)
{
 TExaS_Task4();     // record system time in array, toggle virtual logic analyzer
 Profile_Toggle4(); // viewed by the logic analyzer to know Task4 started
 BSP_LCD_DrawFastHLine(10, 30, 100, COLOR);
 if(x1 < 400)
 {	
	BSP_LCD_DrawBitmap(25, 120 ,player_car, 11,21);
	BSP_LCD_DrawBitmap(85, 120 ,Blank_car, 11,21);
	flag = 0;  // for collision
 }
 if(x1 > 500)
 {
	BSP_LCD_DrawBitmap(85, 120 ,player_car, 11,21);
	BSP_LCD_DrawBitmap(25, 120 ,Blank_car, 11,21);
	flag = 1;  // for collision
 }
}


//---------------- Task5 measures light ----------------
// Display the X,Y coordinate and also the value of the intensity of light on the LCD Screen
// *********Task5_Init*********
// Inputs:  none
// Outputs: none
void Task5_Init(void){
  // assumes BSP_LCD_Init(); has been called

 }

 

// *********Task5*********
// updates the text at the top of the LCD
// Inputs:  none
// Outputs: none
void Task5(void){
  TExaS_Task5();     // record system time in array, toggle virtual logic analyzer
  Profile_Toggle5(); // viewed by the logic analyzer to know Task5 started
	BSP_LCD_DrawString(0, 0,  "x=",  TOPTXTCOLOR);
  BSP_LCD_DrawString(0, 1,  "y=",  TOPTXTCOLOR);
	BSP_LCD_DrawString(10, 0, "Light=", TOPTXTCOLOR);
  BSP_LCD_SetCursor(5,  0); BSP_LCD_OutUDec4(x1,          TOPNUMCOLOR);
  BSP_LCD_SetCursor(5,  1); BSP_LCD_OutUDec4(y1,         MAGCOLOR);
	BSP_LCD_SetCursor(16, 0); BSP_LCD_OutUDec4(LightData/100, LIGHTCOLOR);
 }


//---------------- Task6 measures light ----------------
// Updates the movement of the road
// *********Task6_Init*********
// Inputs:  none
// Outputs: none
void Task6_Init(void)
{
 BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
	
	/*BSP_LCD_DrawFastVLine(110, 100, 20, COLOR);
	BSP_LCD_DrawFastVLine(110, 70, 20, COLOR);
	BSP_LCD_DrawFastVLine(110, 40, 20, COLOR);
	BSP_LCD_DrawFastVLine(10, 100, 20, COLOR);
	BSP_LCD_DrawFastVLine(10, 70, 20, COLOR);
	BSP_LCD_DrawFastVLine(10, 40, 20, COLOR);
	BSP_LCD_DrawFastVLine(60, 110, 10, COLOR);
	BSP_LCD_DrawFastVLine(60, 60, 10, COLOR);
  BSP_LCD_DrawBitmap(110, 40, road3, 3,18);
	BSP_LCD_DrawBitmap(10 ,40, road3_D, 3,18);*/
}

 void Task6(void)
{
 unsigned int count=0, road_flag = 0;
 while (road_flag < 20)
 {
  road_flag++;
	if(road_flag > 0 && road_flag <6)
	{
	 BSP_LCD_DrawBitmap(110, 40 ,road1, 3,28);
	 BSP_LCD_DrawBitmap(10, 40 ,road1_D, 3,28);
	 for(count=0;count<4;count++)
	 {
		BSP_LCD_DrawBitmap(110, 40+count*30, road1, 3,28);
		BSP_LCD_DrawBitmap(10 , 40+count*30, road1_D, 3,28);
	 }
	}
	else if(road_flag > 5 && road_flag <11)
	{
	 BSP_LCD_DrawBitmap(110, 40 ,road2, 3,28);
	 BSP_LCD_DrawBitmap(10, 40 ,road2_D, 3,28);
	 for(count=0;count<4;count++)
	 {
		BSP_LCD_DrawBitmap(110, 40+count*30, road2, 3,28);
		BSP_LCD_DrawBitmap(10 ,40+count*30, road2_D, 3,28);
	 }
	}
  else if(road_flag > 10 && road_flag <16)
	{
	 BSP_LCD_DrawBitmap(110, 40 ,road3, 3,28);
	 BSP_LCD_DrawBitmap(10, 40 ,road3_D, 3,28);
	 for(count=0;count<4;count++)
	 {
	  BSP_LCD_DrawBitmap(110, 40+count*30, road3, 3,28);
		BSP_LCD_DrawBitmap(10 ,40+count*30, road3_D, 3,28);
	 }
	}
	else if(road_flag > 15 && road_flag <21)
	{
	 BSP_LCD_DrawBitmap(110, 40 ,road4, 3,28);
	 BSP_LCD_DrawBitmap(10, 40 ,road4_D, 3,28);
	 for(count=0;count<4;count++)
	 {
	  BSP_LCD_DrawBitmap(110, 40+count*30, road4, 3,28);
		BSP_LCD_DrawBitmap(10 ,40+count*30, road4_D, 3,28);
	 }
  }
	else
	{
	 road_flag = 0;
	}
 }
}
 
 
/*void Task3(void){
	int count = 10;
 for(int i = 3; i < 1 ;i--)
	{
	BSP_LCD_DrawFastVLine(110, 100 - count, 20, COLOR);
	BSP_LCD_DrawFastVLine(110, 70 - count, 20, COLOR);
	BSP_LCD_DrawFastVLine(110, 40- count, 20, COLOR);
	
	}
}*/
//---------------- Task7 measures light ----------------
// Updates the enemy car and check for accident
// *********Task7_Init*********
// Inputs:  none
// Outputs: none
 void Task7_Init(void)
{
}
	 
void Task7 (void)
{
if(j == 0)
 {
  random = rand();
  if (random < 32768)
	 j = 1;
			 
  else 
	 j = 2;
 }
 if(j == 1)
 {
  if(count <11 && count1 <10)
	{
	 BSP_LCD_DrawBitmap(85, 60+count1*10, Blank_car, 11,21);
	 BSP_LCD_DrawBitmap(85, 60+count*10, Enemy_car, 11,21);
	 if((count == 6) && (flag == 1))
	 { 
		BSP_LCD_DrawString( 5, 5, "Game Over!!!", TOPNUMCOLOR);
		game = false;
	 }
	 count ++;
	 count1++;
	}
  else 
	{
	 j=0;
	 count =1;
	 count1 =0;
	}
 }
 else if(j == 2)
 {
  if(count <11 && count1 <10)
	{
	 BSP_LCD_DrawBitmap(25, 60+count1*10, Blank_car, 11,21);
	 BSP_LCD_DrawBitmap(25, 60+count*10, Enemy_car, 11,21);
	 if((count == 6) && (flag == 0))
	 { 
		BSP_LCD_DrawString( 5, 5, "Game Over!!!", TOPNUMCOLOR);
		game = false;
	 }
	 count ++;
	 count1++;
	}
	else 
	{
	 j=0;
	 count =1;
	 count1 =0;
	}
 }
}


 void IntGlobalEnable(void)
{
    __asm("    cpsie   i\n");
}

int main(void)
 {
  while(1)
	{
	 unsigned long period = 8000000; 
	 j=0;
   DisableInterrupts();
   BSP_Clock_InitFastest();
   Profile_Init();               // initialize the 7 hardware profiling pins
	 game = true;
   TExaS_Init(GRADER, 1000 );         // initialize the Lab 1 grader
// TExaS_Init(LOGICANALYZER, 1000);  // initialize the Lab 1 logic analyzer
   Task0_Init();    // joystick init
   Task2_Init();    // light sensor init
	 Task3_Init();    // buttons init
   Task4_Init();    // LCD player car
   Task5_Init();    // LCD text init
	 Task6_Init();   //  movement of road
	 Task7_Init();       // enemy car generation and collision checking
		  
   Time = 0;
   EnableInterrupts(); // interrupts needed for grader to run
	 while(game)
	 {
	  int k = Task3();
	  if (k ==1)
	  {
		 BSP_LCD_FillScreen(BSP_LCD_Color565(0, 0, 0));
     while(game)
		 {
	    for(int i=0; i<10; i++)
			{
       Task0();
		   Task4();
		   Task6();  
	 	   Task7();  // collision and enemy car
		   BSP_Delay1ms(100);
		  }
		  Task2();
			Task5();
			Time++;
		  Profile_Toggle6();
	   }
    }
	 }
  }
 }
