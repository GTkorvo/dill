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

static char *DILL_version = "DILL Version 1.0.100 rev. 8722  -- 2010-08-23 13:38:03 -0400 (Mon, 23 Aug 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

