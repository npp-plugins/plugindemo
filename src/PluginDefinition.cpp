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
#include "menuCmdID.h"

//
// put the headers you need here
//
#include <stdlib.h>
#include <time.h>
#include <shlwapi.h>
#include "DockingFeature/GoToLineDlg.h"

const wchar_t sectionName[] = L"Insert Extesion";
const wchar_t keyName[] = L"doCloseTag";
const wchar_t configFileName[] = L"pluginDemo.ini";

DemoDlg _goToLine;

#ifdef UNICODE 
	#define generic_itoa _itow
#else
	#define generic_itoa itoa
#endif

FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;


wchar_t iniFilePath[MAX_PATH];
bool doCloseTag = false;

#define DOCKABLE_DEMO_INDEX 16

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE hModule)
{
	// Initialize dockable demo dialog
	_goToLine.init((HINSTANCE)hModule, NULL);
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
	::WritePrivateProfileString(sectionName, keyName, doCloseTag ? L"1" : L"0", iniFilePath);
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{
	//
	// Firstly we get the parameters from your plugin config file (if any)
	//

	// get path of plugin configuration
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)iniFilePath);

	// if config path doesn't exist, we create it
	if (PathFileExists(iniFilePath) == FALSE)
	{
		::CreateDirectory(iniFilePath, NULL);
	}

	// make your plugin config file full file path name
	PathAppend(iniFilePath, configFileName);

	// get the parameter value from plugin config
	doCloseTag = (::GetPrivateProfileInt(sectionName, keyName, 0, iniFilePath) != 0);


    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            wchar_t *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    setCommand(0, L"Hello Notepad++", hello, NULL, false);
    setCommand(1, L"Hello (with FX)", helloFX, NULL, false);
	setCommand(2, L"What is Notepad++?", WhatIsNpp, NULL, false);

	// Here you insert a separator
	setCommand(3, L"---", NULL, NULL, false);

	// Shortcut :
	// Following code makes the first command
	// bind to the shortcut Alt-F
	ShortcutKey *shKey = new ShortcutKey;
	shKey->_isAlt = true;
	shKey->_isCtrl = false;
	shKey->_isShift = false;
	shKey->_key = 0x46; //VK_F

	setCommand(4, L"Current Full Path", insertCurrentFullPath, shKey, false);
	setCommand(5, L"Current File Name", insertCurrentFileName, NULL, false);
	setCommand(6, L"Current Directory", insertCurrentDirectory, NULL, false);
	setCommand(7, L"Date & Time - short format", insertShortDateTime, NULL, false);
	setCommand(8, L"Date & Time - long format", insertLongDateTime, NULL, false);

	ShortcutKey *pShKey = new ShortcutKey;
	pShKey->_isAlt = true;
	pShKey->_isCtrl = false;
	pShKey->_isShift = false;
	pShKey->_key = 0x51; //VK_Q
	setCommand(9, L"Close HTML/XML tag automatically", insertHtmlCloseTag, pShKey, doCloseTag);
	
	setCommand(10, L"---", NULL, NULL, false);

	setCommand(11, L"Get File Names Demo", getFileNamesDemo, NULL, false);
	setCommand(12, L"Get Session File Names Demo", getSessionFileNamesDemo, NULL, false);
	setCommand(13, L"Save Current Session Demo", saveCurrentSessionDemo, NULL, false);
	setCommand(14, L"Get Command Shortcut Demo", getCommandShortcutDemo, NULL, false);

	setCommand(15, L"---", NULL, NULL, false);

	setCommand(DOCKABLE_DEMO_INDEX, L"Dockable Dialog Demo", DockableDlgDemo, NULL, false);

	setCommand(17, L"---", NULL, NULL, false);

	setCommand(18, L"Plugin Communication Guide", goToPluginCommunicationGuide, NULL, false);
	setCommand(19, L"Get Plugin Demo Source Code", goToPluginDemoRepo, NULL, false);
}


//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
	delete funcItem[4]._pShKey;
	delete funcItem[9]._pShKey;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//
void hello()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");
}

static DWORD WINAPI threadZoomer(void *)
{
    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return FALSE;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
    
    int currentZoomLevel = (int)::SendMessage(curScintilla, SCI_GETZOOM, 0, 0);

    int i = currentZoomLevel;
	for (int j = 0 ; j < 4 ; j++)
	{	
		for ( ; i >= -10 ; i--)
		{
			::SendMessage(curScintilla, SCI_SETZOOM, i, 0);
			Sleep(30);
		}
		Sleep(100);
		for ( ; i <= 20 ; i++)
		{
			Sleep(30);
			::SendMessage(curScintilla, SCI_SETZOOM, i, 0);
		}
		Sleep(100);
	}

    Sleep(100);
    for ( ; i >= currentZoomLevel ; i--)
    {
        Sleep(30);
        ::SendMessage(curScintilla, SCI_SETZOOM, i, 0);
    }
    return TRUE;
};

void helloFX()
{
    hello();
    HANDLE hThread = ::CreateThread(NULL, 0, threadZoomer, 0, 0, NULL);
    ::CloseHandle(hThread);
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, wchar_t *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

static DWORD WINAPI threadTextPlayer(void *text2display)
{
	// Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return FALSE;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
    srand((unsigned int)time(NULL));
    int rangeMin = 0;
    int rangeMax = 250;
    for (size_t i = 0 ; i < strlen((char *)text2display) ; i++)
    {
		char charToShow[2] = {((char *)text2display)[i], '\0'};

        int ranNum = rangeMin + (int)((double)rand() / ((double)RAND_MAX + 1) * rangeMax);
        Sleep(ranNum + 30);

        ::SendMessage(curScintilla, SCI_APPENDTEXT, 1, (LPARAM)charToShow);
		::SendMessage(curScintilla, SCI_GOTOPOS, ::SendMessage(curScintilla, SCI_GETLENGTH, 0, 0), 0);
    }

    return TRUE;
};

void WhatIsNpp()
{
	char *text2display = "Notepad++ is a free (as in \"free speech\" and also as in \"free beer\") source code editor and Notepad replacement that supports several languages.\n\
		Running in the MS Windows environment, its use is governed by GPL License.\n\
\n\
Based on a powerful editing component Scintilla, Notepad++ is written in C++ and uses pure Win32 API and STL which ensures a higher execution speed and smaller program size.\n\
By optimizing as many routines as possible without losing user friendliness, Notepad++ is trying to reduce the world carbon dioxide emissions. When using less CPU power, the PC can throttle down and reduce power consumption, resulting in a greener environment.";
    HANDLE hThread = ::CreateThread(NULL, 0, threadTextPlayer, text2display, 0, NULL);
    ::CloseHandle(hThread);
}


void insertCurrentPath(int which)
{
	int msg = NPPM_GETFULLCURRENTPATH;
	if (which == FILE_NAME)
		msg = NPPM_GETFILENAME;
	else if (which == CURRENT_DIRECTORY)
		msg = NPPM_GETCURRENTDIRECTORY;

	int currentEdit;
	wchar_t path[MAX_PATH];
	
	// A message to Notepad++ to get a multibyte string (if ANSI mode) or a wide char string (if Unicode mode)
	::SendMessage(nppData._nppHandle, msg, 0, (LPARAM)path);

	//
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND curScint = (currentEdit == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
#ifdef UNICODE
	int encoding = (int)::SendMessage(curScint, SCI_GETCODEPAGE, 0, 0);
	char pathA[MAX_PATH];
	WideCharToMultiByte(encoding, 0, path, -1, pathA, MAX_PATH, NULL, NULL);
	::SendMessage(curScint, SCI_REPLACESEL, 0, (LPARAM)pathA);
#else
	::SendMessage(curScint, SCI_REPLACESEL, 0, (LPARAM)path);
#endif
}



void insertCurrentFullPath()
{
	insertCurrentPath(FULL_CURRENT_PATH);
}
void insertCurrentFileName()
{
	insertCurrentPath(FILE_NAME);
}
void insertCurrentDirectory()
{
	insertCurrentPath(CURRENT_DIRECTORY);
}

const bool shortDate = true;
const bool longDate = false;

void insertShortDateTime()
{
	insertDateTime(shortDate);
}

void insertLongDateTime()
{
	insertDateTime(longDate);
}

void insertDateTime(bool format)
{
	wchar_t date[128];
    wchar_t time[128];
    wchar_t dateTime[256];

    SYSTEMTIME st;
	::GetLocalTime(&st);
	::GetDateFormat(LOCALE_USER_DEFAULT, (format == shortDate)?DATE_SHORTDATE:DATE_LONGDATE, &st, NULL, date, 128);
	::GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, time, 128);

    wsprintf(dateTime, L"%s %s", time, date);

	int currentEdit;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
	HWND curScint = (currentEdit == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	int encoding = (int)::SendMessage(curScint, SCI_GETCODEPAGE, 0, 0);
	char dateTimeA[MAX_PATH];
	WideCharToMultiByte(encoding, 0, dateTime, -1, dateTimeA, MAX_PATH, NULL, NULL);
	::SendMessage(curScint, SCI_REPLACESEL, 0, (LPARAM)dateTimeA);
}

void insertHtmlCloseTag()
{
	doCloseTag = !doCloseTag;
	::CheckMenuItem(::GetMenu(nppData._nppHandle), funcItem[9]._cmdID, MF_BYCOMMAND | (doCloseTag?MF_CHECKED:MF_UNCHECKED));
}

void getFileNamesDemo()
{
	int nbMainViewFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, PRIMARY_VIEW);
	int nbSubViewFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, SECOND_VIEW);
	int nbFile = nbMainViewFile + nbSubViewFile;
	wchar_t toto[10];
	::MessageBox(nppData._nppHandle, generic_itoa(nbFile, toto, 10), L"nb opened files", MB_OK);
	
	wchar_t **fileNames = (wchar_t **)new wchar_t*[nbFile];

	int i = 0;
	for (; i < nbMainViewFile; )
	{
		LRESULT bufferId = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, i, MAIN_VIEW);
		LRESULT len = ::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferId, (WPARAM)nullptr);
		fileNames[i] = new wchar_t[len + 1];
		::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferId, (WPARAM)fileNames[i]);
		++i;
	}

	
	for (int j = 0; j < nbSubViewFile; ++j)
	{
		LRESULT bufferId = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, j, SUB_VIEW);
		LRESULT len = ::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferId, (WPARAM)nullptr);
		fileNames[i] = new wchar_t[len + 1];
		::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferId, (WPARAM)fileNames[i]);
		++i;
	}

	for (int k = 0 ; k < nbFile ; k++)
		::MessageBox(nppData._nppHandle, fileNames[k], L"", MB_OK);

	for (int k = 0 ; k < nbFile ; k++)
	{
		delete [] fileNames[k];
	}
	delete [] fileNames;
}

void getSessionFileNamesDemo()
{
	const wchar_t *sessionFullPath = L"c:\\test.session";
	int nbFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBSESSIONFILES, 0, (LPARAM)sessionFullPath);

	if (!nbFile)
	{
		::MessageBox(nppData._nppHandle, L"Please modify \"sessionFullPath\" in \"NppInsertPlugin.cpp\" in order to point to a valide session file", L"Error :", MB_OK);
		return;
	}
	wchar_t toto[10];
	::MessageBox(nppData._nppHandle, generic_itoa(nbFile, toto, 10), L"nb session files", MB_OK);
	
	wchar_t **fileNames = (wchar_t **)new wchar_t*[nbFile];
	for (int i = 0 ; i < nbFile ; i++)
	{
		fileNames[i] = new wchar_t[MAX_PATH];
	}

	if (::SendMessage(nppData._nppHandle, NPPM_GETSESSIONFILES, (WPARAM)fileNames, (LPARAM)sessionFullPath))
	{
		for (int i = 0 ; i < nbFile ; i++)
			::MessageBox(nppData._nppHandle, fileNames[i], L"session file name :", MB_OK);
	}

	for (int i = 0 ; i < nbFile ; i++)
	{
		delete [] fileNames[i];
	}
	delete [] fileNames;
}

void saveCurrentSessionDemo()
{
	wchar_t *sessionPath = (wchar_t *)::SendMessage(nppData._nppHandle, NPPM_SAVECURRENTSESSION, 0, 0);
	if (sessionPath)
		::MessageBox(nppData._nppHandle, sessionPath, L"Saved Session File :", MB_OK);
}

void getCommandShortcutDemo()
{
	ShortcutKey sk;

	// Grt shortcut of current full file path shortcut
	::SendMessage(nppData._nppHandle, NPPM_GETSHORTCUTBYCMDID, funcItem[4]._cmdID, reinterpret_cast<LPARAM>(&sk));

	std::string msg;
	msg = "KEY: ";
	msg += static_cast<char>(sk._key);
	msg += " Crtl: ";
	msg += sk._isCtrl ? "Yes" : "No";
	msg += " Alt: ";
	msg += sk._isAlt ? "Yes" : "No";
	msg += " Shift: ";
	msg += sk._isShift ? "Yes" : "No";


	::MessageBoxA(nppData._nppHandle, msg.c_str(), "Shorcut", MB_OK);
}

// Dockable Dialog Demo
// 
// This demonstration shows you how to do a dockable dialog.
// You can create your own non dockable dialog - in this case you don't nedd this demonstration.
// You have to create your dialog by inherented DockingDlgInterface class in order to make your dialog dockable
// - please see DemoDlg.h and DemoDlg.cpp to have more informations.
void DockableDlgDemo()
{
	_goToLine.setParent(nppData._nppHandle);
	tTbData	data = {0};

	if (!_goToLine.isCreated())
	{
		_goToLine.create(&data);

		// define the default docking behaviour
		data.uMask = DWS_DF_CONT_RIGHT;

		data.pszModuleName = _goToLine.getPluginFileName();

		// the dlgDlg should be the index of funcItem where the current function pointer is
		// in this case is DOCKABLE_DEMO_INDEX
		data.dlgID = DOCKABLE_DEMO_INDEX;
		::SendMessage(nppData._nppHandle, NPPM_DMMREGASDCKDLG, 0, (LPARAM)&data);
	}
	_goToLine.display();
}

void goToPluginCommunicationGuide()
{
	::ShellExecute(NULL, L"open", L"https://npp-user-manual.org/docs/plugin-communication/", NULL, NULL, SW_SHOWNORMAL);
}

void goToPluginDemoRepo()
{
	::ShellExecute(NULL, L"open", L"https://github.com/npp-plugins/plugindemo", NULL, NULL, SW_SHOWNORMAL);
}
