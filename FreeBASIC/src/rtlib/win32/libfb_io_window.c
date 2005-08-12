/*
 *  libfb - FreeBASIC's runtime library
 *	Copyright (C) 2004-2005 Andre V. T. Vicentini (av1ctor@yahoo.com.br) and others.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * window.c -- ...
 *
 * chng: ago/2005 written [mjs]
 *
 */

#include <stdlib.h>
#include <float.h>
#include "fb.h"
#include "fb_rterr.h"

SMALL_RECT srConsoleWindow;
static SMALL_RECT srRealConsoleWindow;

static __inline__ void ReadConsoleRect( SMALL_RECT *pRect, int GetRealWindow )
{
    CONSOLE_SCREEN_BUFFER_INFO info;

    if( GetConsoleScreenBufferInfo( fb_out_handle, &info )==0 ) {
        memset( pRect, 0, sizeof(SMALL_RECT) );
    } else {
        if( GetRealWindow ) {
            memcpy( pRect, &info.srWindow, sizeof(SMALL_RECT) );
        } else {
            pRect->Left = 0;
            pRect->Top = info.srWindow.Top;
            pRect->Right = info.dwSize.X - 1;
            pRect->Bottom = info.srWindow.Bottom;
        }
    }
}

/** Remembers the current console window coordinates.
 *
 * This function remembers the current console window coordinates. This is
 * required because some applications showing using a SAA interface doesn't
 * use WIDTH first to reduce the console screen buffer size which means that
 * the scroll bar of the console window is always visible/accessible which
 * also implies that the user might scroll up and down while the application
 * is running.
 *
 * When this library would always use the current console window coordinates,
 * the application might show trash when the user scrolled up or down the
 * buffer. But this is not what we want so we're only updating the console
 * window coordinates under the following conditions:
 *
 * - Initialization
 * - After screen buffer size change (using WIDTH)
 * - After printing text
 */
FBCALL void fb_hUpdateConsoleWindow( void )
{
    /* Whenever the console was set by the user, we MUST NOT query this
     * information again because this would cause a mess with SAA
     * applications otherwise. */
    if (ConsoleSetByUser)
        return;

    ReadConsoleRect( &srConsoleWindow, FALSE );
    ReadConsoleRect( &srRealConsoleWindow, TRUE );
}

/*:::::*/
void fb_InitConsoleWindow( void )
{
    static int inited = FALSE;
    if( !inited )
    {
    	inited = TRUE;
    	/* query the console window position/size only when needed */
    	fb_hUpdateConsoleWindow( );
    }
}

/*:::::*/
FBCALL void fb_hRestoreConsoleWindow( void )
{
    SMALL_RECT sr;

    /* Whenever the console was set by the user, there's no need to
     * restore the original window console because we don't have to
     * mess around with scrollable windows */
    if (ConsoleSetByUser)
        return;

    fb_InitConsoleWindow( );

    /* Update only when changed! */
    ReadConsoleRect( &sr, TRUE );
    if( (sr.Left != srRealConsoleWindow.Left)
        || (sr.Top != srRealConsoleWindow.Top)
        || (sr.Right != srRealConsoleWindow.Right)
        || (sr.Bottom != srRealConsoleWindow.Bottom) )
    {
        /* Keep the left/right coordinate of the console */
        sr.Top = srRealConsoleWindow.Top;
        sr.Bottom = srRealConsoleWindow.Bottom;
        SetConsoleWindowInfo( fb_out_handle, TRUE, &srRealConsoleWindow );
    }
}


