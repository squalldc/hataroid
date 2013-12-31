#include "main.h"

#include "gui-android.h"

int DlgAlert_Notice(const char *text)
{
	Debug_Printf("Hatari DlgAlert_Notice: '%s'", text);
	showGenericDialog(text, 1, 0);
	for (;;)
	{
		if (hasDialogResult()==1)
		{
			break;
		}
		else
		{
			usleep(500000); // 0.5 sec
		}
	}
	clearDialogResult();
	return 0;
}

int DlgAlert_Query(const char *text)
{
	Debug_Printf("Hatari DlgAlert_Query: '%s'", text);
	showGenericDialog(text, 0 , 1);
	int res = 0;
	for (;;)
	{
		if (hasDialogResult()==1)
		{
			res = getDialogResult();
			break;
		}
		else
		{
			usleep(500000); // 0.5 sec
		}
	}
	clearDialogResult();
	return res;
}

// dummy functions for unused calls on Android
char* SDLGui_FileSelect(const char *path_and_name, char **zip_path, bool bAllowNew)
{
	return 0;
}

bool Dialog_DoProperty(void)
{
	Debug_Printf("Hatari Dialog_DoProperty");

	clearEmuCommands();

	showOptionsDialog();

	for (;;)
	{
		if (hasEmuCommands())
		{
			break;
		}
		usleep(500000); // 0.5 sec
	}

	while (hasEmuCommands())
	{
		processEmuCommands();
		usleep(50000);
	}

	return 0;
}
