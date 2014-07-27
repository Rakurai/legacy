/*	@(#)stdarg.h	8.3     (ULTRIX)        12/23/92	*/
/*
 * @DEC_COPYRIGHT@
 */
/*
 * HISTORY
 * $Log: stdarg.h,v $
 * Revision 1.1  2001/09/08 09:00:32  legacy
 * First CVS commit, adding all the files to the repository.
 *
 * Revision 4.2.2.3  92/03/12  18:06:05  Keith_Mortensen
 * 	"195-mortens"
 * 
 * Revision 4.2.3.2  92/03/09  15:30:01  Keith_Mortensen
 * 	MIPS 3.0 FCS Integration.
 * 
 * Revision 4.2.2.2  91/12/05  15:03:02  Jim_McHale
 * 	"Integrate with AG pool"
 * 
 * Revision 4.2.1.2  91/11/22  18:16:29  Jim_McHale
 * 	Merge with mips 3.0
 * 
 * Revision 4.2  91/09/20  05:06:21  devbld
 * 	Adding ODE Headers
 * 
 * $EndLog$
 */
/*
 *       @(#)$RCSfile: stdarg.h,v $ $Revision: 1.1 $ (DEC) $Date: 2005-01-20 06:09:16 -0500 (Thu, 20 Jan 2005) $
 */
/*
 * |-----------------------------------------------------------|
 * | Copyright (c) 1991 MIPS Computer Systems, Inc.            |
 * | All Rights Reserved                                       |
 * |-----------------------------------------------------------|
 * |          Restricted Rights Legend                         |
 * | Use, duplication, or disclosure by the Government is      |
 * | subject to restrictions as set forth in                   |
 * | subparagraph (c)(1)(ii) of the Rights in Technical        |
 * | Data and Computer Software Clause of DFARS 252.227-7013.  |
 * |         MIPS Computer Systems, Inc.                       |
 * |         950 DeGuigne Avenue                               |
 * |         Sunnyvale, California 94088-3650, USA             |
 * |-----------------------------------------------------------|
 */
/* $Header: /home/xenith/temp/src/stdarg.h,v 1.1 2001/09/08 09:00:32 legacy Exp $ */

/* 4.8 Variable arguments */

#include "ansi_compat.h"
#ifndef _STDARG_H
#define _STDARG_H

#ifndef _SYSTYPE_SVR4

#ifndef _VA_LIST_
#define _VA_LIST_
typedef char *va_list;
#endif /* _VA_LIST_ */

#define va_end(list)

#ifdef __host_mips

#if defined(_CFE)
#if defined(__STDC__) && (__STDC__ != 0 )
	/* va_start makes list point past the parmN */
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#else
#define va_start(list, name) (void) (list = (void *)((char *)&...))
#endif
#else
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#endif
        /* va_arg aligns list and points past data */
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */

#else /* vax */

#define va_start(list, parmN) list = (va_list)((int)&parmN + sizeof(parmN))
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

#endif

#else   /* _SYSTYPE_SVR4 */
#if (defined(__STDC__) || defined (__SVR4__STDC))

#ifndef _VA_LIST_
#define _VA_LIST_
typedef void *va_list;
#endif  /* _VA_LIST_ */

#define va_end(list)

#ifdef __host_mips

#if defined(_CFE)
#if defined(__STDC__) && (__STDC__ != 0 )
	/* va_start makes list point past the parmN */
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#else
#define va_start(list, name) (void) (list = (void *)((char *)&...))
#endif
#else
#define va_start(list, parmN) (list = ((char *)&parmN + sizeof(parmN)))
#endif
        /* va_arg aligns list and points past data */
#define va_arg(list, mode) ((mode *)(list =\
 (char *) ((((int)list + (__builtin_alignof(mode)<=4?3:7)) &\
 (__builtin_alignof(mode)<=4?-4:-8))+sizeof(mode))))[-1]

/*  +++++++++++++++++++++++++++++++++++++++++++
    Because of parameter passing conventions in C:
    use mode=int for char, and short types
    use mode=double for float types
    use a pointer for array types
    +++++++++++++++++++++++++++++++++++++++++++ */

#else /* vax */

#define va_start(list, parmN) list = (va_list)((int)&parmN + sizeof(parmN))
#define va_arg(list, mode) ((mode *)(list += sizeof(mode)))[-1]

#endif

#else	/* not __STDC__ */
#include <varargs.h>
#endif	/* __STDC__ */

#endif  /* _SYSTYPE_SVR4 */
#endif  /* _STDARG_H */
