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

static char *DILL_version = "DILL Version 1.0.91 rev. 7452  -- 2009-06-26 12:29:08 -0400 (Fri, 26 Jun 2009)))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

