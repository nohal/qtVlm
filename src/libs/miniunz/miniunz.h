#ifndef MINIUNZ_H
#define MINIUNZ_H


#define UZ_EXTRACT_WOPATH 1
#define UZ_OVERWRITE 2

#ifdef  __cplusplus
extern "C" {
#endif

int miniunzip(int mode,const char * zipfilename, const char * dirname,
              const char * filename_to_extract, const char * password);

#ifdef  __cplusplus
}
#endif
#endif // MINIUNZ_H
