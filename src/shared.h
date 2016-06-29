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

#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define PATH_SEP '\\'
#define CHANGE_SEP '/'
#else
#define PATH_SEP '/'
#define CHANGE_SEP '\\'
#endif

int getfilesize(FILE *); 
char* getdpath(char*, bool fixup_slashes, bool find_data_path);
char* stristr(const char*,const char*);
char* strsep( char**, const char* ) throw();
char* qstrsep(char**, const char* );
int   strrep( char*, int, int );
bool  mkrdir(const char*);
bool  copy_file( char*, char*);

class str;
template <class T> class stack;

        
/*I have to write a simple stack unless I want to include <vector> and be
  subject to a program bloat of 900kb! Never.*/
template <class T>
class stack{
    public:
        stack( int c = 2 ){
            m_cap  = c;
            m_data = new T[ m_cap ];
            m_size = 0;
        };
        stack( const stack<T>& );
        ~stack();
        T& operator [] (int i){
            return this->m_data[i];
        }
        stack<T>& operator=( const stack<T>& st );       
        void push(T);
        T pop();
        T* search( T* )const;
        T* search( T )const;
        void clear();
        bool empty()const{ return !m_size; }
        int GetSize()const{ return m_size; }
        int size()const{ return m_size; }
        T*  GetData()const{ return m_data; }
        T*  data()const{ return m_data; }
     private:
         T*  m_data;
         int m_size;
         int m_cap;
};

template <class T>
stack<T>::stack( const stack<T>& st ){
    this->operator=( st );
}
template <class T>
stack<T>& stack<T>::operator=( const stack<T>& st ){
    clear();
    m_size = st.m_size;
    m_cap = st.m_cap;
    m_data = new T[m_cap];
    for( int i = 0; i < m_size; i++){
        m_data[i] = st.m_data[i];
    } 
    return *this;   
}
template <class T>
stack<T>::~stack(){
    clear();
}
template <class T>
void stack<T>::clear(){
    if( m_data ) delete [] m_data;
    m_data = NULL;
    m_size = m_cap = 0;
}
template <class T>
T stack<T>::pop(){
    m_size--;
    if( m_size<0 ){
        m_size = 0;
        return (T)0;
    }    
    return m_data[ m_size ];
}    
template <class T>
void stack<T>::push( T d){
    if( m_size == m_cap ){ 
        m_cap *= 2;
        T* tmp = new T[ m_cap ];
        for(int i=0; i<m_size; i++ ) tmp[i] = m_data[i];
        delete [] m_data;
        m_data = tmp;        
    } 
     
    m_data[ m_size ] = d;
    m_size++;
                 
}
template <class T>
T* stack<T>::search( T d )const{
    return search( &d );
}    
template <class T>
T* stack<T>::search( T* d )const{
    T* result = NULL;
    for( int i=0; i<m_size && !result; i++) 
        if( m_data[i] == *d ) result = &m_data[i];
    return result;
}    
class str{
    public:
	str( const char* s = NULL ){
            if( s ){
                data = new char[ strlen( s ) +1 ];
                strcpy( data, s);
            } 
            else data = NULL;   
        } 
        str( const str& s){
            str nstr( s.data );
            *this = nstr;
        }         
        ~str(){ clear(); }
        void clear(){ if( data ) delete [] data; data = NULL; }
        void resize( unsigned int s ){
            if( data && (s <= strlen(data)) ) return;
            char* tmp = new char[s];
            *tmp = '\0';
            if( data ) {
                strcpy( tmp, data );
                clear();
            }    
            data = tmp;
        }
        /*
         * Commented out because of lurking segfaults... 
        str operator+( char s){
            str tmp = *this;
            char tmp2[2]={0};
            *tmp2 = s;
            tmp += tmp2;
            return tmp;
        }
        str operator+( const char* s){
            str tmp = *this;
            tmp += s;
            return tmp;
        }
        */
        str operator+( str s ){
            return this->operator+( s.data );
        }        
        str& operator+=( const char* s ){
            unsigned int size = 1;
            if( data ) size += strlen( data );
            if( s ) size += strlen( s );
            resize( size );            
            strcat( data, s );
            return *this;
        }    
        str& operator+=( const str& s ){
            this->operator+=( s.data );
            return *this;
        }    
        str& operator = ( const str& s) { 
            return this->operator=( s.data );
        }
        str& operator = (const char* s) { 
            if( s ){
                clear();
                data = new char[ strlen( s ) +1 ];
                strcpy( data, s );
            }    
            return *this;        
        }
        bool operator == ( str& s){
            return !strcmp( data, s.data );
        }
        bool operator == ( const char* pstr){
            return !strcmp( data, pstr );
        }     
        char* data;
    
};

#endif
