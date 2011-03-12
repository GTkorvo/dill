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

static char *DILL_version = "DILL Version 1.0.111 rev. 9763  -- 2011-03-11 13:38:35 -0500 (Fri, 11 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

