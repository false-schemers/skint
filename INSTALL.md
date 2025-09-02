Installing SKINT
================

Quick build
---------------
If you only need the interpreter **right now** on a Unix-like box (x86, ARM, RISC-V):

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
All standard R7RS-small libraries are already baked in; no external files are required.

Traditional build (Unix & macOS)
---------------------------------
1. **Get the sources**
   ```bash
   git clone https://github.com/false-schemers/skint.git
   cd skint
   ```

2. **(Optional) pick a prefix**  
   Default installation path is `/usr/local`.  
   To install elsewhere, run:
   ```bash
   ./configure --prefix=$HOME/opt/skint
   ```

3. **Build**
   ```bash
   make           # uses the included Makefile
   ```

4. **Run the test suite** (≈ 1 s)
   ```bash
   make test
   ```

5. **Install**
   ```bash
   sudo make install        # or just `make install` if prefix is writable
   ```

If you haven't changed the prefix, after installation the command `skint` appears in your `$PATH`.

Fine-tuning the build
---------------------
| Goal | Example command |
|---|---|
| Maximum speed, 64-bit | `clang -O3 -DNDEBUG -DNAN_BOXING [skint].c -lm` |
| Tiny debug build | `gcc -O0 -g -DDEBUG [skint].c -lm` |
| Static binary | `gcc -static -O2 [skint].c -lm` |

- `-DNAN_BOXING` activates NaN-boxing (needs 48-bit address space, typical on x86-64).  
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
All standard R7RS-small libraries are already baked in; no external files are required.

What you get
------------
- A **complete R7RS-small** Scheme system.  
- **No runtime dependencies** beyond the C library and `libm`.  
- Works on **32-bit and 64-bit** x86, ARM, RISC-V, etc. (anything with a C compiler).  
- Static linking (`-static`) is supported for USB-stick portability.

Uninstall for traditional build
---------
```bash
sudo make uninstall   # removes the binary and any man pages
```

Need help?
----------
Open an issue at  
<https://github.com/false-schemers/skint/issues>
