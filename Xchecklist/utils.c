#include "XPLMUtilities.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

static char *msg = NULL;
static size_t msgSize = 0;

/*NOT THREAD SAFE!!!*/

static void xcDebugInt(const char *format, va_list va)
{
  va_list vc;
  int res;
  if(msg == NULL){
    msgSize = 2;
    msg = (char *)malloc(msgSize);
  }
  if(msg == NULL){
    XPLMDebugString("Xchecklist: Couldn't allocate buffer for messages!\n");
    return;
  }
  while(1){ /*looping once, in case of string too big*/
    /*copy, in case we need another go*/
#if IBM
    vc = va; /*no va_copy on VC*/
#else
    va_copy(vc, va);
#endif
    res = vsnprintf(msg, msgSize, format, vc);
    va_end(vc);
    
    if((res > -1) && ((size_t)res < msgSize)){
      XPLMDebugString(msg);
      return;
    }else{
      void *tmp;
      msgSize *= 2;
      if((tmp = realloc(msg, msgSize)) == NULL){
        break;
      }
      msg = tmp;
    }
  }
  XPLMDebugString("Xchecklist: Problem with debug message formatting!\n");
  msg = NULL;
  msgSize = 0;
  return;
}

void xcDebug(const char *format, ...)
{
  va_list ap;
  va_start(ap,format);
  xcDebugInt(format, ap);
  va_end(ap);
}

void xcClose()
{
  free(msg);
  msg = NULL;
  msgSize = 0;
}
