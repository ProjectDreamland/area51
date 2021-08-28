//=========================================================================
//
// E_VIFGIF.CPP
//
//=========================================================================
#include "ps2_vifgif.hpp"

#ifdef TARGET_DEV
    #include "ps2_misc.hpp"
    #include "x_stdio.hpp"
#endif

//=========================================================================

void VIF_MSCAL( void* pTag )
{
    (void)pTag;
    ASSERT(FALSE);
}

//=========================================================================

void VIF_MSCAL( dlist& DList )
{
    (void)DList;
    ASSERT(FALSE);
}

//=========================================================================

void VIFHELP_BuildMPG( void* pTag, s32 DestAddr, void* SrcAddr, s32 Size )
{
    (void)DestAddr;
    (void)SrcAddr;
    (void)pTag;
    (void)Size;
    ASSERT(FALSE);
}

//=========================================================================

void VIFHELP_BuildMPG( dlist& DList, s32 DestAddr, void* SrcAddr, s32 Size )
{
    (void)DList;
    (void)DestAddr;
    (void)SrcAddr;
    (void)Size;
    ASSERT(FALSE);
}

//=========================================================================
//=========================================================================
//=========================================================================
// GIF
//=========================================================================
//=========================================================================
//=========================================================================

void giftag::Dump   ( void )
{
#ifdef TARGET_DEV
    // LIST BASICS
    x_printf("******** GIFTAG ********\n");
    x_printf("%8.8X %8.8X %8.8X %8.8X\n",((u32*)this)[3],((u32*)this)[2],((u32*)this)[1],((u32*)this)[0]);
    x_printf("NLOOP  | %5d |\n", (s32)NLOOP);
    x_printf("EOP    | %5d |\n", (s32)EOP);
    
    // FLAGS
    {
        x_printf("MODE   | %5d | ", (s32)MODE);
        switch( MODE )
        {
            case 0x00: x_printf("PACKED mode\n"); break;
            case 0x01: x_printf("REGLIST mode\n"); break;
            case 0x02: x_printf("IMAGE mode\n"); break;
            case 0x03: x_printf("IMAGE2 mode\n"); break;
        }
    }
    
    // PRIM REGISTER DATA
    {
        u64 Prim = (u64)PRIM;
        char PrimName[][16] = {"POINT","LINE","LINE STRIP","TRIANGLE",
                               "TRIANGLE STRIP","TRIANGLE FAN","SPRITE"};

        sceGsPrim PR = *((sceGsPrim*)&Prim);
        x_printf("PRE    | %5d |\n", (s32)PRE);
        x_printf("PRIM   | %5d |\n", (s32)Prim);
        
        if( PRE )
        {
            x_printf("         PRIMITIVE: %1s\n",PrimName[PR.PRIM]);
            x_printf("         SHADING:   %1s\n",(PR.IIP)?("Flat"):("Gourand"));
            x_printf("         TEXTURING: %1s\n",(PR.TME)?("On"):("Off"));
            x_printf("         FOGGING:   %1s\n",(PR.FGE)?("On"):("Off"));
            x_printf("         ALPHA:     %1s\n",(PR.ABE)?("On"):("Off"));
            x_printf("         AA:        %1s\n",(PR.AA1)?("On"):("Off"));
            x_printf("         TXTCOORD:  %1s\n",(PR.FST)?("UV"):("STQ"));
            x_printf("         CONTEXT:   %1s\n",(PR.CTXT)?("2"):("1"));
            x_printf("         INTERPFIX: %1s\n",(PR.FIX)?("On"):("Off"));
        }
    }

    // REGISTERS
    {
        s32 i;
        s32 R;
        char RegName[][16] = {"PRIM","RGBAQ","ST","UV","XYZF2","XYZ2","TEX0_1","TEX0_2",
                              "CLAMP_1","CLAMP_2","FOG","RESERVED","XYZF3","XYZ3","A+D","NOP"};
                              
        x_printf("NREG:  | %5d |\n", (s32)NREG);
        for( i=0; i<(s32)NREG; i++ )
        {
            R = (s32)(((((u64*)this)[1]) >> (i*4))&0x0F);
            x_printf("         %1s\n",RegName[R]);
        }
    }
    x_printf("************************\n");
#else
    ASSERT( 0 );        
#endif
}
//=========================================================================


