# llvm-playground

Useful tips on out-of-source LLVM passes.


## Running

We can load our LLVM passes at runtime by using the `-load` option to specify
the path to the shared object for that pass. We can add multiple passes by
using the `-load` option more than once.

To produce a list of passes:

```
$ opt -load lib/LLVMHello.so -help
OVERVIEW: llvm .bc -> .bc modular optimizer and analysis printer

USAGE: opt [subcommand] [options] <input bitcode file>

OPTIONS:
  Optimizations available:
...
    -guard-widening           - Widen guards
    -gvn                      - Global Value Numbering
    -gvn-hoist                - Early GVN Hoisting of Expressions
    -hello                    - Hello World Pass
    -indvars                  - Induction Variable Simplification
    -inferattrs               - Infer set function attributes
...
```

To collect timing information while running a pass:

```
$ opt -load lib/LLVMHello.so -hello -time-passes < hello.bc > /dev/null
Hello: __main
Hello: puts
Hello: main
===-------------------------------------------------------------------------===
                      ... Pass execution timing report ...
===-------------------------------------------------------------------------===
  Total Execution Time: 0.0007 seconds (0.0005 wall clock)

   ---User Time---   --User+System--   ---Wall Time---  --- Name ---
   0.0004 ( 55.3%)   0.0004 ( 55.3%)   0.0004 ( 75.7%)  Bitcode Writer
   0.0003 ( 44.7%)   0.0003 ( 44.7%)   0.0001 ( 13.6%)  Hello World Pass
   0.0000 (  0.0%)   0.0000 (  0.0%)   0.0001 ( 10.7%)  Module Verifier
   0.0007 (100.0%)   0.0007 (100.0%)   0.0005 (100.0%)  Total
```

## Obtaining Line Information

We must compile the program with `-g` and `-O0` for the sake of analysis.
We can use a release build and `-O3` when it comes to validating patches.

* http://llvm.org/docs/LangRef.html#specialized-metadata-nodes
* http://jiten-thakkar.com/posts/how-to-read-and-write-metadata-in-llvm
* http://llvm.org/docs/SourceLevelDebugging.html#c-c-source-file-information


## Helpful Links

* http://llvm.org/docs/CMake.html
* https://releases.llvm.org/11.0.0/docs/GettingStarted.html#llvm-tools
* https://releases.llvm.org/11.0.0/docs/GettingStarted.html#an-example-using-the-llvm-tool-chain
