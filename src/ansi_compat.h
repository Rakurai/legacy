/*	@(#)ansi_compat.h	8.4     (ULTRIX)        5/3/93	*/
/*
 * 	@(#)ansi_compat.h	6.1	(ULTRIX)	11/19/91
 */

/************************************************************************
 *									*
 *			Copyright (c) 1990 by			        *
 *		Digital Equipment Corporation, Maynard, MA		*
 *			All rights reserved.				*
 *									*
 *   This software is furnished under a license and may be used and	*
 *   copied  only  in accordance with the terms of such license and	*
 *   with the  inclusion  of  the  above  copyright  notice.   This	*
 *   software  or  any  other copies thereof may not be provided or	*
 *   otherwise made available to any other person.  No title to and	*
 *   ownership of the software is hereby transferred.			*
 *									*
 *   The information in this software is subject to change  without	*
 *   notice  and should not be construed as a commitment by Digital	*
 *   Equipment Corporation.						*
 *									*
 *   Digital assumes no responsibility for the use  or  reliability	*
 *   of its software on equipment which is not supplied by Digital.	*
 *									*
 ************************************************************************/

/*
 *   To avoid namespace pollution when using the ULTRIX header files under the
 * DEC ANSI compiler, all user-visible header files were modifed to reference
 * ANSI-style predefined macro name rather than their traditional names
 * (__ultrix vice ultrix).  Every file which accesses a predefined macro name
 * must include this file before any other files are included or the macros
 * are tested.
 *
 *   In strict ANSI mode, the appropriate ANSI-style macros are already
 * defined and the redefinitions in this file will not be seen.  When using
 * pcc, the traditional macro names are defined and this file will define
 * ANSI-style equivalents of the traditional names.  When using the DEC C
 * compiler, both the traditional and ANSI predefined macro names are
 * available so the definitions in this file are not made visible.
 *
 */


#if !defined(__STDC__) && !defined(__DECC) && !defined(__ANSI_COMPAT) && !defined(_CFE) 

#define __ANSI_COMPAT

#if defined(ultrix) && !defined(__ultrix)
#define	__ultrix      ultrix
#endif

#if defined(unix) && !defined(__unix)
#define	__unix        unix
#endif

#if defined(bsd4_2) && !defined(__bsd4_2)
#define	__bsd4_2      bsd4_2
#endif

#if defined(BSD) && !defined(_BSD)
#define	_BSD          BSD
#endif

#if defined(vax) && !defined(__vax)
#define __vax 	      vax
#endif

#if defined(VAX) && !defined(__VAX)
#define __VAX 	      VAX
#endif

#if defined(mips) && !defined(__mips)
#define	__mips        mips
#endif

#if defined(host_mips) && !defined(__host_mips)
#define	__host_mips   host_mips
#endif

#if defined(MIPSEL) && !defined(__MIPSEL)
#define	__MIPSEL      MIPSEL
#endif

#if defined(MIPSEB) && !defined(__MIPSEB)
#define	__MIPSEB      MIPSEB
#endif

#if defined(SYSTEM_FIVE) && !defined(__SYSTEM_FIVE)
#define	__SYSTEM_FIVE SYSTEM_FIVE
#endif

#if defined(POSIX) && !defined(__POSIX)
#define	__POSIX       POSIX
#endif

#if defined(GFLOAT) && !defined(__GFLOAT)
#define __GFLOAT	GFLOAT
#endif

#if defined(LANGUAGE_C) && !defined(__LANGUAGE_C)
#define	__LANGUAGE_C  LANGUAGE_C
#endif

#if defined(LANGUAGE_PASCAL) && !defined(__LANGUAGE_PASCAL)
#define	__LANGUAGE_PASCAL LANGUAGE_PASCAL
#endif

#if defined(LANGUAGE_FORTRAN) && !defined(__LANGUAGE_FORTRAN)
#define	__LANGUAGE_FORTRAN LANGUAGE_FORTRAN
#endif

#if defined(LANGUAGE_ASSEMBLY) && !defined(__LANGUAGE_ASSEMBLY)
#define	__LANGUAGE_ASSEMBLY LANGUAGE_ASSEMBLY
#endif

#ifdef vaxc
#ifndef __vaxc
#define __vaxc	 vaxc
#endif
#ifndef __VAXC
#define __VAXC   VAXC
#endif
#ifndef __vax11c
#define __vax11c vax11c
#endif
#ifndef __VAX11C
#define __VAX11C VAX11C
#endif
#endif

#if defined(MOXIE) && !defined(__MOXIE)
#define __MOXIE   MOXIE
#endif

#if defined(ULTRIX022) && !defined(__ULTRIX022)
#define __ULTRIX022 ULTRIX022
#endif

#endif /* !(__STDC__) && !(__DECC) && !(__ANSI_COMPAT) && !(_CFE) */
