/* Tests the fibonacci system call. */

#include <stdio.h>
#include <syscall.h>

int main (void) 
{
  printf("%d\n",fibonacci(10));
  exit (57);
}
