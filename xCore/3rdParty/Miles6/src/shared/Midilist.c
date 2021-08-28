//###########################################################################
//##                                                                       ##
//##   MLIST.C                                                             ##
//##                                                                       ##
//##   Standard MIDI file dump utility                                     ##
//##                                                                       ##
//##   V1.00 of 17-Mar-91                                                  ##
//##   V1.01 of 07-Sep-91: New sysex dump formatting                       ##
//##   V1.10 of 23-Oct-91: Roland sysex disassembly/                       ##
//##                       Programmer's Reference features                 ##
//##   V1.11 of  3-Feb-92: BC 3.0 const declarations fixed                 ##
//##   V1.12 of 20-Jun-92: Undefined meta-events handled correctly         ##
//##                                                                       ##
//##   Project: Miles Sound System                                         ##
//##    Author: John Miles                                                 ##
//##                                                                       ##
//###########################################################################
//##                                                                       ##
//##  Copyright (C) RAD Game Tools, Inc.                                   ##
//##                                                                       ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.       ##
//##                                                                       ##
//###########################################################################

#include "mss.h"

#include "imssapi.h"

static char const FAR *buffer;
static char const FAR *bufpnt;
static char const FAR *bufend;
static U32 offset;

static U32 arg_mt;

static U8 sysex_buffer[256];

//################################################################
#define TIMBRE 0
#define PATCH_TEMP 1
#define RHYTHMS 2
#define PATCH 3
#define SYSTEM 4
#define DISPLAY 5
#define WRITE_REQUEST 6
#define ALL_PARMS_RESET 7

#define T_A 0
#define T_N 1
#define T_C 2
#define T_NONE 3


static HMEMDUMP memout;
static U32 last_chunk = 0;

typedef struct
{
   char area_name[128];
   char parm_name[128];
   U16 type;
}
addr_desc;


typedef struct
{
   U16 MSB;
   U16 KSB;
   U16 LSB;
}
sysex_addr;

typedef struct
{
   char *name;
   U16 type;
}
entry;

typedef struct
{
   char *name;
   sysex_addr base;
   U16 base_size;
   U16 base_num;
   U16 group_count;
   const entry *items;
}
Roland_area;

const entry patch_temp[] =
{
   {"TIMBRE GROUP",T_N},
   {"TIMBRE NUMBER",T_N},
   {"KEY SHIFT",T_N},
   {"FINE TUNE",T_N},
   {"BENDER RANGE",T_N},
   {"ASSIGN MODE",T_N},
   {"REVERB SWITCH",T_N},
   {"dummy (ignored if received) (offset $00 $07)",T_N},
   {"OUTPUT LEVEL",T_N},
   {"PANPOT",T_N},
   {"dummy (ignored if received) (offset $00 $0A)",T_N},
   {"dummy (ignored if received) (offset $00 $0B)",T_N},
   {"dummy (ignored if received) (offset $00 $0C)",T_N},
   {"dummy (ignored if received) (offset $00 $0D)",T_N},
   {"dummy (ignored if received) (offset $00 $0E)",T_N},
   {"dummy (ignored if received) (offset $00 $0F)",T_N}
};

const entry patch[] =
{
   {"TIMBRE GROUP",T_N},
   {"TIMBRE NUMBER",T_N},
   {"KEY SHIFT",T_N},
   {"FINE TUNE",T_N},
   {"BENDER RANGE",T_N},
   {"ASSIGN MODE",T_N},
   {"REVERB SWITCH",T_N},
   {"dummy (offset $00 $07)",T_N}
};

const entry rhythm_setup[] =
{
   {"TIMBRE",T_N},
   {"OUTPUT LEVEL",T_N},
   {"PANPOT",T_N},
   {"REVERB SWITCH",T_N}
};

const entry system_area[] =
{
   {"MASTER TUNE",T_N},
   {"REVERB MODE",T_N},
   {"REVERB TIME",T_N},
   {"REVERB LEVEL",T_N},
   {"PARTIAL RESERVE (Part 1)",T_N},
   {"PARTIAL RESERVE (Part 2)",T_N},
   {"PARTIAL RESERVE (Part 3)",T_N},
   {"PARTIAL RESERVE (Part 4)",T_N},
   {"PARTIAL RESERVE (Part 5)",T_N},
   {"PARTIAL RESERVE (Part 6)",T_N},
   {"PARTIAL RESERVE (Part 7)",T_N},
   {"PARTIAL RESERVE (Part 8)",T_N},
   {"PARTIAL RESERVE (Part R)",T_N},
   {"MIDI CHANNEL (Part 1)",T_C},
   {"MIDI CHANNEL (Part 2)",T_C},
   {"MIDI CHANNEL (Part 3)",T_C},
   {"MIDI CHANNEL (Part 4)",T_C},
   {"MIDI CHANNEL (Part 5)",T_C},
   {"MIDI CHANNEL (Part 6)",T_C},
   {"MIDI CHANNEL (Part 7)",T_C},
   {"MIDI CHANNEL (Part 8)",T_C},
   {"MIDI CHANNEL (Part R)",T_C},
   {"MASTER VOLUME",T_N}
};

const entry display[] =
{
   {"MT-32 LCD Display (character 1)",T_A},
   {"MT-32 LCD Display (character 2)",T_A},
   {"MT-32 LCD Display (character 3)",T_A},
   {"MT-32 LCD Display (character 4)",T_A},
   {"MT-32 LCD Display (character 5)",T_A},
   {"MT-32 LCD Display (character 6)",T_A},
   {"MT-32 LCD Display (character 7)",T_A},
   {"MT-32 LCD Display (character 8)",T_A},
   {"MT-32 LCD Display (character 9)",T_A},
   {"MT-32 LCD Display (character 10)",T_A},
   {"MT-32 LCD Display (character 11)",T_A},
   {"MT-32 LCD Display (character 12)",T_A},
   {"MT-32 LCD Display (character 13)",T_A},
   {"MT-32 LCD Display (character 14)",T_A},
   {"MT-32 LCD Display (character 15)",T_A},
   {"MT-32 LCD Display (character 16)",T_A},
   {"MT-32 LCD Display (character 17)",T_A},
   {"MT-32 LCD Display (character 18)",T_A},
   {"MT-32 LCD Display (character 19)",T_A},
   {"MT-32 LCD Display (character 20)",T_A}
};

const entry write_request[] =
{
   {"Timbre Write (part 1)",T_N},
   {"(Internal) (Offset $01)",T_N},
   {"Timbre Write (part 2)",T_N},
   {"(Internal) (Offset $03)",T_N},
   {"Timbre Write (part 3)",T_N},
   {"(Internal) (Offset $05)",T_N},
   {"Timbre Write (part 4)",T_N},
   {"(Internal) (Offset $07)",T_N},
   {"Timbre Write (part 5)",T_N},
   {"(Internal) (Offset $09)",T_N},
   {"Timbre Write (part 6)",T_N},
   {"(Internal) (Offset $0B)",T_N},
   {"Timbre Write (part 7)",T_N},
   {"(Internal) (Offset $0D)",T_N},
   {"Timbre Write (part 8)",T_N},
   {"(Internal) (Offset $0F)",T_N},
   {"Patch Write (part 1)",T_N},
   {"(Internal) (Offset $01)",T_N},
   {"Patch Write (part 2)",T_N},
   {"(Internal) (Offset $03)",T_N},
   {"Patch Write (part 3)",T_N},
   {"(Internal) (Offset $05)",T_N},
   {"Patch Write (part 4)",T_N},
   {"(Internal) (Offset $07)",T_N},
   {"Patch Write (part 5)",T_N},
   {"(Internal) (Offset $09)",T_N},
   {"Patch Write (part 6)",T_N},
   {"(Internal) (Offset $0B)",T_N},
   {"Patch Write (part 7)",T_N},
   {"(Internal) (Offset $0D)",T_N},
   {"Patch Write (part 8)",T_N},
   {"(Internal) (Offset $0F)",T_N},
};

const entry timbre[] =
{
   {"TIMBRE NAME 1",T_A},
   {"TIMBRE NAME 2",T_A},
   {"TIMBRE NAME 3",T_A},
   {"TIMBRE NAME 4",T_A},
   {"TIMBRE NAME 5",T_A},
   {"TIMBRE NAME 6",T_A},
   {"TIMBRE NAME 7",T_A},
   {"TIMBRE NAME 8",T_A},
   {"TIMBRE NAME 9",T_A},
   {"TIMBRE NAME 10",T_A},
   {"Structure of Partial # 1 & 2",T_N},
   {"Structure of Partial # 3 & 4",T_N},
   {"PARTIAL MUTE",T_N},
   {"ENV MODE",T_N},
   {"(Partial #1) WG PITCH COARSE",T_N},
   {"(Partial #1) WG PITCH FINE",T_N},
   {"(Partial #1) WG PITCH KEYFOLLOW",T_N},
   {"(Partial #1) WG PITCH BENDER SW",T_N},
   {"(Partial #1) WG WAVEFORM/PCM BANK",T_N},
   {"(Partial #1) WG PCM WAVE #",T_N},
   {"(Partial #1) WG PULSE WIDTH",T_N},
   {"(Partial #1) WG PW VELO SENS",T_N},
   {"(Partial #1) P-ENV DEPTH",T_N},
   {"(Partial #1) P-ENV VELO SENS",T_N},
   {"(Partial #1) P-ENV TIME KEYF",T_N},
   {"(Partial #1) P-ENV TIME 1",T_N},
   {"(Partial #1) P-ENV TIME 2",T_N},
   {"(Partial #1) P-ENV TIME 3",T_N},
   {"(Partial #1) P-ENV TIME 4",T_N},
   {"(Partial #1) P-ENV LEVEL 0",T_N},
   {"(Partial #1) P-ENV LEVEL 1",T_N},
   {"(Partial #1) P-ENV LEVEL 2",T_N},
   {"(Partial #1) P-ENV SUSTAIN LEVEL",T_N},
   {"(Partial #1) END LEVEL",T_N},
   {"(Partial #1) P-LFO RATE",T_N},
   {"(Partial #1) P-LFO DEPTH",T_N},
   {"(Partial #1) P-LFO MOD SENS",T_N},
   {"(Partial #1) TVF CUTOFF FREQ",T_N},
   {"(Partial #1) TVF RESONANCE",T_N},
   {"(Partial #1) TVF KEYFOLLOW",T_N},
   {"(Partial #1) TVF BIAS POINT/DIR",T_N},
   {"(Partial #1) TVF BIAS LEVEL",T_N},
   {"(Partial #1) TVF ENV DEPTH",T_N},
   {"(Partial #1) TVF ENV VELO SENS",T_N},
   {"(Partial #1) TVF ENV DEPTH KEYF",T_N},
   {"(Partial #1) TVF ENV TIME KEYF",T_N},
   {"(Partial #1) TVF ENV TIME 1",T_N},
   {"(Partial #1) TVF ENV TIME 2",T_N},
   {"(Partial #1) TVF ENV TIME 3",T_N},
   {"(Partial #1) TVF ENV TIME 4",T_N},
   {"(Partial #1) TVF ENV TIME 5",T_N},
   {"(Partial #1) TVF ENV LEVEL 1",T_N},
   {"(Partial #1) TVF ENV LEVEL 2",T_N},
   {"(Partial #1) TVF ENV LEVEL 3",T_N},
   {"(Partial #1) TVF ENV SUSTAIN LEVEL",T_N},
   {"(Partial #1) TVA LEVEL",T_N},
   {"(Partial #1) TVA VELO SENS",T_N},
   {"(Partial #1) TVA BIAS POINT 1",T_N},
   {"(Partial #1) TVA BIAS LEVEL 1",T_N},
   {"(Partial #1) TVA BIAS POINT 2",T_N},
   {"(Partial #1) TVA BIAS LEVEL 2",T_N},
   {"(Partial #1) TVA ENV TIME KEYF",T_N},
   {"(Partial #1) TVA ENV TIME V_FOLLOW",T_N},
   {"(Partial #1) TVA ENV TIME 1",T_N},
   {"(Partial #1) TVA ENV TIME 2",T_N},
   {"(Partial #1) TVA ENV TIME 3",T_N},
   {"(Partial #1) TVA ENV TIME 4",T_N},
   {"(Partial #1) TVA ENV TIME 5",T_N},
   {"(Partial #1) TVA ENV LEVEL 1",T_N},
   {"(Partial #1) TVA ENV LEVEL 2",T_N},
   {"(Partial #1) TVA ENV LEVEL 3",T_N},
   {"(Partial #1) TVA ENV SUSTAIN LEVEL",T_N},
   {"(Partial #2) WG PITCH COARSE",T_N},
   {"(Partial #2) WG PITCH FINE",T_N},
   {"(Partial #2) WG PITCH KEYFOLLOW",T_N},
   {"(Partial #2) WG PITCH BENDER SW",T_N},
   {"(Partial #2) WG WAVEFORM/PCM BANK",T_N},
   {"(Partial #2) WG PCM WAVE #",T_N},
   {"(Partial #2) WG PULSE WIDTH",T_N},
   {"(Partial #2) WG PW VELO SENS",T_N},
   {"(Partial #2) P-ENV DEPTH",T_N},
   {"(Partial #2) P-ENV VELO SENS",T_N},
   {"(Partial #2) P-ENV TIME KEYF",T_N},
   {"(Partial #2) P-ENV TIME 1",T_N},
   {"(Partial #2) P-ENV TIME 2",T_N},
   {"(Partial #2) P-ENV TIME 3",T_N},
   {"(Partial #2) P-ENV TIME 4",T_N},
   {"(Partial #2) P-ENV LEVEL 0",T_N},
   {"(Partial #2) P-ENV LEVEL 1",T_N},
   {"(Partial #2) P-ENV LEVEL 2",T_N},
   {"(Partial #2) P-ENV SUSTAIN LEVEL",T_N},
   {"(Partial #2) END LEVEL",T_N},
   {"(Partial #2) P-LFO RATE",T_N},
   {"(Partial #2) P-LFO DEPTH",T_N},
   {"(Partial #2) P-LFO MOD SENS",T_N},
   {"(Partial #2) TVF CUTOFF FREQ",T_N},
   {"(Partial #2) TVF RESONANCE",T_N},
   {"(Partial #2) TVF KEYFOLLOW",T_N},
   {"(Partial #2) TVF BIAS POINT/DIR",T_N},
   {"(Partial #2) TVF BIAS LEVEL",T_N},
   {"(Partial #2) TVF ENV DEPTH",T_N},
   {"(Partial #2) TVF ENV VELO SENS",T_N},
   {"(Partial #2) TVF ENV DEPTH KEYF",T_N},
   {"(Partial #2) TVF ENV TIME KEYF",T_N},
   {"(Partial #2) TVF ENV TIME 1",T_N},
   {"(Partial #2) TVF ENV TIME 2",T_N},
   {"(Partial #2) TVF ENV TIME 3",T_N},
   {"(Partial #2) TVF ENV TIME 4",T_N},
   {"(Partial #2) TVF ENV TIME 5",T_N},
   {"(Partial #2) TVF ENV LEVEL 1",T_N},
   {"(Partial #2) TVF ENV LEVEL 2",T_N},
   {"(Partial #2) TVF ENV LEVEL 3",T_N},
   {"(Partial #2) TVF ENV SUSTAIN LEVEL",T_N},
   {"(Partial #2) TVA LEVEL",T_N},
   {"(Partial #2) TVA VELO SENS",T_N},
   {"(Partial #2) TVA BIAS POINT 1",T_N},
   {"(Partial #2) TVA BIAS LEVEL 1",T_N},
   {"(Partial #2) TVA BIAS POINT 2",T_N},
   {"(Partial #2) TVA BIAS LEVEL 2",T_N},
   {"(Partial #2) TVA ENV TIME KEYF",T_N},
   {"(Partial #2) TVA ENV TIME V_FOLLOW",T_N},
   {"(Partial #2) TVA ENV TIME 1",T_N},
   {"(Partial #2) TVA ENV TIME 2",T_N},
   {"(Partial #2) TVA ENV TIME 3",T_N},
   {"(Partial #2) TVA ENV TIME 4",T_N},
   {"(Partial #2) TVA ENV TIME 5",T_N},
   {"(Partial #2) TVA ENV LEVEL 1",T_N},
   {"(Partial #2) TVA ENV LEVEL 2",T_N},
   {"(Partial #2) TVA ENV LEVEL 3",T_N},
   {"(Partial #2) TVA ENV SUSTAIN LEVEL",T_N},
   {"(Partial #3) WG PITCH COARSE",T_N},
   {"(Partial #3) WG PITCH FINE",T_N},
   {"(Partial #3) WG PITCH KEYFOLLOW",T_N},
   {"(Partial #3) WG PITCH BENDER SW",T_N},
   {"(Partial #3) WG WAVEFORM/PCM BANK",T_N},
   {"(Partial #3) WG PCM WAVE #",T_N},
   {"(Partial #3) WG PULSE WIDTH",T_N},
   {"(Partial #3) WG PW VELO SENS",T_N},
   {"(Partial #3) P-ENV DEPTH",T_N},
   {"(Partial #3) P-ENV VELO SENS",T_N},
   {"(Partial #3) P-ENV TIME KEYF",T_N},
   {"(Partial #3) P-ENV TIME 1",T_N},
   {"(Partial #3) P-ENV TIME 2",T_N},
   {"(Partial #3) P-ENV TIME 3",T_N},
   {"(Partial #3) P-ENV TIME 4",T_N},
   {"(Partial #3) P-ENV LEVEL 0",T_N},
   {"(Partial #3) P-ENV LEVEL 1",T_N},
   {"(Partial #3) P-ENV LEVEL 2",T_N},
   {"(Partial #3) P-ENV SUSTAIN LEVEL",T_N},
   {"(Partial #3) END LEVEL",T_N},
   {"(Partial #3) P-LFO RATE",T_N},
   {"(Partial #3) P-LFO DEPTH",T_N},
   {"(Partial #3) P-LFO MOD SENS",T_N},
   {"(Partial #3) TVF CUTOFF FREQ",T_N},
   {"(Partial #3) TVF RESONANCE",T_N},
   {"(Partial #3) TVF KEYFOLLOW",T_N},
   {"(Partial #3) TVF BIAS POINT/DIR",T_N},
   {"(Partial #3) TVF BIAS LEVEL",T_N},
   {"(Partial #3) TVF ENV DEPTH",T_N},
   {"(Partial #3) TVF ENV VELO SENS",T_N},
   {"(Partial #3) TVF ENV DEPTH KEYF",T_N},
   {"(Partial #3) TVF ENV TIME KEYF",T_N},
   {"(Partial #3) TVF ENV TIME 1",T_N},
   {"(Partial #3) TVF ENV TIME 2",T_N},
   {"(Partial #3) TVF ENV TIME 3",T_N},
   {"(Partial #3) TVF ENV TIME 4",T_N},
   {"(Partial #3) TVF ENV TIME 5",T_N},
   {"(Partial #3) TVF ENV LEVEL 1",T_N},
   {"(Partial #3) TVF ENV LEVEL 2",T_N},
   {"(Partial #3) TVF ENV LEVEL 3",T_N},
   {"(Partial #3) TVF ENV SUSTAIN LEVEL",T_N},
   {"(Partial #3) TVA LEVEL",T_N},
   {"(Partial #3) TVA VELO SENS",T_N},
   {"(Partial #3) TVA BIAS POINT 1",T_N},
   {"(Partial #3) TVA BIAS LEVEL 1",T_N},
   {"(Partial #3) TVA BIAS POINT 2",T_N},
   {"(Partial #3) TVA BIAS LEVEL 2",T_N},
   {"(Partial #3) TVA ENV TIME KEYF",T_N},
   {"(Partial #3) TVA ENV TIME V_FOLLOW",T_N},
   {"(Partial #3) TVA ENV TIME 1",T_N},
   {"(Partial #3) TVA ENV TIME 2",T_N},
   {"(Partial #3) TVA ENV TIME 3",T_N},
   {"(Partial #3) TVA ENV TIME 4",T_N},
   {"(Partial #3) TVA ENV TIME 5",T_N},
   {"(Partial #3) TVA ENV LEVEL 1",T_N},
   {"(Partial #3) TVA ENV LEVEL 2",T_N},
   {"(Partial #3) TVA ENV LEVEL 3",T_N},
   {"(Partial #3) TVA ENV SUSTAIN LEVEL",T_N},
   {"(Partial #4) WG PITCH COARSE",T_N},
   {"(Partial #4) WG PITCH FINE",T_N},
   {"(Partial #4) WG PITCH KEYFOLLOW",T_N},
   {"(Partial #4) WG PITCH BENDER SW",T_N},
   {"(Partial #4) WG WAVEFORM/PCM BANK",T_N},
   {"(Partial #4) WG PCM WAVE #",T_N},
   {"(Partial #4) WG PULSE WIDTH",T_N},
   {"(Partial #4) WG PW VELO SENS",T_N},
   {"(Partial #4) P-ENV DEPTH",T_N},
   {"(Partial #4) P-ENV VELO SENS",T_N},
   {"(Partial #4) P-ENV TIME KEYF",T_N},
   {"(Partial #4) P-ENV TIME 1",T_N},
   {"(Partial #4) P-ENV TIME 2",T_N},
   {"(Partial #4) P-ENV TIME 3",T_N},
   {"(Partial #4) P-ENV TIME 4",T_N},
   {"(Partial #4) P-ENV LEVEL 0",T_N},
   {"(Partial #4) P-ENV LEVEL 1",T_N},
   {"(Partial #4) P-ENV LEVEL 2",T_N},
   {"(Partial #4) P-ENV SUSTAIN LEVEL",T_N},
   {"(Partial #4) END LEVEL",T_N},
   {"(Partial #4) P-LFO RATE",T_N},
   {"(Partial #4) P-LFO DEPTH",T_N},
   {"(Partial #4) P-LFO MOD SENS",T_N},
   {"(Partial #4) TVF CUTOFF FREQ",T_N},
   {"(Partial #4) TVF RESONANCE",T_N},
   {"(Partial #4) TVF KEYFOLLOW",T_N},
   {"(Partial #4) TVF BIAS POINT/DIR",T_N},
   {"(Partial #4) TVF BIAS LEVEL",T_N},
   {"(Partial #4) TVF ENV DEPTH",T_N},
   {"(Partial #4) TVF ENV VELO SENS",T_N},
   {"(Partial #4) TVF ENV DEPTH KEYF",T_N},
   {"(Partial #4) TVF ENV TIME KEYF",T_N},
   {"(Partial #4) TVF ENV TIME 1",T_N},
   {"(Partial #4) TVF ENV TIME 2",T_N},
   {"(Partial #4) TVF ENV TIME 3",T_N},
   {"(Partial #4) TVF ENV TIME 4",T_N},
   {"(Partial #4) TVF ENV TIME 5",T_N},
   {"(Partial #4) TVF ENV LEVEL 1",T_N},
   {"(Partial #4) TVF ENV LEVEL 2",T_N},
   {"(Partial #4) TVF ENV LEVEL 3",T_N},
   {"(Partial #4) TVF ENV SUSTAIN LEVEL",T_N},
   {"(Partial #4) TVA LEVEL",T_N},
   {"(Partial #4) TVA VELO SENS",T_N},
   {"(Partial #4) TVA BIAS POINT 1",T_N},
   {"(Partial #4) TVA BIAS LEVEL 1",T_N},
   {"(Partial #4) TVA BIAS POINT 2",T_N},
   {"(Partial #4) TVA BIAS LEVEL 2",T_N},
   {"(Partial #4) TVA ENV TIME KEYF",T_N},
   {"(Partial #4) TVA ENV TIME V_FOLLOW",T_N},
   {"(Partial #4) TVA ENV TIME 1",T_N},
   {"(Partial #4) TVA ENV TIME 2",T_N},
   {"(Partial #4) TVA ENV TIME 3",T_N},
   {"(Partial #4) TVA ENV TIME 4",T_N},
   {"(Partial #4) TVA ENV TIME 5",T_N},
   {"(Partial #4) TVA ENV LEVEL 1",T_N},
   {"(Partial #4) TVA ENV LEVEL 2",T_N},
   {"(Partial #4) TVA ENV LEVEL 3",T_N},
   {"(Partial #4) TVA ENV SUSTAIN LEVEL",T_N},
   {"(Unused -- timbre offset $F6)",T_NONE},
   {"(Unused -- timbre offset $F7)",T_NONE},
   {"(Unused -- timbre offset $F8)",T_NONE},
   {"(Unused -- timbre offset $F9)",T_NONE},
   {"(Unused -- timbre offset $FA)",T_NONE},
   {"(Unused -- timbre offset $FB)",T_NONE},
   {"(Unused -- timbre offset $FC)",T_NONE},
   {"(Unused -- timbre offset $FD)",T_NONE},
   {"(Unused -- timbre offset $FE)",T_NONE},
   {"(Unused -- timbre offset $FF)",T_NONE}
};

const Roland_area areas[] =
{
   {"Timbre Temporary Area (part 1-8)",0x02,0x00,0x00,0xf6,1,1,
      timbre},
   {"Patch Temporary Area (part %u)",0x03,0x00,0x00,0x10,1,8,
      patch_temp},
   {"Rhythm Setup Temporary Area (for Key # %u)",0x03,0x01,0x10,0x04,24,85,
      rhythm_setup},
   {"Timbre Temporary Area (part %u)",0x04,0x00,0x00,0xf6,1,8,
      timbre},
   {"Patch Memory #%u",0x05,0x00,0x00,0x08,1,128,
      patch},
   {"Timbre Memory #%u",0x08,0x00,0x00,0x100,1,64,
      timbre},
   {"System area",0x10,0x00,0x00,0x17,1,1,
      system_area},
   {"Display",0x20,0x00,0x00,0x14,1,1,
      display},
   {"Write Request",0x40,0x00,0x00,0x20,1,1,
      write_request},
   {"All parameters reset",0x7f,0x00,0x00,0x01,1,1,
      NULL}
};

//################################################################
static sysex_addr calc_offset(sysex_addr base, U32 offset)
{
   U32 addr;
   sysex_addr result;

   addr = ((U32) base.MSB << 14L) |
          ((U32) base.KSB << 7L) |
          ((U32) base.LSB);

   addr += offset;

   result.LSB = (U16)(addr & 0x7fL);
   result.KSB = (U16)((addr >> 7L) & 0x7fL);
   result.MSB = (U16)((addr >> 14L) & 0x7fL);

   return result;
}

static S32 sysex_window(sysex_addr cur, sysex_addr next, sysex_addr test)
{
   U32 laddr,haddr,taddr;

   laddr = ((U32) cur.MSB << 14L) |
           ((U32) cur.KSB << 7L) |
           ((U32) cur.LSB);

   haddr = ((U32) next.MSB << 14L) |
           ((U32) next.KSB << 7L) |
           ((U32) next.LSB);

   taddr = ((U32) test.MSB << 14L) |
           ((U32) test.KSB << 7L) |
           ((U32) test.LSB);

   if ((taddr >= laddr) && (taddr < haddr))
      return taddr-laddr;

   return -1;
}

static void show_address(U16 curpos, U16 dest, sysex_addr cur)
{
   U16 k;

   AIL_mem_printc(memout,' ');
   for (k=curpos;k<dest;k++)
      AIL_mem_printc(memout,'.');
   AIL_mem_printf(memout," %.2X %.2X %.2X (%.3u %.3u %.3u)\r\n",cur.MSB,cur.KSB,cur.LSB,
      cur.MSB,cur.KSB,cur.LSB);
}

//################################################################
static U16 describe_addr(sysex_addr addr, addr_desc *desc)
{
   U32 i,j,m;
   S32 k;
   sysex_addr cur,next;
   const entry *parms;

   for (i=0;i<sizeof(areas)/sizeof(Roland_area);i++)
      {
      cur = areas[i].base;
      m = areas[i].group_count;

      for (j=0;j<m;j++)
         {
         next = calc_offset(cur,areas[i].base_size);

         k = sysex_window(cur,next,addr);
         if (k != -1)
            {
            if (m > 1)
               AIL_sprintf(desc->area_name,areas[i].name,j+areas[i].base_num);
            else
               AIL_sprintf(desc->area_name,areas[i].name);

            parms = areas[i].items;
            if (parms != NULL)
               {
               AIL_strcpy(desc->parm_name,parms[k].name);
               desc->type = parms[k].type;
               }
            else
               {
               AIL_strcpy(desc->parm_name,"");
               desc->type = T_NONE;
               }

            return 1;
            }
            
         cur = calc_offset(cur,areas[i].base_size);
         }
      }

   return 0;
}

//################################################################
static void show_MR(U16 abridged)
{
   U16 i,gc;
   U16 j,k,m;
   char *tname;
   sysex_addr group,cur;
   const entry *parms;

   AIL_mem_prints(memout,"              Roland MT-32 / LAPC-1 Programmer's Reference Guide\r\n\r\n");

   AIL_mem_prints(memout,"Parameter                                               Hex      Decimal\r\n");
   AIL_mem_prints(memout,"_______________________________________________________ ________ _____________\r\n\r\n");

   for (i=0;i<sizeof(areas)/sizeof(Roland_area);i++)
      {
      cur = areas[i].base;
      gc = areas[i].group_count;

      for (j=0;j<gc;j++)
         {
         tname = areas[i].name;
         k = (U16)((gc > 1) ?
            AIL_mem_printf(memout,tname,j+areas[i].base_num) : AIL_mem_prints(memout,tname));
         if (abridged)
            show_address(k,54,cur);
         else
            AIL_mem_prints(memout,"\r\n");
         group = cur;
         cur = calc_offset(cur,areas[i].base_size);
         if (abridged) continue;

         for (m=0;m<areas[i].base_size;m++)
            {
            parms = areas[i].items;
            k = (U16)((parms==NULL) ?
               AIL_mem_printf(memout,"   %s",tname) : AIL_mem_printf(memout,"   %s",parms[m].name));
            show_address(k,54,group);
            group = calc_offset(group,1);
            }
         }
      if (!abridged)
         {
         AIL_mem_prints(memout,"\r\n");
         }
      }
   AIL_mem_prints(memout,"\r\n\r\n");
}

//################################################################
static U8 get_chr(void)
{
   U8 val;

   val = (*(U8 *) bufpnt);
   offset = AIL_ptr_dif(bufpnt,buffer);

   bufpnt=(C8*)AIL_ptr_add(bufpnt,1);
   return val;
}

static U8 next_chr(void)
{
   return (*(U8 *) bufpnt);
}

static U32 get_vln(void)
{
   U32 val=0L;
   U16 i,cnt=4;

   do
      {
      i = get_chr();
      val = (val << 7) | (U32) (i & 0x7f);
      if (!(i & 0x80))
         cnt = 0;
      else
         --cnt;
      }
   while (cnt);

   return val;
}

static U32 get_ulong(void)
{
   U32 val;

   val = (U32) get_chr();
   val = (val << 8) | (U32) get_chr();
   val = (val << 8) | (U32) get_chr();
   val = (val << 8) | (U32) get_chr();

   return val;
}

static U32 get_24(void)
{
   U32 val;

   val = (U32) get_chr();
   val = (val << 8) | (U32) get_chr();
   val = (val << 8) | (U32) get_chr();

   return val;
}

static U16 get_u(void)
{
   U16 val;

   val = get_chr();
   val = (val << 8) | get_chr();

   return val;
}

//################################################################
static U16 inspect_Roland_sysex(U8 *buffer, U16 len)
{
   U16 i;
   static char tab[] = "           ";
   char lastname[256];
   sysex_addr target;
   addr_desc desc;

   if (len < 7)
      {
      AIL_mem_printf(memout,"%sUnrecognized message format",tab);
      return 0;
      }

   if ((buffer[0] != 0x41) || (buffer[1] != 0x10) || (buffer[2] != 0x16))
      {
      AIL_mem_printf(memout,"%sUnrecognized manufacturer/device code",tab);
      return 0;
      }

   AIL_mem_printf(memout,"%sManufacturer ID = $41\r\n",tab);
   AIL_mem_printf(memout,"%sDevice ID = $10\r\n",tab);
   AIL_mem_printf(memout,"%sModel ID = $16\r\n",tab);
   AIL_mem_printf(memout,"%sCommand = $%.2X ",tab,buffer[3]);
   switch (buffer[3])
      {
      case 0x11: AIL_mem_prints(memout,"(Request data 1: RQ1)"); return 0;
      case 0x12: AIL_mem_prints(memout,"(Data set 1: DT1)"); break;
      case 0x40: AIL_mem_prints(memout,"(Want to send data: WSD)"); return 0;
      case 0x41: AIL_mem_prints(memout,"(Request data: RQD)"); return 0;
      case 0x42: AIL_mem_prints(memout,"(Data set: DAT)"); break;
      case 0x43: AIL_mem_prints(memout,"(Acknowledge: ACK)"); return 0;
      case 0x45: AIL_mem_prints(memout,"(End of data: EOD)"); return 0;
      case 0x4e: AIL_mem_prints(memout,"(Communications error: ERR)"); return 0;
      case 0x4f: AIL_mem_prints(memout,"(Rejection: RJC)"); return 0;
      default: AIL_mem_prints(memout,"(Unknown or invalid command ID)"); return 0;
      }

   target.MSB = buffer[4];
   target.KSB = buffer[5];
   target.LSB = buffer[6];

   AIL_strcpy(lastname,"xxx");

   for (i=7;i<(len-2);i++)
      {
      if (target.MSB == 0x7f)
         {
         AIL_mem_printf(memout,"\r\n%s   All parameters reset",tab);
         target = calc_offset(target,1);
         continue;
         }

      if (!describe_addr(target,&desc))
         {
         AIL_mem_printf(memout,"\r\n%s(Undocumented address -- cannot parse message)",tab);
         return 0;
         }

      if (AIL_strcmp(lastname,desc.area_name))
         {
         AIL_mem_printf(memout,"\r\n%sAddress $%.2X $%.2X $%.2X: %s",tab,target.MSB,target.KSB,
            target.LSB,desc.area_name);
         AIL_strcpy(lastname,desc.area_name);
         }

      switch (desc.type)
         {
         case T_NONE:
            break;
         case T_A:
            AIL_mem_printf(memout,"\r\n%s   %s",tab,desc.parm_name);
            AIL_mem_printf(memout," '%c'",buffer[i]);
            break;
         case T_N:
            AIL_mem_printf(memout,"\r\n%s   %s",tab,desc.parm_name);
            AIL_mem_printf(memout," = %u",buffer[i]);
            break;
         case T_C:
            AIL_mem_printf(memout,"\r\n%s   %s",tab,desc.parm_name);
            AIL_mem_printf(memout," = %u",buffer[i]+1);
            break;
         }

      target = calc_offset(target,1);
      }


   AIL_mem_printf(memout,"\r\n%sChecksum = $%.2X",tab,buffer[len-2]);

   if (buffer[len-1] != 0xf7)
      {
      AIL_mem_prints(memout,"\r\nWARNING: Missing $F7 EOX terminator");
      }

   return 1;
}

//################################################################
static S32 parse_chunk(void)
{                          // returns 0 if last chunk
   U32 h,i,j,l,m,s,k,v;
   char chunk_title[5];
   U32 chunk_len,chunk_end;
   U32 division;
   static U32 tracks_left;
   U32 status,running_status,channel;
   U32 end_of_track;
   U32 delta_time;
   U32 t;
   U32 con_num,con_val;
   static const char bs[][4] = {"MSB","LSB"};
   static const char stat[][4] = {"off","on"};
   static const S32 pwr_2[] = {0,2,4,8,16,32,64,128,256};
   U32 bn;

   for (i=0;i<4;i++) chunk_title[i] = (char)get_chr(); chunk_title[i] = 0;
   chunk_len = get_ulong();
   chunk_end = offset + chunk_len + 1;

   AIL_mem_printf(memout,"Offset $%.4X: Chunk %s, length $%.4X\r\n",offset-7,chunk_title,chunk_len);

   if (!AIL_strnicmp(chunk_title,"MThd",4))
      {
      AIL_mem_printf(memout,"    MIDI file format: %u\r\n",get_u());
      AIL_mem_printf(memout,"    # of tracks: %u\r\n",tracks_left = get_u());
      division = get_u();
      if (division & 0x8000)
         {
         AIL_mem_printf(memout,"    SMPTE division: %u frames per second, %u ticks per frame\r\n",
            256 - ((division & 0xff00) >> 8),division & 0xff);
         }
      else
         {
         AIL_mem_printf(memout,"    MIDI division: %u delta-time ticks per quarter-note\r\n",division);
         }
      }

   if (AIL_strnicmp(chunk_title,"MTrk",4))
      {
      bufpnt = ( C8* ) AIL_ptr_add(buffer,chunk_end);
      return (!last_chunk);          // unknown chunk type
      }

   // else parse MTrk chunk ...

   running_status = 0;
   end_of_track = 0;

   do
      {
      delta_time = get_vln();

      AIL_mem_printf(memout,"    DTime %.4lu: ",delta_time);

      status = 0;

      if (next_chr() >= 0x80)
         status = running_status = get_chr();
      else
         status = running_status;

      if ((status >= 0x80) && (status < 0xf0))
         {
         channel = status & 0x0f;      // channel voice message $8x-$Ex
         status  = status & 0xf0;
         AIL_mem_printf(memout,"Ch %.2u ",channel+1);
         }

      switch (status)
         {
         case 0x80:        // note off
            k = get_chr(); v = get_chr();
            AIL_mem_printf(memout,"note %.3u off, release velocity %.3u",k,v);
            break;
         case 0x90:        // note on
            k = get_chr(); v = get_chr();
            if (v == 0)
               AIL_mem_printf(memout,"note %.3u off",k);
            else
               AIL_mem_printf(memout,"note %.3u on, attack velocity %.3u",k,v);
            break;
         case 0xa0:        // polyphonic key pressure
            k = get_chr(); v = get_chr();
            AIL_mem_printf(memout,"polyphonic key pressure %.3u applied to note %.3u",v,k);
            break;
         case 0xb0:        // control change or channel mode message
            con_num = get_chr();
            con_val = get_chr();

            bn = 0;

            switch (con_num)
               {
               case 1:
                  AIL_mem_printf(memout,"modulation controller %s = %.3u",bs[bn],con_val);
                  break;
               case 2:
                  AIL_mem_printf(memout,"breath controller %s = %.3u",bs[bn],con_val);
                  break;
               case 4:
                  AIL_mem_printf(memout,"foot controller %s = %.3u",bs[bn],con_val);
                  break;
               case 5:
                  AIL_mem_printf(memout,"portamento time %s = %.3u",bs[bn],con_val);
                  break;
               case 6:
                  AIL_mem_printf(memout,"data entry %s = %.3u",bs[bn],con_val);
                  break;
               case 7:
                  AIL_mem_printf(memout,"main volume %s = %.3u",bs[bn],con_val);
                  break;
               case 8:
                  AIL_mem_printf(memout,"balance controller %s = %.3u",bs[bn],con_val);
                  break;
               case 10:
                  AIL_mem_printf(memout,"pan controller %s = %.3u",bs[bn],con_val);
                  break;
               case 11:
                  AIL_mem_printf(memout,"expression controller %s = %.3u",bs[bn],con_val);
                  break;
               case 16:
                  AIL_mem_printf(memout,"general purpose controller #1 %s = %.3u",bs[bn],con_val);
                  break;
               case 17:
                  AIL_mem_printf(memout,"general purpose controller #2 %s = %.3u",bs[bn],con_val);
                  break;
               case 18:
                  AIL_mem_printf(memout,"general purpose controller #3 %s = %.3u",bs[bn],con_val);
                  break;
               case 19:
                  AIL_mem_printf(memout,"general purpose controller #4 %s = %.3u",bs[bn],con_val);
                  break;

               case 32:
                  AIL_mem_printf(memout,"MSS sysex start address MSB (queue 0) %.3u",con_val);
                  break;
               case 33:
                  AIL_mem_printf(memout,"MSS sysex start address KSB (queue 0) %.3u",con_val);
                  break;
               case 34:
                  AIL_mem_printf(memout,"MSS sysex start address LSB (queue 0) %.3u",con_val);
                  break;
               case 35:
                  AIL_mem_printf(memout,"MSS sysex data byte (queue 0) %.3u",con_val);
                  break;
               case 36:
                  AIL_mem_printf(memout,"MSS final sysex data byte (queue 0) %.3u",con_val);
                  break;
               case 37:
                  AIL_mem_printf(memout,"MSS sysex start address MSB (queue 1) %.3u",con_val);
                  break;
               case 38:
                  AIL_mem_printf(memout,"MSS sysex start address KSB (queue 1) %.3u",con_val);
                  break;
               case 39:
                  AIL_mem_printf(memout,"MSS sysex start address LSB (queue 1) %.3u",con_val);
                  break;
               case 40:
                  AIL_mem_printf(memout,"MSS sysex data byte (queue 1) %.3u",con_val);
                  break;
               case 41:
                  AIL_mem_printf(memout,"MSS final sysex data byte (queue 1) %.3u",con_val);
                  break;
               case 42:
                  AIL_mem_printf(memout,"MSS sysex start address MSB (queue 2) %.3u",con_val);
                  break;
               case 43:
                  AIL_mem_printf(memout,"MSS sysex start address KSB (queue 2) %.3u",con_val);
                  break;
               case 44:
                  AIL_mem_printf(memout,"MSS sysex start address LSB (queue 2) %.3u",con_val);
                  break;
               case 45:
                  AIL_mem_printf(memout,"MSS sysex data byte (queue 2) %.3u",con_val);
                  break;
               case 46:
                  AIL_mem_printf(memout,"MSS final sysex data byte (queue 2) %.3u",con_val);
                  break;

               case 58:
                  AIL_mem_printf(memout,"MSS rhythm setup timbre %.3u",con_val);
                  break;
               case 59:
                  AIL_mem_printf(memout,"MSS MT-32 patch reverb switch %.3u",con_val);
                  break;
               case 60:
                  AIL_mem_printf(memout,"MSS MT-32 patch bender range %.3u",con_val);
                  break;
               case 61:
                  AIL_mem_printf(memout,"MSS MT-32 reverb mode %.3u",con_val);
                  break;
               case 62:
                  AIL_mem_printf(memout,"MSS MT-32 reverb time %.3u",con_val);
                  break;
               case 63:
                  AIL_mem_printf(memout,"MSS MT-32 reverb level %.3u",con_val);
                  break;

               case 64:
                  AIL_mem_printf(memout,"damper pedal (sustain) = %s",stat[con_val > 63]);
                  break;
               case 65:
                  AIL_mem_printf(memout,"portamento = %s",stat[con_val > 63]);
                  break;
               case 66:
                  AIL_mem_printf(memout,"sostenuto %s",stat[con_val > 63]);
                  break;
               case 67:
                  AIL_mem_printf(memout,"soft pedal = %s",stat[con_val > 63]);
                  break;
               case 69:
                  AIL_mem_printf(memout,"hold 2 = %s",stat[con_val > 63]);
                  break;
               case 80:
                  AIL_mem_printf(memout,"general purpose controller #5 = %.3u",con_val);
                  break;
               case 81:
                  AIL_mem_printf(memout,"general purpose controller #6 = %.3u",con_val);
                  break;
               case 82:
                  AIL_mem_printf(memout,"general purpose controller #7 = %.3u",con_val);
                  break;
               case 83:
                  AIL_mem_printf(memout,"general purpose controller #8 = %.3u",con_val);
                  break;
               case 91:
                  AIL_mem_printf(memout,"external effects depth = %.3u",con_val);
                  break;
               case 92:
                  AIL_mem_printf(memout,"tremolo depth = %.3u",con_val);
                  break;
               case 93:
                  AIL_mem_printf(memout,"chorus depth = %.3u",con_val);
                  break;
               case 94:
                  AIL_mem_printf(memout,"celeste (detune) depth = %.3u",con_val);
                  break;
               case 95:
                  AIL_mem_printf(memout,"phaser depth = %.3u",con_val);
                  break;

               case 96:
                  AIL_mem_prints(memout,"data increment");
                  break;
               case 97:
                  AIL_mem_prints(memout,"data decrement");
                  break;

               case 98:
                  AIL_mem_printf(memout,"non-registered parameter LSB %.3u selected",con_val);
                  break;
               case 99:
                  AIL_mem_printf(memout,"non-registered parameter MSB %.3u selected",con_val);
                  break;
               case 100:
                  AIL_mem_printf(memout,"registered parameter LSB %.3u selected",con_val);
                  break;
               case 101:
                  AIL_mem_printf(memout,"registered parameter MSB %.3u selected",con_val);
                  break;

               case 110:
                  AIL_mem_printf(memout,"MSS channel lock/release %.3u",con_val);
                  break;
               case 111:
                  AIL_mem_printf(memout,"MSS channel lock protection %.3u",con_val);
                  break;
               case 112:
                  AIL_mem_printf(memout,"MSS voice protection %.3u",con_val);
                  break;
               case 113:
                  AIL_mem_printf(memout,"MSS timbre protection %.3u",con_val);
                  break;
               case 114:
                  AIL_mem_printf(memout,"MSS patch bank select, bank %.3u",con_val);
                  break;
               case 115:
                  AIL_mem_printf(memout,"MSS indirect controller prefix, array[%.3u]",con_val);
                  break;
               case 116:
                  AIL_mem_printf(memout,"MSS loop controller: FOR loop = 1 to %.3u",con_val);
                  break;
               case 117:
                  AIL_mem_printf(memout,"MSS loop controller: NEXT/BREAK value %.3u",con_val);
                  break;
               case 118:
                  AIL_mem_prints(memout,"MSS clear beat / measure count controller");
                  break;
               case 119:
                  AIL_mem_printf(memout,"MSS callback trigger controller %.3u",con_val);
                  break;
               case 120:
                  AIL_mem_printf(memout,"MSS sequence index controller %.3u",con_val);
                  break;

               case 121:
                  AIL_mem_prints(memout,"reset all controllers");
                  break;
               case 122:
                  AIL_mem_printf(memout,"local control %s",stat[con_val > 63]);
                  break;
               case 123:
                  AIL_mem_prints(memout,"all notes off");
                  break;
               case 124:
                  AIL_mem_prints(memout,"omni mode off");
                  break;
               case 125:
                  AIL_mem_prints(memout,"omni mode on");
                  break;
               case 126:
                  AIL_mem_printf(memout,"mono mode on, %.3u channels",con_val);
                  break;
               case 127:
                  AIL_mem_prints(memout,"poly mode on");
                  break;

               default:
                  if (con_num > 127)
                     AIL_mem_printf(memout,"MIDI file syntax error: data value %.3u",con_num);
                  else
                     AIL_mem_printf(memout,"undefined controller #%.3u, value = %.3u",
                     con_num,con_val);
                  break;
               }
            break;

         case 0xc0:        // program change
            AIL_mem_printf(memout,"program change to %.3u",get_chr());
            break;
         case 0xd0:        // channel pressure
            AIL_mem_printf(memout,"channel pressure %.3u applied",get_chr());
            break;
         case 0xe0:        // pitch bend change
            l = get_chr(); h = get_chr();
            AIL_mem_printf(memout,"pitch wheel set to %.5u",h*128+l);
            break;

         case 0xf0:        // system exclusive message
            AIL_mem_printf(memout,"system exclusive message, type $F0\r\n");
            t = get_vln();

            if (arg_mt)
               {
               for (i=0;i<t;i++) sysex_buffer[i] = (char)get_chr();
               if (inspect_Roland_sysex(sysex_buffer,(U16)t)) break;
               AIL_mem_prints(memout,"\r\n           (Hex dump not displayed.)");
               break;
               }

            AIL_mem_prints(memout,"        ");
            i = 0;
            while (t)
               {
               t--;
               AIL_mem_printf(memout,"%.2X ",get_chr());
               if ((++i == 8) && (t)) { AIL_mem_prints(memout,"\r\n        "); i=0;}
               }
            break;

         case 0xf7:        // EOX or secondary SysEX packet
            AIL_mem_prints(memout,"system exclusive message, type $F7\r\n");
            t = get_vln();

            AIL_mem_prints(memout,"        ");
            i = 0;
            while (t)
               {
               t--;                                        
               AIL_mem_printf(memout,"%.2X ",get_chr());
               if ((++i == 8) && (t)) { AIL_mem_prints(memout,"\r\n        "); i=0;}
               }
            break;

         case 0xf1:       // SysCom: MTC quarter-frame
            i = get_chr(); 
            AIL_mem_printf(memout,"MTC quarter-frame message type $%.2X",i);
            break;
         case 0xf2:       // SysCom: song position pointer
            l = get_chr(); h = get_chr();
            AIL_mem_printf(memout,"song position pointer = %.5u",h*128+l);
            break;
         case 0xf3:       // SysCom: song select
            s = get_chr();
            AIL_mem_printf(memout,"song select = %.3u",s);
            break;
         case 0xf6:       // SysCom: tune request
            AIL_mem_prints(memout,"tune request");
            break;

         case 0xff:       // meta-event
            AIL_mem_prints(memout,"meta-event: ");
            m = get_chr();
            t = get_vln();
            switch (m)
               {
               case 0x00:
                  AIL_mem_printf(memout,"sequence number %lu",get_u());
                  t -= 2;
                  while (t--) get_chr();
                  break;
               case 0x01:
                  AIL_mem_prints(memout,"text \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x02:
                  AIL_mem_prints(memout,"copyright notice \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x03:
                  AIL_mem_prints(memout,"sequence/track name \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x04:
                  AIL_mem_prints(memout,"instrument name \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x05:
                  AIL_mem_prints(memout,"lyric \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x06:
                  AIL_mem_prints(memout,"marker \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x07:
                  AIL_mem_prints(memout,"cue point \"");
                  while (t--) AIL_mem_printc(memout,get_chr());
                  AIL_mem_prints(memout,"\"");
                  break;
               case 0x20:
                  AIL_mem_printf(memout,"MIDI channel prefix %.3u",get_chr());
                  t -= 1;
                  while (t--) get_chr();
                  break;
               case 0x2f:
                  AIL_mem_prints(memout,"end of track");
                  while (t--) get_chr();
                  end_of_track = 1;
                  break;
               case 0x51:
                  AIL_mem_printf(memout,"tempo in uS/MIDI quarter-note = %lu",
                     get_24());
                  t -= 3;
                  while (t--) get_chr();
                  break;
               case 0x54:
                  i=get_chr(); j=get_chr(); k=get_chr(); l=get_chr();
                  m=get_chr();
                  AIL_mem_printf(memout,"SMPTE offset %.3u:%.3u:%.3u %.3u:%.3u",i,j,k,l,m);
                  t -= 5;
                  while (t--) get_chr();
                  break;
               case 0x58:
                  i=get_chr(); j=get_chr(); k=get_chr(); l=get_chr();
                  AIL_mem_printf(memout,"time signature %u/%u, c = %.3u, b = %.3u",
                     i,pwr_2[j],k,l);
                  t -= 4;
                  while (t--) get_chr();
                  break;
               case 0x59:
                  i=get_chr(); j=get_chr();
                  AIL_mem_printf(memout,"key signature %u %u",i,j);
                  t -= 2;
                  while (t--) get_chr();
                  break;
               case 0x7c:
                  AIL_mem_prints(memout,"MSS reset beat / measure counter (obsolete) ");
                  while (t--) AIL_mem_printf(memout,"%u ",get_chr());
                  break;
               case 0x7d:
                  i=get_chr();
                  AIL_mem_printf(memout,"MSS callback trigger %u (obsolete) ",i);
                  t-=1;
                  while (t--) AIL_mem_printf(memout,"%u ",get_chr());
                  break;
               case 0x7e:
                  i=get_chr();
                  AIL_mem_printf(memout,"MSS track index %u (obsolete) ",i);
                  t-=1;
                  while (t--) AIL_mem_printf(memout,"%u ",get_chr());
                  break;
               case 0x7f:
                  AIL_mem_printf(memout,"%lu bytes of sequencer-specific meta-event data",t);
                  while (t--) get_chr();
                  break;

               default:
                  AIL_mem_printf(memout,"undefined event code $%.2X",m);
                  while (t--) get_chr();
                  break;
               }
            break;

         default:         // undefined or illegal
            AIL_mem_printf(memout,"Undefined or illegal status byte $%.2X",status);
            break;
         }

      AIL_mem_prints(memout,"\r\n");
      }
   while (!end_of_track);

   if (!(--tracks_left)) last_chunk = 1;

   bufpnt = ( C8* ) AIL_ptr_add(buffer, chunk_end);
   return (!last_chunk);
}


DXDEF
S32       AILEXPORT AIL_list_MIDI         (void const FAR* MIDI,
                                           U32       MIDI_size,
                                           char FAR* FAR* list,
                                           U32  FAR* list_size,
                                           S32       flags)
{
   U32 l;

   if (MIDI == NULL)
     return(0);

   buffer = bufpnt = ( C8* ) MIDI;

   l=MIDI_size;

   bufend = ( C8* ) AIL_ptr_add(buffer,l);

   do
      {
      if (AIL_strnicmp(bufpnt,"MTHD",4)==0)
        break;
      bufpnt = ( C8* )AIL_ptr_add(bufpnt,1);
      }
   while (--l);

   if (!l)
      {
      AIL_set_error("Not a standard MIDI file!");
      return(0);
      }

   memout=AIL_mem_create();
   if (memout==0)
      {
      return(0);
      }

   if (flags&AILMIDILIST_ROLANDUN)
      {
      show_MR(0);
      }
      else if (flags&AILMIDILIST_ROLANDAB)
      {
      show_MR(1);
      }

   arg_mt=(flags&AILMIDILIST_ROLANDSYSEX)?1:0;

   AIL_mem_prints(memout,"MIDI Listing - Version " MSS_VERSION "        " MSS_COPYRIGHT "\r\n");
   AIL_mem_prints(memout,"____________________________________________________________________________\r\n\r\n");

   last_chunk=0;
   while ( parse_chunk( ) )
   {
   };

   AIL_mem_printc(memout,0);

   if ( !AIL_mem_close(memout,(void FAR* FAR*)list,list_size) )
     AIL_set_error("Out of memory.");

   return(1);
}

