#ifndef _MALLINFO_H_
#define _MALLINFO_H_
struct mallinfo
{
  int arena;    /* non-mmapped space allocated from system */
  int ordblks;  /* number of free chunks */
  int smblks;   /* number of fastbin blocks */
  int hblks;    /* number of mmapped regions */
  int hblkhd;   /* space in mmapped regions */
  int usmblks;  /* maximum total allocated space */
  int fsmblks;  /* space available in freed fastbin blocks */
  int uordblks; /* total allocated space */
  int fordblks; /* total free space */
  int keepcost; /* top-most, releasable (via malloc_trim) space */
};

struct myinfo
{
    int arena;
    int freemem;
    int usdmem;
    int freeblks;
    int usdblks;
    int maxfreeblk;
};
#endif
