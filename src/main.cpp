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

#include <string.h>
#include <assert.h>
#include "idx.h"
#include "main.h"
#include "shared.h"

#define EXTRACT 1
#define CREATE 2
#define LIST 3
#define IDXSOUND 1
#define IDXANIM  2
#define IDXSKEL  3
#define IDXSEVT  4

#define DEFAULT_MANIP_SUFFIX "bin"
#define DEFAULT_OUTPUT_FNAME "default"
   
/*globals*/
bool verbose;
bool perform_suffix_manip;
bool no_implicit_mangling;
bool keep_bslash_mangling;
const char * suffix_pattern;
int suffix_len;

int usage(int code) {
    fprintf( stdout,
    "Usage: [options] [files]\n"
    "  -a      idx pack(s) are animation packs, by default they are sound\n"
    "  -e      idx pack(s) are event packs\n"
    "  -s      idx pack(s) are skeleton packs\n"
    "  -m      idx packs(s) are Medieval II packs, only applies when creating packs.\n"
    "  -c      create. Uses '"DEFAULT_OUTPUT_FNAME"' as output file name by default.\n"
    "  -x      extract\n"
    "  -t      list\n"
    "  -f      output file (output filename must follow)\n"
    "          Omitting this option when creating packs is equivalent to '-f "DEFAULT_OUTPUT_FNAME"'\n"
    "  -v      verbose output\n"
    "  -B      strips a given suffix from filenames when packing, adds it when extracting\n"
    "  -b      short for '-B "DEFAULT_MANIP_SUFFIX"' for compatibility with XIDX prior to version 1.5\n"
    "  -p      preserve paths; disable all implicit path mangling\n"
#if PATH_SEP == '\\'
    "          This option does not disable replacing backslashes (\\) with slashes (/).\n"
#else
    "          Equivalent to -P on this OS/platform (but recognised separately for consistency).\n"
#endif
    "  -P      preserve paths; disable all implicit path mangling\n"
#if PATH_SEP == '\\'
    "          This option also disables replacing backslashes (\\) with slashes (/).\n"
#endif
    "  -h      print this help\n"
    "  --help  print this help\n"
    "  --shell gives you a simple \'shell\' for entering commands\n\n"
    "Please see the readme for more information\n"
    );
    return code;
}

char idx_type_flag(int type) {
    switch(type) {
	case IDXANIM:
	    return 'a';
	case IDXSEVT:
	    return 'e';
	case IDXSKEL:
	    return 's';
	default:
	    return '\0';
    }
}

int idx_type_check(int& type, int to_check) {
    if( type != IDXSOUND ) {
	char opt = idx_type_flag(type);
	if ( type == to_check ) {
	    fprintf(stderr, "Ignored duplicate pack type option: '-%c'\n", opt);
	}
	else {
	    fprintf(stderr, "Conflicting pack type options: '-%c' and '-%c'\n" , opt, idx_type_flag(to_check));
	    return usage(-3);
	}
    }
    else {
	type = to_check;
    }
    return 0;
}

char func_type_flag(int func) {
    switch(func) {
	case CREATE:
	    return 'c';
	case LIST:
	    return 't';
	default:
	    return 'x';
    }
}

int func_type_check(int& func, int to_check) {
    if ( func != 0 ) {
	char opt = func_type_flag(func);
	if (func == to_check) {
	    fprintf(stderr, "Ignored duplicate operation option: '-%c'\n", opt);
	}
	else {
	    fprintf(stderr, "Conflicting operation options: '-%c' and '-%c'\n", opt, func_type_flag(to_check));
	    return usage(-2);
	}
    }
    else {
	func = to_check;
    }
    return 0;
}

bool func_create_flag_check(int func, bool& current_value, char opt) {
    if(current_value) {
        fprintf(stderr, "Ignoring redundant '-%c' option.\n", opt);
        return false;
    }
    if(func == 0 || func == CREATE) {
        current_value = true;
        return true;
    }
    else {
        fprintf(stderr, func == EXTRACT ? "The '-%c' flag does not affect extracting packs.\n": "The '-%c' flag does not affect listing packs.\n", opt);
        return false;
    }
}

void func_other_flag_check(int func, bool value, char opt) {
    if(value) {
        fprintf(stderr, func == EXTRACT ? "The '-%c' flag does not affect extracting packs.\n": "The '-%c' flag does not affect listing packs.\n", opt);
    }
}

int other_flag_checks(int& function, bool medieval2, int set, char pOpt) {
    int code = func_type_check(function, set);
    if(code == 0) {
        func_other_flag_check(function, medieval2, 'm');
        func_other_flag_check(function, no_implicit_mangling, pOpt);
    }
    return code;
}

bool misc_redundant_flag_check(bool& value, char opt) {
    if(value) {
        fprintf(stderr, "Ignoring redunant '-%c' flag.\n", opt);
        return false;
    }
    else {
        value = true;
        return true;
    }
}
        
int main(int argc, char *argv[]){
    int   function = 0;
    char* ofile    = NULL;
    int   idxType  = IDXSOUND;  
	  verbose  = false;
    perform_suffix_manip = false;
    no_implicit_mangling = false;
    keep_bslash_mangling = true;
    bool medieval2 = false;
    bool suffix = false;
    stack<const char*> files(1024);
    stack< str > infiles(1024);
    int code = 0;
    char pOpt = 'P';

    fputs( "idx extractor/packer\nVersion 1.6.2\nMade by The Europa Barbarorum Team, based on the tool by Vercingetorix\n\n", stdout );
  
    
    if( argc == 1 ) return usage(1);
    for(int i=1; i<argc; i++){
      if( argv[i][0] == '-' ){
	char *pstr = &(argv[i][1]);
	while( *pstr ){
	    switch ( *pstr ){
		case 'm':
		    func_create_flag_check(function, medieval2, 'm');
            break;
        case 'p':
            func_create_flag_check(function, no_implicit_mangling, 'p');
            if(function == 0) {
                pOpt = 'p';
            }
#if PATH_SEP != '\\'
            fputs("Warning: '-p' is equivalent to '-P' on this OS/platform (but recognised separately for consistency)\n", stderr);
#endif
            break;
        case 'P':
            if(func_create_flag_check(function, no_implicit_mangling, 'P')) {
                keep_bslash_mangling = false;
            }
            break;
		case 'b':
            if(misc_redundant_flag_check(perform_suffix_manip, 'b')) {
                suffix_pattern = DEFAULT_MANIP_SUFFIX;
                suffix_len = MAGIC_LENGTH(DEFAULT_MANIP_SUFFIX) - 1;
            }
		    break;
        case 'B':
            if(misc_redundant_flag_check(perform_suffix_manip, 'B')) {
                suffix = true;
            }
            break;
		case 'a':
		    code = idx_type_check(idxType, IDXANIM);    
		    break;
		case 'e':
		    code = idx_type_check(idxType, IDXSEVT);    
		    break;
		case 's':
		    code = idx_type_check(idxType, IDXSKEL);    
		    break;                  
        case 'x':
            code = other_flag_checks(function, medieval2, EXTRACT, pOpt);
		    break;
		case 'c':
		    code = func_type_check(function, CREATE);
		    break;
		case 't':
            code = other_flag_checks(function, medieval2, LIST, pOpt);
            break;
		case 'v':
		    misc_redundant_flag_check(verbose, 'v');
		    break;
		case 'f':
		    i++;
		    if( !(i<argc) ) {
			fputs( "The '-f' flag requires an output file.\n", stderr );
			return usage(-4);
		    }
		    ofile = argv[ i ]; 
		    break;
        case 'h':
            return usage(0);
		case '-':
		    if( 0 == strcmp( argv[i], "--help") ) return usage(0);
		    else if( 0 == strcmp( argv[i], "--shell" ) ){
			xidxShell shell;
			return shell.Run();
		    }
		    else {
		      fprintf( stderr, "Unkown option \"%s\"\n", pstr );
		      return usage(-6);
		    }
		default:
		    fprintf( stderr, "Unkown option '%c'\n",*pstr );
		    return usage(-6);
		} /*case*/
		if(code!=0) return code;
		pstr++;
	    }/*pstr*/     
	}/*argument parse*/
	else
        {
            if(suffix)
            {
                suffix_pattern = argv[i];
                suffix_len = strlen(suffix_pattern);
                suffix = false;
            }
            else 
            {
                files.push( argv[i] );
            }
        }
    }/*for loop*/
    
    if (function == 0) {
	fputs("Missing operation option.\n", stderr);
	return usage(-1);
    }
    else if ( function != CREATE && files.empty()) {
	fputs("Missing file arguments.\n", stderr);
	return usage(-5);
    }
    
    idxFile *idx;
    switch(idxType) {
	case IDXANIM:
	    idx= medieval2 ? new aidxFile2 : new aidxFile;
	    break;
	case IDXSKEL:
	    idx= medieval2 ? new kidxFile2 : new kidxFile;
	    break;
	case IDXSEVT:
	    idx = medieval2 ? new eidxFile2 : new eidxFile;
	    break;
	default:
	    idx= medieval2 ? new sidxFile2 : new sidxFile;
	    break;
    }
    int result = 0;
    fputs("\n", stdout);
    
    switch (function){
	case EXTRACT:
	    for(int i=0; i<files.GetSize(); i++){
		fprintf( stderr, "Extracting files in '%s'...\n", files[i] );
		if (idx->Extract( files[i], false ) ) {
		    fprintf( stderr, "Sucessfully extracted files from pack: '%s'\n", files[i]);
		}
		else {
		    fprintf( stderr, "Failed to extract files from pack: '%s'\n", files[i]);
		    result = 2;
		}
	    }
	    break;
	case CREATE:
    {
        /*read filenames from stdin*/
        if( files.empty() ){
            char line[256] = {0};
            while( fgets( line, 255, stdin ) ) {
                char * eol= strpbrk(line,"\n\r");
                if(eol) *eol = '\0';
                infiles.push( line );
            } 
            for(int i=0; i<infiles.GetSize(); i++) {
                files.push( infiles[i].data );
            }
        }
        fprintf( stderr, "Creating idx file...\n" );
        if(ofile == NULL) {
            ofile = (char *) DEFAULT_OUTPUT_FNAME;
            fprintf(stderr, "Warning : no output file name specified (use the -f option), will use '%s'\n", ofile);
        }
        if(files.empty()) {
            fprintf(stderr, "Warning : creating empty pack '%s'\n", ofile);
        }
        int numFiles = files.GetSize();
        if (idx->Write( ofile, files.GetData(), numFiles) ) {
            fprintf(stderr, "Successfully created pack: '%s' (packed %d files)\n", ofile, numFiles);
        }
        else {
            fprintf(stderr, "Failed to create pack: '%s'\n", ofile);
            result = 2;
        }
    }
    break;
	default:
	    for(int i=0; i<files.GetSize(); i++){
		fprintf( stderr, "\nListing files in '%s'...\n", files[i] );
		if(!idx->Extract( files[i], true )) {
		    fprintf(stderr, "Failed to list files in pack: '%s'\n", files[i]);
		    result =2;
		}
	    }   
	    break;
    } 
    delete idx;           		
    return result;    
}

//XIDX 'shell'

typedef int (*FNPTR_INT_VOID_PTR)(void*);
static xidxFunc 
functions[] = {
                xidxFunc( "help:?:man", HELP_HELP_STR, &xidxShell::help ),
                xidxFunc( "exit:quit", "exits xidx", &xidxShell::exit ),
                xidxFunc( "scale", HELP_SCALE_STR, &xidxShell::scale ),
                //todo://xidxFunc( "skel", HELP_SKEL_STR, &xidxShell::loadskel ),
                xidxFunc( "copy:cp", HELP_COPY_STR, &xidxShell::copy),
                xidxFunc( "chpath", HELP_CHPATH_STR, &xidxShell::chpath ),
                #ifdef _WIN32
                xidxFunc( "clear:cls:clr", HELP_CLEAR_STR, &xidxShell::clear ),
                #endif
                xidxFunc( "mkdir", HELP_MKDIR_STR, &xidxShell::mkdir ),
                xidxFunc( "fork:~:`", HELP_FORK_STR, &xidxShell::fork )
              };    
                                
xidxShell::xidxShell(){
    memset((void*) input, '\0', MAGIC_LENGTH(input));
}

xidxShell::~xidxShell(){
}
int xidxShell::numargs( const char** args ){
    int count = 0;
    while( *args ) {
        args++;
        count++;
    }
    return count;
}
//omits empty tokens
stack<char*> xidxShell::gettokens( char* str, const char* delim, bool quotes ){
    char** strp = &str;
    char* pstr;
    char* (*tokenizer)( char**, const char*) = quotes ? &qstrsep : strsep;
    stack<char*> tokens;
    
    pstr = tokenizer( strp, delim );
    while( pstr ){       
        if( pstr && *pstr != '\0' ){
            tokens.push( pstr );
            //puts( pstr );
        }    
        pstr = tokenizer( strp, delim );        
    }    
    return tokens;
}    
xidxFunc* xidxShell::getfn( const char* name ){
    for( int i = 0, size = sizeof(functions)/sizeof(xidxFunc); i < size; i++ ){
        char* str = strdup( functions[i].name );
        char* pstr;
        pstr = strtok( str, ":" );
        while( pstr ){           
            if( !strcasecmp( name, pstr ) ){
                free( str );
                return &functions[i];    
            }
            pstr = strtok( NULL, ":" );
        }
        free( str );        
    }
    fprintf( stderr, "error: `%s` is not a valid command try `help`\n", name );
    return NULL;
}

int xidxShell::readfile( const char* filename, const char* mode, char*& buffer,
                            FILE*& fp, unsigned int& fsize ){
    fp = fopen( filename, mode );
    if( !fp ){
        fprintf( stderr, "error: failed to open file `%s`\n", filename );
        return -2;
    }
    
    int size = getfilesize(fp);
    if (size == -1 ) {
	fprintf (stderr, "error: cannot stat: `%s`\n", filename );
	return -5;
    }
    fsize = size;
    void * buf = calloc(fsize, sizeof(char));
    if( ! buf ){
        fprintf( stderr, "error: failed to allocate memory\n" );
        return -3;
    }
    if( fread( buf, 1, fsize, fp ) != fsize ){
        fprintf( stderr, "error: an error occured while reading file\n" );
        free( buf );
        return -4;
    }
    buffer = (char *) buf;
    fseek( fp, 0, SEEK_SET );
    return 0;
}    
        
int xidxShell::help( int argc, char** argv ){
    xidxFunc* function = NULL;
    
    if( argc != 0 ){
        for( int i = 0; i < argc; i++ ){
            function = getfn( argv[i] );
            if( !function ) return -1;
            fprintf( stdout, "%s\n", function->usage );
        }    
    }    
    else{
        fprintf( stdout, "commands are:\n" );
        for( int i = 0, size = sizeof(functions)/sizeof(xidxFunc); i < size; i++ ){
            fprintf( stdout, "    " );
            for( int k = 0; functions[i].name[k] && functions[i].name[k]!=':'; k++)
                fputc( functions[i].name[k], stdout );
        }                 
        fprintf( stdout, "\nfor command usage type help <command>\n" );
    }
    
    return 0;
}
int xidxShell::scale( int argc, char** argv ){
    FILE* fp;
    unsigned int fsize;
    char* buffer;
    int   result;

    if( argc != 1 && argc != 2 ){
        fputs( "incorrect number of arguments. try `help scale`\n", stderr );
        return -1;
    }
    
    result = readfile( argv[0], "r+b", buffer, fp, fsize );
    if( result !=0 && result != -2) {
	fclose(fp);
	return result;
    }
    else {
	if( result == -2 )  return result;
    } 
    
    
    float scale = *(float*)(buffer);
    fprintf( stdout, "current scale is %4.2f\n", scale );
    
    if( argc == 2 ){
        float newscale = atof( argv[1] );
        int nbones = *(short*)(buffer+4);
        float* bnpos = (float*)(buffer+16);
        
        fprintf( stdout, "new scale is %4.2f\n", newscale );
        
        *(float*)(buffer) = newscale;
        scale = newscale / scale;
        
        for( int i = 0; i < nbones; i++ ){
            bnpos[0] *= scale;
            bnpos[1] *= scale;
            bnpos[2] *= scale;
            bnpos += 6;
        }
        fseek( fp, 0, SEEK_SET );
        fwrite( buffer, 1, fsize, fp );
    }
    
    free( buffer );
    fclose( fp );
    return 0;            
}
    
int xidxShell::fork( int argc, char** argv ){
    char tmpstr[256];
    str final=str("");
    for( int i = 0; i < argc; i++ ){
        if( strchr( argv[i], ' ' ) ){
            snprintf( tmpstr, 255, "\"%s\"", argv[i] );         
        }
        else {
	    strncpy( tmpstr, argv[i], sizeof(tmpstr) );
	}
        final += str(tmpstr);
	if(argc -1 > i) {
	    final += str(" ");
	}
    }
    
    return system( final.data );
}

int xidxShell::copy( int argc, char** argv) {
    if(argc < 1) {
	fputs("error: missing <filename> parameter. try `help copy`\n", stderr);
    }
    if(argc < 2) {
	fputs("error: missing <destination> parameter. try `help copy`\n", stderr);
	return 1;
    }
    if(copy_file(argv[0], argv[1])) {
	return 0;
    }
    else {
	fprintf(stderr, "error: unable to copy '%s' to '%s'\n", argv[0], argv[1]);
	return 2;
    }
}

int xidxShell::mkdir( int argc, char** argv) {
    if (argc < 1 ) {
      fputs("error: missing <directory> parameter. try `help mkdir`\n", stderr);
      return 1;
    }
    int result=0;
    for(int i=0; i<argc; ++i) {
      if(! mkrdir(argv[i])) {
	fprintf(stderr, "error: unable to create directory: `%s`\n", argv[i]);
	++result;
      }
    }
    return result;
}
    
int xidxShell::loadskel( int argc, char** argv ){
    //todo
    fputs("stub:: loadskel() not implemented yet, probably never will be.\n", stderr);
    return 0;
}

int xidxShell::chpath( int argc, char** argv ){
    const char* skeleton = NULL;
    const char* oldfn = NULL;
    const char* newfn = NULL;
    bool  wildcard = false;
    bool  list = false;
    bool  copy = false;
    FILE* fp = NULL;
    char* buffer;
    unsigned int   count = 0, fsize = 0;
    
    for( int i = 0; i < argc; i++ ){
        strrep( argv[i], '\\', '/' );
        if( !strcasecmp( argv[i], "all" ) ) wildcard = true;
        else if( 0 != strcasecmp( argv[i], "list" ) ) list = true;
        else if( 0 != strcasecmp( argv[i], "copy" ) ) copy = true;
        else if( skeleton == NULL ) skeleton = argv[i];
        else if( oldfn == NULL ) oldfn = argv[i];
        else if( newfn == NULL ) newfn = argv[i];
	else {
	    fprintf(stderr, "error: bad argument: '%s'. try `help chpath`\n", argv[i]);
	    return -1;
	}
    }
    if( (list && newfn) || (!list && !newfn) || (copy && !newfn) || !oldfn || !skeleton ){
        fprintf( stderr, "error: incorrect number of arguments. try `help chpath`\n");
        return -1;
    }
    //read in file
    int result = readfile( skeleton, "rb", buffer, fp, fsize );
    if( result !=0 && result != -2) {
	fclose(fp);
	return result;
    }
    else {
	if( result == -2 )  return result;
	else fclose( fp );
    }
       
    if( !newfn && list ) newfn = "";
    //search and replace
    for(unsigned int i = 0, len = strlen(oldfn), len2 = strlen(newfn); i < fsize; i++ ){
        if( !strncasecmp( &buffer[i], oldfn, len ) ){
            int diff = len - len2;
            
            if( list ){
                fprintf( stdout, "%s\n", &buffer[i] );
            }
            if( copy ){                
                char* old = strdup( &buffer[i] );
                char* tmp = (char*)calloc( strlen( old ) + 1 + len2,  sizeof(char) );
                strcpy( tmp, newfn );
                strcat( tmp, &old[len] ); 
#if PATH_SEP == '\\'
                strrep( old, CHANGE_SEP, PATH_SEP );
                strrep( tmp, CHANGE_SEP, PATH_SEP );
#endif
                char* pch = strrchr( tmp, PATH_SEP );
                if( pch ){
                    *pch = '\0';
                    mkrdir( tmp );
                    *pch = PATH_SEP;
                }
                
                copy_file(old, tmp);
                
		free( tmp );
                free( old );
            }
            if( !list && !copy ){
                if( diff < 0 ){
                    fsize += -diff;
                    buffer = (char*)realloc( buffer, fsize ); 
                }
                else{
                    fsize -= diff;
                }
                memmove( &buffer[i+len2], &buffer[i+len], fsize - (len2+i) );
                memcpy( &buffer[i], newfn, len2 );
                count++;
            }    
            if( !wildcard ) break;
        }    
    }
    //write out file
    if( !list && !copy ){        
        fp = fopen( skeleton, "wb" );
        fwrite( buffer, 1, fsize, fp );        
        fclose( fp );
        fprintf( stdout, "replaced %d %s\n", count, count==1 ? "string" : "strings");
    }
    free( buffer );    

    return 0;
}

int xidxShell::exit( int argc, char** argv ){
    return 0;
}

#ifdef _WIN32
int xidxShell::clear( int argc, char** argv ){
    return system( "cls" );
} 
#endif

int xidxShell::Parse( char* input2 , xidxFunc* exit_func){
    char command[sizeof(input)] = {0};
    char args[sizeof(input)] = {0};
    xidxFunc* function = NULL;
    
    sscanf( input2, "%s %[^\n]\n", command, args );
    if( command[0] == '\0' ) return 0;
    
    function = getfn( command );     
    if( !function ) return -1;
    else if( function == exit_func) return -2;
    else {
	stack< char* > tokens = gettokens( args, " " );
	tokens.push( NULL );
	return function->execute( tokens.size() - 1, tokens.data() );
    }
}   
 
int xidxShell::Run(){
    xidxFunc* exit_func= getfn("exit");
    fprintf( stderr, "(xidx) " );
    while( fgets( input, sizeof(input), stdin ) ){
        for( int i = 0, loop = 0, front = 0; input[i]; i++ ){
            if( loop != 0 ){
                if( input[i] == '\"' ){
                    loop = 0;
                    continue;
                }
            }        
            else if( input[i] == '\"' ){
                loop = 1;
                continue;
            }
            else if( input[i] == ';' || input[i] == '\n' ){
                input[i] = '\0';                
                if(Parse( &input[front], exit_func ) == -2) return 0;
                front = ++i;
            }    
            
        }  
        fprintf( stderr, "(xidx) " );       
    }
 
    return 0;
}

