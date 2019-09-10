Half Sample Benchmark
=====================

This repo benchmarks various half-sampling implementations for uint8_t data. The target is currently shipping WebAssembly runtimes in browsers that have support for uint64_t types but no explicit SIMD instructions yet.

There are various implementations that handle multiple data elements packed into regular uint32_t or uint64_t elements. Some bit twiddling is required to stop the elements from interfering. Here are some handy references to the general ideas and some specific implementations:
http://aggregate.org/SWAR/over.html (describes general ideas of SWAR - "SIMD Within a Register")
http://aggregate.org/MAGIC (general twiddling algorithms)
http://graphics.stanford.edu/~seander/bithacks.html (more bit twiddling stuff)

To compile/test wasm implementation in d8 (with baseline "liftoff compiler" disabled):
```
$ emcc -O3 test.c halfsample.c -o test.js
$ path/to/v8/out/x64.release/d8 --no-wasm-tier-up -no-liftoff --predictable test.js
```

To test in a browser, you can just output html from emcc, run a server in the folder and open the page in a browser:
```
$ emcc -O3 test.c halfsample.c -o test.html
```

The code can also run natively. I'm on MacOS and have the latest Xcode command line tools installed.

The WebAssembly compilers in current browsers don't do any sort of auto-vectorization, so to make it a somewhat fairer comparison I also tend to disable that for the native build:
```
$ clang -O3 -fno-vectorize -fno-slp-vectorize test.c halfsample.c -o test
$ ./test
```

I've also added an attempt at a WebAsembly SIMD implementation. It compiles successfully with the following command line:
```
$ emcc -O3 -msimd128 -fno-vectorize -fno-slp-vectorize -DUSE_WASM_SIMD test.c halfsample.c -o test.js
```

However this currently fails to run in d8 7.7 on my MacBook Pro:
```
$ path/to/v8/out/x64.release/d8 --no-wasm-tier-up -no-liftoff --predictable --experimental-wasm-simd test.js
failed to asynchronously prepare wasm: CompileError: WebAssembly.instantiate(): Compiling function #52 failed: invalid shift amount @+20117
CompileError: WebAssembly.instantiate(): Compiling function #52 failed: invalid shift amount @+20117
CompileError: WebAssembly.instantiate(): Compiling function #52 failed: invalid shift amount @+20117
```
