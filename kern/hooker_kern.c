
#include "linux/printk.h"
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/module.h>

struct callback {
  void *data_before_callback;
  size_t size_of_data;
  void *address_of_callback;
};

void remove_write_protection(void) {
  unsigned int cr0_register;

  asm volatile("mov %%cr0, %0" : "=r"(cr0_register));

  cr0_register &= ~(1 << 16);

  asm volatile("mov %0, %%cr0" : : "r"(cr0_register));
}

struct callback register_callback(void *callback, void *targeted,
                                  size_t targeted_size,
                                  size_t noop_padding_size) {

  struct callback target_callback = {0};

  size_t offset = 5 + noop_padding_size;

  target_callback.size_of_data = targeted_size + offset;
  target_callback.address_of_callback = targeted;

  target_callback.data_before_callback =
      kmalloc(target_callback.size_of_data, GFP_KERNEL);

  memcpy(target_callback.data_before_callback, targeted,
         target_callback.size_of_data);

  remove_write_protection();
  memmove(targeted + offset, targeted, targeted_size);

  unsigned char *targeted_bytes_arr = (unsigned char *)targeted;

  const unsigned char call = 0xE8;
  remove_write_protection();

  size_t rel_address = (size_t)(callback - (targeted + 5));

  printk("Rel address: %zu\n", rel_address);

  remove_write_protection();
  targeted_bytes_arr[0] = call;
  *(size_t *)(targeted_bytes_arr + 1) = rel_address;

  unsigned long inserted = 5;

  const unsigned char noop = 0x90;
  for (int i = inserted; i < offset; i++) {
    remove_write_protection();
    targeted_bytes_arr[i] = noop;
    inserted += 1;
  }

  return target_callback;
}
void unregister(struct callback clbck) {

  remove_write_protection();
  memcpy(clbck.address_of_callback, clbck.data_before_callback,
         clbck.size_of_data);

  kfree(clbck.data_before_callback);
}
void modified(void) { printk(KERN_INFO "world\n"); }

void foo(void) { printk(KERN_INFO "foo\n"); }

void hello(void) {
  printk(KERN_INFO "Hello\n");
  printk(KERN_INFO "Hello\n");
  printk(KERN_INFO "Hello\n");
  printk(KERN_INFO "Hello\n");
  return;
}
static int __init hooker_init(void) {

  const size_t modified_size = 0x19;
  const size_t noop_slot_size = 3;

  for (int i = 0; i < modified_size; i++) {

    printk("%x\n", *(char *)(&modified + i));
  }

  printk(KERN_INFO "-------------Register-------------\n");

  printk("Hello: %p\n", &hello);
  printk("Modified: %p\n", &modified);

  struct callback hello_callback =
      register_callback(&hello, &modified, modified_size,
                        noop_slot_size /* Selected randomly until it worked */);

  for (int i = 0; i < modified_size + noop_slot_size + 5; i++) {
    printk("%x \n", *(char *)(modified + i));
  }

  modified();
  unregister(hello_callback);

  printk(KERN_INFO "-------------Unregister-------------\n");

  modified();

  foo();

  return 0;
}

static void __exit hooker_exit(void) { return; }

module_init(hooker_init);
module_exit(hooker_exit);

MODULE_LICENSE("GPL");
