//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include <shlwapi.h>
#include "DockingFeature/GoToLineDlg.h"
#include "resource.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern bool doCloseTag;

extern DemoDlg _goToLine;
HINSTANCE g_inst;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID /*lpReserved*/)
{
	try {
		switch (reasonForCall)
		{
			case DLL_PROCESS_ATTACH:
				pluginInit(hModule);
				g_inst = HINSTANCE(hModule);
				break;

			case DLL_PROCESS_DETACH:
				pluginCleanUp();
				break;

			case DLL_THREAD_ATTACH:
				break;

			case DLL_THREAD_DETACH:
				break;
		}
	}
	catch (...) { return FALSE; }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}


extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code) 
	{

		case NPPN_TBMODIFICATION:
		{
			toolbarIconsWithDarkMode tbIcons;
			tbIcons.hToolbarBmp = ::LoadBitmap(g_inst, MAKEINTRESOURCE(IDR_SMILEY_BMP));
			tbIcons.hToolbarIcon = ::LoadIcon(g_inst, MAKEINTRESOURCE(IDI_SMILEY_ICON));
			tbIcons.hToolbarIconDarkMode = ::LoadIcon(g_inst, MAKEINTRESOURCE(IDI_SMILEY_DM_ICON));
			::SendMessage(nppData._nppHandle, NPPM_ADDTOOLBARICON_FORDARKMODE, funcItem[1]._cmdID, (LPARAM)&tbIcons);
		}
		break;

		case NPPN_SHUTDOWN:
		{
			commandMenuCleanUp();
		}
		break;

		case SCN_CHARADDED:
		{
			LangType docType;
			::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, 0, (LPARAM)&docType);
			bool isDocTypeHTML = (docType == L_HTML || docType == L_XML || docType == L_PHP);
			if (doCloseTag && isDocTypeHTML)
			{
				if (notifyCode->ch == '>')
				{
					char buf[512];
					int currentEdit;
					::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
					HWND hCurrentEditView = (currentEdit == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
					int currentPos = int(::SendMessage(hCurrentEditView, SCI_GETCURRENTPOS, 0, 0));
					int beginPos = currentPos - (sizeof(buf) - 1);
					int startPos = (beginPos > 0)?beginPos:0;
					int size = currentPos - startPos;
					char insertString[516] = "</";

					if (size >= 3) 
					{
						struct Sci_TextRangeFull tr = {{startPos, currentPos}, buf};

						::SendMessage(hCurrentEditView, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

						if (buf[size-2] != '/') 
						{

							const char *pBegin = &buf[0];
							const char *pCur = &buf[size - 2];
							int  insertStringSize = 2;

							for (; pCur > pBegin && *pCur != '<' && *pCur != '>' ;)
								pCur--;
								

							if (*pCur == '<')
							{
								pCur++;
								
								while (StrChrA(":_-.", *pCur) || IsCharAlphaNumeric(*pCur))
								{
									insertString[insertStringSize++] = *pCur;
									pCur++;
								}
							}

							insertString[insertStringSize++] = '>';
							insertString[insertStringSize] = '\0';

							if (insertStringSize > 3)
							{				
								::SendMessage(hCurrentEditView, SCI_BEGINUNDOACTION, 0, 0);
								::SendMessage(hCurrentEditView, SCI_REPLACESEL, 0, (LPARAM)insertString);
								::SendMessage(hCurrentEditView, SCI_SETSEL, currentPos, currentPos);
								::SendMessage(hCurrentEditView, SCI_ENDUNDOACTION, 0, 0);
							}
						}
					}	
				}
			}
		}
		break;

		default:
			return;
	}
}


// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// https://github.com/notepad-plus-plus/notepad-plus-plus/issues
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
