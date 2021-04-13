#include <stdio.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <string.h>

#include "unzip.h"
#include "psp.h"



#define	PATH_MAX	256
static unzFile unzfile=NULL;
static char basedir[PATH_MAX];
char* basedirend;

int zip_open(const char *name)
{
	char path[PATH_MAX];
	unzfile = unzOpen(convert_path(name));
	if (unzfile) return (int)unzfile;

	strcpy(basedir,convert_path(name));
			
	basedirend = basedir + strlen(basedir);
	*basedirend++='/';
	
	return -1;
}

int zip_close(void)
{
	if (unzfile) {
		unzClose(unzfile);
		unzfile = NULL;
	}
}

int zopen(const char *filename)
{
	if (unzfile==NULL) {
		strcpy(basedirend,filename);						
		FILE *fd=fopen(basedir,"rb");		
		return (fd?(int)fd:-1);
	}

	int ret = unzLocateFile(unzfile,filename,0);
	if (ret!=UNZ_OK) return -1;
	ret = unzOpenCurrentFile(unzfile);
	if (ret!=UNZ_OK) return -1;
	return unzfile;
}

int zread(int fd,void * buf,unsigned size)
{
	if (unzfile == NULL)
		return fread(buf,1,size,(FILE*)fd);
	return unzReadCurrentFile(unzfile,buf,size);
}

int zclose(int fd)
{
	if (unzfile == NULL) {
		if (fd!=-1) fclose((FILE*)fd);
		return 0;
	}
	return unzCloseCurrentFile(unzfile);
}

char* zgets(int fd,char*buf,unsigned buflen)
{
	int i;
	char ch;
	for(i=0;i<buflen-1;) {
		int r = zread(fd,&ch,1);
		if (r<=0) break;
		buf[i++] = ch;
		if (ch=='\n') break;
	}
	buf[i] = 0;
	return buf;
}
