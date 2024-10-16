/*
gpiCallback.h
GameSpy Presence SDK 
Dan "Mr. Pants" Schoenblum

Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

***********************************************************************
Please see the GameSpy Presence SDK documentation for more information
**********************************************************************/

#ifndef _GPICALLBACK_H_
#define _GPICALLBACK_H_

//INCLUDES
//////////
#include "gpi.h"

//DEFINES
/////////
// Unsolicited Callbacks.
/////////////////////////
#define GPI_ERROR                      0
#define GPI_RECV_BUDDY_REQUEST         1
#define GPI_RECV_BUDDY_STATUS          2
#define GPI_RECV_BUDDY_MESSAGE         3
#define GPI_RECV_GAME_INVITE           4
#define GPI_TRANSFER_CALLBACK          5
#define GPI_NUM_CALLBACKS              6

// Add type - not 0 only for a few.
///////////////////////////////////
#define GPI_ADD_NORMAL                 0
#define GPI_ADD_ERROR                  1
#define GPI_ADD_MESSAGE                2
#define GPI_ADD_NICKS                  3
#define GPI_ADD_PMATCH                 4
#define GPI_ADD_STATUS                 5
#define GPI_ADD_BUDDDYREQUEST          6
#define GPI_ADD_TRANSFER_CALLBACK      7
#define GPI_ADD_REVERSE_BUDDIES        8
#define GPI_ADD_SUGGESTED_UNIQUE       9

//TYPES
///////
// A Callback.
//////////////
typedef struct
{
  GPCallback callback;
  void * param;
} GPICallback;

// Data for a pending callback.
///////////////////////////////
typedef struct GPICallbackData
{
    GPICallback callback;
    void * arg;
    int type;
    int operationID;
    struct GPICallbackData * pnext;
} GPICallbackData;

//FUNCTIONS
///////////
void
gpiCallErrorCallback(
  GPConnection * connection,
  GPResult result,
  GPEnum fatal
);

typedef struct GPIOperation_s *GPIOperation_st;

GPResult
gpiAddCallback(
  GPConnection * connection,
  GPICallback callback,
  void * arg,
  GPIOperation_st operation,
  int type
);

GPResult
gpiProcessCallbacks(
  GPConnection * connection,
  int blockingOperationID
);

#endif
