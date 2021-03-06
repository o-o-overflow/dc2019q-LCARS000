const unsigned char untrusted_bpf[] = {
  0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x08,
  0x3e, 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x35, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x15, 0x00, 0x00, 0x05,
  0xff, 0xff, 0xff, 0xff, 0x15, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x01, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0x3c, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x06, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};
const unsigned int untrusted_bpf_len = 88;
const unsigned char platform_bpf[] = {
  0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x0a,
  0x3e, 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x35, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x15, 0x00, 0x00, 0x07,
  0xff, 0xff, 0xff, 0xff, 0x15, 0x00, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x03, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x15, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x01, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01,
  0x3c, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned int platform_bpf_len = 104;
const unsigned char system_bpf[] = {
  0x20, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x0e,
  0x3e, 0x00, 0x00, 0xc0, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x35, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x40, 0x15, 0x00, 0x00, 0x0b,
  0xff, 0xff, 0xff, 0xff, 0x15, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x08, 0x00, 0x01, 0x00, 0x00, 0x00, 0x15, 0x00, 0x07, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x15, 0x00, 0x06, 0x00, 0x09, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x05, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x15, 0x00, 0x04, 0x00,
  0x0b, 0x00, 0x00, 0x00, 0x15, 0x00, 0x03, 0x00, 0x14, 0x00, 0x00, 0x00,
  0x15, 0x00, 0x02, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x15, 0x00, 0x01, 0x00,
  0x3c, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0x9d, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x7f, 0x06, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};
const unsigned int system_bpf_len = 136;
