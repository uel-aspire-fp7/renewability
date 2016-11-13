#include <stdio.h>

unsigned long compute_value(unsigned long value) {
  return value / 3;
}

int fn1(void)
{
  _Pragma("ASPIRE begin protection(code_mobility,status(mobile))")

  // do some computation to increase block size

  int x = 1, fn = x;
  unsigned long myvalue = (unsigned long)time(NULL);

  myvalue *= 2;
  myvalue = compute_value(myvalue);

  for (; x < 20; x++) {
    myvalue = myvalue - x;
  }

  printf ("Ground Control to Major Tom (fn%d, v: %d)\n", fn, myvalue);

  return x;  
  _Pragma("ASPIRE end");
}

int fn2(void)
{
  _Pragma("ASPIRE begin protection(code_mobility,status(mobile))")
  
  int x = 2;

  printf ("Major Tom to Ground Control (fn%d)\n", x);

  return x;

  _Pragma("ASPIRE end");
}