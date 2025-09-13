# deemon - Deemon scripting language

[![Linux](https://github.com/GrieferAtWork/deemon/actions/workflows/linux.yml/badge.svg)](https://github.com/GrieferAtWork/deemon/actions/workflows/linux.yml) [![Windows (x32)](https://github.com/GrieferAtWork/deemon/actions/workflows/windows-x32.yml/badge.svg)](https://github.com/GrieferAtWork/deemon/actions/workflows/windows-x32.yml) [![Windows (x64)](https://github.com/GrieferAtWork/deemon/actions/workflows/windows-x64.yml/badge.svg)](https://github.com/GrieferAtWork/deemon/actions/workflows/windows-x64.yml)

Deemon is a scripting language with a syntax inspired by C and other languages derived from C, whilst featuring a python-like runtime.

```
import * from deemon;

print "Deemon is totally rad, right?";
print "> ",;
local l = File.stdin.readline().strip().lower();
if (l in { "y", "yes" }) {
	print "Thank you! :D";
} else if (l in { "n", "no" }) {
	print "Aww... You're hurting my feelings :(";
} else {
	print "Huh?";
}
```

Deemon is designed with the following features in mind:
- Being portable to any Unix-like operating system (obviously including Linux), as well as Windows
- Working with, and processing data via sequences and sequence proxies, rather than individually or by always creating new list objects
- Being faster than python while still offering similar runtime functionality (mainly by not having a GIL, a more powerful instruction set, and using static symbol resolution, rather than having locals/globals be runtime hash-mappings)


## Building

**NOTICE**: Deemon uses git submodules (which are required to build deemon), so if you use the *download zip* function, you won't end up with everything that goes into building deemon. So in order to clone deemon in its entirety, you must clone this [git](https://git-scm.com/) through use of:

```sh
git clone --recursive https://github.com/GrieferAtWork/deemon.git
```

With configure  
```sh
bash ./configure
make -j $(nproc)
```

Cross-compiling deemon (using `/opt/kos/binutils/i386-kos/bin/i686-kos-gcc`)  
```sh
bash ./configure --cross-prefix=/opt/kos/binutils/i386-kos/bin/i686-kos-
make -j $(nproc)
```

With visual studio  

- Open `/.vs/deemon-{PlatformToolset}.sln`
	- Select whichever `PlatformToolset` you have installed
	- Deemon *should* be able to compile for any toolset, but sadly there's no way to just tell VS to "use whatever you have installed"
- Select your preferred build configuration and architecture
- `CTRL+SHIFT+B`



## Features

### Core syntax features

- Modular code design with one script able to import another script's globals
- Sequences anywhere: anywhere expressions appear as `,`-separated lists, it's possible to use a sequence as `seq...`, causing its elements to appear as distinct elements. E.g. to concat two sequences, do `{ a..., b... }`
- Object-oriented programming and classes
	- Polymorphism
	- Type inheritance, and multiple base classes
	- Member functions and properties (getsets)
	- Private and public members
	- User-defined constructors and destructors
	- User-definable operators
- All of your usual C-like statements/expressions
- Co-routines (aka. "yield"-functions)
- Generator expressions (`x = for (local elem: y) if (elem) elem + 1;`)
- Java-like lambda functions (`list.sort(key: e -> e.casefold())`)
- Type annotations (`local x: {string: int} = Dict();`)
- Exceptions and try-catch/finally
- With-statements (`with (lock) { ... }`)
- GCC-style statements-in-expressions (`local x = ({ local y = 10; y + 20; });`)
- Optionally assigned variables (`local x; if (y) { x = 10; } print x is bound;`)
- Default/optional/keyword arguments in functions (`function foo(x, y = 10, z?) {} foo(1, z: "Hello");`)
- ...

### Core runtime library

- Full Unicode support (`assert "²".asnumeric() == 2`)
	- Fully featured string API (for doing anything you might think of)
	- Built-in regular expression support
- Infinite-precision integers (`2 ** 999 == 3575430359[...]2834034688`)
- Sequence, Set, and Map-like built-in containers
	- Mutable/Immutable sequences (`List` / `Tuple`)
	- Mutable/Immutable mappings (`Dict` / `Dict.Frozen`)
	- Mutable/Immutable sets (`HashSet` / `HashSet.Frozen`)
	- Everything inherits from a common `Sequence` class providing a fully-featured sequence API
- Everything is reference-counted (automatic cleanup)
- Modules are compiled on first use and automatically re-compiled if modified
- Deemon scripts are compiled and executed as bytecode for better performance
- Full multi-threading support everywhere (everything is thread-safe by default)
	- And I mean *real* multi-threading (i.e. deemon doesn't have a GIL)
- Floating point numbers
- File/std I/O
- Dynamic module imports (`local x = "deemon"; local y = import(x);`)
- Execute strings as code (`print exec("(a, b) -> a + b")(10, 20);`)

### Extended runtime library
- Modules are either native shared libraries (`.so` or `.dll`), or deemon scripts (`.dee`)
- Modules for:
	- Extra sequence types (`collections`)
	- Interfacing with C functions and native system libraries (`ctypes`)
	- Disassembling deemon bytecode (`disassembler`)
	- System APIs (`fs`, `posix`, `win32`)
	- Spawning and controlling sub-processes (`ipc`)
	- Mathematic functions like `sin`, `cos`, etc. (`math`)
	- Low-level networking and sockets (`net`)
	- Extra threading functionality like locks and TLS variables (`threading`)
	- Timestamps and time deltas (`time`)
	- Interfacing with doc strings and other RTTI (`doc`, `doctext`, as well as `/util/doc-server.dee`)
	- ...

Code examples can be found in */util/tut*




## Comparison with deemon100+


### Major improvements

- Introduction of a module-based dependency system that allows code reuse without relying on preprocessor functionality that really didn't fit a scripting language all too well.
- With more emphasis on documentation, deemon now comes shipped with a documentation server accessible via web-browser
	- It should be noted that the documentation server, as well as the RTTI parser are written entirely in deemon.
	- Links listed below require this server to be running locally.
- A complete overhaul of the builtin [`string`](http://localhost:8080/modules/deemon/i:string) type
	- Full unicode support all packed together into a single string type
	- Separation between raw data [`Bytes`](http://localhost:8080/modules/deemon/i:Bytes) and [`string`](http://localhost:8080/modules/deemon/string)s, as well as functionality to decode/encode data and strings
	- Builtin support for regular expressions
	- Addition of miscellaneous functions such as [`indent()`](http://localhost:8080/modules/deemon/i:string/i:indent) or [`findmatch()`](http://localhost:8080/modules/deemon/i:string/i:findmatch) to help in situation where the old string type was struggling
	- Addition of case-insensitive variants of many functions, such as [`casefind()`](http://localhost:8080/modules/deemon/i:string/i:casefind)
- Introduction of a common base class for any [`Sequence`-like type](http://localhost:8080/modules/deemon/i:Sequence)
	- Includes emulation of any kind of sequence operator, as well as a huge set of member functions, including [`find()`](http://localhost:8080/modules/deemon/i:Sequence/find), [`sum()`](http://localhost:8080/modules/deemon/i:Sequence/sum), [`operator add`](http://localhost:8080/modules/deemon/i:Sequence/op:add) or comparisons
	- Also introduced are common base classes for set-like, and mapping-like objects
- Introduction of ASP (Abstract Sequence Proxy)-like objects to allow for lazy computation in functions like [`string.split()`](http://localhost:8080/modules/deemon/i:string/i:split), distributing work load across usage and essentially making such functions O(1) when invoked
- Introduction of default, optional, and named function arguments
	- `function foo(a, b = 10, c?)`
		- `foo("Hello")` Called as `foo(a: "Hello", b: 10, c: /unbound/)`
		- `foo("Hello", "World")` Called as `foo(a: "Hello", b: "World", c: /unbound/)`
		- `foo("Hello", c: "Universe")` Called as `foo(a: "Hello", b: 10, c: "Universe")`
- Introduction of a same-object / different-object operators `===` and `!==`
- Introduction of a new syntax for constructing a [super-view](http://localhost:8080/modules/deemon/i:Super) `foo as Sequence`
- Introduction of a new syntax for checking if variables or attributes are bound `if (x is bound) print x;`
- Introduction of `with`-statements, useful when dealing with files or locks
	- To go alongside, 2 new operator `operator enter()` and `operator leave()` were introduced
- Introduction of an interactive excution mode `deemon -i` where code is executed, and results are printed in real time
- Introduction of a `deepcopy` keyword and operator to go alongside the `copy` keyword
	- Also includes automatic tracking of recursive objects such as a list containing itself.
- Added support for raw string literals `r"the following are 2 seperate characters: \n"`
- Overhaul of exception handlers in user-code now introduces zero-effort exception and finally handlers (as opposed to having a stack of active handlers)
- Overhaul of user-classes which now require member variables to also be declared, significantly improving runtime performance
- Lazy compilation of module source files into pre-compiled file caches improves load time significantly
- Extremely powerful peephole optimization of generated bytecode
- The bytecode generated by deemon has grown so powerful that you can actually write code using it, or have it be printed back to you by a powerful, builtin disassembler (try `deemon -S lib/doc.dee`)
	- If you look at it, it really has more in common with that of a CISC architecture, featuring admirable text compression rates, while still executing assembly as fast as possible
- Added compiler warnings for various questionable cases (including use of reserved keywords as symbol names)
- I actually took the time to write a copy of the entire interpreter in i386 assembly (by hand), providing a significant performance boost on 32-bit Intel machines.
- The builtin [`int`](http://localhost:8080/modules/deemon/i:int) type can have arbitrary precision now, allowing operations with a practically infinite number of digits (though I'm not claiming credit for the implementation; only for the integration and new design centered around it)
- Introduction of type annotations
	- `function add(x: int, y: int): int`
	- Mainly intended for simplified and less redundant documentation, but may also be used for other purposes
- Addition of runtime functionality to execute strings as code
	- `print ([exec from deemon](http://localhost:8080/modules/deemon/i:exec))("10 + 20");`
	- Uses a seperate JIT compiler that directly executes source text, rather than having to preprocess, parse, assembly and link, before finally executing code.


### Noteworthy changes and fixes

- Inplace operators now have significantly different operation protocols than regular operators (`x += y;` is emulated as `x = x + y;` at runtime when no inplace operator exists)
	- As a result of this, strings and other immutable types will appear as though they can be used in inplace operations, when in reality they can't.
	- As a consequence of this, r-values (such as function return values) cannot be used in inplace operations
- The builtin [`int`](http://localhost:8080/modules/deemon/i:int) type is now immutable, meaning that use of integers no longer requires seemingly out of place `copy` statements when passing around integers, or having to create copies when loading them as constants.
- Classes now require the user to declare member variables (also: I actually implemented a syntax for super-initialization in constructors)
- Introduction of new symbol classes for extern (aka. imported) and global (aka. exported) objects
	- Global variables are created when defining a symbols without a `local` prefix in the global scope, or when explicitly prefixed with `global`
	- Global variables (symbols) can be modified by other modules or functions without the need of placing their values inside of a cell (as was, and is still required for local variables referenced in inner functions, or lambda expressions)
	- Symbol and module import works the same way it does in python, in addition to allowing you to write either `import symbol from module` or `from module import symbol`.
	- Additionally, anywhere a variable can appear, you can also write `foo from bar` which will reference a symbol `foo` from a module `bar` without you having to explicitly import that symbol beforehand.
- New style guidelines discourage the use of underscores in symbol names (e.g. it's [`seq.nonempty`](http://localhost:8080/modules/deemon/i:Sequence/i:nonempty) now, instead of [`seq.non_empty()`](http://localhost:8080/modules/deemon/i:Sequence/i:non_empty), which is deprecated)
- New style guidelines discourage the use of functions for state checks, or alternate representations (e.g. it's [`Thread.hasstarted`](http://localhost:8080/modules/deemon/i:Thread/i:hasstarted) now, instead of [`Thread.started()`](http://localhost:8080/modules/deemon/i:Thread/i:started), which is deprecated)
- Builtin types such as [`List`](http://localhost:8080/modules/deemon/i:List) or [`Dict`](http://localhost:8080/modules/deemon/i:Dict) must now be `import * from deemon;`-ed before they appear as symbols
- The builtin type [`Set`](http://localhost:8080/modules/deemon/i:Set) has been renamed to [`HashSet`](http://localhost:8080/modules/deemon/i:HashSet). [`Set from deemon`](http://localhost:8080/modules/deemon/i:Set) is now the base-class for set-like objects
	- Shouldn't really cause any problems in old code though, because deemon 100+'s `Set`-type has always been broken, and never got fixed
	- That said, the builtin `Set` object (now called [`HashSet`](http://localhost:8080/modules/deemon/i:HashSet)) actually works
- Not every object can be weakly referenced, and instead of a dedicated keyword `weak`, [`WeakRef from deemon`](http://localhost:8080/modules/deemon/i:WeakRef) is used to construct weak references
- Single-element [Tuples](http://localhost:8080/modules/deemon/i:Tuple) can now be constructed as `(foo,)`
- While deemon 100's compiler configuration handled pretty much any syntax problem as a warning, deemon 200 is default-configured to produce errors, thus preventing faulty code from accidentally being executed


### Noteworthy maintained features (that will stay)

- Inplace source formatting `deemon -F`
- `pack`-expressions to omit parenthesis (`foo pack 10, 20` is the same as `foo(10, 20)`)
- A fully featured C preprocessor (it's a highly advanced version of tpp, including all of its extensions)
- The `__nth` keyword being used to select secondary variable matches.


### Deprecated features (discouraged usage, but continued maintainance for now)

- `#include <...>` You really shouldn't be including files any more. - Use modules instead (they're way better)
- Various minor syntax changes to steer usercode to being more uniform (warned about in new code; ignored in legacy code)
- The dedicated syntax for cells (`<foo>` is deprecated and very much discouraged)
	- Use [`Cell from deemon`](http://localhost:8080/modules/deemon/i:Cell) instead.
	- Also note that with the introduction of global variables, cell indirection is no longer required in most cases


### Dropped features

- C-emulation of `struct`, `extern`, `union`, etc.
	- The runtime-aspect is still available through [ctypes](http://localhost:8080/modules/ctypes), however no longer has a dedicated syntax
	- Maintained C-like features that won't go away:
		- C-like casts `(int)x` (same as `int(x)`)
		- C-like variable declarations `int x = y;` (same as `local x = int(y);`)
		- Struct-initializers in mappings `Dict { .x = "foo" }` (same as `Dict { "x": "foo" }`)
- `alias`-symbol declarations no longer exist
- `const`-symbol declarations no longer exist (optimization automatically detects `local` / `global` variables written only once as constant, and you can use `final` to enforce write-once behavior)
- `operator ! ()` has been removed, and `!foo` invokes `operator bool()` and logically inverts its result (`!foo` is the same as `foo ? false : true`)
- The `operator move()` constructor has been removed, as well as the `move` keyword (emulated with sightly different semantics for legacy code; see below)
	- Note however that `operator move := ()` (move-assign) hasn't been removed
- Removed the logical XOR operator `^^` (just cast both operands to bool, then use the regular XOR)


### Dropped features that are emulated in legacy code

Legacy code being detected by it #including any of the old headers

- C-like syntax for attributes `__attribute__((attrib))`, `__declspec(attrib)`, `[[attrib]]`
	- Deemon 200 relies less than ever on attributes, and where they are still needed, tags are used `@[attrib]`
- The millions of `__builtin*` functions have all been removed
	- Most notable, even `__builtin_object()` is emulated
- The `move` keyword is defined as an alias for `copy`
- Removed the `weak` keyword
- Various keywords that all start with 2 underscores (`__static_if`, `__if_true`, etc.)
- The old notion of modules no longer exist (The `module` keyword was removed, and the `import` keyword now has a different meaning)
