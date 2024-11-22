/* Tests the fibonacci system call. */

#include <stdio.h>
#include <syscall.h>

int main(void) {
  printf("%d\n", max_of_four_int(1, 2, 3, 4));
  exit(57);
}
