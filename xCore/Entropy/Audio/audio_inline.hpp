#ifndef AUDIO_INLINE_HPP
#define AUDIO_INLINE_HPP

#include "e_Audio.hpp"

//------------------------------------------------------------------------------

inline void RemoveChannelFromList( channel* pChannel )
{ 
    // Remove from the list.
    pChannel->Link.pPrev->Link.pNext = pChannel->Link.pNext;
    pChannel->Link.pNext->Link.pPrev = pChannel->Link.pPrev; 
}

//------------------------------------------------------------------------------

inline void InsertChannelIntoList( channel* pChannel, channel* pInsertionPoint )
{
    // Insert into list.
    pChannel->Link.pPrev             = pInsertionPoint->Link.pPrev;
    pChannel->Link.pNext             = pInsertionPoint;
    pInsertionPoint->Link.pPrev      = pChannel;
    pChannel->Link.pPrev->Link.pNext = pChannel;
}

//------------------------------------------------------------------------------

inline void RemoveVoiceFromList( voice* pVoice )
{ 
    // Remove from the list.
    pVoice->Link.pPrev->Link.pNext = pVoice->Link.pNext;
    pVoice->Link.pNext->Link.pPrev = pVoice->Link.pPrev; 
}

//------------------------------------------------------------------------------

inline void InsertVoiceIntoList( voice* pVoice, voice* pInsertionPoint )
{
    // Insert into list.
    pVoice->Link.pPrev             = pInsertionPoint->Link.pPrev;
    pVoice->Link.pNext             = pInsertionPoint;
    pInsertionPoint->Link.pPrev    = pVoice;
    pVoice->Link.pPrev->Link.pNext = pVoice;
}

//------------------------------------------------------------------------------

inline void RemoveElementFromList( element* pElement )
{ 
    // Remove from the list.
    pElement->Link.pPrev->Link.pNext = pElement->Link.pNext;
    pElement->Link.pNext->Link.pPrev = pElement->Link.pPrev; 
}

//------------------------------------------------------------------------------

inline void InsertElementIntoList( element* pElement, element* pInsertionPoint )
{
    // Insert into list.
    pElement->Link.pPrev             = pInsertionPoint->Link.pPrev;
    pElement->Link.pNext             = pInsertionPoint;
    pInsertionPoint->Link.pPrev      = pElement;
    pElement->Link.pPrev->Link.pNext = pElement;
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceVolume( voice* pVoice )
{
    f32 Result = pVoice->UserVolume * pVoice->Params.Volume * pVoice->pPackage->GetComputedVolume();
    if( g_AudioMgr.IsAudioDuckingEnabled() )
        Result *= pVoice->Params.VolumeDuck;
    return Result;
}

//------------------------------------------------------------------------------

inline f32 CalculateVoicePitch( voice* pVoice )
{
    return pVoice->UserPitch * pVoice->Params.Pitch * pVoice->pPackage->GetComputedPitch();
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceNearFalloff( voice* pVoice )
{
    return pVoice->UserNearFalloff * pVoice->Params.NearFalloff * pVoice->pPackage->GetComputedNearFalloff();
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceFarFalloff( voice* pVoice )
{
    return pVoice->UserFarFalloff * pVoice->Params.FarFalloff * pVoice->pPackage->GetComputedFarFalloff();
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceNearDiffuse( voice* pVoice )
{
    return pVoice->UserNearDiffuse * pVoice->Params.NearDiffuse * pVoice->pPackage->GetComputedNearDiffuse();
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceFarDiffuse( voice* pVoice )
{
    return pVoice->UserFarDiffuse * pVoice->Params.FarDiffuse * pVoice->pPackage->GetComputedFarDiffuse();
}

//------------------------------------------------------------------------------

inline f32 CalculateVoiceEffectSend( voice* pVoice )
{
    return pVoice->UserEffectSend * pVoice->Params.EffectSend * pVoice->pPackage->GetComputedEffectSend();
}

//------------------------------------------------------------------------------

inline f32 CalculateElementVolume( element* pElement )
{
    f32 Result = pElement->Params.Volume * pElement->PositionalVolume * pElement->VolumeVariance * pElement->pVoice->Volume;
    if( g_AudioMgr.IsAudioDuckingEnabled() )
        Result *= pElement->Params.VolumeDuck;
    return Result;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementPitch( element* pElement )
{
    return pElement->Params.Pitch * pElement->PitchVariance * pElement->pVoice->Pitch;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementNearFalloff( element* pElement )
{
    return pElement->Params.NearFalloff * pElement->pVoice->NearFalloff;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementFarFalloff( element* pElement )
{
    return pElement->Params.FarFalloff * pElement->pVoice->FarFalloff;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementNearDiffuse( element* pElement )
{
    return pElement->Params.NearDiffuse * pElement->pVoice->NearDiffuse;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementFarDiffuse( element* pElement )
{
    return pElement->Params.FarDiffuse * pElement->pVoice->FarDiffuse;
}

//------------------------------------------------------------------------------

inline f32 CalculateElementEffectSend( element* pElement )
{
    return pElement->Params.EffectSend * pElement->pVoice->EffectSend;
}

#endif // AUDIO_INLINE_HPP

