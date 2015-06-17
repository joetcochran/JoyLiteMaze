#include <Wire.h>
#include <avr/pgmspace.h>

#define CURSORDELAY                20
#define ITERATION_THRESHOLD        10
#define DIRECTION_UP               1
#define DIRECTION_RIGHT            2
#define DIRECTION_LEFT             3
#define DIRECTION_DOWN             4

#include "WProgram.h"
void setup();
void ReadJoystick();
void loop();
void MoveCursor(byte direction);
void SendData();
void ShowCustom(int Address, unsigned char R1, unsigned char R2, unsigned char R3, unsigned char R4, unsigned char R5, unsigned char R6, unsigned char R7, unsigned char R8,
unsigned char G1, unsigned char G2, unsigned char G3, unsigned char G4, unsigned char G5, unsigned char G6, unsigned char G7, unsigned char G8,
unsigned char Bl1, unsigned char Bl2, unsigned char Bl3, unsigned char Bl4, unsigned char Bl5, unsigned char Bl6, unsigned char Bl7, unsigned char Bl8);
void SentCMD(int  Add);
void GenerateMaze();
void RandomDirections();
boolean isfree(int xpos, int ypos);
unsigned char RainbowCMD[29];
unsigned char State = 0;  
unsigned long timeout;

bool cursorstate;
byte cursorX = 1;
byte cursorY = 1;
int cursordelay = 0;
int movedelay = 0;
bool picture[3][8][8];
byte argument_arrays[3][8];

byte RANSOMISATION PROGMEM = 0 ;
byte MAZE_X PROGMEM = 21 ; 
byte MAZE_Y PROGMEM = 21 ;
byte X = 0;
byte Y = 0;
byte tilerow = 1;
byte tilecol = 1;
bool blnMaze[21][21];
short cDir0X;
short cDir0Y;
short cDir1X;
short cDir1Y;
short cDir2X;
short cDir2Y;
short cDir3X;
short cDir3Y;


byte upIterationCount = 0;
byte downIterationCount = 0;
byte leftIterationCount = 0;
byte rightIterationCount = 0;

byte upPin    PROGMEM =5 ;   
byte downPin  PROGMEM =3 ;  
byte leftPin  PROGMEM =6 ;    
byte rightPin PROGMEM =4 ;    

bool upval;
bool downval;
bool leftval;
bool rightval;


void setup()
{
  pinMode(upPin, INPUT);  
  pinMode(leftPin, INPUT);  
  pinMode(rightPin, INPUT);  
  pinMode(downPin, INPUT);    
  
  digitalWrite(upPin, HIGH);
  digitalWrite(downPin, HIGH);
  digitalWrite(leftPin, HIGH);
  digitalWrite(rightPin, HIGH);
  
  randomSeed(analogRead(0));
  Wire.begin(); // join i2c bus (address optional for master) 
  //randomSeed(analogRead(0));
  
  cDir0X = 0; cDir0Y = 0; 
  cDir1X = 0; cDir1Y = 0; 
  cDir2X = 0; cDir2Y = 0; 
  cDir3X = 0; cDir3Y = 0; 
  GenerateMaze();
  

  
}


void ReadJoystick()
{
  
  upval = !digitalRead(upPin) ;    
  downval = !digitalRead(downPin);    
  leftval = !digitalRead(leftPin) ;   
  rightval = !digitalRead(rightPin);   

  if (upval) {upIterationCount++;} else {upIterationCount=0;}
  if (downval) {downIterationCount++;}  else {downIterationCount=0;}
  if (leftval) {leftIterationCount++;}  else {leftIterationCount=0;}
  if (rightval) {rightIterationCount++;}  else {rightIterationCount=0;}

  if (upIterationCount > ITERATION_THRESHOLD) {upIterationCount = 0; MoveCursor(DIRECTION_UP); }
  if (downIterationCount > ITERATION_THRESHOLD) {downIterationCount = 0; MoveCursor(DIRECTION_DOWN); }
  if (leftIterationCount > ITERATION_THRESHOLD) {leftIterationCount = 0; MoveCursor(DIRECTION_LEFT); }
  if (rightIterationCount > ITERATION_THRESHOLD) {rightIterationCount = 0; MoveCursor(DIRECTION_RIGHT); }


}


void loop()
{ 
  
  int test;

  cursordelay ++;
  ReadJoystick();  
 
  
  if (cursordelay > CURSORDELAY)
  {
    cursorstate = !cursorstate;
    cursordelay = 0;
  }
  
  for (int i=0;i<8;i++)
  {
    for (int j=0;j<8;j++)
    {
       if ((tilerow==3) && (i==7))
       {
        picture[2][i][j] = true;
       } 
       else if ((tilecol==3) && (j==7))
       {  
         picture[2][i][j] = true;
       }
       else
         picture[2][i][j] = !blnMaze[i+((tilerow-1)*7)][j+((tilecol-1)*7)];   
    }
    
  }  
   
  if ((tilerow == 3) && (tilecol == 3))
    picture[1][5][5] = true;
  
  else
    picture[1][5][5] = false;
  
  SendData();  
  
   
}

void MoveCursor(byte direction)
{
  
  if (direction == DIRECTION_UP)
  {
    if (cursorY != 0) 
    {
       if (!picture[2][cursorY-1][cursorX])
      { 
       cursorY--; //normal move
      }
      
    }
    else
    {
       if (tilerow > 1)
       {
         tilerow--;
         cursorY=7;
       }
    }

  }
  if (direction == DIRECTION_DOWN)
  {
    if (cursorY != 7)  
    {
      if (!picture[2][cursorY+1][cursorX])
      {
       cursorY++;
      }
    }
    else
    {
       if (tilerow < 3)
       {
         tilerow++;
         cursorY=0;
       }
    }    
  }
  if (direction == DIRECTION_LEFT)
  {
    if (cursorX != 0)
    {
      if (!picture[2][cursorY][cursorX-1])
      {
       cursorX--;
      }
    }
    else
    {
       if (tilecol > 1)
       {
         tilecol--;
         cursorX=7;
       }
    }    
  }
  if (direction == DIRECTION_RIGHT)
  {
    if (cursorX != 7)
    {
      if (!picture[2][cursorY][cursorX+1])
      {
        cursorX++;
        
      }
    }
    else
    {
       if (tilecol < 3)
       {
         tilecol++;
         cursorX=0;
       }
    }    
  }
  cursorstate = 1;
  cursordelay = 0;
  
}


void SendData()
{
  
   byte ThisRow;
  byte i = 0;
  byte color;
  byte gridrow;
  byte gridcol;
  
  for (color = 0; color < 3; color++)
  {
    i = 0;
    for (int gridrow = 7; gridrow >= 0; gridrow--)
    {
      ThisRow = 0x00;    
      for (int gridcol = 7; gridcol >= 0; gridcol--)
      {
        if (gridcol != 7)
          ThisRow = (byte)(ThisRow << 1);                        

          ThisRow |= ((picture[color][gridrow][gridcol])?0x01:0x00);

        if ((!cursorstate) && (gridcol == cursorX) && (gridrow == cursorY))
        {
          ThisRow = ThisRow >> 1;
          ThisRow = ThisRow << 1;   
        }
        
        //overlay cursor
        if (cursorstate)
        { //if cursor state is ON, and we have a normally unpainted pixel
          //we want to show a default red cursor
            if ((cursorX == gridcol) && 
                (cursorY == gridrow) && 
                (color == 0) && 
                (picture[0][gridrow][gridcol] == 0)  && 
                (picture[1][gridrow][gridcol] == 0) && 
                (picture[2][gridrow][gridcol] == 0) )
               ThisRow |= 0x01;
        }   
        
                
        
        
      }
      argument_arrays[color][i] = ThisRow;
      i++;
    }                
  }
  
   ShowCustom(4,argument_arrays[0][0],argument_arrays[0][1],argument_arrays[0][2],argument_arrays[0][3],argument_arrays[0][4],argument_arrays[0][5],argument_arrays[0][6],argument_arrays[0][7],
              argument_arrays[1][0],argument_arrays[1][1],argument_arrays[1][2],argument_arrays[1][3],argument_arrays[1][4],argument_arrays[1][5],argument_arrays[1][6],argument_arrays[1][7],
              argument_arrays[2][0],argument_arrays[2][1],argument_arrays[2][2],argument_arrays[2][3],argument_arrays[2][4],argument_arrays[2][5],argument_arrays[2][6],argument_arrays[2][7]);
 
}


void ShowCustom(int Address, unsigned char R1, unsigned char R2, unsigned char R3, unsigned char R4, unsigned char R5, unsigned char R6, unsigned char R7, unsigned char R8,
unsigned char G1, unsigned char G2, unsigned char G3, unsigned char G4, unsigned char G5, unsigned char G6, unsigned char G7, unsigned char G8,
unsigned char Bl1, unsigned char Bl2, unsigned char Bl3, unsigned char Bl4, unsigned char Bl5, unsigned char Bl6, unsigned char Bl7, unsigned char Bl8)
{
  
  RainbowCMD[0]='R';
  RainbowCMD[1]=0x04;
  RainbowCMD[2]=0x04;
  RainbowCMD[3]=0x04;
  RainbowCMD[4]=0x04;
  RainbowCMD[5]=R1;
  RainbowCMD[6]=R2;
  RainbowCMD[7]=R3;
  RainbowCMD[8]=R4;
  RainbowCMD[9]=R5;
  RainbowCMD[10]=R6;
  RainbowCMD[11]=R7;
  RainbowCMD[12]=R8;
  
  RainbowCMD[13]=G1;
  RainbowCMD[14]=G2;
  RainbowCMD[15]=G3;
  RainbowCMD[16]=G4;
  RainbowCMD[17]=G5;
  RainbowCMD[18]=G6;
  RainbowCMD[19]=G7;
  RainbowCMD[20]=G8;
  
  
  RainbowCMD[21]=Bl1;
  RainbowCMD[22]=Bl2;
  RainbowCMD[23]=Bl3;
  RainbowCMD[24]=Bl4;
  RainbowCMD[25]=Bl5;
  RainbowCMD[26]=Bl6;
  RainbowCMD[27]=Bl7;
  RainbowCMD[28]=Bl8;
  

   SentCMD(Address);
  
}


//--------------------------------------------------------------------------
//Name:SentCMD
//function: Send a 6 byet Rainbow conmand out 
//parameter: NC  
//----------------------------------------------------------------------------
void SentCMD(int  Add)
{   unsigned char OK=0;
     unsigned char i,temp;
 
    while(!OK)
  {                          
    switch (State)
    { 	

    case 0:                          
      Wire.beginTransmission(Add);
      for (i=0;i<29;i++) Wire.send(RainbowCMD[i]);
      Wire.endTransmission();    
      
      //int i;
      /*int ledPin;
      ledPin = 7;
      for (i=0;i<8;i++)
      {
        if ((RainbowCMD[28]<<i)&0x80)
        {
         digitalWrite(ledPin, HIGH);   // sets the LED on
         delay(500);
         digitalWrite(ledPin, LOW);   // sets the LED on         
       }
       else       
       {
         digitalWrite(ledPin, HIGH);   // sets the LED on
         delay(100);
         digitalWrite(ledPin, LOW);   // sets the LED on
         delay(400);
       } 
       delay(100);
      }
      */
      
      delay(5);   
      State=1;                      
      break;

    case 1:

      Wire.requestFrom(Add,1);   
      if (Wire.available()>0) 
        temp=Wire.receive();    
      else {
        temp =0xFF;
        timeout++;
      }

      if ((temp==1)||(temp==2)) State=2;
      else if (temp==0) State=0;

      if (timeout>5000)
      {
        timeout=0;
        State=0;
      }

      delay(10);
      break;

    case 2:
      OK=1;
      State=0;
      break;

    default:
      State=0;
      break;

    }
  }
}

void GenerateMaze()
{
  int cNX = 0; int cNY = 0;
  int cSX = 0; int cSY = 0;
  int intDir = 0;
  int intDone = 0;
  boolean painted;

  boolean blnBlocked = false;
  do
  {

    do {  
      cSX = (int)random(1,(MAZE_X)); 
    } 
    while (cSX % 2 == 0); 
    do {  
      cSY = (int)random(1,(MAZE_Y)); 
    } 
    while (cSY % 2 == 0); 

    if (intDone == 0)  {  
      blnMaze[cSX][cSY] = true;   
    }

    if (blnMaze[cSX][cSY])
    {
      do
      {
        RandomDirections();
        blnBlocked = true;
        painted = false;

        for(intDir = 0; (intDir < 4); intDir++)
        {
          if (!painted)
          {
            if (intDir == 0)
            { 
              cNX = cSX + (cDir0X * 2);
              cNY = cSY + (cDir0Y * 2);
            }
            if (intDir == 1)
            { 
              cNX = cSX + (cDir1X * 2);
              cNY = cSY + (cDir1Y * 2);
            }
            if (intDir == 2)
            { 
              cNX = cSX + (cDir2X * 2);
              cNY = cSY + (cDir2Y * 2);
            }
            if (intDir == 3)
            { 
              cNX = cSX + (cDir3X * 2);
              cNY = cSY + (cDir3Y * 2);
            }
            if (isfree(cNX, cNY))
            {
              blnMaze[cNX][cNY]  = true;
              if (intDir == 0 ) { blnMaze[cSX + cDir0X][cSY + cDir0Y] = true; }
              if (intDir == 1 ) { blnMaze[cSX + cDir1X][cSY + cDir1Y] = true; }
              if (intDir == 2 ) { blnMaze[cSX + cDir2X][cSY + cDir2Y] = true; }
              if (intDir == 3 ) { blnMaze[cSX + cDir3X][cSY + cDir3Y] = true; }              

              cSX = cNX;
              cSY = cNY;
              blnBlocked = false;
              intDone++; 
              painted = true;
            }
          }
        }                               
      } 
      while(!blnBlocked);            
    }

  } 
  while (  ((intDone + 1) < (((MAZE_X - 1  ) * (MAZE_Y -1 )) / 4)));

}	



void RandomDirections()
{
  //randomSeed(analogRead(0));

  int randdirection;
  randdirection = (int)(random(0,4)); // 0,1,2,3
  cDir0X = 0;
  cDir0Y = 0;
  cDir1X = 0;
  cDir1Y = 0;
  cDir2X = 0;
  cDir2Y = 0;
  cDir3X = 0;
  cDir3Y = 0;
  if (randdirection == 0)
  {
    cDir0X = -1;
    cDir1X = 1;	
    cDir2Y = -1;	
    cDir3Y = 1;	
  }
  if (randdirection == 1)
  {
    cDir3X = -1;
    cDir2X = 1;	
    cDir1Y = -1;	
    cDir0Y = 1;	
  }
  if (randdirection == 2)
  {
    cDir2X = -1;
    cDir3X = 1;	
    cDir0Y = -1;	
    cDir1Y = 1;	
  }
  if (randdirection == 3)
  {
    cDir1X = -1;
    cDir0X = 1;	
    cDir3Y = -1;	
    cDir2Y = 1;	
  }
}

boolean isfree(int xpos, int ypos)
{
  if ((xpos < MAZE_X -1) && (xpos >= 1) && (ypos < MAZE_Y -1) && (ypos >= 1))
  {
    return (!blnMaze[xpos][ypos]);
  }
  return false;
}



int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

