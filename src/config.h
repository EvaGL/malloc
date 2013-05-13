#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stddef.h>

#define FIRST_FIT

//#define MAX_HEAP_SIZE 4096
#define FIT_DELETE_USED
#define ROUND_ROBIN
#define ALLOW_MERGE

#define ALIGN_LOG 2

#define MALLOC_DEBUG 1
#include "default.h"
#endif
