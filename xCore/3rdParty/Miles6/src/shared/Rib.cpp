//############################################################################
//##                                                                        ##
//##  RIB.CPP: RAD Interface Broker API services                            ##
//##                                                                        ##
//##  16-bit protected-mode source compatible with MSC 7.0                  ##
//##  32-bit protected-mode source compatible with MSC 11.0/Watcom 10.6     ##
//##                                                                        ##
//##  Version 1.00 of 20-Apr-98: Initial                                    ##
//##          1.01 of 25-Jan-00: Moved to standalone DLL for Eden           ##
//##                             Don't enumerate NULL providers if no       ##
//##                             specific interface requested               ##
//##          1.02 of 19-Nov-00: Return RIB_NOT_FOUND if requested          ##
//##                             interface entirely unsupported             ##
//##          1.03 of 29-Jun-01: Library provider handles are ref-counted   ##
//##                                                                        ##
//##  Author: John Miles                                                    ##
//##                                                                        ##
//############################################################################
//##                                                                        ##
//##  Copyright (C) RAD Game Tools, Inc.                                    ##
//##                                                                        ##
//##  Contact RAD Game Tools at 425-893-4300 for technical support.         ##
//##                                                                        ##
//############################################################################

#include <stdlib.h>
#include <string.h>

#include "rib.h"
#include "mss.h"
#include "imssapi.h"

#ifdef IS_WIN16
#include <dos.h>
#endif

#ifdef IS_MAC
#include <CodeFragments.h>
#include <TextUtils.h>
#endif

extern char MSS_directory[256];

#define DIAG_REQUEST 0                  // 1 to print interface requests and results

#ifndef MAX_PATH
#define MAX_PATH 512
#endif

C8 error_text[256];

#define INTERFACE_NAME_HASH_SIZE 9      // Use 9-bit hash table for entry names
#define INTERFACE_BLOCK_SIZE     128    // Allocate 128 new entries at a time
#define INTERFACE_SET_DIM (1 << INTERFACE_NAME_HASH_SIZE)

#define PROVIDER_BLOCK_SIZE      32     // Allocate 32 new entries at a time

struct INTERFACE_DESCRIPTOR
{
   HPROVIDER          provider;
   C8                 interface_name[128];
   C8                 entry_name    [128];
   RIB_ENTRY_TYPE     type;
   RIB_DATA_SUBTYPE   subtype;
   U32                token;
};

//
// Interface list entry
//

struct ILIST_ENTRY
{
   public:
   ILIST_ENTRY FAR *next;
   ILIST_ENTRY FAR *prev;
   ILIST_ENTRY FAR *next_in_set;
   ILIST_ENTRY FAR *prev_in_set;
   S32              set_key;
   S32              entry_num;

   void init(INTERFACE_DESCRIPTOR FAR *src)
      {
      contents = *src;
      }

   void cleanup(void)
      {
      }

   S32 is_match(INTERFACE_DESCRIPTOR FAR *src)
      {
      if (!src->interface_name[0])
         {
         return (src->type == contents.type) &&
                (src->provider == contents.provider) &&
                (!AIL_strcmp(src->entry_name, contents.entry_name));
         }

      return (src->type == contents.type) &&
             (src->provider == contents.provider) &&
             (!AIL_strcmp(src->interface_name, contents.interface_name)) &&
             (!AIL_strcmp(src->entry_name, contents.entry_name));
      }

   static U32 set(INTERFACE_DESCRIPTOR FAR *src)
      {
      //
      // Derive hash key from entry name
      //

#ifdef IS_WIN32

	   U32 sum;
      U8 FAR *entry_name = (U8 FAR *) src->entry_name;

	   _asm
	      {
	      mov ebx,entry_name
	      xor eax,eax
	      xor edx,edx

	      ALIGN 16
      chksum:
	      mov dl,BYTE PTR [ebx]
	      add eax,edx
	      inc ebx
	      test edx,edx
	      jnz chksum

	      mov sum,eax
	      }
#else

	   U32 sum;

	   U8 FAR *ptr = (U8 FAR *) src->entry_name;
	   sum     = 0;

	   while (*ptr)
	      {
		   sum += *(ptr++);
	      }

#endif

	   return sum & (INTERFACE_SET_DIM - 1);
      }

   //
   // User data
   //

   INTERFACE_DESCRIPTOR contents;
};

//
// Interface entry list (hashed)
//

class InterfaceList
{
public:
   S32              current_size;
   ILIST_ENTRY FAR *used;
   ILIST_ENTRY FAR *avail;
   ILIST_ENTRY FAR *table[INTERFACE_SET_DIM];
   ILIST_ENTRY FAR *array;

   void construct(void)
      {
      current_size = INTERFACE_BLOCK_SIZE;

      array = (ILIST_ENTRY FAR *)
         AIL_mem_alloc_lock(sizeof(ILIST_ENTRY) * (size_t) current_size);

      S32 i;
      for (i=0; i < current_size; i++)
         {
         array[i].prev = &array[i-1];
         array[i].next = &array[i+1];
         array[i].entry_num = -1;
         }

      array[0].prev = NULL;
      array[current_size-1].next = NULL;

      used = NULL;
      avail = &array[0];

      for (i=0; i < INTERFACE_SET_DIM; i++) table[i] = NULL;
      }

   void destroy(void)
      {
      if (array)
         {
		   while (used) dealloc(used);

		   current_size = 0;
		   AIL_mem_free_lock(array);
		   array = NULL;
         }
      }

   ILIST_ENTRY FAR *verify_interface(HPROVIDER provider, C8 const *name)
      {
      S32 i;
      S32 n = 0;

      for (i=0; i < current_size; i++)
         {
         if (array[i].entry_num != -1)
            {
            if (provider == array[i].contents.provider)
               {
               if (!AIL_strcmp(name, array[i].contents.interface_name))
                  {
                  return &array[i];
                  }
               }
            }
         }

      return NULL;
      }

   ILIST_ENTRY FAR *lookup(INTERFACE_DESCRIPTOR FAR *seed)
      {
      ILIST_ENTRY FAR *found = table[ILIST_ENTRY::set(seed)];

      while (found)
         {
         if (found->is_match(seed))
            break;

         found = found->next_in_set;
         }

      return found;
      }

   ILIST_ENTRY FAR *alloc(INTERFACE_DESCRIPTOR FAR *seed)
      {
      U32 key = ILIST_ENTRY::set(seed);
      ILIST_ENTRY FAR *entry;

      if (avail == NULL)
         {
         S32 prev_size = current_size;
         S32 new_size = prev_size + INTERFACE_BLOCK_SIZE;

         ILIST_ENTRY FAR *old_array = array;
         ILIST_ENTRY FAR *new_array = (ILIST_ENTRY FAR *)
            AIL_mem_alloc_lock(sizeof(ILIST_ENTRY) * (size_t) new_size);
         if (new_array == NULL) return NULL;

         U32 adjust = ((U32) new_array) - ((U32) old_array);

         S32 i;
         for (i=0; i < prev_size; i++)
            {
            new_array[i] = old_array[i];

            if (new_array[i].next_in_set) new_array[i].next_in_set = (ILIST_ENTRY FAR *) (((U32) new_array[i].next_in_set) + adjust);
            if (new_array[i].prev_in_set) new_array[i].prev_in_set = (ILIST_ENTRY FAR *) (((U32) new_array[i].prev_in_set) + adjust);

            if (new_array[i].next) new_array[i].next = (ILIST_ENTRY FAR *) (((U32) new_array[i].next) + adjust);
            if (new_array[i].prev) new_array[i].prev = (ILIST_ENTRY FAR *) (((U32) new_array[i].prev) + adjust);
            }

         for (i=0; i < INTERFACE_SET_DIM; i++)
            if (table[i]) table[i] = (ILIST_ENTRY FAR *) (((U32) table[i]) + adjust);

         for (i=prev_size; i < new_size; i++)
            {
            new_array[i].entry_num = -1;
            new_array[i].prev = &new_array[i-1];
            new_array[i].next = &new_array[i+1];
            }

         new_array[prev_size].prev = NULL;
         new_array[new_size-1].next = NULL;
         used = (ILIST_ENTRY FAR *) (((U32) used) + adjust);
         avail = &new_array[prev_size];
         current_size = new_size;

         AIL_mem_free_lock(array);
         array = new_array;
         }

      entry = avail;
      avail = entry->next;
      entry->next = used;
      entry->prev = NULL;

      if (used) used->prev = entry;
      used = entry;

      entry->set_key = key;
      entry->next_in_set = table[key];
      entry->prev_in_set = NULL;

      if (table[key])
         table[key]->prev_in_set = entry;

      table[key] = entry;
      entry->entry_num = (S32) ((((U32) entry) - ((U32) array)) / sizeof(ILIST_ENTRY));

      entry->init(seed);

      return entry;
      }

   void dealloc(ILIST_ENTRY FAR *entry)
      {
      entry->cleanup();

      if (entry->next_in_set) entry->next_in_set->prev_in_set = entry->prev_in_set;
      if (entry->prev_in_set) entry->prev_in_set->next_in_set = entry->next_in_set;

      entry->entry_num = -1;

      if (entry->next) entry->next->prev = entry->prev;
      if (entry->prev) entry->prev->next = entry->next;

      if (used == entry) used = entry->next;

      if (table[entry->set_key] == entry)
         table[entry->set_key] = entry->next_in_set;

      entry->next = avail;
      entry->prev = NULL;

      if (avail) avail->prev = entry;
      avail = entry;
      }

   void dealloc(S32 entry_num)
      {
      if ((entry_num != -1) && (array[entry_num].entry_num != -1))
         dealloc(&array[entry_num]);
      }
};

//
// Provider list entry
//

struct PVLIST_ENTRY
{
   PVLIST_ENTRY FAR *next;
   PVLIST_ENTRY FAR *prev;
   S32               entry_num;

   //
   // User data
   //

   U32 module;              // HMODULE if DLL, not used if static
   char filename[512];      // Module name for which the handle was allocated
   S32 refcnt;              // # of times associated module name has been loaded
   S32 user_data  [8];      // Miscellaneous user data
   S32 system_data[8];      // Miscellaneous system data
};

//
// Provider list (non-hashed)
//

class ProviderList
{
public:
   S32               current_size;
   PVLIST_ENTRY FAR *used;
   PVLIST_ENTRY FAR *avail;
   PVLIST_ENTRY FAR *array;

   void construct(void)
      {
      current_size = PROVIDER_BLOCK_SIZE;

      array = (PVLIST_ENTRY FAR *)
         AIL_mem_alloc_lock(sizeof(PVLIST_ENTRY) * (size_t) current_size);

      for (S32 i=0; i < current_size; i++)
         {
         array[i].prev = &array[i-1];
         array[i].next = &array[i+1];
         array[i].entry_num = -1;
         }

      array[0].prev = NULL;
      array[current_size-1].next = NULL;

      used = NULL;
      avail = &array[0];
      }

   void destroy(void)
      {
      if (array)
         {
		   while (used) dealloc(used);

		   current_size = 0;
		   AIL_mem_free_lock(array);
		   array = NULL;
         }
      }

   PVLIST_ENTRY FAR *alloc(void)
      {
      PVLIST_ENTRY FAR *entry;

      if (avail == NULL)
         {
         S32 prev_size = current_size;
         S32 new_size = prev_size + PROVIDER_BLOCK_SIZE;

         PVLIST_ENTRY FAR *old_array = array;
         PVLIST_ENTRY FAR *new_array = (PVLIST_ENTRY FAR *)
            AIL_mem_alloc_lock(sizeof(PVLIST_ENTRY) * (size_t) new_size);
         if (new_array == NULL) return NULL;

         U32 adjust = ((U32) new_array) - ((U32) old_array);

         S32 i;
         for (i=0; i < prev_size; i++)
            {
            new_array[i] = old_array[i];

            if (new_array[i].next) new_array[i].next = (PVLIST_ENTRY FAR *) (((U32) new_array[i].next) + adjust);
            if (new_array[i].prev) new_array[i].prev = (PVLIST_ENTRY FAR *) (((U32) new_array[i].prev) + adjust);
            }

         for (i=prev_size; i < new_size; i++)
            {
            new_array[i].entry_num = -1;
            new_array[i].prev = &new_array[i-1];
            new_array[i].next = &new_array[i+1];
            }

         new_array[prev_size].prev = NULL;
         new_array[new_size-1].next = NULL;
         used = (PVLIST_ENTRY FAR *) (((U32) used) + adjust);
         avail = &new_array[prev_size];
         current_size = new_size;

         AIL_mem_free_lock(array);
         array = new_array;
         }

      entry = avail;
      avail = entry->next;
      entry->next = used;
      entry->prev = NULL;

      if (used) used->prev = entry;
      used = entry;

      entry->entry_num = (S32) ((((U32) entry) - ((U32) array)) / sizeof(PVLIST_ENTRY));

      return entry;
      }

   void dealloc(PVLIST_ENTRY FAR *entry)
      {
      entry->entry_num = -1;

      if (entry->next) entry->next->prev = entry->prev;
      if (entry->prev) entry->prev->next = entry->next;

      if (used == entry) used = entry->next;

      entry->next = avail;
      entry->prev = NULL;

      if (avail) avail->prev = entry;
      avail = entry;
      }

   void dealloc(S32 entry_num)
      {
      if ((entry_num != -1) && (array[entry_num].entry_num != -1))
         dealloc(&array[entry_num]);
      }
};

//############################################################################
//#                                                                          #
//# Globals                                                                  #
//#                                                                          #
//# These don't need to be locked for DOS or Win16, since RIB functions      #
//# are not supported in callbacks                                           #
//#                                                                          #
//############################################################################

InterfaceList interface_list;
ProviderList  provider_list;

S32 list_initialized = 0;

HPROVIDER current_provider = 0;

//############################################################################
//#                                                                          #
//# Allocate an HPROVIDER descriptor                                         #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_alloc_provider_handle (U32 module)
{
   //
   // Initialize interface list, if not already done
   //

   if (!list_initialized++)
      {
      interface_list.construct();
      provider_list.construct();

      //
      // Mark entry 0 as permanently in use
      // so that NULL HPROVIDERs can be treated
      // as errors
      //

      provider_list.alloc();
      }                          

   //
   // Allocate provider entry with zero references
   //

   PVLIST_ENTRY FAR *PROVIDER = provider_list.alloc();

   PROVIDER->module = module;
   
   provider_list.array[ PROVIDER->entry_num ].refcnt = 0;
   provider_list.array[ PROVIDER->entry_num ].filename[0] = 0;

   return PROVIDER->entry_num;
}

//############################################################################
//#                                                                          #
//# Free an HPROVIDER descriptor                                             #
//#                                                                          #
//############################################################################

RIBDEF void RIBEXPORT RIB_free_provider_handle (HPROVIDER provider)
{
   provider_list.dealloc(provider);

   //
   // Destroy interface list when last provider freed
   //

   if (list_initialized)
      {
      if (!--list_initialized)
         {
         interface_list.destroy();
         provider_list.destroy();
         }
      }
}

//############################################################################
//#                                                                          #
//# Load a RIB-compatible library                                            #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_load_provider_library (C8 const FAR *filename)
{
   //
   // See if we've already allocated a provider handle for this
   // module
   //

   HPROVIDER provider = 0;
   U32       index = 0;

   while ((list_initialized) &&
            (index < (U32) provider_list.current_size))
      {
      PVLIST_ENTRY *p = &provider_list.array[index];

      if ((index != 0) && (p->entry_num != -1))
         {
         if (!AIL_stricmp(filename,p->filename))
            {
            provider = index;
            break;
            }
         }

      index++;
      }

   //
   // If not. allocate provider entry
   //

   if (provider == 0)
      {
      provider = RIB_alloc_provider_handle(0);

      AIL_strcpy(provider_list.array[provider].filename,
             filename);
      }

   //
   // Set current provider variable for access by library's DllMain()
   // function
   //

   current_provider = provider;

#ifdef IS_WINDOWS

   //
   // Load specified library
   //

   WORD err=SetErrorMode(0x8000);

   HMODULE module = LoadLibrary(filename);

   SetErrorMode(err);

   if (module == NULL)
      {
      RIB_free_provider_handle(provider);
      return NULL;
      }

   provider_list.array[provider].module = (U32)(void FAR*)module;

#else

#ifdef IS_MAC

	CFragConnectionID module;
	CFragSymbolClass theClass;
	Ptr				mainaddr;
	FSSpec			theSpec;
	char fn[256];
	fn[0]=AIL_strlen(filename);
	AIL_memcpy(fn+1,filename,fn[0]);
	OSErr err = 	::FSMakeFSSpec( 0, 0, (U8 FAR*)fn, &theSpec );
	Str63			theOptName = "\p";
	Str63			theErrName = "\p";

	if(err)
	{
	  RIB_free_provider_handle(provider);
	  return NULL;
	}

	err = ::GetDiskFragment( &theSpec, 0/*with begining*/, 0 /*whole data fork*/,
					theOptName, kReferenceCFrag, &module,
					&mainaddr, theErrName );

	if(err)
	{
	  RIB_free_provider_handle(provider);
	  return NULL;
	}
	
    typedef void (*INIT)(HPROVIDER provider, S32 up_down);
	INIT init;

	err = ::FindSymbol( module, "\pMSS_RIB_Main", (Ptr*)&init, &theClass );
	if(err)
	{
	  RIB_free_provider_handle(provider);
	  return NULL;
	}
	
	init(provider,1);

    provider_list.array[provider].module = (U32)module;
#else

   filename = filename;

   provider_list.array[provider].module = 0;

#endif

#endif

   //
   // Bump provider's reference count and return it
   //

   ++provider_list.array[provider].refcnt;

   return provider;
}

//############################################################################
//#                                                                          #
//# Free a RIB-compatible library                                            #
//#                                                                          #
//############################################################################

RIBDEF void RIBEXPORT RIB_free_provider_library (HPROVIDER provider)
{
#if defined(IS_WINDOWS) || defined(IS_MAC)

   if (provider == (HPROVIDER) NULL)
      {
      //
      // Free all providers
      //

      U32 index = 0;

      while ((list_initialized) &&
             (index < (U32) provider_list.current_size))
         {
         PVLIST_ENTRY *p = &provider_list.array[index];

         if ((index != 0) && (p->entry_num != -1))
            {
            RIB_free_provider_library((HPROVIDER) index);
            }

         index++;
         }
      }
   else
      {
      //
      // Free specified provider
      //

      if (provider_list.array[provider].module != 0)
         {
         #ifdef IS_WINDOWS
           FreeLibrary((HMODULE) (provider_list.array[provider].module));
         #else
           ::CloseConnection((CFragConnectionID *)&(provider_list.array[provider].module));
         #endif
         }

      //
      // Decrement reference count on this provider handle, freeing
      // handle when refcnt <= 0
      //

      --provider_list.array[provider].refcnt;

      if (provider_list.array[provider].refcnt <= 0)
         {
         RIB_free_provider_handle(provider);
         }
      }

#else
   provider = provider;

#endif
}

//############################################################################
//#                                                                          #
//# Get provider handle for last loaded library                              #
//#                                                                          #
//# This allows DLL providers to obtain the correct provider handle for      #
//# interface registration within DllMain()                                  #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_provider_library_handle(void)
{
   return current_provider;
}

//############################################################################
//#                                                                          #
//# Handle data or code interface registration                               #
//#                                                                          #
//############################################################################

RIBDEF RIBRESULT RIBEXPORT RIB_register_interface (HPROVIDER                      provider, //)
                                                   C8 const                  FAR *interface_name,
                                                   S32                            entry_count,
                                                   RIB_INTERFACE_ENTRY const FAR *list)
{
   //
   // Register all entries in list
   //

   INTERFACE_DESCRIPTOR temp;

   temp.provider = provider;
   AIL_strcpy(temp.interface_name, interface_name);

   for (S32 i=0; i < entry_count; i++)
      {
      AIL_strcpy(temp.entry_name, list[i].entry_name);

      temp.type    = list[i].type;
      temp.subtype = list[i].subtype;
      temp.token   = list[i].token;

      interface_list.alloc(&temp);
      }

   return RIB_NOERR;
}

//############################################################################
//#                                                                          #
//# Handle requests for data or code interfaces                              #
//#                                                                          #
//############################################################################

RIBDEF RIBRESULT RIBEXPORT RIB_request_interface  (HPROVIDER                provider, //)
                                                   C8 const FAR            *interface_name,
                                                   S32                      entry_count,
                                                   RIB_INTERFACE_ENTRY FAR *list)
{
   //
   // Build descriptor for specified request
   //

   INTERFACE_DESCRIPTOR temp;

   temp.provider = provider;
   AIL_strcpy(temp.interface_name, interface_name);

   //
   // See if provider supports any entries for specified interface
   //

   if (interface_list.verify_interface(provider,
                                       interface_name) == NULL)
      {
      return RIB_NOT_FOUND;
      }

   //
   // Resolve all entries in list
   //

   S32 misses = 0;

   for (S32 i=0; i < entry_count; i++)
      {
#if DIAG_REQUEST
      AIL_debug_printf("RIB: Looking for %s... ",list[i].entry_name);
#endif

      AIL_strcpy(temp.entry_name, list[i].entry_name);
      temp.type = list[i].type;

      ILIST_ENTRY FAR *entry = interface_list.lookup(&temp);

      if (entry == NULL)
         {
#if DIAG_REQUEST
         AIL_debug_printf("Failed!\n");
#endif
         if (list[i].type == RIB_FUNCTION)
            {
            //
            // Failed function lookups return NULL pointer
            //

            *(U32 FAR *) (list[i].token) = 0;
            }
         else
            {
            //
            // Failed pref/attrib token lookups return -1
            //

            *(S32 FAR *) (list[i].token) = -1;
            }

         ++misses;
         }
      else
         {
#if DIAG_REQUEST
         AIL_debug_printf("Found %d (%X)\n",entry->contents.token,
            entry->contents.token);
#endif
         *(U32 FAR *) (list[i].token) = entry->contents.token;
         }
      }

   //
   // Return RIB_NOT_ALL_AVAILABLE if one or more entries could not be
   // resolved
   //
   // These entries are set to NULL (0)
   //

   if (!misses)
      {
      return RIB_NOERR;
      }
   else
      {
      AIL_strcpy(error_text,"One or more requested entries not found");
      return RIB_NOT_ALL_AVAILABLE;
      }
}

//############################################################################
//#                                                                          #
//# Handle request for single data or code interface entry                   #
//#                                                                          #
//############################################################################

RIBDEF RIBRESULT RIBEXPORT RIB_request_interface_entry  (HPROVIDER       provider, //)
                                                         C8 const FAR   *interface_name,
                                                         RIB_ENTRY_TYPE  entry_type,
                                                         C8 const FAR   *entry_name,
                                                         U32 FAR        *token)
{
   //
   // Build descriptor for specified request
   //

   INTERFACE_DESCRIPTOR temp;
   
   temp.provider = provider;
   temp.type     = entry_type;

   AIL_strcpy(temp.interface_name, interface_name);
   AIL_strcpy(temp.entry_name,     entry_name);

#if DIAG_REQUEST
      AIL_debug_printf("RIB: Looking for entry %s... ",entry_name);
#endif

   //
   // Resolve requested entry
   //

   ILIST_ENTRY FAR *entry = interface_list.lookup(&temp);

   if (entry == NULL)
      {
      *token = 0;
#if DIAG_REQUEST
         AIL_debug_printf("Failed!\n");
#endif
      return RIB_NOT_FOUND;
      }
   else
      {
#if DIAG_REQUEST
         AIL_debug_printf("Found %d (%X)\n",entry->contents.token,
            entry->contents.token);
#endif

      *token = entry->contents.token;
      return RIB_NOERR;
      }
}

//############################################################################
//#                                                                          #
//# Unregister interface entries                                             #
//#                                                                          #
//############################################################################

RIBDEF RIBRESULT RIBEXPORT RIB_unregister_interface  (HPROVIDER                      provider, //)
                                                      C8 const FAR                  *interface_name,
                                                      S32                            entry_count,
                                                      RIB_INTERFACE_ENTRY const FAR *list)
{
   //
   // Resolve all entries in list
   //

   S32 misses = 0;

   if ((list != NULL) && (interface_name != NULL))
      {
      //
      // Build descriptor for specified interface
      //

      INTERFACE_DESCRIPTOR temp;
   
      temp.provider = provider;
      AIL_strcpy(temp.interface_name, interface_name);

      //
      // Remove only listed entries
      //

      for (S32 i=0; i < entry_count; i++)
         {
         AIL_strcpy(temp.entry_name, list[i].entry_name);
         temp.type = list[i].type;

         ILIST_ENTRY FAR *entry = interface_list.lookup(&temp);

         if (entry == NULL)
            {
            ++misses;
            }
         else
            {
            interface_list.dealloc(entry);
            }
         }
      }
   else
      {
      //
      // No valid list specified -- remove all entries that match provider
      // and interface name
      //

      for (S32 set=0; set < INTERFACE_SET_DIM; set++)
         {
         ILIST_ENTRY FAR *found = interface_list.table[set];

         while (found)
            {
            ILIST_ENTRY FAR *next = found->next_in_set;

            if (found->contents.provider == provider)
               {
               if ((interface_name == NULL) || 
                   (!AIL_strcmp(interface_name, found->contents.interface_name)))
                   {
                   interface_list.dealloc(found);
                   }
               }

            found = next;
            }
         }
      }

   //
   // Return RIB_NOT_ALL_AVAILABLE if one or more entries were not found
   //

   if (!misses)
      {
      return RIB_NOERR;
      }
   else
      {
      AIL_strcpy(error_text,"One or more entries not found");
      return RIB_NOT_ALL_AVAILABLE;
      }
}

//############################################################################
//#                                                                          #
//# Enumerate interface entries                                              #
//#                                                                          #
//############################################################################

RIBDEF S32 RIBEXPORT RIB_enumerate_interface(HPROVIDER                provider, //)
                                             C8 const FAR            *interface_name,
                                             RIB_ENTRY_TYPE           type,
                                             HINTENUM  FAR           *next,
                                             RIB_INTERFACE_ENTRY FAR *dest)
{
   S32              set;
   ILIST_ENTRY FAR *entry;

   if (*next == HINTENUM_FIRST)
      {
      //
      // Begin search at start of hash space
      //

      set = 0;
      entry = interface_list.table[set];
      }
   else
      {
      //
      // Begin search at next non-empty entry 
      //

      set = (*next) & (INTERFACE_SET_DIM - 1);
      entry = &interface_list.array[(*next) >> INTERFACE_NAME_HASH_SIZE];

      entry = entry->next_in_set;

      while ((entry == NULL) && (set < INTERFACE_SET_DIM))
         {
         entry = interface_list.table[++set];
         }
      }

   //
   // Find next qualified entry
   //

   while (set < INTERFACE_SET_DIM)
      {
      while (entry)
         {
         if (((provider == NULL) || (entry->contents.provider == provider)) &&
             (entry->contents.type == type) &&
             (!AIL_strcmp(entry->contents.interface_name,
                          interface_name)))
            {
            //
            // Qualified entry found -- pack hash set and index # 
            // into U32 and return entry information
            //

            *next = (entry->entry_num << INTERFACE_NAME_HASH_SIZE) | set;

            dest->type       = entry->contents.type;
            dest->entry_name = entry->contents.entry_name;
            dest->token      = entry->contents.token;
            dest->subtype    = entry->contents.subtype;

            return 1;
            }

         entry = entry->next_in_set;
         }

      entry = interface_list.table[++set];
      }

   //
   // No qualified entries found -- reset pointer to start of list and
   // return 0
   //

   *next = HINTENUM_FIRST;
   return 0;
}

//############################################################################
//#                                                                          #
//# Enumerate available providers of specified interface, or of all          #
//# interfaces if interface_name = NULL                                      #
//#                                                                          #
//############################################################################

RIBDEF S32 RIBEXPORT RIB_enumerate_providers     (C8 FAR        *interface_name, //)
                                                  HPROENUM FAR  *next,
                                                  HPROVIDER FAR *dest)
{
   HPROVIDER index;

   HPROENUM dummy = HPROENUM_FIRST;

   if (next == NULL)
      {
      next = &dummy;
      }

   if (*next == HPROENUM_FIRST)
      {
      //
      // Skip dummy first entry
      //

      index = 1;
      }
   else
      {
      index = (*next) + 1;
      }

   //
   // Find next qualified entry
   //

   while (index < (U32) provider_list.current_size)
      {
      S32 qual = 0;

      if (interface_name == NULL)
         {
         S32              set = 0;
         ILIST_ENTRY FAR *entry = interface_list.table[set];

         while ((set < INTERFACE_SET_DIM) && (!qual))
            {
            while (entry)
               {
               if (entry->contents.provider == index)
                  {
                  qual = 1;
                  break;
                  }

               entry = entry->next_in_set;
               }

            entry = interface_list.table[++set];
            }
         }
      else
         {
         S32              set = 0;
         ILIST_ENTRY FAR *entry = interface_list.table[set];

         while ((set < INTERFACE_SET_DIM) && (!qual))
            {
            while (entry)
               {
               if ((entry->contents.provider == index) &&
                   (!AIL_strcmp(entry->contents.interface_name,
                                interface_name)))
                  {
                  qual = 1;
                  break;
                  }

               entry = entry->next_in_set;
               }

            entry = interface_list.table[++set];
            }
         }

      //
      // Return success if match
      //

      if (qual)
         {
         *next = index;
         *dest = index;
         return 1;
         }

      //
      // Otherwise, keep looking...
      //

      ++index;
      }

   //
   // No qualified entries found -- reset pointer to start of list and
   // return 0
   //

   *next = HPROENUM_FIRST;
   *dest = NULL;
   return 0;
}

//############################################################################
//#                                                                          #
//# Return pointer to static string containing value formatted according to  #
//# its type                                                                 #
//#                                                                          #
//############################################################################

RIBDEF C8 FAR * RIBEXPORT RIB_type_string (U32              data, //)
                                           RIB_DATA_SUBTYPE subtype)
{
   static C8 string[256];

   switch (subtype)
      {
      case RIB_HEX:     AIL_sprintf(string,"0x%X",data);              break;
      case RIB_FLOAT:   
         {
         F32 val;

         *(U32 FAR *) (&val) = data;

         AIL_sprintf(string,"%f",val);
         }
         break;

      case RIB_PERCENT:  
         {
         F32 val;

         *(U32 FAR *) (&val) = data;

         AIL_sprintf(string,"%3.1f%%",val);
         }
         break;

      case RIB_BOOL:    AIL_sprintf(string, data ? "True" : "False"); break;
      case RIB_STRING:  AIL_sprintf(string,"%s",data);                break;
      default:          AIL_sprintf(string,"%d",data);                break;
      }

   return string;
}

// match a file suffix

static S32 match_suffix(char const FAR* suffix,char const FAR* file_suffix)
{
  //
  // Scan Windows-compatible filespec string to see if specified
  // suffix is supported
  //

  S32 tl = strlen(file_suffix);

  S32 i;
  for (i=tl-1; i > 0; i--)
     {
     if (file_suffix[i] == '.')
        {
        break;
        }
     }

  C8 const FAR *search_suffix = &file_suffix[i];
  tl                    = strlen(search_suffix);

  while (1)
     {
     S32 sl = AIL_strlen(suffix);

     suffix = &suffix[sl+1];

     sl = AIL_strlen(suffix);

     if (sl == 0)
        {
        break;
        }

     if (sl >= tl)
        {
        //
        // Suffix length is ("*.xxx") longer than requested search
        // suffix (".xxx" or "xxx")
        //

        if (!AIL_stricmp(&suffix[sl-tl],
                          search_suffix))
           {
           return 1;
           }
        }

     suffix = &suffix[sl+1];

     if (!*suffix)
        {
        break;
        }
     }
  return(0);
}


//############################################################################
//#                                                                          #
//# Find provider of specified interface which is capable of supporting the  #
//# specified file type                                                      #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_find_file_provider (C8 const FAR *interface_name, //)
                                                   C8 const FAR *attribute_name,
                                                   C8 const FAR *file_suffix)
{
   S32 set = 0;
   ILIST_ENTRY FAR *entry = interface_list.table[set];

   while (set < INTERFACE_SET_DIM)
      {
      while (entry)
         {
         //
         // Find specified interface's PROVIDER_query_attribute() function
         // entry
         //

         if ((entry->contents.type ==                     RIB_FUNCTION) &&
             (!AIL_strcmp(entry->contents.interface_name, interface_name)) &&
             (!AIL_strcmp(entry->contents.entry_name,    "PROVIDER_query_attribute")))
            {
            //
            // Get address of attribute-query handler
            //

            PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute =
               (PROVIDER_QUERY_ATTRIBUTE) entry->contents.token;

            //
            // Get token for specified filespec attribute
            //

            HATTRIB token;

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name,
                                             &token))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            //
            // Get filespec string value
            //

            C8 FAR *suffix = (C8 FAR *) PROVIDER_query_attribute(token);

            if (match_suffix(suffix,file_suffix))
              return entry->contents.provider;
            
            }

         //
         // Move to next entry in hash set
         //

         entry = entry->next_in_set;
         }

      //
      // Move to next hash set
      //

      entry = interface_list.table[++set];
      }

   return NULL;
}


//############################################################################
//#                                                                          #
//# Find provider of specified interface which is capable of supporting the  #
//# specified file type for in and out                                       #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_find_files_provider (C8 const FAR *interface_name, //)
                                                    C8 const FAR *attribute_name_1,
                                                    C8 const FAR *file_suffix_1,
                                                    C8 const FAR *attribute_name_2,
                                                    C8 const FAR *file_suffix_2)
{
   S32 set = 0;
   ILIST_ENTRY FAR *entry = interface_list.table[set];

   while (set < INTERFACE_SET_DIM)
      {
      while (entry)
         {
         //
         // Find specified interface's PROVIDER_query_attribute() function
         // entry
         //

         if ((entry->contents.type ==                     RIB_FUNCTION) &&
             (!AIL_strcmp(entry->contents.interface_name, interface_name)) &&
             (!AIL_strcmp(entry->contents.entry_name,    "PROVIDER_query_attribute")))
            {
            //
            // Get address of attribute-query handler
            //

            PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute =
               (PROVIDER_QUERY_ATTRIBUTE) entry->contents.token;

            //
            // Get token for specified filespec attribute
            //

            HATTRIB token_1,token_2;

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name_1,
                                             &token_1))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name_2,
                                             &token_2))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            //
            // Get filespec string value
            //

            C8 FAR *suffix_1 = (C8 FAR *) PROVIDER_query_attribute(token_1);
            C8 FAR *suffix_2 = (C8 FAR *) PROVIDER_query_attribute(token_2);

            if ((match_suffix(suffix_1,file_suffix_1)) &&
                (match_suffix(suffix_2,file_suffix_2)))
              return entry->contents.provider;

            }

         //
         // Move to next entry in hash set
         //

         entry = entry->next_in_set;
         }

      //
      // Move to next hash set
      //

      entry = interface_list.table[++set];
      }

   return NULL;
}


//############################################################################
//#                                                                          #
//# Find provider of specified interface which is capable of supporting the  #
//# specified file wave format tage and the output file type                 #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_find_file_dec_provider (C8 const FAR *interface_name, //)
                                                       C8 const FAR *attribute_name_1,
                                                       U32 attribute_value_1,
                                                       C8 const FAR *attribute_name_2,
                                                       C8 const FAR *file_suffix_2)
{
   S32 set = 0;
   ILIST_ENTRY FAR *entry = interface_list.table[set];

   while (set < INTERFACE_SET_DIM)
      {
      while (entry)
         {
         //
         // Find specified interface's PROVIDER_query_attribute() function
         // entry
         //

         if ((entry->contents.type ==                     RIB_FUNCTION) &&
             (!AIL_strcmp(entry->contents.interface_name, interface_name)) &&
             (!AIL_strcmp(entry->contents.entry_name,    "PROVIDER_query_attribute")))
            {
            //
            // Get address of attribute-query handler
            //

            PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute =
               (PROVIDER_QUERY_ATTRIBUTE) entry->contents.token;

            //
            // Get token for specified filespec attribute
            //

            HATTRIB token_1,token_2;

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name_1,
                                             &token_1))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name_2,
                                             &token_2))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            //
            // Get filespec string value
            //

            U32 value1 = PROVIDER_query_attribute(token_1);
            C8 FAR *suffix_2 = (C8 FAR *) PROVIDER_query_attribute(token_2);

            if ((attribute_value_1==value1) &&
                (match_suffix(suffix_2,file_suffix_2)))
              return entry->contents.provider;

            }

         //
         // Move to next entry in hash set
         //

         entry = entry->next_in_set;
         }

      //
      // Move to next hash set
      //

      entry = interface_list.table[++set];
      }

   return NULL;
}


//############################################################################
//#                                                                          #
//# Find provider of specified interface which has the matching attribute    #
//#                                                                          #
//############################################################################

RIBDEF HPROVIDER RIBEXPORT RIB_find_provider (C8 const FAR *interface_name, //)
                                              C8 const FAR *attribute_name,
                                              C8 const FAR *attribute_value)
{
   S32 set = 0;
   ILIST_ENTRY FAR *entry = interface_list.table[set];

   while (set < INTERFACE_SET_DIM)
      {
      while (entry)
         {
         //
         // Find specified interface's PROVIDER_query_attribute() function
         // entry
         //

         if ((entry->contents.type ==                     RIB_FUNCTION) &&
             (!AIL_strcmp(entry->contents.interface_name, interface_name)) &&
             (!AIL_strcmp(entry->contents.entry_name,    "PROVIDER_query_attribute")))
            {
            //
            // Get address of attribute-query handler
            //

            PROVIDER_QUERY_ATTRIBUTE PROVIDER_query_attribute =
               (PROVIDER_QUERY_ATTRIBUTE) entry->contents.token;

            //
            // Get token for specified attribute
            //

            HATTRIB token;

            if (RIB_request_interface_entry  (entry->contents.provider,
                                              entry->contents.interface_name,
                                              RIB_ATTRIBUTE,
                                              attribute_name,
                                             &token))
               {
               //
               // Skip if specified attribute not exported by provider
               //

               entry = entry->next_in_set;
               continue;
               }

            //
            // Get string value
            //

            C8 FAR *value = (C8 FAR *) PROVIDER_query_attribute(token);


            //
            //  do we match?
            //

            if (AIL_stricmp(value,attribute_value)==0)
              return entry->contents.provider;


            }

         //
         // Move to next entry in hash set
         //

         entry = entry->next_in_set;
         }

      //
      // Move to next hash set
      //

      entry = interface_list.table[++set];
      }

   return NULL;
}

//############################################################################
//#                                                                          #
//# Load all RIB provider libraries from application-relative directory      #
//#                                                                          #
//############################################################################

#if defined(IS_WINDOWS) || defined(IS_MAC)

static S32 load_application_providers(char const FAR* dir, char const FAR *in_filespec)
{
#ifdef IS_MAC
////////////////////////////////////////////////////////////
  S32 n = 0;
  CInfoPBRec    filespec;                          /* local pb */
  HFileInfo     *fpb = (HFileInfo *)&filespec;     /* to pointers */
  DirInfo       *dpb = (DirInfo *) &filespec;
  short  idx;
  char name_buffer[256];
  FSSpec fsSpec;
  
  name_buffer[0]=AIL_strlen(dir);
  AIL_memcpy(name_buffer+1,dir,name_buffer[0]);

  OSErr err = ::FSMakeFSSpec(0, 0, (unsigned char *)name_buffer, &fsSpec);
  
  dpb->ioNamePtr = (U8 FAR*)name_buffer;
  fpb->ioFDirIndex = 0;
  PBGetCatInfo(&filespec, FALSE);

  if(dpb->ioResult)
    return 0;
  
  fpb->ioFDirIndex = -1;
  long constDirID = dpb->ioDrDirID;

  for( idx=1; TRUE; idx++)
  {
    /* indexing loop */
    fpb->ioVRefNum = fsSpec.vRefNum;
    fpb->ioNamePtr = (U8 FAR*)name_buffer;             /* buffer to receive name */
    fpb->ioDirID = constDirID;         /* must set on each loop */
    fpb->ioFDirIndex = idx;

    PBGetCatInfo( &filespec, FALSE );
    if (dpb->ioResult)
      return n;     /* exit when no more entries */

    if (!(fpb->ioFlAttrib & 16))
    {
      //Need to check for *.XXX                 ;

      if(!AIL_strnicmp((char *)&name_buffer[fpb->ioNamePtr[0] + 1 - 4], &in_filespec[1], 4))
      {
        char temp[256];
        int i=AIL_strlen(dir);
        AIL_memcpy(temp, dir,i);
        AIL_memcpy(temp+i,fpb->ioNamePtr+1,fpb->ioNamePtr[0]);
        temp[i+fpb->ioNamePtr[0]]=0;

        HPROVIDER index = RIB_load_provider_library(temp);
        if (index)
          ++n;
      }
    }
  }

#else


   S32 n = 0;
   //
   // Get application path
   //

   C8 name_buffer[MAX_PATH+4];

   AIL_strcpy(name_buffer,dir);
   AIL_strcat(name_buffer,in_filespec);

#ifdef IS_WIN32

   //
   // Preload all available ASI providers from application path
   //

   HANDLE          search_handle;
   WIN32_FIND_DATA found;

   search_handle = FindFirstFile(name_buffer, &found);

   if ((search_handle != NULL) && (search_handle != INVALID_HANDLE_VALUE))
      {
      do
         {
         AIL_strcpy(name_buffer, dir);
         AIL_strcat(name_buffer, found.cFileName);

         HPROVIDER index = RIB_load_provider_library(name_buffer);

         if (index)
            {
            ++n;
#if 0
            AIL_debug_printf("RIB: Loaded provider %s = %X\n",name_buffer,index);
#endif
            }
         }
      while (FindNextFile(search_handle, &found));

      FindClose(search_handle);
      }

   return n;
#else
#ifdef IS_WIN16

   struct _find_t found;

   U32 err = _dos_findfirst(name_buffer, _A_NORMAL, &found );

   while (!err)
      {

      AIL_strcpy(name_buffer,dir);
      AIL_strcat(name_buffer,found.name);

      HPROVIDER index = RIB_load_provider_library(name_buffer);

      if (index)
         {
         ++n;
#if 0
         AIL_debug_printf("RIB: Loaded provider %s = %X\n",found.name,index);
#endif
         }

      err = _dos_findnext(&found);
      }

   return n;

#endif
#endif
#endif
}

#endif


//############################################################################
//#                                                                          #
//# Load all RIB provider libraries from application-relative directory      #
//#                                                                          #
//############################################################################

RIBDEF S32 RIBEXPORT RIB_load_application_providers(char const FAR *filespec)
{
#if	!(defined(IS_WINDOWS) || defined(IS_MAC))

   filespec=filespec;
   return(0);

#else

   U32 n;
   char dir[512];

   AIL_strcpy(dir,MSS_Directory);
#ifdef IS_X86
   AIL_strcat(dir,"\\");
#endif

   n=load_application_providers(dir,filespec);


   if (*AIL_redist_directory)
   {
#ifdef IS_X86
     if ((AIL_redist_directory[0]=='\\') || (AIL_redist_directory[1]==':'))
#else
     if (AIL_redist_directory[0] != ':')
#endif
       n+=load_application_providers(AIL_redist_directory,filespec);
     else
     {
       // relative path
       AIL_strcpy(dir,MSS_Directory);
#ifdef IS_X86
       AIL_strcat(dir,"\\");
       AIL_strcat(dir,AIL_redist_directory);
#else
       AIL_strcat(dir,AIL_redist_directory+1);
#endif       

       n+=load_application_providers(dir,filespec);
     }
   }

   return(n);
#endif
}


//############################################################################
//##                                                                        ##
//## Set provider user data value at specified index                        ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given HPROVIDER                                      ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

RIBDEF void RIBEXPORT RIB_set_provider_user_data(HPROVIDER provider, U32 index, S32 value)
{
   if (provider == NULL)
      {
      return;
      }

   provider_list.array[provider].user_data[index] = value;
}

//############################################################################
//##                                                                        ##
//## Get provider user data value at specified index                        ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight user data words ##
//## associated with a given HPROVIDER                                      ##
//##                                                                        ##
//## Callback functions may access the user data array at interrupt time    ##
//##                                                                        ##
//############################################################################

RIBDEF S32 RIBEXPORT RIB_provider_user_data(HPROVIDER provider, U32 index)
{
   if (provider == NULL)
      {
      return 0;
      }

   return provider_list.array[provider].user_data[index];
}


//############################################################################
//##                                                                        ##
//## Set provider system data value at specified index                      ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight system data     ##
//## words associated with a given HPROVIDER                                ##
//##                                                                        ##
//## Callback functions may access the system data array at interrupt time  ##
//##                                                                        ##
//############################################################################

RIBDEF void RIBEXPORT RIB_set_provider_system_data(HPROVIDER provider, U32 index, S32 value)
{
   if (provider == NULL)
      {
      return;
      }

   provider_list.array[provider].system_data[index] = value;
}

//############################################################################
//##                                                                        ##
//## Get provider system data value at specified index                      ##
//##                                                                        ##
//## Any desired 32-bit value may be stored at one of eight system data     ##
//## words associated with a given HPROVIDER                                ##
//##                                                                        ##
//## Callback functions may access the system data array at interrupt time  ##
//##                                                                        ##
//############################################################################

RIBDEF S32 RIBEXPORT RIB_provider_system_data(HPROVIDER provider, U32 index)
{
   if (provider == NULL)
      {
      return 0;
      }

   return provider_list.array[provider].system_data[index];
}

//############################################################################
//#                                                                          #
//# Return current error status                                              #
//#                                                                          #
//############################################################################

RIBDEF C8 FAR * RIBEXPORT RIB_error (void)
{
   return error_text;
}


