/*
    VikingExtractor, to recover images from Viking Lander operations.
    Copyright (C) 2010-2013 Cartesian Theatre <info@cartesiantheatre.com>.
    
    Public discussion on IRC available at #avaneya (irc.freenode.net) 
    or on the mailing list <avaneya@lists.avaneya.com>.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Includes...
#include <windows.h>

// Entry point...
int APIENTRY WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Prompt the user. Petzold will be pissed...
    const int Status = MessageBox(
        NULL, 
        "So there's good news and bad news. The bad news is that the Avaneya: "
        "Viking Lander Remastered software is not supported on legacy operating "
        "systems like Windows and OS X.\n\n"
        
        "But don't despair! The good news, however, is that upgrading to one "
        "that does is simple. Ubuntu is an alternative to proprietary operating "
        "systems like Windows and OS X. Built upon the freedom respecting GNU "
        "operating system, it is virus and malware free, safe, secure, costs "
        "nothing to download and use, is professionally supported, and already "
        "adopted worldwide by millions. Best of all, you won't need to worry "
        "about getting swindled anymore by those wretched monopolists in "
        "Redmond or peddlers of rotten cryolite fruit.\n\n"

        "If you would like to be taken to the Ubuntu homepage to learn more "
        "about this contraband knowledge, press Ok now.",
        "Warning: Legacy Operating System", 
        MB_OKCANCEL | MB_ICONWARNING);

    // If they selected ok, open a browser to the Ubuntu home page...
    if(Status == IDOK)
        ShellExecute(NULL, "open", "http://www.ubuntu.com/desktop", NULL, NULL, SW_SHOWNORMAL);

    // Done...
    return 0;
}

