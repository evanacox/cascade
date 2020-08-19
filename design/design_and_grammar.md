# Cascade Grammar
Note the regex syntax. Also, between any of the `keywords` or *definitions*, whitespace is allowed. Cascade ignores whitespace, besides when initially reading identifiers. 

## Comments
Comments block out part of a file, and any text that is inside a comment is "thrown out" by the initial lexing phase. 

```hs
-- this is a comment
```

*comment* := *single_line_comment* | *multi_line_comment*

Cascade single-line comments are created with `--`. They last until the end of a line.

*single_line_comment* := `--` .* (`\n` | `\r\n`)

Multi-line comments are created with `-*-`, and last until another `-*-`. They **cannot** be nested.

*multi_line_comment* := `-*-` .* `-*-`

## Literals
Literals represent the direct value of an expression, instead of representing the steps to get 
said value. The value is also known at compile-time, so for the most part they are evaluated then. 

```rust
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

```rust
let byte: i8 = 5;
let point3d: Point3D = { x: 5.5368, y: 24.3333333, z: 23830.14322 };
let ref_to_point3d: &Point3D = &other;
let ptr_to_byte: *i8 = @byte;
let ptr_to_ptr_to_byte: **i8 = @ptr;
let ref_to_ptr_to_array_of_ptrs_to_i32s: &*[]*i32 = ...;
```

Types can be a reference or a standard type. References are denoted with a `&`, and must come at the beginning (as you can't have a pointer to a reference, for instance). Pointers and array come afterwards (as you can have a reference to a pointer or array), then the actual type name.
- *type* := *reference*? *pointer*? *builtin_type* | *type_name*
- *reference* := `&`
- *pointer_or_array* := `* mut` | `*` | `[]`
- *builtin_type* := `bool` | *builtin_signed* | *builtin_unsigned*
- *builtin_signed* := `i8` | `i16` | `i32` | `i64` | `i128`
- *builtin_unsigned* := `u8` | `u16` | `u32` | `u64` | `u128`

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

```c++
const SOME_MAGIC_CONSTANT = 320;
const OTHER_MAGIC_CONSTANT: u8 = 30;
```
- *const_decl* := `const` *var_name* (`:` *type*)? `=` *expression* `;`

#### `static` Declaration
`static` declarations are effectively just global variables. They are initialized at the 
program's startup time, and destroyed at program termination.

```c++
static thing = 32;
```

- *static_decl* := `static` *var_name* (`:` *type*)? `=` *expression* `;`

#### `fn` Declaration 
`fn` declarations define a function. They contain a name, a list of arguments (and their types), 
a return type, and a body.

```rust
fn fibonacci(n: i32) i64 {
  match n {
    case 0, 1, 2 -> n,
    case _ -> fibonacci(n - 1) + fibonacci(n - 2)
  }
}
```

- *fn_decl* := `fn` *fn_name* `(` *fn_argument_list* `)` *type* *block_expr*
- *fn_argument* := *var_name* `:` *type*
- *fn_argument_list* := *fn_argument* | *fn_argument* `,` *fn_argument_list*

#### `struct` Declaration
`struct` declarations are the Cascade equivalent of a struct in C. It represents simply a set
of structured data, and has no associated methods or behaviours. It *only* represents
the data itself. Each field has a name and a type, and an optional `pub` visibility 
modifier to allow a field to be accessed. Note: If no fields are marked `pub`, 
all fields are implicitly `pub`.

```rust
struct Point3D {
    pub x: f64;
    pub y: f64;
    pub z: f64;
}
```

- *struct_decl* := `struct` *type_name* `{` *struct_field** `}`
- *struct_field* := `pub`? *var_name* `:` *type* `;`

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

```rust
let x = 5;
```

- *let_stmt* := `let` *var_name* (`:` *type*)? `=` *expression* `;`

#### `mut` Statement
`mut` statements are exactly the same as `let` semantically, besides one key point. They are mutable.

```rust
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

```rust
mut has_failed = false;

loop {
    x = returns_true_if_fails();

    if x {
        break;
    }
}
```

The second type are conditional loops, also known as `while` loops. 

- *loop_while* := `while` *expression* *block_expr*

```rust
mut done = false;

while !done {
    done = keep_trying();
}
```

The final type is the `for..in` loop. They act on ranges and other "iterable" types.

```rust
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

```rust
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

```rust
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

```nasm
ret fib(n - 1) + fib(n - 2);
```

#### `assert` Statement
`assert` is a statement used to "assert" things during runtime. In un-optimized builds the 
condition of the `assert` is checked, and if it's false, the program panics with some debug
information.

- *assert_stmt* := `assert` *expression* `;`

#### Expression Statement
Expression statements are simply expressions that have their return value discarded.

```hs
vector.push(5);

if true {
    -- do something
}
```

- *expression_stmt* := (*expression_without_block* `;`) | (*expression_with_block* `;`?)

## Modules
Modules are the way that Cascade splits up functions and types. They are Cascade's method of namespacing 
symbols and modularizing a program.

```
module example_api;

import std::core::String;

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

#### Literal Expression
Simply a literal of some sort.

- *literal_expr* := *literal*

#### Call Expression
A call to a function (or a functor object)

- *call_arguments* := *expression* (`,` *expression*)*
- *call_expr* := *expression* `(` *call_arguments* `)`

#### Binary Expression
An expression with two terms 

```rust
let result = x | y;
```

- *binary_op* := `|` | `&` | `^` | `and` | `or` | `xor` | `=` 
| `+=` | `-=` | `*=` | `/=` | `%=` | `<<=` | `>>=` | `&=` 
| `|=` | `^=` | `+` | `-` | `*` | `/` | `%` | `<<` | `>>` | `==` | `!=` | `<` | `<=` | `>` | `>=`

- *binary_expr* := *expression* *binary_op* *expression*

#### Unary Expression
An expression with one term

```rust
let x = 5;
let ptr_to_x = @5;
```

- *unary_op* := `&` | `@` | `*` | `-` | `!` | `~`
- *unary_expr* := *unary_op* *expression*

#### Field Access Expression
An attempt to access a field on a struct with the `.` operator.

```c++
foo.bar;
```

- *field_access_expr* := *expression* `.` *var_name*

#### Index Expression
An index expression is any expression using `[]` to access something.

```rust
let result = array[5];
```

- *index_expr* := *expression* `[` *expression* `]`

#### Array Expression
An array expression is simply an expression that evaluates to an array.

```rust
let array = [1, 2, 3, 4, fib(5)];
```

- *array_expr_members* := *expression* | *expression* `,` *array_expr_member*
- *array_expr* := `[` *array_expr_members* `]`

#### Method Call Expression
An attempt to call a method on a struct. While this is syntactically identical to calling a 
functor object in a structure's field, they are different things. Idealy, the analyzer will 
discern which one it is and modify the AST accordingly.  

```c++
foo.bar(10);
```

- *method_call_expr* := *field_access_expr* `(` *call_arguments* `)`


#### Lambda Expression
Lambda expressions are effectively syntax sugar for in-line function declarations, but 
they are also able to "capture" variables in their scope. 

```rust
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

```rust
let y = 3;

let type_deducted_lambda = |x| {
    x + y;
}
```

#### Struct Expression
A struct expression is simply an initialization of a structure in place. 

```c++
Point3D { x: 3.3333333333333, y: 3892.993874, z: 4 };
```

- *struct_expr_field_initializer* := *var_name* `:` *expression* | (*var_name* `:` *expression*) `,` *struct_expr_field_initializer*
- *struct_expr* := *type_name* `{` *struct_expr_field_initializer* `}`

#### If-Then Expression
An If-Then expression is just another form of `if`, in this case it's more appropriate 
for inline use. `else` is not optional, as if-then always evaluates to a result.

```hs
if condition then 5 else 6;
```

- *if_then_expr* := `if` *expression* `then` *expression* `else` *expression*

#### Block Expression
A block expression is simply a list of 0 or more expressions and statements wrapped in `{}`s. They 
evaluate to the last expression in the block. If it's a statement, they evaluate to `void` and can 
not be used to initialize any values. Note that semicolons after `{}`s are not needed.

- *block_expr* := `{` *expression** `}`

```rust
let five_plus_five = {
    let x = 5;

    5 + 5;
}
```

#### If Expression
Just a standard `if`. They can be used as expressions, but only if they can evaluate to a value in every possible case. Note: They always have blocks, the `{}`s are not optional as they are in other languages. 

- *if_expr* := `if` *expression* *block_expr* (`else` (*block_expr* | *if_expr*))

```rust
if condition {
    do_something();
}

let y = if x < -2 {
    x + 3;
} else {
    x * 3 + 1;
}
```
