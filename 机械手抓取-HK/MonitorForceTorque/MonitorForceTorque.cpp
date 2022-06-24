// MonitorForceTorque.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
// MonitorForceTorque.cpp : Defines the entry point for the console application.
//


#include <stdio.h> 
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h> 
#include "Pcan_usb.h"

#define FTSENSORID (8)
#define FTDATAPROPERTY (54)

HANDLE	hConsoleInput, hConsoleOutput;
COORD	myCoord;

typedef struct {
   
		double forceX;
		double forceY;
		double forceZ;
		double torqueX;
		double torqueY;
		double torqueZ;

} ftsstr;

ftsstr ftdata;
unsigned char errorByte;

int getMessage(TPCANMsg *msgIn){
	DWORD err;
	int i;

	while(err = CAN_Read( msgIn )){
		if(err == CAN_ERR_QRCVEMPTY){
			//printf("ERROR - CAN_Read(): QRCVEMPTY\n");
		}else{
			printf("ERROR - CAN_Read(): %ld\n", err);
			return(1);
		}
	}

	if(msgIn->MSGTYPE == MSGTYPE_STATUS){
		// Handle status message
		printf("Got status message, DATA[3] = 0x%.4x\n", msgIn->DATA[3]);
		return(1);
	}

	if(msgIn->MSGTYPE == MSGTYPE_STANDARD){ // Got standard message
		// Process the message
#if 0
		printf("Response: ID=0x%.3x LEN=%d DATA=[", msgIn->ID, msgIn->LEN);
		
		for(i = 0; i < msgIn->LEN; i++){
			printf(" %.2x", msgIn->DATA[i]);
		}
		printf(" ]\n");
#endif
		return(0); // Success
		
	}

	return(1); // Unknown message
}


//can bus
// The first step in accessing the serial port is setting up a file handle.
DWORD initComm(){
	DWORD err;

	// Initialize the CAN hardware
	err = CAN_Init(CAN_BAUD_1M, 0); // Set rate = 1 MBaud, type = 11-bit
	if(err == CAN_ERR_OK) {
		printf("SUCCESS - CAN_Init()\n");
	}else{
		printf("ERROR - CAN_Init(): %ld\n", err);
		return(err);
	}

	// Clear the TX/RX queues
	err = CAN_ResetClient();
	if(err == CAN_ERR_OK){
		printf("SUCCESS - CAN_ResetClient()\n");
	}else{
		printf("ERROR - CAN_ResetClient(): %ld\n", err);
		return(err);
	}

	// Set filter to receive all standard messages (from 0 to 0x3FF)
	err = CAN_MsgFilter(0, 0x3FF, MSGTYPE_STANDARD); 
	if(err == CAN_ERR_OK){
		printf("SUCCESS - CAN_MsgFilter()\n");
	}else{
		printf("ERROR - CAN_MsgFilter(): %ld\n", err);
		return(err);
	}

	return(0);
}


//now the data 

int wakePuck(int bus, int id){
	TPCANMsg msgOut;
	DWORD err;

	// Generate the outbound message
	msgOut.ID = id;
	msgOut.MSGTYPE = MSGTYPE_STANDARD;
	msgOut.LEN = 4;
	msgOut.DATA[0] = 0x85; // Set Status
	msgOut.DATA[1] = 0x00; 
	msgOut.DATA[2] = 0x02; // Status = Ready
	msgOut.DATA[3] = 0x00; 
	
	// Send the message
	err = CAN_Write( &msgOut );
	Sleep(1000);

	return(err);
}

int getPropertyFT(int bus)

{


	TPCANMsg msgOut, msgInOne, msgInTwo;
	DWORD err;
	

	// Generate the outbound message
	msgOut.ID = FTSENSORID;
	msgOut.MSGTYPE = MSGTYPE_STANDARD;
	msgOut.LEN = 1;
	msgOut.DATA[0] = FTDATAPROPERTY; // Get the ft data
	
	// Send the message
	CAN_Write( &msgOut );

	// Read the response
	do {err = getMessage( &msgInOne );} while(msgInOne.ID != 0x50A);
	
	do {err = getMessage( &msgInTwo );} while(msgInTwo.ID != 0x50B);

	if(err){
		return(1);
	}else{
			//parse the message
			
			/* CAN Message

    DWORD ID;        // 11/29 bit identifier
    BYTE  MSGTYPE;   // Bits from MSGTYPE_*
    BYTE  LEN;       // Data Length Code of the Msg (0..8)
    BYTE  DATA[8];   // Data 0 .. 7
	 TPCANMsg;  */
	
		ftdata.forceX = (((int)(*(char*)&msgInOne.DATA[1])<<8) | (msgInOne.DATA[0]))/256.0;
		ftdata.forceY = (((int)(*(char*)&msgInOne.DATA[3])<<8) | (msgInOne.DATA[2]))/256.0;
		ftdata.forceZ = (((int)(*(char*)&msgInOne.DATA[5])<<8) | (msgInOne.DATA[4]))/256.0;

		ftdata.torqueX = (((int)(*(char*)&msgInTwo.DATA[1])<<8) | (msgInTwo.DATA[0]))/4096.0;
		ftdata.torqueY = (((int)(*(char*)&msgInTwo.DATA[3])<<8) | (msgInTwo.DATA[2]))/4096.0;
		ftdata.torqueZ = (((int)(*(char*)&msgInTwo.DATA[5])<<8) | (msgInTwo.DATA[4]))/4096.0;

		if(msgInTwo.LEN == 7) errorByte = msgInTwo.DATA[6];
		else errorByte = 0;

		}
	return (0);

}



//used to set and get prop

int compile(
   int property        /** The property being compiled (use the enumerations in btcan.h) */,
   long longVal        /** The value to set the property to */,
   unsigned char *data /** A pointer to a character buffer in which to build the data payload */,
   int *dataLen        /** A pointer to the total length of the data payload for this packet */,
   int isForSafety        /** A flag indicating whether this packet is destined for the safety circuit or not */)
{
   int i;

   // Check the property
   //if(property > PROP_END)
   //{
   //   syslog(LOG_ERR,"compile(): Invalid property = %d", property);
   //   return(1);
   //}

   /* Insert the property */
   data[0] = property;
   data[1] = 0; /* To align the values for the tater's DSP */

   /* Append the value */
   for (i = 2; i < 6; i++)
   {
      data[i] = (char)(longVal & 0x000000FF);
      longVal >>= 8;
   }

   /* Record the proper data length */
   *dataLen = 6; //(dataType[property] & 0x0007) + 2;

   return (0);
}

void usleep(long us){
	Sleep(us/1000);
}


void ClearScreen(void) //Taken from the Visual C++ Help file...
{ 
  COORD coordScreen = { 0, 0 }; // here's where we'll home the cursor 
  BOOL bSuccess; 
  DWORD cCharsWritten; 
  CONSOLE_SCREEN_BUFFER_INFO csbi; // to get buffer info 
  DWORD dwConSize; // number of character cells in the current buffer 
 
  hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

  //get the number of character cells in the current buffer 
  bSuccess = GetConsoleScreenBufferInfo(hConsoleOutput, &csbi); 
   
  dwConSize = csbi.dwSize.X * csbi.dwSize.Y; 
  //fill the entire screen with blanks 
  bSuccess = FillConsoleOutputCharacter(hConsoleOutput, (TCHAR) ' ', 
      dwConSize, coordScreen, &cCharsWritten); 
  
  //get the current text attribute 
  bSuccess = GetConsoleScreenBufferInfo(hConsoleOutput, &csbi); 
  
  //now set the buffer's attributes accordingly 
  bSuccess = FillConsoleOutputAttribute(hConsoleOutput, csbi.wAttributes, 
      dwConSize, coordScreen, &cCharsWritten); 
  
  //put the cursor at (0, 0)
  bSuccess = SetConsoleCursorPosition(hConsoleOutput, coordScreen); 


  return; 
} 





int setPropertySlow(int bus, int id, int property, int verify, long value)
{
	TPCANMsg msgOut, msgIn;
	DWORD err;
	int dataHeader, i;
   long            response;
   //unsigned char   data[8];
   int             len;

   //syslog(LOG_ERR, "About to compile setProperty, property = %d", property);
   // Compile 'set' packet
   err = compile(property, value, msgOut.DATA, &len, 0);

	// Generate the outbound message
	msgOut.ID = id;
	msgOut.MSGTYPE = MSGTYPE_STANDARD;
	msgOut.LEN = len;
	
	

   //syslog(LOG_ERR, "After compilation data[0] = %d", data[0]);
   msgOut.DATA[0] |= 0x80; // Set the 'Set' bit

   // Send the message
	CAN_Write( &msgOut );

	//Sleep(250);

   // BUG: This will not verify properties from groups of pucks
   
   return(0);
}

int main(void){
	
	char			myChar = 0;
	unsigned long	isShifted;
	INPUT_RECORD	myRec;
	DWORD			actualRead;
	DWORD count;

	hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

	
	//print

	printf("Barrett Technology Inc.\n\n");
	printf("Monitor Force Torque Sensor Program v1.0\n\n\n");
	printf("Press 't' at any time to tare the sensor.\n");
	printf("Press 'q' at any time to quit.\n\n\n");
	int err;
	err = initComm();
	
	//wake puck
	wakePuck(0,8);
	
	//tare
	setPropertySlow(0,FTSENSORID,FTDATAPROPERTY,0,0);
		
		printf("\n\n\n  Force X |  Force Y |  Force Z | Torque X | Torque Y | Torque Z | Error\n");

	while(1)
		{
			getPropertyFT(0);
			printf("\r %8.4lf | %8.4lf | %8.4lf | %8.4lf | %8.4lf | %8.4lf | %2.2x  ", ftdata.forceX, ftdata.forceY, ftdata.forceZ, ftdata.torqueX, ftdata.torqueY, ftdata.torqueZ, errorByte);
			usleep(100000);
			
			
			GetNumberOfConsoleInputEvents(hConsoleInput, &count);
			
			if (count)

			{

			ReadConsoleInput(hConsoleInput,&myRec,1,&actualRead);
			if((myRec.EventType == KEY_EVENT) && (myRec.Event.KeyEvent.bKeyDown))
				{
					myChar = myRec.Event.KeyEvent.uChar.AsciiChar;
					if(myChar == 't')  //tare
						{
							setPropertySlow(0,FTSENSORID,FTDATAPROPERTY,0,0);
						}
					if(myChar == 'q') //quit 
						{
							printf("\n");
							break;
						}			
				}
			}
	}
	

//CloseHandle(port);
	return(0);
}