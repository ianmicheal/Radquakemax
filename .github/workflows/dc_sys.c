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
// dc_sys.c -- Dreamcast/KOS System Driver

#include "quakedef.h"
#include "errno.h"
#include <SDL/SDL.h>
#include <arch/timer.h>
#include <arch/arch.h>
#include <dc/cdrom.h>


qboolean	isDedicated = false;
int noconinput = 0;
cvar_t  sys_linerefresh = {"sys_linerefresh","0"};// set for entity display
cvar_t  sys_nostdout = {"sys_nostdout","0"};
/*
===============================================================================

FILE IO

===============================================================================
*/

#define MAX_HANDLES             50 // 4.8
FILE    *sys_handles[MAX_HANDLES];

int findhandle (void)
{
	int             i;
	
	for (i=1 ; i<MAX_HANDLES ; i++)
		if (!sys_handles[i])
			return i;
	Sys_Error ("out of handles");
	return -1;
}

/*
================
filelength
================
*/
int filelength (FILE *f)
{
	int             pos;
	int             end;

	pos = ftell (f);
	fseek (f, 0, SEEK_END);
	end = ftell (f);
	fseek (f, pos, SEEK_SET);

	return end;
}

int Sys_FileOpenRead (char *path, int *hndl)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "rb");
	if (!f)
	{
		*hndl = -1;
		return -1;
	}
	sys_handles[i] = f;
	*hndl = i;
	
	return filelength(f);
}

int Sys_FileOpenWrite (char *path)
{
	FILE    *f;
	int             i;
	
	i = findhandle ();

	f = fopen(path, "wb");
	if (!f)
		Sys_Error ("Error opening %s", path);
	sys_handles[i] = f;
	
	return i;
}

void Sys_FileClose (int handle)
{
	fclose (sys_handles[handle]);
	sys_handles[handle] = NULL;
}
void Sys_LineRefresh(void)
{
}
void moncontrol(int x)
{
}
void Sys_FileSeek (int handle, int position)
{
	fseek (sys_handles[handle], position, SEEK_SET);
}

int Sys_FileRead (int handle, void *dest, int count)
{
	return fread (dest, 1, count, sys_handles[handle]);
}

int Sys_FileWrite (int handle, void *data, int count)
{
	return fwrite (data, 1, count, sys_handles[handle]);
}

int     Sys_FileTime (char *path)
{
	FILE    *f;
	
	f = fopen(path, "rb");
	if (f)
	{
		fclose(f);
		return 1;
	}
	
	return -1;
}

void Sys_mkdir (char *path)
{
}

/*
===============================================================================

SYSTEM IO

===============================================================================
*/

void Sys_MakeCodeWriteable (unsigned long startaddr, unsigned long length)
{
}


void Sys_Error (char *error, ...)
{
	va_list         argptr;
	char			strbuffer[2048];

	printf ("Sys_Error: ");   
	va_start (argptr, error);
	vsprintf (strbuffer, error, argptr);
	va_end (argptr);
	printf ("%s\n", strbuffer);

	exit (1);
}

void Sys_Printf (char *fmt, ...)
{
	va_list         argptr;
	char			strbuffer[2048];

	va_start (argptr, fmt);
	vsprintf (strbuffer, fmt, argptr);
	printf (strbuffer);
	va_end (argptr);
}

void Sys_Quit (void)
{

}

double Sys_FloatTime (void)
{
	uint32 sec, msec;
	double time;
	timer_ms_gettime(&sec, &msec);
	time = sec + (msec / 1000.0);
	return time;
}
char *Sys_ConsoleInput (void)
{
	return NULL;
}

void Sys_Sleep (void)
{
}

void Sys_HighFPPrecision (void)
{
}

void Sys_LowFPPrecision (void)
{
}

//=============================================================================



// BlackAura (24-12-2002) - Modlist interface
extern int	modmenu_argc;
extern char	**modmenu_argv;

#define DC_BASEDIR	"/cd/quake"
//char *cachedir = "cache[1024]";
void ModMenu(const char *basedir);

int main ()
{
	static quakeparms_t    parms;
	double time, oldtime, newtime;
	extern int vcrFile;
	extern int recording;
	static int frame;

	moncontrol(0);
	// BlackAura (24-12-2002) - Jump to the ModMenu
	ModMenu(DC_BASEDIR);

	// BlackAura (08-12-2002) - Allocate 10MB of RAM
	parms.memsize = 11*1024*1024;
	parms.membase = malloc (parms.memsize);
   // parms.cachedir = cachedir;
	// BlackAura (08-12-2002) - Set base directory
	parms.basedir = DC_BASEDIR;

	// BlackAura (24-12-2002) - Initialise arguments from modlist
	COM_InitArgv (modmenu_argc, modmenu_argv);
	parms.argc = com_argc;
	parms.argv = com_argv;
    //Disable CD AUDIO for modmenu
  // SDL_Init(SDL_INIT_CDROM);
	// BlackAura (08-12-2002) - Init the host
	printf ("Host_Init\n");
	Host_Init (&parms);

  oldtime = Sys_FloatTime () - 0.1;
    while (1)
    {
// find time spent rendering last frame
	newtime = Sys_FloatTime ();
	time = newtime - oldtime;

	if (cls.state == ca_dedicated)
	{   // play vcrfiles at max speed
		if (time < sys_ticrate.value && (vcrFile == -1 || recording) )
		{
			SDL_Delay (1);
			continue;       // not time to run a server only tic yet
		}
		time = sys_ticrate.value;
	}

	if (time > sys_ticrate.value*2)
		oldtime = newtime;
	else
		oldtime += time;

	if (++frame > 5)
		moncontrol(1);      // profile only while we do each Quake frame
	Host_Frame (time);
	moncontrol(0);

// graphic debugging aids
	if (sys_linerefresh.value)
		Sys_LineRefresh ();
    }
}


