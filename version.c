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

static char *DILL_version = "DILL Version 2.1.9 rev. 18762  -- 2014-10-09 10:30:29 -0400 (Thu, 09 Oct 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

