/* This code was written by Bonita Montero (strReplace) and Jacob Navia (main) */
#include <iostream>
#include <algorithm>
#include <string>
#include <array>
#include <cstdint>
#include <climits>
#include <cstring>

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
#include <vector>
void strReplace2( string const &findIn, string const &find, string const &replaceWith, string &ret )
{
    ret.resize( 0 );
    size_t findLength = find.length();
    if( findLength == 0 || findIn.length() < findLength )
        return;
    std::vector<size_t> offsets;
    size_t         findLast = findIn.length() - findLength,
                   i;
    offsets.reserve( 128 );
    for( i = 0; i <= findLast; )
        if( findIn.compare( i, findLength, find ) != 0 )
            ++i;
        else
        {
            size_t cap = offsets.capacity();
            if( cap == offsets.size() )
                offsets.reserve( cap * 2 );
            offsets.emplace_back( i );
            i += findLength;
        }
    ret.reserve( findIn.length() - offsets.size() * find.length() + offsets.size() * replaceWith.length() );
    i = 0;
    for( size_t offset : offsets )
    {
        ret.append( findIn, i, offset - i );
        ret.append( replaceWith );
        i = offset + findLength;
    }
    ret.append( findIn, i, findIn.length() - i );
} 
void strReplace3( string const &findIn, string const &find, string const &replaceWith, string &ret )
{
    using str_cit = string::const_iterator;
    ret.clear();
    size_t const findLength = find.length();
    if( findLength == 0 || findIn.length() < findLength )
    {
        ret = findIn;
        return;
    }
    struct Occurence
    {
        unsigned char c;
        size_t        offset;
        Occurence( unsigned char c, size_t offset ) :
            c( c ),
            offset( offset )
        {
        }
    };
    vector<Occurence> occurences;
    occurences.reserve( findLength );
    for( size_t i = 0; i != findLength; ++i )
        occurences.emplace_back( find[i], i );
    std::sort( occurences.begin(), occurences.end(),
        []( Occurence const &left, Occurence const &right ) -> bool
        {
            return left.c < right.c || left.c == right.c && left.offset < right.offset;
        } );
    using voc_it = vector<Occurence>::iterator;
    struct OccHeader
    {
        voc_it occ;
        size_t n;
        OccHeader() = default;
        OccHeader( voc_it occ, size_t n ) :
            occ( occ ),
            n( n )
        {
        }
    };
    array<OccHeader, 1 << CHAR_BIT> occHeaders;
    fill( occHeaders.begin(), occHeaders.end(), OccHeader( occurences.end(), 0 ) );
    for( voc_it occ = occurences.begin(), firstOcc = occ; ; )
        if( occ != occurences.begin() && occ != occurences.end() && occ->c == occ[-1].c )
            ++occ;
        else
        {
            unsigned char c;
            if( occ != occurences.begin() )
                c               = occ[-1].c,
                occHeaders[c].n = occ - firstOcc,
                firstOcc        = occ;
            if( occ != occurences.end() )
                c                 = occ->c,
                occHeaders[c].occ = occ,
                ++occ;
            else
                break;
        }
    auto reverseCmp = []( str_cit begin, str_cit cmp, size_t len ) -> str_cit
    {
        str_cit scn = begin + len;
        cmp += len;
        for( ; scn > begin; --scn, --cmp )
            if( scn[-1] != cmp[-1] )
                return scn;
        return scn;
    };
    str_cit         it       = findIn.begin(),
                    findLast = findIn.end() - findLength,
                    lastEqCh;
    size_t const    rwLength = replaceWith.length();
    size_t          size     = 0;
    vector<str_cit> spots;
    for( ; it <= findLast; )
        if( (lastEqCh = reverseCmp( it, find.begin(), findLength )) != it )
        {
            str_cit    lastNotEqCh = lastEqCh - 1;
            OccHeader &oh          = occHeaders[(unsigned char)*lastNotEqCh];
            if( oh.occ != occurences.end() )
            {
                size_t notEqDistance = lastNotEqCh - it;
                voc_it firstOcc      = oh.occ;
                size_t n             = oh.n;
                do
                    if( firstOcc[n / 2].offset >= notEqDistance )
                        n /= 2;
                    else
                        firstOcc += n / 2,
                        n        -= n / 2;
                while( n >= 2 );
                if( firstOcc->offset < notEqDistance )
                    size += lastNotEqCh - firstOcc->offset - it,
                    it    = lastNotEqCh - firstOcc->offset;
                else
                    size += findLength,
                    it   += findLength;
            }
            else
                size += lastEqCh - it,
                it    = lastEqCh;
        }
        else
        {
            if( spots.size() == spots.capacity() )
                spots.reserve( spots.capacity() * 2 );
            spots.emplace_back( it );
            size += rwLength;
            it   += findLength;
        }
    ret.resize( size + (findIn.end() - it) );
    string::iterator retIt = ret.begin();
    it = findIn.begin();
    for( str_cit spot : spots )
        memcpy( &*retIt, &*it, spot - it ),
        retIt += spot - it,
        memcpy( &*retIt, &*replaceWith.begin(), rwLength ),
        retIt += rwLength,
        it     = spot + findLength;
    memcpy( &*retIt, &*it, findIn.end() - it );
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
    	strReplace3(buffer,argv[2],argv[3],Str);
	//cout << Str;	
	tmp = Str.c_str();
    f = fopen(argv[4],"wb");
    fwrite(tmp,1,Str.size(),f);
    fclose(f);
    return 0;
}

