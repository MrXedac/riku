#ifndef __STDINT__
#define __STDINT__

/* Mostly taken and adapted from Linux' /usr/include/stdint.h
 * All props to GNU LibC for this */

typedef signed char		int8_t;
typedef short int		  int16_t;
typedef int			      int32_t;
typedef long int		  int64_t;

/* Unsigned.  */
typedef unsigned char		       uint8_t;
typedef unsigned short int	   uint16_t;
typedef unsigned int		       uint32_t;
typedef unsigned long int	     uint64_t;


typedef signed char		 int_least8_t;
typedef short int		   int_least16_t;
typedef int			       int_least32_t;
typedef long int		   int_least64_t;

typedef unsigned char		    uint_least8_t;
typedef unsigned short int	uint_least16_t;
typedef unsigned int		    uint_least32_t;
typedef unsigned long int	  uint_least64_t;


typedef signed char		int_fast8_t;
typedef long int		  int_fast16_t;
typedef long int		  int_fast32_t;
typedef long int		  int_fast64_t;

typedef unsigned char		  uint_fast8_t;
typedef unsigned long int	uint_fast16_t;
typedef unsigned long int	uint_fast32_t;
typedef unsigned long int	uint_fast64_t;

typedef long int		      intptr_t;
typedef unsigned long int	uintptr_t;

typedef long int		      intmax_t;
typedef unsigned long int	uintmax_t;

# define INT8_MIN		(-128)
# define INT16_MIN		(-32767-1)
# define INT32_MIN		(-2147483647-1)
# define INT64_MIN		(-(9223372036854775807)-1)

# define INT8_MAX		(127)
# define INT16_MAX		(32767)
# define INT32_MAX		(2147483647)
# define INT64_MAX		((9223372036854775807))

# define UINT8_MAX		(255)
# define UINT16_MAX		(65535)
# define UINT32_MAX		(4294967295U)
# define UINT64_MAX		((18446744073709551615))


# define INT_LEAST8_MIN		(-128)
# define INT_LEAST16_MIN	(-32767-1)
# define INT_LEAST32_MIN	(-2147483647-1)
# define INT_LEAST64_MIN	(-(9223372036854775807)-1)

# define INT_LEAST8_MAX		(127)
# define INT_LEAST16_MAX	(32767)
# define INT_LEAST32_MAX	(2147483647)
# define INT_LEAST64_MAX	((9223372036854775807))

# define UINT_LEAST8_MAX	(255)
# define UINT_LEAST16_MAX	(65535)
# define UINT_LEAST32_MAX	(4294967295U)
# define UINT_LEAST64_MAX	((18446744073709551615))


# define INT_FAST8_MIN		(-128)
#  define INT_FAST16_MIN	(-9223372036854775807L-1)
#  define INT_FAST32_MIN	(-9223372036854775807L-1)
# define INT_FAST64_MIN		(-(9223372036854775807)-1)

# define INT_FAST8_MAX		(127)
#  define INT_FAST16_MAX	(9223372036854775807L)
#  define INT_FAST32_MAX	(9223372036854775807L)
# define INT_FAST64_MAX		((9223372036854775807))

/* Maximum of fast unsigned integral types having a minimum size.  */
# define UINT_FAST8_MAX		(255)
#  define UINT_FAST16_MAX	(18446744073709551615UL)
#  define UINT_FAST32_MAX	(18446744073709551615UL)
# define UINT_FAST64_MAX	((18446744073709551615))


/* Values to test for integral types holding `void *' pointer.  */
#  define INTPTR_MIN		(-9223372036854775807L-1)
#  define INTPTR_MAX		(9223372036854775807L)
#  define UINTPTR_MAX		(18446744073709551615UL)

/* Minimum for largest signed integral type.  */
# define INTMAX_MIN		(-(9223372036854775807)-1)
/* Maximum for largest signed integral type.  */
# define INTMAX_MAX		((9223372036854775807))

/* Maximum for largest unsigned integral type.  */
# define UINTMAX_MAX		((18446744073709551615))


/* Limits of other integer types.  */

/* Limits of `ptrdiff_t' type.  */
#  define PTRDIFF_MIN		(-9223372036854775807L-1)
#  define PTRDIFF_MAX		(9223372036854775807L)

/* Limits of `sig_atomic_t'.  */
# define SIG_ATOMIC_MIN		(-2147483647-1)
# define SIG_ATOMIC_MAX		(2147483647)

/* Limit of `size_t' type.  */
#  define SIZE_MAX		(18446744073709551615UL)

/* Limits of `wint_t'.  */
# define WINT_MIN		(0u)
# define WINT_MAX		(4294967295u)

/* Signed.  */
# define INT8_C(c)	c
# define INT16_C(c)	c
# define INT32_C(c)	c
# define INT64_C(c)	c ## L

/* Unsigned.  */
# define UINT8_C(c)	c
# define UINT16_C(c)	c
# define UINT32_C(c)	c ## U
#  define UINT64_C(c)	c ## UL

/* Maximal type.  */
#  define INTMAX_C(c)	c ## L
#  define UINTMAX_C(c)	c ## UL

#endif /* stdint.h */
