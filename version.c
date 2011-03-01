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

static char *DILL_version = "DILL Version 1.0.108 rev. 9725  -- 2011-02-28 16:53:34 -0500 (Mon, 28 Feb 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

