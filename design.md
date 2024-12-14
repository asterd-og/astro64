# Astro64 Design
## Instructions
1. nop
2. add(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
3. sub(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
4. mul(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
5. mov(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
6. jmp flags (cond & rel or abs) (dst off r|imm) dest <r|m|imm>
---
- Encoding:
 Encoding can change
 ```
 Size (byte, word, dword or qword) | Op Code    | Src | Dst | Flags
 xx                                | xxxxxx     | xx  | xx  | xxxx
 ```
---
### Src:
0. Reg
1. [Reg]
2. Imm
3. [Imm]
### Dst:
0. Reg
1. [Reg]
2. [Imm]
---
### Flags:
1. Off Src
2. Off Dst
3. Reg Off Src
4. Reg Off Dst
## Registers
- Astro64 contains 16 registers:
  - g0 ... g10 = General Purpose
  - sp = Stack Pointer
  - ip = Instruction Pointer
  - flags = General Flags (Paging enabled, Interrupts enabled, etc)
  - pgtbl = Page Table Address (Physical address)
  - err = Error Code (Page Fault, Debug, Division by 0, etc)
---
### Flags:
 2. Paging enabled
## Memory
- Astro64 has 2 memory addressing modes: Physical & Virtual.
### Virtual memory design
- 5 Level page map, with 256 entries each page level, 2 KB Pages only (Might expand in the future)
```
13 Bits unused | 8 Bits PML5 | 8 Bits PML4 | 8 Bits PML3 | 8 Bits PML2 | 8 Bits PML1 | 11 Bits Page Offset
```
