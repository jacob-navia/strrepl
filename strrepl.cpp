/* This code was written by Bonita Montero (strReplace) and Jacob Navia (main) */
#include <iostream>
#include <string>

using namespace std;

string strReplace( string const &findIn, string const &find, string const &replaceWith )
{
    size_t findLength = find.length();
    if( findIn.length() < findLength )
        return findIn;
    size_t findEnd = findIn.length() - findLength;
    size_t size    = 0,
           i;
    for( i = 0; i <= findEnd; )
        if( findIn.compare( i, findLength, find ) != 0 )
            ++size,
            ++i;
        else
            size += replaceWith.length(),
            i    += findLength;
    string ret;
    ret.reserve( size + findIn.length() - i );
    for( i = 0; i <= findEnd; )
        if( findIn.compare( i, findLength, find ) != 0 )
            ret += findIn[i++];
        else
            ret += replaceWith,
            i   += findLength;
    for( ; i != findIn.length(); ++i )
        ret += findIn[i];
    return move( ret );
}

#include <cstdio>
string Str;
int main(int argc,char *argv[])
{
    char *buffer;
    size_t siz;
    FILE *f; 
	const char *tmp;

    if (argc < 5)  {   
        printf("Usage: %s: <file name> <str-to-find> <replacement> <output-file>",
            argv[0]); 
        exit(1); 
    }   
    f = fopen(argv[1],"rb");
    if (f == NULL) { printf("Impossible to open %s\n",argv[1]);exit(2);}
    if (fseek(f,0,SEEK_END)) {printf("Impossible to seek to end\n");exit(3);}
    siz = ftell(f);
    if ((int)siz < 0) {printf("Impossible to query position\n"); exit(4);}
    buffer = (char *)malloc(siz+1);
    if (buffer == NULL) {printf("No more memory\n"); exit(6);}
    fseek(f,0,SEEK_SET);
    if (siz != fread(buffer,1,siz,f)) { printf("ReadError\n"); exit(5);}
    fclose(f);
    buffer[siz]=0;
	for (int i=0; i< 100; i++)
    	Str = strReplace(buffer,argv[2],argv[3]);
	//cout << Str;	
	tmp = Str.c_str();
    f = fopen(argv[4],"wb");
    fwrite(tmp,1,Str.size(),f);
    fclose(f);
    return 0;
}

