/* libjpeg-turbo build number */
#define BUILD  "opencv-4.5.0-libjpeg-turbo"

/* Compiler's inline keyword */
#undef inline

/* How to obtain function inlining. */
#ifndef INLINE
#if defined(__GNUC__)
#define INLINE inline __attribute__((always_inline))
#elif defined(_MSC_VER)
#define INLINE __forceinline
#else
#define INLINE
#endif
#endif

/* How to obtain thread-local storage */
#define THREAD_LOCAL  

/* Define to the full name of this package. */
#define PACKAGE_NAME  "OpenCV"

/* Version number of package */
#define VERSION  "2.0.5"

/* The size of `size_t', as computed by sizeof. */
#ifdef WIN64
#define SIZEOF_SIZE_T  8
#else
#define SIZEOF_SIZE_T  4
#endif
/* Define if your compiler has __builtin_ctzl() and sizeof(unsigned long) == sizeof(size_t). */
/* #undef HAVE_BUILTIN_CTZL */

/* Define to 1 if you have the <intrin.h> header file. */
#define HAVE_INTRIN_H

#if defined(_MSC_VER) && defined(HAVE_INTRIN_H)
#if (SIZEOF_SIZE_T == 8)
#define HAVE_BITSCANFORWARD64
#elif (SIZEOF_SIZE_T == 4)
#define HAVE_BITSCANFORWARD
#endif
#endif
