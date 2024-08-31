#ifndef JDUPES_CHUNKCACHE_H
#define JDUPES_CHUNKCACHE_H

/* jdupes chunk cache variables
 * This file is part of jdupes; see jdupes.c for license information */

#include <stdint.h>

/* jdupes requires large chunks of memory to use
 * for reading in files. This is performed in file hashing
 * and in match checking.
 * Since the data within the chunks does not need to be preserved
 * between function calls, optimize memory allocation by allowing
 * functions to share chunk memory.
 */
extern uint64_t *j_chunk1;
extern uint64_t *j_chunk2;

#endif /* JDUPES_CHUNKCACHE_H */
