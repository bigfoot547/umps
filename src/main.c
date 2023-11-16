#include <locale.h>
#include <stdio.h>
#include <unistd.h>

#include "ui.h"

int main(void)
{
  setlocale(LC_ALL, "");

  printf("%d\n", getpid());
  #if 0
  volatile int dbg = 1;
  while (dbg);
  #endif

  ui_init();
  ui_handle();
}
