Installing SKINT
================

Quick build
---------------
If you only need the interpreter **right now** on a Unix-like box (64-bit: x86, ARM, Power, RISC-V):

1. **Get the sources**
   ```bash
   git clone https://github.com/false-schemers/skint.git
   cd skint
   ```

2. **Build**  
   ```bash
   gcc -O3 -DNDEBUG -DNAN_BOXING -o skint [skint].c -lm    # or use clang if it is installed
   ```
This produces a standalone binary that you can copy anywhere you like.  
All standard R7RS-small libraries are already baked in; no external files are needed.

In addition to the standard libraries built into the interpreter, SKINT provides external libraries in the distribution’s 
`lib` subdirectory. If you perform a manual install, you can copy this folder to any location you choose and tell SKINT 
where to find it either with `-I` / `-A` command-line options or `SKINT_LIBDIRS` environment variable. Alternatively, you can 
hard-code the library path into the interpreter by adding `-D LIBDIR=`*lib_path* when you compile it.


Traditional build (Unix & macOS)
---------------------------------
1. **Get the sources**
   ```bash
   git clone https://github.com/false-schemers/skint.git
   cd skint
   ```

2. **(Optional) pick a prefix**  
   Default installation path is `/usr/local`.  
   To install elsewhere, run e.g.:
   ```bash
   ./configure --prefix=$HOME/opt/skint
   ```

3. **Build**
   ```bash
   make           # uses the included Makefile
   ```

4. **Run the test suite** (≈ 2 s)
   ```bash
   make test
   ```
   
5. **Run the external libraries test suite** (≈ 20 s)
   ```bash
   make libtest
   ```
   
5. **Install**
   ```bash
   sudo make install        # or just `make install` if prefix is writable
   sudo make libinstall     # ditto, install optional external libraries
   ```

If you haven't changed the prefix, after installation the command `skint` appears in your `$PATH`.

Fine-tuning the build
---------------------
| Goal | Example command |
|---|---|
| Maximum speed, 64-bit | `clang -O3 -DNDEBUG -DNAN_BOXING [skint].c -lm` |
| Tiny debug build | `gcc -O0 -g -DDEBUG [skint].c -lm` |
| Static binary | `gcc -static -O2 [skint].c -lm` |

- `-DNAN_BOXING` activates NaN-boxing (needs 48-bit address space, typical on 64-bit Linux/MacOS/Windows).  
- `-DNDEBUG` removes internal assertions.  
- All options can also be set through `make CFLAGS=... LDFLAGS=...`.

Windows (MSVC / MinGW)
----------------------
- **MSVC (Developer Prompt)**  
  ```cmd
  cl /O2 /DNDEBUG /DNAN_BOXING /Fe"skint.exe" s.c k.c i.c n.c t.c
  ```
- **NMAKE** – see `misc/README.md` for the ready-made makefile.  
- **MinGW** – same flags as Unix:  
  ```cmd
  gcc -O2 -DNDEBUG -DNAN_BOXING -o skint.exe s.c k.c i.c n.c t.c -lm 
  ```

The resulting `skint.exe` is fully self-contained and can be copied between Windows machines.
All standard R7RS-small libraries are already baked in; no external files are needed.

In addition to the standard libraries built into the interpreter, SKINT provides external libraries in the distribution’s 
`lib` subdirectory. If you perform a manual install, you can copy this folder to any location you choose and tell SKINT 
where to find it either with `-I` / `-A` command-line options or `SKINT_LIBDIRS` environment variable. Alternatively, you can 
hard-code the library path into the interpreter by adding `-D LIBDIR=`*lib_path* when you compile it.

What you get
------------
- A **complete R7RS-small** Scheme system (no external files needed)  
- **No runtime dependencies** beyond the C library and `libm`.  
- Works on **32-bit and 64-bit** x86, ARM, RISC-V, etc. (anything with a C compiler).  
- Static linking (`-static`) is supported for USB-stick portability.
- Optional `lib/` tree: **100+ SRFI** ports ready for `(import)`

Uninstall for traditional build
---------
```bash
sudo make uninstall   # removes the binary
sudo make libuninstall   # removes the optional lib tree
```

Need help?
----------
Open an issue at  
<https://github.com/false-schemers/skint/issues>
