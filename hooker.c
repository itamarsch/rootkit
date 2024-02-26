
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

struct callback {
  void *data_before_callback;
  size_t size_of_data;
  void *address_of_callback;
};

void *get_page_start_address(void *ptr, size_t pagesize) {
  return (void *)((uintptr_t)ptr & ~(pagesize - 1));
}

void page_read_write_exec(void *ptr) {

  const size_t pagesize = sysconf(_SC_PAGESIZE);
  void *page_start = get_page_start_address(ptr, pagesize);

  int mprotect_err =
      mprotect(page_start, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);

  if (mprotect_err == -1) {
    printf("Failed mprotect EXEC");
    exit(EXIT_FAILURE);
  }
}

void page_read_exec(void *ptr) {

  size_t pagesize = sysconf(_SC_PAGESIZE);
  void *page_start = get_page_start_address(ptr, pagesize);

  int mprotect_err = mprotect(page_start, pagesize, PROT_READ | PROT_EXEC);

  if (mprotect_err == -1) {
    printf("Failed mprotect EXEC");
    exit(EXIT_FAILURE);
  }
}

struct callback register_callback(void *callback, void *targeted,
                                  size_t targeted_size,
                                  size_t noop_padding_size) {

  page_read_write_exec(targeted);

  struct callback target_callback = {0};

  size_t offset = 5 + noop_padding_size;

  target_callback.size_of_data = targeted_size + offset;
  target_callback.address_of_callback = targeted;

  target_callback.data_before_callback = malloc(target_callback.size_of_data);

  memcpy(target_callback.data_before_callback, targeted,
         target_callback.size_of_data);

  memmove(targeted + offset, targeted, targeted_size);

  unsigned char *targeted_bytes_arr = (unsigned char *)targeted;

  int inserted = 0;

  const unsigned char call = 0xE8;
  targeted_bytes_arr[0] = call;
  inserted++;

  *(size_t *)(targeted + inserted) =
      callback - (targeted + inserted + 4); // Add 4 for address
  inserted += 4;

  const unsigned char noop = 0x90;
  for (int i = inserted; i < offset; i++) {
    targeted_bytes_arr[i] = noop;
    inserted += 1;
  }

  page_read_exec(targeted);

  return target_callback;
}

void unregister(struct callback clbck) {
  page_read_write_exec(clbck.address_of_callback);

  memcpy(clbck.address_of_callback, clbck.data_before_callback,
         clbck.size_of_data);

  page_read_exec(clbck.address_of_callback);

  free(clbck.data_before_callback);
}

void modified(void) { printf("world\n"); }

void foo() { printf("foo\n"); }

void hello(void) { printf("Hello\n"); }

int main(void) {
  struct callback hello_callback = register_callback(
      &hello, &modified, 0x19, 1 /* Selected randomly until it worked */);

  modified();

  unregister(hello_callback);

  printf("-------------Unregister-------------\n");

  modified();

  foo();
}
