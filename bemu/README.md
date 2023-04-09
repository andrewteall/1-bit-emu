# Bemu - The Bit Emulator

Bemu provides away to emulate and debug MC14500 programs.

## Usage
```
Usage: bemu [options] file
Options:
-ap N,         --address-position N         Sets the position of the address within the memory from the MSB. Default: 4
-aw N,         --address-width N            Set the width in bits of the address. Default: 4
 -b N,         --bind-rr-address N          Binds the Result Register pin to the specified address.
 -d,           --debug                      Enable the debugger.
 -e L|B,       --endianess L|B              Set endianess for output file. Default: Little Endian
 -m J|R|O|F S, --map-pin J|R|O|F S,         Maps a User Pin to the specified function.
                                            S = NONE, JUMP, JSR, RET, JSRS, RETS, HLT, RES, or NULL
 -h,           --help                       Print this message and exit.
-io N,         --io-count                   Sets the number of io address available to the system. Default 15.
-ip N,         --instruction-position N     Sets the position of the instruction within the memory from the MSB. Default: 0
-iw N,         --instruction-width N        Set the width in bits of the instruction. Default: 4
 -p,           --print-state                Prints system status after each instruction. Does not work if --debug is set.
-pc N,         --program-counter N          Set the Program Counter to an initial address other than 0. Default: 0
 -r N,         --rom-size N                 Set the size of the Program ROM in bytes. Default: 255
 -s,           --step-mode                  Runs program in step mode. Valid when --debug flag is set.
 -v N,         --v[vvvv]                    Set the log verbosity to N, 0=OFF, 1=FATAL, 2=ERROR, 3=WARNING, 4=INFO, 5=DEBUG.
 ```

The mapped functions are:

```JUMP``` - "Jump" to a specified address like GOTO

```JSR``` - "Jump SubRoutine" Pushes the Program Counter on the stack and then jumps to the specified address.

```RET``` - "Return from subroutine" Pops an address off the stack and stores it in the Program Counter.

```JSRS``` - "Jump SubRoutine Shallow" Stores the Program Counter in a register and jumps to the specified address. Will overwrite existing value stored in register. Useful when only jumping one level deep.

```RETS``` - "Return from Subroutine Shallow" Reads the value address store in a register and sets the Program Counter. Useful when only returning one level deep.

```HLT``` - "Halt" the ICU

```RES``` - "Reset" the ICU
