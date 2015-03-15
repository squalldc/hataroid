LOCAL_PATH:= $(call my-dir)

# glib-main
include $(CLEAR_VARS)

LOCAL_MODULE    := libglib
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/glib
LOCAL_C_INCLUDES += $(APP_PROJECT_PATH)/jni/glib/glib

#glib
LOCAL_SRC_FILES := glib/deprecated/gallocator.c \
	glib/deprecated/gcache.c glib/deprecated/gcompletion.c glib/deprecated/grel.c \
	glib/deprecated/gthread-deprecated.c glib/garray.c \
	glib/gasyncqueue.c glib/gatomic.c glib/gbacktrace.c \
	glib/gbase64.c glib/gbitlock.c glib/gbookmarkfile.c glib/gbytes.c \
	glib/gcharset.c glib/gchecksum.c glib/gconvert.c \
	glib/gdataset.c glib/gdate.c glib/gdatetime.c glib/gdir.c \
	glib/genviron.c glib/gerror.c glib/gfileutils.c glib/ggettext.c glib/ghash.c glib/ghmac.c \
	glib/ghook.c glib/ghostutils.c glib/giochannel.c glib/gkeyfile.c \
	glib/glib-init.c \
	glib/glib-private.c glib/glist.c glib/gmain.c glib/gmappedfile.c \
	glib/gmarkup.c glib/gmem.c glib/gmessages.c glib/gnode.c \
	glib/goption.c glib/gpattern.c glib/gpoll.c glib/gprimes.c glib/gqsort.c glib/gquark.c \
	glib/gqueue.c glib/grand.c glib/gregex.c glib/gscanner.c \
	glib/gsequence.c glib/gshell.c glib/gslice.c glib/gslist.c glib/gstdio.c glib/gstrfuncs.c \
	glib/gstring.c glib/gstringchunk.c glib/gthread.c \
	glib/gthreadpool.c glib/gtimer.c glib/gtimezone.c \
	glib/gtranslit.c glib/gtrashstack.c glib/gtree.c glib/guniprop.c \
	glib/gutf8.c glib/gunibreak.c glib/gunicollate.c \
	glib/gunidecomp.c \
	glib/gurifuncs.c glib/gutils.c glib/gvariant.c \
	glib/gvariant-core.c glib/gvariant-parser.c \
	glib/gvariant-serialiser.c \
	glib/gvarianttypeinfo.c glib/gvarianttype.c glib/gversion.c \
	glib/gwakeup.c glib/gprintf.c glib/glib-unix.c \
	glib/gthread-posix.c \
	glib/gtestutils.c

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS) -DGLIB_COMPILATION
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

include $(BUILD_STATIC_LIBRARY)
