/* scmrt.h -- the $scmrt module's payload/wrapper services as seen
 * from other WCPL C modules (currently lib/scmi.c, whose allocating
 * instructions build string/bytevector/port payloads through them).
 *
 * Under WCPL this header is how a module declares calls into
 * $scmrt: the #pragma module line makes the compiler emit
 * (import "scmrt" ...) for each extern used.  Native builds never
 * include this file -- lib/scmi.c declares the same entry points
 * itself, renamed to their scmrt_ C symbols by its #define block.
 */

#pragma once
#pragma module "scmrt"

#include <stdint.h>

extern void *make_string_payload(int32_t len, int32_t fill);
extern uint64_t string_type(void);
extern void *make_bytevector_payload(int32_t len);
extern uint64_t bytevector_type(void);
extern void *symbol_to_string(uint64_t o);
extern void *number_to_string(uint64_t o, uint64_t radix);
extern void *open_input_file(uint64_t o);
extern void *open_output_file(uint64_t o);
extern void *try_open_input_file(uint64_t o);
extern void *try_open_output_file(uint64_t o);
extern void *make_input_string_port(uint64_t o);
extern void *make_output_string_port(void);
extern void *get_output_string_payload(uint64_t o);
extern void *make_input_bytevector_port(uint64_t o);
extern void *get_output_bytevector_payload(uint64_t o);
extern void *make_output_bytevector_port(void);
extern int32_t port_flags(uint64_t o);
extern void *get_env_payload(uint64_t o);
extern uint64_t port_type(void);
