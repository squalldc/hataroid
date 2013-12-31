/* CMake config.h for Hatari */

/* Define if you have a PNG compatible library */
#define HAVE_LIBPNG 0

/* Define if you have a readline compatible library */
#define HAVE_LIBREADLINE 0

/* Define if you have the PortAudio library */
#define HAVE_PORTAUDIO 0

/* Define if you have a X11 environment */
#define HAVE_X11 0

/* Define to 1 if you have the `z' library (-lz). */
#define HAVE_LIBZ 0

/* Define to 1 if you have the <zlib.h> header file. */
#define HAVE_ZLIB_H 1

/* Define to 1 if you have the <termios.h> header file. */
#define HAVE_TERMIOS_H 0

/* Define to 1 if you have the <glob.h> header file. */
#define HAVE_GLOB_H 0

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <SDL/SDL_config.h> header file. */
#define HAVE_SDL_SDL_CONFIG_H 0

/* Define to 1 if you have the <sys/times.h> header file. */
#define HAVE_SYS_TIMES_H 1

/* Define to 1 if you have the `cfmakeraw' function. */
#define HAVE_CFMAKERAW 0

/* Define to 1 if you have the 'setenv' function. */
#define HAVE_SETENV 0

/* Define to 1 if you have the `select' function. */
#define HAVE_SELECT 0

/* Define to 1 if you have unix domain sockets */
#define HAVE_UNIX_DOMAIN_SOCKETS 0

/* Define to 1 if you have the 'posix_memalign' function. */
#define HAVE_POSIX_MEMALIGN 0

/* Define to 1 if you have the 'memalign' function. */
#define HAVE_MEMALIGN 1

/* Define to 1 if you have the 'gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 0

/* Define to 1 if you have the 'nanosleep' function. */
#define HAVE_NANOSLEEP 0

/* Define to 1 if you have the 'alphasort' function. */
#define HAVE_ALPHASORT 0

/* Define to 1 if you have the 'scandir' function. */
#define HAVE_SCANDIR 1

/* Define to 1 if you have the 'statvfs' function. */
#define HAVE_STATVFS 0

#define ANDROID_USER_APP_DIR	"/sdcard/STDroid"

/* Relative path from bindir to datadir */
#define BIN2DATADIR ANDROID_USER_APP_DIR"/data"

/* Define to 1 to enable DSP 56k emulation for Falcon mode */
#define ENABLE_DSP_EMU 0

/* Define to 1 to enable WINUAE cpu  */
#define ENABLE_WINUAE_CPU 0

/* Define to 1 to use less memory - at the expense of emulation speed */
#define ENABLE_SMALL_MEM 0

/* Define to 1 to enable trace logs - undefine to slightly increase speed */
#define ENABLE_TRACING 0

#define CONFDIR ANDROID_USER_APP_DIR

