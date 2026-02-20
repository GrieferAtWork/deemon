## Magic byte-patterns in memory

In order to aid in debugging, and make memory images (more) reproducible, this deemon implementation defines a number of *magic* byte-patterns that are used to populate memory regions where the contents could technically also be left undefined.

By knowing these mappings, it becomes easier to understand the state of the memory you're looking at.

| Pattern    | Meaning | Debug-only | Reference |
| ---------- | ------- | ---------- | --------- |
| `CCCCCCCC` | Uninitialized heap memory by `Dee_Malloc`<br/>Various instances of `DBG_memset()` to explicitly trash uninitialized memory | y | `DL_DEBUG_MEMSET_ALLOC` |
| `CDCDCDCD` | Uninitialized stack memory by `Dee_Malloca` | y | `Dee_MALLOCA_SKEW_ALLOCA` |
| `DEADBEEF` | Heap memory after `Dee_Free()` | y | `DL_DEBUG_MEMSET_FREE` |
| `FDFDFDFD` | Uninitialized heap memory in `.dec` files | n | `decwriter_malloc_impl` |
| `FEFEFEFE` | Padding bytes at the end of heap blocks in `.dec` files | n | `decwriter_malloc_impl` |

