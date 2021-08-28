//############################################################################
//##                                                                        ##
//##  EXAM3D.C                                                              ##
//##                                                                        ##
//##  3D provider test bed                                                  ##
//##                                                                        ##
//##  V1.00 of 23-Sep-98: Initial V3.0 release                              ##
//##                                                                        ##
//##  Written by Jeff Roberts                                               ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  For technical support, contact RAD Game Tools at 425-893-4300.        ##
//##                                                                        ##
//############################################################################

#include <windows.h>
#include <windowsx.h>
#include "mss.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include <math.h>
#include <commctrl.h>


#define APPNAME "EXAM3D"

#define MAXPROVIDERS 64

static char szAppName[] = APPNAME;

#define cboxTech     100
#define btnPlay      101
#define sldSpeed     102
#define radAroundx   103
#define radBackForth 104
#define radNone      106
#define radCity      107
#define radMountains 108
#define radHallway   109
#define btnStop      110
#define btnAbout     111
#define btnClose     112
#define btnLoad      113
#define btnProviderA 114
#define btnSoundA    115
#define radSpkSet    119
#define rad2Speaker  120
#define radHeadphone 121
#define rad4Speaker  122
#define chkCone      125
#define chkOcclude   126
#define chkObstruct  127

#define txtSpeed     202
#define txtEAX       206
#define txtCurrent   130

#define FASTSPEED 100
#define SLOWSPEED 1
#define CIRCLERADIUS 25.0F
#define STRAIGHTDIST 200.0F
#define CIRCLE (2.0F*3.14159265358979F)

//
// Miscellaneous globals / defines
//

HWND        hwnd;
HWND        combo;
HWND        slider;
HWND        env=0;
HPROVIDER   providers[MAXPROVIDERS];
S32         curprovider=-1;
S32         usingEAX=0;
HPROVIDER   opened_provider=0;
H3DSAMPLE   opened_sample=0;
void*       sample_address;
HDIGDRIVER  DIG;
S32         movetype=0;
F32         X,Y,Z,adj,oX,oY,oZ,oadj,obs,occ;
S32         room;
S32         speaker_type=AIL_3D_2_SPEAKER;
S32         runtimer=0;
char        filename[256]="..\\media\\shot.wav";
S32         rotsnd=0;
S32         nocone=0;
S32         doobstruct=0;
S32         doocclude=0;

//############################################################################
//##                                                                        ##
//## Set the speaker type chose by the user                                 ##
//##                                                                        ##
//############################################################################

static void set_speaker(S32 which)
{
  if (which==1)
    speaker_type=AIL_3D_2_SPEAKER;
  else
  if (which==2)
    speaker_type=AIL_3D_HEADPHONE;
  else
  if (which==3)
    speaker_type=AIL_3D_4_SPEAKER;

  if (opened_provider)
    AIL_set_3D_speaker_type(opened_provider,speaker_type);
}


//############################################################################
//##                                                                        ##
//## Enable or disable a dialog window control                              ##
//##                                                                        ##
//############################################################################

static void Enable_control(WORD control, BOOL state)
{
   EnableWindow( GetDlgItem(hwnd,control), state );
}


//############################################################################
//##                                                                        ##
//## Check or uncheck a dialog window control                               ##
//##                                                                        ##
//############################################################################

static void Check_control(WORD control, BOOL state)
{
   Button_SetCheck(GetDlgItem(hwnd,control), state );
}


//############################################################################
//##                                                                        ##
//## Set the rooms style for EAX                                            ##
//##                                                                        ##
//############################################################################

static void set_room(S32 val)
{
  room=val;

  if ((opened_provider) && (usingEAX))
  {
    AIL_set_3D_room_type(opened_provider,val);

    // turn off EAX, if they choose no reverb
    if (val==ENVIRONMENT_GENERIC)
      AIL_set_3D_sample_effects_level(opened_sample,0.0F);
    else
    {
      // -1 is the provider default level
      AIL_set_3D_sample_effects_level(opened_sample,-1.0F);
    }
  }
}


//############################################################################
//##                                                                        ##
//## Enable or disable the proper controls                                  ##
//##                                                                        ##
//############################################################################

static void set_provider_controls()
{
  S32 open;
  S32 eax;

  open=(curprovider!=-1);
  Enable_control(btnPlay,open);
  Enable_control(txtSpeed,open);
  Enable_control(sldSpeed,open);
  Enable_control(radAroundx,open);
  Enable_control(radBackForth,open);
  Enable_control(radSpkSet,open);
  Enable_control(rad2Speaker,open);
  Enable_control(radHeadphone,open);
  Enable_control(rad4Speaker,open);
  Enable_control(btnStop,open);
  Enable_control(btnProviderA,open);
  Enable_control(btnSoundA,open);
  Enable_control(txtCurrent,open);
  Enable_control(chkCone,open && !nocone);
  Enable_control(chkOcclude,open);
  Enable_control(chkObstruct,open);

  eax=(curprovider!=-1) && usingEAX;

  Enable_control(txtEAX,eax);
  Enable_control(radNone,eax);
  Enable_control(radCity,eax);
  Enable_control(radMountains,eax);
  Enable_control(radHallway,eax);
}


static S32 envwidth;
static S32 soundadjx,soundadjy;
static RECT envrect;
static HBRUSH listbrush,sndbrush,cratebrush,wallbrush;
static HDC envdc;

#define SOUNDSIZE 4
#define LISTENERSIZE 12
#define CRATESIZE 12
#define WALLHEIGHT 48
#define WALLWIDTH 8

// ugly routine to draw the environment with GDI

static void draw_env()
{
  HDC dc;
  HBRUSH old;
  POINT nose[3];

  S32 x,y,dx,dy;
  F32 length;
  
  length=(F32)sqrt(oX*oX+oZ*oZ);
  if (length<=0.00001F)
    length=1.0F;

  if (env==0)
  {
    env=GetDlgItem(hwnd,txtCurrent);
    GetWindowRect(env,&envrect);
    envrect.right-=(envrect.left+12);
    envrect.bottom-=(envrect.top+10);
    envrect.left=12;
    envrect.top=20;
    envwidth=envrect.right-envrect.left-SOUNDSIZE*7;
    soundadjx=12+(envwidth/2)+SOUNDSIZE*3;
    soundadjy=20+((envrect.bottom-envrect.top-SOUNDSIZE*4)/2)+((SOUNDSIZE*3)/2);
    dc=GetDC(env);
    envdc=CreateCompatibleDC(dc);
    SelectObject(envdc,CreateCompatibleBitmap(dc,envrect.right,envrect.bottom));
    ReleaseDC(env,dc);
    listbrush=CreateSolidBrush(RGB(255,128,128));
    sndbrush=CreateSolidBrush(RGB(0,0,128));
    cratebrush=CreateSolidBrush(RGB(208,208,0));
    wallbrush=CreateSolidBrush(RGB(228,0,64));
  }

  if (movetype==0)
  {
    x=(S32)((X*(F32)envwidth)/(CIRCLERADIUS*2.0F));
    y=(S32)((-Z*(F32)envwidth)/(CIRCLERADIUS*2.0F));
    dx=(S32)(((X+((oX/length)*5.0F))*(F32)envwidth)/(CIRCLERADIUS*2.0F));
    dy=(S32)(((-Z+((-oZ/length)*5.0F))*(F32)envwidth)/(CIRCLERADIUS*2.0F));
  }
  else
  {
    x=(S32)((X*(F32)envwidth)/(STRAIGHTDIST*2.0F));
    y=(S32)((-Z*(F32)envwidth)/(STRAIGHTDIST*2.0F));
    dx=(S32)(((X+((oX/length)*40.0F))*(F32)envwidth)/(STRAIGHTDIST*2.0F));
    dy=(S32)(((-Z+((-oZ/length)*40.0F))*(F32)envwidth)/(STRAIGHTDIST*2.0F));
  }

  FillRect(envdc,&envrect,(HBRUSH)(COLOR_WINDOW+1));

//  Uncomment to display the current occ/obs values
//  {char buf[128];sprintf(buf,"%f %f %f %f",adj,(F32)-Z,obs,occ);
//  TextOut(envdc,envrect.left,envrect.top,buf,lstrlen(buf));}

  if (doobstruct)
  {
    RECT r;
    r.left=soundadjx-CRATESIZE;
    r.right=soundadjx+CRATESIZE;
    r.top=soundadjy-CRATESIZE*7;
    r.bottom=soundadjy-CRATESIZE*5;
    FillRect(envdc,&r,cratebrush);

    r.top=soundadjy+CRATESIZE*4;
    r.bottom=soundadjy+CRATESIZE*6;
    FillRect(envdc,&r,cratebrush);
  }


  if (doocclude)
  {
    RECT r;
    if (movetype==0)
    {
      r.left=soundadjx-CRATESIZE*6-WALLWIDTH;
      r.right=soundadjx-CRATESIZE*6;
      r.top=soundadjy-WALLHEIGHT;
      r.bottom=soundadjy+WALLHEIGHT;
      FillRect(envdc,&r,wallbrush);

      r.left=soundadjx+CRATESIZE*6;
      r.right=soundadjx+CRATESIZE*6+WALLWIDTH;
      FillRect(envdc,&r,wallbrush);
    }
    else
    {
      r.left=soundadjx-WALLHEIGHT;
      r.right=soundadjx+WALLHEIGHT;
      r.top=soundadjy-CRATESIZE*9;
      r.bottom=soundadjy-CRATESIZE*9+WALLWIDTH;
      FillRect(envdc,&r,wallbrush);

      r.top=soundadjy+CRATESIZE*7;
      r.bottom=soundadjy+CRATESIZE*7+WALLWIDTH;
      FillRect(envdc,&r,wallbrush);
    }
  }


  old=SelectObject(envdc,listbrush);
  nose[0].x=soundadjx-LISTENERSIZE/3;
  nose[0].y=soundadjy-LISTENERSIZE;
  nose[1].x=soundadjx;
  nose[1].y=soundadjy-((LISTENERSIZE*5)/3);
  nose[2].x=soundadjx+LISTENERSIZE/3;
  nose[2].y=soundadjy-LISTENERSIZE;

  Polygon(envdc,nose,3);
  Ellipse(envdc,soundadjx-LISTENERSIZE,soundadjy-LISTENERSIZE,soundadjx+LISTENERSIZE,soundadjy+LISTENERSIZE);
  SelectObject(envdc,sndbrush);
  MoveToEx(envdc,soundadjx+x,soundadjy+y,0);
  LineTo(envdc,soundadjx+dx,soundadjy+dy);
  Ellipse(envdc,soundadjx+x-SOUNDSIZE,soundadjy+y-SOUNDSIZE,soundadjx+x+SOUNDSIZE,soundadjy+y+SOUNDSIZE);

  SelectObject(envdc,old);
  dc=GetDC(env);
  if (dc)
  {
    BitBlt(dc,envrect.left,envrect.top,envrect.right-envrect.left,envrect.bottom-envrect.top,envdc,envrect.left,envrect.top,SRCCOPY);
    ReleaseDC(env,dc);
  }
}


//############################################################################
//##                                                                        ##
//## Close the provider and free the sound sample                           ##
//##                                                                        ##
//############################################################################

static void release_existing()
{
  if (opened_sample)
  {
    AIL_release_3D_sample_handle(opened_sample);
    opened_sample=0;
  }

  if (opened_provider) 
  {
    AIL_close_3D_provider(opened_provider);
    opened_provider=0;
  }
}


//############################################################################
//##                                                                        ##
//## Initialize the starting 3D values for the sample                       ##
//##                                                                        ##
//############################################################################

static void init_sample_values()
{
  X=Y=Z=0.0F;

  if (movetype==0) 
  {
    X=25.0F;
    adj=0.0F;
    oadj=0.0F;
  } 
  else if (movetype==1)
  {
    Z=75.0F;
    adj=-1.0F;
    oadj=0.0F;
  }
  draw_env();
}


//############################################################################
//##                                                                        ##
//## Read the slider position                                               ##
//##                                                                        ##
//############################################################################

static S32 get_slider_value()
{
  return( ((S32)SendMessage(slider,TBM_GETPOS,0,0)) );
}


//############################################################################
//##                                                                        ##
//## Move the 3D position based on the type of sample movement              ##
//##                                                                        ##
//############################################################################

static void move_sample_values()
{
  if (movetype==0)
  {

    // handle the circle movement
    adj+=(CIRCLE/(((FASTSPEED+1-get_slider_value())*1.6F)+20.0F));
    if (adj>CIRCLE)
      adj-=CIRCLE;

    X=(F32)(CIRCLERADIUS*cos(adj));
    Z=(F32)(CIRCLERADIUS*sin(adj));

    // these would normally raytraced in a 3D engine...
    obs=0.0F;
    if (doobstruct)
    {
      // obstruct against the crates
      if (((adj>=1.43F) && (adj<=1.72F)) || ((adj>=4.51F) && (adj<=4.90F)))
        obs=0.90F;
      else
      {
        if ((adj>=1.38F) && (adj<=1.43F))
          obs=0.90F*((adj-1.38F)/0.05F);
        else if ((adj>=1.72F) && (adj<=1.77F))
          obs=0.90F*(1.0F-((adj-1.72F)/0.05F));
        else if ((adj>=4.46F) && (adj<=4.51F))
          obs=0.90F*((adj-4.46F)/0.05F);
        else if ((adj>=4.90F) && (adj<=4.95F))
          obs=0.90F*(1.0F-((adj-4.90F)/0.05F));
      }
    }

    occ=0.0F;
    if (doocclude)
    {
      // occlude against the walls
      if (((adj>=2.76F) && (adj<=3.51F)) || (adj<=0.40F) || (adj>=5.89F))
        occ=1.0F;
      else
      {
        if ((adj>=0.40F) && (adj<=0.81F))
          occ=1.0F-((adj-0.40F)/0.42F);
        else if ((adj>=2.34F) && (adj<=2.76F))
          occ=(adj-2.34F)/0.42F;
        else if ((adj>=3.51F) && (adj<=3.93F))
          occ=1.0F-((adj-3.51F)/0.42F);
        else if ((adj>=5.47F) && (adj<=5.89F))
          occ=(adj-5.47F)/0.42F;
      }
    }

  } 
  else if (movetype==1)
  {
    //handle the back and forth
    if (Z>STRAIGHTDIST)
      adj=-1.0F;
    else if (Z<-(STRAIGHTDIST))
      adj=1.0F;
    Z+=adj*((get_slider_value()*0.07F)+1.0F);

    // these would normally raytraced in a 3D engine...
    obs=0.0F;
    if (doobstruct)
    {
      // obstruct against the crates
      if ((Z>=156.5F) || (Z<=-136.5F))
        obs=0.50F;
      else
      {
        if ((Z>=115.5F) && (Z<=156.5F))
          obs=0.50F*((Z-115.5F)/41.0F);
        else if ((Z>=-135.5F) && (Z<=-95.5F))
          obs=0.50F*(-(Z+95.5F)/41.0F);
      }
    }

    occ=0.0F;
    if (doocclude)
    {
      // occlude against the walls
      if ((Z>=198.5F) || (Z<=-175.5F))
        occ=1.0F;
      else
      {
        if ((Z>=173.0F) && (Z<=198.5F))
          occ=(Z-173.0F)/25.0F;
        else if ((Z>=-175.0F) && (Z<=-150.0F))
          occ=(-(Z+150.0F)/25.0F);
      }
    }
  }
}


//############################################################################
//##                                                                        ##
//## Set the sample values into the 3D provider                             ##
//##                                                                        ##
//############################################################################

static void set_sample_values()
{
  F32 sX,sY,sZ,sA,dX,dY,dZ;

  // set new position
  AIL_set_3D_position(opened_sample, X, Y, Z);

  if ((rotsnd) && (!nocone))
  {
    // rotate the sound
    oadj+=(CIRCLE/10.0F);
    if (oadj>CIRCLE)
      oadj-=CIRCLE;

    oX=(F32)(CIRCLERADIUS*cos(oadj));
    oY=-Y;
    oZ=(F32)(CIRCLERADIUS*sin(oadj));
  }
  else
  {
    // always face the origin (where the listener is)
    oX=-X;
    oY=-Y;
    oZ=-Z;
  }
  
  AIL_set_3D_orientation(opened_sample, oX, oY, oZ, 0.0F, 1.0F, 0.0F);

  //save old settings
  sX=X;
  sY=Y;
  sZ=Z;
  sA=adj;

  // move the sample values
  move_sample_values();

  // calculate the delta vector
  dX=X-sX;
  dY=Y-sY;
  dZ=Z-sZ;

  // restore the values to original
  X=sX;
  Y=sY;
  Z=sZ;
  adj=sA;

  // set the velocity for doppler effects
  AIL_set_3D_velocity(opened_sample, dX, dY, dZ, ((get_slider_value()/300.0F)+1.0F)/1500.0F);

  // set the obstruction and occlusion
  AIL_set_3D_sample_obstruction(opened_sample, obs);
  AIL_set_3D_sample_occlusion(opened_sample, occ);

  draw_env();
}


//############################################################################
//##                                                                        ##
//## Load a new provider                                                    ##
//##                                                                        ##
//############################################################################

static void set_new_provider(S32 index)
{
  DWORD result;

  runtimer=0;

  curprovider=index;

  //close the already opened provider
  release_existing();

  if (curprovider==-1)
  {
    InvalidateRect(hwnd,0,TRUE);
  }
  else
  {
    //load the new provider
    result = AIL_open_3D_provider(providers[index]);
    if (result != M3D_NOERR) 
    {

      curprovider=-1;
      MessageBox(hwnd,AIL_last_error(),"Error opening...",MB_OK);

     err:
      ComboBox_SetCurSel(combo,0);
      curprovider=-1;
      release_existing();
      InvalidateRect(hwnd,0,TRUE);

    } 
    else
    {

      opened_provider=providers[index];

      //see if we're running under an EAX compatible provider
      result=AIL_3D_room_type(opened_provider);
      usingEAX=(((S32)result)!=-1)?1:0; // will be something other than -1 on EAX

      //obtain a 3D sample handle
      opened_sample=AIL_allocate_3D_sample_handle(opened_provider);

      //now try to load the sample into the provider
      if (AIL_set_3D_sample_file(opened_sample,sample_address)==0) 
      {
        MessageBox(hwnd,AIL_last_error(),"Error loading...",MB_OK);
        goto err;
      }

      // set the sample to loop and set the distances
      AIL_set_3D_sample_loop_count(opened_sample,0);
      AIL_set_3D_sample_distances(opened_sample,200,20);
      AIL_set_3D_sample_cone(opened_sample,90,120,0);

      AIL_3D_sample_cone(opened_sample,0,0,&result);
      nocone=(result!=0)?1:0;

      // set up the starting values
      init_sample_values();
      set_sample_values();
      set_speaker(0);
      set_room(room);

      draw_env();
    }
  }

  set_provider_controls();
}


//############################################################################
//##                                                                        ##
//## Add the provider strings to the combo box                              ##
//##                                                                        ##
//############################################################################

typedef struct provider_stuff
{
  char* name;
  HPROVIDER id;
} provider_stuff;


static int __cdecl comp(const provider_stuff*s1,const provider_stuff*s2)
{
  return( _stricmp(s1->name,s2->name) );
}


static void add_providers()
{
   provider_stuff pi[MAXPROVIDERS];
   U32   n,i;

   HPROENUM next = HPROENUM_FIRST;

   combo=GetDlgItem(hwnd,cboxTech);

   ComboBox_AddString(combo,"Choose a provider...");

   n=0;
   while (AIL_enumerate_3D_providers(&next, &pi[n].id, &pi[n].name) && (n<MAXPROVIDERS))
   {
     ++n;
   }

   qsort(pi,n,sizeof(pi[0]),comp);

   for(i=0;i<n;i++)
   {
      providers[i]=pi[i].id;
      ComboBox_AddString(combo,pi[i].name);
   }

   ComboBox_SetCurSel(combo,0);
}


//############################################################################
//##                                                                        ##
//## Show the about window                                                  ##
//##                                                                        ##
//############################################################################

static void Show_about()
{
   char text[1024];
   char version[8];
   AIL_MSS_version(version,8);
   lstrcpy(text,"Version ");
   lstrcat(text,version);
   lstrcat(text," " MSS_COPYRIGHT "\n\n"
               APPNAME ": Miles Sound System 3D Example program.\n\n"
               "For questions or comments, please contact RAD Game Tools at:\n\n"
               "\tRAD Game Tools\n"
               "\t335 Park Place - Suite G109\n"
               "\tKirkland, WA  98033\n"
               "\t425-893-4300\n"
               "\tFAX: 425-893-9111\n\n"
               "\tWeb: www.radgametools.com\n"
               "\tE-mail: sales@radgametools.com");
   MessageBox(hwnd,text,"About " APPNAME, MB_OK);
}


static void show_provider_props(void)
{
   char buf[3072];
   S32 len=0;

   HINTENUM next = HINTENUM_FIRST;

   RIB_INTERFACE_ENTRY attrib;

   lstrcpy(buf,"None.");

   while (AIL_enumerate_3D_provider_attributes(opened_provider,
                                              &next,
                                              &attrib))
   {
      S32 value;

      AIL_3D_provider_attribute(opened_provider,
                                attrib.entry_name,
                               &value);
      len+=wsprintf(buf+len,"   %s = %s\n",
                            attrib.entry_name,
                            RIB_type_string(value, attrib.subtype));
   }

   MessageBox(hwnd,buf,"3D Provider Attributes",MB_OK|MB_ICONINFORMATION);
}


static void show_sound_props(void)
{
   char buf[3072];
   S32 len=0;

   HINTENUM next = HINTENUM_FIRST;

   RIB_INTERFACE_ENTRY attrib;

   lstrcpy(buf,"None.");

   while (AIL_enumerate_3D_sample_attributes(opened_provider,
                                              &next,
                                              &attrib))
   {
      S32 value;

      AIL_3D_sample_attribute(opened_sample,
                                attrib.entry_name,
                               &value);

      len+=wsprintf(buf+len,"   %s = %s\n",
                            attrib.entry_name,
                            RIB_type_string(value, attrib.subtype));
   }

   MessageBox(hwnd,buf,"3D Sample Attributes",MB_OK|MB_ICONINFORMATION);
}


//############################################################################
//##                                                                        ##
//## Load a sample file into memory (decompress it if necessary)            ##
//##                                                                        ##
//############################################################################

static void* load_sample_file(char* name)
{
   void* s;
   void* d;
   S32 type;
   AILSOUNDINFO info;
   S32 size;

   size=AIL_file_size(name);
   s=AIL_file_read(name,0);

   if (s==0)
     return(0);

   type=AIL_file_type(s,size);

   switch (type)
   {
     case AILFILETYPE_PCM_WAV:
       return(s);

     case AILFILETYPE_ADPCM_WAV:
       AIL_WAV_info(s,&info);
       AIL_decompress_ADPCM(&info,&d,0);
       AIL_mem_free_lock(s);
       return(d);

     case AILFILETYPE_MPEG_L3_AUDIO:
       AIL_decompress_ASI(s,size,name,&d,0,0);
       AIL_mem_free_lock(s);
       return(d);

     default:
       AIL_mem_free_lock(s);
       return(0);
   }
}

static void show_load()
{
   OPENFILENAME fn;

   memset(&fn, 0, sizeof(fn));

   fn.lStructSize       = sizeof(fn);
   fn.hwndOwner         = hwnd;
   fn.lpstrFilter       = "Digital sound files (*.wav;*.mp?)\0*.wav;*.mp?\0All files\0*.*\0";
   fn.nFilterIndex      = 1;
   fn.lpstrFile         = filename;
   fn.nMaxFile          = 126;
   fn.lpstrTitle        = "Select digital sound file...";
   fn.Flags             = OFN_FILEMUSTEXIST |
#ifdef IS_WIN32
                          OFN_EXPLORER | OFN_LONGNAMES |
#endif
                          OFN_NOCHANGEDIR |
                          OFN_PATHMUSTEXIST |
                          OFN_HIDEREADONLY;
   fn.lpstrDefExt       = "wav";

   if (GetOpenFileName(&fn))
   {
     void* ptr=load_sample_file(filename);
     if (ptr==0)
     {
       MessageBox(0,"Couldn't load the digital file.","Error",MB_OK);
       return;
     }
     AIL_mem_free_lock(sample_address);
     sample_address=ptr;
     set_new_provider(curprovider);
   }

}

//############################################################################
//##                                                                        ##
//## Main window procedure                                                  ##
//##                                                                        ##
//############################################################################

LRESULT AILEXPORT Window_proc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  HWND h;
  LRESULT ret;

  switch (message)
  {

      case WM_SETFOCUS:    // deal with the focus in this weird dialog-window
          h=GetWindow(hwnd,GW_CHILD);
          while (h) 
          {
            if ((GetWindowLong(h,GWL_STYLE)&0x2f)==BS_DEFPUSHBUTTON) 
            {
              SetFocus(h);
              goto found;
            }
            h=GetNextWindow(h,GW_HWNDNEXT);
          }
          SetFocus(GetWindow(hwnd,GW_CHILD));
       found:
          break;

      case WM_HSCROLL:
         return 0;

      case WM_COMMAND:

         switch (LOWORD(wParam))
         {

          case cboxTech:
             if (HIWORD(wParam) == CBN_SELENDOK)
             {
               set_new_provider(ComboBox_GetCurSel(combo)-1);
             }
             break;

          case radAroundx:
             movetype=0;
             init_sample_values();
             break;

          case radBackForth:
             movetype=1;
             init_sample_values();
             break;

          case btnPlay:
             if (opened_sample)
             {
               AIL_start_3D_sample(opened_sample);
               runtimer=1;
             }
             break;

          case radNone:
             set_room(ENVIRONMENT_GENERIC);
             break;

          case radCity:
             set_room(ENVIRONMENT_CITY);
             break;

          case radMountains:
             set_room(ENVIRONMENT_MOUNTAINS);
             break;

          case radHallway:
             set_room(ENVIRONMENT_HALLWAY);
             break;

          case rad2Speaker:
            set_speaker(1);
            break;

          case radHeadphone:
            set_speaker(2);
            break;

          case rad4Speaker:
            set_speaker(3);
            break;

          case chkCone:
            rotsnd=(SendMessage(GetDlgItem(hwnd,chkCone),BM_GETCHECK,0,0)==0)?0:1;
            break;

          case chkObstruct:
            doobstruct=(SendMessage(GetDlgItem(hwnd,chkObstruct),BM_GETCHECK,0,0)==0)?0:1;
            draw_env();
            break;

          case chkOcclude:
            doocclude=(SendMessage(GetDlgItem(hwnd,chkOcclude),BM_GETCHECK,0,0)==0)?0:1;
            draw_env();
            break;

          case btnStop:
             if (opened_sample)
             {
               AIL_stop_3D_sample(opened_sample);
               runtimer=0;
             }
             break;

          case btnProviderA:
             show_provider_props();
             break;

          case btnSoundA:
             show_sound_props();
             break;

          case btnLoad:
             show_load();
             break;

          case btnAbout:
             Show_about();
             break;

          case IDCANCEL:
          case btnClose:
             DestroyWindow(hwnd);
             break;
         }
         return 0;

      case WM_PAINT:
        ret=DefWindowProc(hwnd,message,wParam,lParam);
        if (curprovider!=-1)
          draw_env();
        return(ret);

      case WM_TIMER:
        if (runtimer)
        {
          move_sample_values();
          set_sample_values();
        }
        break;

      case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

      }

   return DefWindowProc(hwnd,message,wParam,lParam);
}


static void set_slider_defaults()
{
  slider=GetDlgItem(hwnd,sldSpeed);

  // Set the initial range
  SendMessage(slider, TBM_SETRANGE, TRUE, MAKELONG(SLOWSPEED,FASTSPEED) );

  // Set the initial position
  SendMessage(slider, TBM_SETPOS, TRUE, (FASTSPEED+SLOWSPEED)/2);

}


//############################################################################
//##                                                                        ##
//## WinMain()                                                              ##
//##                                                                        ##
//############################################################################

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszCmdLine, int nCmdShow)
{
   MSG      msg;
   WNDCLASS wndclass;

   //create all the window stuff
   if (!hPrevInstance)
      {
      wndclass.lpszClassName = szAppName;
      wndclass.lpfnWndProc   = (WNDPROC) Window_proc;
      wndclass.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
      wndclass.hInstance     = hInstance;
      wndclass.hIcon         = LoadIcon(hInstance,"Demo");
      wndclass.hCursor       = LoadCursor(NULL,IDC_ARROW);
      wndclass.hbrBackground = (HBRUSH)(COLOR_3DFACE+1);
      wndclass.cbClsExtra    = 0;
      wndclass.cbWndExtra    = DLGWINDOWEXTRA;
      wndclass.lpszMenuName  = NULL;

      RegisterClass(&wndclass);
      }

   InitCommonControls();

   hwnd = CreateDialog(hInstance,szAppName,0,NULL);

   if (hwnd==0) 
   {
     MessageBox(0,"Couldn't create dialog box.","Error",MB_OK);
     return(0);
   }

   // Initialize the Miles Sound System
   AIL_set_redist_directory("..\\..\\redist\\" MSS_REDIST_DIR_NAME);
   AIL_startup();


   // load the sample file (decompressing, if necessary)
   sample_address=load_sample_file(filename);

   if (sample_address==0) 
   {
     MessageBox(0,"Couldn't load the example digital file.","Error",MB_OK);
     return(0);
   }

   // setup the control defaults
   Check_control(rad2Speaker,1);
   Check_control(radAroundx,1);
   Check_control(radNone,1);
   set_slider_defaults();


   // load a digital driver (required for most of the 3D providers)
   if (!AIL_quick_startup(1,0,44100,16,2))
   {
     MessageBox(0,"Couldn't open a digital output device.","Error",MB_OK);
     return(0);
   }

   AIL_quick_handles(&DIG,0,0);

   //set the initial room type
   set_room(ENVIRONMENT_GENERIC);

   //add the provider names to the combo
   add_providers();

   //set the default provider controls
   set_provider_controls();

   //set a timer to move the samples around
   SetTimer(hwnd,1,50,0);

   //
   // Main message loop
   //

   ShowWindow(hwnd,nCmdShow);

   while (GetMessage(&msg, 0, 0, 0)) 
   {

     if (!IsDialogMessage(hwnd,&msg)) 
     {
       TranslateMessage(&msg);
       DispatchMessage(&msg);
     }

   }

   //close down the sound and shutdown
   release_existing();

   AIL_waveOutClose(DIG);

   AIL_shutdown();

   return msg.wParam;
}

