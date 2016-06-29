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

#include "shared.h"

int create_dir ( const char* dir ) {
#ifdef _WIN32
  return _mkdir( dir );
#else
  return mkdir ( dir, 0755 );
#endif
}

/*general functions*/
int getfilesize(FILE *fp){
    struct stat st;
#ifdef _WIN32
    int fd = _fileno(fp);
#else
    int fd = fileno(fp);
#endif
    if ( fstat(fd, &st) == 0 ) {
	return st.st_size;
    }
    else {
	return -1;
    }
} 

int dir_exists(const char* dir) {
  struct stat st;
  if ( stat(dir, &st) == 0) {
    return S_ISDIR(st.st_mode) ? 1 : 0;
  }
  return -1;
}

bool copy_file (char* in, char* out) {
  str oname = str(out);
  char sep[2]= {0};
  sep[0]=PATH_SEP;
  if(dir_exists(oname.data) == 1) {
      oname += str(sep);
      oname += in;
      if(dir_exists(oname.data) == 1) {
	  fprintf(stderr, "Directory exists '%s'\n", oname.data);
	  return false;
      }
  }
  
  FILE* fin = fopen(in, "rb");
  if(!fin) {
      fprintf(stderr, "Unable to open file: '%s'\n", in);
      return false;
  }
  
  FILE* fout= fopen(oname.data, "wb");
  if(!fout) {
      fclose(fin);
      fprintf(stderr, "Unable to open file: '%s'\n", out);
      return false;
  }
  
  size_t bufsz=4096;
  void *buf = calloc(bufsz, 1);
  if (buf == NULL) {
      fclose(fin);
      fclose(fout);
      return false;
  }
  size_t read;
  bool result = true;
  while(!feof(fin) && !ferror(fout)) {
      read = fread(buf, 1, bufsz, fin);
      if(read == bufsz || feof(fin)) {
	  if(fwrite(buf, 1, read, fout) != read) {
	      fprintf(stderr, "Error while writing file: '%s'\n", oname.data);
	      result = false;
	      break;
	  }
      }
      else {
	  fprintf(stderr, "Error while reading file: '%s'\n", in);
	  result=false;
	  break;
      }
  }
  fclose(fin);
  fclose(fout);
  free(buf);
  return result;
}

/*recursive mkdir*/
bool mkrdir( const char* dir){
    int result = dir_exists( dir );
    switch(result) {
      case 0: return false;
      case 1: return true;
      default:
	result = create_dir( dir );
	if( result == 0 ){
	    return true;
	}
	else{
	    char* level = strdup( dir ), *pch;
	    strrep( level, CHANGE_SEP, PATH_SEP );
	    
	    if( (pch = strrchr(level,PATH_SEP)) ) *pch = '\0';
	    else{
		free( level );
		return false;
	    }    
	    if( mkrdir( level ) ) {
		free ( level );
		return create_dir( dir ) == 0;
	    }
	    else {
		free( level );
		return false;
	    }
	}
    }
}

/*extracts the data path from a filename*/
char* getdpath( char* str, bool fixup_slashes, bool find_data_path){
#if PATH_SEP == '\\'
    if(fixup_slashes) strrep( str, PATH_SEP, CHANGE_SEP );
#endif
    if(find_data_path) {
    for( int i=strlen(str); i>=0; i-- ) 
        if( !strncasecmp(&str[i], "data/", 5 ) ) {
            return &str[i];
        } 
    return NULL;
    }
    else {
        return str;
    }
}
/*case insensitive strstr*/
char* stristr( const char* str, const char* sstr) {
    int len = strlen(sstr);
    while(*str){
        if( !strncasecmp(str,sstr, len) ) return (char*)str;
        else str++;
    }
    return NULL;               
}
//strtok replacement
char* strsep_r( char** strp, const char* delim ) noexcept{
    if( !(*strp) ) return NULL;
    char* str = *strp;
    char* tmp = str;
        
    while( *str ){        
        if( strchr( delim, *str ) ) {
            *str = '\0';
            *strp = str + 1;
            return tmp; 
        } 
        str++;              
    }
    *strp = NULL;
    return tmp;
}
//same as strsep() but anything within "" is exempted from the delimators
char* qstrsep( char** strp, const char* delim ){
    if( !(*strp) ) return NULL;
    char* str = *strp;
    char* start = str;
    char* buffer = strdup( str );
    char* pbuff = buffer;
    bool  inquotes = false;
    int   len = strlen( buffer );
    memset( buffer, 0, len );
    
    while( *str ){
        if( inquotes ){
            if( *str == '\"' ){
                str++;
                inquotes = false;
                if( *str == '\0' ){
                    strcpy( start, buffer );
                }    
                continue;
            }
        }        
        else if( *str == '\"' ){
            str++;
            inquotes = true;
            continue;
        }    
        else if( strchr( delim, *str ) ){
            strcpy( start, buffer );
            free( buffer );
            *str = '\0';
            *strp = str + 1;            
            return start; 
        }
        *pbuff++ = *str++;
    }
    
    free( buffer );
    *strp = NULL;
    return start;
}    

//replaces one char with another in a string, returns the number replaced. 
int strrep( char* str, int c, int r ){
    int count = 0;
    while( *str ){
        if( *str == c ) {
            *str = (char) r;
            count++;
        }    
        str++;
    }    
    return count;
}    



