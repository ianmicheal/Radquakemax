/*
Copyright (C) 1996-1997 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// dc_vid.c
// Very basic software mode video driver for Dreamcast/KOS

#include "quakedef.h"
#include "d_local.h"

// BlackAura - KallistiOS headers
#include <kos.h>

viddef_t	vid;				// global video state

// Low-resolution version
#define	BASEWIDTH		320
#define	BASEHEIGHT		200
#define SCREENHEIGHT	240
#define VRAM_OFFSET		4800
#define KOS_VIDMODE		DM_320x240

// High-resolution version - slow
/*
#define	BASEWIDTH		640
#define	BASEHEIGHT		400
#define SCREENHEIGHT	480
#define VRAM_OFFSET		19840
#define KOS_VIDMODE		DM_640x480
*/

byte	vid_buffer[BASEWIDTH*BASEHEIGHT];
short	zbuffer[BASEWIDTH*BASEHEIGHT];

// BlackAura (15-12-2002) - Increase surface cache size
byte	surfcache[SURFCACHE_SIZE_AT_320X200];

unsigned short	d_8to16table[256];
unsigned	d_8to24table[256];

// Copied from nxDoom
#define PACK_RGB565(r,g,b) (((r>>3)<<11)|((g>>2)<<5)|(b>>3))
unsigned short paletteData[256];

// Clears VRAM - gets rid of annoying debris
// Copied from nxDoom
void VID_ClearVRam()
{
	int x, y, ofs;

	// Clear VRAM
	for(y = 0; y < SCREENHEIGHT; y++)
	{
		ofs = (BASEWIDTH * y);
		for(x = 0; x < BASEWIDTH; x++)
			vram_s[ofs + x] = 0;
	}
}

// Adapted from nxDoom
void	VID_SetPalette (unsigned char *palette)
{
	int r, g, b, i, c;
	c = 0;
	for(i = 0; i < 256; i++)
	{
		r = palette[c++];
		g = palette[c++];
		b = palette[c++];
		paletteData[i] = PACK_RGB565(r, g, b);
	}	
}

void	VID_ShiftPalette (unsigned char *palette)
{
	VID_SetPalette (palette);
}

void	VID_Init (unsigned char *palette)
{
	vid.maxwarpwidth = vid.width = vid.conwidth = BASEWIDTH;
	vid.maxwarpheight = vid.height = vid.conheight = BASEHEIGHT;
	vid.aspect = 1.0;
	vid.numpages = 1;
	vid.colormap = host_colormap;
	vid.fullbright = 256 - LittleLong (*((int *)vid.colormap + 2048));
	vid.buffer = vid.conbuffer = vid_buffer;
	vid.rowbytes = vid.conrowbytes = BASEWIDTH;
	
	d_pzbuffer = zbuffer;
	D_InitCaches (surfcache, sizeof(surfcache));

	// Initialise the KOS crap
	VID_ClearVRam();
	vid_set_mode(KOS_VIDMODE, PM_RGB565);

	// Set up the default palette
	VID_SetPalette(palette);
}

void	VID_Shutdown (void)
{
}

// Adapted from nxDoom
void	VID_Update (vrect_t *rects)
{
	int x, y, d = VRAM_OFFSET, s = 0;
	for(y = 0; y < BASEHEIGHT; y++)
		for(x = 0; x < BASEWIDTH; x++)
			vram_s[d++] = paletteData[vid_buffer[s++]];
}

/*
================
D_BeginDirectRect
================
*/
void D_BeginDirectRect (int x, int y, byte *pbitmap, int width, int height)
{
}


/*
================
D_EndDirectRect
================
*/
void D_EndDirectRect (int x, int y, int width, int height)
{
}


