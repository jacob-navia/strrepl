/* STRing REPLace, several programs that substitute one word for another in a buffer 
All timings were done with a file of 4 351 187 bytes containing the text of the bible
(bible.txt).
The machine used for the tests is 
jacob@rock64:~/strrepl$ uname -a
Linux rock64 4.4.152-rockchip64 #1 SMP Sun Aug 26 14:40:54 CEST 2018 aarch64 GNU/Linux

The gcc used is
jacob@rock64:~/strrepl$ gcc -v
Using built-in specs.
COLLECT_GCC=gcc
COLLECT_LTO_WRAPPER=/usr/lib/gcc/aarch64-linux-gnu/6/lto-wrapper
Target: aarch64-linux-gnu
gcc version 6.3.0 20170516 (Debian 6.3.0-18+deb9u1) 
*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if 1
#define Strlen strlen
#else
/*
This version of strlen is one of the fastest, published by D. E. Knuth.
If your version of strlen is slow, use this one.
*/
#define H 0x8080808080808080ULL
#define L 0x0101010101010101ULL
static size_t Strlen(const char *s)
{
	unsigned long long t;
	const char *save = s;
	
	while (1) {
		// This supposes that the input string is aligned
		// or that the machine doesn't trap when reading
		// a 8 byte integer at a random position like the
		// x86
		t = *(unsigned long long *)s;
		if (H & (t - L) & ~t)
			break;
		s += sizeof(long long);
	}
	// This loop will be executed at most 7 times
	while (*s) {
		s++;
	}
	return s - save;
}
#endif
#if 0
/*
This version was contributed by jacob navia. It stores into a flexible array all 
the hits, and uses that information later for the replacement.
jacob@rock64:~/strrepl$ time ./strrepl-c bible.txt lived LIVEDFOREVER kk
real	0m2.217s
user	0m2.170s
sys	0m0.045s
*/
#define CHUNKSIZE 100
int strrepl(char *InputString,char *StringToFind,char *StringToReplace,char *output)
{
	char *offset = NULL, *CurrentPointer,**TableOfHits=NULL,*last;
	size_t insertlen,capacity,tableIdx;
	size_t findlen = Strlen(StringToFind);
	size_t result,delta,total_length;
	int FirstChar,i;

	if (findlen == 0) return 0;
	if (StringToReplace)
		insertlen = Strlen(StringToReplace);
	else
		insertlen = 0;
	if (output) {
		result = 0;
		if (output != InputString) *output = 0;
	}
	else {
		result = Strlen(InputString)+1;
		if (insertlen == findlen) return result;
	}
	FirstChar = *StringToFind;
	if (findlen != insertlen && output != NULL) {
		tableIdx = 0;
		TableOfHits = malloc(CHUNKSIZE*sizeof(char *));
		capacity = CHUNKSIZE;
	}
	else if (output != NULL && findlen == insertlen && output != InputString)
		strcpy(output,InputString);
	for (offset = InputString; *offset; offset++) {
		if (FirstChar == *offset && !strncmp(offset,StringToFind,findlen)) {
			if (output == NULL) {
				result += insertlen - findlen;
				offset += findlen;
				continue;
			}
			if (insertlen == findlen) {
				memcpy(output+(offset-InputString),StringToReplace,insertlen);
				offset += insertlen;
				continue;
			}
			if (tableIdx >= capacity-1) {
				char **tmp;
				tmp = realloc(TableOfHits,sizeof(char *)*(capacity+CHUNKSIZE));
				if (tmp == NULL) {
					free(TableOfHits);
					return -1;
				}
				TableOfHits = tmp;
				capacity += CHUNKSIZE;
			}
			TableOfHits[tableIdx++]=offset;
			offset += findlen;
		}
	}
	if (output == NULL) return result;
	if (tableIdx == 0) {
		free(TableOfHits);
		return 0;
	}
	TableOfHits[tableIdx] = offset;
	total_length = (offset - InputString) - tableIdx * (findlen-insertlen);
	if (findlen < insertlen) {
		CurrentPointer = output;
		last = InputString;
		for (i=0; i<tableIdx;i++) {
			offset = TableOfHits[i];
			memmove(CurrentPointer,last,offset-last);
			CurrentPointer += (offset-last);
			if (insertlen) {
				memcpy(CurrentPointer,StringToReplace,insertlen);
				CurrentPointer += insertlen;
			}
			last = offset+findlen;
			result++;
		}
		memcpy(CurrentPointer,last,TableOfHits[i]-last);
	}
	else  {
		CurrentPointer = output + total_length;
		for (i=tableIdx-1; i >=0; i--) {
			offset = TableOfHits[i];
			delta = (TableOfHits[i+1]-offset)-findlen;
			CurrentPointer -= delta;
			memcpy(CurrentPointer,offset+findlen,delta);
			if (insertlen) {
				CurrentPointer -= insertlen;
				memcpy(CurrentPointer,StringToReplace,insertlen);
			}
			result++;
		}
		memcpy(output,InputString,TableOfHits[0]-InputString);
	}
	free(TableOfHits);
	output[total_length]=0;
	return result;
}
#elif 0
/* 
This version was contributed by Ben Bacarisse. It merges the two passes of the above version into one. In this machine
it suffers from the bad performance of strstr.
jacob@rock64:~/strrepl$ time ./strrepl-c bible.txt lived LIVEDFOREVER kk
real	0m3.283s
user	0m3.236s
sys	0m0.045s
*/
int strrepl(const char *in, const char *pat, const char *rep, char *out)
{
     int patlen = Strlen(pat), replen = Strlen(rep), len = 0;
     const char *src = in, *next;
     while (next = strstr(src, pat)) {
          if (out) memcpy(out + len, src, next - src);
          len += next - src;
          if (out) memcpy(out + len, rep, replen);
          len += replen;
          src = next + patlen;
     }
     int rest = Strlen(src);
     if (out) memcpy(out + len, src, rest + 1);
     return len + rest;
} 
#elif 1
/* This version is the version of Mr Bacarisse modified to be more performant in a linux
ARM 64 system. It replaces strstr with strncmp and makes some changes to improve
performance in this system. This version is the fastest in this system.
jacob@rock64:~/strrepl$ time ./strrepl-c bible.txt lived LIVEDFOREVER kk
real	0m1.778s
user	0m1.731s
sys	0m0.047s
*/
int strrepl(const char *in, const char *pat, const char *rep, char *out)
{
     int patlen = Strlen(pat), replen = Strlen(rep), len = 0;
	int FirstChar = *pat;
     const char *src = in, *next;

	if (patlen == 0) return 0;
	if (out) {
	     for (next = src; *next;next++) {
			if (FirstChar == *next && strncmp(next,pat,patlen) == 0) {
	          memcpy(out + len, src, next - src);
	          len += next - src;
	          memcpy(out + len, rep, replen);
	          len += replen;
	          src = next = next + patlen;
			}
	     }
	}
    else {
	     for (next = src; *next;next++) {
				if (FirstChar == *next && strncmp(next,pat,patlen) == 0) {
	          	len++;
	          	next += patlen;
			}
	     }
		 return (next - src) + (replen-patlen)*len;
	}
     int rest = next-src;
     memcpy(out + len, src, rest + 1);
     return len + rest;
}
#endif
/*
This version was contributed by "jak" It is slower than the others and modifies the interface,
so I gave it another name
*/
int strrepl_jak(const char *in, size_t inSize, const char *pat, const char *rep, char *out)
{
    char *inEnd = (char *)in + inSize,
         *inWalk, *p,
         *patEnd = (char *)pat + (strlen(pat) - 1),
         *repEnd = (char *)rep + (strlen(rep)),
         *patWalk;
    int  patCnt = 0;

    patWalk = (char *)pat;
    for(inWalk = (char *)in; inWalk < inEnd;)
    {
        if(*inWalk == *patWalk)
        {
            if(patWalk++ == patEnd)
            {
                patCnt++;
                for(p = (char *)rep; out && p < repEnd; p++)
                    *out++ = *p;
                patWalk = (char *)pat;
            }
            inWalk++;
        }
        else
        {
            if(patWalk != pat)
            {
                for(p = (char *)pat; out && p < patWalk; p++)
                    *out++ = *p;
                patWalk = (char *)pat;
            }
            else
            {
                if(out)
                    *out++ = *inWalk;
                inWalk++;
            }
        }
    }
    return inSize + ((strlen(rep) - strlen(pat)) * patCnt);
}

/*
Driver for the tests.
*/

int main(int argc,char *argv[])
{
	char *buffer,*tmp;
	size_t siz,siz1;
	FILE *f;

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
	buffer = malloc(siz+1);
	if (buffer == NULL) {printf("No more memory\n"); exit(6);}
	fseek(f,0,SEEK_SET);
	if (siz != fread(buffer,1,siz,f)) { printf("ReadError\n"); exit(5);}
	fclose(f);
	buffer[siz]=0;
	siz = strrepl(buffer,argv[2],argv[3],NULL);
	tmp = malloc(siz+100);
	for (int i=0; i<100;i++) 
		siz1 = strrepl(buffer,argv[2],argv[3],tmp);
	f = fopen(argv[4],"wb");
	fwrite(tmp,1,siz-1,f);
	fclose(f);
	return 0;
}
