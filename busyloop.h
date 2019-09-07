// The emscripten tick.h doesn't support being included in more
// than one implementation file, so include this one after
// #include "tick.h" in the main test.c file

EMSCRIPTEN_KEEPALIVE
uint32_t busy_loop_32(uint32_t ms, uint32_t inner_start_mask,
	uint32_t inner_end, uint32_t inner_step)
{
	tick_t ms_per_sec = 1000;
	tick_t loop_ticks = (ticks_per_sec() * (tick_t)ms) / ms_per_sec;

	tick_t latest_tick = tick();
	tick_t end_ticks = latest_tick + loop_ticks;

	uint32_t outer_loops = 0;
	while(latest_tick <= end_ticks)
	{
		// Make the inner loop depend on the outer one, so compiler
		// can't hoist it out, and make it depend on parameters so
		// it can't compute any results at compile time...
		uint32_t i = outer_loops & inner_start_mask;
		while(i < inner_end)
		{
			i += inner_step;
			const uint32_t mask = 0x80800000;
			if((i & mask) == mask) return 0;
		}

		outer_loops++;
		latest_tick = tick();
	}

	return outer_loops;
}

// Make these globals so compiler has to assume they may change...
uint32_t default_start_mask_32 = 0;
uint32_t default_end_32 = 1 << 12;
uint32_t default_step_32 = 1;

EMSCRIPTEN_KEEPALIVE
uint32_t busy_loop_32_default(uint32_t ms)
{
	return busy_loop_32(ms, default_start_mask_32, default_end_32, default_step_32);
}

EMSCRIPTEN_KEEPALIVE
uint64_t busy_loop_64(uint32_t ms, uint64_t inner_start_mask,
	uint64_t inner_end, uint64_t inner_step)
{
	tick_t ms_per_sec = 1000;
	tick_t loop_ticks = (ticks_per_sec() * (tick_t)ms) / ms_per_sec;

	tick_t latest_tick = tick();
	tick_t end_ticks = latest_tick + loop_ticks;

	uint64_t outer_loops = 0;
	while(latest_tick <= end_ticks)
	{
		// Make the inner loop depend on the outer one, so compiler
		// can't hoist it out, and make it depend on parameters so
		// it can't compute any results at compile time...
		uint64_t i = outer_loops & inner_start_mask;
		while(i < inner_end)
		{
			i += inner_step;
			const uint64_t mask = 0x80800000;
			if((i & mask) == mask) return 0;
		}

		outer_loops++;
		latest_tick = tick();
	}

	return outer_loops;
}

// Makes these globals so compiler has to assume they may change...
uint64_t default_start_mask_64 = 0;
uint64_t default_end_64 = 1 << 12;
uint64_t default_step_64 = 1;

EMSCRIPTEN_KEEPALIVE
uint64_t busy_loop_64_default(uint32_t ms)
{
	return busy_loop_64(ms, default_start_mask_64, default_end_64, default_step_64);
}
