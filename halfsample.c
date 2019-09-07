#include <stdint.h>

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
void half_sample_plain(const uint8_t* in, int in_w, int in_h, uint8_t* out)
{
	const uint8_t* top_ptr = in;
	const uint8_t* bottom_ptr = in + in_w;
	uint8_t* out_ptr = out;

	int out_w = in_w / 2;
	int out_h = in_h / 2;
	for(int y = 0; y < out_h; ++y)
	{
		for(int x = 0; x < out_w; ++x)
		{
			*out_ptr = (top_ptr[0] + top_ptr[1] + bottom_ptr[0] + bottom_ptr[1] + 2) >> 2;
			top_ptr += 2;
			bottom_ptr += 2;
			out_ptr++;
		}
		top_ptr += in_w;
		bottom_ptr += in_w;
	}
}

// Assumptions: in image tightly packed (stride == in_w)
// out image already allocated (in_w / 2) * (in_h / 2) bytes
// in and out 8-byte aligned, in_w % 8 == 0
void half_sample_uint64_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out)
{
	const uint64_t* top_ptr = (uint64_t*)in;
	const uint64_t* bottom_ptr = (uint64_t*)(in + in_w);
	uint32_t* out_ptr = (uint32_t*)(out);

	const uint64_t mask_00ff = 0x00FF00FF00FF00FF;
	const uint64_t twos = 0x0002000200020002;

	int x_blocks = in_w / 8;
	int out_h = in_h / 2;
	for(int y = 0; y < out_h; ++y)
	{
		for(int x = 0; x < x_blocks; ++x)
		{
			uint64_t top_vals = *top_ptr;
			uint64_t bottom_vals = *bottom_ptr;
			top_ptr++;
			bottom_ptr++;

			// Split into separate vectors for odd and even byte indices
			// to allow the pairwise additions (and to give 16-bits of
			// room to accumulate the totals before dividing by 4)
			uint64_t top_vals_even = top_vals & mask_00ff;
			uint64_t top_vals_odd = (top_vals >> 8) & mask_00ff;
			uint64_t bottom_vals_even = bottom_vals & mask_00ff;
			uint64_t bottom_vals_odd = (bottom_vals >> 8) & mask_00ff;

			// Add them all up, +2 for correct rounding
			uint64_t totals = top_vals_even + top_vals_odd + bottom_vals_even + bottom_vals_odd + twos;

			// Divide by 4 and re-assert the mask for the different elements
			uint64_t average = (totals >> 2) & mask_00ff;

			// Now a couple of shifts, ORs, and masks to get the 4 results we want
			// tightly packed into the lower 32 bits
			uint64_t shift_1 = (average >> 8) | average;
			uint64_t shift_1_masked = shift_1 & 0x0000ffff0000ffff;

			uint32_t out4 = (uint32_t)shift_1_masked;
			out4 |= (uint32_t)(shift_1_masked >> 16);

			*out_ptr = out4;
			out_ptr++;
		}

		top_ptr += x_blocks;
		bottom_ptr += x_blocks;
	}
}
