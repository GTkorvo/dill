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

static char *DILL_version = "DILL Version 1.0.103 rev. 8756  -- 2010-09-02 20:10:46 -0400 (Thu, 02 Sep 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

