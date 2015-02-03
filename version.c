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

static char *DILL_version = "DILL Version 2.1.11 rev. 20267  -- 2015-02-02 22:45:01 -0500 (Mon, 02 Feb 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

