#ifndef __SHORTCUTMAP_H__
#define __SHORTCUTMAP_H__

class ShortcutMap
{
	public:
		enum
		{
			kAnchorTR = 0,
			kAnchorBR,
			kNumAnchors
		};

		// KEEP IN SYNC WITH JAVA SIDE
		static const int kMaxKeys[kNumAnchors];
		static const int *kDefaultPreset[kNumAnchors];

	private:
		int *_shortcutMap[kNumAnchors];

	public:
		ShortcutMap();
		~ShortcutMap();

		void clear();
		void setDefault();

		const int *getCurAnchorList(int anchor) { return _shortcutMap[anchor]; }

		void setFromPrefString(const char *pref);

	private:
		const char* _parseKey(const char* curPref, int *vkeyId);
};

#endif //__SHORTCUTMAP_H__
