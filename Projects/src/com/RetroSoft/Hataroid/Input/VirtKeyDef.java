package com.RetroSoft.Hataroid.Input;

public class VirtKeyDef
{
	public int		id;
	public String		name;
	public short		androidKeycode;
	public short		config;
	public short		sort;
	public int		flags;

	public VirtKeyDef(int id_, String name_, short androidKeycode_, short config_, short sort_, int flags_)
	{
		id = id_;
		name = name_;
		androidKeycode = androidKeycode_;
		config = config_;
		sort = sort_;
		flags = flags_;
	}

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

	public final static int VKB_KEY_NumOf		= 120;

	public final static int FLAG_POLY		= (1<<0);
	public final static int FLAG_STFNKEY		= (1<<1);
	public final static int FLAG_STKEY		= (1<<2);
	public final static int FLAG_PERSIST		= (1<<3);
	public final static int FLAG_CUSTOMKEY		= (1<<4);
	public final static int FLAG_JOY		= (1<<5);
	public final static int FLAG_VKB		= (1<<6);
	public final static int FLAG_SCREEN2		= (1<<7);
	public final static int FLAG_SCREEN		= (1<<8);
	public final static int FLAG_MOUSE		= (1<<9);
	public final static int FLAG_MOUSEBUTTON		= (1<<10);
	public final static int FLAG_MAIN		= (1<<11);


	public final static VirtKeyDef[] kDefs = {
		new VirtKeyDef(VKB_KEY_F1, "F1", (short)131, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F2, "F2", (short)132, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F3, "F3", (short)133, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F4, "F4", (short)134, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F5, "F5", (short)135, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F6, "F6", (short)136, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F7, "F7", (short)137, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F8, "F8", (short)138, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F9, "F9", (short)139, (short)1, (short)0, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_F10, "F10", (short)140, (short)1, (short)1, FLAG_POLY|FLAG_STFNKEY),
		new VirtKeyDef(VKB_KEY_ESC, "Esc", (short)111, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_1, "1", (short)8, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_2, "2", (short)9, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_3, "3", (short)10, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_4, "4", (short)11, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_5, "5", (short)12, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_6, "6", (short)13, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_7, "7", (short)14, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_8, "8", (short)15, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_9, "9", (short)16, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_0, "0", (short)7, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_SUBTRACT, "- (Minus)", (short)69, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_EQUAL, "= (Equal)", (short)70, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_BACKQUOTE, "` (Backquote)", (short)68, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_BACKSPACE, "Backspace", (short)67, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_HELP, "Help", (short)-1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_UNDO, "Undo", (short)-1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_LEFTPAREN, "Numpad (", (short)162, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_RIGHTPAREN, "Numpad )", (short)163, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_DIVIDE, "Numpad /", (short)154, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_MULTIPLY, "Numpad *", (short)155, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TAB, "Tab", (short)61, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_Q, "Q", (short)45, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_W, "W", (short)51, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_E, "E", (short)33, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_R, "R", (short)46, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_T, "T", (short)48, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_Y, "Y", (short)53, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_U, "U", (short)49, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_I, "I", (short)37, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_O, "O", (short)43, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_P, "P", (short)44, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTBRACKET, "[ (Left Bracket)", (short)71, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTBRACKET, "] (Right Bracket)", (short)72, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RETURN, "Return", (short)66, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_DELETE, "Delete", (short)112, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_INSERT, "Insert", (short)124, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_UPARROW, "Arrow Up", (short)19, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_HOME, "Home", (short)-1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_7, "Numpad 7", (short)151, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_8, "Numpad 8", (short)152, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_9, "Numpad 9", (short)153, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_SUBTRACT, "Numpad -", (short)156, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_CONTROL, "Control", (short)113, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_A, "A", (short)29, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_S, "S", (short)47, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_D, "D", (short)32, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_F, "F", (short)34, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_G, "G", (short)35, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_H, "H", (short)36, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_J, "J", (short)38, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_K, "K", (short)39, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_L, "L", (short)40, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_SEMICOLON, "; (Semicolon)", (short)74, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_APOSTROPHE, "' (Apostrophe)", (short)75, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RETURN, "Return", (short)66, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TILDA, "~ (Tilda)", (short)-1, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTARROW, "Arrow Left", (short)21, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_DOWNARROW, "Arrow Down", (short)20, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTARROW, "Arrow Right", (short)22, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_4, "Numpad 4", (short)148, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_5, "Numpad 5", (short)149, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_6, "Numpad 6", (short)150, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ADD, "Numpad +", (short)157, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_LEFTSHIFT, "Left Shift", (short)59, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_BACKSLASH, "\\ (Backslash)", (short)73, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_Z, "Z", (short)54, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_X, "X", (short)52, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_C, "C", (short)31, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_V, "V", (short)50, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_B, "B", (short)30, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_N, "N", (short)42, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_M, "M", (short)41, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_COMMA, ", (Comma)", (short)55, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_PERIOD, ". (Period)", (short)56, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_FORWARDSLASH, "/ (Forward Slash)", (short)76, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTSHIFT, "Right Shift", (short)60, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_1, "Numpad 1", (short)145, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_2, "Numpad 2", (short)146, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_3, "Numpad 3", (short)147, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ENTER, "Numpad Enter", (short)160, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_ALTERNATE, "Alternate", (short)57, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_SPACE, "Space", (short)62, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_CAPS, "Capslock", (short)115, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_0, "Numpad 0", (short)144, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_PERIOD, "Numpad . (Period)", (short)158, (short)1, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_NUMPAD_ENTER, "Numpad Enter", (short)160, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_KEYBOARDTOGGLE, "Toggle Keyboard", (short)-1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_KEYBOARDTOGGLE_SEL, "Toggle Keyboard", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_PERSIST),
		new VirtKeyDef(VKB_KEY_JOYLEFT, "Joystick Left", (short)-1, (short)1, (short)1, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYRIGHT, "Joystick Right", (short)-1, (short)1, (short)2, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYDOWN, "Joystick Down", (short)-1, (short)1, (short)4, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYUP, "Joystick Up", (short)-1, (short)1, (short)3, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYFIRE_OLD, "Joystick Fire Old", (short)-1, (short)0, (short)0, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_KEYBOARDZOOM, "Keyboard Zoom", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_KEYBOARDZOOM_SEL, "Keyboard Zoom", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_SCREENZOOM, "Screen Zoom", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_SCREENZOOM_SEL, "Screen Zoom", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_MOUSETOGGLE, "Toggle Mouse/Joystick", (short)-1, (short)1, (short)5, FLAG_CUSTOMKEY|FLAG_JOY),
		new VirtKeyDef(VKB_KEY_JOYTOGGLE, "Toggle Mouse/Joystick", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MOUSE),
		new VirtKeyDef(VKB_KEY_MOUSELB, "Left Mouse Button", (short)-1, (short)1, (short)0, FLAG_MOUSEBUTTON),
		new VirtKeyDef(VKB_KEY_MOUSERB, "Right Mouse Button", (short)-1, (short)1, (short)0, FLAG_MOUSEBUTTON),
		new VirtKeyDef(VKB_KEY_NORMALSPEED, "Toggle Turbo Speed", (short)-1, (short)0, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_TURBOSPEED, "Toggle Turbo Speed", (short)-1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_MAIN),
		new VirtKeyDef(VKB_KEY_SCREENPRESETS, "Cycle Screen Zoom Presets", (short)-1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_SCREEN2),
		new VirtKeyDef(VKB_KEY_KEYBOARDPRESETS, "Cycle Keyboard Zoom Presets", (short)-1, (short)1, (short)0, FLAG_CUSTOMKEY|FLAG_VKB),
		new VirtKeyDef(VKB_KEY_JOYFIRE, "Joystick Fire", (short)-1, (short)1, (short)0, FLAG_JOY),
		new VirtKeyDef(VKB_KEY_LEFTSHIFT_BUTTON, "Left Shift Button", (short)-1, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_RIGHTSHIFT_BUTTON, "Right Shift Button", (short)-1, (short)0, (short)0, FLAG_STKEY),
		new VirtKeyDef(VKB_KEY_TOGGLEUI, "Toggle UI", (short)-1, (short)1, (short)0, FLAG_CUSTOMKEY)
	};
}
