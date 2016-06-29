This repository is not affiliated with the Europa Barbarorum Team, just a fork of the source provided here: http://www.twcenter.net/forums/downloads.php?do=file&id=3559.

----------------------------------------------
Rome- and Medieval II- Total War IDX packer/extractor

Created by the Europa Barbarorum Team (copyright: 2011, 2012, 2013, 2014), based on the original tool by Vercingetorix (copyright: 2004, 2005).

This version of XIDX should build and run under any POSIX compatible environment and Windows NT.

v1.6.2 - 30-05-2014


----------------------------------------------
Table of contents:
----------------------------------------------

  Installation
  Usage
  Latest Changes/Update Log
  Building from source
  Licence Agreement

----------------------------------------------
Installation
----------------------------------------------

If you use a source distribution of the XIDX project you can simply extract the 7zip archive and proceed with make.  For more information, see the 
"Building from source" section below.

If you use a binary release of the XIDX project you can simply extract the 7zip archive which should give you the following versions of the XIDX program:
 - bin/xidx:
     This is an AMD64 ELF executable. It should run on Linux (AMD64).
 - bin/xidx_x86.exe:
     This is an x86 PE executable. It should run on 32bit and 64bit versions of Windows OSes (2k, XP, Vista, 7, Server R2 etc.) and Wine.
     It should run on any processor of Core vintage or later (or AMD designs of similar age).
 - bin/xidx_amd64.exe:
     This is an AMD64 PE executable. It should run on 64bit versions of Windows OSes (XP, Vista, 7, Server R2 etc.)

The project does not provide any rule to install it as system administrator, so you may want to select your preferred executable and move it manually to a 
suitable location. It makes sense to ensure that this location is on the search PATH used for looking up programs by name, since XIDX is purely a 
commandline program. (Therefore being on the search PATH will make it easier to invoke the program from arbitrary directories.)

----------------------------------------------
Usage
----------------------------------------------

If you use a platform that is not supported by either of these binaries you can try to build from source instead.

program usage: [options] [files...]
command line options:
      -a      idx pack(s) are animation packs, by default they are assumed to be sound
      -e      idx pack(s) are event packs
      -s      idx pack(s) are skeleton packs
      -m      idx packs(s) are Medieval II packs, only applies when creating packs.
      -c      create an idx pack file. Uses 'default' as output file name by default.
      -x      extract idx pack file(s)
      -t      list the contents of idx pack file(s)
      -f      name of idx pack when creating. (output filename must follow)
              Omitting this option when creating packs is equivalent to '-f default'
      -v      verbose output
      -B      strips a given suffix from filenames when packing, adds it when extracting
      -b      short for -B bin, for compatibility with XIDX prior to version 1.5
      -p      preserve paths; disable all implicit path mangling
              This option does not disable replacing backslashes (\\) with slashes (/).
              Equivalent to -P on POSIX platforms
      -P      preserve paths; disable all implicit path mangling
              This option also disables replacing backslashes (\\) with slashes (/).
      -h      print the help
      --help  print the help
      --shell gives you a simple 'shell' for entering commands

You may only specify one of the three "xtc" options. When you create a idx pack the 
filenames can be specified on the command line (after your options). If no files are 
on the command line, it will read filenames from stdin (standard input) with one filename 
per line.

As a note regarding repacking pack.idx: There are some filenames that have spaces at the end,
as well as duplicate files in the pack.idx. Unfortunatly we have to deal with this and a listing
of the data/animations folder will not suffice. I recommend that you save the file list of the
vanilla pack before you do any editing. "xidx -ta > filenames_in_pack_idx.txt" would do the trick.
When you go to repack the animations you would use this list as the input for filenames. 
i.e. "xidx -caf pack.idx < filenames_in_pack_idx.txt".


Examples:
    % xidx -x music.idx
    % xidx -t music.idx > filenames_in_music_idx.txt
    % xidx -c -f my_music_idx.idx < filenames_in_music_idx.txt

The first example extracts all the files within music.idx (relative to the current working directory).
The second example lists all the files within music.idx and saves the output to filenames_in_music_idx.txt
The third example creates a new idx pack with the name my_music_idx. The files that are to
be archived are within filenames_in_music_idx.txt which is redirected to stdin.

In short, the above three commands extract the music, list the the files within the pack
and then repacks them.

If you wanted to pack all the files in a folder you could do something like this:

On Windows:
    > dir  data\sounds\SFX\* /a:-D /s /b | xidx -cf my_sfk.idx
      where you would change "data\sounds\SFX\*" to the folder of your choice.

On POSIX systems:
    $ find data/sounds/SFX/ -type f | xidx -cf my_sfk.idx
      where you would change "data/sounds/SFX" to the directory of your choice.

Additionally there are a few issues to be aware of when (re)packing archives with XIDX. The following 
covers 'implicit path rewriting', 'animations', 'skeletons' and (sound) 'events' archives in that order.

=== Implicit path rewriting ===

Please note that when packing XIDX converts absolute paths into relative paths by stripping the component
preceding the last "data/" or "bi/data/" directory tree in the file names and warns about files without 
a "data/" component in their paths. For example:

    - "D:\Mods\my_mod\data\animations\test.cas" would be rewritten to "data/animations/test.cas"
    - "D:\Mods\my_mod\bi\data\animations\test.cas" would be rewritten to "bi/data/animations/test.cas"
    - "D:\Mods\my_mod\data\sounds\SFX\data\test.wav" would be changed to "data/test.wav"
    - "..\animations\test.cas" which has no "data/" component would become "../animations/test.cas" and 
      trigger a warning
    - "D:\Mods\my_mod\test.cas" would not become "D:/Mods/my_mod/test.cas" and trigger a warning as well.
    
This path rewriting is meant to make the tool easier to use when creating portable IDX/DAT archives and 
to simplify interoperability with other commandline tools (especially on Windows).

However, this does mean that the last "data" directory in the path should also be the main "data" directory 
of your mod to ensure that external code in your mod continues to reference the correct file. Otherwise, 
files such as "D:\Mods\my_mod\data\sounds\SFX\data\test.wav" would be changed to "data/test.wav" whereas 
code might still reference "data/sounds/SFX/data/test.wav" which would not exist in the IDX archive. 
Additionally name clashes may also be introduced in this manner, e.g.: when you attempt to pack both 
"D:\Mods\my_mod\data\sounds\SFX\data\test.wav" and "D:\Mods\my_mod\data\test.wav" since both files would be 
renamed to "data/test.wav".

Finally, in the case of skeleton archives XIDX will only store file names instead of file paths in XIDX. 
This is meant to simplify generating working skeleton packs as these should not contain any full or relative 
paths including directory names.

If you find that this automatic path rewriting does more harm than good, you can disable this feature 
by passing either the -p or the -P option. If you want to keep the conversion of backslashes to slashes, you 
should use the -p option on Windows. On POSIX there is no difference as backslashes are not treated specially.

=== Packing animations ===

A common pitfall of packing animations for Medieval II: Total War is that the file format of animations in the 
IDX/DAT archive is different from the CAS files that most animation tools produce. Presently, XIDX does not 
perform such conversion automatically when creating animation packs (not even with the -m flag specified). 

You must make sure that the data you pack is of the right file format first if you wish to use this pack with 
a mod. Otherwise Medieval II: Total War will likely crash on startup (CTD).

Secondly XIDX contains a dubious feature which is supposed to rescale animations. This is a holdover from 
much older versions of XIDX (when it dealt with RTW archives exclusively). 

This option must not be used in combination with the -p or -P options. Scaling is triggered by appending an 
extra parameter to file paths, but -p and -P prevent XIDX from parsing this extra additon. Additonally 
scaling is disabled for packs generated with the -m flag because the scaling algorithm is definitely incorrect 
for the Medieval II: Total War file format. In the worst case XIDX will 'silently' pack the wrong files if you 
try to use scaling in combination with either -p, -P or -m options.

Please note that the use of the scaling option is highly discouraged even when creating Rome: Total War archives. 
It is better to make sure that animations have been properly scaled prior to packing them with XIDX, especially 
considering the fact that the code which performs the scaling does not appear to take animation meta data into 
account. Consider the feature deprecated: valid bug reports against it will result in its definite removal from 
future versions of XIDX.

=== Packing skeletons ===

In the case of skeleton archives XIDX will only store file names instead of file paths in XIDX. This is meant 
to simplify generating working skeleton packs as these should not contain any full or relative paths including 
directory names. This functionality can be disabled using the -p or -P options.

When (re)packing skeletons that have been regenerated by Medieval II: Total War you may need to fix up the 
path names inside the skeleton files. Skeletons refer to animations, and Medieval II: Total War generates 
absolute path names for these references. If you require your skeleton archive to be portable, you need to 
convert such absolute path names in the skeleton files themselves to appropriate relative paths instead. 
Relative paths should start with "data/", typically "data/animations". Without fixing the skeletons Medieval II: 
Total War will likely crash on startup (CTD).

Much the same may apply to RTW skeletons but I have not confirmed this.

=== Packing (sound) events ===

To (re)pack events archives (events.idx and events.dat) correctly the invidiual binaries must be packed in 
the right order and file names must be prefixed with a number indicating the type of event binary as well. 
To make this easier XIDX maintains the following file name convention when unpacking the event files:

<type_number>_<lowercase letter>_<number>[optional_tag].bin

Files should be packed in ascending order of their <type_number>, and in ascending order of their <number>.
Hence 2_a_5.bin should be packed before 4_a_1.bin (since 2 is less than 4), and 4_a_1.bin should be packed
before 4_a_7.bin (since 1 is less than 7).

Valid type numbers are 1 (global settings), 2 (standalone events identified by their number), 3 (standalone
events identified by their name using lexicographic order), and 4 (event banks).

Examples with optional tag:
    - 1_a_1_global_settings.bin
    - 2_b_20_START_GAME.bin
    - 3_a_1_ambient_alchemy.bin
    - 4_b_25_terrain_ambient.bin
		
Without optional tag:
    - 1_a_1.bin
    - 2_c_225.bin
    - 3_b_15.bin
    - 4_a_7.bin

The lowercase letter represents the integer part of the base 10 logarithm (log()) of the <number> field,
starting with `a' for `0', up to `z' for `26'. Hence event number `15' corresponds to the letter `b'. This
convention aids sorting of the files since normal sorting by file name would place files with <number> `100' 
before files with <number> `19': the addition of the extra letter fixes this which means that simply 
ordering by file name also ensures correct ordering for packing with XIDX.

If you use the above convention you can use the following commands to pack events archives correctly:

On Windows:
    > dir data\sounds\unpacked_events\*.bin /s /b /o:n /a:-D | xidx -cebf my_events
    where you would change "data\sounds\unpacked_events\" to the directory of your choice.

On POSIX systems:
    $ find data/sounds/unpacked_events -type f -name *.bin | sort | xidx -cebf my_events
    where you would change "data/sounds/unpacked_events" to the directory of your choice.

Please note that XIDX does not generate the optional tags when unpacking event binaries and ignores it
when packing files: this field serves purely as a label for your own convenience.

----------------------------------------------
Latest Changes/Update Log:
----------------------------------------------
v1.6.2- 30-05-2014 Fix build error on MINGW/GCC 4.9
                   Attempt to make XIDX print somewhat useful output to stdout and stderr when taken separately (I/O redirection)
v1.6.1- 24-03-2014 Fix Windows builds to not require libwinpthread-1.dll (-static).
                   Fix -h option.
                   Report first of -p or -P as not affecting extracting/listing archives.
                   Simplify some options parsing.
v1.6  - 23-03-2014 Add extra warnings and mention default file paths in --help output.
                   Mention -h option in usage info.
                   Add note on correctly packing skeletons, animation CAS file format(s) conversion.
                   Add -P and -p options for disabling implicit path mangling.
                   Write skeleton names instead of paths by default. (May be overriden using -P or -p.)
                   Disable scaling for M2TW animation archives.
v1.5  - 05-04-2013 Add -B option for arbitrary suffix manipulation (takes suffix argument), -b is now equivalent to `-B bin'.
                   Fix bug in suffix manipulation with animation packs (fix bad entry sizes)
v1.4  - 13-10-2012 Fix file version stamp for event packs, introduce RTW version of the events pack file format and enable
		   -m switch to select between M2TW and RTW versions. Extend the readme with a section on correctly generating
		   packs.
v1.3  - 02-10-2012 Improve filename parsing for EVT blobs, fix error message not showing filename, fix non-virtual destructors
v1.2  - 15-04-2012 Fix memory leak when opening archives with the wrong archive type flag
v1.1  - 26-03-2012 Fix memory leaks from exit() in usage
		   Change naming conventions for EVT blobs extracted from EVT packs (more useful info in filenames).
v1.0  - 24-02-2012 Fix bug in generated filenames for evt packs, fix version reporting.
v0.99 - 17-01-2012 Fix Valgrind warnings about unitialised variables, fix bug in sound packing.
		   Fix makefile to generate statically linked binaries for Windows.
v0.98 - 17-10-2011 Port/fixes to build on Linux with GCC (tested AMD64 builds of GCC 4.3.2, 4.5.2)
		   Builds with GCC-Mingw32 (tested AMD64 build, version 4.4.4)
		   Fix lurking segfaults on exit and inside the fork function of the XIDX shell.
		   Fix memory allocation, so the tool does not allocate lots of memory it never uses.
		   Add -m option for building Medieval II versions of supported pack types.
		   Add -b option for transparent suffix ('.bin') manipulation (useful with skeletons for avoiding name collisions).
		   Add -e option for assembly/disassembly of event packs.
		   Strip batch files, MS Visual Studio project file etc., added makefile.

v0.97 - 19-10-2005 Fixed problem where BI animations would not pak
v0.94 - 09-06-2005 Further fixes. Added skeleton scaling.
v0.93 - 08-06-2005 Major bug fixes regarding animation scaling
v0.91 - 06-06-2005 Added xidx shell.
v0.89 - 02-06-2005 Added skeleton extraction/packing. Fixed animation scale problem.
v0.82 - 23-04-2005 Animation packs now supported
v0.70 - 17-04-2005 Released

----------------------------------------------
Building from source
----------------------------------------------
The XIDX project now ships a makefile which can be used with GNU make to make building the project easier. For compiling the project 
using the makefile you will need a reasonably modern GCC distribution (G++ frontend) and/or MINGW frontend to same if you intend to 
build a Windows executable. Note that the makefile additionally performs cross compilation to build Windows executables through MINGW. 
The frontends to GCC which it will attempt to use for compiling the project are: g++, i586-mingw32msvc-g++, i686-w64-mingw32-g++, and 
x86_64-w64-mingw32-g++. 

Note that if both i586-mingw32msvc-g++ and i686-w64-mingw32-g++ are found the latter is skipped. The former should be compatible roughly
with all processors of Pentium vintage or later, the latter offers compatibility with Core processors and later. 

If you happen to have other MINGW C++ programs installed you may be able to use dynamically linked versions of the MINGW executables. These have the 
benefit of smaller executable sizes. To do this, simply leave the STATICS variable blank. Generated binaries will then depend on libgcc_sjlj-1.dll and 
libstc++-6.dll in order to work.

When a frontend cannot be found the makefile will skip compilation for the corresponding target platform automatically. 
This means that the makefile may generate the following executables:
  - bin/xidx:
      This is the native executable. It's target platform is whatever the default target platform happens to be (typically the same as the build platform).
      Thus if you run an AMD64 flavour of Linux, this will be an AMD64 ELF binary.
  - bin/xidx_x86.exe:
      This is a statically linked x86 PE executable. It should run on 32bit and 64bit versions of Windows OSes (XP, Vista, 7, Server R2 etc.)
  - bin/xidx_amd64.exe:
      This is a statically linked AMD64 PE executable. It should run on 64bit versions of Windows OSes (XP, Vista, 7, Server R2 etc.)

The dependencies are minimal, any proper POSIX compatible environment with a C++ standard library should satisfy the minimum requirements. When compiling 
for Windows based systems the code uses some Windows specific non-deprecated aliases of deprecated POSIX standard functions as this may help to suppress 
C++ standard conformance warnings when building on Windows.

The makefile provides the following three targets:
  - build:
      This is the default target and builds the program from source. This target includes provisions for cross compiling XIDX to build the Windows 
      binaries on Linux/Unix through MINGW.
  - bin:
      This target sets up the bin/ directory for use by the build target. This target is not intended to be called manually.
  - clean:
      Removes the bin/ directory structure.

It should be possible to build the source using toolchains other than G++ and G++/MINGW, however the makefile doesn't attempt to support this out of the box.

----------------------------------------------
Licence Agreement:
----------------------------------------------
Terms of Use
This software is free and there is no warranty what so ever. Use at your own risk. 
See licence.txt for a copy of the program licence.

    Copyright (C) 2004, 2005  Vercingetorix <vercingetorix11@gmail.com>
    Copyright (C) 2011, 2012, 2013, 2014 The Europa Barbarorum Team <webmaster@europabarbarorum.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details at 
    http://www.fsf.org/licensing/licenses/gpl.txt
