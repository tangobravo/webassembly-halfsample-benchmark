#include <stdint.h>

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
void half_sample_plain(const uint8_t* in, int in_w, int in_h, uint8_t* out);

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
// in and out 8-byte aligned, in_w % 8 == 0
void half_sample_uint64_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out);

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
// in and out 4-byte aligned, in_w % 4 == 0
void half_sample_uint32_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out);

// As above but in_w % 8 == 0 (inner loop computes 2 uint32_t blocks per iteration)
void half_sample_uint32x2_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out);

#ifdef USE_WASM_SIMD

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
// in and out 16-byte aligned, in_w % 32 == 0
void half_sample_wasm_simd(const uint8_t* in, int in_w, int in_h, uint8_t* out);

#endif
