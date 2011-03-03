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

static char *DILL_version = "DILL Version 1.0.109 rev. 9729  -- 2011-03-02 14:52:33 -0500 (Wed, 02 Mar 2011)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

