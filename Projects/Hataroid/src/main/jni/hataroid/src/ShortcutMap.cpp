#include <stdio.h>
#include <stdlib.h>

#include "VirtKBDefs.h"
#include "ShortcutMap.h"

const int ShortcutMap::kMaxKeys[kNumAnchors] = {
	6, // kAnchorTR
	4, // kAnchorBR
};

static const int kDefaultTRKeys[] = {
	VKB_KEY_TURBOSPEED,
	VKB_KEY_Y,
	VKB_KEY_N,
	VKB_KEY_1,
	VKB_KEY_2,
	-1,//VKB_KEY_RETURN,
};

static const int kDefaultBRKeys[] = {
	VKB_KEY_SPACE,
	VKB_KEY_LEFTSHIFT,
	VKB_KEY_ALTERNATE,
	-1,//VKB_KEY_CONTROL,
};

const int *ShortcutMap::kDefaultPreset[kNumAnchors] = {
	kDefaultTRKeys,
	kDefaultBRKeys,
};

ShortcutMap::ShortcutMap()
{
	for (int i = 0; i < kNumAnchors; ++i)
	{
		_shortcutMap[i] = new int [kMaxKeys[i]];
	}

	setDefault();
}

ShortcutMap::~ShortcutMap()
{
	for (int i = 0; i < kNumAnchors; ++i)
	{
		delete [] _shortcutMap[i];
		_shortcutMap[i] = 0;
	}
}

void ShortcutMap::clear()
{
	for (int i = 0; i < kNumAnchors; ++i)
	{
		for (int k = 0; k < kMaxKeys[i]; ++k)
		{
			_shortcutMap[i][k] = -1;
		}
	}
}

void ShortcutMap::setDefault()
{
	for (int i = 0; i < kNumAnchors; ++i)
	{
		for (int k = 0; k < kMaxKeys[i]; ++k)
		{
			int vkId = kDefaultPreset[i][k];
			if (vkId == VKB_KEY_TURBOSPEED)
			{
				vkId = VKB_KEY_NORMALSPEED;
			}
			_shortcutMap[i][k] = vkId;
		}
	}
}

void ShortcutMap::setFromPrefString(const char *pref)
{
	clear();

	const char *curPref = pref;
	for (int i = 0; i < kNumAnchors; ++i)
	{
		for (int k = 0; k < kMaxKeys[i]; ++k)
		{
			int vkId = -1;
			curPref = _parseKey(curPref, &vkId);

			if (vkId == VKB_KEY_TURBOSPEED)
			{
				vkId = VKB_KEY_NORMALSPEED;
			}

			_shortcutMap[i][k] = vkId;
		}
	}
}

const char* ShortcutMap::_parseKey(const char* curPref, int *vkeyId)
{
	*vkeyId = -1;

	if (curPref != 0)
	{
		int val = 0;

		int i = 0;
		int c = 0;

		c = curPref[i];

		// end of str
		if (c == 0)
		{
			return 0;
		}

		// check neg
		if (c == '-')
		{
			// don't care about the num, skip to end
			while (c != ',' && c != 0) { c = curPref[++i]; }
			return &curPref[(c==',')?(i+1):i];
		}

		while (c != ',' && c != 0)
		{
			if (c < '0' || c > '9')
			{
				// invalid num, skip to end and return
				while (c != ',' && c != 0) { c = curPref[++i]; }
				return &curPref[(c==',')?(i+1):i];
			}

			val *= 10;
			val += c - '0';

			c = curPref[++i];
		}

		*vkeyId = val;
		return &curPref[(c==',')?(i+1):i];
	}

	return 0;
}
