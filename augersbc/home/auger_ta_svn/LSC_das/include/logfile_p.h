#if !defined(_logfilelib_P_H_) 
#define _logfilelib_P_H_  
/*****************************************
     Function prototypes of file(s) logfilelib.c
     generated  Thu Mar 30 16:55:39 ART 2006
     by auger
*****************************************/
/* logfilelib.c */
void LogSetProgName( const char *name);
void LogInitTime(unsigned int *temps);
void LogSetFileName( const char *fname);
void LogSetNewFile( const char *fname);
void LogSetNoclose() ;
int LogPrt( const char *str, int we, const char * fun );
FILE *LogOpen(void);
void LogClose(void);
int LogDebug( const char * fun, int we, const char * fromat, ... ) ;
int LogPrint(int we, const char *format, ...);
int LogPrintTimed(int we, const char *format, ...);
int LogPrintDate(int we, const char *format, ...);
int LogPrintSysDate(int we, const char *format, ...);

#define LOG_DEBUG(...) LogDebug(__func__, __VA_ARGS__)

#endif
