This directory contains option-specific implementations of parts of `n.h` and `n.c` source files to be used when the
`OPT_UNICODE` and/or `OPT_ENHTTY` options are set.

When compiled with the `OPT_UNICODE` option, SKINT represents strings internally as UTF-8 encoded byte sequences prefixed 
by metadata headers containing both the logical character count and the total byte length. File I/O, file system paths, 
environment variables, and system commands are handled using UTF-8. On Windows systems, this is implemented by translating 
UTF-8 strings to wide characters for native API calls. The interpreter also automatically detects and skips UTF-8 Byte 
Order Marks (BOM) upon opening files, and encodes null characters using Modified UTF-8 (0xC0 0x80) to allow safe 
propagation through standard C-string boundaries. 

Character access and modification performance varies based on the 
contents of the string. For purely ASCII strings (where the byte count matches the logical character count), character access 
via index runs in O(1) constant time. However, for strings containing multibyte Unicode characters, index-based operations 
(such as `string-ref` and `string-set!`) require a linear scan from the beginning of the string to decode preceding UTF-8 sequences, 
resulting in O(N) asymptotic performance where N is the target character index. Modifying such a string may trigger reallocation
of the associated storage unless UTF-8 lengths of the new and the old character match. 

When compiled with the OPT_ENHTTY option, SKINT provides enhanced console interaction and line-editing capabilities 
for the REPL across different operating systems. On Windows, the interpreter queries the original console code pages and 
configures the input and output streams to UTF-8 (CP_UTF8) for proper rendering. Text input and output are translated to and 
from wide characters via native console APIs (`ReadConsoleW` and `WriteConsoleW`), allowing standard console input operations. 
On exit, a registered cleanup callback automatically restores the terminal's original code page state.

On Linux and MacOS, SKINT dynamically probes and loads compatible terminal-editing libraries at runtime. Using dynamic 
linking interfaces (`dlopen` and `dlsym`), it scans a prioritized list of candidate shared libraries, including various 
versions of GNU Readline (`libreadline`) and BSD Editline (`libedit`), located in standard system paths or MacOS package managers 
(such as Homebrew and MacPorts). Resolving these symbols dynamically ensures that the resulting executable remains portable 
and runnable even if a specific line-editing library is absent on the host environment.

Once a terminal library is successfully bound, the module enables interactive features such as persistent command history 
and tab-based autocompletion. The module resolves API hooks for history management, enabling the loading, persistent storage, 
and append operations of REPL command lines in a designated history file (`~/.skint_history`). 
Additionally, it maps completion hooks to support filename generation and 
custom symbol matching based on identifiers loaded from `~/.skint_completion`, improving interactive REPL navigation.


