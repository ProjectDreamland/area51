/* SCE CONFIDENTIAL
 "PlayStation 2" Programmer Tool Runtime Library Release 3.0.2
 $Id: liberx.h,v 1.25 2004/06/25 13:47:26 kono Exp $
*/
/* 
 *                      Emotion Engine Library
 *                          Version 0.00
 *                           Shift-JIS
 *
 *      Copyright (C) 1998-2003 Sony Computer Entertainment Inc.
 *                        All Rights Reserved.
 *
 *                       liberx - liberx.h
 *                     header file of liberx
 *
 *       Version        Date            assign      Log
 *  --------------------------------------------------------------------
 *      0.00          2002-11-29        kono        the first version
 *      0.01          2003-04-07        kono        add self stop-unload
 *      0.02          2003-04-15        kono        add async ver function and
 *                                                  load from buffer ver function
 *      0.03          2003-04-25        kono        apply to official error code
 *      0.04          2003-05-16        kono        add english comment
 *                                                  sorry my poor english
 *                                                  please read my native language
 *                                                  (i.e. japanese) comment...
 *                                                  I have no confidence.
 *      0.05          2003-09-02        kono        add SCE_ERX_ENCRYPT_DNAS_NOHDD
 *                                                  flag. for DNAS-inst(nohdd)
 *      0.06          2004-06-25        kono        add sceErxSelfStopUnloadLoadStartModuleFile()
 */

#ifndef _LIBERX_H
#define _LIBERX_H

#include <sys/types.h>
#include <eekernel.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

/* モジュールのエントリ関数が呼ばれたときの理由コード */
#define SCE_ERX_REASON_START			(0)		/* モジュールの起動時 */
#define SCE_ERX_REASON_STOP				(1)		/* モジュールの停止時 */
#define SCE_ERX_REASON_RESTART			(2)		/* モジュールの再起動時(未サポート) */

/* モジュールのエントリ関数の返り値を定義 */
#define SCE_ERX_RESIDENT_END			(0)		/* 常駐終了 */
#define SCE_ERX_NO_RESIDENT_END			(1)		/* 非常駐終了 */
#define SCE_ERX_FAREWELL_END			(1)		/* 非常駐終了 */
#define SCE_ERX_REMOVABLE_RESIDENT_END	(2)		/* 常駐終了(アンロード対応) */

/* メモリアロケート戦術 */
#define SCE_ERX_SMEM_LOW				(0x00000000U)		/* 下位側アドレスよりメモリ確保 */
#define SCE_ERX_SMEM_HIGH				(0x00000001U)		/* 上位側アドレスよりメモリ確保 */
#define SCE_ERX_SMEM_ADDR				(0x00000002U)		/* アドレス指定してメモリ確保 */
#define SCE_ERX_SMEM_LOWALIGNED			(0x00000004U)		/* 下位側アドレスよりアラインメント指定つきメモリ確保 */
#define SCE_ERX_SMEM_HIGHALIGNED		(0x00000005U)		/* 上位側アドレスよりアラインメント指定つきメモリ確保 */

/* モジュールファイルロード時のファイルアクセス指定 */
#define SCE_ERX_ACCESS_NOSEEK			(0x00000000U)		/* シークなし,   ワークメモリ使用量大 */
#define SCE_ERX_ACCESS_SEEKFEW			(0x00000100U)		/* シーク回数少, ワークメモリ使用量中 */
#define SCE_ERX_ACCESS_SEEKMANY			(0x00000200U)		/* シーク回数多, ワークメモリ使用なし */

/* モジュールの圧縮指定 */
#define SCE_ERX_ENCODE_GZIP				(0x00001000U)		/* gzip圧縮指定 */

/* モジュールの暗号化指定 */
#define SCE_ERX_ENCRYPT_DNAS			(0x00002000U)		/* DNAS(hdd)暗号化指定 */
#define SCE_ERX_ENCRYPT_DNAS_NOHDD		(0x00004000U)		/* DNAS(nohdd)暗号化指定 */

/* liberx library*/
int sceErxInit(int iPriority, void *pStart, u_int uiSize, int nMaxModules);
int sceErxExit(void);

/* sysmem library */
void *sceErxAllocSysMemory(int type, u_int size, void *addr);
int   sceErxFreeSysMemory(void *area);
int   sceErxRecordMemConsumption(int fEnable);
u_int sceErxQueryMemSize(void);
int   sceErxQueryFreeMemSize(u_int *puiTotalFree, u_int *puiMaxFree);
int   sceErxQueryFreeMemSticky(u_int *puiTotalFree, u_int *puiMaxFree);
int   sceErxQueryBlockInfo(const void *addr, void **ppBlkStart, u_int *puiSize);
int   sceErxQueryRegionType(const void *addr, u_int size);

/* loadcore library */
typedef struct SceErxLibraryHeader {
	u_int	magic[4];		/* library header magic(0x41C00000) */
	const char *name;		/* ライブラリ名(ASCIZ) */
	u_short	version;		/* ライブラリのバージョン */
	u_short	flags;
	u_int reserved[2];
} SceErxLibraryHeader;

typedef struct SceErxModuleStatus {
	char	name[56];		/* モジュール名 */
	u_short	version;		/* モジュールのバージョン */
	u_short flags;			/* モジュールの状態 */
	int		id;				/* モジュールを識別する ID 番号 */
	u_int	entry_addr;		/* モジュールの実行開始アドレス */
	u_int	gp_value;		/* モジュールの GP レジスタ値 */
	u_int	text_addr;		/* モジュールの先頭アドレス */
	u_int	text_size;		/* モジュールのテキストセグメントサイズ */
	u_int	data_size;		/* モジュールのデータセグメントサイズ */
	u_int	bss_size;		/* モジュールの未初期化データセグメントサイズ */
	u_int   erx_lib_addr;	/* ERXライブラリエントリの開始アドレス */
	u_int   erx_lib_size;	/* ERXライブラリエントリのサイズ */
	u_int   erx_stub_addr;	/* ERXライブラリスタブの開始アドレス */
	u_int   erx_stub_size;	/* ERXライブラリスタブのサイズ */
} SceErxModuleStatus;

/* SceErxModuleStatus.flags */
#define SCE_ERX_MSTF_LOADED			(0x0001)		/* ロード直後 */
#define SCE_ERX_MSTF_EXEC			(0x0002)		/* ロード後のエントリルーチンを実行中 */
#define SCE_ERX_MSTF_RESIDENT		(0x0003)		/* 常駐状態 */
#define SCE_ERX_MSTF_STOPPING		(0x0004)		/* 停止のためにエントリルーチンを実行中 */
#define SCE_ERX_MSTF_SelfSTOPPING	(0x0005)		/* 自己停止のためにエントリルーチンを実行中 */
#define SCE_ERX_MSTF_STOPPED		(0x0006)		/* 停止状態 */
#define SCE_ERX_MSTF_SelfSTOPPED	(0x0007)		/* 自己停止状態 */
#define SCE_ERX_MSTF_MASK(x)		((x) & 0x000F)

#define SCE_ERX_MSTF_REMOVABLE		(0x0010)		/* アンロード対応 */
#define SCE_ERX_MSTF_NOSYSALLOC		(0x0020)		/* sysmem管理外のメモリにロードされた */
#define SCE_ERX_MSTF_CLEARMOD		(0x0040)		/* アンロード時のメモリクリア設定 */

/* 常駐ライブラリのエントリテーブル登録 */
int   sceErxRegisterLibraryEntries(SceErxLibraryHeader *lib);

/* エントリテーブルの登録抹消 */
int   sceErxReleaseLibraryEntries(SceErxLibraryHeader *lib);

/* 常駐ライブラリを使用しているクライアントの個数を取得 */
int   sceErxGetLibraryClients(const SceErxLibraryHeader *pLibHead);

int   sceErxSearchModuleByName(const char *modulename);
int   sceErxSearchModuleByAddress(const void *addr);
int   sceErxGetModuleIdList(int *readbuf, int readbufsize, int *modulecount);
int   sceErxGetModuleIdListByName(const char *modulename, int *readbuf, int readbufsize, int *modulecount);
int   sceErxReferModuleStatus(int modid, SceErxModuleStatus *status);

int   sceErxSetModuleFlags(int modid, int flag);


/* modload library */
typedef struct {
	const char		*name;
	unsigned int	version;
} SceErxModuleInfo;

int   sceErxLoadModuleFile(const char *filename, u_int mode, void *addr);
int   sceErxLoadModuleFileAsync(const char *filename, u_int mode, void *addr);
int   sceErxLoadModuleBuffer(u_int *inbuf, u_int objsize, u_int mode, void *addr);
int   sceErxLoadModuleBufferAsync(u_int *inbuf, u_int objsize, u_int mode, void *addr);
int   sceErxLoadStartModuleFile(const char *filename, u_int mode, void *addr, int args, const char *argp, int *result);
int   sceErxLoadStartModuleFileAsync(const char *filename, u_int mode, void *addr, int args, const char *argp, int *result);
int   sceErxLoadStartModuleBuffer(u_int *inbuf, u_int objsize, u_int mode, void *addr,
									const char *filename, int args, const char *argp, int *result);
int   sceErxLoadStartModuleBufferAsync(u_int *inbuf, u_int objsize, u_int mode, void *addr,
									const char *filename, int args, const char *argp, int *result);

int   sceErxStartModule(int modid, const char *filename, int args, const char *argp, int *result);
int   sceErxStartModuleAsync(int modid, const char *filename, int args, const char *argp, int *result);
int   sceErxStopModule(int modid, int args, const char *argp, int *result);
int   sceErxStopModuleAsync(int modid, int args, const char *argp, int *result);
int   sceErxUnloadModule(int modid);
int   sceErxUnloadModuleAsync(int modid);
int   sceErxStopUnloadModule(int modid, int args, const char *argp, int *result);
int   sceErxStopUnloadModuleAsync(int modid, int args, const char *argp, int *result);

int   sceErxSelfStopUnloadModule(int args, const char *argp, int *result);
int   sceErxSelfStopUnloadLoadStartModuleFile(int stopargs, const char *stopargp, int *modresult,
												const char *filename, u_int mode, void *addr, int startargs, const char *startargp);

int   sceErxSync(int mode, int *result);

extern __inline__ void *sceErxQueryGp(void);		/* 現在の$gpレジスタ値を取得 */
extern __inline__ void *sceErxSetGp(void *newgp);	/* $gpレジスタに新しい値を設定(戻り値は現在の$gp値) */
void *sceErxGetModuleGp(void *target);				/* 指定アドレスを含むモジュールの_gpアドレス値を取得 */
extern __inline__ void *sceErxSetModuleGp(void);	/* $gpレジスタにこのモジュールでの_gpアドレス値を設定(戻り値は現在の$gp値) */

/* 現在の$gpレジスタ値を取得 */
extern __inline__ void *sceErxQueryGp(void)
{
	void *gp;
	__asm__ volatile (
		"move		%0, $28\n"
		: "=r"(gp)
		:
		: "memory"
	);
	return (gp);
}

/* $gpレジスタに新しい値を設定(戻り値は現在の$gp値) */
extern __inline__ void *sceErxSetGp(void *newgp)
{
	void *oldgp;
	__asm__ volatile (
		"move		%0, $28\n"
		"move		$28, %1\n"
		: "=&r"(oldgp)
		: "r"(newgp)
		: "memory"
	);
	return (oldgp);
}

/* $gpレジスタにこのモジュールでの_gp値を設定(戻り値は現在の$gp値) */
extern __inline__ void *sceErxSetModuleGp(void)
{
	void *oldgp;
	__asm__ volatile (
		"move		%0, $28\n"
		"move		$28, %1\n"
		: "=&r"(oldgp)
		: "r"(&_gp)
		: "memory"
	);
	return (oldgp);
}


/* $gpレジスタ値の保存,復帰つき関数呼び出し */
/* スタックのデータは128バイトを上限としてコピー,戻り値はポインタ型を想定 */
#if defined(SCE_ERX_USE_GPEXECHELPER)

/* 呼び出し先関数ポインタが含まれるモジュールから_gpを特定する */
void *sceErxExecModuleFunction(void (*pFunc)(), ...);

#if !defined(__GNUC__)

/* $gpレジスタの値を明示的に指定して関数呼び出し */
void *sceErxExecGpFunction(void *newgp, void (*pFunc)(), ...);

#else	/* !defined(__GNUC__) */

/* $gpレジスタの値を明示的に指定して関数呼び出し */
extern __inline__ void *sceErxExecGpFunction(void *newgp, void (*pFunc)(), ...);

extern __inline__ void *sceErxExecGpFunction(void *newgp, void (*pFunc)(), ...)
{
	void *oldgp;
	void *pReturn;

	/* $gpレジスタの保存と新しい$gpレジスタ値設定 */
	oldgp = sceErxSetGp(newgp);

	/* 間接的な関数の呼び出し */
	pReturn  = __builtin_apply(pFunc, __builtin_apply_args(), 128);

	/* $gpレジスタ値復帰 */
	sceErxSetGp(oldgp);
	__builtin_return (pReturn);
}
#endif	/* !defined(__GNUC__) */
#endif	/* SCE_ERX_USE_GPEXECHELPER */

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif	/* _LIBERX_H */
