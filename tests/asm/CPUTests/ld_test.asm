INCLUDE "hardware.inc"

SECTION "Header", ROM0[$100]

	jp EntryPoint

	ds $150 - @, 0 ; Make room for the header

SECTION "EntryPoint", ROM0[$200]
EntryPoint:
LiteralLoad:
    ld b, $12
    ld c, $34
    ld d, $56
    ld e, $78
    ld h, $9A
    ld l, $BC
    ld a, $DE
    jp LitteralLoad16Bits ; First check

SECTION "LitteralLoad16Bits", ROM0[$300]
LitteralLoad16Bits:
    ld bc, $BCDE
    ld de, $1234
    ld hl, $5678
    ld sp, $1111
    jp LitteralLoadMemory ; Second check

SECTION "LitteralLoadMemory", ROM0[$400]
LitteralLoadMemory:
    ld hl, $C000 ; Start of WRAM
    ld [hl], $7F
    jp CleanUp ; Third check

SECTION "CleanUp", ROM0[$450]
CleanUp:
    ld [hl], $00
    ld bc, $0000
    ld de, $0000
    ld hl, $0000
    ld sp, $0000
    ld a, $00
    jp BIntoRegister
    
SECTION "BIntoRegister", ROM0[$500]
BIntoRegister:
    ld b, $34
    ld c, b
    ld b, $56
    ld d, b
    ld b, $78
    ld e, b
    ld b, $9A
    ld h, b
    ld b, $BC
    ld l, b
    ld b, $DE
    ld a, b
    ld b, $12
    ld b, b
    jp CIntoRegister ; Fourth check
    
SECTION "CIntoRegister", ROM0[$600]
CIntoRegister:
    ld c, $23
    ld b, c
    ld c, $67
    ld d, c
    ld c, $89
    ld e, c
    ld c, $AB
    ld h, c
    ld c, $CD
    ld l, c
    ld c, $EF
    ld a, c
    ld c, $45
    ld c, c
    jp DIntoRegister ; Fifth check
    
SECTION "DIntoRegister", ROM0[$700]
DIntoRegister:
    ld d, $13
    ld b, d
    ld d, $24
    ld c, d
    ld d, $46
    ld e, d
    ld d, $57
    ld h, d
    ld d, $68
    ld l, d
    ld d, $79
    ld a, d
    ld d, $35
    ld d, d
    jp EIntoRegister ; Sixth check

SECTION "EIntoRegister", ROM0[$800]
EIntoRegister:
    ld e, $14
    ld b, e
    ld e, $25
    ld c, e
    ld e, $36
    ld d, e
    ld e, $58
    ld h, e
    ld e, $69
    ld l, e
    ld e, $7A
    ld a, e
    ld e, $47
    ld e, e
    jp HIntoRegister ; Seventh check
    
SECTION "HIntoRegister", ROM0[$900]
HIntoRegister:
    ld h, $53
    ld b, h
    ld h, $64
    ld c, h
    ld h, $75
    ld d, h
    ld h, $86
    ld e, h
    ld h, $A8
    ld l, h
    ld h, $B9
    ld a, h
    ld h, $97
    ld h, h
    jp LIntoRegister ; Eighth check

SECTION "LIntoRegister", ROM0[$A00]
LIntoRegister:
    ld l, $63
    ld b, l
    ld l, $74
    ld c, l
    ld l, $85
    ld d, l
    ld l, $96
    ld e, l
    ld l, $A7
    ld h, l
    ld l, $C9
    ld a, l
    ld l, $B8
    ld l, l
    jp AIntoRegister ; Nineth check

SECTION "AIntoRegister", ROM0[$B00]
AIntoRegister:
    ld a, $73
    ld b, a
    ld a, $84
    ld c, a
    ld a, $95
    ld d, a
    ld a, $A6
    ld e, a
    ld a, $B7
    ld h, a
    ld a, $C8
    ld l, a
    ld a, $D9
    ld a, a
    jp AFromMemoryRegister ; Tenth check

SECTION "AFromMemoryRegister", ROM0[$C00]
AFromMemoryRegister:
    ; Setup
    ld hl, $FF80
    ld [hl], $AB
    ld hl, $FF81
    ld [hl], $CD
    ld bc, $C080
    ld hl, $C080
    ld [hl], $12
    ld de, $C002
    ld hl, $C002
    ld [hl], $34
    ld hl, $C003
    ld [hl], $45
    ld hl, $C004
    ld [hl], $67
    ld hl, $C003
    jp AFromMemoryRegister1

SECTION "AFromMemoryRegister1", ROM0[$C30]
AFromMemoryRegister1:
    ld a, [bc]
    ld a, [de]
    ld a, [hli]
    ld a, [hld]
    ld a, [$FF00 + c]
    ldh a, [$FF00 + $81]
    ld a, [$FF80]
    jp AToMemoryRegisters

SECTION "AToMemoryRegisters", ROM0[$D00]
AToMemoryRegisters:
    ; Setup
    ld bc, $C080
    ld de, $C000
    ld hl, $C001
    jp AToMemoryRegisters1

SECTION "AToMemoryRegisters1", ROM0[$D10]
AToMemoryRegisters1:
    ld a, $12
    ld [bc], a
    ld a, $34
    ld [de], a
    ld a, $56
    ld [hli], a
    ld a, $78
    ld [hld], a
    ld a, $9A
    ld [$FF00 + c], a
    ld a, $BC
    ldh [$FF00 + $81], a
    ld a, $DE
    ld [$FF82], a
    jp SpecialLoads_SP

SECTION "SpecialLoads_SP", ROM0[$E00]
SpecialLoads_SP:
    ld hl, $9FFF
    ld sp, hl
    ld [$C000], sp
    ld hl, sp + 10
    ld hl, sp - 10
    jp Done

SECTION "Done", ROM0[$3FF0]
Done:
	jp Done