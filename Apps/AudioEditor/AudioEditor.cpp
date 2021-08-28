//=========================================================================
// AUDIO EDITOR.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "AudioEditor.hpp"
#include "Resource.h"
#include "FaderDialog.h"
#include "..\EDRscDesc\RSCDesc.hpp"

//=========================================================================
// LOCAL
//=========================================================================


//=========================================================================
//  EXTERNALS
//=========================================================================
f32 g_DescVolume        = 1.0f;
f32 g_DescVolumeVar     = 0.0f;
f32 g_DescPitch         = 1.0f;
f32 g_DescPitchVar      = 0.0f;
f32 g_DescPan           = 0.0f;
s32 g_DescPriority      = 128;
f32 g_DescEffectSend    = 0.0f;
f32 g_DescNearFalloff   = 1.0f;
f32 g_DescFarFalloff    = 1.0f;
f32 g_DescNearDiffuse   = 1.0f;
f32 g_DescFarDiffuse    = 1.0f;
u8  g_DescRollOff;


f32 g_ElementVolume     = 1.0f;
f32 g_ElementVolumeVar  = 0.0f;
f32 g_ElementPitch      = 1.0f;
f32 g_ElementPitchVar   = 0.0f;
f32 g_ElementPan        = 0.0f;
s32 g_ElementPriority   = 128;
f32 g_ElementEffectSend = 0.0f;
f32 g_ElementNearFalloff= 1.0f;;
f32 g_ElementFarFalloff = 1.0f;
s32 g_ElementTemperature= HOT;
f32 g_ElementNearDiffuse= 1.0f;
f32 g_ElementFarDiffuse = 1.0f;
u8  g_ElementRollOff;


//=========================================================================
// AUDIO EDITOR
//=========================================================================

audio_editor::audio_editor( void )
{
    g_DescRollOff      |= LINEAR_ROLLOFF;
    g_ElementRollOff   |= LINEAR_ROLLOFF;

    m_PackageSelected           = -1;
    g_pFaderList                = "NONE\0";
    m_MultiSelect               = FALSE;
    m_DefaultParamsMode         = FALSE;
    m_DefaultElementParamsMode  = FALSE;
    m_pCallbackObject           = NULL;
    m_fnpUpdateLabel            = NULL;

    m_MultipleSelection.SetGrowAmount( 8 );

    // Clear the multiple selection values.
    m_MultiSelectionVolume      = 1.0f;    
    m_MultiSelectionVolumeVar   = 0.0f;
    m_MultiSelectionPitch       = 1.0f;
    m_MultiSelectionPitchVar    = 0.0f;
    m_MultiSelectionPan         = 0.0f;
    m_MultiSelectionPriority    = 128;
    m_MultiSelectionEffectSend  = 0.0f;
    m_MultiSelectionNearFalloff = 1.0f;
    m_MultiSelectionFarFalloff  = 1.0f;
    m_MultiSelectionNearDiffuse = 1.0f;
    m_MultiSelectionFarDiffuse  = 1.0f;
    m_MultiSelectionRollOff     |= LINEAR_ROLLOFF;
    m_MultiSelectionTemperature = HOT;
}

audio_editor::~audio_editor ( void )
{
    for( s32 i = 0; i < m_pDesc.GetCount(); i++)
    {
        m_pDesc[ i ].SetBeingEdited( FALSE );
    }
}

//=========================================================================

void audio_editor::OnEnumProp( prop_enum& List )
{
    if( m_DefaultParamsMode )
    {
        DefaultParamsEnumProp( List );
        return;
    }
    else if( m_DefaultElementParamsMode )
    {
        DefaultElementParamsEnumProp( List );
        return;
    }

	// Make sure that we are in bounds.
    if( m_pDesc.GetCount() == 0 )
		return;

    if( (m_PackageSelected == -1) || (m_PackageSelected >= m_pDesc.GetCount()) )
        return;

    // Is the multiple selection mode enabled.
    if( m_MultiSelect )
    {
        MultiSelectEnumProp( List );
    }
    else
    {
        m_pDesc( m_PackageSelected ).OnEditorEnumProp( List );
    }
}

//=========================================================================

xbool audio_editor::OnProperty( prop_query& I )
{
    if( m_DefaultParamsMode )
    {
        return DefaultParamsProperty( I );
    }
    else if( m_DefaultElementParamsMode )
    {
        return DefaultElementParamsProperty( I );
    }

	// Make sure that we are in bounds.
    if( m_pDesc.GetCount() == 0 )
		return FALSE;

    if( (m_PackageSelected == -1) || (m_PackageSelected >= m_pDesc.GetCount()) )
        return FALSE;

	// Is the multiple selection mode enabled.
    if( m_MultiSelect )
    {
        return MultiSelectProperty( I );
    }
    else
    {
        xbool Ret = m_pDesc( m_PackageSelected ).OnEditorProperty( I );

        if( !x_strcmp( I.GetName(), "Descriptor[]\\Label" ) )
        {
            s32 DescSelected = m_pDesc( m_PackageSelected ).m_DescriptorSelected;
            
            // Since prop interface doesn't know about the view this is the best way to update the tree view item.
            if( (m_fnpUpdateLabel) && (m_pCallbackObject) && (I.IsRead() == FALSE) )
            {
                m_fnpUpdateLabel(   m_pCallbackObject, 
                                    m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescSelected ].m_Label );  
            }
        }
        else if(  !x_strcmp( I.GetName(), "Intensity[]\\Level" ) )
        {
            s32 IntensitySelected = m_pDesc( m_PackageSelected ).m_IntensitySelected;

            if( (m_fnpUpdateLabel) && (m_pCallbackObject) && (I.IsRead() == FALSE) )
            {
                m_fnpUpdateLabel(   m_pCallbackObject, 
                                    xfs( "%d", m_pDesc( m_PackageSelected ).m_pIntensity[ IntensitySelected ].m_Level) );
            }
        }
        else if(  !x_strcmp( I.GetName(), "ResDesc\\Name" ) )
        {
            if( (m_fnpUpdateLabel) && (m_pCallbackObject) && (I.IsRead() == FALSE) )
                m_fnpUpdateLabel( m_pCallbackObject, m_pDesc( m_PackageSelected ).GetName() );    
        }
        

        return Ret;
    }
}

//=========================================================================

void audio_editor::Save ( void )
{    
    x_try;

    if( m_pDesc.GetCount() == 0 )
        x_throw ("There is not package open or created" );


    for( s32 i = 0; i < m_pDesc.GetCount(); i++ )
    {
        // Has the package been changed.
        if( m_pDesc[i].IsChanged() )
        {
            m_pDesc[i].OnCheckIntegrity();
            g_RescDescMGR.Save( m_pDesc[i] );
        }
    }
    
    x_catch_display;
}

//=========================================================================

void audio_editor::Load ( const char* pFileName )
{
    x_try;
    
    m_PackageSelected   = -1;
    m_MultiSelect       = FALSE;
     
    EndEdit();
    BeginEdit( editor_audio_package::GetSafeType( g_RescDescMGR.Load( pFileName ) ) );

    x_catch_display;
}

//=========================================================================

void audio_editor::NewPackage ( void )
{
    x_try;
    EndEdit();
    
    BeginEdit( editor_audio_package::GetSafeType( g_RescDescMGR.CreateRscDesc( (const char*)xfs("New AudioPackage %d.audiopkg", m_pDesc.GetCount()) ) ) );
    
    m_pDesc( m_AddedPackage ).m_Volume        = g_DescVolume;
    m_pDesc( m_AddedPackage ).m_VolumeVar     = g_DescVolumeVar;
    m_pDesc( m_AddedPackage ).m_Pitch         = g_DescPitch;
    m_pDesc( m_AddedPackage ).m_PitchVar      = g_DescPitchVar;
    m_pDesc( m_AddedPackage ).m_Pan           = g_DescPan;
    m_pDesc( m_AddedPackage ).m_Priority      = g_DescPriority;
    m_pDesc( m_AddedPackage ).m_EffectSend    = g_DescEffectSend;
    m_pDesc( m_AddedPackage ).m_NearFalloff   = g_DescNearFalloff;
    m_pDesc( m_AddedPackage ).m_FarFalloff    = g_DescFarFalloff;
    m_pDesc( m_AddedPackage ).m_NearDiffuse   = g_DescNearDiffuse;
    m_pDesc( m_AddedPackage ).m_FarDiffuse    = g_DescFarDiffuse;
    m_pDesc( m_AddedPackage ).m_RollOff       = g_DescRollOff;

    
    m_pDesc( m_AddedPackage ).SetChanged( TRUE );
    x_catch_display;
}

//=========================================================================

void audio_editor::EndEdit( void )
{
    if( m_pDesc.GetCount() && m_PackageSelected != -1 )
        m_pDesc( m_PackageSelected ).SetBeingEdited( FALSE );
}

//=========================================================================

xbool audio_editor::NeedSave ( void )
{
    return m_pDesc.GetCount();
}

//=========================================================================

void audio_editor::BeginEdit( editor_audio_package& Audio )
{
    Audio.SetBeingEdited( TRUE );
    editor_audio_package& NewPackage = m_pDesc.Add( m_AddedPackage );
    NewPackage = Audio;
}

//=========================================================================

s32 audio_editor::NewDescriptor     ( void )
{
    if( m_pDesc.GetCount() == 0)
        x_throw ("There is not package open or created" );
    
    editor_descriptor Descriptor;
    m_pDesc( m_PackageSelected ).m_pDescriptorList.Append( Descriptor );
    m_pDesc( m_PackageSelected ).m_DescCount = m_pDesc( m_PackageSelected ).m_pDescriptorList.GetCount();
    x_strncpy( m_pDesc( m_PackageSelected ).m_pDescriptorList[m_pDesc( m_PackageSelected ).m_DescCount - 1].m_Label, (const char*)xfs("Descriptor%d", m_pDesc( m_PackageSelected ).m_DescCount), 128 );
    
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );

    s32 Index = m_pDesc( m_PackageSelected ).m_pDescriptorList.GetCount() - 1;

    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_Volume        = g_DescVolume;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_VolumeVar     = g_DescVolumeVar;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_Pitch         = g_DescPitch;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_PitchVar      = g_DescPitchVar;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_Pan           = g_DescPan;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_Priority      = g_DescPriority;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_EffectSend    = g_DescEffectSend;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_NearFalloff   = g_DescNearFalloff;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_FarFalloff    = g_DescFarFalloff;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_NearDiffuse   = g_DescNearDiffuse;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_FarDiffuse    = g_DescFarDiffuse;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_RollOff       = g_DescRollOff;

    
    m_pDesc( m_PackageSelected ).m_pDescriptorList[Index].m_Type = EDITOR_SIMPLE;

    return Index;
}

//=========================================================================

s32 audio_editor::NewIntensity ( void )
{
    if( m_pDesc.GetCount() == 0)
        x_throw ("There is not package open or created" );
    
    editor_intensity Intensity;
   
    m_pDesc( m_PackageSelected ).m_pIntensity.Append( Intensity );
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );

    return m_pDesc( m_PackageSelected ).m_pIntensity.GetCount() - 1;
}

//=========================================================================

void audio_editor::Delete ( xhandle PackageHandle )
{
    s32 PackageIndex = m_pDesc.GetIndexByHandle( PackageHandle );

    // Check if the index is valid.
    if( (m_pDesc.GetCount() > PackageIndex) && (PackageIndex >= 0) )
    {
        for( s32 i = 0; i < m_pDesc( PackageHandle ).m_pDescriptorList.GetCount(); i++ )
        {
            m_pDesc( PackageHandle ).m_pDescriptorList[i].m_pElements.Clear();
        }
         
        m_pDesc( PackageHandle ).m_pDescriptorList.Clear();
        m_pDesc( PackageHandle ).m_pIntensity.Clear();

        g_RescDescMGR.DeleteRscDesc( m_pDesc( PackageHandle ).GetName() );
        m_pDesc.DeleteByHandle( PackageHandle );
    }
}

//=========================================================================

void audio_editor::DeleteAll ( void )
{
    for( s32 i = 0; i < m_pDesc.GetCount(); i++ )
        m_pDesc.DeleteByIndex( 0 );
}

//=========================================================================

s32 audio_editor::NewElement     ( s32 DescriptorIndex )
{
    if( m_pDesc.GetCount() == 0 )
        x_throw ("There is not package open or created" );
    
    editor_element Element;
//    Element.m_ParentDesc = m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex];
    // WOAH! The above line copies the entire parent editor_descriptor each time an element is added.
    // The editor_descriptor has an xarray of all the elements! So for each element N added, you get a duplicate of 
    // the descriptor list, plus N - 1 additional copies of the existing elements.

    // Only the parent's label is used... if you need more elements to look at, either copy them one at a time, or 
    // see if this code can simply use a reference to the root descriptor.
    x_strncpy(Element.m_ParentDesc.m_Label, m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_Label, 128);

    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements.Append( Element );

    // If there are more than one editor_element then default the editor_descriptor type to complex.
    if( (m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements.GetCount() > 1) &&
        (m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_Type == EDITOR_SIMPLE) )
    {
        m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_Type = EDITOR_COMPLEX;
    }
    
    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );

    s32 Index = m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements.GetCount() - 1;
   
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_Volume        = g_ElementVolume;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_VolumeVar     = g_ElementVolumeVar;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_Pitch         = g_ElementPitch;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_PitchVar      = g_ElementPitchVar;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_Pan           = g_ElementPan;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_Priority      = g_ElementPriority;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_EffectSend    = g_ElementEffectSend;    
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_NearFalloff   = g_ElementNearFalloff;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_FarFalloff    = g_ElementFarFalloff;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_Temperature   = g_ElementTemperature;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_NearDiffuse   = g_ElementNearDiffuse;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_FarDiffuse    = g_ElementFarDiffuse;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescriptorIndex].m_pElements[ Index ].m_RollOff       = g_ElementRollOff;


    return Index;
}

//=========================================================================

xbool audio_editor::DeleteDescriptor  ( s32 DescIndex )
{
    s32 i = 0;
    if( m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pReferencingElement.GetCount() )
    {
        xstring String ( "This editor_descriptor is being refrenced by one of the editor_element inside these other editor_descriptors\n" );
        for( i = 0; i < m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pReferencingElement.GetCount(); i++ )
        {
            editor_element Element = *m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pReferencingElement[ i ];
            String += Element.m_ParentDesc.m_Label;
            String += "\n";
        }
        
        // Ask the user if he still want to continue.
        if( AfxMessageBox( (const char*)String, MB_OKCANCEL ) != IDOK )
            return FALSE;

        // Reset all the Element's refrence index that were refrencing this editor_descriptor.
        for( i = 0; i < m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pReferencingElement.GetCount(); i++ )
        {
            editor_element* pElement = m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pReferencingElement[ i ];
            pElement->m_pReferenceLabel[0]  = 0;
            pElement->m_ReferenceDescIndex =  -1;
        }
    }
        
    // Delete all the editor_elements.
    for( i = 0; i < m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements.GetCount(); i++ )
    {
        DeleteElement( DescIndex, i );
    }

    editor_descriptor& rDesc = m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ];

    for( i = 0; i < m_pDesc( m_PackageSelected ).m_pIntensity.GetCount(); i++ )
    {
        for( s32 j = 0; j < m_pDesc( m_PackageSelected ).m_pIntensity[i].m_pDescriptors.GetCount(); j++ )
        {
            if( !x_strcmp( (const char *)m_pDesc( m_PackageSelected ).m_pIntensity[i].m_pDescriptors[j], rDesc.m_Label) )
            {
                m_pDesc( m_PackageSelected ).m_pIntensity[i].m_pDescriptors.Delete( j );
                break;
            }
        }
    }

    m_pDesc( m_PackageSelected ).m_pDescriptorList.Delete( DescIndex );
    m_pDesc( m_PackageSelected ).m_DescCount--;

    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );

    return TRUE;
}

//=========================================================================

xbool audio_editor::RefrenceDescriptor( s32 DescIndex, s32 ElementIndex, s32 RefDescIndex )
{
    // Set the refrence to the editor_descriptor and also add this editor_element to the editor_descriptor's refrence list.
    x_strncpy( m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pReferenceLabel , m_pDesc( m_PackageSelected ).m_pDescriptorList[ RefDescIndex ].m_Label, 128 );
    m_pDesc( m_PackageSelected ).m_pDescriptorList[ RefDescIndex ].m_pReferencingElement.Append( &m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ] );
    
    m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_ReferenceDescIndex = m_pDesc[ m_PackageSelected ].m_pDescriptorList[ RefDescIndex ].m_pReferencingElement.GetCount() - 1;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSampleName[0] = 0;
    m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_pSamplePathName[0] = 0;
    
    // Equals the editor_descriptor.
    m_pDesc( m_PackageSelected ).m_pDescriptorList[ DescIndex ].m_pElements[ ElementIndex ].m_Temperature = 1;

    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );
    
    return TRUE;
}

//=========================================================================

void audio_editor::DeleteElement ( s32 DescIndex, s32 ElementIndex )
{
    m_pDesc( m_PackageSelected ).m_pDescriptorList[DescIndex].m_pElements.Delete( ElementIndex );

    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );
}

//=========================================================================

void audio_editor::DeleteIntensity( s32 IntensityIndex )
{
    m_pDesc( m_PackageSelected ).m_pIntensity.Delete( IntensityIndex );

    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );
}

//=========================================================================

void audio_editor::DeleteIntensityDesc( s32 IntensityIndex, s32 DescIndex )
{
    m_pDesc( m_PackageSelected ).m_pIntensity[ IntensityIndex ].m_pDescriptors.Delete( DescIndex );

    // This package is dirty.
    m_pDesc( m_PackageSelected ).SetChanged( TRUE );
}

//=========================================================================

void audio_editor::CompileAll ( void )
{
    for( s32 i = 0; i < m_pDesc.GetCount(); i++ )
    {
//        m_pDesc[i].OnGetCompilerRules();
    }
//  g_RescDescMGR
}

//=========================================================================

void audio_editor::CompilePackage ( xhandle PackageIndex )
{
//    m_pDesc[ PackageIndex ].OnGetCompilerRules();
}

//=========================================================================

void audio_editor::ReBuildAll ( void )
{
    for( s32 i = 0; i < m_pDesc.GetCount(); i++ )
    {
        m_pDesc[i].m_RebuildPackage = TRUE;
    }
}

//=========================================================================

void audio_editor::ReBuildPackage ( xhandle PackageIndex )
{
    m_pDesc( PackageIndex ).m_RebuildPackage = TRUE;
}

//=========================================================================

void audio_editor::MultiSelectEnumProp ( prop_enum& List )
{
    // The packages properties.
    List.PropEnumHeader( "Multiple Selection",   "", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Volume",          "Volume of the multiple selection, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Volume Var",      "Volume Variance of the multiple selection, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Pitch",           "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Pitch Var",       "Pitch variance of the multiple selection, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Pan",             "Pan of the multiple selection, Range( -1.0 -- 1.0 )", 0 );
    List.PropEnumInt   ( "Multiple Selection\\Priority",        "Priority of the multiple selection, Range( 0 -- 255)", 0 );
    List.PropEnumFloat ( "Multiple Selection\\EffectSend",      "Reverb of the multiple selection, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Near Falloff",    "Near falloff of the multiple selection, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Far falloff",     "Far falloff of the multiple selection, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Near Diffuse",    "Near diffuse of the multiple selection, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "Multiple Selection\\Far Diffuse",     "Far diffuse of the multiple selection, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumEnum  ( "Multiple Selection\\Rolloff Curve",   "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );

    xbool bOnlyElementSel = FALSE;
    for( s32 i = 0; i < m_MultipleSelection.GetCount(); i++ )
    {
        // Set the packages property.
        if( m_MultipleSelection[i].m_SelectedType & ELEMENT_SELECTED )
        {
            bOnlyElementSel = TRUE;
        }
        else
        {
            bOnlyElementSel = FALSE;
            break;
        }
    }

    if( bOnlyElementSel )
        List.PropEnumEnum ( "Multiple Selection\\Temperature", "HOT\0WARM\0COLD\0", "The temperature of the multi selected elements (Descriptor, Hot, Warm, Cold)", 0 );
}

//=========================================================================

xbool audio_editor::MultiSelectProperty  ( prop_query& I )
{
    if( I.VarFloat ( "Multiple Selection\\Volume"  , m_MultiSelectionVolume ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( VOLUME_DIRTY );
        return TRUE;
    }

    // The Volume Var.
    if( I.VarFloat ( "Multiple Selection\\Volume Var"  , m_MultiSelectionVolumeVar ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( VOLUME_VAR_DIRTY );
        return TRUE;
    }

    // The Pitch.
    if( I.VarFloat ( "Multiple Selection\\Pitch"        , m_MultiSelectionPitch ) )
    {
        if( (m_MultiSelectionPitch < 0.015625)  )//&& (m_MultiSelectionPitch != -2) )
        {
            m_MultiSelectionPitch = 0.015625;
        }
        else if( m_MultiSelectionPitch > 4 )
        {
            m_MultiSelectionPitch = 4;
        }
        
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( PITCH_DIRTY );

        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( "Multiple Selection\\Pitch Var"   , m_MultiSelectionPitchVar ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( PITCH_VAR_DIRTY );
        return TRUE;
    }

    // The Pan.
    if( I.VarFloat ( "Multiple Selection\\Pan"        , m_MultiSelectionPan ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( PAN_DIRTY );
        return TRUE;
    }

    // The Priority.
    if( I.VarInt   ( "Multiple Selection\\Priority"    , m_MultiSelectionPriority ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( PRIORITY_DIRTY );
        return TRUE;
    }

    // The EffectSend.
    if( I.VarFloat ( "Multiple Selection\\EffectSend"   , m_MultiSelectionEffectSend ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( EFFECTSEND_DIRTY );
        return TRUE;
    }

    // The Near Falloff.
    if( I.VarFloat ( "Multiple Selection\\Near Falloff" , m_MultiSelectionNearFalloff ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( NEARFALLOFF_DIRTY );
        return TRUE;
    }

    // The Far falloff.
    if( I.VarFloat ( "Multiple Selection\\Far falloff"   , m_MultiSelectionFarFalloff ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( FARFALLOFF_DIRTY );
        return TRUE;
    }

    // The Near Diffuse.
    if( I.VarFloat ( "Multiple Selection\\Near Diffuse"    , m_MultiSelectionNearDiffuse ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( NEARDIFFUSE_DIRTY );

        return TRUE;
    }

    // The Far Diffuse.
    if( I.VarFloat ( "Multiple Selection\\Far Diffuse"     , m_MultiSelectionFarDiffuse  ) )
    {
        if( I.IsRead() == FALSE )
            ApplyChangesToProperty( FARDIFFUSE_DIRTY );

        return TRUE;
    }

    // The rolloff curve.
    if( I.IsVar( "Multiple Selection\\Rolloff Curve" ) )
    {
        if( I.IsRead () )
        {
            if( m_MultiSelectionRollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( m_MultiSelectionRollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( m_MultiSelectionRollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            m_MultiSelectionRollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                m_MultiSelectionRollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                m_MultiSelectionRollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                m_MultiSelectionRollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
            
            ApplyChangesToProperty( ROLLOFF_DIRTY );

        }
        return TRUE;
    }

    // The temperature of the sample.
    if( I.IsVar( "Multiple Selection\\Temperature" ) )
    {
        if( I.IsRead () )
        {
            switch( m_MultiSelectionTemperature )
            {
                case HOT     : I.SetVarEnum( "HOT" ); break;
                case WARM    : I.SetVarEnum( "WARM" ); break;
                case COLD    : I.SetVarEnum( "COLD" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "HOT", I.GetVarEnum()) )
            {
                m_MultiSelectionTemperature = HOT;
            }
            else if( !x_stricmp( "WARM", I.GetVarEnum() ) )
            {
                m_MultiSelectionTemperature = WARM;
            }
            else if( !x_stricmp( "COLD", I.GetVarEnum() ) )
            {
                m_MultiSelectionTemperature = COLD;
            }

            ApplyChangesToProperty( TEMPERATURE_DIRTY );
        }
        return TRUE;
    }
    
    return FALSE;
}

//=========================================================================

void audio_editor::InsertMultiSelIndex ( multi_sel& SelectionIndex )
{
    m_MultipleSelection.Append( SelectionIndex );

    // Clear the multiple selection values.
    m_MultiSelectionVolume      = 1.0f;    
    m_MultiSelectionVolumeVar   = 0.0f;
    m_MultiSelectionPitch       = 1.0f;
    m_MultiSelectionPitchVar    = 0.0f;
    m_MultiSelectionPan         = 0.0f;
    m_MultiSelectionPriority    = 128;
    m_MultiSelectionEffectSend  = 0.0f;
    m_MultiSelectionNearFalloff = 1.0f;
    m_MultiSelectionFarFalloff  = 1.0f;
    m_MultiSelectionNearDiffuse = 1.0f;
    m_MultiSelectionFarDiffuse  = 1.0f;
    m_MultiSelectionRollOff     |= LINEAR_ROLLOFF;
    m_MultiSelectionTemperature = HOT;

}

//=========================================================================

void audio_editor::ClearMultiSelIndex ( void )
{
    m_MultipleSelection.Clear();
}

//=========================================================================

void audio_editor::DefaultParamsEnumProp ( prop_enum& List )
{
    // The packages properties.
    List.PropEnumHeader( "Default Params",   "", 0 );
    List.PropEnumFloat ( "Default Params\\Volume",          "Volume of the package and descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Volume Var",      "Volume Variance of the package and descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Pitch",           "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
    List.PropEnumFloat ( "Default Params\\Pitch Var",       "Pitch variance of the package and descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Pan",             "Pan of the package and descriptor, Range( -1.0 -- 1.0 )", 0 );
    List.PropEnumInt   ( "Default Params\\Priority",        "Priority of the package and descriptor, Range( 0 -- 255)", 0 );
    List.PropEnumFloat ( "Default Params\\EffectSend",      "Reverb of the package and descriptor, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Near Falloff",    "Near falloff of the package and descriptor, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Far falloff",     "Far falloff of the package and descriptor, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Near Diffuse",    "Near diffuse of the package and descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "Default Params\\Far Diffuse",     "Far diffuse of the package and descriptor, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumEnum  ( "Default Params\\Rolloff Curve",   "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );
}

//=========================================================================

xbool audio_editor::DefaultParamsProperty ( prop_query& I )
{
    if( I.VarFloat ( "Default Params\\Volume"  , g_DescVolume ) )
    {
        return TRUE;
    }

    // The Volume Var.
    if( I.VarFloat ( "Default Params\\Volume Var"  , g_DescVolumeVar ) )
    {
        return TRUE;
    }

    // The Pitch.
    if( I.VarFloat ( "Default Params\\Pitch"        , g_DescPitch ) )
    {
        if( (g_DescPitch < 0.015625) )//&& (g_DescPitch != -2) )
        {
            g_DescPitch = 0.015625;
        }
        else if( g_DescPitch > 4 )
        {
            g_DescPitch = 4;
        }
        
        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( "Default Params\\Pitch Var"   , g_DescPitchVar ) )
    {
        return TRUE;
    }

    // The Pan.
    if( I.VarFloat ( "Default Params\\Pan"        , g_DescPan ) )
    {
        return TRUE;
    }

    // The Priority.
    if( I.VarInt   ( "Default Params\\Priority"    , g_DescPriority ) )
    {
        return TRUE;
    }

    // The EffectSend.
    if( I.VarFloat ( "Default Params\\EffectSend"   , g_DescEffectSend ) )
    {
        return TRUE;
    }

    // The Near Falloff.
    if( I.VarFloat ( "Default Params\\Near Falloff" , g_DescNearFalloff ) )
    {
        return TRUE;
    }

    // The Far falloff.
    if( I.VarFloat ( "Default Params\\Far falloff"   , g_DescFarFalloff ) )
    {
        return TRUE;
    }

    // The Near Diffuse.
    if( I.VarFloat ( "Default Params\\Near Diffuse"    , g_DescNearDiffuse ) )
        return TRUE;

    // The Far Diffuse.
    if( I.VarFloat ( "Default Params\\Far Diffuse"     , g_DescFarDiffuse  ) )
        return TRUE;

    // The rolloff curve.
    if( I.IsVar( "Default Params\\Rolloff Curve" ) )
    {
        if( I.IsRead () )
        {
            if( g_DescRollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( g_DescRollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( g_DescRollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            g_DescRollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                g_DescRollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                g_DescRollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                g_DescRollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void audio_editor::DefaultElementParamsEnumProp ( prop_enum& List )
{
    // The packages properties.
    List.PropEnumHeader( "Default Element Params",   "", 0 );
    List.PropEnumFloat ( "Default Element Params\\Volume",          "Volume of the element, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Volume Var",      "Volume Variance of the element, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Pitch",           "The pitch of the editor_descriptor, Range( 2^-6 -- 2^2 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Pitch Var",       "Pitch variance of the element, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Pan",             "Pan of the element, Range( -1.0 -- 1.0 )", 0 );
    List.PropEnumInt   ( "Default Element Params\\Priority",        "Priority of the element, Range( 0 -- 255)", 0 );
    List.PropEnumFloat ( "Default Element Params\\EffectSend",      "Reverb of the element, Range( 0.0 -- 1.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Near Falloff",    "Near falloff of the element, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Far falloff",     "Far falloff of the element, Range( 0.0 -- 2.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Near Diffuse",    "Near diffuse of the element, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumFloat ( "Default Element Params\\Far Diffuse",     "Far diffuse of the element, Range( 0.0 -- 10.0 )", 0 );
    List.PropEnumEnum  ( "Default Element Params\\Rolloff Curve",   "Linear\0Slow\0Fast", "The type of rolloff curve", 0 );
    List.PropEnumEnum  ( "Default Element Params\\Temperature",      "HOT\0WARM\0COLD\0", "The temperature of the element", 0 );
}

//=========================================================================

xbool audio_editor::DefaultElementParamsProperty ( prop_query& I )
{
    if( I.VarFloat ( "Default Element Params\\Volume"  , g_ElementVolume ) )
    {
        return TRUE;
    }

    // The Volume Var.
    if( I.VarFloat ( "Default Element Params\\Volume Var"  , g_ElementVolumeVar ) )
    {
        return TRUE;
    }

    // The Pitch.
    if( I.VarFloat ( "Default Element Params\\Pitch"        , g_ElementPitch ) )
    {
        if( (g_ElementPitch < 0.015625) )// && (g_ElementPitch != -2) )
        {
            g_ElementPitch = 0.015625;
        }
        else if( g_ElementPitch > 4 )
        {
            g_ElementPitch = 4;
        }
        
        return TRUE;
    }

    // The Pitch Var.
    if( I.VarFloat ( "Default Element Params\\Pitch Var"   , g_ElementPitchVar ) )
    {
        return TRUE;
    }

    // The Pan.
    if( I.VarFloat ( "Default Element Params\\Pan"        , g_ElementPan ) )
    {
        return TRUE;
    }

    // The Priority.
    if( I.VarInt   ( "Default Element Params\\Priority"    , g_ElementPriority ) )
    {
        return TRUE;
    }

    // The EffectSend.
    if( I.VarFloat ( "Default Element Params\\EffectSend"   , g_ElementEffectSend ) )
    {
        return TRUE;
    }

    // The Near Falloff.
    if( I.VarFloat ( "Default Element Params\\Near Falloff" , g_ElementNearFalloff ) )
    {
        return TRUE;
    }

    // The Far falloff.
    if( I.VarFloat ( "Default Element Params\\Far falloff"   , g_ElementFarFalloff ) )
    {
        return TRUE;
    }

    // The Near Diffuse.
    if( I.VarFloat ( "Default Element Params\\Near Diffuse"    , g_ElementNearDiffuse ) )
        return TRUE;

    // The Far Diffuse.
    if( I.VarFloat ( "Default Element Params\\Far Diffuse"     , g_ElementFarDiffuse  ) )
        return TRUE;

    // The rolloff curve.
    if( I.IsVar( "Default Element Params\\Rolloff Curve" ) )
    {
        if( I.IsRead () )
        {
            if( g_ElementRollOff & LINEAR_ROLLOFF )
                I.SetVarEnum( "Linear" );
            else if( g_ElementRollOff & FAST_ROLLOFF )
                I.SetVarEnum( "Fast" );
            else if( g_ElementRollOff & SLOW_ROLLOFF )
                I.SetVarEnum( "Slow" );
            else
                ASSERT( 0 );
        }
        else
        {
            g_ElementRollOff = 0;

            if( x_stricmp( I.GetVarEnum(), "Linear" ) == 0 )
                g_ElementRollOff |= LINEAR_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Fast" ) == 0 )
                g_ElementRollOff |= FAST_ROLLOFF;
            else if( x_stricmp( I.GetVarEnum(), "Slow" ) == 0 )
                g_ElementRollOff |= SLOW_ROLLOFF;
            else
                ASSERT( 0 );
        }
        return TRUE;
    }

    // The temperature of the sample.
    if( I.IsVar("Default Element Params\\Temperature" ) )
    {
        if( I.IsRead () )
        {
            switch( g_ElementTemperature )
            {
                case HOT     : I.SetVarEnum( "HOT" ); break;
                case WARM    : I.SetVarEnum( "WARM" ); break;
                case COLD    : I.SetVarEnum( "COLD" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "HOT", I.GetVarEnum()) )
            {
                g_ElementTemperature = HOT;
            }
            else if( !x_stricmp( "WARM", I.GetVarEnum() ) )
            {
                g_ElementTemperature = WARM;
            }
            else if( !x_stricmp( "COLD", I.GetVarEnum() ) )
            {
                g_ElementTemperature = COLD;
            }
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

void audio_editor::ApplyChangesToProperty ( multisel_dirty DirtyFlag )
{
    for( s32 i = 0; i < m_MultipleSelection.GetCount(); i++ )
    {
        // Set the packages property.
        if( m_MultipleSelection[i].m_SelectedType & PACKAGE_SELECTED )
        {
            s32 Package = m_MultipleSelection[i].m_PackageHandle;
            
            m_pDesc( Package ).SetChanged( TRUE );

            switch( DirtyFlag )
            {
                case VOLUME_DIRTY:
                {
                    m_pDesc( Package ).m_Volume = m_MultiSelectionVolume;                
                }
                break;
                case VOLUME_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_VolumeVar = m_MultiSelectionVolumeVar;
                }
                break;
                case PITCH_DIRTY:
                {
                    m_pDesc( Package ).m_Pitch = m_MultiSelectionPitch;
                }
                break;
                case PITCH_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_PitchVar= m_MultiSelectionPitchVar;
                }
                break;
                case PAN_DIRTY:
                {
                    m_pDesc( Package ).m_Pan = m_MultiSelectionPan;
                }
                break;
                case PRIORITY_DIRTY:
                {
                    m_pDesc( Package ).m_Priority = m_MultiSelectionPriority;
                }
                break;
                case EFFECTSEND_DIRTY:
                {
                    m_pDesc( Package ).m_EffectSend = m_MultiSelectionEffectSend;
                }
                break;
                case NEARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_NearFalloff = m_MultiSelectionNearFalloff;
                }
                break;
                case FARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_FarFalloff = m_MultiSelectionFarFalloff;
                }
                break;
                case ROLLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_RollOff = m_MultiSelectionRollOff;
                }
                break;
                case NEARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_NearDiffuse = m_MultiSelectionNearDiffuse;                
                }
                break;
                case FARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_FarDiffuse = m_MultiSelectionFarDiffuse;
                }
                break;
                default:
                    ASSERT( 0 );
                break;    
            }                
        }
        // Set the descriptors property.
        else if( m_MultipleSelection[i].m_SelectedType & DESCRIPTOR_SELECTED )
        {
            s32 Package = m_MultipleSelection[i].m_PackageHandle;
            s32 Desc    = m_MultipleSelection[i].m_DescriptorIndex;
            
            m_pDesc( Package ).SetChanged( TRUE );

            switch( DirtyFlag )
            {
                case VOLUME_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_Volume = m_MultiSelectionVolume;                
                }
                break;
                case VOLUME_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_VolumeVar = m_MultiSelectionVolumeVar;
                }
                break;
                case PITCH_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_Pitch = m_MultiSelectionPitch;
                }
                break;
                case PITCH_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_PitchVar= m_MultiSelectionPitchVar;
                }
                break;
                case PAN_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_Pan = m_MultiSelectionPan;
                }
                break;
                case PRIORITY_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_Priority = m_MultiSelectionPriority;
                }
                break;
                case EFFECTSEND_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_EffectSend = m_MultiSelectionEffectSend;
                }
                break;
                case NEARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_NearFalloff = m_MultiSelectionNearFalloff;
                }
                break;
                case FARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_FarFalloff = m_MultiSelectionFarFalloff;
                }
                break;
                case ROLLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_RollOff = m_MultiSelectionRollOff;
                }
                break;
                case NEARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_NearDiffuse = m_MultiSelectionNearDiffuse;                
                }
                break;
                case FARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_FarDiffuse = m_MultiSelectionFarDiffuse;
                }
                break;
                default:
                    ASSERT( 0 );
                break;    
            }                
        }
        // Set the elements property.
        else if( m_MultipleSelection[i].m_SelectedType & ELEMENT_SELECTED )
        {
            s32 Package = m_MultipleSelection[i].m_PackageHandle;
            s32 Desc    = m_MultipleSelection[i].m_DescriptorIndex;
            s32 Element = m_MultipleSelection[i].m_ElementIndex;
            
            m_pDesc( Package ).SetChanged( TRUE );

            switch( DirtyFlag )
            {
                case VOLUME_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_Volume = m_MultiSelectionVolume;                
                }
                break;
                case VOLUME_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_VolumeVar = m_MultiSelectionVolumeVar;
                }
                break;
                case PITCH_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_Pitch = m_MultiSelectionPitch;
                }
                break;
                case PITCH_VAR_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_PitchVar= m_MultiSelectionPitchVar;
                }
                break;
                case PAN_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_Pan = m_MultiSelectionPan;
                }
                break;
                case PRIORITY_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_Priority = m_MultiSelectionPriority;
                }
                break;
                case EFFECTSEND_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_EffectSend = m_MultiSelectionEffectSend;
                }
                break;
                case NEARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_NearFalloff = m_MultiSelectionNearFalloff;
                }
                break;
                case FARFALLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_FarFalloff = m_MultiSelectionFarFalloff;
                }
                break;
                case ROLLOFF_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_RollOff = m_MultiSelectionRollOff;
                }
                break;
                case NEARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_NearDiffuse = m_MultiSelectionNearDiffuse;                
                }
                break;
                case FARDIFFUSE_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_FarDiffuse = m_MultiSelectionFarDiffuse;
                }
                break;
                case TEMPERATURE_DIRTY:
                {
                    m_pDesc( Package ).m_pDescriptorList[ Desc ].m_pElements[ Element ].m_Temperature = m_MultiSelectionTemperature;
                }
                break;
                default:
                    ASSERT( 0 );
                break;    
            }            
        }
        else
        {
            // This better never come here.
            ASSERT( 0 );
        }
    }
}

//=========================================================================
