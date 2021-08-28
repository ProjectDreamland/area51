/*
gpiProfile.h
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

#ifndef _GPIPROFILE_H_
#define _GPIPROFILE_H_

//INCLUDES
//////////
#include "gpi.h"

//DEFINES
/////////
#define GPI_SIG_LEN      33

//TYPES
///////
// The status for a buddy profile.
//////////////////////////////////
typedef struct
{
  int buddyIndex;
  GPEnum status;
  char * statusString;
  char * locationString;
  unsigned int ip;
  int port;
} GPIBuddyStatus;

// Profile data.
////////////////
typedef struct GPIProfile
{
  int profileId;
  int userId;
  GPIBuddyStatus * buddyStatus;
  GPIInfoCache * cache;
  char * authSig;
  int requestCount;
  char * peerSig;
} GPIProfile;

// A list of profiles.
//////////////////////
typedef struct
{
  HashTable profileTable;
  int num;
  int numBuddies;
} GPIProfileList;

//FUNCTIONS
///////////
GPIBool
gpiInitProfiles(
  GPConnection * connection
);

GPIProfile *
gpiProfileListAdd(
  GPConnection * connection,
  int id
);

GPIBool
gpiGetProfile(
  GPConnection * connection,
  GPProfile profileid,
  GPIProfile ** pProfile
);

GPResult
gpiProcessNewProfile(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
);

GPResult
gpiNewProfile(
  GPConnection * connection,
  const char nick[GP_NICK_LEN],
  GPEnum replace,
  GPEnum blocking,
  GPCallback callback,
  void * param
);

GPResult
gpiDeleteProfile(
  GPConnection * connection
);

void
gpiRemoveProfile(
  GPConnection * connection,
  GPIProfile * profile
);

void
gpiRemoveProfileByID(
  GPConnection * connection,
  int profileid
);

GPResult
gpiLoadDiskProfiles(
  GPConnection * connection
);

GPResult
gpiSaveDiskProfiles(
  GPConnection * connection
);

GPResult
gpiFindProfileByUser(
  GPConnection * connection,
  char nick[GP_NICK_LEN],
  char email[GP_EMAIL_LEN],
  GPIProfile ** profile
);

// return false to stop the mapping
typedef GPIBool
(* gpiProfileMapFunc)(
  GPConnection * connection,
  GPIProfile * profile,
  void * data
);

GPIBool
gpiProfileMap(
  GPConnection * connection,
  gpiProfileMapFunc func,
  void * data
);

GPIProfile *
gpiFindBuddy(
  GPConnection * connection,
  int buddyIndex
);

GPIBool
gpiCanFreeProfile(
  GPIProfile * profile
);

void gpiSetInfoCacheFilename(
  const char filename[FILENAME_MAX + 1]
);

#endif
