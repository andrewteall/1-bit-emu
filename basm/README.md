# Basm - The Bit Assembler
Basm allows for assembly of MC14500 programs. Uses DASM syntax for the most part with the MC14500 Handbook Mnemonics.

## Usage
```sh
Usage: basm [options] file
Options:
 -a N,         --align N                    Align or pad the binary with 0 to a specified size N.
-ap N,         --address-position N         Sets the position of the address within the memory from the MSB. Default: 4
-aw N,         --address-width N            Set the width in bits of the address. Default: 4
 -e [L|B],     --endianess [L|B]            Set endianess for output file. Default: Little Endian
 -f FILE,      --filename FILE              Specify to the file to be assembled. Otherwise the last argument is used.
 -h,           --help                       Print this message and exit.
-ip N,         --instruction-position N     Sets the position of the instruction within the memory from the MSB. Default: 0
-iw N,         --instruction-width N        Set the width in bits of the instruction. Default: 4
 -l,           --lable-table                Print Label Table.
 -m N,         --max-include-depth N        Maximum depth of files that can be included. Default: 10
 -o FILE,      --outfile FILE               Specify output file. Default: $file.bin
               --print-parsed               Prints tokens after parsing.
 -p,           --pretty-print               Pretty print binary file at completion.
 -s,           --split-file                 Makes two seperate files one with Opcodes and one with Operands.
 -t,           --tokenize-only              Only read the file and generate tokens.
 -v N,         --v[vvvv]                    Set the log verbosity to N, 0=OFF, 1=FATAL, 2=ERROR, 3=WARNING, 4=INFO, 5=DEBUG.
 ```