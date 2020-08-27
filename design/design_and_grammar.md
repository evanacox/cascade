# Cascade Grammar
Note the regex syntax. Also, between any of the `keywords` or *definitions*, whitespace is allowed. Cascade ignores whitespace, besides when initially reading identifiers. 

## Comments
Comments block out part of a file, and any text that is inside a comment is "thrown out" by the initial lexing phase. 

```
-- this is a comment
```

*comment* := *single_line_comment* | *multi_line_comment*

Cascade single-line comments are created with `--`. They last until the end of a line.

*single_line_comment* := `--` .* (`\n` | `\r\n`)

Multi-line comments are created with `-*`, and last until another `*-`. They **cannot** be nested.

*multi_line_comment* := `-*` .* `*-`

## Literals
Literals represent the direct value of an expression, instead of representing the steps to get 
said value. The value is also known at compile-time, so for the most part they are evaluated then. 

```
let char_literal = 'a';
let string_literal = "This is an example";
let number_literal = 48290342;
let bool_literal = true;
let other_bool_lit = false;
```

- *literal* := *number_literal* | *string_literal* | *char_literal* | *bool_literal*


Number literals are currently only decimal literals, no hex/octal/binary support yet.

- *number_literal* := [`0`-`9`]+

Strings are UTF-8 encoded, and thus the string literals are assumed to be UTF-8. 

- *string_literal* := `"`.*`"`

Character literals are assumed to be one byte wide, equal to one UTF-8 code point. Any character that 
doesn't fit in a single code point needs to be represented as a String. 

- *char_literal* := `'`.`'`

Boolean literals are `true` or `false` values. Simple as that.

- *bool_literal* := `true` | `false`

## Identifiers
**All identifiers are ASCII**. Non-ASCII identifiers cause readability problems, and are a source of 
potential issues in the compiler.

Module names must begin with a lowercase alphabetical character, and contain alphanumeric characters 
and underscores. They are preferred to be `lower_snake_case`.

- *module_name* := [`a`-`z`] [`a`-`z` `A`-`Z` `0`-`9` `_`]

Type names must be uppercase in the beginning, e.g `String` instead of `string` or `Complex` instead 
of `complex`. Underscores and numbers are valid in the name, but they must **not** be the first 
character of the name. The first character must be an uppercase alphabetical character. They are 
preferred to be in `PascalCase` (also known as `UpperCamelCase`).

- *type_name* := [`A`-`Z`] [`a`-`z` `A`-`Z` `0`-`9` `_`]*

Function names begin with an underscore, or a lowercase alphabetical character. The other characters 
must be alphanumeric, or underscores. They are preferred to be in `lower_snake_case`.

- *fn_name* := [`a`-`z` `_`] [`a`-`z` `A`-`Z` `0`-`9` `_`]*

Variable names follow the same pattern. The first character must be an underscore or alphabetical 
character, and the rest can be alphanumeric and with underscores. Preferred naming schem is 
`lower_snake_case`.
- *var_name* := [`a`-`z` `A`-`Z` `0`-`9` `_`]*

## Types
Cascade is statically typed and not very implicit, and thus requires a way to write out its types when they aren't deducted. 

```
let byte: i8 = 5;
let byte_array: []i8 = [3, 2, 1, 0];
let point3d: Point3D = { x: 5.5368, y: 24.3333333, z: 23830.14322 };
let ref_to_point3d: &Point3D = &other;
let ptr_to_byte: *i8 = @byte;
let ptr_to_ptr_to_byte: **i8 = @ptr;
let ref_to_ptr_to_array_of_ptrs_to_i32s: &*[]*i32 = ...;
```

Types can be a reference or a standard type. References are denoted with a `&`, and must come at the beginning (as you can't have a pointer to a reference, for instance). Pointers and array come afterwards (as you can have a reference to a pointer or array), then the actual type name.
- *type* := *reference*? *pointer_or_array** (*builtin_type* | *type_name*)
- *reference* := `&` | `&` `mut`
- *pointer_or_array* := `*` `mut` | `*` | `[]`
- *builtin_type* := `bool` | *builtin_signed* | *builtin_unsigned* | *builtin_fp*
- *builtin_signed* := `i8` | `i16` | `i32` | `i64` | `i128` | `isize`
- *builtin_unsigned* := `u8` | `u16` | `u32` | `u64` | `u128` | `usize`
- *builtin_fp* := `f32` | `f64`

#### References
References are very similar to the C++ feature of the same name. They are effectively pointers that are guaranteed to be valid pointers at creation, and they cannot be reassigned.

#### Pointers
Just your standard pointers, used for lower-level code and for interacting with C code.

#### Arrays
Arrays in Cascade are slightly easier to deal with than their counterparts in C, as they keep track of their own length. However, they are still effectively C arrays, and can be used in FFI as such. 

## Declarations
Declarations declare a "thing" that is accessible throughout the current scope. This could 
be a constant, a variable, a function, a type, or anything else.

#### `const` Declaration
`const` declarations are compile-time constant expressions assigned to a value. The closest 
approximation is `constexpr` in C++. Preferred naming scheme is `SCREAMING_SNAKE_CASE`. 

```++
const SOME_MAGIC_CONSTANT = 320;
const OTHER_MAGIC_CONSTANT: u8 = 30;
```
- *const_decl* := `const` *var_name* (`:` *type*)? `=` *expression* `;`

#### `static` Declaration
`static` declarations are effectively just global variables. They are initialized at the 
program's startup time, and destroyed at program termination.

```++
static thing = 32;
```

- *static_decl* := `static` *var_name* (`:` *type*)? `=` *expression* `;`

#### `fn` Declaration 
- *fn_decl* := `fn` *fn_name* `(` *fn_argument_list* `)` *type* *block_expr*

`fn` declarations define a function. They contain a name, a list of arguments (and their types), 
a return type, and a body.

```
fn fibonacci(n: i32) i64 {
  match n {
    case 0, 1, 2 -> n,
    case _ -> fibonacci(n - 1) + fibonacci(n - 2)
  }
}
```

- *fn_argument* := *var_name* `:` *type*
- *fn_argument_list* := *fn_argument* | *fn_argument* `,` *fn_argument_list*

#### `struct` Declaration
- *struct_decl* := `struct` *type_name* (`:` *struct_inherit_list*) ?`{` (*struct_field* | *struct_method*)* `}`
- *struct_inherit_list* := *type_name* | *type_name* `,` *struct_inherit_list*
- *struct_field_visibility* := `pub` | `internal`
- *struct_field* := *struct_field_visibility*? *var_name* `:` *type* `;`
- *struct_method_name* := ((`@`(`create`|`destroy`|`move`|`clone`)) | (*fn_name*))
- *struct_method_arg_list* := (`self`)? (`,` *fn_argument_list*)?
- *struct_method* := *struct_field_visibility*? `fn` *struct_method_name* `(` *struct_method_arg_list* `)` *block_expr*

`struct` declarations are the Cascade equivalent of a class. Each field has a name and a type, and an optional `pub` visibility 
modifier to allow a field to be accessed. **Note: If every field is not marked `pub` and there are no methods, every field is implicitly `pub`.**

Structs have four special methods, called `@create`, `@destroy`, `@move` and `@clone`. These functions are called on construction,
destruction, moving and cloning respectively. 

Methods are either freestanding member functions (no `&self` / `self` argument), or they are normal member functions (have a `&self` argument).
`&self` is a special argument that is automatically passed, and acts effectively for type `T` as a `&T` pointing to the object
they were called on. Objects can also take in `&mut self`, to get a `&mut T`. Finally, methods can take ownership of the instance they're operating on
with `self`. 

```
struct Point {
    pub x: f64;
    pub y: f64;

    pub fn @create(self, x: f64, y: f64) {
        self.x = x;
        self.y = y;
    }
}

struct Point3D extends Point {
    -- self.super is now a Point instance
    pub z: f64;

    -- "static" function, notice how it doesn't have
    -- a `self` argument
    pub fn from(other: Point3D) Point3D {
        Point3D(other.x, other.y, other.z);
    }
    
    pub fn @create(&mut self, x: f64, y: f64, z: f64) {
        -- if super is run at all, it **must** be run first
        self.super(x, y);

        self.z = z;
    }
}
```

##### `@` Methods
All methods starting with an `@` are called automatically by the compiler, and cannot be explicitly invoked. All of them
can optionally have `pub` switched for `internal` (or outright removed). This could be useful if you wanted to make a class only usable as a derived instance, or if you wanted to make an object unclonable. Note: This can make it impossible to use
an object if you, say, hid `@destroy`.

##### `@create` Methods
`@create` is effectively a main constructor for a type. It takes in a `mut self` argument, and any arguments the user declares. There
can only be one, `@create` cannot be overloaded.

It is called by the compiler automatically whenever an object is constructed outside of a struct expression. Before control is given
to the user-defined `@create`, `@create` has been called on no fields, and all variables are uninitialized. Not initializing all fields
is undefined behaviour.

```
struct Point {
    pub fn @create(&mut self, x: f64, y: f64) {}
}

let pt = Point(3.0, 2.0); -- @create called here
```

Moving, cloning, and copying builtins are all allowed inside `@create`. 

It can be implicitly generated if none is given:
```
pub fn @create(&self) {}
```

##### `@destroy` Methods
`@destroy` is the destructor for an object. 

It is called by the compiler automatically whenever an object goes out of scope.

It can be implicitly generated if none is given, and is the equivalent of:
```
pub fn @destroy(&self) {}
```

All fields have their `@destroy` called after control returns from the structure's `@destroy`.

```
struct Point {
    -- ...
    pub fn @destroy()
}

{
    let pt = Point(3.0, 2.0);

    -- @destroy called here
}
```

`@destroy` **must not** have any arguments besides `&mut self` or `&self`. It is allowed to modify any fields, and move any fields.

##### `@move` Methods
`@move` is the "move constructor" of an object.

It has the following signature, assuming type `T`:
```
fn @move(&mut self, mut other: T) {}
```

It can be default generated, is approximately this in pseudocode:
```
pub fn @move(&mut self, mut other: T) {
    for field in other {
        self.field = other.field;
    }
}
```
It is a simple memberwise move of every field.


It is allowed to move any 

#### `interface` Declarations and `extension` Blocks
**TO BE DECIDED**

## Statements
Statements are side-effects in the program. They simply make something happen.

- *statement* := *let_stmt*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *mut_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *assert_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *loop_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *break_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *continue_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *ret_stmt* <br />
&nbsp;&nbsp;&nbsp;&nbsp;| *expression_stmt* <br />

#### `let` Statement
`let` statements are local, immutable variable declarations.

```
let x = 5;
```

- *let_stmt* := `let` *var_name* (`:` *type*)? `=` *expression* `;`

#### `mut` Statement
`mut` statements are exactly the same as `let` semantically, besides one key point. They are mutable.

```
mut x = 6;
x = 3;
```

- *mut_stmt* := `mut` *var_name* (`:` *type*)? `=` *expression* `;`

#### Loop Statements
Loop statements are the looping construct. They have several forms.

- *loop_stmt* := *loop_unconditional* | *loop_while* | *loop_in*

The first type of loops are unconditional loops. They continue until they a `break` statement 
is executed, at which case they exit and execution continues.

- *loop_unconditional* := `loop` *block_expr*

```
mut has_failed = false;

loop {
    has_failed = returns_true_if_fails();

    if has_failed {
        break;
    }
}
```

The second type are conditional loops, also known as `while` loops. 

- *loop_while* := `while` *expression* *block_expr*

```
mut done = false;

while !done {
    done = keep_trying();
}
```

The final type is the `for..in` loop. They act on ranges and other "iterable" types.

```
let array = [1, 2, 3, 4, 5];

for i in 1..5 {
    array[i] *= 2;
}

for i in array {
    io::println(i);
}
```

- *loop_in* := `for` *var_name* `in` *expression* *block_expr*

#### `break` Statement
The break statement was previously shown inside a `loop`, and the usage
is the same as in other languages. It "breaks out" of a loop.

```
loop {
    if cond {
        break;
    }
}

io::println("Broke out of the loop!");
```

- *break_stmt* := `break` `;`

#### `continue` Statement
Again, `continue` in Cascade is the same as in other languages. It immediately jumps to the beginning of the loop.

- *continue_stmt* := `continue` `;`

```
for x in list {
    if x < 5 {
        continue;
    }

    do_some_calculation(x);
}
```

#### `ret` Statement
`ret` is effectively a `return`. It's just shorter to type. When they are reached, 
they immediately end execution of the current function and return the value
that the expression being returned evaluates to.

- *ret_stmt* := `ret` *expression*? `;`

```
ret fib(n - 1) + fib(n - 2);
```

#### `assert` Statement
`assert` is a statement used to "assert" things during runtime. In un-optimized builds the 
condition of the `assert` is checked, and if it's false, the program panics with some debug
information.

- *assert_stmt* := `assert` *expression* `;`

#### Expression Statement
Expression statements are simply expressions that have their return value discarded. Expressions with blocks (`{}`) do not have
semicolons.

```
vector.push(5);

if true {
    -- do something
}
```

- *expression_stmt* := (*expression_without_block* `;`) | (*expression_with_block*)

## Modules
Modules are the way that Cascade splits up functions and types. They are Cascade's method of namespacing 
symbols and modularizing a program.

```
module example_api;

import std::core::String;

import TypeOne, TypeTwo from some::lib;

import std::core::Vector as Vec;

export import example_api::submodule;

export const MAGIC_CONSTANT = 5234890;

export fn user_exists(user: String) bool {}
```

#### `module` declaration

- *module_decl* := `module` *module_path* `;`

Modules are declared before any other declarations. They declare the entire module name, due to
the fact that Cascade doesn't 

Modules can be nested, therefore module and import declarations need to account for this. They can
be nested like so: `foo::bar::a::b::c`. 

- *module_path* := *module_name* | *module_name* `::` *module_path*

#### `import` declaration

- *import_decl* := *import_single* | *import_multiple*

Import declarations with `import` can import a module path, and can optionally alias it with `as`.

- *import_single* := `import` *module_path* (`as` *var_name*)? `;`

`from` can be used to import multiple items from a module. 

- *import_multiple_items* := *var_name* | *var_name* `,` *import_multiple_items*
- *import_multiple* := `import` *import_multiple_items* `from` *module_path* `;` 

#### `export` Modifier
Marks an item as "exported" from the module. Can be applied to any declaration (besides `module`).

- *export_modifier* := `export` (*import_decl* | *const_decl* | *static_decl* | *fn_decl* | *struct_decl*)

## Expressions
Expressions are the main way work gets done in Cascade. They are the building block of most of the language, 
and are intentionally made to be very flexible.

- *expression* := *expression_without_block* | *expression_with_block*

- *expression_without_block* := *literal_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *identifier_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *call_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *binary_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *unary_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *field_access_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *index_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *array_expression*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *method_call_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *lambda_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *struct_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *if_then_expr*<br />

- *expression_with_block* := *block_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *if_expr*<br />
&nbsp;&nbsp;&nbsp;&nbsp;| *match_expr*<br />

#### Precedence

<!-- This was generated, not hand-written -->
| Operator                                                                 	| Description                                                                                                                    	| Associativity 	|
|--------------------------------------------------------------------------	|--------------------------------------------------------------------------------------------------------------------------------	|---------------	|
| ( *expr* )<br>{ *exprs* }                                                	| Grouping                                                                                                                       	|               	|
| *type*(*args*)<br>*expr*()<br>*expr*[]<br>a.*name*<br>a.*method*(*args*) 	| Constructors<br>Function Calls<br>Index Access<br>Field Access<br>Method Call                                                  	| Left-to-right 	|
| ~a<br>*a<br>&a<br>@a<br>+a -a<br>clone a                                 	| Logical and Bitwise NOT<br>Dereference<br>Get Reference<br>Address-of<br>Unary Plus / Minus<br>Clone                           	| Right-to-left 	|
| a * b<br>a / b<br>a % b                                                  	| Multiplication<br>Division<br>Remainder                                                                                        	| Left-to-right 	|
| a + b<br>a - b                                                           	| Addition<br>Subtraction                                                                                                        	| Left-to-right 	|
| <<<br>>>                                                                 	| Bitwise Left Shift<br>Bitwise Right Shift                                                                                      	| Left-to-right 	|
| &                                                                        	| Bitwise AND                                                                                                                    	| Left-to-right 	|
| ^                                                                        	| Bitwise XOR                                                                                                                    	| Left-to-right 	|
| \|                                                                       	| Bitwise OR                                                                                                                     	| Left-to-right 	|
| < <=<br>> >=<br>== !=                                                    	| Less-than, Less-than-or-equal<br>Greater-than, Greater-than-or-equal<br>Equals, does not equal                                 	| Left-to-right 	|
| not                                                                      	| Logical NOT                                                                                                                    	| Right-to-left 	|
| and                                                                      	| Logical AND                                                                                                                    	| Left-to-right 	|
| xor                                                                      	| Logical XOR                                                                                                                    	| Left-to-right 	|
| or                                                                       	| Logical OR                                                                                                                     	| Left-to-right 	|
| if<br>match                                                              	| If expressions<br>Match expressions                                                                                            	|               	|
| =<br>+= -=<br>*= /= %=<br><<= >>=<br>&= \|= ^=                           	| Assignment<br>Compound assignment (+-)<br>Compound assignment (*/%)<br>Compound assignment (<<>>)<br>Compound Assignment(&\|^) 	| Right-to-left 	|
#### Literal Expression
Simply a literal of some sort.

- *literal_expr* := *literal*

#### Identifier
Simply an identifier that could refer to a value, e.g `result` or `x`

- *identifier_expr* := *var_name*

#### Call Expression
A call to a function (or a functor object)

- *call_arguments* := *expression* (`,` *expression*)*
- *call_expr* := *expression* `(` *call_arguments* `)`

#### Binary Expression
An expression with two terms 

```
let result = x | y;
```

- *binary_op* := `|` | `&` | `^` | `and` | `or` | `xor` | `=` 
| `+=` | `-=` | `*=` | `/=` | `%=` | `<<=` | `>>=` | `&=` 
| `|=` | `^=` | `+` | `-` | `*` | `/` | `%` | `<<` | `>>` | `==` | `!=` | `<` | `<=` | `>` | `>=`

- *binary_expr* := *expression* *binary_op* *expression*

#### Unary Expression
An expression with one term

```
let x = 5;
let ptr_to_x = @5;
```

- *unary_op* := `&` | `@` | `*` | `-` | `not` | `~`
- *unary_expr* := *unary_op* *expression*

#### Field Access Expression
An attempt to access a field on a struct with the `.` operator.

```++
foo.bar;
```

- *field_access_expr* := *expression* `.` *var_name*

#### Index Expression
An index expression is any expression using `[]` to access something.

```
let result = array[5];
```

- *index_expr* := *expression* `[` *expression* `]`

#### Array Expression
An array expression is simply an expression that evaluates to an array.

```
let array = [1, 2, 3, 4, fib(5)];
```

- *array_expr_members* := *expression* | *expression* `,` *array_expr_member*
- *array_expr* := `[` *array_expr_members* `]`

#### Method Call Expression
An attempt to call a method on a struct. While this is syntactically identical to calling a 
functor object in a structure's field, they are different things. Idealy, the analyzer will 
discern which one it is and modify the AST accordingly.  

```++
foo.bar(10);
```

- *method_call_expr* := *field_access_expr* `(` *call_arguments* `)`


#### Lambda Expression
Lambda expressions are effectively syntax sugar for in-line function declarations, but 
they are also able to "capture" variables in their scope. 

```
let y = 3;

let lambda = |x: i32| i32 {
    x + y;
}

assert lambda(5) == 8;
```

- *lambda_expr* := *lambda_typed* | *lambda_untyped*
- *lambda_typed* := `|` *fn_argument_list* `|` *type* *block*

Lambdas can also be type-deducted, allowing you to drop the parameter type and the return type. **Note: They still can only be used with one type! They aren't like a lambda with `auto` parameters in C++, they are not templates.**

- *lambda_argument_list* := *var_name* | *var_name* `,` *lambda_argument_list*
- *lambda_untyped* := `|` *lambda_argument_list* `|` *block_expr*

```
let y = 3;

let type_deducted_lambda = |x| {
    x + y;
}
```

#### Struct Expression
A struct expression is simply an initialization of a structure in place. 

```++
Point3D { x: 3.3333333333333, y: 3892.993874, z: 4 };
```

- *struct_expr_field_initializer* := *var_name* `:` *expression* | (*var_name* `:` *expression*) `,` *struct_expr_field_initializer*
- *struct_expr* := *type_name* `{` *struct_expr_field_initializer* `}`

#### If-Then Expression
An If-Then expression is just another form of `if`, in this case it's more appropriate 
for inline use. `else` is not optional, as if-then always evaluates to a result.

```
if condition then 5 else 6;
```

- *if_then_expr* := `if` *expression* `then` *expression* `else` *expression*

#### Block Expression
A block expression is simply a list of 0 or more expressions and statements wrapped in `{}`s. They 
evaluate to the last expression in the block. If it's a statement, they evaluate to `void` and can 
not be used to initialize any values. Note that semicolons after `{}`s are not needed.

- *block_expr* := `{` *expression** `}`

```
let five_plus_five = {
    let x = 5;

    5 + 5;
}
```

#### If Expression
Just a standard `if`. They can be used as expressions, but only if they can evaluate to a value in every possible case. Note: They always have blocks, the `{}`s are not optional as they are in other languages. 

- *if_expr* := `if` *expression* *block_expr* (`else` (*block_expr* | *if_expr*))?

```
if condition {
    do_something();
}

let y = if x < -2 {
    x + 3;
} else {
    x * 3 + 1;
}
```

#### Match Expression
- *match_expr* := `match` *expression* `{`
- *match_case* := `case` *expression* `->` *expression*
- *match_list* := *match_case* | *match_case* `,` *match_list*  

## Construction, Destruction, Move and Clone Semantics
Cascade has decided to ditch the implicit-ness of move/copy semantics, and references for that matter. The language 
is also very RAII-style, with destruction of an object happening at the end of its scope.

The only implicit thing the language does are moves, and clones
on builtin integral/floating-point types.

```
struct Point {
    pub x: f64;
    pub y: f64;

    pub fn @destroy(self) { ... }
}

fn accepts_point(pt: Point) { ... }

let pt = Point { x = 3.3, y = 12.5 };

accepts_point(pt); -- move happens here
```

After a move, the compiler will enforce no more attempts to access that variable, and the value will be removed from the stack. `Point::@destroy` will not be called. 

```
fn accepts_point(pt: Point) {
    io::print(pt.x); -- pt.x is copied here

    -- pt reaches end of scope without being moved,
    -- pt.@destroy() is called here
}
```

If you need to pass a *copy* of an object, you can use `clone`.

```
accepts_point(clone(pt))
```

This creates a new `Point` instance (let's call it `pt2`), and calls `Point::@clone(&mut pt2, &pt)`. 

## Polymorphism
**TO BE DECIDED**