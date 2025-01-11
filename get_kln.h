#include <linux/kprobes.h>
#include <linux/module.h>

#define KPROBE_PRE_HANDLER(fname)                                              \
  static int __kprobes fname(struct kprobe *p, struct pt_regs *regs)

long unsigned int kln_addr = 0;

typedef unsigned long (*kln_p)(const char *);
unsigned long (*kln_pointer)(const char *name) = NULL;

static struct kprobe kp0, kp1;

static int __attribute__((__section__(".kprobes.text")))
handler_pre0(struct kprobe *p, struct pt_regs *regs) {
  kln_addr = (--regs->ip);

  return 0;
}

KPROBE_PRE_HANDLER(handler_pre1) { return 0; }

static int do_register_kprobe(struct kprobe *kp, char *symbol_name,
                              void *handler) {
  int ret;

  kp->symbol_name = symbol_name;
  kp->pre_handler = handler;

  ret = register_kprobe(kp);
  if (ret < 0) {
    pr_err(
        "do_register_kprobe: failed to register for symbol %s, returning %d\n",
        symbol_name, ret);
    return ret;
  }

  pr_info("Planted krpobe for symbol %s at %p\n", symbol_name, kp->addr);

  return ret;
}

// this is the function that I have modified, as the name suggests it returns a
// pointer to the extracted kallsyms_lookup_name function
kln_p get_kln_p(void);
kln_p get_kln_p(void) {
  int status;

  status = do_register_kprobe(&kp0, "kallsyms_lookup_name", handler_pre0);

  if (status < 0)
    return NULL;

  status = do_register_kprobe(&kp1, "kallsyms_lookup_name", handler_pre1);

  if (status < 0) {
    // cleaning initial krpobe
    unregister_kprobe(&kp0);
    return NULL;
  }

  unregister_kprobe(&kp0);
  unregister_kprobe(&kp1);

  printk(KERN_INFO "kallsyms_lookup_name address = 0x%lx\n", kln_addr);

  kln_pointer = (unsigned long (*)(const char *name))kln_addr;

  return kln_pointer;
}
