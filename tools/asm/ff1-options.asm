; SPDX-License-Identifier: GPL-3.0-or-later
; US MMC1 ROM only. Addresses are bank 0E CPU addresses.
; References: Entroper/FF1Disassembly bank_0E, bank_0F.
; Bit7 of each NPC's game_flags byte is unused by vanilla. Bits0..2 retained.
; Rewards use their own persistent claim bit, NOT current item ownership.
; This prevents consumed rewards being reissued and Matoya being skipped when
; the Herb was found elsewhere. Gate events and consumed trade inputs stay intact.
.group major
.org $931e
.limit $9338
 LDY #$06
 JSR $9079
 BCS elf_awake
 LDA $13
 RTS
elf_awake:
 LDY #$06
elf_item:
 LDX #$05
 JMP reward
.org $9338
.limit $9352
 LDA $6022
 BNE astos_fight
 LDA $11
 RTS
astos_fight:
 LDY #$07
 JSR $9096
 LDA #$7d
 JSR $90c5
 LDY #$07
astos_item:
 LDX #$03
 JMP reward
.org $9398
.limit $93ba
 LDY #$0a
 LDA $6200,Y
 BMI matoya_done
 LDA $6023
 BEQ matoya_need
 DEC $6023
matoya_item:
 LDX #$04
 JMP reward
matoya_need:
 LDA $11
 RTS
matoya_done:
 LDA $13
 RTS
.org $93e4
.limit $93fb
 LDY #$0c
 JSR $9091
 BCS sarda_wait
 LDY #$0d
sarda_item:
 LDX #$0a
 JMP reward
sarda_wait:
 LDA $12
 RTS
.org $9438
.limit $9468
cube:
 LDY #$11
cube_item:
 LDX #$0e
 JMP reward
princess:
 LDY #$12
princess_item:
 LDX #$01
 JMP reward
fairy:
 LDY #$13
fairy_item:
 LDX #$10
 JMP reward
reward:
 LDA $6200,Y
 BMI reward_done
 ORA #$80
 STA $6200,Y
 INC $6020,X
 STX $61
 INC $7d
 LDA #$f0
 RTS
reward_done:
 LDA $12
 RTS
.org $9594
.limit $95ae
 LDY #$0b
 JSR $9079
 BCS chime_known
 LDA $13
 RTS
chime_known:
 LDY #$bb
chime_item:
 LDX #$0c
 JMP reward
; Update only the handlers whose entry addresses moved.
.org $90f7
.limit $90fb
.word princess, fairy
; The unused/alternate robot must not supply an extra vanilla Cube.
.org $9217
.limit $9219
.word cube
.group shops
; Consumable/quest items still increment their counter. Weapons go to the
; first free party weapon slot. Full inventories return carry, without charging.
; 84E2..84FF is documented unused padding.
.org $84e2
.limit $8500
shop_award:
 LDX $030c
 CPX #$1c
 BCS shop_weapon
 LDA $6020,X
 CMP #$63
 BCS shop_done
 INC $6020,X
 CLC
shop_done:
 RTS
shop_weapon:
 JMP weapon_award
; BF3A..BF4F is documented unused phantom code, before weapon permissions.
.org $bf3a
.limit $bf50
weapon_award:
 JSR $dd34
 BCS weapon_done
 LDA $030c
 SEC
 SBC #$1b
 STA $6100,X
 CLC
weapon_done:
 RTS
.org $a4ad
.limit $a4cd
 JSR shop_award
 BCS shop_full
 JSR $a4cd
 LDA #$13
 JSR $aa5b
 JMP $a481
shop_full:
 LDA #$0c
 JSR $aa5b
 JMP $a481

.group slot_prices
; Preserve global item/resale prices. Buy-list price tokens use a parallel list
; of ORIGINAL slot item IDs. 0340..0345 is outside shop str_buf (0300..0338)
; and below palette memory (03C0); rebuilt whenever inventory is loaded.
.bank $0f
.org $ecb9
.limit $ecd0
 PHA
 LDA #$0d
 JSR $fe03
 PLA
 JSR global_price
price_return:
 LDA #$0e
 JMP $fe03
.org $ecd0
.limit $ecda
load_slot_prices:
 LDA #$0d
 JSR $fe03
 JMP slot_price_list
.bank $0d
; Disassembly documents B5C9..B7FF as unused. Use B600..B7E2 only.
.org $b600
.limit $b640
global_price:
 ASL A
 STA $12
 LDA #$5e
 ROL A
 STA $13
 LDY #$00
 LDA ($12),Y
 STA $10
 INY
 LDA ($12),Y
 STA $11
 LDA #$00
 STA $12
 RTS
.org $b640
.limit $b680
slot_price_list:
 LDA #$00
 STA $0305
 STA $11
 LDA $51
 ASL A
 ASL A
 ROL $11
 CLC
 ADC $51
 STA $10
 LDA $11
 ADC #$b6
 STA $11
 LDA $10
 CLC
 ADC #$80
 STA $10
 BCC slot_pointer_ready
 INC $11
slot_pointer_ready:
 LDY #$04
slot_copy:
 LDA ($10),Y
 STA $0340,Y
 DEY
 BPL slot_copy
 LDA #$01
 STA $0345
 JMP price_return
.bank $0e
.org $a7e9
.limit $a7ef
 JMP load_slot_prices
; Change only the price token; the name token still identifies the bought item.
; 8467..847F is FF padding after the final shop and before copyright text.
.org $a867
.limit $a86a
 JSR slot_price_token
.org $8467
.limit $8480
slot_price_token:
 LDA $0345
 BEQ sale_price_token
 LDA $0340,X
 BNE store_price_token
sale_price_token:
 LDA $0300,X
store_price_token:
 STA $0316,Y
 RTS
; Sell list uses ordinary item prices; its confirmation still halves that price.
.org $a806
.limit $a809
 JSR selling_prices
.org $84f8
.limit $8500
selling_prices:
 LDA #$00
 STA $0345
 TAY
 RTS
; Confirmation: item ID from name token, price ID from price token.
.org $aa7e
.limit $aa81
 LDA $02fb,X
.org $aa88
.limit $aa8b
 LDA $0300,X
