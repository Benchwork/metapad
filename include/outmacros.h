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

#ifndef OUTMACROS_H
#define OUTMACROS_H

#define ERROROUT(_x) MessageBox(hwnd, _x, STR_METAPAD, MB_OK | MB_ICONEXCLAMATION)
#define MSGOUT(_x) MessageBox(hwnd, _x, STR_METAPAD, MB_OK | MB_ICONINFORMATION)
#define DBGOUT(_x, _y) MessageBox(hwnd, _x, _y, MB_OK | MB_ICONEXCLAMATION)

#endif
