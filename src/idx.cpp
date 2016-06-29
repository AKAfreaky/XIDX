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

#include <ctype.h>
#include <math.h>
#include "idx.h" 
#define IO_CHECK(a,b) if(io_check((a), (b))) { return false; }
#define IO_BUF_CHECK(a,b, buf, fp) if(io_buf_check((a), (b), buf, fp)) { buf = NULL; return false; }

#define DAT_SUFFIX "dat"
#define IDX_SUFFIX "idx"
stack<str> idxSubFile::dirs;

void append_suffix(const char* filename, const char* suffix, int len, char* r) {
    strncpy(r, filename, len);
    r[len] = '.';
    strcpy(&r[len + 1], suffix);
}

bool io_check(unsigned int cmpA, unsigned int cmpB) {
    if(cmpA != cmpB) {
	fputs("I/O Error\n", stderr);
	return true;
    }
    return false;
}

bool io_buf_check(unsigned int cmpA, unsigned int cmpB, void* buf, FILE* fp) {	
    if(io_check(cmpA, cmpB)) {
	free(buf);
	if(fp != NULL) {
	    fclose(fp);
	}
	return true;
    }
    return false;
}

void replace_suffix(const char* filename, const char* suffix, int len,char* r){
    return append_suffix(filename, suffix, len - strlen(suffix) -1, r);
}

int basename_length(const char* name, int len) {
    const char* pstr = strrchr( name,'/' );
    if( pstr ) {
	return len - (pstr - name) -1;
    }
    else {
	return len;
    }
}

void mangle_name(char* name, char* r, int len) {
    int s_len = 1 + suffix_len;
    memset(r, '\0', len + s_len);
    
    int len2 = basename_length(name, len);
    if (len2 > s_len) {
	char *suffix = name + (len - suffix_len) * sizeof(char);
        char * dot = suffix - sizeof(char);
	if (*dot == '.' && strcasecmp(suffix, suffix_pattern) == 0) {
	    strncpy(r, name, len);
	    return;
	}
    }
    if(len2 > 250) {
	replace_suffix(name, suffix_pattern, len, r);
    }
    else {
	append_suffix(name, suffix_pattern, len, r);
    }
}

void un_mangle_name(char* name, char *r, int len) {
    int s_len = 1 + suffix_len;
    memset(r, '\0', len + s_len);
    
    int len2 = basename_length(name, len);
    if(len2 > s_len) {
	char *suffix = name + (len - suffix_len) *sizeof(char);
	char * dot = suffix - sizeof(char);
        if (*dot == '.' && strcasecmp(suffix, suffix_pattern) == 0) {
	    strncpy(r, name, len - s_len);
	    return;
	}
    }
    strncpy(r, name, len);
}

/*class defs*/
idxSubFile::idxSubFile(){
    beginFileOffset = fileSize = 0;
    memset((void*) filename, '\0', MAGIC_LENGTH(filename));
    buffer = NULL;
}

idxSubFile::~idxSubFile(){
    Clean();
}
void idxSubFile::SetFilename( const char* str ){
    strncpy( filename, str, MAGIC_LENGTH(filename) );
}    
void idxSubFile::Clean(){
    if( buffer!=NULL ) free(buffer);
    buffer = NULL;
    //filename[0] = '\0';
    
}    
void idxSubFile::ParseFilename( const char* fname ){
    /*make the path relative to data\*/
    char* tstr = strdup( fname ); 
    const char* formated = getdpath( tstr, keep_bslash_mangling, !no_implicit_mangling);   
    if(no_implicit_mangling) {
        strncpy( filename, formated, MAGIC_LENGTH(filename) );
    }
    else {
        if( !formated ){
            if( strchr( tstr, '/' ) )
                fprintf(stderr,"warning: file %s does not have \"data\" in path\n", tstr);
            strncpy( filename, tstr, MAGIC_LENGTH(filename) );
        } 
        else{
            if( (formated - tstr) >= 3 ){
                if( !strncasecmp( &formated[-3], "bi/", 3 ) ){
                    formated -= 3;                
                }    
            }    
            strncpy( filename, formated, MAGIC_LENGTH(filename) );
        }
    }
    free( tstr );
}
void idxSubFile::PrintFilename(){
    if(perform_suffix_manip) {
        int ll =strlen(filename);
        char fn[ll+ 1 + suffix_len];
        mangle_name(filename, fn, ll);
        fputs(fn, stdout);
    }
    else {
        fputs(filename, stdout);
    }
}    
bool idxSubFile::PrepareDataOut(){
    return true;
}
    
bool idxSubFile::Extract(FILE* fp2){
    if( buffer!= NULL ) {
	free(buffer); 
	buffer =NULL;
    }
    void *ptr=calloc(fileSize, sizeof(char));
    if( ptr == NULL) {
	return false;
    }
    buffer = (char*) ptr;
    
    fseek(fp2, beginFileOffset, SEEK_SET);
    
    IO_BUF_CHECK(fread(buffer, sizeof(char), fileSize, fp2), fileSize, buffer, NULL )
    
    char* pstr = strrchr( filename,'/' );
    
    if( pstr ) {
        *pstr = '\0';
        if( !idxSubFile::dirs.search( str(filename) ) ){
            if( !mkrdir( filename ) ) return false;
            else idxSubFile::dirs.push( filename );
        }
        *pstr = '/';
    }
    FILE* fpout;
    if(perform_suffix_manip) {
	int ll =strlen(filename);
	char fn[ll + 1 + suffix_len];
	mangle_name(filename, fn, ll);
	fpout = fopen(fn, "wb");
    }
    else {
	fpout = fopen(filename, "wb");
    }
    if( fpout==NULL ) {
	free(buffer);
	buffer =NULL;
	return false;
    }
    
    bool result=PrepareDataOut();
    if(result) {
	IO_BUF_CHECK(fwrite(buffer, sizeof(char), fileSize, fpout), fileSize, buffer, fpout);
    }
    free(buffer);
    buffer = NULL;
    fclose( fpout );
    return result;
} 

      
bool idxSubFile::ExtractFile( FILE* fp, FILE* fp2 ){
    
    Clean();
    
    if( fp !=NULL ) {
	bool result = Read(fp);
	if(result && fp2 !=NULL) {
	    return Extract(fp2);
	}
	return result;
    }
    else {
	return false;
    }
}


bool idxSubFile::WriteFile( FILE* fp, FILE* fp2, const char* fname ){
    FILE* fpin = fopen( fname, "rb" );
    if( fpin==NULL ) { 
	fprintf(stderr, "Unable to open file: %s\n", fname); 
	return false;
    }
    Clean();
    int size= getfilesize( fpin );
    if(size == -1 ) {
      fclose(fpin);
      fprintf(stderr, "Cannot stat: %s\n", fname);
      return false;
    }
    fileSize = size;
    void* ptr = calloc(fileSize, sizeof(char));
    if( ptr ==NULL) {
	fclose(fpin);
	return false;
    }
    buffer = (char*) ptr;
    IO_BUF_CHECK(fread( buffer, 1, fileSize, fpin ), fileSize, buffer, fpin)
    fclose( fpin );   
    
    beginFileOffset =  ftell( fp2 );
    
    if(GatherInfo() && Write( fp )) {  
        IO_BUF_CHECK(fwrite( buffer, sizeof(char), fileSize, fp2 ), fileSize, buffer, NULL)
	fflush( fp );
	fflush( fp2 );
	free( buffer );
	buffer = NULL;
	return true;
    }
    else {
	fprintf(stderr, "Unable to pack file: %s\n", fname);
	free( buffer);
	buffer = NULL;
	return false;
    }
} 
     
idxFile::idxFile(){
    fileVersion = numFiles = 0;
    subFiles = NULL;
    filetype = NULL;
    fp = fp2 = NULL;
}    

idxFile::~idxFile(){
    if( subFiles !=NULL ) { 
      delete subFiles;
    }
}


bool idxFile::OpenFiles( const char* filename, const char* mode ){
    int len = strlen(filename);
    static const int max = MAGIC_LENGTH(DAT_SUFFIX) > MAGIC_LENGTH(IDX_SUFFIX) ? MAGIC_LENGTH(DAT_SUFFIX) : MAGIC_LENGTH(IDX_SUFFIX);
    int s_len = max + len;
    char p1[s_len];
    char p2[s_len];
    
    memset(p1, '\0', s_len);
    memset(p2, '\0', s_len);
    
    if(len > max) {
	const char* suffix = filename + (len- max) * sizeof(char);
	if ( strcasecmp(suffix, DAT_SUFFIX) == 0) {
	   
	    replace_suffix(filename, IDX_SUFFIX, len, p1);
	    strcpy(p2, filename);
	}
	else {
	    if(strcasecmp(suffix, IDX_SUFFIX)==0) {
		strcpy(p1,filename);
		replace_suffix(filename, DAT_SUFFIX, len, p2);
	    }
	    else {
		append_suffix(filename, IDX_SUFFIX, len, p1);
		append_suffix(filename, DAT_SUFFIX, len, p2);
	    }
	}
    }
    else {
	append_suffix(filename, IDX_SUFFIX, len, p1);
	append_suffix(filename, DAT_SUFFIX, len, p2);
    }
    
    fp = fopen( p1, mode );
    if( fp == NULL ) {
        fprintf( stderr, "failed to open '%s'\n", p1 );
	return false;
    }    
    fp2 = fopen( p2, mode );
    if( fp2 == NULL ){
        fprintf( stderr, "failed to open '%s'\n", p2 );
	fclose( fp );
	fp = NULL;
	return false;
    }
    return true;
}

void idxFile::CloseFiles(){
    if( fp != NULL ) fclose(fp);
    if( fp2!= NULL ) fclose(fp2);
    fp = fp2 = NULL;
}

      
bool idxFile::Write( const char* filename, const char** files, int count ){
    Init();
    if( !OpenFiles( filename, "wb" ) ) return false;
    
    WriteHeader(); 
    Allocate();
    
    for( int i = 0; i<count; i++ ){
        fprintf(stdout,"Packing file %d/%d ... %s\r", i+1, count, files[i]); 
        //parse
        subFiles->ParseFilename( files[i] );
        int result = subFiles->WriteFile( fp, fp2, files[i] );
        if( !result ) {
            fprintf( stderr, "Failed to pack file: %s\n", files[i]);
        }
        else if( verbose ) {
	    if(perform_suffix_manip) {
		int ll =strlen(filename);
		char fn[ll+ 1 + suffix_len];
		un_mangle_name(subFiles->filename, fn, ll);
		puts(fn);
	    }
	    else {
		puts( subFiles->filename );
	    }
	}
        
        numFiles += result;
    } 
    WriteHeader();   
    CloseFiles();  
    return numFiles;
} 

bool idxFile::Extract(const char* filename, bool list){
    if( !OpenFiles( filename, "rb" ) ) return false;
    
    if( !ReadHeader() ) {
	CloseFiles();
	return false;
    }
    
    Allocate();
    unsigned int success_count=0;
    for(unsigned int i = 0; i<numFiles; i++ ){
        if( !subFiles->ExtractFile( fp, list ? NULL : fp2 ) ) {
	    fprintf( stderr, "Error reading archived file '%s' from pack: '%s'\n", subFiles->filename, filename);
	}
        else {
	    success_count++;
	    if( verbose||list ) subFiles->PrintFilename();
	    else fprintf(stderr,"Extracting file %d/%d\r", i,numFiles);
	}
    }
    CloseFiles();
    return success_count == numFiles;
}

/*Sound sub idx file*/
sidxSubFile::sidxSubFile(){
    control[0]  = control[1] = control[2] = control[3] = 0;
}

bool sidxSubFile::Read( FILE* fp ){
    if( fp==NULL ) return false;
    IO_CHECK(fread(&beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fread(&fileSize, sizeof(int), 1, fp), 1)
    IO_CHECK(fread(control, sizeof(int), 4, fp), 4)
        
    /*read filename*/
    for( unsigned int i=0; (filename[i?i-1:0]||!i)&&i<MAGIC_LENGTH(filename); i++) filename[i] = fgetc(fp);
    fseek( fp, 3, SEEK_CUR);
    
    return true;
}  

bool sidxSubFile::Write( FILE* fp ){
    if( fp==NULL ) return false;
    
    IO_CHECK(fwrite( &beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite( &fileSize, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite( control, sizeof(int), 4, fp), 4)
    
    unsigned int ll =strlen(filename);
    if(perform_suffix_manip) {
	char fn[ll+ 1 + suffix_len];
	un_mangle_name(filename, fn, ll);
	ll=strlen(fn);
	IO_CHECK(fwrite(fn, sizeof(char), ll, fp), ll)
    }
    else {
	IO_CHECK(fwrite( filename, sizeof(char), ll, fp ), ll)
    }
    
    IO_CHECK(fwrite( &STRDELIM, sizeof(int), 1, fp), 1)
    
    return true;
}     

bool sidxSubFile::GatherInfo(){
    
    if( stristr(filename, ".mp3") ){
        control[0] = control[1] = control[2] = 0;
        control[3] = MP3TAG;
    }
    else{
        int* iptr = (int*) buffer;
        while( *iptr != 544501094/*fmt */&&iptr<(int*)(buffer+fileSize-16) ) iptr++;
        control[0] = *(iptr+3);
        control[1] = 16;
        control[2] = *(((short*)(iptr))+5);
        control[3] = *(((short*)(iptr))+4) == 1 ? 1 : 2; /*compression?*/     
    } 
    return true;
}    
  
  
/*Sound idx file*/
void sidxFile::Allocate(){
    if( subFiles != NULL ) delete subFiles;
    subFiles = new sidxSubFile;
}  

void sidxFile::Init(){
    fileVersion = 4;
    filetype    = SSND;    
}

void sidxFile2::Init(){
    fileVersion = 5;
    filetype    = SSND;
}

bool sidxFile::ReadHeader(){
    char checktype[MAGIC_LENGTH(SSND) ] = {0};
    IO_CHECK(fread( checktype,MAGIC_LENGTH(SSND) - 1, 1, fp), 1)
    if( strcmp( checktype, SSND) !=0) {
        fprintf( stderr, "File is not a sound idx archive.\n");
        return false;
    }    
    fseek( fp, 12,  SEEK_SET  ); 
    IO_CHECK(fread( &numFiles, sizeof(int), 1, fp), 1)
    fseek( fp, 24,  SEEK_SET  );  
    return true;
} 

void sidxFile::WriteHeader(){
    fseek( fp, 0,  SEEK_SET  ); 
    fseek( fp2, 0, SEEK_SET  );
    int zero[2] ={0};
    
    fwrite( filetype, sizeof(char), strlen(filetype), fp);
    fwrite( &fileVersion, sizeof(int), 1, fp);
    fwrite( &numFiles, sizeof(int), 1, fp);
    fwrite( zero, sizeof(int), 2, fp);
    
    fwrite( filetype, sizeof(char), strlen(filetype), fp2);
    fwrite( &fileVersion, sizeof(int), 1, fp2);
    fwrite( &numFiles, sizeof(int), 1, fp2);
    fwrite( zero, sizeof(int), 2, fp2);
    
    fflush( fp );
    fflush( fp2 );
    
}   

/*Animation sub idx file*/
aidxSubFile::aidxSubFile(bool allowScaling){
    entrySize = 0;
    scale = 1.0f;
    numFrames =  numBones = 0;
    enableScaling = allowScaling;
}
void aidxSubFile::PrintFilename(){
    if( verbose && scale != 1.0f ){
	if(perform_suffix_manip) {
	    int ll =strlen(filename);
	    char fn[ll+ 1 + suffix_len];
	    mangle_name(filename, fn, ll);
	    fprintf( stdout, "%s;scale=%f\n",  fn, scale);
	}
	else {
	    fprintf( stdout, "%s;scale=%f\n",  filename, scale);
	}
    }
    else idxSubFile::PrintFilename();
}
void aidxSubFile::ParseFilename( const char* fname ) {
    if(no_implicit_mangling || !enableScaling) {
        idxSubFile::ParseFilename(fname);
    }
    else {
        char* tstr = strdup( fname );
        char* pstr = stristr( tstr, ";scale=" );
        if( pstr ){
            sscanf( pstr, ";scale=%f", &scale );
            *pstr = '\0';         
        }
        else scale = 1.0f;
        idxSubFile::ParseFilename( tstr );
        //strcpy( filename, tstr);
        free( tstr );
    }
}      

bool aidxSubFile::WriteFile( FILE* fp, FILE* fp2, const char* fname) {
    if(no_implicit_mangling || !enableScaling) {
        return idxSubFile::WriteFile(fp, fp2, fname);
    }
    else {
        char *tstr = strdup(fname);
        char * pstr = stristr(tstr, ";scale=");
        if(pstr) {
            *pstr = '\0';
        }
        bool result = idxSubFile::WriteFile(fp, fp2, tstr);
        free(tstr);
        return result;
    }
}
    
bool aidxSubFile::PrepareDataOut(){
    if(enableScaling) {
        bool result;
        scale = 1.0f / scale;
        result = Scale();
        scale = 1.0f / scale;
        return result;
    }
    else {
        return true;
    }
}    
     
bool aidxSubFile::Scale(){
    if( buffer==NULL ) return false;        
    
    if( (scale != 1.0f) && (buffer[4] == 1) ){           
        //if( CheckProblemCas() ) return true; 
               
        //float* pelvis = (float*) &buffer[ (numFrames*numBones*16)+5 ];        
        //float* dist = (float*) &pelvis[ numFrames*3 ];
        //float* root = (float*) &dist[ 4*((numFrames-1) / 2) ];
        //float* end = &root[ numFrames*3 ];
        //for( int i = 0; i < 3*numFrames; i++ ){            
        //    pelvis[i] *= scale;
        //    root[i] *= scale;
        //}    
        //for( int i = 0; i < 4*((numFrames-1) / 2); i++ ){
        //    dist[ i ] *= scale;
        //}
        float* start = (float*) &buffer[ (numFrames*numBones*16)+5 ];
        int loop = numFrames*3*2 + (4*((numFrames-1) / 2)) + 8;
        
        for( int i = 0; i < loop; i++ ){
            start[i] *= scale;
        }    
    }    
    return true;
}
    
bool aidxSubFile::Read( FILE* fp ){
    if( fp==NULL ) return false;
    
    IO_CHECK(fread(&entrySize, sizeof(int), 1, fp), 1)
    IO_CHECK(fread(&beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fread(&fileSize, sizeof(int), 1, fp), 1)
    IO_CHECK(fread(&scale, sizeof(float), 1, fp), 1)
    IO_CHECK(fread(&numFrames, sizeof(short), 1, fp), 1)
    IO_CHECK(fread(&numBones, sizeof(short), 1, fp), 1)
    IO_CHECK(fread(&type, sizeof(char), 1, fp), 1)
    
    unsigned int sz= entrySize - 9;
    if(sz > MAGIC_LENGTH(filename) + 1) {
	IO_CHECK(fread(filename,sizeof(char), MAGIC_LENGTH(filename), fp), MAGIC_LENGTH(filename))
	fprintf(stderr, "Warning: filename of length %d (bytes) truncated to:\n%s\n", sz, filename);
    }
    else {
	IO_CHECK(fread(filename,sizeof(char), sz, fp), sz)
    }
    return true;
}  

bool aidxSubFile::Write( FILE* fp ){
    if( fp==NULL ) return false;
    
    unsigned int ll =strlen(filename), esize;
    char fn[ll + 1 + suffix_len];
    
    if(perform_suffix_manip) {
        un_mangle_name(filename, fn, ll);
        ll = strlen(fn) + 1; // new length + terminating 0 byte
    }
    else {
        ll ++; // length + terminating 0 byte
    }
    esize = ll + 9;
    
    IO_CHECK(fwrite(&esize, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite(&beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite(&fileSize, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite(&scale, sizeof(float), 1, fp), 1)
    IO_CHECK(fwrite(&numFrames, sizeof(short), 1, fp), 1)
    IO_CHECK(fwrite(&numBones, sizeof(short), 1, fp), 1)
    IO_CHECK(fwrite(&type, sizeof(char), 1, fp), 1)
    
    IO_CHECK(fwrite(perform_suffix_manip ? fn : filename, sizeof(char), ll, fp), ll)
    
    return true;
}     

bool aidxSubFile::GatherInfo(){
    if( buffer==NULL ) return false;
    
    entrySize = strlen(filename) + 10;
    //scale = 1.0f;
    numFrames = ((short*)buffer)[0];
    numBones = ((short*)buffer)[1]; 
    type = ((char*)buffer)[4];   
    return Scale();
    //CheckProblemCas();
    
    //return true;
}
  
/*Animation idx file*/
void aidxFile::Allocate(){
    if( subFiles!= NULL ) delete subFiles;
    subFiles = new aidxSubFile(true);
}  
void aidxFile2::Allocate() {
    if( subFiles!= NULL) delete subFiles;
    subFiles = new aidxSubFile(false);
}

void aidxFile::Init(){
    fileVersion = 4;
    filetype    = SANM;    
}

void aidxFile2::Init() {
    fileVersion = 9;
    filetype    = SANM;
}
   
bool aidxFile::ReadHeader(){
    char checktype[MAGIC_LENGTH(SANM) ] = {0};
    IO_CHECK(fread( checktype,MAGIC_LENGTH(SANM) -1,1,fp), 1)
    if( strcmp( checktype, SANM) != 0) {
        fprintf( stderr, "File is not an animation idx archive.\n");
        return false;
    }    
    fseek( fp, 16,  SEEK_SET  ); 
    IO_CHECK(fread( &numFiles, sizeof(int), 1, fp), 1)
    return true;
} 

void aidxFile::WriteHeader(){
    fseek( fp, 0,  SEEK_SET  ); 
    fseek( fp2, 0, SEEK_SET  );
    short zero=0;
    
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp);
    fwrite( &zero, sizeof(short), 1, fp);
    fwrite( &fileVersion, sizeof(int), 1, fp);
    fwrite( &numFiles, sizeof(int), 1, fp);
   
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp2);
    fwrite( &zero, sizeof(short), 1, fp2);
    fwrite( &fileVersion, sizeof(int), 1, fp2);
    fwrite( &numFiles, sizeof(int), 1, fp2);
    
    fflush( fp );
    fflush( fp2 );
    
}   

/*Skeleton sub idx file*/
kidxSubFile::kidxSubFile(){
    len = 0;
}
    
bool kidxSubFile::Read( FILE* fp ){
    if( fp==NULL ) return false;
    
    IO_CHECK(fread( &len, sizeof(int), 1, fp ), 1)
    IO_CHECK(fread( &beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fread( &fileSize, sizeof(int), 1, fp ), 1)
    IO_CHECK(fread( filename, len ,1,fp), 1)
    
    return true;
}  

bool kidxSubFile::Write( FILE* fp ){
    if( fp==NULL ) return false;
    
    IO_CHECK(fwrite( &len, sizeof(int), 1, fp ), 1)
    IO_CHECK(fwrite( &beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite( &fileSize, sizeof(int), 1, fp ), 1)
    if(perform_suffix_manip) {
	int ll=strlen(filename);
	char fn[ll + 1 + suffix_len];
	un_mangle_name(filename, fn, ll );
    
    IO_CHECK(fwrite(fn, sizeof(char), len, fp), len)
    }
    else {
	IO_CHECK(fwrite( filename, sizeof(char), len, fp ), len)
    }
    return true;
}     
void kidxSubFile::ParseFilename( const char* fname ){
    if(no_implicit_mangling) {
        idxSubFile::ParseFilename(fname);
    }
    else {
        char* tstr = strdup( fname );
#if PATH_SEP == '\\'
        if (keep_bslash_mangling) {
            strrep( tstr, PATH_SEP, CHANGE_SEP );
        }
#endif
        int slen = strlen(tstr);
        char * str = tstr + slen - basename_length(tstr, slen);
        strncpy(filename, str, MAGIC_LENGTH(filename));
        free (tstr);
    }
}
bool kidxSubFile::GatherInfo(){
    if( buffer == NULL ) return false;
    len = strlen(filename);
    if(perform_suffix_manip) {
	char fn[len + 1 + suffix_len];
	un_mangle_name(filename, fn, len);
	len = strlen(fn);
    }
    len++;
    //fprintf(stderr, "Basename length for '%s' ==> %u\n", filename, basename_length(filename, len));
    
    return true;
}    
  
  
/*Skeleton idx file*/
void kidxFile::Allocate(){
    if( subFiles != NULL ) delete subFiles;
    subFiles = new kidxSubFile;
}  

void kidxFile::Init(){
    fileVersion = 3;
    filetype    = SSKL;    
}

void kidxFile2::Init(){
    fileVersion = 0x18000E;
    filetype    = SSKL;    
}
   
bool kidxFile::ReadHeader(){
    char checktype[MAGIC_LENGTH(SSKL) ] = {0};
    IO_CHECK(fread(checktype,MAGIC_LENGTH(SSKL) -1, 1,fp), 1)
    if( strcmp( checktype, SSKL ) != 0) {
        fprintf( stderr, "File is not a skeleton idx archive.\n");
        return false;
    }    
    fseek( fp, 16,  SEEK_SET  ); 
    IO_CHECK(fread( &numFiles, sizeof(int), 1, fp ), 1)
    return true;
} 

void kidxFile::WriteHeader(){
    fseek( fp, 0,  SEEK_SET  ); 
    fseek( fp2, 0, SEEK_SET  );
    short zero=0;
    
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp);
    fwrite( &zero, sizeof(short), 1, fp);
    fwrite( &fileVersion, sizeof(int), 1, fp);
    fwrite( &numFiles, sizeof(int), 1, fp);
   
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp2);
    fwrite( &zero, sizeof(short), 1, fp2);
    fwrite( &fileVersion, sizeof(int), 1, fp2);
    fwrite( &numFiles, sizeof(int), 1, fp2);
    
    fflush( fp );
    fflush( fp2 );
    
}   

void eidxFile::Allocate(){
    if( subFiles != NULL ) delete subFiles;
    subFiles = new eidxSubFile;
}

void eidxFile::Init(){
    fileVersion = 0x30;
    filetype    = SEVT;
}

void eidxFile2::Init(){
    fileVersion = 0x49;
    filetype    = SEVT;
}

bool eidxFile::ReadHeader(){
    char checktype[MAGIC_LENGTH(SEVT)] = {0};
    IO_CHECK(fread( checktype, MAGIC_LENGTH(SEVT) -1, 1, fp), 1)
    if( strcmp( checktype, SEVT) != 0) {
	fprintf( stderr, "File is not an event idx archive.\n");
	return false;
    }
    fseek( fp, 16, SEEK_SET );
    IO_CHECK(fread( &numFiles, sizeof(int), 1, fp), 1)
    return true;
}

void eidxFile::WriteHeader(){
    fseek( fp, 0,  SEEK_SET  ); 
    fseek( fp2, 0, SEEK_SET  );
    int zero =0;
    
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp);
    fwrite( &zero, sizeof(int), 1, fp);
    fwrite( &fileVersion, sizeof(int), 1, fp);
    fwrite( &numFiles, sizeof(int), 1, fp);
   
    fwrite( filetype, sizeof(char), strlen(filetype)+1, fp2);
    fwrite( &zero, sizeof(int), 1, fp2);
    fwrite( &fileVersion, sizeof(int), 1, fp2);
    fwrite( &numFiles, sizeof(int), 1, fp2);
    
    fflush( fp );
    fflush( fp2 );
    
}

eidxSubFile::eidxSubFile(){
    num = frameId = 0;
    tainted = false;
}

bool eidxSubFile::GatherInfo(){
    return buffer != NULL;
}

void eidxSubFile::ParseFilename( const char* name) {
    idxSubFile::ParseFilename(name);
    int len = strlen(filename), fr = len - basename_length(filename, len);

    if(isdigit(filename[fr]) == false) {
	fprintf(stderr, 
		"File name does not match '<type_number><record_name>' naming convention.\n"
		"Expected a number between 1 and 4 (inclusive), got: '%c' in: '%s'.\n",
		filename[fr],
		filename);
	tainted = true;
	return;
    } 
    int type = atoi(filename + (fr * sizeof(char)));
    if(type < 1 ||type > 4) {
	fprintf(stderr, 
		"File name does not match '<type_number><record_name>' naming convention.\n"
		"Expected a number between 1 and 4 (inclusive), got: '%d', in: '%s'.\n", 
		type, 
		filename);
	tainted=true;
    }
    else {
	frameId = type;
	tainted = false;
    }
}

char code(unsigned int num) {
    int c = (int) (log((double) num) / log(10.0f));
    return (char) (c + 97);
}

bool eidxSubFile::Read(FILE* fp) {
    if(fp == NULL || tainted) return false;
    int rlen= 0, new_frame=0;
    IO_CHECK(fread(&rlen, sizeof(int), 1, fp), 1)
    if(rlen !=4) {
	fprintf(stderr, "Bad record length: %d\nOnly records of length 4 are supported.\n", rlen);
	return false;
    }
    IO_CHECK(fread( &beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fread( &fileSize, sizeof(int), 1, fp ), 1)
    IO_CHECK(fread( &new_frame, sizeof(int), 1, fp), 1)
    if(new_frame!= frameId) {
	num = 0;
	frameId = new_frame;
    }
    num++;
    memset(filename, '\0', MAGIC_LENGTH(filename));
    sprintf(filename, "%d_%c_%d.bin", frameId, code(num), num);
    return true;
}


bool eidxSubFile::Write(FILE* fp) {
    if(fp == NULL || tainted) return false;
    int four = 4;
    IO_CHECK(fwrite(&four, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite(&beginFileOffset, sizeof(int), 1, fp), 1)
    IO_CHECK(fwrite(&fileSize, sizeof(int), 1, fp ), 1)
    IO_CHECK(fwrite(&frameId, sizeof(int), 1, fp), 1)
    return true;
}