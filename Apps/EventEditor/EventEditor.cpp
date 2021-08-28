#include "stdafx.h"
#include "EventEditor.hpp"

//=========================================================================
// LOCAL
//=========================================================================
DEFINE_RSC_TYPE( s_Desc, event_description, "evt", "Event Files (*.evt)|*.evt", "Associates events with data" );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

event_description::event_description( void ) : rsc_desc( s_Desc ) {}

//=========================================================================

void event_description::OnEnumProp( prop_enum& List )
{
    for( s32 i=0; i<m_EventList.GetCount(); i++ )
    {
        event&     E  = m_EventList[i];

        List.AddHeader( E.m_EventName, "This is a property type", PROP_TYPE_HEADER );

        s32 j = 0;

        for( j=0; j< E.m_EventAudioList.GetCount(); j++ )
        {
             List.AddString( xfs( "%s\\Sound[%d]",         E.m_EventName,j), "Sound Descriptor (Label)" );
             List.AddFloat ( xfs( "%s\\Sound[%d]\\Volume", E.m_EventName,j), "Volume of the sound, this is acumulated with the current volume of the Sound Descriptor" );
             List.AddFloat ( xfs( "%s\\Sound[%d]\\Pitch",  E.m_EventName,j), "Pitch of the sound, this is acumulated with the current volume of the Sound Descriptor" );
        }
        
        for( j=0; j< E.m_EventParticleList.GetCount(); j++ )
        {
             List.AddString( xfs( "%s\\Particle[%d]",           E.m_EventName,j), "Name of the paricle" );
             List.AddFloat ( xfs( "%s\\Particle[%d]\\Velocity", E.m_EventName,j), "Velocity of the particle" );
             List.AddFloat ( xfs( "%s\\Particle[%d]\\Speed",    E.m_EventName,j), "Speed of the particle" );
        }
    }
}

//=========================================================================

xbool event_description::OnProperty( prop_query& I )
{
    for( s32 i=0; i<m_EventList.GetCount(); i++ )
    {
        event&     E  = m_EventList[i];

        if( I.GetIndex(0) < E.m_EventAudioList.GetCount() )
        {
            // Look through all the audio types associated with the event.
            if( I.VarString( xfs( "%s\\Sound[]", E.m_EventName), E.m_EventAudioList[I.GetIndex(0)].m_Name, 128 ))
                return TRUE;

            if( I.VarFloat( xfs( "%s\\Sound[]\\Pitch", E.m_EventName), E.m_EventAudioList[I.GetIndex(0)].m_Pitch ) )
                return TRUE;

            if( I.VarFloat( xfs( "%s\\Sound[]\\Volume", E.m_EventName), E.m_EventAudioList[I.GetIndex(0)].m_Volume ) )
                return TRUE;
        }

        if( I.GetIndex(0) < E.m_EventParticleList.GetCount() )
        {
            // Look through all the particle types associated with the event.
            if( I.VarString( xfs( "%s\\Particle[]", E.m_EventName), E.m_EventParticleList[I.GetIndex(0)].m_Name, 128 ))
                return TRUE;

            if( I.VarFloat( xfs( "%s\\Particle[]\\Velocity", E.m_EventName), E.m_EventParticleList[I.GetIndex(0)].m_Velocity ) )
                return TRUE;

            if( I.VarFloat( xfs( "%s\\Particle[]\\Speed", E.m_EventName), E.m_EventParticleList[I.GetIndex(0)].m_Speed ) )
                return TRUE;
        }
   }
	return FALSE;
}

//=========================================================================

void event_description::OnGetDependencies( xarray<xstring>& List )
{
}

//=========================================================================

void event_description::OnGetCompilerRules( xstring& CompilerRules )
{

}

//=========================================================================

void event_description::OnCheckIntegrity ( void )
{
    s32 i = 0;
    for( ; i < m_EventList.GetCount(); i++ )
    {
        if( (m_EventList[i].m_EventAudioList.GetCount() == 0) && (m_EventList[i].m_EventParticleList.GetCount == 0) )
            x_throw( xfs( "Unable to save \nEvent %s, doesn't have anything associated with it./nPlease attach something or delete it", m_EventList[i].m_EventName ) );
    }
}

//=========================================================================

event_editor::event_editor( void )
{
    m_pDesc = NULL;
}

//=========================================================================

void event_editor::OnEnumProp( prop_enum& List )
{
	if( m_pDesc == NULL )
		return;

	m_pDesc->OnEnumProp( List );
}

//=========================================================================

xbool event_editor::OnProperty( prop_query& I )
{
	if( m_pDesc == NULL )
		return FALSE;

	return m_pDesc->OnProperty( I );
}

//=========================================================================

void event_editor::AddEvent( const char* pEvent )
{
    if( pEvent == NULL )
        x_throw( "Invalid Event name. (Someone pass a NULL pointer" );

    if( x_strlen( pEvent ) > 128 )
        x_throw( xfs( "The name enter was larger than 128 [%s]", pEvent ));

    ASSERT( m_pDesc );

    event_description::event& Event = m_pDesc->m_EventList.Append();
    
    // Clear the event.
    Event.Clear();
    x_strcpy( Event.m_EventName, pEvent );    
}

//=========================================================================

event_description::audio_event* event_editor::AddAudioEvent( s32 EventIndex )
{
    ASSERT( m_pDesc );

    event_description::audio_event& AudioEvent = m_pDesc->m_EventList[ EventIndex ].m_EventAudioList.Append();
    
    // Clear the audio event.
    AudioEvent.Clear();
    return &AudioEvent;
}

//=========================================================================

event_description::particle_event* event_editor::AddParticleEvent( s32 EventIndex )
{
    ASSERT( m_pDesc );

    event_description::particle_event& ParticleEvent = m_pDesc->m_EventList[ EventIndex ].m_EventParticleList.Append();
    
    // Clear the particle event.
    ParticleEvent.Clear();
    return &ParticleEvent;
}

//=========================================================================

void event_editor::Save ( void )
{
    if( m_pDesc == NULL )
        x_throw ("There is not package open or created" );

    m_pDesc->OnCheckIntegrity();

    text_out TextOut;
    TextOut.OpenFile( m_pDesc->GetName() );
    m_pDesc->OnSave( TextOut );
    TextOut.CloseFile();
}

//=========================================================================

void event_editor::Load ( const char* pFileName )
{
    x_try;

    New();

    text_in TextIn;
    TextIn.OpenFile( pFileName );
    m_pDesc->OnLoad( TextIn );
    TextIn.CloseFile();

    x_catch_begin;

    delete []m_pDesc;

    x_catch_end_ret;
}

//=========================================================================

void event_editor::Edit( event_description& EventDesc )
{
    EventDesc.SetBeenEdited( TRUE );
    m_pDesc = &EventDesc;
}

//=========================================================================

void event_editor::New ( void )
{
    if( m_pDesc )
        delete m_pDesc;

    m_pDesc = new event_description;
    if( m_pDesc == NULL )
        x_throw( "Out of memory" );
}

//=========================================================================

xbool event_editor::NeedSave ( void )
{
    return m_pDesc != NULL;
}

//=========================================================================

void event_description::event::Clear( void )
{ 
    m_EventName[0] = 0; 
    m_EventAudioList.Clear(); 
    m_EventParticleList.Clear(); 
}

//=========================================================================

void event_description::audio_event::Clear( void )
{ 
    m_Volume = 1.0f; 
    m_Pitch = 1.0f; 
    m_Name[0] = 0; 
}

//=========================================================================

void event_description::particle_event::Clear( void )
{ 
    m_Velocity = 0.0f; 
    m_Speed = 0.0f; 
    m_Name[0] = 0; 
}
