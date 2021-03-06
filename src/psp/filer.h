#ifndef FILER_H
#define FILER_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern char LastPath[], FilerMsg[];

int getExtId(const char *szFilePath);

int searchFile(const char *path, const char *name);
int getFilePath(char *out);
int InitFiler(char*msg,char*path);

// 有効な拡張子
enum {    
  EXT_ZIP,
  EXT_UNKNOWN  
};


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
