#include "stdafx.h"
#include "DecalEditor.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include "Decals\DecalPackage.hpp"

#include "Auxiliary\MiscUtils\PropertyEnum.hpp"

#include "..\Support\Decals\DecalDefinition.hpp"

//=========================================================================
// GLOBALS USED TO FORCE LINK OF ANONYMOUS RESOURCE TYPE
//=========================================================================

s32 g_decal_link = 0;

//=========================================================================
// LOCAL
//=========================================================================

DEFINE_RSC_TYPE( s_DecalPkgDesc, decal_pkg_desc, "decalpkg", "Decal Files (*.decalpkg)|*.decalpkg", "A decal package." );

//=========================================================================
// DECAL GROUP DESC
//=========================================================================

decal_pkg_desc::group_desc::group_desc( void ) :
    Color( XCOLOR_WHITE ),
    iDecal( -1 ),
    nDecals( 0 )
{
    Name[0] = '\0';
}


//=========================================================================
// DECAL DEFINITION DESC
//=========================================================================

decal_pkg_desc::decal_desc::decal_desc( void )
{
    Reset();
}

//=========================================================================

void decal_pkg_desc::decal_desc::Reset( void )
{
    MinSize.Set ( 50.0f, 50.0f );
    MaxSize.Set ( 50.0f, 50.0f );
    Color.Set   ( 128, 128, 128, 255 ),
    Name[0]       = '\0';
    MinRoll       = R_0;
    MaxRoll       = R_0;
    BitmapName[0] = '\0';
    MaxVisible    = 20;
    FadeTime      = 2.0f;
    Flags         = decal_definition::DECAL_FLAG_USE_TRI |
                    decal_definition::DECAL_FLAG_NO_CLIP |
                    decal_definition::DECAL_FLAG_KEEP_SIZE_RATIO;
    x_strcpy( PreferredFormat, "Palletized 4" );
    x_strcpy( BlendMode,       "Additive"     );
}

//=========================================================================
// DECAL PACKAGE DESC
//=========================================================================

decal_pkg_desc::decal_pkg_desc( void ) :
    rsc_desc    ( s_DecalPkgDesc ),
    m_Groups    (),
    m_Decals    ()
{
}

//=========================================================================

void decal_pkg_desc::OnEnumProp( prop_enum& List )
{
    rsc_desc::OnEnumProp( List );

    List.PropEnumButton( "ResDesc\\Add Group", "Adds a new decal group to the list.", PROP_TYPE_MUST_ENUM );
    EnumGroups( List );
}

//=========================================================================

void decal_pkg_desc::EnumGroups( prop_enum& List )
{
    List.PropEnumInt( "ResDesc\\NumGroups", "Number of groups this package has", PROP_TYPE_DONT_SHOW );

    s32 i;
    for ( i = 0; i < m_Groups.GetCount(); i++ )
    {
        char HeaderName[1024];
        x_sprintf( HeaderName, "ResDesc\\Group[%d]", i );
        
        List.PropEnumString( HeaderName,                            "A group of decals.",                PROP_TYPE_HEADER    );
        List.PropEnumString( xfs( "%s\\Name", HeaderName ),         "The name of this group.",           PROP_TYPE_MUST_ENUM );
        List.PropEnumColor ( xfs( "%s\\Color", HeaderName ),        "Generic color for this group.\n(currently used for blood particle color)", PROP_TYPE_MUST_ENUM );
        List.PropEnumButton( xfs( "%s\\Remove Group", HeaderName ), "Remove this group from the list.",  PROP_TYPE_MUST_ENUM );
        List.PropEnumButton( xfs( "%s\\Add Decal", HeaderName ),    "Add a decal to this group.",        PROP_TYPE_MUST_ENUM );
        List.PropEnumInt   ( xfs( "%s\\First Decal", HeaderName ),  "The first decal of this group",     PROP_TYPE_DONT_SHOW );
        List.PropEnumInt   ( xfs( "%s\\Num Decals", HeaderName ),   "Number of decals in this group.",   PROP_TYPE_DONT_SHOW );

        EnumDecals( List, i );
    }
}

//=========================================================================

void decal_pkg_desc::EnumDecals( prop_enum& List, s32 iGroup )
{
    ASSERT( (iGroup>=0) && (iGroup < m_Groups.GetCount()) );

    s32 iDecal;
    for ( iDecal = 0; iDecal < m_Groups[iGroup].nDecals; iDecal++ )
    {
        char HeaderName[1024];
        x_sprintf( HeaderName, "ResDesc\\Group[%d]\\Decal[%d]", iGroup, iDecal );

        List.PropEnumString  ( HeaderName,                                 "A decal definition.", PROP_TYPE_HEADER                     );
        List.PropEnumButton  ( xfs( "%s\\Remove Decal",      HeaderName ), "Remove this decal definition.", PROP_TYPE_MUST_ENUM        );
        List.PropEnumString  ( xfs( "%s\\Name",              HeaderName ), "The name of this decal.", PROP_TYPE_MUST_ENUM              );
        List.PropEnumVector2 ( xfs( "%s\\MinSize",           HeaderName ), "This is the minimum size of the decal (in cm).", 0         );
        List.PropEnumVector2 ( xfs( "%s\\MaxSize",           HeaderName ), "This is the maximum size of the decal (in cm).", 0         );
        List.PropEnumAngle   ( xfs( "%s\\MinRoll",           HeaderName ), "This is the minimum amount to rotate the decal.", 0        );
        List.PropEnumAngle   ( xfs( "%s\\MaxRoll",           HeaderName ), "This is the maximum amount to rotate the decal.", 0        );
        List.PropEnumColor   ( xfs( "%s\\Color",             HeaderName ), "This will be the decal's color.", 0                        );
        List.PropEnumInt     ( xfs( "%s\\MaxVisible",        HeaderName ), "This is the maximum number of this decal type allowed.", 0 );
        List.PropEnumFloat   ( xfs( "%s\\FadeOutTime",       HeaderName ), "Fade out time if FlagFadeOut and decal is dynamic.", 0     );
        List.PropEnumFileName( xfs( "%s\\BitmapName",        HeaderName ),
                               "TGA Files (*.tga)|*.tga|BMP Files (*.bmp)|*.bmp|All Files (*.*)|*.*||", 
                               "This is the source bitmap to be compiled.", 0 );
        List.PropEnumEnum    ( xfs( "%s\\PreferredFormat",   HeaderName ),
                               "Palletized 8\0Palletized 4\0",
                               "What bit-depth do you want to compile with?", 0 );
        List.PropEnumBool    ( xfs( "%s\\FlagUseTri",        HeaderName ), "This decal can use triangles to render (vs. quads).", 0             );
        List.PropEnumBool    ( xfs( "%s\\FlagNoClip",        HeaderName ), "This decal should not clip against polygon edges.", 0               );
        List.PropEnumBool    ( xfs( "%s\\FlagUseProjection", HeaderName ), "This decal will use the incoming ray to calc mapping.", 0           );
        List.PropEnumBool    ( xfs( "%s\\FlagKeepSizeRatio", HeaderName ), "Maintain the width/height ratio when calculating size.", 0          );
        List.PropEnumBool    ( xfs( "%s\\FlagPermanent",     HeaderName ), "This decal will never disappear or fade out.", 0                    );
        List.PropEnumBool    ( xfs( "%s\\FlagFadeOut",       HeaderName ), "This decal will fade out over time (dynamic decals only!)", 0       );
        List.PropEnumBool    ( xfs( "%s\\FlagGlowing",       HeaderName ), "This decal uses the glow effect (cannot be mixed with env. map)", 0 );
        List.PropEnumBool    ( xfs( "%s\\FlagEnvMapped",     HeaderName ), "This decal will be env. mapped (cannot be mised with glowing)", 0   );
        List.PropEnumEnum    ( xfs( "%s\\BlendMode",         HeaderName ),
                               "Normal\0Additive\0Subtractive\0Intensity\0",
                               "What type of blending will this decal have?", 0 );
    }
}

//=========================================================================

xbool decal_pkg_desc::OnProperty( prop_query& I )
{
    if( rsc_desc::OnProperty( I ) )
    {
    }
    else if( I.IsVar( "ResDesc\\Add Group" ) )
    {
        if ( I.IsRead() )
        {
            I.SetVarButton( "Add Group" );
        }
        else
        {
            AddGroup();
        }
    }
    else if ( OnPropertyGroups( I ) )
    {
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool decal_pkg_desc::OnPropertyGroups( prop_query& I )
{
    s32 nGroups = m_Groups.GetCount();
    if( I.VarInt( "ResDesc\\NumGroups", nGroups ) )
    {
        if( !I.IsRead() )
        {
            m_Groups.SetCount( nGroups );
        }
    }
    else
    if( I.IsBasePath( "ResDesc\\Group[]" ) )
    {
        s32 Index = I.GetIndex(0);
        ASSERT( (Index>=0) && (Index < m_Groups.GetCount()) );

        s32 iDecal  = m_Groups[Index].iDecal;
        s32 nDecals = m_Groups[Index].nDecals;
        if ( I.IsVar( "ResDesc\\Group[]" ) )
        {
            if ( I.IsRead() )
            {
                I.SetVarString( m_Groups[Index].Name, 32 );
            }
            else
            {
                ASSERT( 0 );
            }
        }
        else if ( I.VarString( "ResDesc\\Group[]\\Name", m_Groups[Index].Name, 32 ) )
        {
        }
        else if ( I.VarColor( "ResDesc\\Group[]\\Color", m_Groups[Index].Color ) )
        {
        }
        else if ( I.IsVar( "ResDesc\\Group[]\\Remove Group" ) )
        {
            if ( I.IsRead() )
            {
                I.SetVarButton( "Remove Group" );
            }
            else
            {
                RemoveGroup( Index );
            }
        }
        else if ( I.IsVar( "ResDesc\\Group[]\\Add Decal" ) )
        {
            if ( I.IsRead() )
            {
                I.SetVarButton( "Add Decal" );
            }
            else
            {
                AddDecal( Index );
            }
        }
        else if ( I.VarInt( "ResDesc\\Group[]\\First Decal", iDecal ) )
        {
            if ( !I.IsRead() )
            {
                m_Groups[Index].iDecal = iDecal;
            }
        }
        else if ( I.VarInt( "ResDesc\\Group[]\\Num Decals", nDecals ) )
        {
            if ( !I.IsRead() )
            {
                m_Groups[Index].nDecals = nDecals;
                GrowDecalList( iDecal + nDecals );
            }
        }
        else if ( OnPropertyDecals( I ) )
        {
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

xbool decal_pkg_desc::HandleFlag( prop_query& I, const char* PropName, u32& PropFlags, u32 FlagBit )
{
    if ( I.IsVar( PropName ) )
    {
        if ( I.IsRead() )
        {
            I.SetVarBool((PropFlags&FlagBit)==FlagBit);
        }
        else
        {
            if ( I.GetVarBool() )
                PropFlags |= FlagBit;
            else
                PropFlags &= ~FlagBit;
        }

        return TRUE;
    }

    return FALSE;
}

//=========================================================================

xbool decal_pkg_desc::OnPropertyDecals( prop_query& I )
{
    if( I.IsBasePath( "ResDesc\\Group[]\\Decal[]" ) )
    {
        s32 iGroup = I.GetIndex(0);
        s32 iDecal = I.GetIndex(1);

        ASSERT( (iGroup>=0) && (iGroup<m_Groups.GetCount()) );
        ASSERT( (iDecal>=0) && (iDecal<m_Groups[iGroup].nDecals) );
        decal_desc& DecalDesc = m_Decals[m_Groups[iGroup].iDecal+iDecal];

        char HeaderName[1024];
        x_strcpy( HeaderName, "ResDesc\\Group[]\\Decal[]" );
        if ( I.IsVar( HeaderName ) )
        {
            if ( I.IsRead() )
            {
                I.SetVarString( DecalDesc.Name, 32 );
            }
            else
            {
                ASSERT( 0 );
            }
        }
        else if ( I.IsVar( xfs( "%s\\Remove Decal", HeaderName ) ) )
        {
            if ( I.IsRead() )
            {
                I.SetVarButton( "Remove Decal" );
            }
            else
            {
                RemoveDecal( iGroup, iDecal );
            }
        }
        else if ( I.VarString( xfs( "%s\\Name", HeaderName ), DecalDesc.Name, 32 ) )
        {
        }
        else if ( I.VarVector2( xfs( "%s\\MinSize", HeaderName ), DecalDesc.MinSize ) )
        {
        }
        else if ( I.VarVector2( xfs( "%s\\MaxSize", HeaderName ), DecalDesc.MaxSize ) )
        {
        }
        else if ( I.VarAngle( xfs( "%s\\MinRoll", HeaderName ), DecalDesc.MinRoll, R_0, R_360+R_1 ) )
        {
        }
        else if ( I.VarAngle( xfs( "%s\\MaxRoll", HeaderName ), DecalDesc.MaxRoll, R_0, R_360+R_1 ) )
        {
        }
        else if ( I.VarColor( xfs( "%s\\Color", HeaderName ), DecalDesc.Color ) )
        {
        }
        else if ( I.VarInt( xfs( "%s\\MaxVisible", HeaderName ), DecalDesc.MaxVisible, 1, 2000 ) )
        {
        }
        else if ( I.VarFloat( xfs( "%s\\FadeOutTime", HeaderName ), DecalDesc.FadeTime, 0.1f, 10000.0f ) )
        {
        }
        else if ( I.VarFileName( xfs( "%s\\BitmapName", HeaderName ), DecalDesc.BitmapName, 256 ) )
        {
        }
        else if ( I.VarEnum( xfs( "%s\\PreferredFormat", HeaderName ), DecalDesc.PreferredFormat ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagUseTri", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_USE_TRI ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagNoClip", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_NO_CLIP ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagUseProjection", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_USE_PROJECTION ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagKeepSizeRatio", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_KEEP_SIZE_RATIO ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagPermanent", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_PERMANENT ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagFadeOut", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_FADE_OUT ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagGlowing", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_ADD_GLOW ) )
        {
        }
        else if( HandleFlag( I, xfs( "%s\\FlagEnvMapped", HeaderName ), DecalDesc.Flags, decal_definition::DECAL_FLAG_ENV_MAPPED ) )
        {
        }
        else if ( I.VarEnum( xfs( "%s\\BlendMode", HeaderName ), DecalDesc.BlendMode ) )
        {
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

//=========================================================================

void decal_pkg_desc::AddGroup( void )
{
    group_desc& Desc = m_Groups.Append();
    Desc.Name[0]     = '\0';
    Desc.Color       = XCOLOR_WHITE;
    Desc.iDecal      = -1;
    Desc.nDecals     = 0;
}

//=========================================================================

void decal_pkg_desc::RemoveGroup( s32 Index )
{
    ASSERT( (Index >= 0) && (Index < m_Groups.GetCount()) );

    // remove any decals associated with this group
    if ( m_Groups[Index].nDecals )
    {
        s32 DecalStart = m_Groups[Index].iDecal;
        m_Decals.Delete( DecalStart, m_Groups[Index].nDecals );
        m_Groups[Index].iDecal  = -1;
        m_Groups[Index].nDecals = 0;
        
        // fix up the start offsets for the other groups
        s32 i;
        for ( i = Index+1; i < m_Groups.GetCount(); i++ )
        {
            m_Groups[i].iDecal = DecalStart;
            DecalStart += m_Groups[i].nDecals;
        }
    }

    // delete the group
    m_Groups.Delete( Index );

}

//=========================================================================

void decal_pkg_desc::GrowDecalList( s32 NewSize )
{
    // nothing to do if the list is already big enough
    if ( NewSize < m_Decals.GetCount() )
        return;

    // grow the list and construct the newly added list entries
    s32 i;
    s32 StartConstruct = m_Decals.GetCount();
    m_Decals.SetCount( NewSize );
    for ( i = StartConstruct; i < NewSize; i++ )
    {
        m_Decals[i].Reset();
    }
}

//=========================================================================

void decal_pkg_desc::AddDecal( s32 iGroup )
{
    ASSERT( (iGroup>=0) && (iGroup < m_Groups.GetCount()) );

    // find insertion point
    s32 InsertPoint;
    if ( m_Groups[iGroup].nDecals == 0 )
    {
        // list is currently empty...calculate a new start point
        s32 i;
        InsertPoint = 0;
        for ( i = 0; i < iGroup; i++ )
        {
            InsertPoint += m_Groups[i].nDecals;
        }
        
        m_Groups[iGroup].iDecal = InsertPoint;
    }
    else
    {
        InsertPoint = m_Groups[iGroup].iDecal + m_Groups[iGroup].nDecals;
    }

    // Insert the decal and reset the properties of it
    m_Decals.Insert( InsertPoint );
    m_Decals[InsertPoint].Reset();
    m_Groups[iGroup].nDecals++;

    // move the start offsets of all groups after this one
    s32 i;
    for ( i = iGroup+1; i < m_Groups.GetCount(); i++ )
    {
        m_Groups[i].iDecal++;
    }
}

//=========================================================================

void decal_pkg_desc::RemoveDecal( s32 iGroup,
                                  s32 iDecal )
{
    ASSERT( (iGroup>=0) && (iGroup < m_Groups.GetCount()) );
    ASSERT( (iDecal>=0) && (iDecal < m_Groups[iGroup].nDecals) );

    // delete the decal
    m_Decals.Delete(m_Groups[iGroup].iDecal+iDecal);
    m_Groups[iGroup].nDecals--;

    // fix up the start offsets for the other groups
    s32 i;
    s32 DecalStart = m_Groups[iGroup].iDecal+m_Groups[iGroup].nDecals;
    for ( i = iGroup+1; i < m_Groups.GetCount(); i++ )
    {
        m_Groups[i].iDecal = DecalStart;
        DecalStart += m_Groups[i].nDecals;
    }
}

//=========================================================================

void decal_pkg_desc::OnCheckIntegrity( void )
{
    rsc_desc::OnCheckIntegrity();

    s32 iGroup;
    s32 iDecal;

    for ( iGroup = 0; iGroup < m_Groups.GetCount(); iGroup++ )
    {
        for ( iDecal = m_Groups[iGroup].iDecal;
              iDecal < (m_Groups[iGroup].iDecal+m_Groups[iGroup].nDecals);
              iDecal++ )
        {
            if ( m_Decals[iDecal].BitmapName[0] == '\0' )
            {
                x_throw( xfs( "You must enter a valid bitmap for group[%d], decal[%d]", iGroup, iDecal-m_Groups[iGroup].iDecal ) );
            }
        }
    }
}

//=========================================================================

void decal_pkg_desc::OnGetCompilerDependencies( xarray<xstring>& List ) 
{
    OnCheckIntegrity();

    s32 iDecal;
    for ( iDecal = 0; iDecal < m_Decals.GetCount(); iDecal++ )
    {
        List.Append() = m_Decals[iDecal].BitmapName;
    }
}

//=========================================================================

void decal_pkg_desc::OnGetFinalDependencies( xarray<xstring>&   List,
                                             platform           Platform,
                                             const char*        pDirectory )
{
    // load the compiled decal package
    decal_package*  pDecalPkg;
    fileio          DecalPackageIO;
    DecalPackageIO.Load( xfs( "%s\\%s", pDirectory, GetName() ), pDecalPkg );
    if( pDecalPkg == NULL )
    {
        x_throw( xfs("Unable to open [%s] so I can't check for dependencies", xfs( "%s\\%s", pDirectory, GetName())) );
    }

    // grab all of the xbmps used by this decal package
    s32 i;
    for( i = 0; i < pDecalPkg->GetNDecalDefs(); i++ )
    {
        const decal_definition& DecalDef = pDecalPkg->GetDecalDef( i );

        // We dont want paths here...
        char *p = (char*)DecalDef.m_BitmapName + x_strlen( DecalDef.m_BitmapName );
        while( (p > DecalDef.m_BitmapName) && (*(p-1) != '\\') && (*(p-1) != '/') )
            p--;

        List.Append() = p;
        LOG_MESSAGE( "decal_pkg_desc::OnGetFinalDependencies",
                     "File: [%s]",
                     p );

    }

    // clean up
    delete pDecalPkg;
}

//=========================================================================
void decal_pkg_desc::OnGetCompilerRules( xstring& CompilerRules ) 
{
    CompilerRules.Clear();
    
    CompilerRules += "DecalCompiler.exe ";
    CompilerRules += "-NUMGROUPS ";
    CompilerRules += xfs("%d ", m_Groups.GetCount());
    CompilerRules += "-NUMDECALS ";
    CompilerRules += xfs("%d ", m_Decals.GetCount());

    s32 iGroup;
    for ( iGroup = 0; iGroup < m_Groups.GetCount(); iGroup++ )
    {
        group_desc& GroupDesc = m_Groups[iGroup];
        const char* pName     = GroupDesc.Name[0] == '\0' ? "unnamed" : GroupDesc.Name;
        if ( GroupDesc.nDecals == 0 )
            GroupDesc.iDecal = -1;

        CompilerRules += "-GROUP ";
        CompilerRules += xfs( "%s ", GroupDesc.Name );
        CompilerRules += "-GROUPCOLOR ";
        CompilerRules += xfs( "%x ", (u32)GroupDesc.Color );
        CompilerRules += "-DECALSTART ";
        CompilerRules += xfs( "%d ", GroupDesc.iDecal );
        CompilerRules += "-DECALCOUNT ";
        CompilerRules += xfs( "%d ", GroupDesc.nDecals );
    }

    s32 iDecal;
    for ( iDecal = 0; iDecal < m_Decals.GetCount(); iDecal++ )
    {
        decal_desc& DecalDesc = m_Decals[iDecal];
        const char* pName     = DecalDesc.Name[0] == '\0' ? "unnamed" : DecalDesc.Name;
        
        CompilerRules += "-DECAL ";
        CompilerRules += xfs( "%s ", DecalDesc.Name );
        CompilerRules += "-MINWIDTH ";
        CompilerRules += xfs( "%f ", DecalDesc.MinSize.X );
        CompilerRules += "-MINHEIGHT ";
        CompilerRules += xfs( "%f ", DecalDesc.MinSize.Y );
        CompilerRules += "-MAXWIDTH ";
        CompilerRules += xfs( "%f ", DecalDesc.MaxSize.X );
        CompilerRules += "-MAXHEIGHT ";
        CompilerRules += xfs( "%f ", DecalDesc.MaxSize.Y );
        CompilerRules += "-MINROLL ";
        CompilerRules += xfs( "%f ", DecalDesc.MinRoll );
        CompilerRules += "-MAXROLL ";
        CompilerRules += xfs( "%f ", DecalDesc.MaxRoll );
        CompilerRules += "-COLOR ";
        CompilerRules += xfs( "%x ", (u32)DecalDesc.Color );
        CompilerRules += "-MAXVIS ";
        CompilerRules += xfs( "%d ", DecalDesc.MaxVisible );
        CompilerRules += "-BITMAP ";
        CompilerRules += xfs( "\"%s\" ", DecalDesc.BitmapName );
        
        if ( !x_strcmp( DecalDesc.PreferredFormat, "Palletized 8" ) )
            CompilerRules += "-P8 ";
        if ( !x_strcmp( DecalDesc.PreferredFormat, "Palletized 4" ) )
            CompilerRules += "-P4 ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_USE_TRI )
            CompilerRules += "-USE_TRI ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_NO_CLIP )
            CompilerRules += "-NO_CLIP ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_USE_PROJECTION )
            CompilerRules += "-USE_PROJECTION ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_KEEP_SIZE_RATIO )
            CompilerRules += "-KEEP_SIZE_RATIO ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_PERMANENT )
            CompilerRules += "-PERMANENT ";
        if ( DecalDesc.Flags & decal_definition::DECAL_FLAG_FADE_OUT )
        {
            CompilerRules += "-FADE_OUT ";
            CompilerRules += xfs( "%f ", DecalDesc.FadeTime );
        }
        if( DecalDesc.Flags & decal_definition::DECAL_FLAG_ADD_GLOW )
            CompilerRules += "-ADD_GLOW ";
        if( DecalDesc.Flags & decal_definition::DECAL_FLAG_ENV_MAPPED )
            CompilerRules += "-ENV_MAPPED ";
        if ( !x_strcmp( DecalDesc.BlendMode, "Normal" ) )
            CompilerRules += "-BLEND_NORMAL ";
        if ( !x_strcmp( DecalDesc.BlendMode, "Additive" ) )
            CompilerRules += "-BLEND_ADD ";
        if ( !x_strcmp( DecalDesc.BlendMode, "Subtractive" ) )
            CompilerRules += "-BLEND_SUBTRACT ";
        if ( !x_strcmp( DecalDesc.BlendMode, "Intensity" ) )
            CompilerRules += "-BLEND_INTENSITY ";
    }
}

//=========================================================================
