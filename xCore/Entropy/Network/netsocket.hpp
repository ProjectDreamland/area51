//==============================================================================
//
//  NETLIB.HPP
//
//==============================================================================

#ifndef NETSOCKET_HPP
#define NETSOCKET_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "x_time.hpp"

//==============================================================================
//  DEFINITIONS
//==============================================================================

class bitstream;

class net_socket
{
public:
                        net_socket          ( void );
                        net_socket          ( s32 Port );

        void            Clear               ( void );
        xbool           IsEmpty             ( void )                    { return m_Address.IsEmpty();       };
        xbool           IsClosed            ( void );
  const char*           GetStrIP            ( void ) const              { return m_Address.GetStrIP();      };
  const char*           GetStrAddress       ( void ) const              { return m_Address.GetStrAddress(); };

        s32             GetIP               ( void )          const     { return m_Address.GetIP();         };
        s32             GetPort             ( void )          const     { return m_Address.GetPort();       };
        void            SetBlocking         ( xbool Block );
        xbool           GetBlocking         ( void )          const;

  const net_address&    GetAddress          ( void )          const     { return m_Address;                 };

        // Port Binding
        xbool           Bind                ( s32 StartPort=0, s32 Flags=0 );
        void            Close               ( void );

        // Establishing TCP connections. Due to the way Sony implemented their network protocol, a socket
        // does NOT have to be bound first before a TCP connection can be established. 
        // Connect will bind and establish the connection
        // Listen will bind and place the port in a listen state
        // Accept will poll to see if someone has tried to connect
        xbool           Connect             ( const net_address& Remote, s32 LocalPort, f32 Timeout );
        xbool           IsConnected         ( void );
        xbool           Listen              ( s32 MaxClients,s32 LocalPort );
        xbool           Accept              ( net_address& Remote );
    
        // Data I/O
        // Note: For TCP, if the length is returned as -1, then that meant the connection has
        // been forcibly closed by the other side. Locally, it should be closed as soon as possible.
        // Send/Receive done using the prototype without a remote address assume it's on a connected
        // TCP socket only.
        xbool           Send                ( const net_address& Remote, const void* pData,s32 Length);
        xbool           Send                ( const net_address& Remote, bitstream& BitStream );
        xbool           Send                ( const void* pData, s32 Length );
        xbool           Receive             ( net_address& Remote, void* pBuffer, s32& Length);
        xbool           Receive             ( net_address& Remote, bitstream& Bitstream );
        xbool           Receive             ( void* pBuffer, s32& Length );
    
        xbool           CanReceive          ( void );
        xbool           CanSend             ( void );

        void            SetFlags            ( s32 Flags )                       { m_Flags = Flags;                          };
        s32             GetFlags            ( void )                            { return m_Flags;                           };

        // Stats
        s64             GetBytesSent        ( void )          const             { return m_BytesSent;                       };
        s64             GetBytesReceived    ( void )          const             { return m_BytesReceived;                   };
        s64             GetPacketsSent      ( void )          const             { return m_PacketsSent;                     };
        s64             GetPacketsReceived  ( void )          const             { return m_PacketsReceived;                 };
        s32             GetAverageBytesSent ( void )          const;
        s32             GetAverageBytesReceived(void)         const;

private:
        net_address     m_Address;
        s32             m_Socket;
        s32             m_Flags;
        s64             m_BytesSent;
        s64             m_BytesReceived;
        s64             m_PacketsSent;
        s64             m_PacketsReceived;

friend void    sys_net_Send    ( net_socket&   Local, 
                          const  net_address&  Remote, 
                          const  void*         pBuffer, 
                                 s32           BufferSize );

friend xbool   sys_net_Receive ( net_socket&   Local,
                                 net_address&  Remote,
                                 void*         pBuffer,
                                 s32&          BufferSize );
};

//==============================================================================
#endif // NETSOCKET_HPP
//==============================================================================
