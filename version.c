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

static char *DILL_version = "DILL Version 1.0.94 rev. 7498  -- 2009-08-18 19:19:27 -0400 (Tue, 18 Aug 2009))))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

