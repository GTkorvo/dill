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

static char *DILL_version = "DILL Version 1.0.97 rev. 7841  -- 2010-01-04 15:06:50 -0500 (Mon, 04 Jan 2010)))))))))))\n";

void DILLprint_version(){
    printf("%s",DILL_version);
}

