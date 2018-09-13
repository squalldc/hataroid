package com.RetroSoft.Hataroid.Input;

public class VirtKeyDef
{
	public int		id;
	public String[]		name;
	public short[]		androidKeycode;
	public short		config;
	public short		scut;
	public short		sort;
	public int		flags;

	public VirtKeyDef(int id_, String[] name_, short[] androidKeycode_, short config_, short scut_, short sort_, int flags_)
	{
		id = id_;
		name = name_;
		androidKeycode = androidKeycode_;
		config = config_;
		scut = scut_;
		sort = sort_;
		flags = flags_;
	}

	public String getName(int localeID)				{ return name[localeID]; }
	public short getAndroidKeyCode(int localeID)	{ return androidKeycode[localeID]; }

	public final static int VKB_KEY_F1		= 0;
	public final static int VKB_KEY_F2		= 1;
	public final static int VKB_KEY_F3		= 2;
	public final static int VKB_KEY_F4		= 3;
	public final static int VKB_KEY_F5		= 4;
	public final static int VKB_KEY_F6		= 5;
	public final static int VKB_KEY_F7		= 6;
	public final static int VKB_KEY_F8		= 7;
	public final static int VKB_KEY_F9		= 8;
	public final static int VKB_KEY_F10		= 9;
	public final static int VKB_KEY_ESC		= 10;
	public final static int VKB_KEY_1		= 11;
	public final static int VKB_KEY_2		= 12;
	public final static int VKB_KEY_3		= 13;
	public final static int VKB_KEY_4		= 14;
	public final static int VKB_KEY_5		= 15;
	public final static int VKB_KEY_6		= 16;
	public final static int VKB_KEY_7		= 17;
	public final static int VKB_KEY_8		= 18;
	public final static int VKB_KEY_9		= 19;
	public final static int VKB_KEY_0		= 20;
	public final static int VKB_KEY_SUBTRACT		= 21;
	public final static int VKB_KEY_EQUAL		= 22;
	public final static int VKB_KEY_BACKQUOTE		= 23;
	public final static int VKB_KEY_BACKSPACE		= 24;
	public final static int VKB_KEY_HELP		= 25;
	public final static int VKB_KEY_UNDO		= 26;
	public final static int VKB_KEY_NUMPAD_LEFTPAREN		= 27;
	public final static int VKB_KEY_NUMPAD_RIGHTPAREN		= 28;
	public final static int VKB_KEY_NUMPAD_DIVIDE		= 29;
	public final static int VKB_KEY_NUMPAD_MULTIPLY		= 30;
	public final static int VKB_KEY_TAB		= 31;
	public final static int VKB_KEY_Q		= 32;
	public final static int VKB_KEY_W		= 33;
	public final static int VKB_KEY_E		= 34;
	public final static int VKB_KEY_R		= 35;
	public final static int VKB_KEY_T		= 36;
	public final static int VKB_KEY_Y		= 37;
	public final static int VKB_KEY_U		= 38;
	public final static int VKB_KEY_I		= 39;
	public final static int VKB_KEY_O		= 40;
	public final static int VKB_KEY_P		= 41;
	public final static int VKB_KEY_LEFTBRACKET		= 42;
	public final static int VKB_KEY_RIGHTBRACKET		= 43;
	public final static int VKB_KEY_RETURN		= 44;
	public final static int VKB_KEY_DELETE		= 45;
	public final static int VKB_KEY_INSERT		= 46;
	public final static int VKB_KEY_UPARROW		= 47;
	public final static int VKB_KEY_HOME		= 48;
	public final static int VKB_KEY_NUMPAD_7		= 49;
	public final static int VKB_KEY_NUMPAD_8		= 50;
	public final static int VKB_KEY_NUMPAD_9		= 51;
	public final static int VKB_KEY_NUMPAD_SUBTRACT		= 52;
	public final static int VKB_KEY_CONTROL		= 53;
	public final static int VKB_KEY_A		= 54;
	public final static int VKB_KEY_S		= 55;
	public final static int VKB_KEY_D		= 56;
	public final static int VKB_KEY_F		= 57;
	public final static int VKB_KEY_G		= 58;
	public final static int VKB_KEY_H		= 59;
	public final static int VKB_KEY_J		= 60;
	public final static int VKB_KEY_K		= 61;
	public final static int VKB_KEY_L		= 62;
	public final static int VKB_KEY_SEMICOLON		= 63;
	public final static int VKB_KEY_APOSTROPHE		= 64;
	public final static int VKB_KEY_TILDA		= 66;
	public final static int VKB_KEY_LEFTARROW		= 67;
	public final static int VKB_KEY_DOWNARROW		= 68;
	public final static int VKB_KEY_RIGHTARROW		= 69;
	public final static int VKB_KEY_NUMPAD_4		= 70;
	public final static int VKB_KEY_NUMPAD_5		= 71;
	public final static int VKB_KEY_NUMPAD_6		= 72;
	public final static int VKB_KEY_NUMPAD_ADD		= 73;
	public final static int VKB_KEY_LEFTSHIFT		= 74;
	public final static int VKB_KEY_BACKSLASH		= 75;
	public final static int VKB_KEY_Z		= 76;
	public final static int VKB_KEY_X		= 77;
	public final static int VKB_KEY_C		= 78;
	public final static int VKB_KEY_V		= 79;
	public final static int VKB_KEY_B		= 80;
	public final static int VKB_KEY_N		= 81;
	public final static int VKB_KEY_M		= 82;
	public final static int VKB_KEY_COMMA		= 83;
	public final static int VKB_KEY_PERIOD		= 84;
	public final static int VKB_KEY_FORWARDSLASH		= 85;
	public final static int VKB_KEY_RIGHTSHIFT		= 86;
	public final static int VKB_KEY_NUMPAD_1		= 87;
	public final static int VKB_KEY_NUMPAD_2		= 88;
	public final static int VKB_KEY_NUMPAD_3		= 89;
	public final static int VKB_KEY_NUMPAD_ENTER		= 90;
	public final static int VKB_KEY_ALTERNATE		= 91;
	public final static int VKB_KEY_SPACE		= 92;
	public final static int VKB_KEY_CAPS		= 93;
	public final static int VKB_KEY_NUMPAD_0		= 94;
	public final static int VKB_KEY_NUMPAD_PERIOD		= 95;
	public final static int VKB_KEY_KEYBOARDTOGGLE		= 97;
	public final static int VKB_KEY_KEYBOARDTOGGLE_SEL		= 98;
	public final static int VKB_KEY_JOYLEFT		= 99;
	public final static int VKB_KEY_JOYRIGHT		= 100;
	public final static int VKB_KEY_JOYDOWN		= 101;
	public final static int VKB_KEY_JOYUP		= 102;
	public final static int VKB_KEY_JOYFIRE_OLD		= 103;
	public final static int VKB_KEY_KEYBOARDZOOM		= 104;
	public final static int VKB_KEY_KEYBOARDZOOM_SEL		= 105;
	public final static int VKB_KEY_SCREENZOOM		= 106;
	public final static int VKB_KEY_SCREENZOOM_SEL		= 107;
	public final static int VKB_KEY_MOUSETOGGLE		= 108;
	public final static int VKB_KEY_JOYTOGGLE		= 109;
	public final static int VKB_KEY_MOUSELB		= 110;
	public final static int VKB_KEY_MOUSERB		= 111;
	public final static int VKB_KEY_NORMALSPEED		= 112;
	public final static int VKB_KEY_TURBOSPEED		= 113;
	public final static int VKB_KEY_SCREENPRESETS		= 114;
	public final static int VKB_KEY_KEYBOARDPRESETS		= 115;
	public final static int VKB_KEY_JOYFIRE		= 116;
	public final static int VKB_KEY_LEFTSHIFT_BUTTON		= 117;
	public final static int VKB_KEY_RIGHTSHIFT_BUTTON		= 118;
	public final static int VKB_KEY_TOGGLEUI		= 119;
	public final static int VKB_KEY_AUTOFIRE		= 120;
	public final static int VKB_KEY_QUICKSAVESTATE		= 121;
	public final static int VKB_KEY_QUICKLOADSTATE		= 122;
	public final static int VKB_KEY_ANDROID_MENU		= 123;
	public final static int VKB_KEY_FLOPPY_MENU		= 124;
	public final static int VKB_KEY_JOY2LEFT		= 125;
	public final static int VKB_KEY_JOY2RIGHT		= 126;
	public final static int VKB_KEY_JOY2DOWN		= 127;
	public final static int VKB_KEY_JOY2UP		= 128;
	public final static int VKB_KEY_JOY2FIRE		= 129;
	public final static int VKB_KEY_FLOPPYA_INSERT		= 130;
	public final static int VKB_KEY_FLOPPYB_INSERT		= 131;
	public final static int VKB_KEY_SETTINGS_MENU		= 132;
	public final static int VKB_KEY_ANDROID_BACK		= 133;
	public final static int VKB_KEY_PAUSE		= 134;
	public final static int VKB_KEY_NAVLEFT		= 135;
	public final static int VKB_KEY_NAVRIGHT		= 136;
	public final static int VKB_KEY_NAVDOWN		= 137;
	public final static int VKB_KEY_NAVUP		= 138;
	public final static int VKB_KEY_NAVBTN		= 139;
	public final static int VKB_KEY_LEFTSHIFT_HOLD		= 140;
	public final static int VKB_KEY_RIGHTSHIFT_HOLD		= 141;
	public final static int VKB_KEY_ALTERNATE_HOLD		= 142;
	public final static int VKB_KEY_CONTROL_HOLD		= 143;
	public final static int VKB_KEY_MODLOCK		= 144;
	public final static int VKB_KEY_BKG		= 145;

	public final static int VKB_KEY_NumOf		= 146;

	public final static int FLAG_STFNKEY		= (1<<0);
	public final static int FLAG_POLY		= (1<<1);
	public final static int FLAG_STKEY		= (1<<2);
	public final static int FLAG_REGION_REMAP		= (1<<3);
	public final static int FLAG_MODKEY		= (1<<4);
	public final static int FLAG_CUSTOMKEY		= (1<<5);
	public final static int FLAG_PERSIST		= (1<<6);
	public final static int FLAG_JOY		= (1<<7);
	public final static int FLAG_VKB		= (1<<8);
	public final static int FLAG_SCREEN		= (1<<9);
	public final static int FLAG_SCREEN2		= (1<<10);
	public final static int FLAG_MOUSE		= (1<<11);
	public final static int FLAG_MOUSEBUTTON		= (1<<12);
	public final static int FLAG_MAIN		= (1<<13);
	public final static int FLAG_HOLDKEY		= (1<<14);


	public final static VirtKeyDef[] kDefs = {
		new VirtKeyDef(VKB_KEY_F1, new String[] { "F1", "F1", "F1" }, new short[] { (short)131, (short)131, (short)131 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F2, new String[] { "F2", "F2", "F2" }, new short[] { (short)132, (short)132, (short)132 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F3, new String[] { "F3", "F3", "F3" }, new short[] { (short)133, (short)133, (short)133 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F4, new String[] { "F4", "F4", "F4" }, new short[] { (short)134, (short)134, (short)134 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F5, new String[] { "F5", "F5", "F5" }, new short[] { (short)135, (short)135, (short)135 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F6, new String[] { "F6", "F6", "F6" }, new short[] { (short)136, (short)136, (short)136 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F7, new String[] { "F7", "F7", "F7" }, new short[] { (short)137, (short)137, (short)137 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F8, new String[] { "F8", "F8", "F8" }, new short[] { (short)138, (short)138, (short)138 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F9, new String[] { "F9", "F9", "F9" }, new short[] { (short)139, (short)139, (short)139 }, (short)1, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F10, new String[] { "F10", "F10", "F10" }, new short[] { (short)140, (short)140, (short)140 }, (short)1, (short)1, (short)1, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_ESC, new String[] { "Esc", "Esc", "Esc" }, new short[] { (short)111, (short)111, (short)111 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_1, new String[] { "1", "1", "1" }, new short[] { (short)8, (short)8, (short)8 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_2, new String[] { "2", "2", "2" }, new short[] { (short)9, (short)9, (short)9 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_3, new String[] { "3", "3", "3" }, new short[] { (short)10, (short)10, (short)10 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_4, new String[] { "4", "4", "4" }, new short[] { (short)11, (short)11, (short)11 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_5, new String[] { "5", "5", "5" }, new short[] { (short)12, (short)12, (short)12 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_6, new String[] { "6", "6", "6" }, new short[] { (short)13, (short)13, (short)13 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_7, new String[] { "7", "7", "7" }, new short[] { (short)14, (short)14, (short)14 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_8, new String[] { "8", "8", "8" }, new short[] { (short)15, (short)15, (short)15 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_9, new String[] { "9", "9", "9" }, new short[] { (short)16, (short)16, (short)16 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_0, new String[] { "0", "0", "0" }, new short[] { (short)7, (short)7, (short)7 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_SUBTRACT, new String[] { "- (Minus)", "\u00df", ") (Right Bracket)" }, new short[] { (short)69, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_EQUAL, new String[] { "= (Equal)", "\u00b4 (Acute Accent)", "- (Minus)" }, new short[] { (short)70, (short)-1, (short)69 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_BACKQUOTE, new String[] { "` (Backquote)", "#", "` (Grave Accent)" }, new short[] { (short)68, (short)-1, (short)68 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_BACKSPACE, new String[] { "Backspace", "Backspace", "Backspace" }, new short[] { (short)67, (short)67, (short)67 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_HELP, new String[] { "Help", "Help", "Help" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_UNDO, new String[] { "Undo", "Undo", "Undo" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_LEFTPAREN, new String[] { "Numpad (", "Numpad (", "Numpad (" }, new short[] { (short)162, (short)162, (short)162 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_RIGHTPAREN, new String[] { "Numpad )", "Numpad )", "Numpad )" }, new short[] { (short)163, (short)163, (short)163 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_DIVIDE, new String[] { "Numpad /", "Numpad /", "Numpad /" }, new short[] { (short)154, (short)154, (short)154 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_MULTIPLY, new String[] { "Numpad *", "Numpad *", "Numpad *" }, new short[] { (short)155, (short)155, (short)155 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TAB, new String[] { "Tab", "Tab", "Tab" }, new short[] { (short)61, (short)61, (short)61 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_Q, new String[] { "Q", "Q", "A" }, new short[] { (short)45, (short)45, (short)29 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_W, new String[] { "W", "W", "Z" }, new short[] { (short)51, (short)51, (short)54 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_E, new String[] { "E", "E", "E" }, new short[] { (short)33, (short)33, (short)33 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_R, new String[] { "R", "R", "R" }, new short[] { (short)46, (short)46, (short)46 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_T, new String[] { "T", "T", "T" }, new short[] { (short)48, (short)48, (short)48 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_Y, new String[] { "Y", "Z", "Y" }, new short[] { (short)53, (short)54, (short)53 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_U, new String[] { "U", "U", "U" }, new short[] { (short)49, (short)49, (short)49 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_I, new String[] { "I", "I", "I" }, new short[] { (short)37, (short)37, (short)37 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_O, new String[] { "O", "O", "O" }, new short[] { (short)43, (short)43, (short)43 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_P, new String[] { "P", "P", "P" }, new short[] { (short)44, (short)44, (short)44 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTBRACKET, new String[] { "[ (Left Bracket)", "\u00dc", "^" }, new short[] { (short)71, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_RIGHTBRACKET, new String[] { "] (Right Bracket)", "+", "$" }, new short[] { (short)72, (short)81, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_RETURN, new String[] { "Return", "Return", "Return" }, new short[] { (short)66, (short)66, (short)66 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_DELETE, new String[] { "Delete", "Delete", "Delete" }, new short[] { (short)112, (short)112, (short)112 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_INSERT, new String[] { "Insert", "Insert", "Insert" }, new short[] { (short)124, (short)124, (short)124 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_UPARROW, new String[] { "Arrow Up", "Arrow Up", "Arrow Up" }, new short[] { (short)19, (short)19, (short)19 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_HOME, new String[] { "Home", "Home", "Home" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_7, new String[] { "Numpad 7", "Numpad 7", "Numpad 7" }, new short[] { (short)151, (short)151, (short)151 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_8, new String[] { "Numpad 8", "Numpad 8", "Numpad 8" }, new short[] { (short)152, (short)152, (short)152 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_9, new String[] { "Numpad 9", "Numpad 9", "Numpad 9" }, new short[] { (short)153, (short)153, (short)153 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_SUBTRACT, new String[] { "Numpad -", "Numpad -", "Numpad -" }, new short[] { (short)156, (short)156, (short)156 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_CONTROL, new String[] { "Control", "Control", "Control" }, new short[] { (short)113, (short)113, (short)113 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_MODKEY),
		new VirtKeyDef(VKB_KEY_A, new String[] { "A", "A", "Q" }, new short[] { (short)29, (short)29, (short)45 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_S, new String[] { "S", "S", "S" }, new short[] { (short)47, (short)47, (short)47 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_D, new String[] { "D", "D", "D" }, new short[] { (short)32, (short)32, (short)32 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_F, new String[] { "F", "F", "F" }, new short[] { (short)34, (short)34, (short)34 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_G, new String[] { "G", "G", "G" }, new short[] { (short)35, (short)35, (short)35 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_H, new String[] { "H", "H", "H" }, new short[] { (short)36, (short)36, (short)36 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_J, new String[] { "J", "J", "J" }, new short[] { (short)38, (short)38, (short)38 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_K, new String[] { "K", "K", "K" }, new short[] { (short)39, (short)39, (short)39 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_L, new String[] { "L", "L", "L" }, new short[] { (short)40, (short)40, (short)40 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_SEMICOLON, new String[] { "; (Semicolon)", "\u00d6", "M" }, new short[] { (short)74, (short)-1, (short)41 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_APOSTROPHE, new String[] { "' (Apostrophe)", "\u00c4", "\u00f9" }, new short[] { (short)75, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_RETURN, new String[] { "Return", "Return", "Return" }, new short[] { (short)66, (short)66, (short)66 }, (short)0, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TILDA, new String[] { "~ (Tilda)", "~ (Tilda)", "#" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTARROW, new String[] { "Arrow Left", "Arrow Left", "Arrow Left" }, new short[] { (short)21, (short)21, (short)21 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_DOWNARROW, new String[] { "Arrow Down", "Arrow Down", "Arrow Down" }, new short[] { (short)20, (short)20, (short)20 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTARROW, new String[] { "Arrow Right", "Arrow Right", "Arrow Right" }, new short[] { (short)22, (short)22, (short)22 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_4, new String[] { "Numpad 4", "Numpad 4", "Numpad 4" }, new short[] { (short)148, (short)148, (short)148 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_5, new String[] { "Numpad 5", "Numpad 5", "Numpad 5" }, new short[] { (short)149, (short)149, (short)149 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_6, new String[] { "Numpad 6", "Numpad 6", "Numpad 6" }, new short[] { (short)150, (short)150, (short)150 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ADD, new String[] { "Numpad +", "Numpad +", "Numpad +" }, new short[] { (short)157, (short)157, (short)157 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTSHIFT, new String[] { "Left Shift", "Left Shift", "Left Shift" }, new short[] { (short)59, (short)59, (short)59 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_MODKEY),
		new VirtKeyDef(VKB_KEY_BACKSLASH, new String[] { "\\ (Backslash)", "<", "<" }, new short[] { (short)73, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_Z, new String[] { "Z", "Y", "W" }, new short[] { (short)54, (short)53, (short)51 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_X, new String[] { "X", "X", "X" }, new short[] { (short)52, (short)52, (short)52 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_C, new String[] { "C", "C", "C" }, new short[] { (short)31, (short)31, (short)31 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_V, new String[] { "V", "V", "V" }, new short[] { (short)50, (short)50, (short)50 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_B, new String[] { "B", "B", "B" }, new short[] { (short)30, (short)30, (short)30 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_N, new String[] { "N", "N", "N" }, new short[] { (short)42, (short)42, (short)42 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_M, new String[] { "M", "M", ", (Comma)" }, new short[] { (short)41, (short)41, (short)55 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_COMMA, new String[] { ", (Comma)", ", (Comma)", "; (Semicolon)" }, new short[] { (short)55, (short)55, (short)74 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_PERIOD, new String[] { ". (Period)", ". (Period)", ": (Colon)" }, new short[] { (short)56, (short)56, (short)-1 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_FORWARDSLASH, new String[] { "/ (Forward Slash)", "- (Minus)", "= (Equal)" }, new short[] { (short)76, (short)69, (short)70 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_REGION_REMAP),
		new VirtKeyDef(VKB_KEY_RIGHTSHIFT, new String[] { "Right Shift", "Right Shift", "Right Shift" }, new short[] { (short)60, (short)60, (short)60 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_MODKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_1, new String[] { "Numpad 1", "Numpad 1", "Numpad 1" }, new short[] { (short)145, (short)145, (short)145 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_2, new String[] { "Numpad 2", "Numpad 2", "Numpad 2" }, new short[] { (short)146, (short)146, (short)146 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_3, new String[] { "Numpad 3", "Numpad 3", "Numpad 3" }, new short[] { (short)147, (short)147, (short)147 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ENTER, new String[] { "Numpad Enter", "Numpad Enter", "Numpad Enter" }, new short[] { (short)160, (short)160, (short)160 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_ALTERNATE, new String[] { "Alternate", "Alternate", "Alternate" }, new short[] { (short)57, (short)57, (short)57 }, (short)1, (short)1, (short)0, FLAG_STKEY|FLAG_MODKEY),
		new VirtKeyDef(VKB_KEY_SPACE, new String[] { "Space", "Space", "Space" }, new short[] { (short)62, (short)62, (short)62 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_CAPS, new String[] { "Capslock", "Capslock", "Capslock" }, new short[] { (short)115, (short)115, (short)115 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_0, new String[] { "Numpad 0", "Numpad 0", "Numpad 0" }, new short[] { (short)144, (short)144, (short)144 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_PERIOD, new String[] { "Numpad . (Period)", "Numpad . (Period)", "Numpad . (Period)" }, new short[] { (short)158, (short)158, (short)158 }, (short)1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ENTER, new String[] { "Numpad Enter", "Numpad Enter", "Numpad Enter" }, new short[] { (short)160, (short)160, (short)160 }, (short)0, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_KEYBOARDTOGGLE, new String[] { "Toggle Keyboard", "Toggle Keyboard", "Toggle Keyboard" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_KEYBOARDTOGGLE_SEL, new String[] { "Toggle Keyboard", "Toggle Keyboard", "Toggle Keyboard" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_JOYLEFT, new String[] { "Joystick Left", "Joystick Left", "Joystick Left" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)1, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYRIGHT, new String[] { "Joystick Right", "Joystick Right", "Joystick Right" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)2, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYDOWN, new String[] { "Joystick Down", "Joystick Down", "Joystick Down" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)4, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYUP, new String[] { "Joystick Up", "Joystick Up", "Joystick Up" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)3, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYFIRE_OLD, new String[] { "Joystick Fire Old", "Joystick Fire Old", "Joystick Fire Old" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_KEYBOARDZOOM, new String[] { "Keyboard Zoom", "Keyboard Zoom", "Keyboard Zoom" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_KEYBOARDZOOM_SEL, new String[] { "Keyboard Zoom", "Keyboard Zoom", "Keyboard Zoom" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_SCREENZOOM, new String[] { "Screen Zoom", "Screen Zoom", "Screen Zoom" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_SCREENZOOM_SEL, new String[] { "Screen Zoom", "Screen Zoom", "Screen Zoom" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_MOUSETOGGLE, new String[] { "Toggle Mouse/Joystick", "Toggle Mouse/Joystick", "Toggle Mouse/Joystick" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)10, FLAG_CUSTOMKEY|FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYTOGGLE, new String[] { "Toggle Mouse/Joystick", "Toggle Mouse/Joystick", "Toggle Mouse/Joystick" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MOUSE),
		new VirtKeyDef(VKB_KEY_MOUSELB, new String[] { "Left Mouse Button", "Left Mouse Button", "Left Mouse Button" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_MOUSEBUTTON),
		new VirtKeyDef(VKB_KEY_MOUSERB, new String[] { "Right Mouse Button", "Right Mouse Button", "Right Mouse Button" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_MOUSEBUTTON),
		new VirtKeyDef(VKB_KEY_NORMALSPEED, new String[] { "Toggle Turbo Speed", "Toggle Turbo Speed", "Toggle Turbo Speed" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_TURBOSPEED, new String[] { "Toggle Turbo Speed", "Toggle Turbo Speed", "Toggle Turbo Speed" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_SCREENPRESETS, new String[] { "Cycle Screen Zoom Presets", "Cycle Screen Zoom Presets", "Cycle Screen Zoom Presets" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_KEYBOARDPRESETS, new String[] { "Cycle Keyboard Zoom Presets", "Cycle Keyboard Zoom Presets", "Cycle Keyboard Zoom Presets" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_JOYFIRE, new String[] { "Joystick Fire", "Joystick Fire", "Joystick Fire" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_LEFTSHIFT_BUTTON, new String[] { "Left Shift Button", "Left Shift Button", "Left Shift Button" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTSHIFT_BUTTON, new String[] { "Right Shift Button", "Right Shift Button", "Right Shift Button" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TOGGLEUI, new String[] { "Toggle UI", "Toggle UI", "Toggle UI" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY),
		new VirtKeyDef(VKB_KEY_AUTOFIRE, new String[] { "Toggle Auto Fire", "Toggle Auto Fire", "Toggle Auto Fire" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY),
		new VirtKeyDef(VKB_KEY_QUICKSAVESTATE, new String[] { "Quick Save State", "Quick Save State", "Quick Save State" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY),
		new VirtKeyDef(VKB_KEY_QUICKLOADSTATE, new String[] { "Quick Load State", "Quick Load State", "Quick Load State" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY),
		new VirtKeyDef(VKB_KEY_ANDROID_MENU, new String[] { "Android Soft Menu", "Android Soft Menu", "Android Soft Menu" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_FLOPPY_MENU, new String[] { "Floppy Menu", "Floppy Menu", "Floppy Menu" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_JOY2LEFT, new String[] { "Joystick 2 Left", "Joystick 2 Left", "Joystick 2 Left" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)6, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOY2RIGHT, new String[] { "Joystick 2 Right", "Joystick 2 Right", "Joystick 2 Right" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)7, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOY2DOWN, new String[] { "Joystick 2 Down", "Joystick 2 Down", "Joystick 2 Down" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)9, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOY2UP, new String[] { "Joystick 2 Up", "Joystick 2 Up", "Joystick 2 Up" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)8, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOY2FIRE, new String[] { "Joystick 2 Fire", "Joystick 2 Fire", "Joystick 2 Fire" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)0, (short)5, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_FLOPPYA_INSERT, new String[] { "Floppy A Insert", "Floppy A Insert", "Floppy A Insert" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_FLOPPYB_INSERT, new String[] { "Floppy B Insert", "Floppy B Insert", "Floppy B Insert" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_SETTINGS_MENU, new String[] { "Settings Menu", "Settings Menu", "Settings Menu" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_ANDROID_BACK, new String[] { "Android Back", "Android Back", "Android Back" }, new short[] { (short)4, (short)4, (short)4 }, (short)1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_PAUSE, new String[] { "Pause Toggle", "Pause Toggle", "Pause Toggle" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_NAVLEFT, new String[] { "Nav Left", "Nav Left", "Nav Left" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_NAVRIGHT, new String[] { "Nav Right", "Nav Right", "Nav Right" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_NAVDOWN, new String[] { "Nav Down", "Nav Down", "Nav Down" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_NAVUP, new String[] { "Nav Up", "Nav Up", "Nav Up" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_NAVBTN, new String[] { "Nav Button", "Nav Button", "Nav Button" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_LEFTSHIFT_HOLD, new String[] { "Left Shift Hold", "Left Shift Hold", "Left Shift Hold" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY|FLAG_HOLDKEY),
		new VirtKeyDef(VKB_KEY_RIGHTSHIFT_HOLD, new String[] { "Right Shift Hold", "Right Shift Hold", "Right Shift Hold" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY|FLAG_HOLDKEY),
		new VirtKeyDef(VKB_KEY_ALTERNATE_HOLD, new String[] { "Alternate Hold", "Alternate Hold", "Alternate Hold" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY|FLAG_HOLDKEY),
		new VirtKeyDef(VKB_KEY_CONTROL_HOLD, new String[] { "Control Hold", "Control Hold", "Control Hold" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_STKEY|FLAG_HOLDKEY),
		new VirtKeyDef(VKB_KEY_MODLOCK, new String[] { "Mod Lock", "Mod Lock", "Mod Lock" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_BKG, new String[] { "Background", "Background", "Background" }, new short[] { (short)-1, (short)-1, (short)-1 }, (short)0, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN|FLAG_PERSIST)
	};
}
