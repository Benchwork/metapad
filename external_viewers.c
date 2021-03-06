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
 * @file external_viewers.c
 * @brief External viewers functions.
 */

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <tchar.h>
#include <shellapi.h>

#include "include/file_save.h"
#include "include/typedefs.h"
#include "include/tmp_protos.h"
#include "include/external_viewers.h"
#include "include/strings.h"
#include "include/outmacros.h"
#include "include/resource.h"

extern HWND hwnd;
extern TCHAR szCaptionFile[];
extern TCHAR szFile[];
extern TCHAR szDir[];
extern BOOL bDirtyFile;

extern option_struct options;

/**
 * Try to execute a third party program.
 *
 * @param lpExecutable Path to program executable.
 * @param lpCommandLine Command line arguments to pass to program.
 * @return TRUE if successful, FALSE otherwise.
 */
BOOL ExecuteProgram(LPCTSTR lpExecutable, LPCTSTR lpCommandLine)
{
	TCHAR szCmdLine[1024];
	LPTSTR lpFormat;

	if (lpExecutable[0] == _T('"') && lpExecutable[lstrlen(lpExecutable) - 1] == _T('"')) {
		// quotes already present
		lpFormat = _T("%s %s");
	}
	else {
		// executable file must be quoted to conform to Win32 file name
		// specs.
		lpFormat = _T("\"%s\" %s");
	}

	wsprintf(szCmdLine, lpFormat, lpExecutable, lpCommandLine);

	if (lstrcmpi(lpExecutable + (lstrlen(lpExecutable) - 4), _T(".exe")) != 0) {
		/// @todo Should this inform about which error happened?
		if ((INT_PTR)ShellExecute(NULL, NULL, lpExecutable, szCmdLine, szDir, SW_SHOWNORMAL) <= 32) {
			return FALSE;
		}
	}
	else {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));

		si.cb = sizeof(STARTUPINFO);
		si.wShowWindow = SW_SHOWNORMAL;
		si.dwFlags = STARTF_USESHOWWINDOW;

		if (!CreateProcess(lpExecutable, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			return FALSE;
		}
		else {
			// We don't use the handles so close them now
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	return TRUE;
}

/**
 * Launch the primary external viewer.
 */
void LaunchPrimaryExternalViewer(void)
{
	TCHAR szLaunch[MAXFN] = {_T('\0')};

	lstrcat(szLaunch, options.szArgs);
	lstrcat(szLaunch, _T(" \""));
	lstrcat(szLaunch, szFile);
	lstrcat(szLaunch, _T("\""));
	if (!ExecuteProgram(options.szBrowser, szLaunch))
	ERROROUT(GetString(IDS_PRIMARY_VIEWER_ERROR));
}

/**
 * Launch the secondary external viewer.
 */
static void LaunchSecondaryExternalViewer(void)
{
	TCHAR szLaunch[MAXFN] = {_T('\0')};

	lstrcat(szLaunch, options.szArgs2);
	lstrcat(szLaunch, _T(" \""));
	lstrcat(szLaunch, szFile);
	lstrcat(szLaunch, _T("\""));
	if (!ExecuteProgram(options.szBrowser2, szLaunch))
		ERROROUT(GetString(IDS_SECONDARY_VIEWER_ERROR));
}

/**
 * Open current file on an external viewer.
 *
 * @param bCustom TRUE to use one of the custom viewers, FALSE to use associated program.
 * @param bSecondary TRUE to use secondary viewer, FALSE to use primary viewer. Ignored if the bCustom is FALSE.
 */
void LaunchInViewer(BOOL bCustom, BOOL bSecondary)
{
	if (bCustom) {
		if (!bSecondary && options.szBrowser[0] == _T('\0')) {
			MessageBox(hwnd, GetString(IDS_PRIMARY_VIEWER_MISSING), STR_METAPAD, MB_OK|MB_ICONEXCLAMATION);
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_VIEW_OPTIONS, 0), 0);
			return;
		}
//DBGOUT(options.szBrowser2, _T("run szBrowser2 value:"));
		if (bSecondary && options.szBrowser2[0] == _T('\0')) {
			MessageBox(hwnd, GetString(IDS_SECONDARY_VIEWER_MISSING), STR_METAPAD, MB_OK|MB_ICONEXCLAMATION);
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_VIEW_OPTIONS, 0), 0);
			return;
		}
	}

	if (szFile[0] == _T('\0') || (bDirtyFile && options.nLaunchSave != 2)) {
		int res = IDYES;
		if (options.nLaunchSave == 0) {
			TCHAR szBuffer[MAXFN];
			wsprintf(szBuffer, GetString(IDS_DIRTYFILE), szCaptionFile);
			res = MessageBox(hwnd, szBuffer, STR_METAPAD, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
		}
		if (res == IDCANCEL) {
			return;
		}
		else if (res == IDYES) {
			if (!SaveCurrentFile()) {
				return;
			}
		}
	}
	if (szFile[0] != _T('\0')) {
		if (bCustom) {
			if (bSecondary) {
				LaunchSecondaryExternalViewer();
			}
			else {
				LaunchPrimaryExternalViewer();
			}
		}
		else {
			INT_PTR ret = (INT_PTR)ShellExecute(NULL, _T("open"), szFile, NULL, szDir, SW_SHOW);
			if (ret <= 32) {
				switch (ret) {
				case SE_ERR_NOASSOC:
					ERROROUT(GetString(IDS_NO_DEFAULT_VIEWER));
					break;
				default:
					ERROROUT(GetString(IDS_DEFAULT_VIEWER_ERROR));
				}
			}
		}
	}

	if (options.bLaunchClose) {
		DestroyWindow(hwnd);
	}
}
