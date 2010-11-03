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

static char *DILL_version = "DILL Version 1.0.107 rev. 9040  -- 2010-11-02 18:11:01 -0400 (Tue, 02 Nov 2010)\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

