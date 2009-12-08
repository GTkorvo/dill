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

static char *DILL_version = "DILL Version 1.0.95 rev. 7799  -- 2009-12-07 19:55:47 -0500 (Mon, 07 Dec 2009)))))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

