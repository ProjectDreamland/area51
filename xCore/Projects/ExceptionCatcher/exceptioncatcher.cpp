#include "x_files.hpp"
#include "netlib.hpp"
#include "ps2/ps2_exceptiondefs.hpp"
#include <windows.h>

#define EXCEPTION_LOG_NAME  "\\\\project1\\tmp\\exceptions.txt"

u8  ReceiveBuffer[MAX_PACKET_SIZE];
//EE,IOP Common GPR  
#define GPR_at 1
#define GPR_v0 2
#define GPR_v1 3
#define GPR_a0 4
#define GPR_a1 5
#define GPR_a2 6
#define GPR_a3 7
#define GPR_t0 8
#define GPR_t1 9
#define GPR_t2 10
#define GPR_t3 11
#define GPR_t4 12
#define GPR_t5 13
#define GPR_t6 14
#define GPR_t7 15
#define GPR_s0 16
#define GPR_s1 17
#define GPR_s2 18
#define GPR_s3 19
#define GPR_s4 20
#define GPR_s5 21
#define GPR_s6 22
#define GPR_s7 23
#define GPR_t8 24
#define GPR_t9 25
#define GPR_k0 26
#define GPR_k1 27
#define GPR_gp 28
#define GPR_sp 29
#define GPR_fp 30
#define GPR_ra 31

//IOP Additional Register
#define IOP_HI            (32)
#define IOP_LO            (33)
#define IOP_SR            (34)
#define IOP_EPC           (35)
#define IOP_IEIDI         (36)
#define IOP_CAUSE         (37)
#define IOP_TAR          (38)
#define IOP_BADVADDR     (39)
#define IOP_DCIC         (40)
#define IOP_BPC          (41)
#define IOP_BPCM         (42)
#define IOP_BDA          (43)
#define IOP_BDAM         (44)



const char *ExceptionCode[]=
{
    "Interrupt",
    "TLB modification",
    "TLB on load or instr fetch",
    "TLB on store",
    "Address error on load or instr fetch",
    "Address error on store",
    "Bus error on load or store",
    "Syscall",
    "Breakpoint",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap",
    "Unknown cause"
};

void ParsePacket(except_request &Request,X_FILE *fp,net_address &Client);
void ExceptStackDump(X_FILE *fp,u32 StackPointer,u32 *pRaw,u32 *pCooked);
extern void ServiceMain();

int ExceptionMain( int argc, char** argv )
{

    net_address         Server;
    net_address         Client;
    interface_info      info;
    s32                 Length;
    xbool               status;
    except_request      Request;
    except_reply        Reply;
    char                ClientName[128];
    X_FILE               *fpLogFile;

    x_Init();

    net_Init();

    x_DebugMsg("server_manager: Waiting for IP address to be assigned...\n");
    while (1)
    {
        s32 i;

        net_GetInterfaceInfo(0,info);
        if (info.Address != 0)
            break;
        for (i=0;i<100000;i++);
    }
    x_DebugMsg("server_manager: Interface attached, IP = %d\n",info.Address);

    net_BindBroadcast(Server,EXCEPT_LOG_PORT);
    Server.SetBlocking(TRUE);

    x_printf("Exception catcher initialized on port %d, waiting for connections.\n",Server.Port);
    while(1)
    {

        Client.Port = Server.Port;
        Client.IP   = (u32)-1;
        status = net_Receive(Server,Client,&Request,Length);
        if (status)
        {
//            x_DebugMsg("Received: Type=%d, client=%08x:%d,Length=%d\n",Request.Header.Type,Client.IP,Client.Port,Length);
            switch(Request.Write.Type)
            {
            case EXCEPT_REQ_FIND:
                net_IPToStr(Request.Find.Address,ClientName);
                x_printf("An exception handler at address %s:%d is looking for a server.\n",ClientName,Request.Find.Port);
                Reply.Find.Address  = Server.IP;
                Reply.Find.Port     = Server.Port;
                Length = sizeof(Reply.Find);
                break;
//----------------------------------------------------------------------------
            case EXCEPT_REQ_WRITE:
                net_IPToStr(Client.IP,ClientName);
                fpLogFile = x_fopen(EXCEPTION_LOG_NAME,"at");
                x_printf("Received exception packet from %s:%d\n",ClientName,Client.Port);
                x_fseek(fpLogFile,0,X_SEEK_END);
                // Here we really need to format the data going in to the text file. We can
                // not bother with that for now.
                ParsePacket(Request,fpLogFile,Client);
                x_fclose(fpLogFile);
                Length = sizeof(Reply.Write);

                break;
//----------------------------------------------------------------------------
             default:
                Reply.Write.Status = EXCEPT_ERR_ERROR;
                Length = sizeof(Reply.Write);
                x_DebugMsg("Invalid request type %d\n",Request.Write.Type);
                break;
            }
            if (Length)
            {
                Length = sizeof(Reply);
                Reply.Sequence = Request.Write.Sequence;
                net_Send(Server,Client,&Reply,Length);
            }
//            x_DebugMsg("Sent: Client=%08x:%d, Length=%d,sequence = \n",Client.IP,Client.Port,Length,Reply.Header.Sequence);
        }
    }

    return 0;
}


void ParsePacket(except_request &Request,X_FILE *fp,net_address &Client)
{
    char textbuff[128];
    SYSTEMTIME SystemTime;
    s32 j;


    net_IPToStr(Client.IP,textbuff);

    GetLocalTime(&SystemTime);
    x_fprintf(fp, "=================== START OF EXCEPTION ======================\n");
    x_fprintf(fp, "*** Received at %d:%02d:%02d, on %02d/%02d/%04d, from %s:%d\n",
                        SystemTime.wHour,SystemTime.wMinute,SystemTime.wSecond,
                        SystemTime.wMonth,SystemTime.wDay,SystemTime.wYear,
                        textbuff,Client.Port);
    x_fprintf(fp, "TIME: %02d:%02d:%02d.%03d\n",Request.Write.Data.Time[0],Request.Write.Data.Time[1],
                                                Request.Write.Data.Time[2],Request.Write.Data.Time[3]);
    net_IPToStr(Request.Write.Data.LocalIP,textbuff);
    x_fprintf(fp, "Local IP: %s, version %s\n",textbuff,Request.Write.Data.DebugVersion);
    if (Request.Write.Data.DebugMessage[0])
    {
        x_fprintf(fp,"Debug State=%s\n",Request.Write.Data.DebugMessage);
    }
    switch (Request.Write.Data.Type)
    {
    case EXCEPT_IOP:
        x_fprintf(fp,"          *** IOP EXCEPTION ***\n\n");
        x_fprintf(fp,"cause = 0x%08x, %s\n",Request.Write.Data.Iop.gpr[IOP_CAUSE],ExceptionCode[(Request.Write.Data.Iop.gpr[IOP_CAUSE]>>2)&31]);
        x_fprintf(fp,"Module %s Version %02x.%02x\n",Request.Write.Data.Iop.module ,(Request.Write.Data.Iop.version)>>8 ,(Request.Write.Data.Iop.version)&0xf);
        x_fprintf(fp,"Offset 0x%08x, badvaddr=0x%08x\n\n",Request.Write.Data.Iop.offset,Request.Write.Data.Iop.gpr[IOP_BADVADDR]); 
        x_fprintf(fp,"pc=0x%08x ra=0x%08x sp=0x%08x\n",Request.Write.Data.Iop.gpr[IOP_EPC], Request.Write.Data.Iop.gpr[GPR_ra], Request.Write.Data.Iop.gpr[GPR_sp]);
        x_fprintf(fp,"gp=0x%08x fp=0x%08x sr=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_gp], Request.Write.Data.Iop.gpr[GPR_fp], Request.Write.Data.Iop.gpr[IOP_SR]);
        x_fprintf(fp,"\n");
        x_fprintf(fp,"at=0x%08x v0=0x%08x v1=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_at], Request.Write.Data.Iop.gpr[GPR_v0], Request.Write.Data.Iop.gpr[GPR_v1]);
        x_fprintf(fp,"a0=0x%08x a1=0x%08x a2=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_a0], Request.Write.Data.Iop.gpr[GPR_a1], Request.Write.Data.Iop.gpr[GPR_a2]);
        x_fprintf(fp,"a3=0x%08x t0=0x%08x t1=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_a3], Request.Write.Data.Iop.gpr[GPR_t0], Request.Write.Data.Iop.gpr[GPR_t1]);
        x_fprintf(fp,"t2=0x%08x t3=0x%08x t4=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_t2], Request.Write.Data.Iop.gpr[GPR_t3], Request.Write.Data.Iop.gpr[GPR_t4]);
        x_fprintf(fp,"t5=0x%08x t6=0x%08x t7=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_t5], Request.Write.Data.Iop.gpr[GPR_t6], Request.Write.Data.Iop.gpr[GPR_t7]);
        x_fprintf(fp,"s0=0x%08x s1=0x%08x s2=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_s0], Request.Write.Data.Iop.gpr[GPR_s1], Request.Write.Data.Iop.gpr[GPR_s2]);
        x_fprintf(fp,"s3=0x%08x s4=0x%08x s5=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_s3], Request.Write.Data.Iop.gpr[GPR_s4], Request.Write.Data.Iop.gpr[GPR_s5]);
        x_fprintf(fp,"s6=0x%08x s7=0x%08x t8=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_s6], Request.Write.Data.Iop.gpr[GPR_s7], Request.Write.Data.Iop.gpr[GPR_t8]);
        x_fprintf(fp,"t9=0x%08x k0=0x%08x k1=0x%08x\n",Request.Write.Data.Iop.gpr[GPR_t9], Request.Write.Data.Iop.gpr[GPR_k0], Request.Write.Data.Iop.gpr[GPR_k1]);


        break;
    case EXCEPT_EE:
        x_fprintf(fp,"***** EE EXCEPTION ****\n");
                j= (Request.Write.Data.EE.cause>>2)&31;
        if (j>13)
            j=13;

        x_fprintf(fp,"    ** %s **\n",ExceptionCode[j]);
        x_fprintf(fp,"pc=0x%08x ra=0x%08x sp=0x%08x\n",Request.Write.Data.EE.epc, Request.Write.Data.EE.gpr[GPR_ra], Request.Write.Data.EE.gpr[GPR_sp]);
        x_fprintf(fp,"gp=0x%08x fp=0x%08x sr=0x%08x\n\n",Request.Write.Data.EE.gpr[GPR_gp], Request.Write.Data.EE.gpr[GPR_fp], Request.Write.Data.EE.gpr[IOP_SR]);
        // Dump register list
        x_fprintf(fp,"at=0x%08x v0=0x%08x v1=0x%08x\n",Request.Write.Data.EE.gpr[GPR_at],Request.Write.Data.EE.gpr[GPR_v0],Request.Write.Data.EE.gpr[GPR_v1]);
        x_fprintf(fp,"a0=0x%08x a1=0x%08x a2=0x%08x\n",Request.Write.Data.EE.gpr[GPR_a0],Request.Write.Data.EE.gpr[GPR_a1],Request.Write.Data.EE.gpr[GPR_a2]);
        x_fprintf(fp,"a3=0x%08x t0=0x%08x t1=0x%08x\n",Request.Write.Data.EE.gpr[GPR_a3],Request.Write.Data.EE.gpr[GPR_t0],Request.Write.Data.EE.gpr[GPR_t1]);
        x_fprintf(fp,"t2=0x%08x t3=0x%08x t4=0x%08x\n",Request.Write.Data.EE.gpr[GPR_t2],Request.Write.Data.EE.gpr[GPR_t3],Request.Write.Data.EE.gpr[GPR_t4]);
        x_fprintf(fp,"t5=0x%08x t6=0x%08x t7=0x%08x\n",Request.Write.Data.EE.gpr[GPR_t5],Request.Write.Data.EE.gpr[GPR_t6],Request.Write.Data.EE.gpr[GPR_t7]);
        x_fprintf(fp,"s0=0x%08x s1=0x%08x s2=0x%08x\n",Request.Write.Data.EE.gpr[GPR_s0],Request.Write.Data.EE.gpr[GPR_s1],Request.Write.Data.EE.gpr[GPR_s2]);
        x_fprintf(fp,"s3=0x%08x s4=0x%08x s5=0x%08x\n",Request.Write.Data.EE.gpr[GPR_s3],Request.Write.Data.EE.gpr[GPR_s4],Request.Write.Data.EE.gpr[GPR_s5]);
        x_fprintf(fp,"s6=0x%08x s7=0x%08x t8=0x%08x\n",Request.Write.Data.EE.gpr[GPR_s6],Request.Write.Data.EE.gpr[GPR_s7],Request.Write.Data.EE.gpr[GPR_t8]);
        x_fprintf(fp,"t9=0x%08x k0=0x%08x k1=0x%08x\n",Request.Write.Data.EE.gpr[GPR_t9],Request.Write.Data.EE.gpr[GPR_k0],Request.Write.Data.EE.gpr[GPR_k1]);
        x_fprintf(fp,"\n");
        ExceptStackDump(fp,Request.Write.Data.EE.gpr[GPR_sp],Request.Write.Data.EE.RawStack,Request.Write.Data.EE.CookedStack);

        break;
    case EXCEPT_ASSERT:
        x_fprintf(fp,"***** ASSERTION FAILED ****\n");
        x_fprintf(fp,"FILE: %s\n",Request.Write.Data.Assert.Filename);
        x_fprintf(fp,"LINE: %d\n",Request.Write.Data.Assert.LineNumber);
        x_fprintf(fp,"EXPR: %s\n",Request.Write.Data.Assert.Expression);
        x_fprintf(fp,"MESG: %s\n",Request.Write.Data.Assert.Message);
        ExceptStackDump(fp,Request.Write.Data.Assert.StackPointer,Request.Write.Data.Assert.RawStack,Request.Write.Data.Assert.CookedStack);
        break;

    case EXCEPT_LOG:
        x_fprintf(fp,"***** DEBUG LOG ENTRY ****\n");
        x_fprintf(fp,"MESG: %s\n",Request.Write.Data.Log.LogMessage);
        break;
    default:
        x_fprintf(fp,"Unknown exception type\n");
        break;
    }

    x_fprintf(fp,"==================== END OF EXCEPTION =======================\n\n\n");
}

void ExceptStackDump(X_FILE *fp,u32 StackPointer,u32 *pRaw,u32 *pCooked)
{
    s32 j,k;

    x_fprintf(fp,"   *** RAW STACK DUMP FROM %08x ***\n",StackPointer);
    for (j=0;j<4;j++)
    {
        for (k=0;k<4;k++)
        {
            x_fprintf(fp,"0x%08x ",*pRaw++);
        }
        x_fprintf(fp,"\n");
    }

    x_fprintf(fp,"\n  *** COOKED STACK DUMP FROM %08x ***\n",StackPointer);

    for (j=0;j<3;j++)
    {
        for (k=0;k<4;k++)
        {
            x_fprintf(fp,"0x%08x ",*pCooked++);
        }
        x_fprintf(fp,"\n");
    }
}
