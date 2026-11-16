# Deemon language syntax

At its core, Deemon's syntax is an amalgamation of C, C++, Python, Java and Javascript:

## Inherited Features

### C

- Preprocessor (`#define`, `#include`, ...), though use of it is discouraged (see **Module System**)
- Braces: `{ STATEMENTS; }`
- If-statement: `if (COND) TT; else FF;`, `if (COND) { TT; } else { FF; }`
- For-statement: `for (local i = 0; i < 10; ++i) { ... }`
- While-statement: `while (COND) { BODY; }`
- Do-while-statement: `do { BODY; } while (COND);`
- Loop-control: `break;`, `continue;`
- Switch-statement: `switch (EXPR) { case 1: BODY_1; /*fallthru*/ case 2: BODY_1_OR_2; break; default: BODY_DEFAULT; break; }`
- Goto: `label: STATEMENTS; goto label;`
- Strings: `"strings\nand C-style \\-escapes"`
- Integers: `10`, `0b1011`, `0xFF` (with arbitrary precision, like Python's `long` type)
- Floats: `123.5E-2`
- Return-statement: `return EXPR;`
- Static variables: `static local functionScopedStatic = INITIALIZER;` (static variables survive across multiple invocation of surrounding function, and their INITIALIZER is invoked only once)
- All standard C operators (see **Operators** below)
- Statements-in-expressions: `local x = ({ STMT; });`
- Assertions: `assert EXPR;`, `assert EXPR, MESSAGE;`, `assert(EXPR);`, `assert(EXPR, MESSAGE);`
	- Throws `errors.AssertionError` if `!EXPR`.
	- `MESSAGE` is optional and only evaluated on failure.


### C++

- `for (local ITEM: SEQUENCE) { BODY; }`
	- Same as `foreach (local ITEM: SEQUENCE.operator iter()) { BODY; }`
- `foreach (local ITEM: ITERATOR) { BODY; }`
- `for (local FIRST, SECOND: PAIRS) { BODY; }`
- Lamdas: `[](a: int, b: int) -> a + b`, `[](a: int, b: int) { return a + b; }` (But note that **Java**-style lambdas are preferred)
- `try { throw Error("Oh No!"); } catch (Error as e) { HANDLER; } catch (e...) { HANDLER; } finally { FINALLY; }`
	- Also possible in expressions: `local x = try int(INPUT) catch (ValueError from errors) -1;`
- Explicit operator invocation `X.operator ... (ARGS)`:
	- Useful: `X.operator . ("ATTR")` same as `X.ATTR`, but `"ATTR"` can be dynamic
- Operator lambdas `local AND = operator &;` (same as `local AND = (a, b) -> a & b;`)
- `true`/`false`-builtin (pre-defined, no need to import)

Classes:

```
class MyClass: BaseClass, Sequence {
private:
	member foo;
	private member bar = DEFAULT_INIT;
	public member baz: TypeAnnotation;
public:

	static final CONSTANT: int = 42;

	this(args...): super(args...) { }

	operator + (other: MyClass): MyClass {
		return MyClass(bar + other.bar);
	}

	function func(x: int, y: int): int {
		return super.otherFunc(x, y + this.foo);
	}

	property attrib: string = {
		get(): string {
			...
		}
		del() {
			...
		}
		set(v: string) {
			...
		}
	}
}
```

The `this`-constructor can also be defaulted (which also adds automatic `operator repr` and comparison):

```
class Point {
	public member x: int;
	public member y: int;
	this = default;
}

local p = Point(x: 10, y: 20);
print repr p; // "Point(x: 10, y: 20)"
```

### Python

- Optional type annotations (use of these is *highly* encouraged):
	- `local x: int;`
	- `local x: int | none = none;`
	- `local sequence: {string...};`
	- `local mapping: {int: string};`
- `none`-builtin (pre-defined, no need to import)
	- CAUTION: unlike Python, Deemon's `none` is a *universal no-op* (it implements everything by re-returning itself: `none.does.not("matter")` evaluates to `none`), rather than throwing an error like Python's `None` or Javascript's `null`/`undefined`
- With-statement: `with (local fp = File.open(...)) ...` (s.a. operators `enter` and `leave`)
- Yield functions (aka. Co-Routines): `function foo(): {ItemType...} { for (local x: seq) yield x; }`
- Sequences: `local sequence: {ItemType...} = { first, second }`
	- CAUTION: unlike Python (where this would be a `set`), this is an *abstract sequence* (some object derived from `deemon.Sequence`)
- Mapping: `local sequence: {KeyType: ItemType} = { KEY1: ITEM2, KEY2: ITEM2 }`
	- CAUTION: unlike Python (where this would be a `dict`), this is an *abstract mapping* (some object derived from `deemon.Mapping`; `deemon.Dict` is a modifiable sub-class of `deemon.Mapping`)
- Tuple: `local tuple: (FirstType, SecondType) = (first, second);`
	- same as `deemon.Tuple{ first, second }`
- List: `local list: {ItemType...} = [first, second]`
	- same as `deemon.List{ first, second }`
- Generators: `local x = for (local item: y) item + 2;`
	- same as `local x = (() -> { for (local item: y) yield item + 2; })();`
- Print-statement: (CAUTION: derived from python2's print statement; not a function)
	- `print EXPR1, EXPR2;` (prints: `f"{EXPR1} {EXPR2}\n"` -- note the `" "` between operands in the output)
	- `print(EXPR1, EXPR2);` (prints: `f"{EXPR1}{EXPR2}\n"`)
		- Add parenthesis to prevent injection of `" "` between expressions
	- `print(EXPR1, EXPR2),;` (prints: `f"{EXPR1}{EXPR2"`)
		- Add dangling `,` to suppress trailing newline.
	- `print FILE: EXPR1, EXPR2;`
		- Prefix `FILE:` to print to `FILE` instead of `deemon.File.stdout`
- Keyword arguments: `foo(start: 10, end: 20)` (CAUTION: uses `:` instead of Python's `=` to separate argument names and values)
	- Use `**` to expand a mapping into keyword arguments `foo(**{ "start": 10, "end": 20 })`. Same as `foo(start: 10, end: 20)`
	- Positional and keyword arguments can be mixed, but keywords must always come last
- Global/local split: `global final EXPORTED_FROM_MY_MODULE = 42;`. CAUTION: Deemon's compiler uses **static** symbol resolution, meaning:
	- Symbols are deleted at the end of their declaration scope
	- Symbols must be declared before they can be used
	- Symbols with identical names are OK and can shadow each other
	- There is no way to dynamically define globals at runtime like there is in Python
	- `function`s and `class`es declared in the global scope are `global` by default
	- Anything that is `global` can be `import`-ed by another module, or queried by-name at runtime as `EXPORTING_MODULE.operator . (EXPORT_NAME)` (s.a. **Module System**)
- Symbol-unbind: `del(EXPR);`: Unbind `EXPR`
- Symbol-delete: `del EXPR;`: Unbind `EXPR` and (if declared in current scope) delete symbol
- Template-strings: `f"foo = {FOO}"`
- Stringize expression: `str EXPR`, `str(EXPR)` (returns a human-readable representation of `EXPR`)
- Expression representation: `repr EXPR`, `repr(EXPR)` (return Deemon-code approximating how `EXPR` was constructed)
- `in`-expression: `ITEM in SEQUENCE` (same as `SEQUENCE.operator contains(ITEM)`)
	- Also allowed: `ITEM !in SEQUENCE` (to negate result)
- `import`-statement: `from MODULE import SYMBOL;`
	- Also allowed: `import SYMBOL from MODULE;` (preferred spelling)
	- `import * from MODULE;` (import all symbols from `MODULE`)
	- `import MODULE;` (bind `MODULE` as a local `MODULE`)
	- `import "MODULE" as ALIAS;` (bind `MODULE` as a local `ALIAS`)
	- `import "SYMBOL" as ALIAS from MODULE;`
	- `local importedModule = import(MODULE_NAME_EXPR);` (CAUTION: This import happens at runtime, whereas the other spellings happen during compilation)
	- See **Module System** for more info on what `MODULE` can be


### Java

- Block strings:
  ```
  local x = """
     this string can span
     multiple lines""";
  ```
- Compact lambdas:
	- `local func = () -> 42;`
	- `local func = (a, b) -> a + b;`
	- `local func = (a, b) -> { return a + b; };`
- `final` declarations (write-once)
	- Related: `varying` can be written to negate the effect of `final` where implicit

### Javascript

- Function declarations (preferred way of declaring named functions; implies `final`, unless qualified with `varying`):
  ```
  function foo(a: int, b: string = "DEFAULT", optional?: float): string {
  	return f"a: {a}, b: {b}, optional: {optional is bound ? optional : "<UNBOUND>"}";
  }
  ```
- `none`-substitution: `MAYBE_NONE ?? FALLBACK` (same as `MAYBE_NONE !is none ? MAYBE_NONE : FALLBACK`)
	- CAUTION: In Javascript, `undefined` is also substituted. Deemon doesn't have an `undefined` object, and (e.g.) trying to access unknown attributes raises `UnknownAttribute`. `OBJ.UNKNOWN_ATTR ?? FALLBACK` does **NOT** work. Use `(try OBJ.UNKNOWN_ATTR catch (UnknownAttribute) FALLBACK)` in this case.

### Deemon-specific language features

- Operators:
	- `EXPR...`: Simulate to a `,`-separated expansion of `EXPR` at the point of use:
		- Python equivalent would be `*EXPR`
		- `foo(10, ARGS..., 20)` calls foo with `2 + #ARGS` arguments `{ 10, ARGS[0], ARGS[1], ..., ARGS[#ARGS-1], 20 }`
		- `{ "before", ITEMS1..., "middle", ITEMS2..., "end" }`
		- Special case: `local first, none, third = ITEMS...;` unpack `ITEMS` into 3 objects and assign to variables. (same as Python `first, _, third = ITEMS`)
- Range generators: `[:END]`, `[START:END]`, `[START:END,STEP]`
	- `START` defaults to `0` (or rather: `(type END)()`)
	- `STEP` defaults to `1` (or rather: causes `+= STEP` to be used internally instead of `++`)
	- Same as `(for (local _i = START; _i < END; _i += STEP) _i)`
- copy-expression: `copy EXPR`, `copy(EXPR)`  (create a shallow copy of `EXPR`)
- deep-copy-expression: `deepcopy EXPR`, `deepcopy(EXPR)`  (create a deep copy of `EXPR`)
- same-object: `EXPR1 === EXPR2` (determine if `EXPR1` and `EXPR2` refer to the same object).
	- No operator is called, for reference you can think:
		- Deemon: `a == b` <=> Java: `a.equals(b)`
		- Deemon: `a === b` <=> Java: `a == b`
	- Also allowed: `EXPR1 !== EXPR2` (same as `!(EXPR1 === EXPR2)`)
- `is`-expression: `OBJECT is TYPE` (same as `type(OBJECT).extends(TYPE)`)
	- Also allowed: `OBJECT !is TYPE` (to negate result)
	- Special case when `TYPE === none`: `OBJECT is none` is (preferred) syntax sugar for `OBJECT === none`
- Lazy imports in expressions: `local x = SYMBOL from MODULE;` (same as `import SYMBOL from MODULE; local x = SYMBOL;`, but without also declaring `SYMBOL` locally)
- Bound-checks: `function foo(required, optional?) { return optional is bound ? "BOUND" : "UNBOUND"; }` (`foo(10, 20) == "BOUND"`, `foo(10) == "UNBOUND"`)
	- Also work for: local variables, attributes (`OBJ.ATTR is bound`) and sequence items / mapping keys (`SEQUENCE[INDEX] is bound`, `MAPPING[KEY] is bound`)
	- Syntax sugar for `deemon.boundattr`, `deemon.bounditem`.
- Super-casts: `EXPR as TYPE` (compose a `deemon.Super` object around `EXPR` that dispatches all interactions through `TYPE`)


## Operators

The following operators are defined. Every operator can be referenced by `operator SYMBOL` or `operator NAME`, though (where possible), use of dedicated syntax is preferred:

| Dedicated Syntax | Symbol | Name | Return Type | Notes |
| --- | --- | --- | --- | --- |
| `copy EXPR`, `copy(EXPR)` | | `copy` | Like `EXPR` | |
| `DST := VALUE` | `:=` | `assign` | `none` | Use is discouraged. This assigns value **to object**; do not confuse with `SYMBOL = VALUE` which stores value **in symbol** (and is not an operator) |
| | `move:=` | `moveassign` | `none` | |
| `str EXPR`, `str(EXPR)` | | `str` | `deemon.string` | Human-readable description of `EXPR` |
| `repr EXPR`, `repr(EXPR)` | | `repr` | `deemon.string` | Deemon-code approximating `EXPR` |
| `!!EXPR` | | `bool` | `deemon.bool` | Used to evaluate operands of `if`, `?`, `&&`, `||`, etc. |
| | | `next` | | Yield next item from `deemon.Iterator`; throws `StopIteration` if exhausted; Don't call directly, use `foreach` |
| `FUNC(ARGS...)` | `()` | `call` | | |
| | | `int` | `deemon.int` | Convert to integer |
| | | `float` | `deemon.float` | Convert to float |
| `~X` | `~` | `inv` | | |
| `+X` | `+` | `pos` | | |
| `-X` | `-` | `neg` | | |
| `X + Y` | `+` | `add` | | |
| `X - Y` | `-` | `sub` | | |
| `X * Y` | `*` | `mul` | | |
| `X / Y` | `/` | `div` | | |
| `X % Y` | `%` | `mod` | | |
| `X << Y` | `<<` | `shl` | | |
| `X >> Y` | `>>` | `shr` | | |
| `X & Y` | `&` | `and` | | |
| `X \| Y` | `\|` | `or` | | |
| `X ^ Y` | `^` | `xor` | | |
| `X ** Y` | `**` | `pow` | | |
| `++X` | `++` | `inc` | | |
| `--X` | `--` | `dec` | | |
| `X += Y` | `+=` | `iadd` | | |
| `X -= Y` | `-=` | `isub` | | |
| `X *= Y` | `*=` | `imul` | | |
| `X /= Y` | `/=` | `idiv` | | |
| `X %= Y` | `%=` | `imod` | | |
| `X <<= Y` | `<<=` | `ishl` | | |
| `X >>= Y` | `>>=` | `ishr` | | |
| `X &= Y` | `&=` | `iand` | | |
| `X \|= Y` | `\|=` | `ior` | | |
| `X ^= Y` | `^=` | `ixor` | | |
| `X **= Y` | `**=` | `ipow` | | |
| | | `hash` | `int` | |
| `X == Y` | `==` | `eq` | `deemon.bool` (not mandatory) | |
| `X != Y` | `!=` | `ne` | `deemon.bool` (not mandatory) | |
| `X < Y` | `<` | `lo` | `deemon.bool` (not mandatory) | |
| `X <= Y` | `<=` | `le` | `deemon.bool` (not mandatory) | |
| `X > Y` | `>` | `gr` | `deemon.bool` (not mandatory) | |
| `X >= Y` | `>=` | `ge` | `deemon.bool` (not mandatory) | |
| | | `iter` | `deemon.Iterator` (not mandatory) | |
| `#SEQUENCE` | `#` | `size` | `deemon.int` (not mandatory) | |
| `ITEM in SEQUENCE` (note reversed operands) | | `contains` | `deemon.bool` (not mandatory) | |
| `SEQUENCE[INDEX]`, `MAPPING[KEY]` | `[]` | `getitem` | | |
| `del SEQUENCE[INDEX];`, `del MAPPING[KEY];` | `del[]` | `delitem` | `none` | |
| `SEQUENCE[INDEX] = VALUE;`, `MAPPING[KEY] = VALUE;` | `[]=` | `setitem` | | |
| `SEQUENCE[START:END]` | `[:]` | `getrange` | | |
| `del SEQUENCE[START:END];` | `del[:]` | `delrange` | `none` | |
| `SEQUENCE[START:END] = VALUE;` | `[:]=` | `setrange` | | |
| `OBJ.ATTR` | `.` | `getattr` | | |
| `del OBJ.ATTR;` | `del.` | `delattr` | | |
| `OBJ.ATTR = VALUE;` | `.=` | `setattr` | | |
| `with (EXPR) { ... }` (on enter) | | `enter` | | |
| `with (EXPR) { ... }` (on exit) | | `leave` | | |

## Module System

Every `.dee` script is its own module. During instantiation, a module's root scope is executed as code, though it is expected that modules meant for import are written such that executing their root scope has no side-effects.

Scripts that are meant to behave different when imported (`import .otherModule;`), vs. executed directly (`deemon otherModule.dee`) can wrap any code that should only be compiled in the later case in `#ifdef __MAIN__` (similar to Python's `if __name__ == '__main__':`)

In import-statements, `MODULE` (the name) can either be quoted as a string (in which case an alias is required), or written directly: `import deemon;` (same as `import "deemon" as deemon;`).

Module names can take 4 different forms (any of which are also accepted when `import(MODULE_NAME_EXPR)` is used as a function):

1. `.`: A single `.` acts as a stand-in for the calling module
2. `MODULE`, `DIR.MODULE`, `DIR1.DIR2.MODULE`, ...: **Global module**. In this case, `.` is replaced with `/` and `.dee` (or `.dll` / `.so`) is appended. Then, the resulting filename is loaded from Deemon's standard library (the `/lib` folder of this git)
3. `.MODULE`, `.DIR.MODULE`, `.DIR1.DIR2.MODULE`, ...: **Relative module**. Here, the same transformation happens as with global modules, but the resulting file is search for relative to the directory containing the calling module
4. `something/containing/slashes.dee`: Anything that contains `/` (or `\` on windows) is treated as a filename. Relative filenames are resolved to the directory containing the calling module, but absolute filenames are also allowed. (This spelling method is discouraged as it is less portable than `.MODULE`)


## Important modules

For more information, run `deemon --help /MODULE`. For more information on individual exports run `deemon --help /MODULE/EXPORT` (or `deemon --help /MODULE/EXPORT/SUBATTR1/SUBATTR2`). For example, to get help on `deemon.string.find`, run `deemon --help /deemon/string/find`.

### deemon

Special, builtin module (does not exist as a file in `/lib`)

- Functions: `boundattr`, `bounditem`, `bounditem`, `compare`, `equals`, `hasattr`, `hash`, `hasitem`, `hasitem`, `exec`
- Types: `Attribute`, `Bytes`, `Callable`, `Cell`, `Dict`, `Error`, `File`, `Frame`, `Function`, `HashSet`, `InstanceMethod`, `Iterator`, `List`, `Mapping`, `Module`, `Numeric`, `Object`, `Property`, `Sequence`, `Set`, `Signal`, `Super`, `Thread`, `Traceback`, `Tuple`, `Type`, `WeakRef`, `WeakRefAble`, `bool`, `enumattr` (treat as function), `float`, `int`, `string`
- Singletons: `gc`

### errors

Aliases for all builtin sub-classes of `deemon.Error`. The following is a tree showing the error class hierarchy. All error types shown are exported by-name from `errors`:

- Signal
	- Interrupt
		- KeyboardInterrupt
		- ThreadExit
		- StopIteration
- Error
	- AttributeError
		- UnboundAttribute
		- UnknownAttribute
		- RestrictedAttribute
	- CompilerError
		- SyntaxError
		- SymbolError
	- ThreadCrash
	- NoMemory
	- RuntimeError
		- NotImplemented
		- AssertionError
		- UnboundLocal
		- StackOverflow
		- SegFault
		- IllegalInstruction
	- TypeError
		- CannotWeakReference
	- ValueError
		- ArithmeticError
			- IntegerOverflow
			- DivideByZero
			- NegativeShift
		- SequenceError
			- KeyError
				- IndexError
					- EmptySequence
				- UnboundItem
				- UnknownKey
				- ReadOnlyKey
			- UnpackError
			- ItemNotFound
				- RegexNotFound
		- UnicodeError
			- UnicodeDecodeError
			- UnicodeEncodeError
		- ReferenceError
			- EmptyWeakReference
		- BufferError
	- SystemError
		- UnsupportedAPI
		- FSError
			- FileAccessError
				- ReadOnlyFile
			- FileNotFound
				- NoDirectory
				- NoSymlink
			- FileExists
				- IsDirectory
				- DirectoryNotEmpty
			- FileClosed
			- CrossDeviceLink
			- BusyFile
	- AppExit

## Useful functions

- `Error.display()`: When inside of a `catch`-block, you can call `Error.display()` or `Error.display(reason: "Failed to do the thing")` to get Deemon to print your message, the error, and a traceback to `deemon.File.stderr`
