# uwe-asm
a functional assembly language

## What it is
uwe-asm is a purely functional assembly language; a chip can built that runs uwe-asm natively (eventually I might try to make one). It follows all the requirements for a purely functional language (isolated IO, partial execution, first class functions, etc.) using only a string of bytes.

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
- Function arguments (at most 5) *TODO*
- `000...0` to signify the end of the line; think of this as a semicolon

So, for example, the line:

`0x00000003 0x80000000 0x00000002 0xC0000009 0x00000000`

Means:

`Var 3, Built-in func. 0, Var 2, Int 9, 0`

Which translates to:

`Var3 = add(Var2, 9);`
