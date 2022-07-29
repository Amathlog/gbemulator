INCLUDE "hardware.inc"

SECTION "Header", ROM0[$100]

	jp EntryPoint

	ds $150 - @, 0 ; Make room for the header

SECTION "EntryPoint", ROM0[$200]
EntryPoint:
AddTest:
    ; Setup
    ld a, 0
    ld b, 1
    ld c, 2
    ld d, 3
    ld e, 4
    ld hl, $C005
    ld [hl], $FF

    ; Will store all the results in the stack
    ld sp, $D000

    add a, b
    push af
    add a, c
    push af
    add a, d
    push af
    add a, e
    push af
    add a, h
    push af
    add a, l
    push af
    add a, [hl]
    push af
    add a, a
    push af
    add a, 10
    push af

    jp AdcTest

SECTION "AdcTest", ROM0[$300]
AdcTest:
    ; Setup
    ld b, 1
    ld c, 2
    ld d, 3
    ld e, 4
    ld hl, $C005
    ld [hl], $FF

    ; Will store all the results in the stack
    ld sp, $D000

    ld a, $FF
    adc a, b
    push af
    adc a, b
    push af

    ld a, $FF
    adc a, c
    push af
    adc a, c
    push af

    ld a, $FF
    adc a, d
    push af
    adc a, d
    push af

    ld a, $FF
    adc a, e
    push af
    adc a, e
    push af

    ld a, $FF
    adc a, h
    push af
    adc a, h
    push af

    adc a, l
    push af
    adc a, l
    push af

    ld a, $FF
    adc a, [hl]
    push af
    adc a, [hl]
    push af

    ld a, 1
    adc a, a
    push af
    adc a, a
    push af

    ld a, $FF
    adc a, 10
    push af
    adc a, 10
    push af

    jp Done

SECTION "SubTest", ROM0[$400]
SubTest:
        ; Setup
        ld a, 0
        ld b, 1
        ld c, 2
        ld d, 3
        ld e, 4
        ld hl, $C005
        ld [hl], $FF
    
        ; Will store all the results in the stack
        ld sp, $D000
    
        sub a, b
        push af
        sub a, c
        push af
        sub a, d
        push af
        sub a, e
        push af
        sub a, h
        push af
        sub a, l
        push af
        sub a, [hl]
        push af
        sub a, a
        push af
        sub a, 10
        push af
    
        jp Done

SECTION "Done", ROM0[$3FF0]
Done:
    jp Done