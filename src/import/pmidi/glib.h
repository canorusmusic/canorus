/* GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/*
 * Modified by the GLib Team and others 1997-1999.  See the AUTHORS
 * file for a list of people on the GLib Team.  See the ChangeLog
 * files for a list of changes.  These files are distributed with
 * GLib at ftp://ftp.gtk.org/pub/gtk/. 
 * Modified: Steve Ratcliffe May 1999.  File exists so that pmidi
 * is not dependant on glib.
 */

#ifndef __G_LIB_H__
#define __G_LIB_H__

#include <stdlib.h>
#include <string.h>

/* system specific config file glibconfig.h provides definitions for
 * the extrema of many of the standard types. These are:
 *
 *  G_MINSHORT, G_MAXSHORT
 *  G_MININT, G_MAXINT
 *  G_MINLONG, G_MAXLONG
 *  G_MINFLOAT, G_MAXFLOAT
 *  G_MINDOUBLE, G_MAXDOUBLE
 *
 * It also provides the following typedefs:
 *
 *  gint8, guint8
 *  gint16, guint16
 *  gint32, guint32
 *  gint64, guint64
 *
 * It defines the G_BYTE_ORDER symbol to one of G_*_ENDIAN (see later in
 * this file). 
 *
 * And it provides a way to store and retrieve a `gint' in/from a `gpointer'.
 * This is useful to pass an integer instead of a pointer to a callback.
 *
 *  GINT_TO_POINTER(i), GUINT_TO_POINTER(i)
 *  GPOINTER_TO_INT(p), GPOINTER_TO_UINT(p)
 *
 * Finally, it provide the following wrappers to STDC functions:
 *
 *  g_ATEXIT
 *    To register hooks which are executed on exit().
 *    Usually a wrapper for STDC atexit.
 *
 *  void *g_memmove(void *dest, const void *src, guint count);
 *    A wrapper for STDC memmove, or an implementation, if memmove doesn't
 *    exist.  The prototype looks like the above, give or take a const,
 *    or size_t.
 */

typedef signed char gint8;
typedef unsigned char guint8;
typedef signed short gint16;
typedef unsigned short guint16;
typedef signed int gint32;
typedef unsigned int guint32;

/* include varargs functions for assertment macros
 */
#include <stdarg.h>

/* optionally feature DMALLOC memory allocation debugger
 */
#ifdef USE_DMALLOC
#include "dmalloc.h"
#endif


#ifdef NATIVE_WIN32

/* On native Win32, directory separator is the backslash, and search path
 * separator is the semicolon.
 */
#define G_DIR_SEPARATOR '\\'
#define G_DIR_SEPARATOR_S "\\"
#define G_SEARCHPATH_SEPARATOR ';'
#define G_SEARCHPATH_SEPARATOR_S ";"

#else  /* !NATIVE_WIN32 */

/* Unix */

#define G_DIR_SEPARATOR '/'
#define G_DIR_SEPARATOR_S "/"
#define G_SEARCHPATH_SEPARATOR ':'
#define G_SEARCHPATH_SEPARATOR_S ":"

#endif /* !NATIVE_WIN32 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


/* Provide definitions for some commonly used macros.
 *  Some of them are only provided if they haven't already
 *  been defined. It is assumed that if they are already
 *  defined then the current definition is correct.
 */
#ifndef	NULL
#define	NULL	((void*) 0)
#endif

#ifndef	FALSE
#define	FALSE	(0)
#endif

#ifndef	TRUE
#define	TRUE	(!FALSE)
#endif

#undef	MAX
#define MAX(a, b)  (((a) > (b)) ? (a) : (b))

#undef	MIN
#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#undef	ABS
#define ABS(a)	   (((a) < 0) ? -(a) : (a))

#undef	CLAMP
#define CLAMP(x, low, high)  (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))


/* Define G_VA_COPY() to do the right thing for copying va_list variables.
 * glibconfig.h may have already defined G_VA_COPY as va_copy or __va_copy.
 */
#if !defined (G_VA_COPY)
#  if defined (__GNUC__) && defined (__PPC__) && (defined (_CALL_SYSV) || defined (_WIN32))
#  define G_VA_COPY(ap1, ap2)	  (*(ap1) = *(ap2))
#  elif defined (G_VA_COPY_AS_ARRAY)
#  define G_VA_COPY(ap1, ap2)	  g_memmove ((ap1), (ap2), sizeof (va_list))
#  else /* va_list is a pointer */
#  define G_VA_COPY(ap1, ap2)	  ((ap1) = (ap2))
#  endif /* va_list is a pointer */
#endif /* !G_VA_COPY */


/* Provide convenience macros for handling structure
 * fields through their offsets.
 */
#define G_STRUCT_OFFSET(struct_type, member)	\
    ((gulong) ((gchar*) &((struct_type*) 0)->member))
#define G_STRUCT_MEMBER_P(struct_p, struct_offset)   \
    ((gpointer) ((gchar*) (struct_p) + (gulong) (struct_offset)))
#define G_STRUCT_MEMBER(member_type, struct_p, struct_offset)   \
    (*(member_type*) G_STRUCT_MEMBER_P ((struct_p), (struct_offset)))


/* inlining hassle. for compilers that don't allow the `inline' keyword,
 * mostly because of strict ANSI C compliance or dumbness, we try to fall
 * back to either `__inline__' or `__inline'.
 * we define G_CAN_INLINE, if the compiler seems to be actually
 * *capable* to do function inlining, in which case inline function bodys
 * do make sense. we also define G_INLINE_FUNC to properly export the
 * function prototypes if no inlining can be performed.
 * we special case most of the stuff, so inline functions can have a normal
 * implementation by defining G_INLINE_FUNC to extern and G_CAN_INLINE to 1.
 */
#ifndef G_INLINE_FUNC
#  define G_CAN_INLINE 1
#endif
#ifdef G_HAVE_INLINE
#  if defined (__GNUC__) && defined (__STRICT_ANSI__)
#    undef inline
#    define inline __inline__
#  endif
#else /* !G_HAVE_INLINE */
#  undef inline
#  if defined (G_HAVE___INLINE__)
#    define inline __inline__
#  else /* !inline && !__inline__ */
#    if defined (G_HAVE___INLINE)
#      define inline __inline
#    else /* !inline && !__inline__ && !__inline */
#      define inline /* don't inline, then */
#      ifndef G_INLINE_FUNC
#	 undef G_CAN_INLINE
#      endif
#    endif
#  endif
#endif
#ifndef G_INLINE_FUNC
#  ifdef __GNUC__
#    ifdef __OPTIMIZE__
#      define G_INLINE_FUNC extern inline
#    else
#      undef G_CAN_INLINE
#      define G_INLINE_FUNC extern
#    endif
#  else /* !__GNUC__ */
#    ifdef G_CAN_INLINE
#      define G_INLINE_FUNC static inline
#    else
#      define G_INLINE_FUNC extern
#    endif
#  endif /* !__GNUC__ */
#endif /* !G_INLINE_FUNC */


/* Provide simple macro statement wrappers (adapted from Perl):
 *  G_STMT_START { statements; } G_STMT_END;
 *  can be used as a single statement, as in
 *  if (x) G_STMT_START { ... } G_STMT_END; else ...
 *
 *  For gcc we will wrap the statements within `({' and `})' braces.
 *  For SunOS they will be wrapped within `if (1)' and `else (void) 0',
 *  and otherwise within `do' and `while (0)'.
 */
#if !(defined (G_STMT_START) && defined (G_STMT_END))
#  if defined (__GNUC__) && !defined (__STRICT_ANSI__) && !defined (__cplusplus)
#    define G_STMT_START	(void)(
#    define G_STMT_END		)
#  else
#    if (defined (sun) || defined (__sun__))
#      define G_STMT_START	if (1)
#      define G_STMT_END	else (void)0
#    else
#      define G_STMT_START	do
#      define G_STMT_END	while (0)
#    endif
#  endif
#endif


/* Provide macros to feature the GCC function attribute.
 */
#if	__GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4)
#define G_GNUC_PRINTF( format_idx, arg_idx )	\
  __attribute__((format (printf, format_idx, arg_idx)))
#define G_GNUC_SCANF( format_idx, arg_idx )	\
  __attribute__((format (scanf, format_idx, arg_idx)))
#define G_GNUC_FORMAT( arg_idx )		\
  __attribute__((format_arg (arg_idx)))
#define G_GNUC_NORETURN				\
  __attribute__((noreturn))
#define G_GNUC_CONST				\
  __attribute__((const))
#define G_GNUC_UNUSED				\
  __attribute__((unused))
#else	/* !__GNUC__ */
#define G_GNUC_PRINTF( format_idx, arg_idx )
#define G_GNUC_SCANF( format_idx, arg_idx )
#define G_GNUC_FORMAT( arg_idx )
#define G_GNUC_NORETURN
#define G_GNUC_CONST
#define	G_GNUC_UNUSED
#endif	/* !__GNUC__ */


/* Wrap the gcc __PRETTY_FUNCTION__ and __FUNCTION__ variables with
 * macros, so we can refer to them as strings unconditionally.
 */
#ifdef	__GNUC__
#define	G_GNUC_FUNCTION		__FUNCTION__
#define	G_GNUC_PRETTY_FUNCTION	__PRETTY_FUNCTION__
#else	/* !__GNUC__ */
#define	G_GNUC_FUNCTION		""
#define	G_GNUC_PRETTY_FUNCTION	""
#endif	/* !__GNUC__ */

/* we try to provide a usefull equivalent for ATEXIT if it is
 * not defined, but use is actually abandoned. people should
 * use g_atexit() instead.
 */
#ifndef ATEXIT
# define ATEXIT(proc)	g_ATEXIT(proc)
#else
# define G_NATIVE_ATEXIT
#endif /* ATEXIT */

/* Hacker macro to place breakpoints for elected machines.
 * Actual use is strongly deprecated of course ;)
 */
#if defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#define	G_BREAKPOINT()		G_STMT_START{ __asm__ __volatile__ ("int $03"); }G_STMT_END
#elif defined (__alpha__) && defined (__GNUC__) && __GNUC__ >= 2
#define	G_BREAKPOINT()		G_STMT_START{ __asm__ __volatile__ ("bpt"); }G_STMT_END
#else	/* !__i386__ && !__alpha__ */
#define	G_BREAKPOINT()
#endif	/* __i386__ */


/* Provide macros for easily allocating memory. The macros
 *  will cast the allocated memory to the specified type
 *  in order to avoid compiler warnings. (Makes the code neater).
 */

#  define g_new(type, count)	  \
      ((type *) malloc ((unsigned) sizeof (type) * (count)))
#  define g_new0(type, count)	  \
      ((type *) calloc ((unsigned) sizeof (type) * (count), 1))
#  define g_renew(type, mem, count)	  \
      ((type *) realloc (mem, (unsigned) sizeof (type) * (count)))

#define g_mem_chunk_create(type, pre_alloc, alloc_type)	( \
  g_mem_chunk_new (#type " mem chunks (" #pre_alloc ")", \
		   sizeof (type), \
		   sizeof (type) * (pre_alloc), \
		   (alloc_type)) \
)
#define g_chunk_new(type, chunk)	( \
  (type *) g_mem_chunk_alloc (chunk) \
)
#define g_chunk_new0(type, chunk)	( \
  (type *) g_mem_chunk_alloc0 (chunk) \
)
#define g_chunk_free(mem, mem_chunk)	G_STMT_START { \
  g_mem_chunk_free ((mem_chunk), (mem)); \
} G_STMT_END


#define g_string(x) #x


/* Provide macros for error handling. The "assert" macros will
 *  exit on failure. The "return" macros will exit the current
 *  function. Two different definitions are given for the macros
 *  if G_DISABLE_ASSERT is not defined, in order to support gcc's
 *  __PRETTY_FUNCTION__ capability.
 */

#ifdef G_DISABLE_ASSERT

#define g_assert(expr)
#define g_assert_not_reached()

#else /* !G_DISABLE_ASSERT */

#ifdef __GNUC__

#define g_assert(expr)			G_STMT_START{		\
     if (!(expr))						\
       g_log (G_LOG_DOMAIN,					\
	      G_LOG_LEVEL_ERROR,				\
	      "file %s: line %d (%s): assertion failed: (%s)",	\
	      __FILE__,						\
	      __LINE__,						\
	      __PRETTY_FUNCTION__,				\
	      #expr);			}G_STMT_END

#define g_assert_not_reached()		G_STMT_START{		\
     g_log (G_LOG_DOMAIN,					\
	    G_LOG_LEVEL_ERROR,					\
	    "file %s: line %d (%s): should not be reached",	\
	    __FILE__,						\
	    __LINE__,						\
	    __PRETTY_FUNCTION__);	}G_STMT_END

#else /* !__GNUC__ */

#define g_assert(expr)			G_STMT_START{		\
     if (!(expr))						\
       g_log (G_LOG_DOMAIN,					\
	      G_LOG_LEVEL_ERROR,				\
	      "file %s: line %d: assertion failed: (%s)",	\
	      __FILE__,						\
	      __LINE__,						\
	      #expr);			}G_STMT_END

#define g_assert_not_reached()		G_STMT_START{	\
     g_log (G_LOG_DOMAIN,				\
	    G_LOG_LEVEL_ERROR,				\
	    "file %s: line %d: should not be reached",	\
	    __FILE__,					\
	    __LINE__);		}G_STMT_END

#endif /* __GNUC__ */

#endif /* !G_DISABLE_ASSERT */


#ifdef G_DISABLE_CHECKS

#define g_return_if_fail(expr)
#define g_return_val_if_fail(expr,val)

#else /* !G_DISABLE_CHECKS */

#ifdef __GNUC__

#define g_return_if_fail(expr)		G_STMT_START{			\
     if (!(expr))							\
       {								\
	 g_log (G_LOG_DOMAIN,						\
		G_LOG_LEVEL_CRITICAL,					\
		"file %s: line %d (%s): assertion `%s' failed.",	\
		__FILE__,						\
		__LINE__,						\
		__PRETTY_FUNCTION__,					\
		#expr);							\
	 return;							\
       };				}G_STMT_END

#define g_return_val_if_fail(expr,val)	G_STMT_START{			\
     if (!(expr))							\
       {								\
	 g_log (G_LOG_DOMAIN,						\
		G_LOG_LEVEL_CRITICAL,					\
		"file %s: line %d (%s): assertion `%s' failed.",	\
		__FILE__,						\
		__LINE__,						\
		__PRETTY_FUNCTION__,					\
		#expr);							\
	 return val;							\
       };				}G_STMT_END

#else /* !__GNUC__ */

#define g_return_if_fail(expr)		G_STMT_START{		\
     if (!(expr))						\
       {							\
	 g_log (G_LOG_DOMAIN,					\
		G_LOG_LEVEL_CRITICAL,				\
		"file %s: line %d: assertion `%s' failed.",	\
		__FILE__,					\
		__LINE__,					\
		#expr);						\
	 return;						\
       };				}G_STMT_END

#define g_return_val_if_fail(expr, val)	G_STMT_START{		\
     if (!(expr))						\
       {							\
	 g_log (G_LOG_DOMAIN,					\
		G_LOG_LEVEL_CRITICAL,				\
		"file %s: line %d: assertion `%s' failed.",	\
		__FILE__,					\
		__LINE__,					\
		#expr);						\
	 return val;						\
       };				}G_STMT_END

#endif /* !__GNUC__ */

#endif /* !G_DISABLE_CHECKS */


/* Provide type definitions for commonly used types.
 *  These are useful because a "gint8" can be adjusted
 *  to be 1 byte (8 bits) on all platforms. Similarly and
 *  more importantly, "gint32" can be adjusted to be
 *  4 bytes (32 bits) on all platforms.
 */

typedef char   gchar;
typedef short  gshort;
typedef long   glong;
typedef int    gint;
typedef gint   gboolean;

typedef unsigned char	guchar;
typedef unsigned short	gushort;
typedef unsigned long	gulong;
typedef unsigned int	guint;

typedef float	gfloat;
typedef double	gdouble;

/* HAVE_LONG_DOUBLE doesn't work correctly on all platforms.
 * Since gldouble isn't used anywhere, just disable it for now */

#if 0
#ifdef HAVE_LONG_DOUBLE
typedef long double gldouble;
#else /* HAVE_LONG_DOUBLE */
typedef double gldouble;
#endif /* HAVE_LONG_DOUBLE */
#endif /* 0 */

typedef void* gpointer;
typedef const void *gconstpointer;


typedef gint32	gssize;
typedef guint32 gsize;
typedef guint32 GQuark;
typedef gint32	GTime;


/* Portable endian checks and conversions
 *
 * glibconfig.h defines G_BYTE_ORDER which expands to one of
 * the below macros.
 */
#define G_LITTLE_ENDIAN 1234
#define G_BIG_ENDIAN    4321
#define G_PDP_ENDIAN    3412		/* unused, need specific PDP check */	


/* Basic bit swapping functions
 */
#define GUINT16_SWAP_LE_BE_CONSTANT(val)	((guint16) ( \
    (((guint16) (val) & (guint16) 0x00ffU) << 8) | \
    (((guint16) (val) & (guint16) 0xff00U) >> 8)))
#define GUINT32_SWAP_LE_BE_CONSTANT(val)	((guint32) ( \
    (((guint32) (val) & (guint32) 0x000000ffU) << 24) | \
    (((guint32) (val) & (guint32) 0x0000ff00U) <<  8) | \
    (((guint32) (val) & (guint32) 0x00ff0000U) >>  8) | \
    (((guint32) (val) & (guint32) 0xff000000U) >> 24)))

/* Intel specific stuff for speed
 */
#if defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#  define GUINT16_SWAP_LE_BE_X86(val) \
     (__extension__					\
      ({ register guint16 __v;				\
	 if (__builtin_constant_p (val))		\
	   __v = GUINT16_SWAP_LE_BE_CONSTANT (val);	\
	 else						\
	   __asm__ __const__ ("rorw $8, %w0"		\
			      : "=r" (__v)		\
			      : "0" ((guint16) (val)));	\
	__v; }))
#  define GUINT16_SWAP_LE_BE(val) (GUINT16_SWAP_LE_BE_X86 (val))
#  if !defined(__i486__) && !defined(__i586__) \
      && !defined(__pentium__) && !defined(__i686__) && !defined(__pentiumpro__)
#     define GUINT32_SWAP_LE_BE_X86(val) \
        (__extension__						\
         ({ register guint32 __v;				\
	    if (__builtin_constant_p (val))			\
	      __v = GUINT32_SWAP_LE_BE_CONSTANT (val);		\
	  else							\
	    __asm__ __const__ ("rorw $8, %w0\n\t"		\
			       "rorl $16, %0\n\t"		\
			       "rorw $8, %w0"			\
			       : "=r" (__v)			\
			       : "0" ((guint32) (val)));	\
	__v; }))
#  else /* 486 and higher has bswap */
#     define GUINT32_SWAP_LE_BE_X86(val) \
        (__extension__						\
         ({ register guint32 __v;				\
	    if (__builtin_constant_p (val))			\
	      __v = GUINT32_SWAP_LE_BE_CONSTANT (val);		\
	  else							\
	    __asm__ __const__ ("bswap %0"			\
			       : "=r" (__v)			\
			       : "0" ((guint32) (val)));	\
	__v; }))
#  endif /* processor specific 32-bit stuff */
#  define GUINT32_SWAP_LE_BE(val) (GUINT32_SWAP_LE_BE_X86 (val))
#else /* !__i386__ */
#  define GUINT16_SWAP_LE_BE(val) (GUINT16_SWAP_LE_BE_CONSTANT (val))
#  define GUINT32_SWAP_LE_BE(val) (GUINT32_SWAP_LE_BE_CONSTANT (val))
#endif /* __i386__ */

#ifdef G_HAVE_GINT64
#  define GUINT64_SWAP_LE_BE_CONSTANT(val)	((guint64) ( \
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x00000000000000ffU)) << 56) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x000000000000ff00U)) << 40) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x0000000000ff0000U)) << 24) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x00000000ff000000U)) <<  8) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x000000ff00000000U)) >>  8) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x0000ff0000000000U)) >> 24) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0x00ff000000000000U)) >> 40) |	\
      (((guint64) (val) &						\
	(guint64) G_GINT64_CONSTANT(0xff00000000000000U)) >> 56)))
#  if defined (__i386__) && defined (__GNUC__) && __GNUC__ >= 2
#    define GUINT64_SWAP_LE_BE_X86(val) \
	(__extension__						\
	 ({ union { guint64 __ll;				\
		    guint32 __l[2]; } __r;			\
	    if (__builtin_constant_p (val))			\
	      __r.__ll = GUINT64_SWAP_LE_BE_CONSTANT (val);	\
	    else						\
	      {							\
	 	union { guint64 __ll;				\
			guint32 __l[2]; } __w;			\
		__w.__ll = ((guint64) val);			\
		__r.__l[0] = GUINT32_SWAP_LE_BE (__w.__l[1]);	\
		__r.__l[1] = GUINT32_SWAP_LE_BE (__w.__l[0]);	\
	      }							\
	  __r.__ll; }))
#    define GUINT64_SWAP_LE_BE(val) (GUINT64_SWAP_LE_BE_X86 (val))
#  else /* !__i386__ */
#    define GUINT64_SWAP_LE_BE(val) (GUINT64_SWAP_LE_BE_CONSTANT(val))
#  endif
#endif

#define GUINT16_SWAP_LE_PDP(val)	((guint16) (val))
#define GUINT16_SWAP_BE_PDP(val)	(GUINT16_SWAP_LE_BE (val))
#define GUINT32_SWAP_LE_PDP(val)	((guint32) ( \
    (((guint32) (val) & (guint32) 0x0000ffffU) << 16) | \
    (((guint32) (val) & (guint32) 0xffff0000U) >> 16)))
#define GUINT32_SWAP_BE_PDP(val)	((guint32) ( \
    (((guint32) (val) & (guint32) 0x00ff00ffU) << 8) | \
    (((guint32) (val) & (guint32) 0xff00ff00U) >> 8)))

/* The G*_TO_?E() macros are defined in glibconfig.h.
 * The transformation is symmetric, so the FROM just maps to the TO.
 */
#define GINT16_FROM_LE(val)	(GINT16_TO_LE (val))
#define GUINT16_FROM_LE(val)	(GUINT16_TO_LE (val))
#define GINT16_FROM_BE(val)	(GINT16_TO_BE (val))
#define GUINT16_FROM_BE(val)	(GUINT16_TO_BE (val))
#define GINT32_FROM_LE(val)	(GINT32_TO_LE (val))
#define GUINT32_FROM_LE(val)	(GUINT32_TO_LE (val))
#define GINT32_FROM_BE(val)	(GINT32_TO_BE (val))
#define GUINT32_FROM_BE(val)	(GUINT32_TO_BE (val))

#ifdef G_HAVE_GINT64
#define GINT64_FROM_LE(val)	(GINT64_TO_LE (val))
#define GUINT64_FROM_LE(val)	(GUINT64_TO_LE (val))
#define GINT64_FROM_BE(val)	(GINT64_TO_BE (val))
#define GUINT64_FROM_BE(val)	(GUINT64_TO_BE (val))
#endif

#define GLONG_FROM_LE(val)	(GLONG_TO_LE (val))
#define GULONG_FROM_LE(val)	(GULONG_TO_LE (val))
#define GLONG_FROM_BE(val)	(GLONG_TO_BE (val))
#define GULONG_FROM_BE(val)	(GULONG_TO_BE (val))

#define GINT_FROM_LE(val)	(GINT_TO_LE (val))
#define GUINT_FROM_LE(val)	(GUINT_TO_LE (val))
#define GINT_FROM_BE(val)	(GINT_TO_BE (val))
#define GUINT_FROM_BE(val)	(GUINT_TO_BE (val))


/* Portable versions of host-network order stuff
 */
#define g_ntohl(val) (GUINT32_FROM_BE (val))
#define g_ntohs(val) (GUINT16_FROM_BE (val))
#define g_htonl(val) (GUINT32_TO_BE (val))
#define g_htons(val) (GUINT16_TO_BE (val))


/* Glib version.
 * we prefix variable declarations so they can
 * properly get exported in windows dlls.
 */
#ifdef NATIVE_WIN32
#  ifdef GLIB_COMPILATION
#    define GUTILS_C_VAR __declspec(dllexport)
#  else /* !GLIB_COMPILATION */
#    define GUTILS_C_VAR extern __declspec(dllimport)
#  endif /* !GLIB_COMPILATION */
#else /* !NATIVE_WIN32 */
#  define GUTILS_C_VAR extern
#endif /* !NATIVE_WIN32 */

GUTILS_C_VAR const guint glib_major_version;
GUTILS_C_VAR const guint glib_minor_version;
GUTILS_C_VAR const guint glib_micro_version;
GUTILS_C_VAR const guint glib_interface_age;
GUTILS_C_VAR const guint glib_binary_age;

#define GLIB_CHECK_VERSION(major,minor,micro)    \
    (GLIB_MAJOR_VERSION > (major) || \
     (GLIB_MAJOR_VERSION == (major) && GLIB_MINOR_VERSION > (minor)) || \
     (GLIB_MAJOR_VERSION == (major) && GLIB_MINOR_VERSION == (minor) && \
      GLIB_MICRO_VERSION >= (micro)))

/* Forward declarations of glib types.
 */
typedef struct _GAllocator	GAllocator;
typedef struct _GArray		GArray;
typedef struct _GByteArray	GByteArray;
typedef struct _GCache		GCache;
typedef struct _GCompletion	GCompletion;
typedef	struct _GData		GData;
typedef struct _GDebugKey	GDebugKey;
typedef struct _GHashTable	GHashTable;
typedef struct _GHook		GHook;
typedef struct _GHookList	GHookList;
typedef struct _GList		GList;
typedef struct _GMemChunk	GMemChunk;
typedef struct _GNode		GNode;
typedef struct _GPtrArray	GPtrArray;
typedef struct _GRelation	GRelation;
typedef struct _GScanner	GScanner;
typedef struct _GScannerConfig	GScannerConfig;
typedef struct _GSList		GSList;
typedef struct _GString		GString;
typedef struct _GStringChunk	GStringChunk;
typedef struct _GTimer		GTimer;
typedef struct _GTree		GTree;
typedef struct _GTuples		GTuples;
typedef union  _GTokenValue	GTokenValue;
typedef struct _GIOChannel	GIOChannel;

typedef enum
{
  G_TRAVERSE_LEAFS	= 1 << 0,
  G_TRAVERSE_NON_LEAFS	= 1 << 1,
  G_TRAVERSE_ALL	= G_TRAVERSE_LEAFS | G_TRAVERSE_NON_LEAFS,
  G_TRAVERSE_MASK	= 0x03
} GTraverseFlags;

typedef enum
{
  G_IN_ORDER,
  G_PRE_ORDER,
  G_POST_ORDER,
  G_LEVEL_ORDER
} GTraverseType;

/* Log level shift offset for user defined
 * log levels (0-7 are used by GLib).
 */
#define	G_LOG_LEVEL_USER_SHIFT	(8)

/* Glib log levels and flags.
 */
typedef enum
{
  /* log flags */
  G_LOG_FLAG_RECURSION		= 1 << 0,
  G_LOG_FLAG_FATAL		= 1 << 1,
  
  /* GLib log levels */
  G_LOG_LEVEL_ERROR		= 1 << 2,	/* always fatal */
  G_LOG_LEVEL_CRITICAL		= 1 << 3,
  G_LOG_LEVEL_WARNING		= 1 << 4,
  G_LOG_LEVEL_MESSAGE		= 1 << 5,
  G_LOG_LEVEL_INFO		= 1 << 6,
  G_LOG_LEVEL_DEBUG		= 1 << 7,
  
  G_LOG_LEVEL_MASK		= ~(G_LOG_FLAG_RECURSION | G_LOG_FLAG_FATAL)
} GLogLevelFlags;

/* GLib log levels that are considered fatal by default */
#define	G_LOG_FATAL_MASK	(G_LOG_FLAG_RECURSION | G_LOG_LEVEL_ERROR)


typedef gpointer	(*GCacheNewFunc)	(gpointer	key);
typedef gpointer	(*GCacheDupFunc)	(gpointer	value);
typedef void		(*GCacheDestroyFunc)	(gpointer	value);
typedef gint		(*GCompareFunc)		(gconstpointer	a,
						 gconstpointer	b);
typedef gchar*		(*GCompletionFunc)	(gpointer);
typedef void		(*GDestroyNotify)	(gpointer	data);
typedef void		(*GDataForeachFunc)	(GQuark		key_id,
						 gpointer	data,
						 gpointer	user_data);
typedef void		(*GFunc)		(gpointer	data,
						 gpointer	user_data);
typedef guint		(*GHashFunc)		(gconstpointer	key);
typedef void		(*GFreeFunc)		(gpointer	data);
typedef void		(*GHFunc)		(gpointer	key,
						 gpointer	value,
						 gpointer	user_data);
typedef gboolean	(*GHRFunc)		(gpointer	key,
						 gpointer	value,
						 gpointer	user_data);
typedef gint		(*GHookCompareFunc)	(GHook		*new_hook,
						 GHook		*sibling);
typedef gboolean	(*GHookFindFunc)	(GHook		*hook,
						 gpointer	 data);
typedef void		(*GHookMarshaller)	(GHook		*hook,
						 gpointer	 data);
typedef gboolean	(*GHookCheckMarshaller)	(GHook		*hook,
						 gpointer	 data);
typedef void		(*GHookFunc)		(gpointer	 data);
typedef gboolean	(*GHookCheckFunc)	(gpointer	 data);
typedef void		(*GHookFreeFunc)	(GHookList      *hook_list,
						 GHook          *hook);
typedef void		(*GLogFunc)		(const gchar   *log_domain,
						 GLogLevelFlags	log_level,
						 const gchar   *message,
						 gpointer	user_data);
typedef gboolean	(*GNodeTraverseFunc)	(GNode	       *node,
						 gpointer	data);
typedef void		(*GNodeForeachFunc)	(GNode	       *node,
						 gpointer	data);
typedef gint		(*GSearchFunc)		(gpointer	key,
						 gpointer	data);
typedef void		(*GScannerMsgFunc)	(GScanner      *scanner,
						 gchar	       *message,
						 gint		error);
typedef gint		(*GTraverseFunc)	(gpointer	key,
						 gpointer	value,
						 gpointer	data);
typedef	void		(*GVoidFunc)		(void);


struct _GList
{
  gpointer data;
  GList *next;
  GList *prev;
};

struct _GSList
{
  gpointer data;
  GSList *next;
};

struct _GString
{
  gchar *str;
  gint len;
};

struct _GArray
{
  gchar *data;
  guint len;
};

struct _GByteArray
{
  guint8 *data;
  guint	  len;
};

struct _GPtrArray
{
  gpointer *pdata;
  guint	    len;
};

struct _GTuples
{
  guint len;
};

struct _GDebugKey
{
  gchar *key;
  guint	 value;
};


/* Doubly linked lists
 */
void   g_list_push_allocator    (GAllocator     *allocator);
void   g_list_pop_allocator     (void);
GList* g_list_alloc		(void);
void   g_list_free		(GList		*list);
void   g_list_free_1		(GList		*list);
GList* g_list_append		(GList		*list,
				 gpointer	 data);
GList* g_list_prepend		(GList		*list,
				 gpointer	 data);
GList* g_list_insert		(GList		*list,
				 gpointer	 data,
				 gint		 position);
GList* g_list_insert_sorted	(GList		*list,
				 gpointer	 data,
				 GCompareFunc	 func);
GList* g_list_concat		(GList		*list1,
				 GList		*list2);
GList* g_list_remove		(GList		*list,
				 gpointer	 data);
GList* g_list_remove_link	(GList		*list,
				 GList		*llink);
GList* g_list_reverse		(GList		*list);
GList* g_list_copy		(GList		*list);
GList* g_list_nth		(GList		*list,
				 guint		 n);
GList* g_list_find		(GList		*list,
				 gpointer	 data);
GList* g_list_find_custom	(GList		*list,
				 gpointer	 data,
				 GCompareFunc	 func);
gint   g_list_position		(GList		*list,
				 GList		*llink);
gint   g_list_index		(GList		*list,
				 gpointer	 data);
GList* g_list_last		(GList		*list);
GList* g_list_first		(GList		*list);
guint  g_list_length		(GList		*list);
void   g_list_foreach		(GList		*list,
				 GFunc		 func,
				 gpointer	 user_data);
GList* g_list_sort              (GList          *list,
		                 GCompareFunc    compare_func);
gpointer g_list_nth_data	(GList		*list,
				 guint		 n);
#define g_list_previous(list)	((list) ? (((GList *)(list))->prev) : NULL)
#define g_list_next(list)	((list) ? (((GList *)(list))->next) : NULL)


/* Singly linked lists
 */
void    g_slist_push_allocator  (GAllocator     *allocator);
void    g_slist_pop_allocator   (void);
GSList* g_slist_alloc		(void);
void	g_slist_free		(GSList		*list);
void	g_slist_free_1		(GSList		*list);
GSList* g_slist_append		(GSList		*list,
				 gpointer	 data);
GSList* g_slist_prepend		(GSList		*list,
				 gpointer	 data);
GSList* g_slist_insert		(GSList		*list,
				 gpointer	 data,
				 gint		 position);
GSList* g_slist_insert_sorted	(GSList		*list,
				 gpointer	 data,
				 GCompareFunc	 func);
GSList* g_slist_concat		(GSList		*list1,
				 GSList		*list2);
GSList* g_slist_remove		(GSList		*list,
				 gpointer	 data);
GSList* g_slist_remove_link	(GSList		*list,
				 GSList		*llink);
GSList* g_slist_reverse		(GSList		*list);
GSList*	g_slist_copy		(GSList		*list);
GSList* g_slist_nth		(GSList		*list,
				 guint		 n);
GSList* g_slist_find		(GSList		*list,
				 gpointer	 data);
GSList* g_slist_find_custom	(GSList		*list,
				 gpointer	 data,
				 GCompareFunc	 func);
gint	g_slist_position	(GSList		*list,
				 GSList		*llink);
gint	g_slist_index		(GSList		*list,
				 gpointer	 data);
GSList* g_slist_last		(GSList		*list);
guint	g_slist_length		(GSList		*list);
void	g_slist_foreach		(GSList		*list,
				 GFunc		 func,
				 gpointer	 user_data);
GSList*  g_slist_sort           (GSList          *list,
		                 GCompareFunc    compare_func);
gpointer g_slist_nth_data	(GSList		*list,
				 guint		 n);
#define g_slist_next(slist)	((slist) ? (((GSList *)(slist))->next) : NULL)


/* Hash tables
 */
GHashTable* g_hash_table_new		(GHashFunc	 hash_func,
					 GCompareFunc	 key_compare_func);
void	    g_hash_table_destroy	(GHashTable	*hash_table);
void	    g_hash_table_insert		(GHashTable	*hash_table,
					 gpointer	 key,
					 gpointer	 value);
void	    g_hash_table_remove		(GHashTable	*hash_table,
					 gconstpointer	 key);
gpointer    g_hash_table_lookup		(GHashTable	*hash_table,
					 gconstpointer	 key);
gboolean    g_hash_table_lookup_extended(GHashTable	*hash_table,
					 gconstpointer	 lookup_key,
					 gpointer	*orig_key,
					 gpointer	*value);
void	    g_hash_table_freeze		(GHashTable	*hash_table);
void	    g_hash_table_thaw		(GHashTable	*hash_table);
void	    g_hash_table_foreach	(GHashTable	*hash_table,
					 GHFunc		 func,
					 gpointer	 user_data);
guint	    g_hash_table_foreach_remove	(GHashTable	*hash_table,
					 GHRFunc	 func,
					 gpointer	 user_data);
guint	    g_hash_table_size		(GHashTable	*hash_table);


/* Caches
 */
GCache*	 g_cache_new	       (GCacheNewFunc	   value_new_func,
				GCacheDestroyFunc  value_destroy_func,
				GCacheDupFunc	   key_dup_func,
				GCacheDestroyFunc  key_destroy_func,
				GHashFunc	   hash_key_func,
				GHashFunc	   hash_value_func,
				GCompareFunc	   key_compare_func);
void	 g_cache_destroy       (GCache		  *cache);
gpointer g_cache_insert	       (GCache		  *cache,
				gpointer	   key);
void	 g_cache_remove	       (GCache		  *cache,
				gpointer	   value);
void	 g_cache_key_foreach   (GCache		  *cache,
				GHFunc		   func,
				gpointer	   user_data);
void	 g_cache_value_foreach (GCache		  *cache,
				GHFunc		   func,
				gpointer	   user_data);


/* Balanced binary trees
 */
GTree*	 g_tree_new	 (GCompareFunc	 key_compare_func);
void	 g_tree_destroy	 (GTree		*tree);
void	 g_tree_insert	 (GTree		*tree,
			  gpointer	 key,
			  gpointer	 value);
void	 g_tree_remove	 (GTree		*tree,
			  gpointer	 key);
gpointer g_tree_lookup	 (GTree		*tree,
			  gpointer	 key);
void	 g_tree_traverse (GTree		*tree,
			  GTraverseFunc	 traverse_func,
			  GTraverseType	 traverse_type,
			  gpointer	 data);
gpointer g_tree_search	 (GTree		*tree,
			  GSearchFunc	 search_func,
			  gpointer	 data);
gint	 g_tree_height	 (GTree		*tree);
gint	 g_tree_nnodes	 (GTree		*tree);



/* N-way tree implementation
 */
struct _GNode
{
  gpointer data;
  GNode	  *next;
  GNode	  *prev;
  GNode	  *parent;
  GNode	  *children;
};

#define	 G_NODE_IS_ROOT(node)	(((GNode*) (node))->parent == NULL && \
				 ((GNode*) (node))->prev == NULL && \
				 ((GNode*) (node))->next == NULL)
#define	 G_NODE_IS_LEAF(node)	(((GNode*) (node))->children == NULL)

void     g_node_push_allocator  (GAllocator       *allocator);
void     g_node_pop_allocator   (void);
GNode*	 g_node_new		(gpointer	   data);
void	 g_node_destroy		(GNode		  *root);
void	 g_node_unlink		(GNode		  *node);
GNode*	 g_node_insert		(GNode		  *parent,
				 gint		   position,
				 GNode		  *node);
GNode*	 g_node_insert_before	(GNode		  *parent,
				 GNode		  *sibling,
				 GNode		  *node);
GNode*	 g_node_prepend		(GNode		  *parent,
				 GNode		  *node);
guint	 g_node_n_nodes		(GNode		  *root,
				 GTraverseFlags	   flags);
GNode*	 g_node_get_root	(GNode		  *node);
gboolean g_node_is_ancestor	(GNode		  *node,
				 GNode		  *descendant);
guint	 g_node_depth		(GNode		  *node);
GNode*	 g_node_find		(GNode		  *root,
				 GTraverseType	   order,
				 GTraverseFlags	   flags,
				 gpointer	   data);

/* convenience macros */
#define g_node_append(parent, node)				\
     g_node_insert_before ((parent), NULL, (node))
#define	g_node_insert_data(parent, position, data)		\
     g_node_insert ((parent), (position), g_node_new (data))
#define	g_node_insert_data_before(parent, sibling, data)	\
     g_node_insert_before ((parent), (sibling), g_node_new (data))
#define	g_node_prepend_data(parent, data)			\
     g_node_prepend ((parent), g_node_new (data))
#define	g_node_append_data(parent, data)			\
     g_node_insert_before ((parent), NULL, g_node_new (data))

/* traversal function, assumes that `node' is root
 * (only traverses `node' and its subtree).
 * this function is just a high level interface to
 * low level traversal functions, optimized for speed.
 */
void	 g_node_traverse	(GNode		  *root,
				 GTraverseType	   order,
				 GTraverseFlags	   flags,
				 gint		   max_depth,
				 GNodeTraverseFunc func,
				 gpointer	   data);

/* return the maximum tree height starting with `node', this is an expensive
 * operation, since we need to visit all nodes. this could be shortened by
 * adding `guint height' to struct _GNode, but then again, this is not very
 * often needed, and would make g_node_insert() more time consuming.
 */
guint	 g_node_max_height	 (GNode *root);

void	 g_node_children_foreach (GNode		  *node,
				  GTraverseFlags   flags,
				  GNodeForeachFunc func,
				  gpointer	   data);
void	 g_node_reverse_children (GNode		  *node);
guint	 g_node_n_children	 (GNode		  *node);
GNode*	 g_node_nth_child	 (GNode		  *node,
				  guint		   n);
GNode*	 g_node_last_child	 (GNode		  *node);
GNode*	 g_node_find_child	 (GNode		  *node,
				  GTraverseFlags   flags,
				  gpointer	   data);
gint	 g_node_child_position	 (GNode		  *node,
				  GNode		  *child);
gint	 g_node_child_index	 (GNode		  *node,
				  gpointer	   data);

GNode*	 g_node_first_sibling	 (GNode		  *node);
GNode*	 g_node_last_sibling	 (GNode		  *node);

#define	 g_node_prev_sibling(node)	((node) ? \
					 ((GNode*) (node))->prev : NULL)
#define	 g_node_next_sibling(node)	((node) ? \
					 ((GNode*) (node))->next : NULL)
#define	 g_node_first_child(node)	((node) ? \
					 ((GNode*) (node))->children : NULL)


/* Callback maintenance functions
 */
#define G_HOOK_FLAG_USER_SHIFT	(4)
typedef enum
{
  G_HOOK_FLAG_ACTIVE	= 1 << 0,
  G_HOOK_FLAG_IN_CALL	= 1 << 1,
  G_HOOK_FLAG_MASK	= 0x0f
} GHookFlagMask;

#define	G_HOOK_DEFERRED_DESTROY	((GHookFreeFunc) 0x01)

struct _GHookList
{
  guint		 seq_id;
  guint		 hook_size;
  guint		 is_setup : 1;
  GHook		*hooks;
  GMemChunk	*hook_memchunk;
  GHookFreeFunc	 hook_free; /* virtual function */
  GHookFreeFunc	 hook_destroy; /* virtual function */
};

struct _GHook
{
  gpointer	 data;
  GHook		*next;
  GHook		*prev;
  guint		 ref_count;
  guint		 hook_id;
  guint		 flags;
  gpointer	 func;
  GDestroyNotify destroy;
};

#define	G_HOOK_ACTIVE(hook)		((((GHook*) hook)->flags & \
					  G_HOOK_FLAG_ACTIVE) != 0)
#define	G_HOOK_IN_CALL(hook)		((((GHook*) hook)->flags & \
					  G_HOOK_FLAG_IN_CALL) != 0)
#define G_HOOK_IS_VALID(hook)		(((GHook*) hook)->hook_id != 0 && \
					 G_HOOK_ACTIVE (hook))
#define G_HOOK_IS_UNLINKED(hook)	(((GHook*) hook)->next == NULL && \
					 ((GHook*) hook)->prev == NULL && \
					 ((GHook*) hook)->hook_id == 0 && \
					 ((GHook*) hook)->ref_count == 0)

void	 g_hook_list_init		(GHookList		*hook_list,
					 guint			 hook_size);
void	 g_hook_list_clear		(GHookList		*hook_list);
GHook*	 g_hook_alloc			(GHookList		*hook_list);
void	 g_hook_free			(GHookList		*hook_list,
					 GHook			*hook);
void	 g_hook_ref			(GHookList		*hook_list,
					 GHook			*hook);
void	 g_hook_unref			(GHookList		*hook_list,
					 GHook			*hook);
gboolean g_hook_destroy			(GHookList		*hook_list,
					 guint			 hook_id);
void	 g_hook_destroy_link		(GHookList		*hook_list,
					 GHook			*hook);
void	 g_hook_prepend			(GHookList		*hook_list,
					 GHook			*hook);
void	 g_hook_insert_before		(GHookList		*hook_list,
					 GHook			*sibling,
					 GHook			*hook);
void	 g_hook_insert_sorted		(GHookList		*hook_list,
					 GHook			*hook,
					 GHookCompareFunc	 func);
GHook*	 g_hook_get			(GHookList		*hook_list,
					 guint			 hook_id);
GHook*	 g_hook_find			(GHookList		*hook_list,
					 gboolean		 need_valids,
					 GHookFindFunc		 func,
					 gpointer		 data);
GHook*	 g_hook_find_data		(GHookList		*hook_list,
					 gboolean		 need_valids,
					 gpointer		 data);
GHook*	 g_hook_find_func		(GHookList		*hook_list,
					 gboolean		 need_valids,
					 gpointer		 func);
GHook*	 g_hook_find_func_data		(GHookList		*hook_list,
					 gboolean		 need_valids,
					 gpointer		 func,
					 gpointer		 data);
/* return the first valid hook, and increment its reference count */
GHook*	 g_hook_first_valid		(GHookList		*hook_list,
					 gboolean		 may_be_in_call);
/* return the next valid hook with incremented reference count, and
 * decrement the reference count of the original hook
 */
GHook*	 g_hook_next_valid		(GHookList		*hook_list,
					 GHook			*hook,
					 gboolean		 may_be_in_call);

/* GHookCompareFunc implementation to insert hooks sorted by their id */
gint	 g_hook_compare_ids		(GHook			*new_hook,
					 GHook			*sibling);

/* convenience macros */
#define	 g_hook_append( hook_list, hook )  \
     g_hook_insert_before ((hook_list), NULL, (hook))

/* invoke all valid hooks with the (*GHookFunc) signature.
 */
void	 g_hook_list_invoke		(GHookList		*hook_list,
					 gboolean		 may_recurse);
/* invoke all valid hooks with the (*GHookCheckFunc) signature,
 * and destroy the hook if FALSE is returned.
 */
void	 g_hook_list_invoke_check	(GHookList		*hook_list,
					 gboolean		 may_recurse);
/* invoke a marshaller on all valid hooks.
 */
void	 g_hook_list_marshal		(GHookList		*hook_list,
					 gboolean		 may_recurse,
					 GHookMarshaller	 marshaller,
					 gpointer		 data);
void	 g_hook_list_marshal_check	(GHookList		*hook_list,
					 gboolean		 may_recurse,
					 GHookCheckMarshaller	 marshaller,
					 gpointer		 data);


/* Fatal error handlers.
 * g_on_error_query() will prompt the user to either
 * [E]xit, [H]alt, [P]roceed or show [S]tack trace.
 * g_on_error_stack_trace() invokes gdb, which attaches to the current
 * process and shows a stack trace.
 * These function may cause different actions on non-unix platforms.
 * The prg_name arg is required by gdb to find the executable, if it is
 * passed as NULL, g_on_error_query() will try g_get_prgname().
 */
void g_on_error_query (const gchar *prg_name);
void g_on_error_stack_trace (const gchar *prg_name);


/* Logging mechanism
 */
extern	        const gchar		*g_log_domain_glib;
guint		g_log_set_handler	(const gchar	*log_domain,
					 GLogLevelFlags	 log_levels,
					 GLogFunc	 log_func,
					 gpointer	 user_data);
void		g_log_remove_handler	(const gchar	*log_domain,
					 guint		 handler_id);
void		g_log_default_handler	(const gchar	*log_domain,
					 GLogLevelFlags	 log_level,
					 const gchar	*message,
					 gpointer	 unused_data);
void		g_log			(const gchar	*log_domain,
					 GLogLevelFlags	 log_level,
					 const gchar	*format,
					 ...) G_GNUC_PRINTF (3, 4);
void		g_logv			(const gchar	*log_domain,
					 GLogLevelFlags	 log_level,
					 const gchar	*format,
					 va_list	 args);
GLogLevelFlags	g_log_set_fatal_mask	(const gchar	*log_domain,
					 GLogLevelFlags	 fatal_mask);
GLogLevelFlags	g_log_set_always_fatal	(GLogLevelFlags	 fatal_mask);
#ifndef	G_LOG_DOMAIN
#define	G_LOG_DOMAIN	((gchar*) 0)
#endif	/* G_LOG_DOMAIN */

#define	g_error(format, args...)	g_log (G_LOG_DOMAIN, \
					       G_LOG_LEVEL_ERROR, \
					       format, ##args)
#define	g_message(format, args...)	g_log (G_LOG_DOMAIN, \
					       G_LOG_LEVEL_MESSAGE, \
					       format, ##args)
#define	g_warning printf



/* Memory allocation and debugging
 */
#define g_malloc(size)	     ((gpointer)malloc(size))
#define g_malloc0(size)	     ((gpointer)calloc(size, 1))
#define g_realloc(mem,size)  ((gpointer)realloc(mem, size))
#define g_free(mem)	     free(mem)


void	 g_mem_profile (void);
void	 g_mem_check   (gpointer  mem);


/* String utility functions that modify a string argument or
 * return a constant string that must not be freed.
 */
#define	 G_STR_DELIMITERS	"_-|> <."
gchar*	 g_strdelimit		(gchar	     *string,
				 const gchar *delimiters,
				 gchar	      new_delimiter);
gdouble	 g_strtod		(const gchar *nptr,
				 gchar	    **endptr);
gchar*	 g_strerror		(gint	      errnum);
gchar*	 g_strsignal		(gint	      signum);
gint	 g_strcasecmp		(const gchar *s1,
				 const gchar *s2);
gint	 g_strncasecmp		(const gchar *s1,
				 const gchar *s2,
				 guint 	      n);
void	 g_strdown		(gchar	     *string);
void	 g_strup		(gchar	     *string);
void	 g_strreverse		(gchar	     *string);
/* removes leading spaces */
gchar*   g_strchug              (gchar        *string);
/* removes trailing spaces */
gchar*  g_strchomp              (gchar        *string);
/* removes leading & trailing spaces */
#define g_strstrip( string )	g_strchomp (g_strchug (string))

/* String utility functions that return a newly allocated string which
 * ought to be freed from the caller at some point.
 */
gchar*	 g_strdup		(const gchar *str);
gchar*	 g_strdup_printf	(const gchar *format,
				 ...) G_GNUC_PRINTF (1, 2);
gchar*	 g_strdup_vprintf	(const gchar *format,
				 va_list      args);
gchar*	 g_strndup		(const gchar *str,
				 guint	      n);
gchar*	 g_strnfill		(guint	      length,
				 gchar	      fill_char);
gchar*	 g_strconcat		(const gchar *string1,
				 ...); /* NULL terminated */
gchar*   g_strjoin		(const gchar  *separator,
				 ...); /* NULL terminated */
gchar*	 g_strescape		(gchar	      *string);
gpointer g_memdup		(gconstpointer mem,
				 guint	       byte_size);

/* NULL terminated string arrays.
 * g_strsplit() splits up string into max_tokens tokens at delim and
 * returns a newly allocated string array.
 * g_strjoinv() concatenates all of str_array's strings, sliding in an
 * optional separator, the returned string is newly allocated.
 * g_strfreev() frees the array itself and all of its strings.
 */
gchar**	 g_strsplit		(const gchar  *string,
				 const gchar  *delimiter,
				 gint          max_tokens);
gchar*   g_strjoinv		(const gchar  *separator,
				 gchar       **str_array);
void     g_strfreev		(gchar       **str_array);



/* calculate a string size, guarranteed to fit format + args.
 */
guint	g_printf_string_upper_bound (const gchar* format,
				     va_list	  args);


/* Retrive static string info
 */
gchar*	g_get_user_name		(void);
gchar*	g_get_real_name		(void);
gchar*	g_get_home_dir		(void);
gchar*	g_get_tmp_dir		(void);
gchar*	g_get_prgname		(void);
void	g_set_prgname		(const gchar *prgname);


/* Miscellaneous utility functions
 */
guint	g_parse_debug_string	(const gchar *string,
				 GDebugKey   *keys,
				 guint	      nkeys);
gint	g_snprintf		(gchar	     *string,
				 gulong	      n,
				 gchar const *format,
				 ...) G_GNUC_PRINTF (3, 4);
gint	g_vsnprintf		(gchar	     *string,
				 gulong	      n,
				 gchar const *format,
				 va_list      args);
gchar*	g_basename		(const gchar *file_name);
/* Check if a file name is an absolute path */
gboolean g_path_is_absolute	(const gchar *file_name);
/* In case of absolute paths, skip the root part */
gchar*  g_path_skip_root	(gchar       *file_name);

/* strings are newly allocated with g_malloc() */
gchar*	g_dirname		(const gchar *file_name);
gchar*	g_get_current_dir	(void);
gchar*  g_getenv		(const gchar *variable);


/* we use a GLib function as a replacement for ATEXIT, so
 * the programmer is not required to check the return value
 * (if there is any in the implementation) and doesn't encounter
 * missing include files.
 */
void	g_atexit		(GVoidFunc    func);


/* Bit tests
 */
G_INLINE_FUNC gint	g_bit_nth_lsf (guint32 mask,
				       gint    nth_bit);
#ifdef	G_CAN_INLINE
G_INLINE_FUNC gint
g_bit_nth_lsf (guint32 mask,
	       gint    nth_bit)
{
  do
    {
      nth_bit++;
      if (mask & (1 << (guint) nth_bit))
	return nth_bit;
    }
  while (nth_bit < 32);
  return -1;
}
#endif	/* G_CAN_INLINE */

G_INLINE_FUNC gint	g_bit_nth_msf (guint32 mask,
				       gint    nth_bit);
#ifdef G_CAN_INLINE
G_INLINE_FUNC gint
g_bit_nth_msf (guint32 mask,
	       gint    nth_bit)
{
  if (nth_bit < 0)
    nth_bit = 32;
  do
    {
      nth_bit--;
      if (mask & (1 << (guint) nth_bit))
	return nth_bit;
    }
  while (nth_bit > 0);
  return -1;
}
#endif	/* G_CAN_INLINE */

G_INLINE_FUNC guint	g_bit_storage (guint number);
#ifdef G_CAN_INLINE
G_INLINE_FUNC guint
g_bit_storage (guint number)
{
  register guint n_bits = 0;
  
  do
    {
      n_bits++;
      number >>= 1;
    }
  while (number);
  return n_bits;
}
#endif	/* G_CAN_INLINE */

/* String Chunks
 */
GStringChunk* g_string_chunk_new	   (gint size);
void	      g_string_chunk_free	   (GStringChunk *chunk);
gchar*	      g_string_chunk_insert	   (GStringChunk *chunk,
					    const gchar	 *string);
gchar*	      g_string_chunk_insert_const  (GStringChunk *chunk,
					    const gchar	 *string);


/* Strings
 */
GString* g_string_new	    (const gchar *init);
GString* g_string_sized_new (guint	  dfl_size);
void	 g_string_free	    (GString	 *string,
			     gint	  free_segment);
GString* g_string_assign    (GString	 *lval,
			     const gchar *rval);
GString* g_string_truncate  (GString	 *string,
			     gint	  len);
GString* g_string_append    (GString	 *string,
			     const gchar *val);
GString* g_string_append_c  (GString	 *string,
			     gchar	  c);
GString* g_string_prepend   (GString	 *string,
			     const gchar *val);
GString* g_string_prepend_c (GString	 *string,
			     gchar	  c);
GString* g_string_insert    (GString	 *string,
			     gint	  pos,
			     const gchar *val);
GString* g_string_insert_c  (GString	 *string,
			     gint	  pos,
			     gchar	  c);
GString* g_string_erase	    (GString	 *string,
			     gint	  pos,
			     gint	  len);
GString* g_string_down	    (GString	 *string);
GString* g_string_up	    (GString	 *string);
void	 g_string_sprintf   (GString	 *string,
			     const gchar *format,
			     ...) G_GNUC_PRINTF (2, 3);
void	 g_string_sprintfa  (GString	 *string,
			     const gchar *format,
			     ...) G_GNUC_PRINTF (2, 3);


/* Resizable arrays, remove fills any cleared spot and shortens the
 * array, while preserving the order. remove_fast will distort the
 * order by moving the last element to the position of the removed 
 */

#define g_array_append_val(a,v)	  pmidi_array_append_vals (a, &v, 1)
#define g_array_prepend_val(a,v)  g_array_prepend_vals (a, &v, 1)
#define g_array_insert_val(a,i,v) g_array_insert_vals (a, i, &v, 1)
#define g_array_index(a,t,i)      (((t*) (a)->data) [(i)])

GArray* pmidi_array_new	          (gboolean	    zero_terminated,
				   gboolean	    clear,
				   guint	    element_size);
void	pmidi_array_free	          (GArray	   *array,
				   gboolean	    free_segment);
GArray* pmidi_array_append_vals       (GArray	   *array,
				   gconstpointer    data,
				   guint	    len);
GArray* g_array_prepend_vals      (GArray	   *array,
				   gconstpointer    data,
				   guint	    len);
GArray* g_array_insert_vals       (GArray          *array,
				   guint            index,
				   gconstpointer    data,
				   guint            len);
GArray* g_array_set_size          (GArray	   *array,
				   guint	    length);
GArray* g_array_remove_index	  (GArray	   *array,
				   guint	    index);
GArray* g_array_remove_index_fast (GArray	   *array,
				   guint	    index);

/* Resizable pointer array.  This interface is much less complicated
 * than the above.  Add appends appends a pointer.  Remove fills any
 * cleared spot and shortens the array. remove_fast will again distort
 * order.  
 */
#define	    g_ptr_array_index(array,index) (array->pdata)[index]
GPtrArray*  pmidi_ptr_array_new		   (void);
void	    pmidi_ptr_array_free		   (GPtrArray	*array,
					    gboolean	 free_seg);
void	    g_ptr_array_set_size	   (GPtrArray	*array,
					    gint	 length);
gpointer    g_ptr_array_remove_index	   (GPtrArray	*array,
					    guint	 index);
gpointer    pmidi_ptr_array_remove_index_fast  (GPtrArray	*array,
					    guint	 index);
gboolean    g_ptr_array_remove		   (GPtrArray	*array,
					    gpointer	 data);
gboolean    g_ptr_array_remove_fast        (GPtrArray	*array,
					    gpointer	 data);
void	    pmidi_ptr_array_add		   (GPtrArray	*array,
					    gpointer	 data);

/* Byte arrays, an array of guint8.  Implemented as a GArray,
 * but type-safe.
 */

GByteArray* g_byte_array_new	           (void);
void	    g_byte_array_free	           (GByteArray	 *array,
					    gboolean	  free_segment);
GByteArray* g_byte_array_append	           (GByteArray	 *array,
					    const guint8 *data,
					    guint	  len);
GByteArray* g_byte_array_prepend           (GByteArray	 *array,
					    const guint8 *data,
					    guint	  len);
GByteArray* g_byte_array_set_size          (GByteArray	 *array,
					    guint	  length);
GByteArray* g_byte_array_remove_index	   (GByteArray	 *array,
					    guint	  index);
GByteArray* g_byte_array_remove_index_fast (GByteArray	 *array,
					    guint	  index);

#endif /* __G_LIB_H__ */
