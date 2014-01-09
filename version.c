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

static char *DILL_version = "DILL Version 2.1.3 rev. 16664  -- 2014-01-08 11:51:52 -0500 (Wed, 08 Jan 2014)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

