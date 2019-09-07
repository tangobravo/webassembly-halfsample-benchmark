#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "tick.h"
#include "busyloop.h"
#include "halfsample.h"

// The output of the benchmark function depends on a variable
// that can theoretically be changed and the random seed, for which
// a setter function is also exported; this should stop any extreme
// compiler optimizations from deciding they can avoid doing the
// full half-sample operations we are trying to benchmark
static int benchmark_accumulate_index = 0;

EMSCRIPTEN_KEEPALIVE
void set_benchmark_accumulate_index(int index)
{
	benchmark_accumulate_index = index;
}

EMSCRIPTEN_KEEPALIVE
void set_random_seed(unsigned int seed)
{
	srand(seed);
}

const int NUM_FUNCTIONS = 2;

EMSCRIPTEN_KEEPALIVE
double benchmark(int func, size_t in_w, size_t in_h, size_t iterations,
	size_t iteration_stride, int* accumulator_result)
{
	size_t in_byte_count = in_w * in_h;
	size_t out_byte_count = (in_w / 2) * (in_h / 2);

	iteration_stride = (iteration_stride / 8) * 8; // Ensure multiple of 8
	size_t in_bytes_total = in_byte_count + iteration_stride * (iterations - 1);

	uint8_t* in_data = (uint8_t*)malloc(in_bytes_total);
	uint8_t* out_data = (uint8_t*)malloc(out_byte_count);

	if(in_data == 0 || out_data == 0) {
		free(in_data);
		free(out_data);
		return -1;
	}

	// Fill in the random bytes
	for(size_t i = 0; i < in_bytes_total; ++i) {
		in_data[i] = (uint8_t)(rand() & 0xff);
	}

	uint8_t* in_im = in_data;
	int accumulator = 0;

	tick_t start = tick();

	for(int i = 0; i < iterations; ++i)
	{
		if(func == 0)
			half_sample_plain(in_im, in_w, in_h, out_data);
		else
			half_sample_uint64_blocks(in_im, in_w, in_h, out_data);

		in_im += iteration_stride;
		accumulator += out_data[benchmark_accumulate_index];
	}

	tick_t end = tick();

	double ticks_per_ms = ((double)ticks_per_sec()) / 1000.0;
	double ms = (double)(end - start) / ticks_per_ms;

	free(in_data);
	free(out_data);

	*accumulator_result = accumulator;
	return ms;
}

EMSCRIPTEN_KEEPALIVE
void run_benchmark()
{
	int test_w = 1280;
	int test_h = 720;

	// Set the test pixel for accumulation to the middle of the
	// ouput image
	int out_stride = test_w / 2;
	int out_mid_px = (test_h / 4) * out_stride + (test_w / 4);
	set_benchmark_accumulate_index(out_mid_px);

	// Busy loop to compare 32/64 bit performance and to give the
	// OS time to ramp the CPU up to max frequency
	uint32_t busy_32_results[10];
	uint64_t busy_64_results[10];
	for(int i = 0; i < 10; ++i)
	{
		busy_32_results[i] = busy_loop_32_default(10);
		busy_64_results[i] = busy_loop_64_default(10);
	}

	// Now do the benchmark 10 times, using a small stride so most
	// of the memory is likely to be in cache
	double timings[NUM_FUNCTIONS][10];
	for(int i = 0; i < 10; ++i)
	{
		int first_accumulator_result = 0;
		for(int f = 0; f < NUM_FUNCTIONS; ++f)
		{
			// Adjust the order of calls for each iteration
			// I don't expect it to affect timings, but just in case...
			int func = (i + f) % NUM_FUNCTIONS;

			// Call the benchmark function
			int accumulator = 0;
			set_random_seed(i);
			timings[func][i] = benchmark(func, test_w, test_h, 10, 16, &accumulator);
			if(f == 0) {
				first_accumulator_result = accumulator;
			} else {
				if(accumulator != first_accumulator_result) {
					printf("Error: Methods disagree\n");
					return;
				}
			}
		}
	}

	// Print out the results
	printf("======\nBusy loop counts (32 bit | 64 bit function):\n");
	for(int i = 0; i < 10; ++i)
		printf("%" PRIu32 " | %" PRIu64 "\n", busy_32_results[i], busy_64_results[i]);
	printf("======\n");

	printf("Benchmark timings (ms):\n");
	for(int f = 0; f < NUM_FUNCTIONS; ++f)
	{
		if(f == 0) printf("half_sample_plain:\n");
		if(f == 1) printf("half_sample_uint64_blocks:\n");

		for(int i = 0; i < 10; ++i)
			printf(" %0.3f\n", timings[f][i]);
	}
	printf("======\n");
}

int main()
{
#ifdef __EMSCRIPTEN__
	printf("Running benchmark every 5 seconds...\n");
	EM_ASM(
		var func = (typeof setInterval === 'function' ? setInterval : setTimeout);
		func(Module._run_benchmark, 5000);
	);
#else
	run_benchmark();
#endif
	return 0;
}
