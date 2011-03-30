/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: mainLogMsg.h,v 1.1 1998/07/16 14:14:03 decoster Exp $
 */

#ifndef _MAINLOGMSG_H_
#define _MAINLOGMSG_H_

#ifdef LOG_MESSAGES

#include "sys/times.h"
#include <stdio.h>

FILE  *logFileId = stdout;
int   logTimeBegin = 0;
struct tms TheLogTime;
char* logFileName="log";

#define SetLogFileName(name) logFileName=(name)
#define LogMessage(msg) logFileId=fopen(logFileName,"a");fprintf(logFileId,msg);fclose(logFileId)
#define LogMessage2(fmt,var) logFileId=fopen(logFileName,"a");fprintf(logFileId,fmt,var);fclose(logFileId)

#define SetLogTimeBegin() times(&TheLogTime);logTimeBegin=TheLogTime.tms_utime
#define LogTime() times(&TheLogTime);logFileId=fopen(logFileName,"a");fprintf(logFileId,"%d ",(int)(TheLogTime.tms_utime-logTimeBegin));fclose(logFileId)
/*#define InitLogFileId(name) (logFileId=fopen(name,"w"))
#define LogMessage(msg) fprintf(logFileId,msg)
#define LogMessage2(fmt,var) fprintf(logFileId,fmt,var)
#define FcloseLogFile() fclose(logFileId)

#define SetLogTimeBegin() times(&TheLogTime);logTimeBegin=TheLogTime.tms_utime
#define LogTime() times(&TheLogTime);fprintf(logFileId," %d ",(int)(TheLogTime.tms_utime-logTimeBegin))
*/
#else /* ifndef LOG_MESSAGES */

#define SetLogFileName(name)
#define LogMessage(msg)
#define LogMessage2(fmt,var)

#define SetLogTimeBegin()
#define LogTime()

#endif /* LOG_MESSAGES */

#endif /* _MAINLOGMSG_H_ */
