#include <linux/kprobes.h>
#include <linux/module.h>

enum syscall {
  UNKNOWN,
  KILL
};


struct unknown_metadata {
};

union metadata {
  struct unknown_metadata unknown_metadata;
};




struct syscall_metadata {
  enum syscall syscall;
  union metadata metadata; 
};

void free_syscall_metadata(struct syscall_metadata syscall_metadata);

void free_syscall_metadata(struct syscall_metadata syscall_metadata) {
  switch (syscall_metadata.syscall) {
  case KILL:
  case UNKNOWN:
    break;
  }
} 
