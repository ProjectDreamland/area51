//==============================================================================
//
//  DNAS.hpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
// This is the DNAS overlay interface layer. It is responsible for loading and
// unloading the overlay.
// BIG NOTE: If odd things start happening after the overlay is removed, that may
// be an indication that something was not correctly shutdown within the overlay.

#ifndef __DNAS_HPP
#define __DNAS_HPP

#include "x_types.hpp"
#include "network/ps2/dnas/dnas_private.hpp"

enum dnas_error
{
    DNAS_ERROR_COMMFAILED = -128,
    DNAS_ERROR_GETID_FAILED,
    DNAS_STATUS_OK   = 0,
    DNAS_STATUS_BUSY,

};

struct dnas_init
{
    const byte*         pAuthData;
    s32                 AuthLength;
    const byte*         pPassPhrase;
    s32                 PassPhraseLength;
    s32                 Category;
};

enum dnas_state
{
    IDLE,
    WAITING_AUTHENTICATION,
    WAITING_DOWNLOAD,
    WAITING_KEY,

};

class dnas_authenticate
{

public:
    s32                     Init                        ( const char* pFilename );
    s32                     Update                      ( f32 DeltaTime,s32& Progress );
    void                    Kill                        ( void );
    void                    InitAuthentication          ( dnas_init* pInit );
    void                    KillAuthentication          ( void );
    void                    InitEncryption              ( void );
    void                    KillEncryption              ( void );
    const char*             GetErrorLabel               ( s32 ErrorCode );
    dnas_state              GetState                    ( void );
    void                    GetUniqueId                 ( byte* pData, s32& Length );
    s32                     GetDecryptedLength          ( void* pData, s32 EncryptedLength );
    s32                     GetEncryptedLength          ( void* pData, s32 UnencryptedLength );

    xbool                   Decrypt                     ( void* pData, s32 EncryptedLength, s32 DecryptedLength );
    xbool                   Encrypt                     ( void* pData, s32 EncryptedLength, s32 DecryptedLength );

private:
    typedef s32             init_prototype              ( dnas_jump_table* );
    typedef void            kill_prototype              ( void );
    typedef s32             init_auth_prototype         ( dnas_init* );
    typedef void            kill_auth_prototype         ( void );

    typedef s32             update_prototype            ( f32, s32& );
    typedef const char*     errorlabel_prototype        ( s32 );
    typedef dnas_state      getstate_prototype          ( void );
    typedef void            getunique_prototype         ( byte*, s32& );
    typedef s32             decrypt_prototype           ( void* pData, s32 SourceLength, s32 DestLength );
    typedef s32             encrypt_prototype           ( void* pData, s32 SourceLength, s32 DestLength );
    typedef s32             dec_length_prototype        ( void* pData, s32 Length );
    typedef s32             enc_length_prototype        ( void* pData, s32 Length );
    typedef s32             init_enc_prototype          ( void );
    typedef s32             kill_enc_prototype          ( void );

    void*                   m_pOverlay;
    s32                     m_OverlayLength;
    f32                     m_Time;
    s32                     m_LibnetHandle;
    s32                     m_MSifHandle;
    init_prototype*         m_pInitFunction;
    kill_prototype*         m_pKillFunction;
    init_auth_prototype*    m_pInitAuthFunction;
    kill_auth_prototype*    m_pKillAuthFunction;
    update_prototype*       m_pUpdateFunction;
    errorlabel_prototype*   m_pErrorLabelFunction;
    getstate_prototype*     m_pGetStateFunction;
    getunique_prototype*    m_pGetUniqueFunction;
    encrypt_prototype*      m_pEncryptFunction;
    decrypt_prototype*      m_pDecryptFunction;
    enc_length_prototype*   m_pEncryptLengthFunction;
    dec_length_prototype*   m_pDecryptLengthFunction;
    init_enc_prototype*     m_pInitEncryptionFunction;
    kill_enc_prototype*     m_pKillEncryptionFunction;
};

#endif // __DNAS_HPP