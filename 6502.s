.segment "CODE"
.org $8000

start:
    ldx #5

loop:
    txa
    jsr print_digit
    dex
    bne loop
    brk

print_digit:
    adc #'0'
    sta $FF00
    lda #$0A
    sta $FF00
    rts
