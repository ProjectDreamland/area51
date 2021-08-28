/* CYGNUS LOCAL whole file */

#ifndef _SIMD_H_INCLUDED
#define _SIMD_H_INCLUDED

#if (__GNUC__ >= 2 && __GNUC_MINOR__ >= 9) \
    && (defined (R5900) || defined (_R5900) || defined (__R5900))

/* 128 bit integer type */
typedef int __m128  __attribute__ ((mode (TI)));

/* 64 bit integer type */
typedef int __m64   __attribute__ ((mode (DI)));

/* 128 bit vector types */
typedef int __v16qi __attribute__ ((mode (V16QI)));
typedef int __v8hi  __attribute__ ((mode (V8HI)));
typedef int __v4si  __attribute__ ((mode (V4SI)));
typedef int __v2di  __attribute__ ((mode (V2DI)));
typedef int __v2si  __attribute__ ((mode (V2SI)));

/* Builtin functions for SIMD instructions. Each builtin corresponds to a
   single SIMD instruction.  These macros allow the builtins to be called
   with any 128 bit integer of vector type as arguments.  */
#define _pabsh(A) ((__v8hi)__builtin_mips5900_pabsh ((__v8hi)(A)))
#define _pabsw(A) ((__v4si)__builtin_mips5900_pabsw ((__v4si)(A)))
#define _paddb(A,B) ((__v16qi)__builtin_mips5900_paddb ((__v16qi)(A),(__v16qi)(B)))
#define _paddh(A,B) ((__v8hi)__builtin_mips5900_paddh ((__v8hi)(A),(__v8hi)(B)))
#define _paddsb(A,B) ((__v16qi)__builtin_mips5900_paddsb ((__v16qi)(A),(__v16qi)(B)))
#define _paddsh(A,B) ((__v8hi)__builtin_mips5900_paddsh ((__v8hi)(A),(__v8hi)(B)))
#define _paddsw(A,B) ((__v4si)__builtin_mips5900_paddsw ((__v4si)(A),(__v4si)(B)))
#define _paddub(A,B) ((__v16qi)__builtin_mips5900_paddub ((__v16qi)(A),(__v16qi)(B)))
#define _padduh(A,B) ((__v8hi)__builtin_mips5900_padduh ((__v8hi)(A),(__v8hi)(B)))
#define _padduw(A,B) ((__v4si)__builtin_mips5900_padduw ((__v4si)(A),(__v4si)(B)))
#define _paddw(A,B) ((__v4si)__builtin_mips5900_paddw ((__v4si)(A),(__v4si)(B)))
#define _padsbh(A,B) ((__v8hi)__builtin_mips5900_padsbh ((__v8hi)(A),(__v8hi)(B)))
#define _pand(A,B) ((__v2di)__builtin_mips5900_pand ((__v2di)(A),(__v2di)(B)))
#define _pceqb(A,B) ((__v16qi)__builtin_mips5900_pceqb ((__v16qi)(A),(__v16qi)(B)))
#define _pceqh(A,B) ((__v8hi)__builtin_mips5900_pceqh ((__v8hi)(A),(__v8hi)(B)))
#define _pceqw(A,B) ((__v4si)__builtin_mips5900_pceqw ((__v4si)(A),(__v4si)(B)))
#define _pcgtb(A,B) ((__v16qi)__builtin_mips5900_pcgtb ((__v16qi)(A),(__v16qi)(B)))
#define _pcgth(A,B) ((__v8hi)__builtin_mips5900_pcgth ((__v8hi)(A),(__v8hi)(B)))
#define _pcgtw(A,B) ((__v4si)__builtin_mips5900_pcgtw ((__v4si)(A),(__v4si)(B)))
#define _pcpyh(A) ((__v8hi)__builtin_mips5900_pcpyh ((__v8hi)(A)))
#define _pcpyld(A,B) ((__v2di)__builtin_mips5900_pcpyld ((__v2di)(A),(__v2di)(B)))
#define _pcpyud(A,B) ((__v2di)__builtin_mips5900_pcpyud ((__v2di)(A),(__v2di)(B)))

#define _pdivbw(A,B) ((void)__builtin_mips5900_pdivbw ((__v4si)(A),(__v8hi)(B)))
#define _pdivuw(A,B) ((void)__builtin_mips5900_pdivuw ((__v2di)(A),(__v2di)(B)))
#define _pdivw(A,B) ((void)__builtin_mips5900_pdivw ((__v2di)(A),(__v2di)(B)))
#define _phmadh(A,B) ((__v4si)__builtin_mips5900_phmadh ((__v8hi)(A),(__v8hi)(B)))
#define _phmsbh(A,B) ((__v4si)__builtin_mips5900_phmsbh ((__v8hi)(A),(__v8hi)(B)))
#define _pmaddh(A,B) ((__v4si)__builtin_mips5900_pmaddh ((__v8hi)(A),(__v8hi)(B)))
#define _pmadduw(A,B) ((__v2di)__builtin_mips5900_pmadduw ((__v4si)(A),(__v4si)(B)))
#define _pmaddw(A,B) ((__v2di)__builtin_mips5900_pmaddw ((__v4si)(A),(__v4si)(B)))
#define _pmsubh(A,B) ((__v4si)__builtin_mips5900_pmsubh ((__v8hi)(A),(__v8hi)(B)))
#define _pmsubw(A,B) ((__v2di)__builtin_mips5900_pmsubw ((__v4si)(A),(__v4si)(B)))
#define _pmulth(A,B) ((__v4si)__builtin_mips5900_pmulth ((__v8hi)(A),(__v8hi)(B)))
#define _pmultuw(A,B) ((__v2di)__builtin_mips5900_pmultuw ((__v4si)(A),(__v4si)(B)))
#define _pmultw(A,B) ((__v2di)__builtin_mips5900_pmultw ((__v4si)(A),(__v4si)(B)))
#define _pexch(A) ((__v8hi)__builtin_mips5900_pexch ((__v8hi)(A)))
#define _pexcw(A) ((__v4si)__builtin_mips5900_pexcw ((__v4si)(A)))
#define _pexeh(A) ((__v8hi)__builtin_mips5900_pexeh ((__v8hi)(A)))
#define _pexew(A) ((__v4si)__builtin_mips5900_pexew ((__v4si)(A)))
#define _pext5(A) ((__v4si)__builtin_mips5900_pext5 ((__v8hi)(A)))
#define _pextlb(A,B) ((__v16qi)__builtin_mips5900_pextlb ((__v16qi)(A),(__v16qi)(B)))
#define _pextlh(A,B) ((__v8hi)__builtin_mips5900_pextlh ((__v8hi)(A),(__v8hi)(B)))
#define _pextlw(A,B) ((__v4si)__builtin_mips5900_pextlw ((__v4si)(A),(__v4si)(B)))
#define _pextub(A,B) ((__v16qi)__builtin_mips5900_pextub ((__v16qi)(A),(__v16qi)(B)))
#define _pextuh(A,B) ((__v8hi)__builtin_mips5900_pextuh ((__v8hi)(A),(__v8hi)(B)))
#define _pextuw(A,B) ((__v4si)__builtin_mips5900_pextuw ((__v4si)(A),(__v4si)(B)))
#define _pinth(A,B) ((__v8hi)__builtin_mips5900_pinth ((__v8hi)(A),(__v8hi)(B)))
#define _pinteh(A,B) ((__v8hi)__builtin_mips5900_pinteh ((__v8hi)(A),(__v8hi)(B)))
#define _plzcw(A) ((__v2si)__builtin_mips5900_plzcw ((__v2si)(A)))
#define _pmaxh(A,B) ((__v8hi)__builtin_mips5900_pmaxh ((__v8hi)(A),(__v8hi)(B)))
#define _pmaxw(A,B) ((__v4si)__builtin_mips5900_pmaxw ((__v4si)(A),(__v4si)(B)))
#define _pminh(A,B) ((__v8hi)__builtin_mips5900_pminh ((__v8hi)(A),(__v8hi)(B)))
#define _pminw(A,B) ((__v4si)__builtin_mips5900_pminw ((__v4si)(A),(__v4si)(B)))
#define _pnor(A,B) ((__v2di)__builtin_mips5900_pnor ((__v2di)(A),(__v2di)(B)))
#define _por(A,B) ((__v2di)__builtin_mips5900_por ((__v2di)(A),(__v2di)(B)))
#define _ppac5(A) ((__v8hi)__builtin_mips5900_ppac5 ((__v4si)(A)))
#define _ppacb(A,B) ((__v16qi)__builtin_mips5900_ppacb ((__v16qi)(A),(__v16qi)(B)))
#define _ppach(A,B) ((__v8hi)__builtin_mips5900_ppach ((__v8hi)(A),(__v8hi)(B)))
#define _ppacw(A,B) ((__v4si)__builtin_mips5900_ppacw ((__v4si)(A),(__v4si)(B)))
#define _prevh(A) ((__v8hi)__builtin_mips5900_prevh ((__v8hi)(A)))
#define _prot3w(A) ((__v4si)__builtin_mips5900_prot3w ((__v4si)(A)))
#define _psllh(A,B) ((__v8hi)__builtin_mips5900_psllh ((__v8hi)(A),(int)(B)))
#define _psllvw(A,B) ((__v2di)__builtin_mips5900_psllvw ((__v2di)(A),(__v2di)(B)))
#define _psllw(A,B) ((__v4si)__builtin_mips5900_psllw ((__v4si)(A),(int)(B)))
#define _psrah(A,B) ((__v8hi)__builtin_mips5900_psrah ((__v8hi)(A),(int)(B)))
#define _psravw(A,B) ((__v2di)__builtin_mips5900_psravw ((__v2di)(A),(__v2di)(B)))
#define _psraw(A,B) ((__v4si)__builtin_mips5900_psraw ((__v4si)(A),(int)(B)))
#define _psrlh(A,B) ((__v8hi)__builtin_mips5900_psrlh ((__v8hi)(A),(int)(B)))
#define _psrlvw(A,B) ((__v2di)__builtin_mips5900_psrlvw ((__v2di)(A),(__v2di)(B)))
#define _psrlw(A,B) ((__v4si)__builtin_mips5900_psrlw ((__v4si)(A),(int)(B)))
#define _psubb(A,B) ((__v16qi)__builtin_mips5900_psubb ((__v16qi)(A),(__v16qi)(B)))
#define _psubh(A,B) ((__v8hi)__builtin_mips5900_psubh ((__v8hi)(A),(__v8hi)(B)))
#define _psubsb(A,B) ((__v16qi)__builtin_mips5900_psubsb ((__v16qi)(A),(__v16qi)(B)))
#define _psubsh(A,B) ((__v8hi)__builtin_mips5900_psubsh ((__v8hi)(A),(__v8hi)(B)))
#define _psubsw(A,B) ((__v4si)__builtin_mips5900_psubsw ((__v4si)(A),(__v4si)(B)))
#define _psubub(A,B) ((__v16qi)__builtin_mips5900_psubub ((__v16qi)(A),(__v16qi)(B)))
#define _psubuh(A,B) ((__v8hi)__builtin_mips5900_psubuh ((__v8hi)(A),(__v8hi)(B)))
#define _psubuw(A,B) ((__v4si)__builtin_mips5900_psubuw ((__v4si)(A),(__v4si)(B)))
#define _psubw(A,B) ((__v4si)__builtin_mips5900_psubw ((__v4si)(A),(__v4si)(B)))
#define _pxor(A,B) ((__v2di)__builtin_mips5900_pxor ((__v2di)(A),(__v2di)(B)))

/* Funnel shifting functions.  */
#define _mtsab(A,B) ((void)__builtin_mips5900_mtsab ((int)(A),(int)(B)))
#define _mtsah(A,B) ((void)__builtin_mips5900_mtsah ((int)(A),(int)(B)))
#define _mtsa(A) ((void)__builtin_mips5900_mtsa ((__m64)(A)))
#define _mfsa() ((__m64)__builtin_mips5900_mfsa ())
#define _qfsrv(A,B) ((__m128)__builtin_mips5900_qfsrv ((__m128)(A),(__m128)(B)))

/* handle HI and LO */
#define _pmfhi() ((__v2di)__builtin_mips5900_pmfhi ())
#define _pmflo() ((__v2di)__builtin_mips5900_pmflo ())
#define _pmfhl_lw() ((__v4si)__builtin_mips5900_pmfhl_lw ())
#define _pmfhl_uw() ((__v4si)__builtin_mips5900_pmfhl_uw ())
#define _pmfhl_slw() ((__v2di)__builtin_mips5900_pmfhl_slw ())
#define _pmfhl_lh() ((__v8hi)__builtin_mips5900_pmfhl_lh ())
#define _pmfhl_sh() ((__v8hi)__builtin_mips5900_pmfhl_sh ())

#define _pmthi(A) ((void)__builtin_mips5900_pmthi ((__v2di)(A)))
#define _pmtlo(A) ((void)__builtin_mips5900_pmtlo ((__v2di)(A)))
#define _pmthl_lw(A) ((void)__builtin_mips5900_pmthl_lw ((__v4si)(A)))

#endif /* (__GNUC__ >= 2 && __GNUC_MINOR__ >= 9) \
          && (defined (R5900) || defined (_R5900) || defined (__R5900)) */

#endif /* _SIMD_H_INCLUDED */
