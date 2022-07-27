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
    jp Done ; Tenth check

SECTION "Done", ROM0[$3FF0]
Done:
	jp Done