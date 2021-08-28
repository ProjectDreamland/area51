/*
gpiSearch.h
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

#ifndef _GPISEARCH_H_
#define _GPISEARCH_H_

//INCLUDES
//////////
#include "gpi.h"

//TYPES
///////
#define GPI_SEARCH_PROFILE         1
#define GPI_SEARCH_IS_VALID        2
#define GPI_SEARCH_NICKS           3
#define GPI_SEARCH_PLAYERS         4
#define GPI_SEARCH_CHECK           5
#define GPI_SEARCH_NEWUSER         6
#define GPI_SEARCH_OTHERS_BUDDY    7
#define GPI_SEARCH_SUGGEST_UNIQUE  8

// Profile Search operation data.
/////////////////////////////////
typedef struct
{
	int type;
	SOCKET sock;
	GPIBuffer inputBuffer;
	GPIBuffer outputBuffer;
	char nick[GP_NICK_LEN];
	char uniquenick[GP_UNIQUENICK_LEN];
	char email[GP_EMAIL_LEN];
	char firstname[GP_FIRSTNAME_LEN];
	char lastname[GP_LASTNAME_LEN];
	char password[GP_PASSWORD_LEN];
	char cdkey[GP_CDKEY_LEN];
	int icquin;
	int skip;
	int productID;
	GPIBool processing;
	GPIBool remove;
} GPISearchData;

//FUNCTIONS
///////////
GPResult
gpiProfileSearch(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char firstname[GP_FIRSTNAME_LEN],
  const char lastname[GP_LASTNAME_LEN],
  int icquin,
  int skip,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiIsValidEmail(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiGetUserNicks(
  GPConnection * connection,
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiFindPlayers(
  GPConnection * connection,
  int productID,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiCheckUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiNewUser(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char email[GP_EMAIL_LEN],
  const char password[GP_PASSWORD_LEN],
  const char cdkey[GP_CDKEY_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiOthersBuddy(
  GPConnection * connection,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult gpiSuggestUniqueNick(
  GPConnection * connection,
  const char desirednick[GP_NICK_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiProcessSearches(
  GPConnection * connection
);

#endif
