/*
 *   Copyright 1997 Centre de Recherche Paul Pascal, Bordeaux, France
 *  Written by Nicolas Decoster.
 *
 *  The author may be reached (Email) at the address
 *      decoster@crpp.u-bordeaux.fr
 *
 *  $Id: logMsg.h,v 1.1 1998/07/16 14:13:47 decoster Exp $
 */

#ifndef _LOGMSG_H_
#define _LOGMSG_H_

#ifdef LOG_MESSAGES

#include "sys/times.h"
#include <stdio.h>

extern FILE  *logFileId;
extern int   logTimeBegin;
extern struct tms TheLogTime;
extern char* logFileName;

#define SetLogFileName(name) logFileName=(name)
#define LogMessage(msg) logFileId=fopen(logFileName,"a");fprintf(logFileId,msg);fclose(logFileId)
#define LogMessage2(fmt,var) logFileId=fopen(logFileName,"a");fprintf(logFileId,fmt,var);fclose(logFileId)

#define SetLogTimeBegin() times(&TheLogTime);logTimeBegin=TheLogTime.tms_utime
#define LogTime() times(&TheLogTime);logFileId=fopen(logFileName,"a");fprintf(logFileId,"%d ",(int)(TheLogTime.tms_utime-logTimeBegin));fclose(logFileId)

#else /* ifndef LOG_MESSAGES */

#define SetLogFileName(name)
#define LogMessage(msg)
#define LogMessage2(fmt,var)

#define SetLogTimeBegin()
#define LogTime()

#endif /* LOG_MESSAGES */

#endif /* _LOGMSG_H_ */
