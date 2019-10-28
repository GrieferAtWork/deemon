# deemon - Deemon scripting language

Deemon, completely rewritten with a new focus on clean, intuitive and functional language design, while still maintaining a backwards compatibility rate high enough to allow for simple porting of existing code.
For more information on changes, fixes and improvements, see `/lib/LANGUAGE.txt`

Deemon is a C-like, interpreted, object-orient and exception-enabled scripting language, greatly inspired by python's runtime library, while sharing many syntax constructs with common languages such as C, java and javascript.

At its core, deemon is designed for sequence and string processing, being the inventor of the expand-expression (as seen in something like `x = [a...,42,b...];` which creates a new list consisting of the items from `a`, followed by `42`, then those from `b`), as well as including many language constructs useful in such situations, including `yield`-statements, lambda functions, and generator expression (such as `foo = for (local x: bar) x.strip();`, where `foo` is a sequence containing the elements of `bar` after thore were transformed with a call to a member function `strip`).

Especially following this rewrite, deemon is shining more than ever when it comes to string functionality, providing *regular expression* support, as well as support for *wild cards*, alongside fully featured *unicode* integration.

In other areas deemon continues to shine, being more expandable than ever with the introduction of a module-based approach to code dependencies, offering a library that comes preloaded with an `fs` module allowing for filesystem operations, or the builtin `File from deemon` type allowing for optionally buffered file or TTY I/O, across modules such as `time` for working with the gregorian calender, and `net`, which provides an object-orient model for sockets. But it doesn't just end there, as creating a new module is just as simple as putting together a small deemon script containing a couple of functions.

Deemon is a universally applicable language that has learned much from its past mistakes, shortcomings, as well as strengths, now allowing you to write highly efficient code, that is just as easy to read as it was to write.

Code examples can be found in */util/tut*



## Building
With visual studio  
	- Open `/.vs/deemon.sln`
	- Select one of the MSVC build configurations
	- CTRL+SHIFT+B

With configure  
```sh
./configure
make
```

Cross-compiling deemon  
```sh
./configure --cross-prefix=/ops/kos/binutils/i386-kos/bin/i686-kos-
make
```



### Major improvements
	- Introduction of a module-based dependency system that allows code reuse without relying on preprocessor functionality that really didn't fit a scripting language all too well.
	- With more emphasis on documentation, deemon now comes shipped with a documentation server accessing via web-browser
		- It should be of note that the documentation server, as well as documentation text processor is written entirely in deemon.
		- Links listed below require that you are running it locally.
	- A complete overhaul of the builtin [`string`](http://localhost:8080/modules/deemon/i:string)
		- Full unicode support all packed together into a single string type
		- Separation between raw data [`Bytes`](http://localhost:8080/modules/deemon/i:Bytes) and [`string`]((http://localhost:8080/modules/deemon/string))s, as well as functionality to decode/encode data and strings
		- Builtin support for regular expressions
		- Addition of miscellaneous functions such as [`indent()`](http://localhost:8080/modules/deemon/i:string/i:indent) or [`findmatch()`](http://localhost:8080/modules/deemon/i:string/i:findmatch) to help in situation where the old string type was struggling
		- Addition of case-insensitive variants of many functions, such as [`casefind()`](http://localhost:8080/modules/deemon/i:string/i:casefind)
	- Introduction of a common base class for any [`Sequence`-like type](http://localhost:8080/modules/deemon/i:Sequence)
		- Includes emulation of any kind of sequence operator, as well as a huge set of member functions, including [`find()`](http://localhost:8080/modules/deemon/i:Sequence/find), [`sum()`](http://localhost:8080/modules/deemon/i:Sequence/sum), [`operator add`](http://localhost:8080/modules/deemon/i:Sequence/op:add) or comparisons
		- Also introduced are common base classes for set-like, and mapping-like objects
	- Introduction of ASP (Abstract Sequence Proxy)-like objects allows for lazy computation in functions like [`string.split()`](http://localhost:8080/modules/deemon/i:string/i:split), distributing work load across usage and essentially making such functions O(1) when invoked
	- Introduction of default, optional, named and keyword arguments for user-functions
		- `function foo(a, b = 10,c ?)`
			- `foo("Hello")` Called as `foo(a: "Hello", b: 10, c: /unbound/)`
			- `foo("Hello", "World")` Called as `foo(a: "Hello", b: "World", c: /unbound/)`
			- `foo("Hello", c: "Universe")` Called as `foo(a: "Hello", b: 10, c: "Universe")`
	- Introduction of a same-object / different-object operators `===` and `!==`
	- Introduction of a new syntax for constructing a [super-view](http://localhost:8080/modules/deemon/i:Super) `foo as Sequence`
	- Introduction of a new syntax for checking if variables or attributes are bound `if (x is bound) print x;`
	- Introduction of `with`-statements, useful when dealing with files or synchronization primitives
		- To go alongside, 2 new operator `operator enter()` and `operator leave()` were introduced
	- Introduction of an interactive excution mode `deemon -i` where code is executed, and results are printed as it is typed by the user
	- Introduction of a `deepcopy` keyword and operator to go alongside the `copy` keyword
		- Also includes automatic tracking of recursive objects such as a list containing itself.
	- Added support for raw string literals `r"the following are 2 seperate characters: \n"`
	- Overhaul of exception handlers in user-code now introduces zero-effort exception and finally handlers (as opposed to some stack of active handlers)
	- Overhaul of user-classes now require member variables to also be declared, significantly improving runtime performance
	- Lazy compilation of module source files into pre-compiled file caches improves load time significantly
	- Extremely powerful peephole optimization of generated bytecode
	- The bytecode generated by deemon has grown so powerful that you can actually write code using it, or have it be printed back to you by a powerful, builtin disassembler
		- If you look at it, it really has more in common with that of a CISC architecture, featuring admirable text compression rates, while still executing assembly as fast as possible
	- Added compiler warnings for various questionable cases (including use of reserved keywords as symbol names)
	- I actually took the time to write a copy of the entire interpreter in i386 assembly (by hand), providing a significant performance boost on 32-bit Intel machines.
	- The builtin `int` type can have arbitrary precision now, allowing operations with a practically infinite number of digits (though I'm not claiming credit for the implementation; only for the integration and new design centered around it)
	- Introduction of type annotations
		- `function add(x: int, y: int): int`
		- Mainly intended for simplified and less redundant documentation, but may also be used for other purposes
	- Addition of runtime functionality to execute strings as code
		- `print (exec from deemon)("10 + 20");`
		- Uses a seperate JIT compiler that directly executes source text, rather than having to preprocess, parse, assembly and link, before finally executing code.

### Noteworthy changes and fixes
	- Inplace operators now have significantly different operation protocols than regular operators (`x += y;` is emulated as `x = x + y;` at runtime when no inplace operator exists)
		- As a result of this, strings and other immutable types will appear as though they can be used in inplace operations, when in reality they can't.
		- As a consequence of this, r-values (such as function return values) cannot be used in inplace operations
	- The builtin [`int`](http://localhost:8080/modules/deemon/i:int) type is now a singleton, meaning that use of integers no longer requires seemingly out of place `copy` statements when passing around integers, or having to create copies when loading them as constants.
	- Classes now require the user to declare member variables (also: I actually implemented a syntax for super-initialization in constructors)
	- Introduction of new symbol classes for extern (aka. imported) and global (aka. exported) objects
		- Global variables are created when defining a symbols without a `local` prefix in the global scope, or when explicitly prefixed with `global`
		- Global variables (symbols) can be modified by other modules or functions without the need of placing their values inside of a cell (as was, and is still required for local variables referenced in inner functions, or lambda expressions)
		- Symbol and module import works the same way it does in python, with the additional that you are free to write either `import symbol from module` or `from module import symbol`.
		- Additionally, anywhere a variable can appear, you can also write `foo from bar` which will reference a symbol `foo` from a module `bar` without you having to explicitly import that symbol beforehand.
	- New style guidelines discourage the use of underscores in symbol names (e.g. it's [`seq.nonempty()`](http://localhost:8080/modules/deemon/i:Sequence/i:nonempty) now, instead of [`seq.non_empty()`](http://localhost:8080/modules/deemon/i:Sequence/i:non_empty), which is deprecated)
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
	- `const`-symbol declarations no longer exist (optimization automatically detects `local` variables written only once as constant)
		- You may alternatively declare variables as `final` to prevent them from being re-assigned
	- `operator ! ()` has been removed, and `!foo` invokes `operator bool()` and logically inverts its result
	- The `operator move()` constructor has been removed, as well as the `move` keyword
		- Note however that `operator move := ()` (move-assign) hasn't been removed
	- Removed the logical XOR operator `^^` (just cast both operands to bool, then use the regular XOR)

### Dropped features that are emulated in legacy code
Legacy code being detected by it #including any of the old headers
	- C-like syntax for attributes `__attribute__((attrib))`, `__declspec(attrib)`, `[[attrib]]`
		- Deemon 200 relies less than ever on attributes, and where they are useful, tags are used `@attrib`
	- The millions of `__builtin*` functions have all been removed
		- Most notable, even `__builtin_object()` is emulated
	- The `move` keyword is defined as an alias for `copy`
	- Removed the `weak` keyword
	- Various keywords that all start with 2 underscores (`__static_if`, `__if_true`, etc.)
	- The old notion of modules no longer exist (The `module` keyword was removed, and the `import` keyword's now has a different meaning)
