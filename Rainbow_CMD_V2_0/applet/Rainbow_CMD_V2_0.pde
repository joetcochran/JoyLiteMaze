#include "Rainbow.h"
#include <Wire.h>
#include <avr/pgmspace.h>

//=============================================================
extern unsigned char dots_color[2][3][8][4];  //define Two Buffs (one for Display ,the other for receive data)
extern unsigned char GamaTab[16];             //define the Gamma value for correct the different LED matrix
extern unsigned char Prefabnicatel[5][3][8][4];
extern unsigned char ASCII_Char[52][8];
extern unsigned char ASCII_Number[10][8];
//=============================================================
unsigned char line,level;
unsigned char Buffprt=0;
unsigned char State=0;
unsigned char g8Flag1;
unsigned char RainbowCMD[29]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

byte arraysize = 29;
void setup()
{
  _init();

}

void loop()
{

  switch (State)
  {

    case waitingcmd:   
    break;

    case processing:
  	GetCMD();
    State=checking;
    break;

    case checking:
    if(CheckRequest)
    {
      State=waitingcmd;
      ClrRequest;
    }
    break;

   default:
    State=waitingcmd; 
    break;
  }
  
}


ISR(TIMER2_OVF_vect)          //Timer2  Service 
{ 
  TCNT2 = GamaTab[level];    // Reset a  scanning time by gamma value table
  flash_next_line(line,level);  // sacan the next line in LED matrix level by level.
  line++;
  if(line>7)        // when have scaned all LEC the back to line 0 and add the level
  {
    line=0;
    level++;
    if(level>15)       level=0;
  }
}

void init_timer2(void)               
{
  TCCR2A |= (1 << WGM21) | (1 << WGM20);   
  TCCR2B |= (1<<CS22);   // by clk/64
  TCCR2B &= ~((1<<CS21) | (1<<CS20));   // by clk/64
  TCCR2B &= ~((1<<WGM21) | (1<<WGM20));   // Use normal mode
  ASSR |= (0<<AS2);       // Use internal clock - external clock not used in Arduino
  TIMSK2 |= (1<<TOIE2) | (0<<OCIE2B);   //Timer2 Overflow Interrupt Enable
  TCNT2 = GamaTab[0];
  sei();   
}

void _init(void)    // define the pin mode
{
  DDRD=0xff;
  DDRC=0xff;
  DDRB=0xff;
  PORTD=0;
  PORTB=0;
  Wire.begin(4); // join i2c bus (address optional for master) 
  Wire.onReceive(receiveEvent); // define the receive function for receiving data from master
  Wire.onRequest(requestEvent); // define the request function for the request from maseter 
  init_timer2();  // initial the timer for scanning the LED matrix
}



void receiveEvent(int howMany)
{
  unsigned char i=0;
  while(Wire.available()>0)
  { 

    RainbowCMD[i]=Wire.receive();
    i++;
  }

  if((i==arraysize)&&(RainbowCMD[0]=='R')) State=processing;
  else      State=waitingcmd;	

}

void requestEvent(void)
{

  Wire.send(State); 
  if ((State==processing)||(State==checking))  SetRequest;

}


void DispshowWebcamFrame(void)
{
   unsigned char color=0,row=0,dots=0;
   unsigned char temp;
   unsigned char fir,sec;
   byte i = 5;
   
   RainbowCMD[1]=0;
   
  for(color=0;color<3;color++)
  {
    for (row=0;row<8;row++)
    {
      for (dots=0;dots<4;dots++)
      {
         dots_color[((Buffprt+1)&1)][color][row][dots] = RainbowCMD[i];
         i++;
      }
    }
  }
  Buffprt++;
  Buffprt&=1;

}


void DispshowPicture(void)
{
   unsigned char pi,shifts;
   unsigned char color=0,row=0,dots=0;
   unsigned char temp;
   unsigned char fir,sec;

   shifts=((RainbowCMD[2]>>4)&0x0F);
   pi=RainbowCMD[4];
   RainbowCMD[1]=0;
   
  for(color=0;color<3;color++)
  {
    for (row=0;row<8;row++)
    {
      for (dots=0;dots<4;dots++)
      {

        if (shifts&0x01)
        {             
          temp = dots + (shifts>>1);
	   fir=pgm_read_byte(&(Prefabnicatel[pi][color][row][(temp<4)?(temp):(temp-4)]));
	   sec=pgm_read_byte(&(Prefabnicatel[pi][color][row][(temp<3)?(temp+1):(temp-3)]));
          dots_color[((Buffprt+1)&1)][color][row][dots] = (fir<<4)|(sec>>4);  
        }        
        else
        {
          temp = dots + (shifts>>1);
         dots_color[((Buffprt+1)&1)][color][row][dots] = pgm_read_byte(&(Prefabnicatel[pi][color][row][(temp<4)?(temp):(temp-4)]));  
        }
      }
    }
  }
  Buffprt++;
  Buffprt&=1;

}

void DispshowChar(void)
{
  unsigned char Col_Red,Col_Blue,Col_Green,shift,ASCII;
  unsigned char tempword,color,row,dots,Num,tempdata,tempcol,AS;
  
  shift=((RainbowCMD[2]>>4)&0x0F);
  Col_Red=(RainbowCMD[2]&0x0F);
  Col_Green=((RainbowCMD[3]>>4)&0x0F);
  Col_Blue=(RainbowCMD[3]&0x0F);
  ASCII=RainbowCMD[4];
  RainbowCMD[1]=0;




    if((ASCII>64)&&(ASCII<91)) AS=ASCII-65; 
    else if((ASCII>96)&&(ASCII<123)) AS=ASCII-71;
    else if( (ASCII>='0')&&(ASCII<='9')) AS=ASCII-48;	
	
  for(color=0;color<3;color++)
  {
     if(color==0)        tempcol=Col_Green;
    else if(color==1)   tempcol=Col_Red;
    else if(color==2)   tempcol=Col_Blue;

    for (row=0;row<8;row++)
    {  
      if( (ASCII>='0')&&(ASCII<='9'))
       tempword=pgm_read_byte(&(ASCII_Number[AS][row]));	
      else
      tempword=pgm_read_byte(&(ASCII_Char[AS][row]));

      tempword=(shift<7)?(tempword<<shift):(tempword>>(shift-8));	 

      for (dots=0;dots<4;dots++)
      {


        if((tempword<<(2*dots))&0x80)
        {
          tempdata&=0x0F;
          tempdata|=(tempcol<<4);
        }
        else
        {
          tempdata&=0x0F;
        }

        if((tempword<<(2*dots+1))&0x80)
        {
          tempdata&=0xF0;
          tempdata|=tempcol;
        }
        else
        {
          tempdata&=0xF0;
        }   


        dots_color[((Buffprt+1)&1)][color][row][dots]=tempdata;	  


      }
    }
  }
  Buffprt++;
  Buffprt&=1;
  
}

void DispshowColor(void)
{   
  unsigned char color=0,row=0,dots=0;
  unsigned char Gr,Bl,Re;

  Re=(RainbowCMD[2]&0x0F);
  Gr=((RainbowCMD[3]>>4)&0x0F);
  Bl=(RainbowCMD[3]&0x0F);
  RainbowCMD[1]=0;
  
  for(color=0;color<3;color++)
  {
    for (row=0;row<8;row++)
    {
      for (dots=0;dots<4;dots++)
      {
        switch (color)
        {
        case 0://green
          dots_color[((Buffprt+1)&1)][color][row][dots]=( Gr|(Gr<<4));
          break;

        case 1://blue
          dots_color[((Buffprt+1)&1)][color][row][dots]= (Bl|(Bl<<4));
          break;

        case 2://red
          dots_color[((Buffprt+1)&1)][color][row][dots]= (Re|(Re<<4));
          break;

        default:
          break;
        }
      }
    }
  }
  Buffprt++;
  Buffprt&=1;
}


void DispshowCustom(void)
{ 
    unsigned char Col_Red,Col_Blue,Col_Green,shift,ASCII;
  unsigned char tempword,color,row,dots,Num,tempdata,tempcol,AS;
  int i;
  shift=0;
 
  RainbowCMD[1]=0;

  byte wholerowdata;
  byte dotpair;
  
  //render
  for(color=0;color<3;color++)
  {    
    for (row=0;row<8;row++)
    {
      if (color == 0)
        wholerowdata = RainbowCMD[13+row];
      if (color == 1)
        wholerowdata = RainbowCMD[5+row];
      if (color == 2)
        wholerowdata = RainbowCMD[21+row]; 
      for (dots=0;dots<4;dots++)
      {
        dotpair = 0x00;
        //higher order 4-bits
        if ((wholerowdata<<(dots*2))&0x80)
           dotpair|=0xF0;
           
        //lower order 4-bits
        if ((wholerowdata<<(dots*2+1))&0x80)
           dotpair|=0x0F;
           
           
   //     if ((color == 2) && (row == 7))
    //       dotpair = 0x00;  
        dots_color[((Buffprt+1)&1)][color][row][dots]=dotpair;	  
      }
    }
  } 
  
  


 Buffprt++;
 Buffprt&=1;
 
}



void GetCMD(void)
{
  switch(RainbowCMD[1])
  	{
  	   case showPrefabnicatel:
	   DispshowPicture();	
	   break;

	   case showChar:
	   DispshowChar();	
	   break;

	   case showColor:
	   DispshowColor();	
	   break;

	   case showCustom:
	   DispshowCustom();	
	   break;
	   
           case showWebcamFrame:
	   DispshowWebcamFrame();	
	   break;
	 
  	}

}



//==============================================================
void shift_1_bit(unsigned char LS)  //shift 1 bit of  1 Byte color data into Shift register by clock
{
  if(LS)
  {
    shift_data_1;
  }
  else
  {
    shift_data_0;
  }
  clk_rising;
}
//==============================================================
void flash_next_line(unsigned char line,unsigned char level) // scan one line
{
  disable_oe;
  close_all_line;
  open_line(line);
  shift_24_bit(line,level);
  enable_oe;
}

//==============================================================
void shift_24_bit(unsigned char line,unsigned char level)   // display one line by the color level in buff
{
  unsigned char color=0,row=0;
  unsigned char data0=0,data1=0;
  le_high;
  for(color=0;color<3;color++)//GBR
  {
    for(row=0;row<4;row++)
    {
      data1=dots_color[Buffprt][color][line][row]&0x0f;
      data0=dots_color[Buffprt][color][line][row]>>4;

      if(data0>level)   //gray scale,0x0f aways light
      {
        shift_1_bit(1);
      }
      else
      {
        shift_1_bit(0);
      }

      if(data1>level)
      {
        shift_1_bit(1);
      }
      else
      {
        shift_1_bit(0);
      }
    }
  }
  le_low;
}



//==============================================================
void open_line(unsigned char line)     // open the scaning line 
{
  switch(line)
  {
  case 0:
    {
      open_line0;
      break;
    }
  case 1:
    {
      open_line1;
      break;
    }
  case 2:
    {
      open_line2;
      break;
    }
  case 3:
    {
      open_line3;
      break;
    }
  case 4:
    {
      open_line4;
      break;
    }
  case 5:
    {
      open_line5;
      break;
    }
  case 6:
    {
      open_line6;
      break;
    }
  case 7:
    {
      open_line7;
      break;
    }
  }
}



