/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *      USB auto module loader
 *
 *                          Version 0.4.0
 *                          Shift-JIS
 *
 *      Copyright (C) 2001 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 *                             usbmload.h
 *
 *        Version       Date            Design     Log
 *  --------------------------------------------------------------------
 *        0.3.1         Sep,27,2000     fukunaga   First release version
 *        0.3.3         Jan,10,2001     fukunaga   
 *        0.4.0         July,4,2001     fukunaga   Unload
 */

/*********************************************************
                         Macros
**********************************************************/
#define USBML_OK 0
#define USBML_NG -1

#define WILDCARD_INT -1
#define NOT_LOADED   0
#define LOADED       1
#define CAT_INACTIVE 0
#define CAT_ACTIVE   1

#define TESTLOAD_MASK (1<<2)
#define TESTLOAD_OK   (1<<2)
#define TESTLOAD_NG   0

#define USBML_ON   1
#define USBML_OFF  0


/*********************************************************
                           Type 
**********************************************************/

/* デバイスIDとファイル名の関係を表すリストの要素 */

#define NUM_OF_ARGV 8   

typedef struct _USBDEV_t {
  struct _USBDEV_t* forw;  /* for list structure */
  char* dispname;	    /* display name  */
  int vendor;              /* Device vendor */
  int product;             /* Device product */
  int release;             /* Device USB spec release */
  int class;               /* Interface class */
  int subclass;            /* Interface subclass */
  int protocol;            /* Interface protocol */
  char* category;          /* Category */
  char* path;              /* driver file path */
  char* argv[NUM_OF_ARGV]; /* driver parameters */
  int argc;                /* number of parameters */
  char activate_flag;      /* Activate ON/OFF */
  /* Information of Loaded Module */
  int modid;               /* Loaded Module ID */
  char modname[56];        /* Loaded Module Name */
  int load_result;         /* Result of LoadStartModule() */
} USBDEV_t;


/*********************************************************
                         Prototype
**********************************************************/
int sceUsbmlDisable(void);
int sceUsbmlEnable(void);
int sceUsbmlActivateCategory(const char* category);
int sceUsbmlInactivateCategory(const char* category);

typedef USBDEV_t* (*sceUsbmlPopDevinfo)(void);
typedef void (*sceUsbmlLoadFunc)(sceUsbmlPopDevinfo pop_devinfo);
int sceUsbmlRegisterLoadFunc(sceUsbmlLoadFunc loadfunc);

void sceUsbmlUnregisterLoadFunc(void);
int sceUsbmlLoadConffile(const char* conffile);
int sceUsbmlRegisterDevice(USBDEV_t* device);
int sceUsbmlChangeThreadPriority(int prio1);
