#ifndef __VirtKBDefs_h
#define __VirtKBDefs_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct
{
	int		id;
	char *		desc;
	char *		name;
	short		androidKeycode;
	short		config;
	short		scut;
	short		sort;
	unsigned char		scancode;
	short		v[8];
	unsigned int		flags;
	short		qv[4];
	short		numRefs;
	const short*		pRefs;

} VirtKeyDef;

extern const VirtKeyDef g_vkbKeyDefs[];
extern const int g_vkbKeyDefsSize;

enum
{
	VKB_KEY_F1		= 0,
	VKB_KEY_F2		= 1,
	VKB_KEY_F3		= 2,
	VKB_KEY_F4		= 3,
	VKB_KEY_F5		= 4,
	VKB_KEY_F6		= 5,
	VKB_KEY_F7		= 6,
	VKB_KEY_F8		= 7,
	VKB_KEY_F9		= 8,
	VKB_KEY_F10		= 9,
	VKB_KEY_ESC		= 10,
	VKB_KEY_1		= 11,
	VKB_KEY_2		= 12,
	VKB_KEY_3		= 13,
	VKB_KEY_4		= 14,
	VKB_KEY_5		= 15,
	VKB_KEY_6		= 16,
	VKB_KEY_7		= 17,
	VKB_KEY_8		= 18,
	VKB_KEY_9		= 19,
	VKB_KEY_0		= 20,
	VKB_KEY_SUBTRACT		= 21,
	VKB_KEY_EQUAL		= 22,
	VKB_KEY_BACKQUOTE		= 23,
	VKB_KEY_BACKSPACE		= 24,
	VKB_KEY_HELP		= 25,
	VKB_KEY_UNDO		= 26,
	VKB_KEY_NUMPAD_LEFTPAREN		= 27,
	VKB_KEY_NUMPAD_RIGHTPAREN		= 28,
	VKB_KEY_NUMPAD_DIVIDE		= 29,
	VKB_KEY_NUMPAD_MULTIPLY		= 30,
	VKB_KEY_TAB		= 31,
	VKB_KEY_Q		= 32,
	VKB_KEY_W		= 33,
	VKB_KEY_E		= 34,
	VKB_KEY_R		= 35,
	VKB_KEY_T		= 36,
	VKB_KEY_Y		= 37,
	VKB_KEY_U		= 38,
	VKB_KEY_I		= 39,
	VKB_KEY_O		= 40,
	VKB_KEY_P		= 41,
	VKB_KEY_LEFTBRACKET		= 42,
	VKB_KEY_RIGHTBRACKET		= 43,
	VKB_KEY_RETURN		= 44,
	VKB_KEY_DELETE		= 45,
	VKB_KEY_INSERT		= 46,
	VKB_KEY_UPARROW		= 47,
	VKB_KEY_HOME		= 48,
	VKB_KEY_NUMPAD_7		= 49,
	VKB_KEY_NUMPAD_8		= 50,
	VKB_KEY_NUMPAD_9		= 51,
	VKB_KEY_NUMPAD_SUBTRACT		= 52,
	VKB_KEY_CONTROL		= 53,
	VKB_KEY_A		= 54,
	VKB_KEY_S		= 55,
	VKB_KEY_D		= 56,
	VKB_KEY_F		= 57,
	VKB_KEY_G		= 58,
	VKB_KEY_H		= 59,
	VKB_KEY_J		= 60,
	VKB_KEY_K		= 61,
	VKB_KEY_L		= 62,
	VKB_KEY_SEMICOLON		= 63,
	VKB_KEY_APOSTROPHE		= 64,
	VKB_KEY_TILDA		= 66,
	VKB_KEY_LEFTARROW		= 67,
	VKB_KEY_DOWNARROW		= 68,
	VKB_KEY_RIGHTARROW		= 69,
	VKB_KEY_NUMPAD_4		= 70,
	VKB_KEY_NUMPAD_5		= 71,
	VKB_KEY_NUMPAD_6		= 72,
	VKB_KEY_NUMPAD_ADD		= 73,
	VKB_KEY_LEFTSHIFT		= 74,
	VKB_KEY_BACKSLASH		= 75,
	VKB_KEY_Z		= 76,
	VKB_KEY_X		= 77,
	VKB_KEY_C		= 78,
	VKB_KEY_V		= 79,
	VKB_KEY_B		= 80,
	VKB_KEY_N		= 81,
	VKB_KEY_M		= 82,
	VKB_KEY_COMMA		= 83,
	VKB_KEY_PERIOD		= 84,
	VKB_KEY_FORWARDSLASH		= 85,
	VKB_KEY_RIGHTSHIFT		= 86,
	VKB_KEY_NUMPAD_1		= 87,
	VKB_KEY_NUMPAD_2		= 88,
	VKB_KEY_NUMPAD_3		= 89,
	VKB_KEY_NUMPAD_ENTER		= 90,
	VKB_KEY_ALTERNATE		= 91,
	VKB_KEY_SPACE		= 92,
	VKB_KEY_CAPS		= 93,
	VKB_KEY_NUMPAD_0		= 94,
	VKB_KEY_NUMPAD_PERIOD		= 95,
	VKB_KEY_KEYBOARDTOGGLE		= 97,
	VKB_KEY_KEYBOARDTOGGLE_SEL		= 98,
	VKB_KEY_JOYLEFT		= 99,
	VKB_KEY_JOYRIGHT		= 100,
	VKB_KEY_JOYDOWN		= 101,
	VKB_KEY_JOYUP		= 102,
	VKB_KEY_JOYFIRE_OLD		= 103,
	VKB_KEY_KEYBOARDZOOM		= 104,
	VKB_KEY_KEYBOARDZOOM_SEL		= 105,
	VKB_KEY_SCREENZOOM		= 106,
	VKB_KEY_SCREENZOOM_SEL		= 107,
	VKB_KEY_MOUSETOGGLE		= 108,
	VKB_KEY_JOYTOGGLE		= 109,
	VKB_KEY_MOUSELB		= 110,
	VKB_KEY_MOUSERB		= 111,
	VKB_KEY_NORMALSPEED		= 112,
	VKB_KEY_TURBOSPEED		= 113,
	VKB_KEY_SCREENPRESETS		= 114,
	VKB_KEY_KEYBOARDPRESETS		= 115,
	VKB_KEY_JOYFIRE		= 116,
	VKB_KEY_LEFTSHIFT_BUTTON		= 117,
	VKB_KEY_RIGHTSHIFT_BUTTON		= 118,
	VKB_KEY_TOGGLEUI		= 119,
	VKB_KEY_AUTOFIRE		= 120,
	VKB_KEY_QUICKSAVESTATE		= 121,
	VKB_KEY_QUICKLOADSTATE		= 122,
	VKB_KEY_NumOf		= 123
};

enum
{
	FLAG_POLY		= (1<<0),
	FLAG_STFNKEY		= (1<<1),
	FLAG_STKEY		= (1<<2),
	FLAG_PERSIST		= (1<<3),
	FLAG_CUSTOMKEY		= (1<<4),
	FLAG_JOY		= (1<<5),
	FLAG_VKB		= (1<<6),
	FLAG_SCREEN2		= (1<<7),
	FLAG_SCREEN		= (1<<8),
	FLAG_MOUSE		= (1<<9),
	FLAG_MOUSEBUTTON		= (1<<10),
	FLAG_MAIN		= (1<<11)
};

extern const int g_vkbTexFullW;
extern const int g_vkbTexFullH;
extern const int g_vkbTexKbW;
extern const int g_vkbTexKbH;

typedef struct
{
	int		minY;
	int		maxY;
	int		minID;
	int		maxID;
} RowSearch;

extern const RowSearch g_vkbRowSearch[];
extern const int g_vkbRowSearchSize;

#ifdef __cplusplus
};  /* end of extern "C" */
#endif /* __cplusplus */

#endif //__VirtKBDefs_h
