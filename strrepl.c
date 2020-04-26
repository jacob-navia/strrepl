#include <string.h>
#include <stdlib.h>
#define CHUNKSIZE 100
int strrepl(char *InputString,char *StringToFind,char *StringToReplace,char *output)
{
	char *offset = NULL, *CurrentPointer,**TableOfHits=NULL,*last;
	size_t insertlen,capacity,tableIdx;
	size_t findlen = strlen(StringToFind);
	size_t result,delta,total_length;
	int FirstChar,i;

	if (findlen == 0) return 0;
	if (StringToReplace)
		insertlen = strlen(StringToReplace);
	else
		insertlen = 0;
	if (output) {
		result = 0;
		if (output != InputString) *output = 0;
	}
	else {
		result = strlen(InputString)+1;
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
				result += insertlen;result -= findlen;
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

#include <stdio.h>
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
