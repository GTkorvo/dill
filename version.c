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

static char *DILL_version = "DILL Version 1.0.145 rev. 14768  -- 2013-06-02 08:53:29 -0400 (Sun, 02 Jun 2013)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

