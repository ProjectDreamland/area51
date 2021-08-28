
#ifndef _NNINTERNAL_H_
#define _NNINTERNAL_H_

#include "natneg.h"
#define MATCHUP1_HOSTNAME "natneg1.gamespy.com"
#define MATCHUP2_HOSTNAME "natneg2.gamespy.com"
#define MATCHUP_PORT 27901

#define FINISHED_NOERROR 0
#define FINISHED_ERROR_DEADBEAT_PARTNER 1
#define FINISHED_ERROR_INIT_PACKETS_TIMEDOUT 2



#define INIT_RETRY_TIME 500
#define INIT_RETRY_COUNT 30
#define NNINBUF_LEN 512
#define PING_RETRY_TIME 700
#define PING_RETRY_COUNT 20
#define FINISHED_IDLE_TIME 5000
#define PARTNER_WAIT_TIME 60000

#define NN_PROTVER 2

#define NN_PT_GP  0
#define NN_PT_NN1 1
#define NN_PT_NN2 2

#define NN_INIT 0
#define NN_INITACK 1
#define NN_ERTTEST 2
#define NN_ERTACK 3
#define NN_STATEUPDATE 4
#define NN_CONNECT 5
#define NN_CONNECT_ACK 6
#define NN_CONNECT_PING 7
#define NN_BACKUP_TEST 8
#define NN_BACKUP_ACK 9

#ifndef _PS2
#pragma pack(1)
#endif


#define INITPACKET_SIZE 21
#define INITPACKET_ADDRESS_OFFSET 15
typedef struct _InitPacket
{
	unsigned char magic[NATNEG_MAGIC_LEN];
	unsigned char version;
	unsigned char packettype;
	int cookie;	
	unsigned char porttype;
	unsigned char clientindex;
	unsigned char usegameport;
	unsigned int localip;
	unsigned short localport;
} InitPacket;

#define CONNECTPACKET_SIZE 20
typedef struct _ConnectPacket
{
	unsigned char magic[NATNEG_MAGIC_LEN];
	unsigned char version;
	unsigned char packettype;
	int cookie;	
	unsigned int remoteIP;
	unsigned short remotePort;
	unsigned char gotyourdata;
	unsigned char finished;
} ConnectPacket;


#ifndef _PS2	// PS2 preprocessor will TRUNCATE file if it sees this
#pragma pack()
#endif

#endif
