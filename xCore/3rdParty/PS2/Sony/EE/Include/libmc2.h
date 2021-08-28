/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 */
/* 
 *                      Memory Card Library II
 *                          Version 1.0
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2002 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       libmc2 - libmc2.h
 *                     header file of libmc2.h
 *
 *    Version  Date        Design   Log
 *  ------------------------------------------------------------------------
 *    0.10     2002- 4-18  iwano    initial
 *    0.11     2002- 9-16  iwano    for beta release
 *    0.12     2002-10- 5  iwano    change error code consept
 *    0.13     2002-10-17  iwano    sceMc2xx -> SCE_MC2_XX
 *    0.13     2002-10-22  iwano    SCE_MC2_OLD_DEFINE
 *
 */

#ifndef _LIBMC2_H_
#define _LIBMC2_H_

#include <eetypes.h>
#include <sceerrno.h>

/* 定数マクロ */

#define SCE_MC2_DMA_BUFFER_MAX	(16)
#define SCE_MC2_MAX_DEVICE_NAME	(16)
#define SCE_MC2_MAX_NAME_LEN	(32)
#define SCE_MC2_MAX_PATH_LEN	(68)
#define SCE_MC2_PORT_1		(2)
#define SCE_MC2_PORT_2		(3)
#define SCE_MC2_SPECIFIC_PORT		( 1 << 1 )
#define SCE_MC2_SPECIFIC_DRIVER_NUMBER	( 1 << 2 )
#define SCE_MC2_SPECIFIC_DEVICE_NAME	( 1 << 3 )

/* メモリーカード情報 構造体 */

typedef struct {
	int type;		/* メディアタイプ 1:PS1/PDA, 2:PS2 */
	int isFormat;		/* 1:フォーマット済み 0:未フォーマット */
	int freeClust;		/* 空きクラスタ数 */
} SceMc2InfoParam;

/* 日付 構造体 */

typedef struct {
	unsigned char reserved;
	unsigned char sec;
	unsigned char min;
	unsigned char hour;
	unsigned char day;
	unsigned char month;
	unsigned short year;
} SceMc2DateParam;

/* ディレクトリ,ファイル情報 構造体 */

typedef struct {
	SceMc2DateParam creation;
	SceMc2DateParam modification;
	unsigned int size;
	unsigned short attribute;
	unsigned short resv1;
	char name[SCE_MC2_MAX_NAME_LEN];
} SceMc2DirParam;

/* バーチャルソケット構造体 */
typedef struct {
	unsigned int option;
	int port;
	int slot;
	int number;
	char name[SCE_MC2_MAX_DEVICE_NAME];
} SceMc2SocketParam;

/* メディアタイプ */

#define SCE_MC2_TYPE_NO_CARD	(0)
#define SCE_MC2_TYPE_PS1	(1)
#define SCE_MC2_TYPE_PS2	(2)
#define SCE_MC2_TYPE_PDA	(3)

/* 関数コード */

#define SCE_MC2_FUNC_NONE	(0)
#define SCE_MC2_FUNC_GETINFO	(2)
#define SCE_MC2_FUNC_FORMAT	(3)
#define SCE_MC2_FUNC_UNFORMAT	(4)
#define SCE_MC2_FUNC_READFILE	(5)
#define SCE_MC2_FUNC_WRITEFILE	(6)
#define SCE_MC2_FUNC_CREATEFILE	(7)
#define SCE_MC2_FUNC_DELETE	(8)
#define SCE_MC2_FUNC_RENAME	(9)
#define SCE_MC2_FUNC_GETDIR	(10)
#define SCE_MC2_FUNC_MKDIR	(11)
#define SCE_MC2_FUNC_CHDIR	(12)
#define SCE_MC2_FUNC_CHMOD	(13)
#define SCE_MC2_FUNC_SEARCHFILE	(14)
#define SCE_MC2_FUNC_GETENTSPC	(15)
#define SCE_MC2_FUNC_PAGEREAD	(16)
#define SCE_MC2_FUNC_PAGEWRITE	(17)
#define SCE_MC2_FUNC_OPEN	(20)
#define SCE_MC2_FUNC_CLOSE	(21)
#define SCE_MC2_FUNC_READ	(22)
#define SCE_MC2_FUNC_WRITE	(23)
#define SCE_MC2_FUNC_SEEK	(24)
#define SCE_MC2_FUNC_GETDIRALL	(100)



/* 非同期関数処理状態マクロ */

#define SCE_MC2_STAT_EXEC_RUN		(0)
#define SCE_MC2_STAT_EXEC_FINISH	(1)
#define SCE_MC2_STAT_EXEC_IDLE		(-1)

/* ファイル属性 */

#define SCE_MC2_FILE_ATTR_PS1		0x1000
#define SCE_MC2_FILE_ATTR_PDA_EXEC	0x0800
#define SCE_MC2_FILE_ATTR_CLOSED	0x0080
#define SCE_MC2_FILE_ATTR_SUBDIR	0x0020
#define	SCE_MC2_FILE_ATTR_DUPPROHIBIT	0x0008
#define SCE_MC2_FILE_ATTR_EXECUTABLE	0x0004
#define SCE_MC2_FILE_ATTR_WRITEABLE	0x0002
#define SCE_MC2_FILE_ATTR_READABLE	0x0001


#ifdef SCE_MC2_OLD_DEFINE

#define SCE_EDBROKEN		SCE_EDEVICE_BROKEN
#define SCE_EFBROKEN		SCE_EFILE_BROKEN
#define SCE_ENEWCARD		SCE_ENEW_DEVICE
#define SCE_EDFORMAT		(47)
#define SCE_ENODIRENT		0x9fff
/* エラーコード */
#define	sceMc2ErrNone			SCE_OK	/* 正常終了 */
#define	sceMc2ErrCardNotExist		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENODEV)		/* カードなし */
#define	sceMc2ErrCardBroken		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EDEVICE_BROKEN) /* カード破損の可能性 */
#define	sceMc2ErrNewCard		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_ENEW_DEVICE)	/* 新規カード検出 */
#define	sceMc2ErrNotFormat		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EFORMAT)		/* 未フォーマット */
#define	sceMc2ErrNoSpace		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENOSPC)		/* 空き容量不足 */
#define	sceMc2ErrAccessFaild		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ECONNREFUSED)	/* メモリカードにアクセスできない */
#define	sceMc2ErrFileNotExist		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENOENT)		/* ファイルが存在しない */
#define	sceMc2ErrFileExist		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EEXIST)		/* 既に同名のファイルかディレクトリが存在している */
#define	sceMc2ErrFileBroken		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EFILE_BROKEN)	/* ファイル破損の可能性 */
#define	sceMc2ErrFileReadProtected	SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EACCES)		/* 読込み禁止属性である */
#define	sceMc2ErrFileWriteProtected	SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EACCES)		/* 書込み禁止属性である */
#define	sceMc2ErrFileCopyProtected	SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EACCES)		/* コピー禁止属性である */
#define	sceMc2ErrPermissionDenied	SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EACCES)		/* その他アクセス権にまつわるエラー */
#define	sceMc2ErrDirNotExist		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_ENODIRENT)	/* ディレクトリが存在しない */
#define	sceMc2ErrNotDir			SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENOTDIR)		/* ディレクトリではない（ファイルである）*/
#define	sceMc2ErrDirBroken		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EFILE_BROKEN)	/* ディレクトリエントリ破損の可能性 */
#define	sceMc2ErrDirNotEmpty		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENOTEMPTY)	/* ディレクトリ内が空になっていない */
#define	sceMc2ErrNoDirSpace		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENFILE)		/* ディレクトリエントリ不足 */
#define	sceMc2ErrInvidParam		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EINVAL)		/* 指定された数値が不正 */
#define sceMc2ErrTooLongName		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENAMETOOLONG)	/* ファイルまたはディレクトリ名が長すぎる */
#define sceMc2ErrTooManyDepth		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EMDEPTH)	/* ディレクトリが深すぎる */
#define	sceMc2ErrSema			SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_ESEMA)	/* セマフォの作成に失敗した */
#define sceMc2ErrThread			SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_ETHREAD)	/* スレッドの作成に失敗した */
#define	sceMc2ErrSocket			SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENFILE)		/* ソケット管理テーブルが一杯で作成に失敗した */
#define	sceMc2ErrSocketId		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EID)		/* ソケットIDが不正 */
#define sceMc2ErrNotInitialize		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,SCE_EINIT)	/*  */
#define sceMc2ErrSystemBusy		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,EBUSY)		/*  */
#define sceMc2ErrNotSupport		SCE_ERROR_ENCODE(SCE_ERROR_PREFIX_MC2,ENOTSUP)		/*  */
#define	sceMc2ErrExtend			(0x8000)
/* 関数コード */
#define sceMc2FuncNone			SCE_MC2_FUNC_NONE
#define sceMc2FuncGetInfo		SCE_MC2_FUNC_GETINFO
#define sceMc2FuncFormat		SCE_MC2_FUNC_FORMAT
#define sceMc2FuncUnFormat		SCE_MC2_FUNC_UNFORMAT
#define sceMc2FuncReadFile		SCE_MC2_FUNC_READFILE
#define sceMc2FuncWriteFile		SCE_MC2_FUNC_WRITEFILE
#define sceMc2FuncCreateFile		SCE_MC2_FUNC_CREATEFILE
#define sceMc2FuncDelete		SCE_MC2_FUNC_DELETE
#define sceMc2FuncRename		SCE_MC2_FUNC_RENAME
#define sceMc2FuncGetDir		SCE_MC2_FUNC_GETDIR
#define sceMc2FuncMkdir			SCE_MC2_FUNC_MKDIR
#define sceMc2FuncChdir			SCE_MC2_FUNC_CHDIR
#define sceMc2FuncChmod			SCE_MC2_FUNC_CHMOD
#define sceMc2FuncSearchFile		SCE_MC2_FUNC_SEARCHFILE
#define sceMc2FuncGetEntSpace		SCE_MC2_FUNC_GETENTSPC
#define sceMc2FuncReadPage		SCE_MC2_FUNC_PAGEREAD
#define sceMc2FuncWritePage		SCE_MC2_FUNC_PAGEWRITE
#define sceMc2FuncOpen			SCE_MC2_FUNC_OPEN
#define sceMc2FuncClose			SCE_MC2_FUNC_CLOSE
#define sceMc2FuncRead			SCE_MC2_FUNC_READ
#define sceMc2FuncWrite			SCE_MC2_FUNC_WRITE
#define sceMc2FuncSeek			SCE_MC2_FUNC_SEEK
#define sceMc2FuncGetDirAll		SCE_MC2_FUNC_GETDIRALL
/* ファイル属性 */
#define sceMc2FileAttrPS1		SCE_MC2_FILE_ATTR_PS1
#define sceMc2FileAttrPDAExec		SCE_MC2_FILE_ATTR_PDA_EXEC
#define sceMc2FileAttrClosed		SCE_MC2_FILE_ATTR_CLOSED
#define sceMc2McFileAttrSubdir		SCE_MC2_FILE_ATTR_SUBDIR
#define	sceMc2FileAttrDupProhibit	SCE_MC2_FILE_ATTR_DUPPROHIBIT
#define sceMc2FileAttrExecutable	SCE_MC2_FILE_ATTR_EXECUTABLE
#define sceMc2FileAttrWriteable		SCE_MC2_FILE_ATTR_WRITEABLE
#define sceMc2FileAttrReadable		SCE_MC2_FILE_ATTR_READABLE
/* メディア */
#define sceMc2TypeNoCard		SCE_MC2_TYPE_NO_CARD
#define sceMc2TypePS1			SCE_MC2_TYPE_PS1
#define sceMc2TypePS2			SCE_MC2_TYPE_PS2
#define sceMc2TypePDA			SCE_MC2_TYPE_PDA
/* 非同期関数処理状態マクロ */
#define sceMc2StatExecRun		SCE_MC2_STAT_EXEC_RUN
#define sceMc2StatExecFinish		SCE_MC2_STAT_EXEC_FINISH
#define sceMc2StatExecIdle		SCE_MC2_STAT_EXEC_IDLE

#endif /* SCE_MC2_OLD_DEFINE */



#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

int sceMc2Init(int);
int sceMc2End(void);
int sceMc2CreateSocket( SceMc2SocketParam* psocket, u_long128* paddr);
int sceMc2DeleteSocket( int id );
int sceMc2ChangeThreadPriority( int prio );

int sceMc2GetInfoAsync(int id, SceMc2InfoParam* info);
int sceMc2FormatAsync(int id);
int sceMc2UnformatAsync(int id);
int sceMc2ReadFileAsync(int id, const char* name, void* bufs, int ofs, unsigned int size);
int sceMc2WriteFileAsync(int id, const char* name, const void* bufs, int ofs, unsigned int size);
int sceMc2CreateFileAsync(int id, const char* name);
int sceMc2DeleteAsync(int id, const char* name);
int sceMc2RenameAsync(int id, const char* org_name, const char* new_name);
int sceMc2MkdirAsync(int id, const char* name);
int sceMc2ChdirAsync(int id, const char* name, char* current);
int sceMc2GetDirAsync(int id, const char* name, int ofs, int maxent, SceMc2DirParam* dir, int* count);
int sceMc2ChmodAsync(int id, const char* name, unsigned short attr);
int sceMc2SearchFileAsync(int id, const char* name, SceMc2DirParam* dir_param);
int sceMc2GetEntSpaceAsync(int id, const char* name);
int sceMc2CheckAsync(int* cmds, int* rslt);
int sceMc2Sync(int* cmds, int* rslt);

int sceMc2GetInfo(int id, SceMc2InfoParam* info);
int sceMc2Format(int id);
int sceMc2Unformat(int id);
int sceMc2ReadFile(int id, const char* name, void* bufs, int ofs, unsigned int size);
int sceMc2WriteFile(int id, const char* name, const void* bufs, int ofs, unsigned int size);
int sceMc2CreateFile(int id, const char* name);
int sceMc2Delete(int id, const char* name);
int sceMc2Rename(int id, const char* org_name, const char* new_name);
int sceMc2Mkdir(int id, const char* name);
int sceMc2Chdir(int id, const char* name, char* current);
int sceMc2GetDir(int id, const char* name, int ofs, int maxent, SceMc2DirParam* dir, int* count);
int sceMc2Chmod(int id, const char* name, unsigned short attr);
int sceMc2SearchFile(int id, const char* name, SceMc2DirParam* dir_param);
int sceMc2GetEntSpace(int id, const char* name);
int sceMc2SearchSocket(int port, int slot);
void *sceMc2GetErxEntries(void);

#if defined(__LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif
#endif /* _LIBMC2_H_ */
