#include <linux/kprobes.h>
#include <linux/module.h>

#pragma once

enum syscall { UNKNOWN, KILL, OPEN };

struct unknown_metadata {};

struct open_metadata {
  bool should_fail;
};

union metadata {
  struct unknown_metadata unknown_metadata;
  struct open_metadata open_metadata;
};

struct syscall_metadata {
  enum syscall syscall;
  union metadata metadata;
};

void free_syscall_metadata(struct syscall_metadata syscall_metadata);

void free_syscall_metadata(struct syscall_metadata syscall_metadata) {
  switch (syscall_metadata.syscall) {
  case OPEN:
  case KILL:
  case UNKNOWN:
    break;
  }
}
