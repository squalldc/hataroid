LOCAL_PATH:= $(call my-dir)

# hatari-main
include $(CLEAR_VARS)

LOCAL_MODULE    := libhatari
LOCAL_C_INCLUDES += jni/zlib
LOCAL_C_INCLUDES += jni/SDL/include
LOCAL_C_INCLUDES += jni/hatari/cmake
LOCAL_C_INCLUDES += jni/hatari/src/includes
LOCAL_C_INCLUDES += jni/hatari/src/debug
LOCAL_C_INCLUDES += jni/hatari/src/falcon
LOCAL_C_INCLUDES += jni/hatari/src/gui-sdl

#cpu
LOCAL_SRC_FILES :=	src/uae-cpu/cpudefs.c src/uae-cpu/cpuemu.c src/uae-cpu/cpustbl.c \
					src/uae-cpu/fpp.c src/uae-cpu/hatari-glue.c src/uae-cpu/cpuMemory.c \
					src/uae-cpu/newcpu.c src/uae-cpu/readcpu.c
LOCAL_C_INCLUDES += jni/hatari/src/uae-cpu

#cpu
#LOCAL_SRC_FILES := src/Cyclone/Cyclone.s
#LOCAL_C_INCLUDES += jni/hatari/src/Cyclone

#debug
LOCAL_SRC_FILES +=	src/debug/68kDisass.c src/debug/breakcond.c src/debug/debugcpu.c src/debug/debugdsp.c \
					src/debug/debugInfo.c src/debug/debugui.c src/debug/evaluate.c src/debug/log.c \
					src/debug/profile.c src/debug/profilecpu.c src/debug/profiledsp.c src/debug/symbols.c src/debug/history.c \
					src/debug/natfeats.c src/debug/console.c

#falcon
LOCAL_SRC_FILES +=	src/falcon/crossbar.c src/falcon/dsp.c src/falcon/dsp_core.c src/falcon/dsp_cpu.c \
					src/falcon/dsp_disasm.c src/falcon/hostscreen.c src/falcon/microphone.c \
					src/falcon/nvram.c src/falcon/videl.c

#gui-sdl
#LOCAL_SRC_FILES +=	src/gui-sdl/dlgAbout.c src/gui-sdl/dlgAlert.c src/gui-sdl/dlgDevice.c src/gui-sdl/dlgFileSelect.c \
#					src/gui-sdl/dlgFloppy.c src/gui-sdl/dlgHardDisk.c src/gui-sdl/dlgJoystick.c src/gui-sdl/dlgKeyboard.c \
#					src/gui-sdl/dlgMain.c src/gui-sdl/dlgMemory.c src/gui-sdl/dlgNewDisk.c src/gui-sdl/dlgRom.c \
#					src/gui-sdl/dlgScreen.c src/gui-sdl/dlgSound.c src/gui-sdl/dlgSystem.c src/gui-sdl/sdlgui.c
LOCAL_SRC_FILES +=	src/gui-sdl/sdlgui.c

#gui-android
LOCAL_SRC_FILES +=	src/gui-android/gui-android.c

#main
LOCAL_SRC_FILES +=	src/acia.c src/audio.c src/avi_record.c src/bios.c src/blitter.c src/cart.c \
					src/cfgopts.c src/change.c src/clocks_timings.c src/configuration.c \
					src/control.c src/createBlankImage.c src/cycInt.c src/cycles.c \
					src/dim.c src/dmaSnd.c src/fdc.c src/file.c \
					src/floppy.c src/gemdos.c src/hd6301_cpu.c src/hdc.c src/ide.c \
					src/ikbd.c src/ioMem.c src/ioMemTabFalcon.c \
					src/ioMemTabST.c src/ioMemTabSTE.c src/ioMemTabTT.c src/joy.c src/keymap.c src/m68000.c \
					src/main.c src/memorySnapShot.c src/mfp.c src/midi.c src/msa.c src/options.c src/paths.c \
					src/printer.c src/psg.c src/reset.c src/resolution.c src/rs232.c src/rtc.c src/scandir.c \
					src/screen.c src/screenSnapShot.c src/sound.c src/spec512.c src/st.c src/statusbar.c \
					src/stMemory.c src/str.c src/tos.c src/unzip.c src/utils.c src/vdi.c src/video.c \
					src/wavFormat.c src/xbios.c src/ymFormat.c src/zip.c src/shortcut.c

#					src/dialog.c

LOCAL_CFLAGS		+= $(MY_LOCAL_CFLAGS)
LOCAL_ARM_MODE		:= $(MY_LOCAL_ARM_MODE)

include $(BUILD_STATIC_LIBRARY)
