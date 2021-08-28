/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0
 */
/*
 *      NET Configuration Library
 *
 *      Copyright (C) 2000-2003 Sony Computer Entertainment Inc.
 *                       All Rights Reserved.
 *
 * $Id: netcnf.h,v 1.18 2003/09/12 05:55:06 ksh Exp $
 */

#if !defined(_NETCNF_H)
#define _NETCNF_H

#if defined(__cplusplus)
extern "C" {
#endif

#define sceNETCNF_STR_MAX	256

typedef struct sceNetCnfList {
	int type;				/* entry type */
	int stat;				/* entry status */
	char sys_name[sceNETCNF_STR_MAX];	/* system name */
	char usr_name[sceNETCNF_STR_MAX]; 	/* user name */
} sceNetCnfList_t;

typedef struct sceNetCnfAddress {
	int reserved;	/* must be zero */
	char data[16];
} sceNetCnfAddress_t;

typedef struct sceNetCnfRoutingEntry {
	struct sceNetCnfAddress dstaddr;
	struct sceNetCnfAddress gateway;
	struct sceNetCnfAddress genmask;
	int flags;
	int mss;
	int window;
	char interface[8 + 1];
} sceNetCnfRoutingEntry_t;

/* flags of struct sceNetCnfRoutingEntry */
#define sceNetCnfRoutingF_Up		0x01	/* route is up */
#define sceNetCnfRoutingF_Host		0x02	/* target is host */
#define sceNetCnfRoutingF_Gateway	0x04	/* use gateway */
#define sceNetCnfRoutingF_Dynamic	0x08	/* dynamically installed */
#define sceNetCnfRoutingF_Modified	0x10	/* modified */

typedef struct sceNetCnfCommand {
	struct sceNetCnfCommand *forw, *back;
	int code;
} sceNetCnfCommand_t;

#define sceNetCnf_CMD_ADD_NAMESERVER	1
#define sceNetCnf_CMD_DEL_NAMESERVER	2
#define sceNetCnf_CMD_ADD_ROUTING	3
#define sceNetCnf_CMD_DEL_ROUTING	4

typedef struct sceNetCnfUnknown {
	struct sceNetCnfUnknown *forw, *back;
} sceNetCnfUnknown_t;

typedef struct sceNetCnfUnknownList {
	struct sceNetCnfUnknown *head, *tail;
} sceNetCnfUnknownList_t;

#define sceNetCnf_BOOL_DEFAULT		0xff
#define sceNetCnf_MAX_PHONE_NUMBERS	10

typedef struct sceNetCnfInterface {
	int type;
	u_char *vendor;
	u_char *product;
	u_char *location;
	u_char dhcp;
	u_char *dhcp_host_name;
	u_char dhcp_host_name_null_terminated;
	u_char dhcp_release_on_stop;
	u_char *address;
	u_char *netmask;

	u_char *chat_additional;
	int redial_count;
	int redial_interval;
	u_char *outside_number;
	u_char *outside_delay;
	u_char *phone_numbers[sceNetCnf_MAX_PHONE_NUMBERS];
	u_char answer_mode;
	int answer_timeout;
	int dialing_type;

	u_char *chat_login;
	u_char *auth_name;
	u_char *auth_key;
	u_char *peer_name;
	u_char *peer_key;
	int lcp_timeout;
	int ipcp_timeout;
	int idle_timeout;
	int connect_timeout;
	struct {
		u_char mru_nego;
		u_char accm_nego;
		u_char magic_nego;
		u_char prc_nego;
		u_char acc_nego;
		u_char address_nego;
		u_char vjcomp_nego;
		u_char dns1_nego;
		u_char dns2_nego;
		u_char reserved_nego[7];

		u_short mru;
		u_long accm;
		u_char auth;
		u_char f_mru;
		u_char f_accm;
		u_char f_auth;

		u_char *ip_address;
		u_char *ip_mask;
		u_char *dns1;
		u_char *dns2;

		u_long reserved_value[8];
	} want, allow;
	int log_flags;
	u_char force_chap_type;
	u_char omit_empty_frame;

	u_char pppoe;
	u_char pppoe_host_uniq_auto;
	u_char pppoe_reserved[2];
	u_char *pppoe_service_name;
	u_char *pppoe_ac_name;
	int mtu;

	u_char lcp_max_configure;
	u_char lcp_max_terminate;
	u_char ipcp_max_configure;
	u_char ipcp_max_terminate;
	u_char auth_timeout;
	u_char auth_max_failure;
	u_char reserved[6];

	int phy_config;

	struct sceNetCnfCommand *cmd_head, *cmd_tail;

	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfInterface_t;

/* type */
#define sceNetCnf_IFC_TYPE_ANY		0
#define sceNetCnf_IFC_TYPE_ETH		1
#define sceNetCnf_IFC_TYPE_PPP		2
#define sceNetCnf_IFC_TYPE_NIC		3

/* dialing_type */
#define sceNetCnf_DIALING_TYPE_DEFAULT		-1
#define sceNetCnf_DIALING_TYPE_TONE		0
#define sceNetCnf_DIALING_TYPE_PULSE		1
#define sceNetCnf_DIALING_TYPE_ANY		2

/* phy config */
#define sceNetCnf_PHYCONFIG_AUTO	1	/* auto-nego */
#define sceNetCnf_PHYCONFIG_10		2	/* 10,Half-Duplex */
#define sceNetCnf_PHYCONFIG_10_FD	3	/* 10,Full-Duplex */
#define sceNetCnf_PHYCONFIG_10_FD_PAUSE	4	/* 10,Full-Duplex,Pause */
#define sceNetCnf_PHYCONFIG_TX		5	/* TX,Half-Duplex */
#define sceNetCnf_PHYCONFIG_TX_FD	6	/* TX,Full-Duplex */
#define sceNetCnf_PHYCONFIG_TX_FD_PAUSE	7	/* TX,Full-Duplex,Pause */

typedef struct sceNetCnfDial {
	u_char *tone_dial;
	u_char *pulse_dial;
	u_char *any_dial;
	u_char *chat_init;
	u_char *chat_dial;
	u_char *chat_answer;
	u_char *redial_string;

	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfDial_t;

typedef struct sceNetCnfCtl {
	struct sceNetCnfDial *dial;
	struct sceNetCnfInterface *ifc;
	int id, phone_index, redial_index;
	char interface[8 + 1];
} sceNetCnfCtl_t;

typedef struct sceNetCnfPair {
	struct sceNetCnfPair *forw, *back;

	u_char *display_name;
	u_char *attach_ifc;
	u_char *attach_dev;

	struct sceNetCnfInterface *ifc;
	struct sceNetCnfInterface *dev;

	struct sceNetCnfUnknownList unknown_list;

	struct sceNetCnfCtl *ctl;
} sceNetCnfPair_t;

typedef struct sceNetCnfRoot {
	struct sceNetCnfPair *pair_head, *pair_tail;

	int version;

	u_char *chat_additional;
	int redial_count;
	int redial_interval;
	u_char *outside_number;
	u_char *outside_delay;
	int dialing_type;

	struct sceNetCnfUnknownList unknown_list;
} sceNetCnfRoot_t;

#define sceNetCnf_CURRENT_VERSION	3

typedef struct sceNetCnfEnv {
	char *dir_name;			/* Load arg, Save arg */
	char *arg_fname;		/* Load arg, Save arg */
	void *mem_base;			/* Load arg, Save arg */
	void *mem_ptr;			/* Load arg, Save arg */
	void *mem_last;			/* Load arg, Save arg */
	int req;			/* Load arg, Save arg */
	struct sceNetCnfRoot *root;	/* Load ret, Save arg (req=NET) */
	struct sceNetCnfInterface *ifc;	/* Load ret, Save arg (req=ATTACH) */
	int f_no_check_magic;		/* Load opt */
	int f_no_decode;		/* Load opt, Save opt */
	int f_verbose;			/* Load opt, Save opt */
	int file_err;			/* Load ref, Save ref */
	int alloc_err;			/* Load ref, Save ref */
	int syntax_err;			/* Load ref, Save ref */
	char *fname;			/* (work area) */
	int lno;			/* (work area) */
	u_char lbuf[1024];		/* (work area) */
	u_char dbuf[1024];		/* (work area) */
	int ac;				/* (work area) */
	u_char *av[10 + 1];		/* (work area) */
} sceNetCnfEnv_t;

/* req for sceNetCnfEnv */
#define sceNetCnf_REQ_NET	1	/* NET_CNF */
#define sceNetCnf_REQ_ATTACH	2	/* ATTACH_CNF */

/* magic line */
#define sceNetCnf_MAGIC_STR	"# <Sony Computer Entertainment Inc.>"
#define sceNetCnf_MAGIC_LEN	36

/* function return values */
#define sceNETCNF_NG			-1	/* no good */
#define sceNETCNF_ALLOC_ERROR		-2	/* no space for memory */
#define sceNETCNF_OPEN_ERROR		-3	/* file open error */
#define sceNETCNF_READ_ERROR		-4	/* file read error */
#define sceNETCNF_WRITE_ERROR		-5	/* file write error */
#define sceNETCNF_SEEK_ERROR		-6	/* file seek error */
#define sceNETCNF_REMOVE_ERROR		-7	/* file remove error */
#define sceNETCNF_ENTRY_NOT_FOUND	-8	/* entry not found */
#define sceNETCNF_INVALID_FNAME		-9	/* invalid fname */
#define sceNETCNF_INVALID_TYPE		-10	/* invalid type */
#define sceNETCNF_INVALID_USR_NAME	-11	/* invalid usr_name */
#define sceNETCNF_TOO_MANY_ENTRIES	-12	/* too many entries */
#define sceNETCNF_ID_ERROR		-13	/* can't get individual ID */
#define sceNETCNF_SYNTAX_ERROR		-14	/* syntax error */
#define sceNETCNF_MAGIC_ERROR		-15	/* magic error */
#define sceNETCNF_CAPACITY_ERROR	-16	/* capacity error */
#define sceNETCNF_UNKNOWN_DEVICE	-17	/* unknown device */
#define sceNETCNF_IO_ERROR		-18	/* I/O error */
#define sceNETCNF_TOO_LONG_STR		-19	/* too long string */
#define sceNETCNF_AOL_CONFIGURATION     -20     /* this is AOL configuration */
#define sceNETCNF_LOAD_ATTACH_ERROR     -21     /* load_attach error */

/*** callback ***/
typedef int (*sceNetCnfOpenFunction)(const char *device, const char *pathname, int flags, int mode, int *filesize);
typedef int (*sceNetCnfReadFunction)(int fd, const char *device, const char *pathname, void *buf, int offset, int size);
typedef int (*sceNetCnfCloseFunction)(int fd);

typedef struct sceNetCnfCallback {
	int type;
	sceNetCnfOpenFunction open;
	sceNetCnfReadFunction read;
	sceNetCnfCloseFunction close;
} sceNetCnfCallback_t;

#define	sceNetCnf_DEVICE_ANY	(0)
#define	sceNetCnf_DEVICE_MC	(1)

void sceNetCnfSetCallback(sceNetCnfCallback_t *pcallback);
/*** callback end ***/

int sceNetCnfName2Address(sceNetCnfAddress_t *paddr, char *buf);
int sceNetCnfAddress2String(char *buf, int len, sceNetCnfAddress_t *paddr);
int sceNetCnfGetCount(char *fname, int type);
int sceNetCnfGetList(char *fname, int type, sceNetCnfList_t *p);
int sceNetCnfLoadEntry(char *fname, int type, char *usr_name,
	sceNetCnfEnv_t *e);
int sceNetCnfAddEntry(char *fname, int type, char *usr_name,
	sceNetCnfEnv_t *e);
int sceNetCnfEditEntry(char *fname, int type, char *usr_name,
	char *new_usr_name, sceNetCnfEnv_t *e);
int sceNetCnfDeleteEntry(char *fname, int type, char *usr_name);
int sceNetCnfSetLatestEntry(char *fname, int type, char *usr_name);
void *sceNetCnfAllocMem(sceNetCnfEnv_t *e, int size, int align);
int sceNetCnfInitIFC(sceNetCnfInterface_t *ifc);
int sceNetCnfLoadConf(sceNetCnfEnv_t *e);
int sceNetCnfLoadDial(sceNetCnfEnv_t *e, sceNetCnfPair_t *pair);
int sceNetCnfMergeConf(sceNetCnfEnv_t *e);
int sceNetCnfDeleteAll(char *dev);
int sceNetCnfCheckCapacity(char *fname);
int sceNetCnfConvA2S(char *sp, char *dp, int len);
int sceNetCnfConvS2A(char *sp, char *dp, int len);
int sceNetCnfCheckSpecialProvider(char *fname, int type, char *usr_name,
				  sceNetCnfEnv_t *e);

#if defined(__cplusplus)
}
#endif

#endif	/* _NETCNF_H */
