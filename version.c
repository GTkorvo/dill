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

static char *DILL_version = "DILL Version 1.0.141 rev. 14115  -- 2013-04-08 09:23:18 -0400 (Mon, 08 Apr 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

