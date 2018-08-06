# deemon - Deemon scripting language

Deemon, completely rewritten with a new focus on clean, intuitive and functional language design, while still maintaining a backwards compatibility rate high enough to allow for simple porting of existing code.
For more information on changes, fixes and improvements, see /lib/LANGUAGE.txt

Deemon is a C-like, interpreted, object-orient and exception-enabled scripting language, greatly inspired by python's runtime library, while sharing many syntax constructs with common languages such as C, java and javascript.

At its core, deemon is designed for sequence and string processing, being the inventor of the expand-expression (as seen in something like <code>x = [a...,42,b...];</code> which creates a new list consisting of the items from <code>a</code>, followed by <code>42</code>, then those from <code>b</code>), as well as including many language constructs useful in such situations, including <code>yield</code>-statements, and generator expression (such as <code>foo = for (local x: bar) x.strip();</code>, where <code>foo</code> is a sequence containing the elements of <code>bar</code> after thore were transformed with a call to a member function <code>strip</code>), or lambda functions.

Especially in this rewrite, deemon is shining more than ever when it comes to string functionality, providing <b>regular expression</b> support, as well as support for <b>wild cards</b>, alongside fully featured <b>unicode</b> support.

In other area, deemon continues to shine, being more expandable than ever with the introduction of a module-based library that comes preloaded with an <code>fs</code> module allowing for filesystem operations, or the builtin <code>file from deemon</code> type allowing for optionally buffered file or TTY I/O, across modules such as <code>time</code> for working with the gregorian calender, and the <code>net</code> providing an object-orient model for sockets.

Deemon is truely a universally useful language that has learned from its past mistakes and shortcomings, allowing you to write highly efficient code, that will be just as easy to read as it was to write.

### Major improvements
  - Introduction of a module-based dependency system that allows code reuse without relying on preprocessor functionality that really didn't fit a scripting language all too well.
  - With more emphasis on documentation, deemon now comes shipped with a documentation server accessing via web-browser
    - It should be of note that the documentation server, as well as documentation text processor is written entirely in deemon.
    - Links listed below require that you are running it locally.
  - A complete overhaul of the builtin <code>string</code> (http://localhost:8080/modules/deemon/string)
    - Full unicode support all packed together into a single string type
    - Separation between raw data (http://localhost:8080/modules/deemon/bytes) and strings, as well as functionality to decode/encode data and strings
    - Builtin support for regular expressions
    - Addition of miscellaneous functions such as <code>indent()</code> or <code>findmatch()</code> to help in situation where the old string type was struggling
    - Addition of case-insensitive variants of many functions, such as <code>casefind()</code>
  - Introduction of a common base class for any sequence-like type http://localhost:8080/modules/deemon/sequence
    - Includes emulation of any kind of sequence operator, as well as a huge set of member functions, including <code>find()</code>, <code>sum()</code>, <code>operator add</code> or comparisons
    - Also introduced are common base classes for set-like, and mapping-like objects
  - Introduction of ASP (Abstract Sequence Proxy)-like objects allows for lazy computation in functions like <code>string.split()</code>, distributing work load across usage and essentially making such functions O(1) when invoked
  - Introduction of default, optional and named arguments for user-functions
    - <code>function foo(a,b = 10,c?)</code>
    - <code>foo(42, c: 7); // foo(42,10,7);</code>
  - Introduction of a same-object / different-object operators <code>===</code> and <code>!==</code>
  - Introduction of a new syntax for constructing a super-view <code>foo as sequence</code>
  - Introduction of a new syntax for checking if variables or attributes are bound <code>if (x is bound) print x;</code>
  - Introduction of <code>with</code>-statements, useful when dealing with files or synchronization primitives
    - To go alongside, 2 new operator <code>operator enter</code> and <code>operator leave</code> were introduced
  - Introduction of an interactive excution mode <code>deemon -i</code> where code is executed, and results are printed as it is typed by the user
  - Introduction of a <code>deepcopy</code> keyword and operator to go alongside the <code>copy</code> keyword
    - Also includes automatic tracking of recursive objects such as a list containing itself.
  - Added support for raw string literals <code>r"the following are 2 characters: \n"</code>
  - Overhaul of exception handlers in user-code now introduces zero-effort exception and finally handlers (as opposed to some stack of active handlers)
  - Overhaul of user-classes now require member variables to also be declared, significantly improving runtime performance
  - Lazy compilation of module source files into pre-compiled file caches improves load time significantly
  - Extremely powerful peephole optimization of generated bytecode
  - The bytecode generated by deemon has grown so powerful that you can actually write code using it, or have it be printed back to you by a powerful, builtin disassembler
    - If you look at it, it really has more in common with that of a CISC architecture, featuring admirable text compression rates, while still executing assembly as fast as possible
  - Added compiler warnings for various questionable cases (including use of reserved keywords as symbol names)
  - I actually took the time to write a copy of the entire interpreter in i386 assembly (by hand), providing a significant performance boost on 32-bit Intel machines.
  - The builtin <code>int</code> type can have arbitrary precision now, allowing operations with a practically infinite number of digits (though I'm not claiming credit for the implementation; only for the integration and new design centered around it)

### Noteworthy changes and fixes
  - Inplace operators have significantly changed operation protocols than regular operators (<code>x += y;</code> is emulated as <code>x = x + y;</code> at runtime when no inplace operator exists)
    - As a result of this, strings and other immutable types will appear as though they can be used in inplace operations, when in actuality they can't.
  - Classes now require the user to declare member variables (also: I actually implemented a syntax for super-initialization in constructors)
  - Introduction of new symbol classes for extern (aka. imported) and global (aka. exported) objects
    - Global variables are created when defining a symbols without a <code>local</code> prefix in the global scope, or when explicitly prefixed with <code>global</code>
    - Global variables (symbols) can be modified by other modules or functions without the need of placing their values inside of a cell (as was, and is still required for local variables referenced in inner functions, aka. lambda expressions)
    - Symbol and module import works the same way it does in python, with the additional that you are free to either write <code>import symbol from module</code> or <code>from module import symbol</code>.
    - Additionally, anywhere a variable can appear, you can also write <code>foo from bar</code> which will reference a symbol <code>foo</code> from a module <code>bar</code> without you having to explicitly import that symbol beforehand.
  - The style guidelines now discourage the use of underscores in symbol names (e.g. it's <code>seq.nonempty()</code> now, instead of <code>seq.non_empty()</code>, which is deprecated)
  - Builtin types such as <code>list</code> or <code>dict</code> must now be <code>import * from deemon;</code>-ed before they appear as symbols
  - The builtin type <code>set</code> has been renamed to <code>hashset</code>. <code>set from deemon</code> is now the base-class for set-like objects
    - Shouldn't really cause any problems in old code though, because deemon 100+'s <code>set</code>-type has always been broken, and never got fixed
  - The builtin <code>set</code> object (now called <code>hashset</code>) actually works
  - Not every object can be weakly referenced now, and instead of a dedicated keyword <code>weak</code>, <code>weakref from deemon</code> is used to construct weak references
  - Single-element tuples can now be constructed as <code>(foo,)</code>
  - While deemon 100's compiler configuration handled pretty much any syntax problem with a warning, deemon 200 is default-configured to produce errors, thus preventing faulty code from accidentally being executed

### Noteworthy maintained features (that will stay)
  - Inplace source formatting <code>deemon -F</code>
  - <code>pack</code>-expressions to omit parenthesis (<code>foo pack 10,20</code> is the same as <code>foo(10,20)</code>)
  - A fully featured C preprocessor (it's a highly advanced version of tpp, including all of its extensions)
  - The <code>\_\_nth</code> keyword being used to select secondary variable matches.

### Deprecated features (discouraged usage, but continued maintainance)
  - <code>#include \<...\></code> You really shouldn't be including files any more. - Use modules instead (they're way better)
  - Various minor syntax changes to steer usercode to being more uniform (warned about in new code; ignored in legacy code)
  - The dedicated syntax for cells (<code>\<foo\></code> is deprecated and not encouraged)
    - Use <code>cell from deemon</code> instead.

### Dropped features
  - C-emulation of <code>struct</code>, <code>extern</code>, <code>union</code>, etc.
    - The runtime-aspect is still available through ctypes (http://localhost:8080/modules/ctypes), however don't have a dedicated syntax any more
    - Maintained C-like features that won't go away:
      - C-like casts <code>(int)x</code> (same as <code>int(x)</code>)
      - C-like variable declarations <code>int x = y;</code> (same as <code>local x = int(y);</code>)
  - <code>alias</code>-symbol declarations no longer exist
  - <code>const</code>-symbol declarations no longer exist (optimization automatically detects <code>local</code> variables written only once as constant)
  - <code>operator !</code> has been removed, and <code>!foo</code> invokes <code>operator bool</code> and logically inverts its result
  - The <code>operator move()</code> constructor has been removed, as well as the <code>move</code> keyword.
  - Removed the logical XOR operator <code>^^</code> (just cast both operands to bool, then use the regular XOR)

### Dropped features that are emulated in legacy code
Legacy code being detected by it #including any of the old headers
  - C-like syntax for attributes <code>\_\_attribute\_\_((attrib))</code>, <code>\_\_declspec(attrib)</code>, <code>[[attrib]]</code>
    - Deemon 200 relies less than ever on attributes, and where they are useful, tags are used <code>@attrib</code>
    - Note that tags will likely undergo their own overhaul in the near future in order to user-code to define its own tags
  - The millions of <code>\_\_builtin*</code> functions have all been removed
    - Most notable, even <code>\_\_builtin\_object()</code> is emulated
  - Removed the <code>weak</code> keyword
  - Various keywords that all start with 2 underscores (<code>__static_if</code>, <code>__if_true</code>, etc.)
  - The old notion of modules no longer exist (The <code>module</code> keyword was removed, and the <code>import</code> keyword's meaning has an entirely new meaning)



## Building
Without deemon already installed
$ bash make.sh
With deemon already installed
$ deemon magic.dee
Cross-compiling deemon
$ export CROSS_PREFIX="/bin/i686-w64-mingw32-"
$ bash make.sh

## Known problems
While the core compiles fine under linux, dex modules don't. I havn't yet figured out how to get an ELF shared library to link against a statically linked executable's dynamic symbol table. Deemon 100 didn't run into this problem because it had the core be a shared library, too, which I'm not going to do because dynamically relocating the core at runtime would be \_way\_ too expensive due to the enourmous number of pointers between static structures.

On Windows everything is pre-linked at a fixed address, and linking against executables is just as simple as linking against libraries, and I had no problems getting it to work.




