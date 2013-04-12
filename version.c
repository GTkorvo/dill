#if defined(FUNCPROTO) || defined(__STDC__) || defined(__cplusplus) || defined(c_plusplus)
#ifndef ARGS
#define ARGS(args) args
#endif
#else
#ifndef ARGS
#define ARGS(args) (/*args*/)
#endif
#endif

#include <stdio.h>
#include "config.h"

static char *DILL_version = "DILL Version 1.0.143 rev. 14178  -- 2013-04-11 13:59:54 -0400 (Thu, 11 Apr 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

