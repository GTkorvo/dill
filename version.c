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

static char *DILL_version = "DILL Version 1.0.90 rev. 7446  -- 2009-06-24 13:39:50 -0400 (Wed, 24 Jun 2009))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

