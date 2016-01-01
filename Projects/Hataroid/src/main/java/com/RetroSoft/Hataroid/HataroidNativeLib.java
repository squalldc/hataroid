package com.RetroSoft.Hataroid;

public class HataroidNativeLib
{
	public static native void libExit();
		
	public static native void emulationInit(HataroidActivity instance, String[] keys, String[] vals);
	public static native void emulationMain();
	public static native void emulationDestroy(HataroidActivity instance);

	public static native void emulationPause();
	public static native void emulationResume();
	
	public static native void onSurfaceCreated();
	public static native void onSurfaceChanged(int width, int height);
	public static native boolean onDrawFrame();

    public static native void updateInput(	boolean t0, float tx0, float ty0,
    										boolean t1, float tx1, float ty1,
    										boolean t2, float tx2, float ty2,
    										float mouseX, float mouseY, int mouseBtns,
    										int[] keyPresses);

    public static native void emulatorSetOptions(String[] keys, String[] vals);
	public static native void emulatorResetCold();
	public static native void emulatorResetWarm();
	public static native void emulatorToggleUserPaused();
	public static native boolean emulatorGetUserPaused();
	public static native void emulatorEjectFloppy(int drive);
	public static native void emulatorInsertFloppy(int drive, String filename, String zipPath, String dispName);
	
	public static native void emulatorSaveStateSave(String path, String filepath, int saveSlot);
	public static native void emulatorSaveStateLoad(String path, String filepath, int saveSlot);

	public static native boolean emulatorAutoSaveStoreOnExit(String saveFolder);
	public static native void emulatorAutoSaveLoadOnStart(String saveFolder);

	public static native String emulatorGetCurFloppy(int floppy);
	public static native String emulatorGetCurFloppyZip(int floppy);

	public static native void emulatorSendMidiInstrPatch(int[] instrDefs);
	public static native void emulatorReceiveHardwareMidiBytes(int count, byte[] b);

	public static native void hataroidDialogResult(int result);
	public static native void hataroidSettingsResult(int result);
	
    //public static native int     andMidiInit();
    //public static native int[]   andMidiConfig();
    //public static native int     andMidiRender(short a[]);
    //public static native boolean andMidiWrite(byte a[]);
    //public static native boolean andMidiShutdown();
}
