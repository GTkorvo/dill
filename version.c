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

static char *DILL_version = "DILL Version 1.0.96 rev. 7801  -- 2009-12-08 09:09:16 -0500 (Tue, 08 Dec 2009))))))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

