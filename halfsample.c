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

void half_sample_uint32_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out)
{
	const uint32_t* top_ptr = (uint32_t*)in;
	const uint32_t* bottom_ptr = (uint32_t*)(in + in_w);
	uint16_t* out_ptr = (uint16_t*)(out);

	const uint32_t mask_00ff = 0x00FF00FF;
	const uint32_t twos = 0x00020002;

	int x_blocks = in_w / 4;
	int out_h = in_h / 2;
	for(int y = 0; y < out_h; ++y)
	{
		for(int x = 0; x < x_blocks; ++x)
		{
			uint32_t top_vals = *top_ptr;
			uint32_t bottom_vals = *bottom_ptr;
			top_ptr++;
			bottom_ptr++;

			// Split into separate vectors for odd and even byte indices
			// to allow the pairwise additions (and to give 16-bits of
			// room to accumulate the totals before dividing by 4)
			uint32_t top_vals_even = top_vals & mask_00ff;
			uint32_t top_vals_odd = (top_vals >> 8) & mask_00ff;
			uint32_t bottom_vals_even = bottom_vals & mask_00ff;
			uint32_t bottom_vals_odd = (bottom_vals >> 8) & mask_00ff;

			// Add them all up, +2 for correct rounding
			uint32_t totals = top_vals_even + top_vals_odd + bottom_vals_even + bottom_vals_odd + twos;

			// Divide by 4 and re-assert the mask for the different elements
			uint32_t average = (totals >> 2) & mask_00ff;

			// Now a shift and OR, to pack the two results into the
			// lower 16 bits
			uint32_t shifted = (average >> 8) | average;
			uint16_t out2 = (uint16_t)shifted;

			*out_ptr = out2;
			out_ptr++;
		}

		top_ptr += x_blocks;
		bottom_ptr += x_blocks;
	}
}

void half_sample_uint32x2_blocks(const uint8_t* in, int in_w, int in_h, uint8_t* out)
{
	const uint32_t* top_ptr = (uint32_t*)in;
	const uint32_t* bottom_ptr = (uint32_t*)(in + in_w);
	uint32_t* out_ptr = (uint32_t*)(out);

	const uint32_t mask_00ff = 0x00FF00FF;
	const uint32_t twos = 0x00020002;

	int x_blocks = in_w / 8;
	int x_skip = in_w / 4;
	int out_h = in_h / 2;
	for(int y = 0; y < out_h; ++y)
	{
		for(int x = 0; x < x_blocks; ++x)
		{
			uint32_t top_vals_1 = *top_ptr;
			uint32_t bottom_vals_1 = *bottom_ptr;
			top_ptr++;
			bottom_ptr++;

			uint32_t top_vals_2 = *top_ptr;
			uint32_t bottom_vals_2 = *bottom_ptr;
			top_ptr++;
			bottom_ptr++;

			// Split into separate vectors for odd and even byte indices
			// to allow the pairwise additions (and to give 16-bits of
			// room to accumulate the totals before dividing by 4)
			uint32_t top_vals_even_1 = top_vals_1 & mask_00ff;
			uint32_t top_vals_odd_1 = (top_vals_1 >> 8) & mask_00ff;
			uint32_t bottom_vals_even_1 = bottom_vals_1 & mask_00ff;
			uint32_t bottom_vals_odd_1 = (bottom_vals_1 >> 8) & mask_00ff;

			uint32_t top_vals_even_2 = top_vals_2 & mask_00ff;
			uint32_t top_vals_odd_2 = (top_vals_2 >> 8) & mask_00ff;
			uint32_t bottom_vals_even_2 = bottom_vals_2 & mask_00ff;
			uint32_t bottom_vals_odd_2 = (bottom_vals_2 >> 8) & mask_00ff;

			// Add them all up, +2 for correct rounding
			uint32_t totals_1 = top_vals_even_1 + top_vals_odd_1 + bottom_vals_even_1 + bottom_vals_odd_1 + twos;
			uint32_t totals_2 = top_vals_even_2 + top_vals_odd_2 + bottom_vals_even_2 + bottom_vals_odd_2 + twos;

			// Divide by 4 and re-assert the mask for the different elements
			uint32_t average_1 = (totals_1 >> 2) & mask_00ff;
			uint32_t average_2 = (totals_2 >> 2) & mask_00ff;

			// Now shifts and ORs to pack the 4 results back for saving
			uint32_t shifted_1 = (average_1 >> 8) | average_1;
			uint32_t shifted_2 = (average_2 >> 8) | average_2;
			uint32_t out4 = (shifted_1 & 0x0000FFFF) | (shifted_2 << 16);

			*out_ptr = out4;
			out_ptr++;
		}

		top_ptr += x_skip;
		bottom_ptr += x_skip;
	}
}
