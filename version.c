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

static char *DILL_version = "DILL Version 2.1.14 rev. 20523  -- 2015-03-03 11:00:20 -0500 (Tue, 03 Mar 2015)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

