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

static char *DILL_version = "DILL Version 1.0.129 rev. 11101  -- 2012-05-02 21:26:42 -0400 (Wed, 02 May 2012)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

