// This is for inline assembler subs 

#ifndef INCLUDED_MACROS_H
#define INCLUDED_MACROS_H

static int __inline__ scancmp(const char* str1, const char* str2, int len)
{
  int rc;
__asm__( " cld " \
	 " repne cmpsb " \
	 : "=c" (rc) \
	 : "S" (str1), "D" (str2), "c" (len) \
	 : "%cc" );
  return rc;
}

#endif
