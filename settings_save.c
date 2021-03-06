/****************************************************************************/
/*                                                                          */
/*   metapad 3.6                                                            */
/*                                                                          */
/*   Copyright (C) 1999-2011 Alexander Davidson                             */
/*                                                                          */
/*   This program is free software: you can redistribute it and/or modify   */
/*   it under the terms of the GNU General Public License as published by   */
/*   the Free Software Foundation, either version 3 of the License, or      */
/*   (at your option) any later version.                                    */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*   GNU General Public License for more details.                           */
/*                                                                          */
/*   You should have received a copy of the GNU General Public License      */
/*   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
/*                                                                          */
/****************************************************************************/

/**
 * @file settings_save.c
 * @brief Settings saving functions.
 */

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <tchar.h>

#ifdef BUILD_METAPAD_UNICODE
#include <wchar.h>
#endif

#include "include/hexencoder.h"
#include "include/tmp_protos.h"
#include "include/consts.h"
#include "include/strings.h"
#include "include/typedefs.h"

extern HANDLE globalHeap;
extern TCHAR szMetapadIni[MAXFN];
extern option_struct options;
extern BOOL bWordWrap;
extern BOOL bPrimaryFont;
extern BOOL bSmartSelect;
extern BOOL bShowStatus;
extern BOOL bShowToolbar;
extern BOOL bAlwaysOnTop;
extern BOOL bTransparent;
extern BOOL bCloseAfterFind;
extern BOOL bNoFindHidden;
extern BOOL g_bIniMode;
extern TCHAR FindArray[NUMFINDS][MAXFIND];
extern TCHAR ReplaceArray[NUMFINDS][MAXFIND];
extern TCHAR szDir[MAXFN];

#ifdef USE_RICH_EDIT
extern BOOL bHyperlinks;
#endif

/**
 * Save an option.
 *
 * @param[in] hKey A handle to an open registry key. If NULL, SaveOption stores the option in an ini file.
 * @param[in] name Name of the value to be set.
 * @param[in] dwType The type of lpData.
 * @param[in] lpData The data to be stored.
 * @param[in] cbData The size of lpData, in bytes.
 * @return TRUE if the value was stored successfully, FALSE otherwise.
 */
BOOL SaveOption(HKEY hKey, LPCTSTR name, DWORD dwType, CONST LPBYTE lpData, DWORD cbData)
{
	if (hKey) {
		if (RegSetValueEx(hKey, name, 0, dwType, lpData, cbData) != ERROR_SUCCESS){
			ReportLastError();
			return FALSE;
		} else return TRUE;
	}
	else {
		BOOLEAN writeSucceeded = TRUE;
		switch (dwType) {
			case REG_DWORD: {
				TCHAR val[10];
				int int32 = 0;
				int32 = (int32 << 8) + lpData[3];
				int32 = (int32 << 8) + lpData[2];
				int32 = (int32 << 8) + lpData[1];
				int32 = (int32 << 8) + lpData[0];
				wsprintf(val, _T("%d"), int32);
				writeSucceeded = WritePrivateProfileString(_T("Options"), name, val, szMetapadIni);
				break;
			}
			case REG_SZ:
				writeSucceeded = WritePrivateProfileString(_T("Options"), name, (TCHAR*)lpData, szMetapadIni);
				break;
			case REG_BINARY: {
				TCHAR *szBuffer = (LPTSTR)HeapAlloc(globalHeap, HEAP_ZERO_MEMORY, 2 * (cbData + 1) * sizeof(TCHAR));
				BinToHex( lpData, cbData, szBuffer );
				writeSucceeded = WritePrivateProfileString(_T("Options"), name, (TCHAR*)szBuffer, szMetapadIni);
				HeapFree(globalHeap, 0, szBuffer);
				break;
			}
		}
		return writeSucceeded;
	}
}

/**
 * Save all of metapad's options.
 */
void SaveOptions(void)
{
	HKEY key = NULL;
	BOOL writeSucceeded = TRUE;

	if (!g_bIniMode) {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
			ReportLastError();
			return;
		}
	}

	writeSucceeded &= SaveOption(key, _T("bSystemColours"), REG_DWORD, (LPBYTE)&options.bSystemColours, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSystemColours2"), REG_DWORD, (LPBYTE)&options.bSystemColours2, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoSmartHome"), REG_DWORD, (LPBYTE)&options.bNoSmartHome, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoAutoSaveExt"), REG_DWORD, (LPBYTE)&options.bNoAutoSaveExt, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bContextCursor"), REG_DWORD, (LPBYTE)&options.bContextCursor, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bCurrentFindFont"), REG_DWORD, (LPBYTE)&options.bCurrentFindFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bPrintWithSecondaryFont"), REG_DWORD, (LPBYTE)&options.bPrintWithSecondaryFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoSaveHistory"), REG_DWORD, (LPBYTE)&options.bNoSaveHistory, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoFindAutoSelect"), REG_DWORD, (LPBYTE)&options.bNoFindAutoSelect, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bHideGotoOffset"), REG_DWORD, (LPBYTE)&options.bHideGotoOffset, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bRecentOnOwn"), REG_DWORD, (LPBYTE)&options.bRecentOnOwn, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bDontInsertTime"), REG_DWORD, (LPBYTE)&options.bDontInsertTime, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoWarningPrompt"), REG_DWORD, (LPBYTE)&options.bNoWarningPrompt, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bUnFlatToolbar"), REG_DWORD, (LPBYTE)&options.bUnFlatToolbar, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bReadOnlyMenu"), REG_DWORD, (LPBYTE)&options.bReadOnlyMenu, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bStickyWindow"), REG_DWORD, (LPBYTE)&options.bStickyWindow, sizeof(BOOL));
//	writeSucceeded &= SaveOption(key, _T("nStatusFontWidth"), REG_DWORD, (LPBYTE)&options.nStatusFontWidth, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nSelectionMarginWidth"), REG_DWORD, (LPBYTE)&options.nSelectionMarginWidth, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nMaxMRU"), REG_DWORD, (LPBYTE)&options.nMaxMRU, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nFormatIndex"), REG_DWORD, (LPBYTE)&options.nFormatIndex, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nTransparentPct"), REG_DWORD, (LPBYTE)&options.nTransparentPct, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("bNoCaptionDir"), REG_DWORD, (LPBYTE)&options.bNoCaptionDir, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bAutoIndent"), REG_DWORD, (LPBYTE)&options.bAutoIndent, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bInsertSpaces"), REG_DWORD, (LPBYTE)&options.bInsertSpaces, sizeof(BOOL));
 	writeSucceeded &= SaveOption(key, _T("bFindAutoWrap"), REG_DWORD, (LPBYTE)&options.bFindAutoWrap, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bQuickExit"), REG_DWORD, (LPBYTE)&options.bQuickExit, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveWindowPlacement"), REG_DWORD, (LPBYTE)&options.bSaveWindowPlacement, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveMenuSettings"), REG_DWORD, (LPBYTE)&options.bSaveMenuSettings, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveDirectory"), REG_DWORD, (LPBYTE)&options.bSaveDirectory, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bLaunchClose"), REG_DWORD, (LPBYTE)&options.bLaunchClose, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoFaves"), REG_DWORD, (LPBYTE)&options.bNoFaves, sizeof(BOOL));
#ifndef USE_RICH_EDIT
	writeSucceeded &= SaveOption(key, _T("bDefaultPrintFont"), REG_DWORD, (LPBYTE)&options.bDefaultPrintFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bAlwaysLaunch"), REG_DWORD, (LPBYTE)&options.bAlwaysLaunch, sizeof(BOOL));
#else
	writeSucceeded &= SaveOption(key, _T("bLinkDoubleClick"), REG_DWORD, (LPBYTE)&options.bLinkDoubleClick, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bHideScrollbars"), REG_DWORD, (LPBYTE)&options.bHideScrollbars, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSuppressUndoBufferPrompt"), REG_DWORD, (LPBYTE)&options.bSuppressUndoBufferPrompt, sizeof(BOOL));
#endif
	writeSucceeded &= SaveOption(key, _T("nLaunchSave"), REG_DWORD, (LPBYTE)&options.nLaunchSave, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nTabStops"), REG_DWORD, (LPBYTE)&options.nTabStops, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nPrimaryFont"), REG_DWORD, (LPBYTE)&options.nPrimaryFont, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nSecondaryFont"), REG_DWORD, (LPBYTE)&options.nSecondaryFont, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("PrimaryFont"), REG_BINARY, (LPBYTE)&options.PrimaryFont, sizeof(LOGFONT));
	writeSucceeded &= SaveOption(key, _T("SecondaryFont"), REG_BINARY, (LPBYTE)&options.SecondaryFont, sizeof(LOGFONT));
	writeSucceeded &= SaveOption(key, _T("szBrowser"), REG_SZ, (LPBYTE)&options.szBrowser, sizeof(options.szBrowser));
	writeSucceeded &= SaveOption(key, _T("szArgs"), REG_SZ, (LPBYTE)&options.szArgs, sizeof(options.szArgs));
	writeSucceeded &= SaveOption(key, _T("szBrowser2"), REG_SZ, (LPBYTE)&options.szBrowser2, sizeof(options.szBrowser2));
	writeSucceeded &= SaveOption(key, _T("szArgs2"), REG_SZ, (LPBYTE)&options.szArgs2, sizeof(options.szArgs2));
	writeSucceeded &= SaveOption(key, _T("szQuote"), REG_BINARY, (LPBYTE)&options.szQuote, sizeof(options.szQuote));
	writeSucceeded &= SaveOption(key, _T("szLangPlugin"), REG_SZ, (LPBYTE)&options.szLangPlugin, sizeof(options.szLangPlugin));
	writeSucceeded &= SaveOption(key, _T("szFavDir"), REG_SZ, (LPBYTE)&options.szFavDir, sizeof(options.szFavDir));
	if (key) {
		writeSucceeded &= SaveOption(key, _T("MacroArray"), REG_BINARY, (LPBYTE)&options.MacroArray, sizeof(options.MacroArray));
	}
	else {
		TCHAR keyname[14];
		int i;
		TCHAR bounded[MAXMACRO + 2];
		for (i = 0; i < 10; ++i) {
			wsprintf(keyname, _T("szMacroArray%d"), i);
			wsprintf(bounded, _T("[%s]"), options.MacroArray[i]);
			writeSucceeded &= SaveOption(key, keyname, REG_SZ, (LPBYTE)&bounded, MAXMACRO);
		}
	}
	writeSucceeded &= SaveOption(key, _T("BackColour"), REG_BINARY, (LPBYTE)&options.BackColour, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("FontColour"), REG_BINARY, (LPBYTE)&options.FontColour, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("BackColour2"), REG_BINARY, (LPBYTE)&options.BackColour2, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("FontColour2"), REG_BINARY, (LPBYTE)&options.FontColour2, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("rMargins"), REG_BINARY, (LPBYTE)&options.rMargins, sizeof(RECT));

	if (!writeSucceeded) {
		ReportLastError();
	}

	if (key) {
		RegCloseKey(key);
	}
}

/**
 * Save a window's placement.
 *
 * @param[in] hWndSave Handle to the window which placement is to be saved.
 */
void SaveWindowPlacement(HWND hWndSave)
{
	HKEY key = NULL;
	BOOL writeSucceeded = TRUE;
	RECT rect;
	int left, top, width, height, max;
	WINDOWPLACEMENT wndpl;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWndSave, &wndpl);

	if (wndpl.showCmd == SW_SHOWNORMAL) {
		GetWindowRect(hWndSave, &rect);
		max = SW_SHOWNORMAL;
	}
	else {
		if (wndpl.showCmd == SW_SHOWMAXIMIZED)
			max = SW_SHOWMAXIMIZED;
		else
			max = SW_SHOWNORMAL;
		rect = wndpl.rcNormalPosition;
	}

	left = rect.left;
	top = rect.top;
	width = rect.right - left;
	height = rect.bottom - top;

	if (!g_bIniMode) {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
			ReportLastError();
			return;
		}
	}

	writeSucceeded &= SaveOption(key, _T("w_WindowState"), REG_DWORD, (LPBYTE)&max, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("w_Left"), REG_DWORD, (LPBYTE)&left, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("w_Top"), REG_DWORD, (LPBYTE)&top, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("w_Width"), REG_DWORD, (LPBYTE)&width, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("w_Height"), REG_DWORD, (LPBYTE)&height, sizeof(int));

	if (!writeSucceeded) {
		ReportLastError();
	}

	if (key != NULL) {
		RegCloseKey(key);
	}
}

/**
 * Save metapad's menus and data.
 */
void SaveMenusAndData(void)
{
	HKEY key = NULL;

	if (!g_bIniMode) {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
			ReportLastError();
			return;
		}
	}
	if (options.bSaveMenuSettings) {
		SaveOption(key, _T("m_WordWrap"), REG_DWORD, (LPBYTE)&bWordWrap, sizeof(BOOL));
		SaveOption(key, _T("m_PrimaryFont"), REG_DWORD, (LPBYTE)&bPrimaryFont, sizeof(BOOL));
		SaveOption(key, _T("m_SmartSelect"), REG_DWORD, (LPBYTE)&bSmartSelect, sizeof(BOOL));
#ifdef USE_RICH_EDIT
		SaveOption(key, _T("m_Hyperlinks"), REG_DWORD, (LPBYTE)&bHyperlinks, sizeof(BOOL));
#endif
		SaveOption(key, _T("m_ShowStatus"), REG_DWORD, (LPBYTE)&bShowStatus, sizeof(BOOL));
		SaveOption(key, _T("m_ShowToolbar"), REG_DWORD, (LPBYTE)&bShowToolbar, sizeof(BOOL));
		SaveOption(key, _T("m_AlwaysOnTop"), REG_DWORD, (LPBYTE)&bAlwaysOnTop, sizeof(BOOL));
		SaveOption(key, _T("m_Transparent"), REG_DWORD, (LPBYTE)&bTransparent, sizeof(BOOL));
		SaveOption(key, _T("bCloseAfterFind"), REG_DWORD, (LPBYTE)&bCloseAfterFind, sizeof(BOOL));
		SaveOption(key, _T("bNoFindHidden"), REG_DWORD, (LPBYTE)&bNoFindHidden, sizeof(BOOL));
	}

	if (!options.bNoSaveHistory) {
		if (key) {
			SaveOption(key, _T("FindArray"), REG_BINARY, (LPBYTE)&FindArray, sizeof(FindArray));
			SaveOption(key, _T("ReplaceArray"), REG_BINARY, (LPBYTE)&ReplaceArray, sizeof(ReplaceArray));
		}
		else {
			TCHAR keyname[16];
			TCHAR bounded[MAXFIND + 2];
			int i;
			for (i = 0; i < 10; ++i) {
				wsprintf(keyname, _T("szFindArray%d"), i);
				wsprintf(bounded, _T("[%s]"), &FindArray[i]);
				SaveOption(key, keyname, REG_SZ, (LPBYTE)bounded, MAXFIND);
			}
			for (i = 0; i < 10; ++i) {
				wsprintf(keyname, _T("szReplaceArray%d"), i);
				wsprintf(bounded, _T("[%s]"), &ReplaceArray[i]);
				SaveOption(key, keyname, REG_SZ, (LPBYTE)bounded, MAXFIND);
			}
		}
	}

	if (options.bSaveDirectory) {
		SaveOption(key, _T("szLastDirectory"), REG_SZ, (LPBYTE)szDir, sizeof(TCHAR) * (lstrlen(szDir) + 1));
	}

	if (key != NULL) {
		RegCloseKey(key);
	}
}
