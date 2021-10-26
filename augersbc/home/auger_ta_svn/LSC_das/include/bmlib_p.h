#if !defined(_bmlib_P_H_) 
#define _bmlib_P_H_  
/*****************************************
     Function prototypes of file(s) bmlib.c
     generated  jeu mar 23 22:18:50 CET 2006
     by desplanc
*****************************************/
/* bmlib.c */
char *BMVersion(void);
error_code BMDumpHeader(BMIdent bmid, FILE *fout);
error_code BMCreate(BMIdent *bmid, char *name, int element_size, int nb_elements);
error_code BMLink(BMIdent *bmid, char *name, int element_size, int nb_elements);
error_code BMUnlink(BMIdent bmid);
error_code BMReset(BMIdent bmid);
error_code BMGetStatus(BMIdent bmid, BM_STATUS *pstat);
error_code BMAdd(BMIdent bmid, void **data, unsigned char *tag);
error_code BMFindNext(BMIdent bmid, void *data, int (*compar)(const void *), void **next);
error_code BMFind(BMIdent bmid, void *data, int (*compar)(const void *), void **next);
error_code BMFindTag(BMIdent bmid, void *data, int (*compar)(const void *), int tagidx, u_char oldtag, u_char newtag);
error_code BMFindOldTag(BMIdent bmid, void *data, int tagidx, u_char oldtag, u_char newtag);
error_code BMFindNewTag(BMIdent bmid, void *data, int tagidx, u_char oldtag, u_char newtag);
error_code BMFindOld(BMIdent bmid, void *data);
error_code BMFindNew(BMIdent bmid, void *data);
error_code BMGetOldList(BMIdent bmid, int (*handle)(const void *));
error_code BMGetNewList(BMIdent bmid, int (*handle)(const void *));
#endif
