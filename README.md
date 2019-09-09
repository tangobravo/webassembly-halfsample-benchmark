Half Sample Benchmark
=====================

This repo benchmarks various half-sampling implementations for uint8_t data. The target is currently shipping WebAssembly runtimes in browsers that have support for uint64_t types but no explicit SIMD instructions yet.

To compile/test wasm implementation in d8 (with baseline "liftoff compiler" disabled):
```
$ emcc -O3 test.c halfsample.c -o test.js
$ /path/to/v8/out/x64.release/d8 --no-wasm-tier-up -no-liftoff --predictable test.js
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
