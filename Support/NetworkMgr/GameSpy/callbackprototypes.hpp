#if !defined(FRIEND)
#error Macro must be defined prior to this file being included!
#endif

FRIEND void     gamespy_server_query            ( ServerBrowser sb, SBCallbackReason reason, SBServer server, void *instance );
FRIEND void     gamespy_indirect_server_query   ( ServerBrowser sb, SBCallbackReason reason, SBServer server, void *instance );
FRIEND void     gamespy_extended_info_query     ( ServerBrowser sb, SBCallbackReason reason, SBServer server, void *instance );

FRIEND void     gamespy_serverkey               ( int keyid, qr2_buffer_t outbuf, void *userdata );
FRIEND void     gamespy_playerkey               ( int keyid, int index, qr2_buffer_t outbuf, void *userdata );
FRIEND void     gamespy_teamkey                 ( int keyid, int index, qr2_buffer_t outbuf, void *userdata );
FRIEND void     gamespy_keylist                 ( qr2_key_type keytype, qr2_keybuffer_t keybuffer, void *userdata );
FRIEND int      gamespy_count                   ( qr2_key_type keytype, void *userdata );
FRIEND void     gamespy_adderror                ( qr2_error_t error, gsi_char *errmsg, void *userdata );
FRIEND void     gamespy_nat_negotiate           ( int cookie, void* userdata );
FRIEND void     gamespy_nat_progress            ( NegotiateState state, void *userdata );
FRIEND void     gamespy_nat_complete            ( NegotiateResult result, SOCKET gamesocket, struct sockaddr_in *remoteaddr, void *userdata);
FRIEND void     gamespy_public_address          ( unsigned int ip, unsigned short port, void* userdata );

FRIEND void     gamespy_connect                 ( GPConnection* pconnection, GPConnectResponseArg * arg, void * param );
FRIEND void     gamespy_emailcheck              ( GPConnection* pconnection, GPIsValidEmailResponseArg* arg, void* param );
FRIEND void     gamespy_buddy_request           ( GPConnection* pConnection, GPRecvBuddyRequestArg* pRequest, void* pInstance );
FRIEND void     gamespy_buddy_status            ( GPConnection* pConnection, GPRecvBuddyStatusArg* pStatus, void* pInstance );
FRIEND void     gamespy_buddy_invite            ( GPConnection* pConnection, GPRecvGameInviteArg* pInvite, void* pInstance );
FRIEND void     gamespy_motd_complete           ( GPConnection* pConnection, GPProfileSearchResponseArg* pResponse, void* pInstance );
FRIEND void     gamespy_security_complete       ( GPConnection* pConnection, GPProfileSearchResponseArg* pResponse, void* pInstance );
FRIEND void     gamespy_error                   ( GPConnection* pConnection, GPErrorArg* pError, void* pParam );
