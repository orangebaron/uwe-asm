# uwe-asm
a functional assembly/machine language

## What it is
uwe-asm is a purely functional assembly/machine language; a chip can built that runs uwe-asm natively (eventually I might try to make one). It follows all the requirements for a purely functional language (isolated IO, partial execution, first class functions, etc.) using only a string of bytes.

## How it works
uwe-asm code is a set of ints; they come in 4 types:

| Highest 2 bits | Type | Example |
|:-:|:-:|:-:|
| `00` | Variable | `00000...01` -> "The value of variable number 1" |
| `01` | Function Reference | `010000...0101` -> "The function at location 5" |
| `10` | Built-In Function/Variable | `10000...01` -> "Built-in function 0 (aka add)" *TODO* |
| `11` | Integer | `11000...01110` -> "(Int) 14" |

A line of uwe-asm code contains:
- A variable to write to (to return, write the returned value variable 0)
- A function to call
- Function arguments (at most 5, so that the line takes up at most 8 integers) *TODO*
- `000...0` to signify the end of the line; think of this as a semicolon

So, for example, the line:

`0x00000003 0x40000009 0x00000002 0xC0000009 0x00000000`

Means:

`Var 3, Func Reference 9, Var 2, Int 9, 0`

Which translates to:

`Var3 = Func9(Var2, 9);`

### Built-In Function List

| Number | Name | Notes/Exceptions |
|:-:|:-:|:-:|
| 0 | Add | If only a single argument is given, returns `arg+1` |
| 1 | Subtract | If only a single argument is given, returns `0-arg` |
| 2... | .... | ... *TODO* |
| n | ls | Creates a list of `[arg 1, arg 2, ...]` |
| n+1 | ls2D | Creates `[[arg 1, arg 2, ...]]` |
| n+2 | insert | Returns `[arg 1, ...second-to-last arg]++(last arg, a list)` |
| n+3 | append | Returns `(arg 1, a list)++[arg 2, arg 3, ...]` |
| n+4 | get | Gets `(arg 1, a list)[arg 2]` |
| n+5 | set | Arg 1, a list, but with element (arg 2) set to (arg 3) |
| n+6 | size | Returns the size of arg |

### Explanation of IO

The main function will return IO to be performed. IO is a 2D (or more) list of operations to be performed, with each element being one of the following:

- `[Function reference]`, which is called, and the return value inserted to the beginning of the IO list.
- `[Function reference, arg 1, arg 2, ...]` Calls that function with those arguments and appends the value to the beginning of the IO list
- `[Built-in func]` Calls that built-in func
- `[Built-in func, arg 1, arg 2, ...]` Calls that built-in func with those arguments
- `[[...],[...],...]` Run the IO at the first element of the first list; nested IO can be made in this way, which is necessary in certain cases.

Once one IO is performed, its return value is fed into the next IO. For example, `[[input],[print]]` will input, feed that to the `print` function, and print the inputted value. If the value is not required, it is discarded. For example, in `[[input],[print,'A']`, a character is inputted, but 'A' is printed, not that character.

IO may differ from platform to platform depending on the platform's needs, but it will always follow this list-based format.
