// Common implementation of a progress bar - CP

#include "stdio.h"
#include "progbar.h"

const char* pbar="*";

void ProgBar(int len)
{
  int i;
  printf("[");
  for (i=0; i<len; i++) printf("-");
  printf("]");
  for (i=0; i<len+1; i++) printf("\x08");
}

