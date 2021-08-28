#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include "x_types.hpp"
#include "x_files.hpp"
#include "decode.hpp"
#include "encode.hpp"
#include "parse.hpp"
#include "package.hpp"
#include "main.hpp"

#define ALIGNMENT           16          // 16 byte alignment needed in SPU

s32         verbose_flag = 0;
target_type target_platform;


//-----------------------------------------------------------------------------
target_type GetTargetPlatform(void)
{
    return target_platform;
}

//-----------------------------------------------------------------------------
void SetTargetPlatform(target_type TargetType)
{
    target_platform = TargetType;
}

//-----------------------------------------------------------------------------
s8      TargetEndian(s8 h)
{
    return h;
}

//-----------------------------------------------------------------------------
s16     TargetEndian16(s16 h)
{
    if (GetTargetPlatform()==TARGET_TYPE_GCN)
    {
        return ( ENDIAN_SWAP_16(h) );
    }
    else
    {
        return h;
    }
}

//-----------------------------------------------------------------------------
s32     TargetEndian32(s32 h)
{
    if (GetTargetPlatform()==TARGET_TYPE_GCN)
    {
        return ( ENDIAN_SWAP_32(h) );
    }
    else
    {
        return h;
    }
}

//-----------------------------------------------------------------------------
void main(s32 argc,char *argv[])
{
    char    *new_argv[5];
    char    *pArg;
    s32     argv_index;
    parse_output Output;
    parse_variables Defaults;

    printf("Soundpkg (c)2002 Inevitable Entertainment. Written by Biscuit\n");
    printf("Compiled at %s on %s\n",__TIME__,__DATE__);
    argc--;
    argv++;
    argv_index=0;

    SetTargetPlatform(TARGET_TYPE_PC);
    while (argc) 
    {
        pArg=*argv;
        if (pArg[0]=='-')
        {
            switch(pArg[1])
            {
            case 'v':
            case 'V':
                verbose_flag = 1;
                break;

            default:
                printf("SoundPkg: Warning, invalid command option - '%c'\n",argv[argv_index][1]);
                break;
            }
        }
        else
        {
            new_argv[argv_index]=pArg;
            argv_index++;
        }
        argc--;
        argv++;
    }

    if (argv_index != 1)
    {
        printf("SoundPkg: Usage [opts] <control-file>\n");
        printf("    Where the control file is a text file containing the\n");
        printf("    definitions for each of the simple/complex sound effects.\n");
        printf("    Options are:\n");
        printf("        -v - Verbose mode\n");
        exit(-1);
    }

    x_Init();
    gcn_EncodeInit();
    ps2_EncodeInit();
    ParseControlFile(new_argv[0],Output,Defaults);

    if (verbose_flag)
    {
        complex_effect *pEffects;

        pEffects = Output.m_pEffectHead;
        while (pEffects)
        {
            x_printf("New Effect\n");
            x_printf("  Label       %s\n",pEffects->m_Label);
            x_printf("  Volume      %2.2f\n",pEffects->m_Vars.m_Volume);
            x_printf("  Pitch       %2.2f\n",pEffects->m_Vars.m_Pitch);
            x_printf("  Identifier  0x%08x\n",pEffects->m_Id);
            pEffects = pEffects->m_pNext;
        }
    }

    if (Output.m_LabelFilename[0]==0x0)
    {
        printf("SoundPkg: No label output file defined\n");
        exit(-1);
    }
    package_Write(Output,verbose_flag);
    gcn_EncodeKill();
    ps2_EncodeKill();
}

