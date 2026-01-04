
# .dec file format



## Brief

This document describes and documents the `.dec` file format.



## Introduction

The purpose of `.dec` files is to speed up module imports by allowing the compiler to skip (re-)compilation of `.dee` source files if a matching, and sufficiently up-to-date `.dec` file can be found.

By default, the deemon compiler uses and emits `.<module name>.dec` files for every `.dee` file it encounters when executing user-code and hitting `import` statements or expressions.



## Summary

Every `.dec` file is made up of 3 segments:

- **EHDR**: File header containing information about the remainder of the file
	- aka. "executable header"
	- s.a. `Dec_Ehdr`
- **HEAP**: A specially formatted sequence of data-blobs containing dumps of uninitialized, unrelocated deemon objects
	- s.a. `Dec_Ehdr::e_heap`
	- s.a. `Dec_Ehdr::e_heap`
	- s.a. `struct Dee_heapregion`
- **RELOC**: Various tables of offsets into other parts of the `.dec` files, describing locations where certain types of relocations must be applied
	- s.a. `Dec_Ehdr::e_typedata::td_reloc::*`
	- s.a. `Dec_Rel`
	- s.a. `Dec_RRel`
	- s.a. `Dec_RRela`



## EHDR

- The EHDR is *variable-length*, with its exact length being dependent on various factors relating to how deemon was compiled.
- The exact size of the EHDR can be read from field `er_offsetof_heap`, which specifies the offset to the **HEAP** portion of the `.dec` file
- The following only documents the compilation-independent portion of the EHDR


### Layout

| Offset (hex) | Size | Signed | Field                 | Example              | Validated | Description |
| ------------ | ---- | ------ | --------------------- | -------------------- | --------- | ----------- |
| `00-03`      | `4`  | -      | `e_ident`             | `"\177DEC"`          | Yes       | Magic identification marker for .dec file.<br/>Always has the value `{DECMAG0,DECMAG1,DECMAG2,DECMAG3}` |
| `04-04`      | `1`  | -      | `e_mach`              | `1`                  | Yes       | Host architecture identifier of system that generated this file (one of `Dee_DEC_*`) |
| `05-05`      | `1`  | -      | `e_type`              | `0`                  | Yes       | Type of `.dec` file. Always `0` (aka. `Dee_DEC_TYPE_RELOC`) in files found on-disk |
| `06-07`      | `2`  | -      | `e_version`           | `1`                  | Yes       | Version number of `.dec` file. Always `1` (aka. `DVERSION_CUR`).<br/>For backwards-compatibility with an older `.dec` file format, this is maintained as a 2-byte field at offset `6` |
| `08-09`      | `2`  | No     | `er_offsetof_heap`    | `104`                | Yes       | Offset from start of file to start of **HEAP** segment. As such, this is also the size of the **EHDR** segment |
| `0A-0A`      | `1`  | No     | `er_sizeof_pointer`   | `4`                  | Yes       | The size of a pointer (in bytes). This field also specifies the word-size of locations affected by relocation (s.a. **RELOC**) and the size of fields and links in the **HEAP** |
| `0B-0B`      | `1`  | -      | -                     | `0`                  | Yes       | A (currently) unused field that must be set to `0` (meant for future extensions) |
| `0C-0F`      | `4`  | No     | `er_offsetof_eof`     | `0x000394FB`         | Yes       | Offset to end of **RELOC** segment, and expected to match the total size of the `.dec` file |
| `10-1F`      | `16` | -      | `er_deemon_build_id`  | `0x0011223344556677` `0x8899AABBCCDDEEFF` | Yes | The value of `DeeExec_GetBuildId()`. This field serves the same purpose as `er_deemon_build_id` and `er_deemon_host_id` in verifying that the `.dec` file is compatible with the current deemon installation. |
| `20-27`      | `8`  | No     | `er_build_timestamp`  | `0x00064729E08F8FC0` | -         | The # of milliseconds since `01-01-1970` when the this dec file was generated. When the associated `.dee` file (or any of the files listed by `er_offsetof_files`) is newer than this, then the `.dec` file must be ignored. |
| `28-2B`      | `4`  | No     | `e_offsetof_gchead`   | `0x00000078`         | -         | Offset from start of file to the first `struct gc_head_link`, or `0` if there are no GC objects.<br/>But note that since every `.dec` file must start with an embedded `DeeModuleObject` (which is also a GC-object), this field should never be `0`.<br/>Assuming a regular `.dec` file (as produced by the deemon compiler), this field can also be treated as the offset to the `struct gc_head_link` of the dec file's embedded module object. |
| `2C-2F`      | `4`  | No     | `e_offsetof_gctail`   | `0x000317E0`         | -         | Offset from start of file to the last `struct gc_head_link`, or `0` if there are no GC objects.<br/>Together with `e_offsetof_gchead`, these fields are used with `DeeGC_TrackAll` to start tracking all GC objects within a `.dec` file at the same time as the `.dec` file is made visible to the global module cache.<br/>Pointers other than `e_offsetof_gchead->gc_pself` and `e_offsetof_gctail->gc_next` between every embedded GC object are expected to be initialized using self-relocations (s.a. `er_offsetof_srel` and segment **RELOC**) |
| `30-34`      | `4`  | No     | `er_offsetof_srel`    | `0x000338F4`         | -         | Table of self-relocations.<br/>Offset from start of EHDR to `Dec_Rel[]` array (terminated by `Dec_Rel::r_addr == 0`), using `REL_BASE = (uintptr_t)EHDR` (iow: the base address of where the EHDR was loaded into memory) |
| `35-37`      | `4`  | No     | `er_offsetof_drel`    | `0x00036A8C`         | -         | Table of deemon-core relocations.<br/>Offset from start of EHDR to `Dec_Rel[]` array (terminated by `Dec_Rel::r_addr == 0`), using `REL_BASE = (uintptr_t)&DeeModule_Deemon`<br/>See **RELOC** section on `Dec_Rel` |
| `38-3B`      | `4`  | No     | `er_offsetof_drrel`   | `0x00036AF8`         | -         | Table of reference-counted deemon-core relocations.<br/>Offset from start of EHDR to `Dec_RRel[]` array (terminated by `Dec_RRel::r_addr == 0`), using `REL_BASE = (uintptr_t)&DeeModule_Deemon`<br/>See **RELOC** section on `Dec_RRel` |
| `3C-3F`      | `4`  | No     | `er_offsetof_drrela`  | `0x00036AF8`         | -         | Table of reference-counted-with-addend deemon-core relocations.<br/>Offset from start of EHDR to `Dec_RRela[]` array (terminated by `Dec_RRela::r_addr == 0`), using `REL_BASE = (uintptr_t)&DeeModule_Deemon`<br/>See **RELOC** section on `Dec_RRela` |
| `40-43`      | `4`  | No     | `er_offsetof_deps`    | `0x000338E0`         | -         | Table of additional module dependencies (other than the deemon core).<br/>Since the relative offsets between objects contained within `.dec` files or **DEX** modules (`.so` or `.dll` files) remain consistent, even after a `.dec` file (or **DEX** module) was loaded into memory and relocated, all types of modules can be used as dependencies, and references to every type of object contained within can simply be encoded via relocations (allowing arbitrary encoding of pointers to heap structures or objects of other modules).<br/>Offset from start of EHDR to `Dec_Dhdr[]` array (terminated by `Dec_Dhdr::d_modspec.d_offsetof_modname == 0 && Dec_Dhdr::d_modspec.d_offsetof_rel == 0`)<br/>See **RELOC** section on `Dec_Dhdr` |
| `44-47`      | `4`  | No     | `er_offsetof_files`   | `0x00000000`         | -         | List of extra absolute or relative (to directory containing the `.dec` file) names for files that must each be checked by the `.dec` loader to see if they might be newer than `er_build_timestamp`.<br/>By default, only the corresponding `.dee` file is checked for being newer than the `.dec` file, in which case the `.dec` file is ignored and the associated source file is re-compiled.<br/>When this field is `0`, no extra files must be checked.<br/>This list is terminated by by a `Dec_Dstr::ds_length == 0` entry. List elements follow each other one-after-the-other, with optional, unused padding in-between each other such that every element is aligned to a 4-byte boundary (relative to the start of the file).<br/>See **RELOC** section on `Dec_Dstr` |
| `48-??`      | ?    | ?      | N/A                   | N/A                  | -         | Extra fields are host/implementation-specific, are not necessarily present, and are prone to change. The end (and size) of the **EHDR** is specified in `er_offsetof_heap`. |





## HEAP

- The HEAP encompasses the *main* portion of a `.dec` file, as it contains the actual objects that are encoded within a `.dec` file.
- The layout of the HEAP depends on certain values from the **EHDR**:
	- All (non-payload) fields have a word-size of `Dec_Ehdr::er_sizeof_pointer` bytes each. As such, offsets below are documented as multiples of *words*, rather than bytes.
	- All structures (Header, Chunk, Tail) must be aligned to `2 * Dec_Ehdr::er_sizeof_pointer`-byte boundaries (when viewed as offsets from the start of the `.dec` file).
- The heap consists of 3 structures:
	- `struct Dee_heapregion`: Header
	- `struct Dee_heapchunk`: Chunk (repeated many times)
	- `struct Dee_heaptail`: Tail
- Further documentation on the format used by the HEAP-, why this format is used, and how exactly this works and interfaces with deemon's heap system (i.e. `Dee_Free()`, `Dee_Realloc()`, etc...), can be found in `/deemon/include/heap.h`

### `struct Dee_heapregion` (Header)

The header of the HEAP segment, including information about the segment's size, and other meta-data fields needed by the runtime

#### Layout

| Word | Signed | Field                 | Example              | Description |
| ---- | ------ | --------------------- | -------------------- | ----------- |
| `0`  | No     | `hr_size`             | `0x00001600`         | Offset from start of the heap end of tail (in bytes) |
| `1`  | -      | `hr_destroy`          | `<undefined>`        | Uninitialized memory within the `.dec` file. When loaded into memory, this field is set to `&DeeDec_heapregion_destroy` |
| `2`  | -      | `hr_first`            | `{0x000001C0, 0x000000C4}` | First heap chunk (see **HEAP** section on *`struct Dee_heapchunk` (Chunk)*).<br/>In regular `.dec` files, this first chunk is assumed to hold the `struct gc_head_link` of the primary `DeeModuleObject` as its payload. |
| ...  | ...    | ...                   | N/A                  | ... |
| `hr_size / Dec_Ehdr::er_sizeof_pointer - 2` | - | `hr_tail` | `{0x000000C0, 0x00000000}` | Heap tail (see **HEAP** section on *`struct Dee_heaptail` (Tail)*) |


### `struct Dee_heapchunk` (Chunk)

- Every chunk starts on a 2-word aligned boundary (i.e. `8`-byte on 32-bit, `16`-byte on 64-bit) (s.a. `Dee_HEAPCHUNK_ALIGN`)
- Every chunk starts with a 2-word header, followed by the variable-length payload area (whose size must also be a multiple of 2 words)

#### Layout

| Word | Signed | Field                 | Example              | Description |
| ---- | ------ | --------------------- | -------------------- | ----------- |
| `0`  | No     | `hc_prevsize`         | `0x000001C0`         | Size of preceding chunk (including that chunk's header), in bytes. If this is the first chunk, set to `0` instead.<br/>s.a. `Dee_HEAPCHUNK_PREV()` |
| `1`  | No     | `hc_head`             | `0x000000C4`         | Size of this chunk (including this header), or'd with the number `4`.<br/>Because deemon requires a host pointer size of at least 4 bytes, and heap chunks are always aligned by 2 words, that means that the minimum size-alignment of heap chunks is `8`. As such, this magic `4` flag can never conflict with chunk sizes. It is however required by `Dee_Free()` in order to correctly identify an object chunk as belonging to a custom `struct Dee_heapregion`.<br/>s.a. `Dee_HEAPCHUNK_HEAD()` |
| ...  | ...    | ...                   | ....                 | Chunk payload |
| `hc_head-4` | ... | ...               | ....                 | Next chunk (or `struct Dee_heaptail`) |



### `struct Dee_heaptail` (Tail)

- Special marker for the last chunk of the heap (and thus: the last 2 words of the associated `struct Dee_heapregion`)
- The offset of the heap tail from the start of the **HEAP** segment is always `Dee_heapregion::hr_size - (2 * Dec_Ehdr::er_sizeof_pointer)`


#### Layout

| Word | Signed | Field                 | Example              | Description |
| ---- | ------ | --------------------- | -------------------- | ----------- |
| `0`  | No     | `ht_lastsize`         | `0x000000C0`         | Size of last regular heap chunk (including that chunk's header), in bytes. If this is the first chunk, set to `0` instead. |
| `1`  | -      | `ht_zero`             | `0x00000000`         | Must be zero.<br/>HINT: Since this field must be present in every `.dec` file, and since it is guarantied to consist of at least 4 consecutive 0-bytes, the offset of this field can/is used to encode absent/unused relocation arrays in `Dec_Ehdr` and `Dec_Dhdr` |





## RELOC

- This segment makes up the remainder of a `.dec` file and generally starts at the end of the **HEAP** segment
- No special data-structure exists at the start of this segment (yet). However, once relocations have been applied, a module loader may `munmap()` everything from the original file that comes after the end of the file's **HEAP** segment (i.e. everything after `Dec_Ehdr::er_offsetof_heap + Dee_heapregion::hr_size`)
- The order of structures within the **RELOC** segment is undefined
- The following structures exist within the **RELOC** segment, and are pointed to (using offsets relative to the start of the **EHDR** segment) from the listed sources:
	- `Dec_Rel`. Pointed-to by:
		- `Dec_Ehdr::er_offsetof_srel`
		- `Dec_Ehdr::er_offsetof_drel`
		- `Dec_Dhdr::d_offsetof_rel`
	- `Dec_RRel`. Pointed-to by:
		- `Dec_Ehdr::er_offsetof_drrel`
		- `Dec_Dhdr::d_offsetof_rrel`
	- `Dec_RRela`. Pointed-to by:
		- `Dec_Ehdr::er_offsetof_drrela`
		- `Dec_Dhdr::d_offsetof_rrela`
	- `Dec_Dhdr`. Pointed-to by:
		- `Dec_Ehdr::er_offsetof_deps`
	- `Dec_Dstr`. Pointed-to by:
		- `Dec_Ehdr::er_offsetof_files`
			- special case: points to an array of strings, whereas all other pointers only point to a singular string
		- `Dec_Dhdr::d_offsetof_modname`


### `Dec_Dhdr` (DEC Dependeny HeaDeR)

- Descriptor for a dependency of this `.dec` file.
- This header gives the name of another module that must be loaded in order for this `.dec` file to be usable.
- Additionally, a number of different relocation tables are also specified, each of which can be used to resolve pointer-fields of objects found in the **HEAP** segment, allowing them to link against other objects/heap structures from arbitrary, dependent modules.

#### Layout

| Offset (hex) | Size | Signed | Field                 | Example              | Description |
| ------------ | ---- | ------ | --------------------- | -------------------- | ----------- |
| `00-03`      | `4`  | No     | `d_offsetof_modname`  | `0x000394F0`         | Offset (from start of **EHDR**) to a `Dec_Dstr` specifying the name of the dependent module. This name uses the same (set of) format(s) accepted by deemon's builtin `import` function, and the `DeeModule_Import()` API:<br/><ul><li>Using either libpath-notation: `net.ftp`</li><li>Or relative dot-notation: `.folder.script`</li><li>Or relative file-notation:<ul><li>`./script`</li><li>`./script.dee`</li></ul></li><li>Or absolute file-notation: <ul><li>`/opt/deemon/folder/script`</li><li>`/opt/deemon/folder/script.dee`</li><li>`E:\c\deemon\folder\script`</li><li>`E:\c\deemon\folder\script.dee`</li></ul></li></ul>Relative names are interpreted relative to the directory containing the `.dee` file. |
| `04-07`      | `4`  | No     | `d_offsetof_rel`      | `0x000338DC`         | Table of normal relocations.<br/>Offset from start of EHDR to `Dec_Rel[]` array (terminated by `Dec_Rel::r_addr == 0`), using `REL_BASE = (uintptr_t)(DeeModuleObject *)import(d_offsetof_modname)`<br/>See **RELOC** section on `Dec_Rel` |
| `08-0B`      | `4`  | No     | `d_offsetof_rrel`     | `0x000394E8`         | Table of incref-relocations.<br/>Offset from start of EHDR to `Dec_RRel[]` array (terminated by `Dec_RRel::r_addr == 0`), using `REL_BASE = (uintptr_t)(DeeModuleObject *)import(d_offsetof_modname)`<br/>See **RELOC** section on `Dec_RRel` |
| `0C-0F`      | `4`  | No     | `d_offsetof_rrela`    | `0x000338DC`         | Table of incref-relocations-with-addend.<br/>Offset from start of EHDR to `Dec_RRela[]` array (terminated by `Dec_RRela::r_addr == 0`), using `REL_BASE = (uintptr_t)(DeeModuleObject *)import(d_offsetof_modname)`<br/>See **RELOC** section on `Dec_RRela` |
| `10-1F`      | `16` | No     | `d_buildid`           | `0x0011223344556677` `0x8899AABBCCDDEEFF` | Expected Build ID of dependency. If this doesn't match the dependent module's *actual* Build ID, behavior is the same as if `er_deemon_build_id` were wrong. |




### `Dec_Rel` (DEC RELocation)

Descriptor for a simple relocation.

The following function is used to resolve this type of relocation (assuming that `Dec_Ehdr::er_sizeof_pointer == sizeof(void *)`), using the appropriate value for `REL_BASE`, as specified by the documentation of the field that points at this relocation:

```c
static void apply_rel(Dec_Ehdr *ehdr, Dec_Rel *rel, uintptr_t REL_BASE) {
	char **pointer = (char **)((char *)ehdr + rel->r_addr);
	*pointer += REL_BASE;
}
```

#### Layout

| Offset (hex) | Size | Signed | Field    | Example      | Description |
| ------------ | ---- | ------ | -------- | ------------ | ----------- |
| `00-03`      | `4`  | No     | `r_addr` | `0x000005C4` | Offset (from start of **EHDR**) to a word-sized (as per `Dec_Ehdr::er_sizeof_pointer`) location that must incremented by the relevant `REL_BASE`. |


### `Dec_RRel` (DEC Reference-counted RELocation)

Reference-counted relocation. Applied the same way as `Dec_Rel`, but after being applied, the resulting pointer must be dereferenced, interpreted as a `DeeObject *`, and that object must then be incref'd.

The following function is used to resolve this type of relocation (assuming that `Dec_Ehdr::er_sizeof_pointer == sizeof(void *)`):

```c
static bool apply_rrel(Dec_Ehdr *ehdr, Dec_RRel *rel, uintptr_t REL_BASE) {
	char **pointer = (char **)((char *)ehdr + rel->r_addr);
	DeeObject *obj;
	*pointer += REL_BASE;
	// Notice the indirection here: the **value** of the pointer **AFTER** being relocated
	//                              is interpreted as an object, and then incref'd!
	obj = (DeeObject *)*pointer;
	return Dee_IncrefIfNotZero(obj);
}
```

If this function returns `false`, the dec file cannot be loaded, all already-performed incref operations are reverted, and behavior is the same as if `er_deemon_build_id` were wrong.


#### Layout

| Offset (hex) | Size | Signed | Field    | Example      | Description |
| ------------ | ---- | ------ | -------- | ------------ | ----------- |
| `00-03`      | `4`  | No     | `r_addr` | `0x000005C4` | Offset (from start of **EHDR**) to a word-sized (as per `Dec_Ehdr::er_sizeof_pointer`) location that must incremented by the relevant `REL_BASE`. After having been adjusted, that pointer's value must then be read (i.e.: the pointer must be dereferenced), and the result must be interpreted as a `DeeObject *`, which must then be `Dee_Incref`'d. |


### `Dec_RRela` (DEC Reference-counted-with-Addend RELocation)

Reference-counted-with-addend relocation. Same as `Dec_RRel`, but provides an additional addend that must be added to the dereferenced pointer after it was already relocated, in order to reach the base address of the relevant object.

The following function is used to resolve this type of relocation (assuming that `Dec_Ehdr::er_sizeof_pointer == sizeof(void *)`):

```c
static bool apply_rrela(Dec_Ehdr *ehdr, Dec_RRela *rel, uintptr_t REL_BASE) {
	char **pointer = (char **)((char *)ehdr + rel->r_addr);
	DeeObject *obj;
	*pointer += REL_BASE;
	// Note that "r_offs" is **SIGNED**, meaning that negative offsets are also allowed here!
	obj = (DeeObject *)(*pointer + rel->r_offs);
	return Dee_IncrefIfNotZero(obj);
}
```

If this function returns `false`, the dec file cannot be loaded, all already-performed incref operations are reverted, and behavior is the same as if `er_deemon_build_id` were wrong.


#### Layout

| Offset (hex) | Size | Signed | Field    | Example      | Description |
| ------------ | ---- | ------ | -------- | ------------ | ----------- |
| `00-03`      | `4`  | No     | `r_addr` | `0x000005C4` | Offset (from start of **EHDR**) to a word-sized (as per `Dec_Ehdr::er_sizeof_pointer`) location that must incremented by the relevant `REL_BASE`. After having been adjusted, that pointer's value must then be read (i.e.: the pointer must be dereferenced), `r_offs` (a **signed** offset) must be added to it, and the result must be interpreted as a `DeeObject *`, which must then be `Dee_Incref`'d |
| `04-07`      | `4`  | Yes    | `r_off`  | `0xFFFFFFEC` | A **signed** offset that must be added to the pointer's value after it was relocated, in order to reach the base address of a `DeeObject` that must then be incref'd. |



### `Dec_Dstr` (DEC Dependeny STRing)

A fixed-length (but still NUL-terminated) string. When this string appears within an array, additional elements are always aligned to a multiple of `4` bytes (with extra space in-between remaining unused / uninitialized).


#### Layout

| Offset (hex)  | Size        | Signed | Field       | Example      | Description |
| ------------- | ----------- | ------ | ----------- | ------------ | ----------- |
| `00-03`       | `4`         | No     | `ds_length` | `0x00000006` | Length of the string (in bytes, excluding the terminating `'\0'`-character) |
| `04-??`       | `ds_length` | -      | `ds_string` | `"errors"`   | The actual string itself, encoded using `utf-8` |
| `4+ds_length` | `1`         | -      | N/A         | `0x00`       | A trailing NUL-byte |
| ...           | ?           | -      | N/A         | N/A          | Unused padding |
| `4+(ds_length+4)&~3` | ...  | ...    | ...         | ...          | Next `Dec_Dstr` (if part of array) |
