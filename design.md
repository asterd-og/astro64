# Astro64 Design
## Instructions
0. nop
1. add(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
2. sub(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
3. mul(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
4. div(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
5. mov(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
6. jmp flags (cond & rel or abs) (src off r|imm) src <r|m|imm>
7. push(8|16|32|64) (src off r|imm) src <r|m|imm>
8. pop(8|16|32|64) (dst off r|imm) dst <r|m|imm>
9. call (src off r|imm) src <r|m|imm>
10. ret
11. and(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
12. or(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
13. xor(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
14. not(8|16|32|64) (dst off r|imm) dest <r|m>
15. shl(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
16. shr(8|16|32|64) (src off r|imm) src <r|m|imm> (dst off r|imm) dest <r|m>
17. sei (set enable interrupts)
18. sdi (set disable interrupts)
19. int (src off r|imm) src <r|m|imm>
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
- Astro64 contains 17 registers:
  - g0 ... g10 = General Purpose
  - sp = Stack Pointer
  - fp = Frame Pointer
  - ip = Instruction Pointer
  - flags = General Flags (Paging enabled, Interrupts enabled, etc)
  - pgtbl = Page Table Address (Physical address)
  - ivtbl = Interrupt Vector Table
---
### Flags:
 1. Carry
 2. Zero
 3. Paging enabled
 4. Interrupts enabled
## Memory
- Astro64 has 2 memory addressing modes: Physical & Virtual.
### Virtual memory design
- 5 Level page map, with 256 entries each page level, 2 KB Pages only (Might expand in the future)
```
13 Bits unused | 8 Bits PML5 | 8 Bits PML4 | 8 Bits PML3 | 8 Bits PML2 | 8 Bits PML1 | 11 Bits Page Offset
```
## Interrupts
### Interrupt Vector (IV)
| Size | Name         |
| ---- | ------------ |
| 8    | Handler Addr |
| 1    | Flags        |
### Interrupt Vector Table (IVT)
| Size | Name       |
| ---- | ---------- |
| 8    | Table Addr |
| 1    | Count      |
- Table Addr should point to the address of a table that holds an array of IVs (Interrupt Vectors) in a row.