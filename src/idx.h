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

#ifndef IDX_H
#define IDX_H

#define MAGIC_LENGTH( magic ) ( sizeof( magic ) / sizeof( char ) )

/*magic numbers*/
const char SSND[] = "SND.PACK";
const char SANM[] = "ANIM.PACK";
const char SSKL[] = "SKEL.PACK";
const char SEVT[] = "EVT.PACK";
const unsigned int STRDELIM = 3452816640u;

#define MP3TAG 13


#include "main.h"
#include <stdio.h>
#include <stdlib.h>


class idxSubFile
{
   public:
	idxSubFile();
	virtual ~idxSubFile();
	virtual bool Read( FILE* ) = 0;
	virtual bool Write( FILE* ) = 0;
	virtual bool GatherInfo() = 0;
	virtual bool PrepareDataOut();
	virtual void ParseFilename( const char* );
	virtual void PrintFilename();
	void Clean();
	bool Extract( FILE* );
	bool ExtractFile( FILE*, FILE* );
	virtual bool WriteFile( FILE*, FILE*, const char* );
	void SetFilename( const char* );

   public:
	static stack<str> dirs;
	unsigned int beginFileOffset;  // where the file starts
	unsigned int fileSize;         // how big it is...
	char filename[256];
	char* buffer;
};

class idxFile
{
   public:
	idxFile();
	virtual ~idxFile();
	bool Write( const char*, const char**, int );
	bool Extract( const char*, bool );

   private:
	virtual void WriteHeader() = 0;
	virtual bool ReadHeader() = 0;
	virtual void Init() = 0;
	virtual void Allocate() = 0;
	bool OpenFiles( const char*, const char* );
	void CloseFiles();

   public:
	const char* filetype;
	unsigned int fileVersion;
	unsigned int numFiles;
	idxSubFile* subFiles;
	FILE *fp, *fp2;
};

class sidxSubFile : public idxSubFile
{
   public:
	sidxSubFile();
	virtual bool Read( FILE* );
	virtual bool Write( FILE* );
	virtual bool GatherInfo();

   private:
	int control[4];  // contains info about sound file,
};

class sidxFile : public idxFile
{
   public:
	virtual void Init();

   private:
	virtual void WriteHeader();
	virtual bool ReadHeader();
	virtual void Allocate();
};

class sidxFile2 : public sidxFile
{
   public:
	virtual void Init();
};
class aidxSubFile : public idxSubFile
{
   public:
	aidxSubFile( bool allowScaling );
	virtual bool Read( FILE* );
	virtual bool Write( FILE* );
	virtual bool GatherInfo();
	virtual bool PrepareDataOut();
	virtual void ParseFilename( const char* );
	virtual void PrintFilename();
	virtual bool WriteFile( FILE*, FILE*, const char* );
	bool Scale();

   private:
	bool CheckProblemCas();
	bool enableScaling;

   public:
	int entrySize;
	float scale;  // deprecated??
	short numFrames;
	short numBones;
	char type;
};

class aidxFile : public idxFile
{
   public:
	virtual void Init();

   private:
	virtual void WriteHeader();
	virtual bool ReadHeader();
	virtual void Allocate();
};

class aidxFile2 : public aidxFile
{
   public:
	virtual void Init();

   private:
	virtual void Allocate();
};

class kidxSubFile : public idxSubFile
{
   public:
	kidxSubFile();
	virtual bool Read( FILE* );
	virtual bool Write( FILE* );
	virtual bool GatherInfo();
	virtual void ParseFilename( const char* );

   public:
	unsigned int len;
};

class kidxFile : public idxFile
{
   public:
	virtual void Init();

   private:
	virtual void WriteHeader();
	virtual bool ReadHeader();
	virtual void Allocate();
};

class kidxFile2 : public kidxFile
{
   public:
	virtual void Init();
};

class eidxFile : public idxFile
{
   public:
	virtual void Init();

   private:
	virtual void WriteHeader();
	virtual bool ReadHeader();
	virtual void Allocate();
};

class eidxFile2 : public eidxFile
{
   public:
	virtual void Init();
};

class eidxSubFile : public idxSubFile
{
   public:
	eidxSubFile();
	virtual bool Read( FILE* );
	virtual bool Write( FILE* );
	virtual bool GatherInfo();
	virtual void ParseFilename( const char* );

   private:
	int num, frameId;
	bool tainted;
};
#endif
