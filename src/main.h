/* Copyright (C) 2004, 2005        Vercingetorix <vercingetorix11@gmail.com>
 * Copyright (C) 2011, 2012, 2013, 2014  The Europa Barbarorum Team <webmaster@europabarbarorum.com>
 * 
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License 
 * as published by the Free Software Foundation; either version 2 
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License 
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#ifndef _XIDX_MAIN_H
#define _XIDX_MAIN_H

#include <string.h>
#include "shared.h"

#define HELP_HELP_STR "usage: help [command]\nexamples:\n\thelp chpath"

#define HELP_CHPATH_STR \
    "changes the filename paths in a skeleton file\n\n"\
    "usage: chpath <skeleton filename> [LIST] [COPY] [ALL] <filename> <new filename>\n"\
    "specifying \"ALL\" will match all strings as opposed to just the first\n"\
    "specifying \"COPY\" will actually copy the files and not modify the paths\n"\
    "specifying \"LIST\" will list the paths\n"\
    "examples:\n\tchpath fs_new_skel ALL data/animations/LIS data/animations/NIS"\
    "\n\tchpath fs_new_skel \"data/animations/LIS 01 Stand Idle.cas\""\
    " \"data/animations/My new animation.cas\""\
    "\n\tchpath fs_new_skel COPY ALL data/animations/ data/animations/new_skel/"
    
#define HELP_SKEL_STR "replaces the skeleton hierarchy with one from a .cas file\n" \
                      "usage: skel <skeleton filename> <cas filename>\n" \
                      "examples:\n\tskel fs_new_skel monster.cas"
#define HELP_SCALE_STR \
    "changes the scale of a skeleton\n" \
    "usage: scale <skeleton filename> [scale value]\n" \
    "if you do not specify a scale value it will print the current one"
    
#define HELP_CLEAR_STR "clears screen"
#define HELP_COPY_STR "copy a file\n" \
		      "usage: copy <filename> <destination>"

#define HELP_FORK_STR "passes the command to the system to be executed\n" \
                      "usage: fork <command>\n" \
                      "example:\n\tfork xidx -ts skeleton.idx" 
		      
#define HELP_MKDIR_STR "create a direcotry, similiar to mkdir -p.\naccepts both Windows and UNIX style pathnames.\n" \
		       "usage: mkdir <directories>\n" \
		       "example:\n\tmkdir new/dir" \
		       "\n\tmkdir with\\different\\slashes" \
		       "\n\tmkdir \"dir with spaces\" and/another\\without"

/*globals...*/
extern bool  verbose;
extern bool  perform_suffix_manip;
extern bool  no_implicit_mangling;
extern bool  keep_bslash_mangling;
extern const char * suffix_pattern;
extern int suffix_len;

//functions to go with shell
struct xidxFunc{    
    xidxFunc( const char* name, const char* usage, int (*execute)(int, char**) ){
        this->name  = name;
        this->usage = usage;
        this->execute = execute;
    }    
    const char* name;
    const char* usage;
    int (*execute)( int, char** );
};    

        
//XIDX 'shell'
class xidxShell{
    public:
        xidxShell();
        ~xidxShell();
    public:
        int Run();
    public:
        static int help( int, char** );
	static int chpath( int, char** );
        static int copy( int, char** );
        #ifdef _WIN32
        static int clear( int, char** );
	#endif
        static int loadskel( int, char** );
        static int fork( int, char** );
        static int scale( int, char** );
        static int exit( int, char** );
	static int mkdir( int, char** );
    private:
        int Parse( char*,xidxFunc* );
        static xidxFunc* getfn( const char* );
        static stack<char*> gettokens( char*, const char*, bool=true );
        static int numargs( const char** );
        static int readfile( const char*, const char*, char*&, FILE*&, unsigned int& );
    private:
        char input[256];
};    
    
#endif
