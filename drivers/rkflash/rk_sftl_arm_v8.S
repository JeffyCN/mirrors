/* SPDX-License-Identifier: GPL-2.0 */

/* Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 * date: 2022-08-15
 */
	.arch armv8-a
	.file	"rk_sftl.c"
	.text
	.align	2
	.type	l2p_addr_tran.isra.0, %function
l2p_addr_tran.isra.0:
	adrp	x4, .LANCHOR0
	add	x4, x4, :lo12:.LANCHOR0
	ldrh	w3, [x4, 8]
	ldrh	w6, [x4, 10]
	ldrh	w4, [x4, 14]
	cmp	w4, 4
	bne	.L2
	lsr	w3, w3, 1
	ubfiz	w6, w6, 1, 15
.L2:
	lsr	w5, w0, 10
	and	w0, w0, 1023
	and	w4, w5, 65535
	and	w5, w5, 65535
	udiv	w4, w4, w3
	msub	w3, w3, w4, w5
	madd	w3, w3, w6, w0
	mov	w0, 0
	str	w3, [x1]
	str	w4, [x2]
	ret
	.size	l2p_addr_tran.isra.0, .-l2p_addr_tran.isra.0
	.align	2
	.global	ftl_print_sblk_info
	.type	ftl_print_sblk_info, %function
ftl_print_sblk_info:
	stp	x29, x30, [sp, -32]!
	adrp	x1, .LC0
	adrp	x0, .LC1
	add	x1, x1, :lo12:.LC0
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	add	x0, x0, :lo12:.LC1
	bl	sftl_printk
	adrp	x0, .LC2
	add	x0, x0, :lo12:.LC2
	ldrh	w1, [x19, 24]
	ldr	x3, [x19, 72]
	ldrh	w5, [x19, 28]
	ubfiz	x2, x1, 1, 16
	ldrb	w4, [x19, 32]
	ldrh	w6, [x3, x2]
	ldrb	w3, [x19, 30]
	ldrh	w2, [x19, 26]
	bl	sftl_printk
	ldrh	w1, [x19, 80]
	adrp	x0, .LC3
	ldr	x3, [x19, 72]
	add	x0, x0, :lo12:.LC3
	ldrh	w5, [x19, 84]
	ubfiz	x2, x1, 1, 16
	ldrb	w4, [x19, 88]
	ldrh	w6, [x3, x2]
	ldrb	w3, [x19, 86]
	ldrh	w2, [x19, 82]
	bl	sftl_printk
	ldrh	w1, [x19, 128]
	adrp	x0, .LC4
	ldr	x3, [x19, 72]
	add	x0, x0, :lo12:.LC4
	ldrh	w5, [x19, 132]
	ubfiz	x2, x1, 1, 16
	ldrb	w4, [x19, 136]
	ldrh	w6, [x3, x2]
	ldrb	w3, [x19, 134]
	ldrh	w2, [x19, 130]
	bl	sftl_printk
	ldrh	w1, [x19, 176]
	adrp	x0, .LC5
	ldr	x3, [x19, 72]
	add	x0, x0, :lo12:.LC5
	ldrh	w5, [x19, 180]
	ubfiz	x2, x1, 1, 16
	ldrb	w4, [x19, 184]
	ldrh	w6, [x3, x2]
	ldrb	w3, [x19, 182]
	ldrh	w2, [x19, 178]
	bl	sftl_printk
	ldrh	w3, [x19, 224]
	adrp	x0, .LC6
	ldrh	w2, [x19, 226]
	add	x0, x0, :lo12:.LC6
	ldrh	w1, [x19, 228]
	bl	sftl_printk
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	ftl_print_sblk_info, .-ftl_print_sblk_info
	.align	2
	.global	Ftl_log2
	.type	Ftl_log2, %function
Ftl_log2:
	mov	w2, 1
	mov	w1, 0
.L6:
	cmp	w2, w0
	bls	.L7
	sub	w0, w1, #1
	ret
.L7:
	add	w1, w1, 1
	lsl	w2, w2, 1
	and	w1, w1, 65535
	b	.L6
	.size	Ftl_log2, .-Ftl_log2
	.align	2
	.global	FtlPrintInfo
	.type	FtlPrintInfo, %function
FtlPrintInfo:
	ret
	.size	FtlPrintInfo, .-FtlPrintInfo
	.align	2
	.global	FtlSysBlkNumInit
	.type	FtlSysBlkNumInit, %function
FtlSysBlkNumInit:
	and	w0, w0, 65535
	mov	w1, 24
	cmp	w0, 24
	csel	w0, w0, w1, cs
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ldrh	w2, [x1, 236]
	ldrh	w3, [x1, 246]
	str	w0, [x1, 232]
	mul	w2, w2, w0
	sub	w0, w3, w0
	strh	w0, [x1, 244]
	ldr	w0, [x1, 252]
	str	w2, [x1, 240]
	sub	w2, w0, w2
	mov	w0, 0
	str	w2, [x1, 248]
	ret
	.size	FtlSysBlkNumInit, .-FtlSysBlkNumInit
	.align	2
	.global	FtlConstantsInit
	.type	FtlConstantsInit, %function
FtlConstantsInit:
	stp	x29, x30, [sp, -16]!
	mov	x7, x0
	adrp	x4, .LANCHOR0
	add	x29, sp, 0
	ldrh	w2, [x0]
	add	x0, x4, :lo12:.LANCHOR0
	ldrh	w3, [x7, 14]
	ldrh	w6, [x7, 2]
	ldrh	w5, [x7, 4]
	cmp	w3, 4
	ldrh	w1, [x7, 6]
	strh	w2, [x0, 256]
	strh	w6, [x0, 258]
	strh	w5, [x0, 260]
	strh	w1, [x0, 246]
	strh	w3, [x0, 262]
	bne	.L11
	lsr	w1, w1, 1
	strh	w1, [x0, 246]
	mov	w1, 8
	strh	w1, [x0, 262]
.L12:
	add	x1, x4, :lo12:.LANCHOR0
	mov	x0, 0
	add	x3, x1, 264
.L13:
	strb	w0, [x0, x3]
	add	x0, x0, 1
	cmp	x0, 32
	bne	.L13
	mov	w0, 5
	strh	wzr, [x1, 298]
	strh	w0, [x1, 296]
	cmp	w2, 1
	bne	.L14
	strh	w2, [x1, 296]
.L14:
	add	x3, x4, :lo12:.LANCHOR0
	mov	w0, 640
	strh	w0, [x3, 300]
	ldrh	w0, [x3, 260]
	ldrh	w5, [x3, 246]
	mul	w6, w6, w0
	mul	w0, w0, w5
	and	w6, w6, 65535
	strh	w6, [x3, 236]
	and	w0, w0, 65535
	strh	w0, [x3, 302]
	bl	Ftl_log2
	ldrh	w9, [x7, 12]
	ldrh	w8, [x3, 262]
	strh	w0, [x3, 304]
	strh	w9, [x3, 306]
	mul	w0, w6, w9
	strh	w9, [x3, 308]
	strh	w0, [x3, 310]
	mov	w0, w8
	bl	Ftl_log2
	and	w2, w0, 65535
	strh	w0, [x3, 312]
	ubfiz	w0, w8, 9, 7
	strh	w0, [x3, 314]
	ubfx	w0, w0, 8, 8
	strh	w0, [x3, 316]
	ldrh	w0, [x7, 20]
	strh	w0, [x3, 318]
	mul	w0, w6, w5
	str	w0, [x3, 252]
	mul	w0, w0, w8
	mul	w8, w8, w9
	mul	w0, w0, w9
	asr	w0, w0, 11
	str	w0, [x3, 320]
	mov	w0, 5120
	sdiv	w0, w0, w8
	and	w0, w0, 65535
	cmp	w0, 4
	bls	.L15
.L21:
	add	x1, x4, :lo12:.LANCHOR0
	strh	w0, [x3, 324]
	mov	w0, 640
	lsl	w5, w5, 6
	asr	w0, w0, w2
	add	w2, w2, 9
	add	w0, w0, 2
	strh	w0, [x1, 326]
	asr	w5, w5, w2
	strh	w5, [x1, 328]
	and	w5, w5, 65535
	cmp	w6, 1
	mul	w0, w5, w6
	add	w5, w5, 8
	str	w0, [x1, 332]
	ldrh	w0, [x1, 324]
	udiv	w0, w0, w6
	add	w5, w0, w5
	beq	.L17
.L22:
	add	x4, x4, :lo12:.LANCHOR0
	str	w5, [x1, 232]
	ldrh	w0, [x4, 232]
	bl	FtlSysBlkNumInit
	ldr	w0, [x4, 232]
	str	w0, [x4, 336]
	ldr	w0, [x4, 248]
	ldrh	w1, [x4, 306]
	str	wzr, [x4, 344]
	lsl	w0, w0, 2
	mul	w0, w0, w1
	ldrh	w1, [x4, 312]
	add	w1, w1, 9
	lsr	w0, w0, w1
	add	w0, w0, 2
	strh	w0, [x4, 340]
	mov	w0, 32
	strh	w0, [x4, 342]
	ldrh	w0, [x4, 324]
	add	w0, w0, 3
	strh	w0, [x4, 324]
	ldr	w0, [x4, 332]
	add	w0, w0, 3
	str	w0, [x4, 332]
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
.L11:
	cmp	w3, 8
	bne	.L12
	cmp	w5, 1
	bne	.L12
	lsr	w1, w1, 1
	strh	w1, [x0, 246]
	mov	w1, 2
	strh	w1, [x0, 260]
	b	.L12
.L15:
	mov	w0, 4
	b	.L21
.L17:
	add	w5, w5, 4
	b	.L22
	.size	FtlConstantsInit, .-FtlConstantsInit
	.align	2
	.global	IsBlkInVendorPart
	.type	IsBlkInVendorPart, %function
IsBlkInVendorPart:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ldrh	w2, [x1, 348]
	cbz	w2, .L27
	ldrh	w2, [x1, 324]
	ldr	x3, [x1, 352]
	mov	x1, 0
.L25:
	cmp	w2, w1, uxth
	bhi	.L26
.L27:
	mov	w0, 0
	ret
.L26:
	add	x1, x1, 1
	add	x4, x3, x1, lsl 1
	ldrh	w4, [x4, -2]
	cmp	w4, w0
	bne	.L25
	mov	w0, 1
	ret
	.size	IsBlkInVendorPart, .-IsBlkInVendorPart
	.align	2
	.global	FtlCacheWriteBack
	.type	FtlCacheWriteBack, %function
FtlCacheWriteBack:
	mov	w0, 0
	ret
	.size	FtlCacheWriteBack, .-FtlCacheWriteBack
	.align	2
	.global	sftl_get_density
	.type	sftl_get_density, %function
sftl_get_density:
	adrp	x0, .LANCHOR0+344
	ldr	w0, [x0, #:lo12:.LANCHOR0+344]
	ret
	.size	sftl_get_density, .-sftl_get_density
	.align	2
	.global	FtlBbmMapBadBlock
	.type	FtlBbmMapBadBlock, %function
FtlBbmMapBadBlock:
	stp	x29, x30, [sp, -32]!
	and	w1, w0, 65535
	mov	w4, 1
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	add	x19, x19, 360
	ldrh	w0, [x19, -58]
	udiv	w3, w1, w0
	and	w2, w3, 65535
	msub	w3, w3, w0, w1
	add	x0, x19, x2, uxth 3
	and	w3, w3, 65535
	ldr	x0, [x0, 32]
	ubfx	x5, x3, 5, 11
	lsl	x5, x5, 2
	lsl	w4, w4, w3
	ldr	w6, [x0, x5]
	orr	w4, w4, w6
	str	w4, [x0, x5]
	adrp	x0, .LC7
	add	x0, x0, :lo12:.LC7
	bl	sftl_printk
	ldrh	w0, [x19, 6]
	add	w0, w0, 1
	strh	w0, [x19, 6]
	mov	w0, 0
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlBbmMapBadBlock, .-FtlBbmMapBadBlock
	.align	2
	.global	FtlBbmIsBadBlock
	.type	FtlBbmIsBadBlock, %function
FtlBbmIsBadBlock:
	adrp	x2, .LANCHOR0
	add	x3, x2, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ldrh	w1, [x3, 302]
	udiv	w2, w0, w1
	msub	w0, w2, w1, w0
	add	x2, x3, x2, uxth 3
	and	w0, w0, 65535
	ldr	x1, [x2, 392]
	ubfx	x3, x0, 5, 11
	ldr	w1, [x1, x3, lsl 2]
	lsr	w0, w1, w0
	and	w0, w0, 1
	ret
	.size	FtlBbmIsBadBlock, .-FtlBbmIsBadBlock
	.align	2
	.global	FtlBbtInfoPrint
	.type	FtlBbtInfoPrint, %function
FtlBbtInfoPrint:
	ret
	.size	FtlBbtInfoPrint, .-FtlBbtInfoPrint
	.align	2
	.global	FtlBbtCalcTotleCnt
	.type	FtlBbtCalcTotleCnt, %function
FtlBbtCalcTotleCnt:
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	mov	w4, 0
	mov	w5, 0
	ldrh	w6, [x0, 302]
	ldrh	w0, [x0, 258]
	mul	w6, w6, w0
	cmp	w4, w6
	blt	.L45
	mov	w0, w5
	ret
.L45:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
.L38:
	mov	w0, w4
	bl	FtlBbmIsBadBlock
	cbz	w0, .L37
	add	w5, w5, 1
	and	w5, w5, 65535
.L37:
	add	w4, w4, 1
	and	w4, w4, 65535
	cmp	w4, w6
	blt	.L38
	mov	w0, w5
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlBbtCalcTotleCnt, .-FtlBbtCalcTotleCnt
	.align	2
	.global	V2P_block
	.type	V2P_block, %function
V2P_block:
	adrp	x4, .LANCHOR0
	add	x4, x4, :lo12:.LANCHOR0
	and	w0, w0, 65535
	and	w1, w1, 65535
	ldrh	w2, [x4, 260]
	ldrh	w4, [x4, 302]
	udiv	w3, w0, w2
	msub	w0, w3, w2, w0
	madd	w2, w2, w1, w0
	madd	w0, w3, w4, w2
	ret
	.size	V2P_block, .-V2P_block
	.align	2
	.global	P2V_plane
	.type	P2V_plane, %function
P2V_plane:
	adrp	x2, .LANCHOR0
	add	x2, x2, :lo12:.LANCHOR0
	and	w3, w0, 65535
	ldrh	w1, [x2, 260]
	ldrh	w2, [x2, 302]
	udiv	w0, w3, w1
	udiv	w2, w3, w2
	msub	w0, w0, w1, w3
	madd	w0, w1, w2, w0
	ret
	.size	P2V_plane, .-P2V_plane
	.align	2
	.global	P2V_block_in_plane
	.type	P2V_block_in_plane, %function
P2V_block_in_plane:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	and	w3, w0, 65535
	ldrh	w2, [x1, 302]
	ldrh	w1, [x1, 260]
	udiv	w0, w3, w2
	msub	w0, w0, w2, w3
	and	w0, w0, 65535
	udiv	w0, w0, w1
	ret
	.size	P2V_block_in_plane, .-P2V_block_in_plane
	.align	2
	.global	ftl_cmp_data_ver
	.type	ftl_cmp_data_ver, %function
ftl_cmp_data_ver:
	cmp	w0, w1
	mov	w2, -2147483648
	bls	.L50
	sub	w1, w0, w1
	cmp	w1, w2
	cset	w0, ls
	ret
.L50:
	sub	w1, w1, w0
	cmp	w1, w2
	cset	w0, hi
	ret
	.size	ftl_cmp_data_ver, .-ftl_cmp_data_ver
	.align	2
	.global	FtlFreeSysBlkQueueEmpty
	.type	FtlFreeSysBlkQueueEmpty, %function
FtlFreeSysBlkQueueEmpty:
	adrp	x0, .LANCHOR0+462
	ldrh	w0, [x0, #:lo12:.LANCHOR0+462]
	cmp	w0, 0
	cset	w0, eq
	ret
	.size	FtlFreeSysBlkQueueEmpty, .-FtlFreeSysBlkQueueEmpty
	.align	2
	.global	FtlFreeSysBlkQueueFull
	.type	FtlFreeSysBlkQueueFull, %function
FtlFreeSysBlkQueueFull:
	adrp	x0, .LANCHOR0+462
	ldrh	w0, [x0, #:lo12:.LANCHOR0+462]
	cmp	w0, 1024
	cset	w0, eq
	ret
	.size	FtlFreeSysBlkQueueFull, .-FtlFreeSysBlkQueueFull
	.align	2
	.global	FtlFreeSysBLkSort
	.type	FtlFreeSysBLkSort, %function
FtlFreeSysBLkSort:
	adrp	x0, .LANCHOR0
	add	x2, x0, :lo12:.LANCHOR0
	add	x1, x2, 456
	ldrh	w3, [x1, 6]
	cbz	w3, .L54
	ldrh	w5, [x2, 2540]
	mov	w6, 0
	ldrh	w3, [x1, 2]
	mov	w4, 0
	ldrh	w2, [x1, 4]
	and	w5, w5, 31
.L56:
	cmp	w5, w4
	bgt	.L57
	cbz	w6, .L54
	add	x0, x0, :lo12:.LANCHOR0
	strh	w3, [x0, 458]
	strh	w2, [x0, 460]
.L54:
	ret
.L57:
	add	x6, x1, x3, sxtw 1
	add	w4, w4, 1
	add	w3, w3, 1
	and	w4, w4, 65535
	and	w3, w3, 1023
	ldrh	w7, [x6, 8]
	add	x6, x1, x2, sxtw 1
	strh	w7, [x6, 8]
	mov	w6, 1
	add	w2, w2, w6
	and	w2, w2, 1023
	b	.L56
	.size	FtlFreeSysBLkSort, .-FtlFreeSysBLkSort
	.align	2
	.global	IsInFreeQueue
	.type	IsInFreeQueue, %function
IsInFreeQueue:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	add	x1, x1, 456
	and	w0, w0, 65535
	ldrh	w4, [x1, 6]
	cmp	w4, 1024
	beq	.L67
	ldrh	w5, [x1, 2]
	mov	w3, 0
.L65:
	cmp	w3, w4
	bcc	.L66
.L67:
	mov	w0, 0
	ret
.L66:
	add	w2, w3, w5
	ubfiz	x2, x2, 1, 10
	add	x2, x1, x2
	ldrh	w2, [x2, 8]
	cmp	w2, w0
	beq	.L68
	add	w3, w3, 1
	b	.L65
.L68:
	mov	w0, 1
	ret
	.size	IsInFreeQueue, .-IsInFreeQueue
	.align	2
	.global	insert_data_list
	.type	insert_data_list, %function
insert_data_list:
	adrp	x2, .LANCHOR0
	add	x3, x2, :lo12:.LANCHOR0
	and	w0, w0, 65535
	mov	x4, x2
	ldrh	w1, [x3, 244]
	cmp	w1, w0
	bls	.L71
	mov	w6, 6
	ldr	x8, [x3, 2560]
	mov	w1, -1
	umull	x7, w0, w6
	add	x13, x8, x7
	strh	w1, [x13, 2]
	strh	w1, [x8, x7]
	ldr	x1, [x3, 2568]
	cbnz	x1, .L72
	str	x13, [x3, 2568]
.L71:
	mov	w0, 0
	ret
.L72:
	ldr	x11, [x3, 72]
	ubfiz	x2, x0, 1, 16
	ldr	x10, [x3, 2560]
	mov	x9, -6148914691236517206
	movk	x9, 0xaaab, lsl 0
	ldrh	w14, [x3, 244]
	mov	w15, 65535
	mov	w3, 0
	ldrh	w5, [x11, x2]
	ldrh	w2, [x13, 4]
	cmp	w2, 0
	mul	w5, w5, w2
	sub	x2, x1, x10
	asr	x2, x2, 1
	csinv	w5, w5, wzr, ne
	mul	x2, x2, x9
	and	w2, w2, 65535
.L77:
	add	w3, w3, 1
	and	w3, w3, 65535
	cmp	w3, w14
	bhi	.L71
	cmp	w0, w2
	beq	.L71
	ldrh	w12, [x1, 4]
	cbz	w12, .L75
	ubfiz	x9, x2, 1, 16
	ldrh	w9, [x11, x9]
	mul	w9, w9, w12
	cmp	w9, w5
	bcs	.L75
	ldrh	w9, [x1]
	cmp	w9, w15
	bne	.L76
	strh	w2, [x13, 2]
	add	x2, x4, :lo12:.LANCHOR0
	strh	w0, [x1]
	str	x13, [x2, 2576]
	b	.L71
.L76:
	umaddl	x1, w9, w6, x10
	mov	w2, w9
	b	.L77
.L75:
	strh	w2, [x8, x7]
	ldrh	w2, [x1, 2]
	strh	w2, [x13, 2]
	add	x2, x4, :lo12:.LANCHOR0
	ldr	x3, [x2, 2568]
	cmp	x1, x3
	bne	.L78
	strh	w0, [x1, 2]
	str	x13, [x2, 2568]
	b	.L71
.L78:
	ldrh	w3, [x1, 2]
	mov	w4, 6
	ldr	x2, [x2, 2560]
	umull	x3, w3, w4
	strh	w0, [x2, x3]
	strh	w0, [x1, 2]
	b	.L71
	.size	insert_data_list, .-insert_data_list
	.align	2
	.global	INSERT_DATA_LIST
	.type	INSERT_DATA_LIST, %function
INSERT_DATA_LIST:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	insert_data_list
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	ldrh	w0, [x1, 2584]
	add	w0, w0, 1
	and	w0, w0, 65535
	strh	w0, [x1, 2584]
	ldrh	w1, [x1, 244]
	cmp	w1, w0
	bcs	.L83
	adrp	x1, .LANCHOR1
	adrp	x0, .LC8
	mov	w2, 214
	add	x1, x1, :lo12:.LANCHOR1
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L83:
	ldp	x29, x30, [sp], 16
	ret
	.size	INSERT_DATA_LIST, .-INSERT_DATA_LIST
	.align	2
	.global	insert_free_list
	.type	insert_free_list, %function
insert_free_list:
	and	w0, w0, 65535
	mov	w7, 65535
	cmp	w0, w7
	beq	.L87
	adrp	x2, .LANCHOR0
	add	x5, x2, :lo12:.LANCHOR0
	mov	w6, 6
	mov	w1, -1
	mov	x3, x2
	ldr	x9, [x5, 2560]
	umull	x8, w0, w6
	add	x4, x9, x8
	strh	w1, [x4, 2]
	strh	w1, [x9, x8]
	ldr	x1, [x5, 2592]
	cbnz	x1, .L88
	str	x4, [x5, 2592]
.L87:
	mov	w0, 0
	ret
.L88:
	ldr	x11, [x5, 2600]
	ubfiz	x2, x0, 1, 16
	ldr	x10, [x5, 2560]
	mov	x5, -6148914691236517206
	movk	x5, 0xaaab, lsl 0
	ldrh	w12, [x11, x2]
	sub	x2, x1, x10
	asr	x2, x2, 1
	mul	x2, x2, x5
	and	w2, w2, 65535
.L91:
	ubfiz	x5, x2, 1, 16
	ldrh	w5, [x11, x5]
	cmp	w5, w12
	bcs	.L89
	ldrh	w5, [x1]
	cmp	w5, w7
	bne	.L90
	strh	w2, [x4, 2]
	strh	w0, [x1]
	b	.L87
.L90:
	umaddl	x1, w5, w6, x10
	mov	w2, w5
	b	.L91
.L89:
	ldrh	w5, [x1, 2]
	strh	w5, [x4, 2]
	strh	w2, [x9, x8]
	add	x2, x3, :lo12:.LANCHOR0
	ldr	x3, [x2, 2592]
	cmp	x1, x3
	bne	.L92
	strh	w0, [x1, 2]
	str	x4, [x2, 2592]
	b	.L87
.L92:
	ldrh	w3, [x1, 2]
	mov	w4, 6
	ldr	x2, [x2, 2560]
	umull	x3, w3, w4
	strh	w0, [x2, x3]
	strh	w0, [x1, 2]
	b	.L87
	.size	insert_free_list, .-insert_free_list
	.align	2
	.global	INSERT_FREE_LIST
	.type	INSERT_FREE_LIST, %function
INSERT_FREE_LIST:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	insert_free_list
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	ldrh	w0, [x1, 228]
	add	w0, w0, 1
	and	w0, w0, 65535
	strh	w0, [x1, 228]
	ldrh	w1, [x1, 244]
	cmp	w1, w0
	bcs	.L93
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 207
	add	x1, x1, 24
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L93:
	ldp	x29, x30, [sp], 16
	ret
	.size	INSERT_FREE_LIST, .-INSERT_FREE_LIST
	.align	2
	.global	List_remove_node
	.type	List_remove_node, %function
List_remove_node:
	stp	x29, x30, [sp, -64]!
	and	w1, w1, 65535
	mov	w2, 6
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	str	x23, [sp, 48]
	mov	x23, x0
	add	x0, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	umull	x21, w1, w2
	ldr	x22, [x0, 2560]
	mov	w0, 65535
	add	x20, x22, x21
	ldrh	w1, [x20, 2]
	cmp	w1, w0
	bne	.L97
	ldr	x0, [x23]
	cmp	x20, x0
	beq	.L97
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 372
	add	x1, x1, 48
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L97:
	ldr	x0, [x23]
	mov	w1, 65535
	cmp	x20, x0
	ldrh	w0, [x22, x21]
	bne	.L98
	cmp	w0, w1
	bne	.L99
	str	xzr, [x23]
.L100:
	mov	w0, -1
	strh	w0, [x22, x21]
	strh	w0, [x20, 2]
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldr	x23, [sp, 48]
	ldp	x29, x30, [sp], 64
	ret
.L99:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w2, 6
	ldr	x1, [x19, 2560]
	umaddl	x0, w0, w2, x1
	mov	w1, -1
	str	x0, [x23]
	strh	w1, [x0, 2]
	b	.L100
.L98:
	cmp	w0, w1
	ldrh	w1, [x20, 2]
	bne	.L101
	cmp	w1, w0
	beq	.L100
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, 6
	mov	w2, -1
	umull	x1, w1, w0
	ldr	x0, [x19, 2560]
	strh	w2, [x0, x1]
	b	.L100
.L101:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w2, 6
	ldr	x3, [x19, 2560]
	umaddl	x0, w0, w2, x3
	strh	w1, [x0, 2]
	ldrh	w0, [x20, 2]
	ldr	x1, [x19, 2560]
	ldrh	w3, [x22, x21]
	umull	x0, w0, w2
	strh	w3, [x1, x0]
	b	.L100
	.size	List_remove_node, .-List_remove_node
	.align	2
	.global	List_pop_index_node
	.type	List_pop_index_node, %function
List_pop_index_node:
	ldr	x2, [x0]
	cbz	x2, .L109
	stp	x29, x30, [sp, -32]!
	adrp	x3, .LANCHOR0+2560
	and	w1, w1, 65535
	mov	w4, 65535
	add	x29, sp, 0
	str	x19, [sp, 16]
	mov	w5, 6
	ldr	x19, [x3, #:lo12:.LANCHOR0+2560]
.L105:
	cbnz	w1, .L106
.L108:
	sub	x19, x2, x19
	mov	x2, -6148914691236517206
	asr	x19, x19, 1
	movk	x2, 0xaaab, lsl 0
	mul	x19, x19, x2
	and	w19, w19, 65535
	mov	w1, w19
	bl	List_remove_node
	mov	w0, w19
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L106:
	ldrh	w3, [x2]
	cmp	w3, w4
	beq	.L108
	sub	w1, w1, #1
	umaddl	x2, w3, w5, x19
	and	w1, w1, 65535
	b	.L105
.L109:
	mov	w0, 65535
	ret
	.size	List_pop_index_node, .-List_pop_index_node
	.align	2
	.global	List_pop_head_node
	.type	List_pop_head_node, %function
List_pop_head_node:
	stp	x29, x30, [sp, -16]!
	mov	w1, 0
	add	x29, sp, 0
	bl	List_pop_index_node
	ldp	x29, x30, [sp], 16
	ret
	.size	List_pop_head_node, .-List_pop_head_node
	.align	2
	.global	List_get_gc_head_node
	.type	List_get_gc_head_node, %function
List_get_gc_head_node:
	and	w2, w0, 65535
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	ldr	x1, [x0, 2568]
	cbz	x1, .L121
	ldr	x3, [x0, 2560]
	mov	w4, 6
	mov	w0, 65535
.L118:
	cbz	w2, .L119
	ldrh	w1, [x1]
	cmp	w1, w0
	bne	.L120
	ret
.L120:
	sub	w2, w2, #1
	umaddl	x1, w1, w4, x3
	and	w2, w2, 65535
	b	.L118
.L121:
	mov	w0, 65535
	ret
.L119:
	sub	x0, x1, x3
	mov	x1, -6148914691236517206
	asr	x0, x0, 1
	movk	x1, 0xaaab, lsl 0
	mul	x0, x0, x1
	and	w0, w0, 65535
	ret
	.size	List_get_gc_head_node, .-List_get_gc_head_node
	.align	2
	.global	List_update_data_list
	.type	List_update_data_list, %function
List_update_data_list:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	and	w20, w0, 65535
	adrp	x19, .LANCHOR0
	add	x0, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	ldrh	w1, [x0, 24]
	cmp	w1, w20
	beq	.L124
	ldrh	w1, [x0, 80]
	cmp	w1, w20
	beq	.L124
	ldrh	w1, [x0, 128]
	cmp	w1, w20
	beq	.L124
	mov	w22, 6
	ldr	x24, [x0, 2560]
	ldr	x1, [x0, 2568]
	umull	x22, w20, w22
	add	x23, x24, x22
	cmp	x23, x1
	beq	.L124
	ldr	x2, [x0, 72]
	ubfiz	x1, x20, 1, 16
	ldrh	w0, [x23, 4]
	cmp	w0, 0
	ldrh	w21, [x2, x1]
	mov	w1, 65535
	mul	w21, w21, w0
	ldrh	w0, [x23, 2]
	csinv	w21, w21, wzr, ne
	cmp	w0, w1
	bne	.L127
	ldrh	w1, [x24, x22]
	cmp	w1, w0
	bne	.L127
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 463
	add	x1, x1, 72
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L127:
	ldrh	w0, [x23, 2]
	mov	w1, 65535
	cmp	w0, w1
	bne	.L128
	ldrh	w1, [x24, x22]
	cmp	w1, w0
	beq	.L124
.L128:
	mov	w1, 6
	add	x22, x19, :lo12:.LANCHOR0
	mov	x2, -6148914691236517206
	umull	x0, w0, w1
	movk	x2, 0xaaab, lsl 0
	asr	x1, x0, 1
	mul	x1, x1, x2
	ldr	x2, [x22, 72]
	ldrh	w1, [x2, x1, lsl 1]
	ldr	x2, [x22, 2560]
	add	x0, x2, x0
	ldrh	w2, [x0, 4]
	cmp	w2, 0
	mul	w0, w1, w2
	csinv	w0, w0, wzr, ne
	cmp	w21, w0
	bcs	.L124
	mov	w1, w20
	add	x0, x22, 2568
	bl	List_remove_node
	ldrh	w0, [x22, 2584]
	cbnz	w0, .L130
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 474
	add	x1, x1, 72
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L130:
	add	x19, x19, :lo12:.LANCHOR0
	ldrh	w0, [x19, 2584]
	sub	w0, w0, #1
	strh	w0, [x19, 2584]
	mov	w0, w20
	bl	INSERT_DATA_LIST
.L124:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 64
	ret
	.size	List_update_data_list, .-List_update_data_list
	.align	2
	.global	select_l2p_ram_region
	.type	select_l2p_ram_region, %function
select_l2p_ram_region:
	stp	x29, x30, [sp, -32]!
	adrp	x1, .LANCHOR0
	add	x0, x1, :lo12:.LANCHOR0
	mov	x3, 0
	add	x29, sp, 0
	str	x19, [sp, 16]
	mov	w4, 65535
	ldrh	w2, [x0, 342]
	ldr	x0, [x0, 2608]
.L135:
	and	w19, w3, 65535
	cmp	w19, w2
	bcc	.L137
	add	x4, x0, 4
	mov	w19, w2
	mov	w6, -2147483648
	mov	w3, 0
.L138:
	cmp	w3, w2
	bne	.L140
	cmp	w19, w2
	bcc	.L136
	add	x1, x1, :lo12:.LANCHOR0
	mov	w19, w2
	mov	w3, -1
	ldrh	w4, [x1, 2616]
	mov	w1, 0
.L141:
	cmp	w1, w2
	bne	.L143
	cmp	w19, w1
	bcc	.L136
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	mov	w2, 796
	add	x1, x1, 96
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L136
.L137:
	add	x3, x3, 1
	add	x5, x0, x3, lsl 4
	ldrh	w5, [x5, -16]
	cmp	w5, w4
	bne	.L135
.L136:
	mov	w0, w19
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L140:
	ldr	w5, [x4]
	tbnz	w5, #31, .L139
	cmp	w6, w5
	bls	.L139
	mov	w6, w5
	mov	w19, w3
.L139:
	add	w3, w3, 1
	add	x4, x4, 16
	and	w3, w3, 65535
	b	.L138
.L143:
	ldr	w6, [x0, 4]
	cmp	w3, w6
	bls	.L142
	ldrh	w5, [x0]
	cmp	w5, w4
	csel	w3, w3, w6, eq
	csel	w19, w19, w1, eq
.L142:
	add	w1, w1, 1
	add	x0, x0, 16
	and	w1, w1, 65535
	b	.L141
	.size	select_l2p_ram_region, .-select_l2p_ram_region
	.align	2
	.global	FtlUpdateVaildLpn
	.type	FtlUpdateVaildLpn, %function
FtlUpdateVaildLpn:
	adrp	x1, .LANCHOR0
	add	x3, x1, :lo12:.LANCHOR0
	ldrh	w2, [x3, 2618]
	cmp	w2, 4
	bhi	.L146
	cbnz	w0, .L146
	add	w2, w2, 1
	strh	w2, [x3, 2618]
	ret
.L146:
	add	x0, x1, :lo12:.LANCHOR0
	mov	w6, 65535
	mov	x1, 0
	ldrh	w4, [x0, 244]
	ldr	x5, [x0, 72]
	strh	wzr, [x0, 2618]
	str	wzr, [x0, 2620]
.L147:
	cmp	w4, w1, uxth
	bhi	.L149
	ret
.L149:
	ldrh	w3, [x5, x1, lsl 1]
	cmp	w3, w6
	beq	.L148
	ldr	w2, [x0, 2620]
	add	w2, w2, w3
	str	w2, [x0, 2620]
.L148:
	add	x1, x1, 1
	b	.L147
	.size	FtlUpdateVaildLpn, .-FtlUpdateVaildLpn
	.align	2
	.global	ftl_sb_update_avl_pages
	.type	ftl_sb_update_avl_pages, %function
ftl_sb_update_avl_pages:
	and	w6, w1, 65535
	adrp	x4, .LANCHOR0
	add	x1, x4, :lo12:.LANCHOR0
	and	w2, w2, 65535
	strh	wzr, [x0, 4]
	ldrh	w3, [x1, 236]
	mov	w1, 65535
.L152:
	cmp	w3, w2, uxth
	bhi	.L154
	add	x4, x4, :lo12:.LANCHOR0
	ubfiz	x3, x3, 1, 16
	add	x3, x3, 16
	add	x2, x0, 16
	add	x3, x0, x3
	mov	w5, 65535
	ldrh	w1, [x4, 306]
	sub	w1, w1, #1
	and	w1, w1, 65535
	sub	w1, w1, w6
.L155:
	cmp	x2, x3
	bne	.L157
	ret
.L154:
	add	x5, x0, x2, sxtw 1
	ldrh	w5, [x5, 16]
	cmp	w5, w1
	beq	.L153
	ldrh	w5, [x0, 4]
	add	w5, w5, 1
	strh	w5, [x0, 4]
.L153:
	add	w2, w2, 1
	b	.L152
.L157:
	ldrh	w4, [x2]
	cmp	w4, w5
	beq	.L156
	ldrh	w4, [x0, 4]
	add	w4, w1, w4
	strh	w4, [x0, 4]
.L156:
	add	x2, x2, 2
	b	.L155
	.size	ftl_sb_update_avl_pages, .-ftl_sb_update_avl_pages
	.align	2
	.global	FtlSlcSuperblockCheck
	.type	FtlSlcSuperblockCheck, %function
FtlSlcSuperblockCheck:
	ldrh	w1, [x0, 4]
	cbz	w1, .L158
	ldrh	w2, [x0]
	mov	w1, 65535
	cmp	w2, w1
	beq	.L158
	ldrb	w2, [x0, 6]
	add	x2, x2, 8
	ldrh	w3, [x0, x2, lsl 1]
	adrp	x2, .LANCHOR0+236
	ldrh	w4, [x2, #:lo12:.LANCHOR0+236]
	mov	w2, w1
.L161:
	cmp	w3, w2
	beq	.L163
.L158:
	ret
.L163:
	ldrb	w1, [x0, 6]
	add	w1, w1, 1
	and	w1, w1, 255
	strb	w1, [x0, 6]
	cmp	w1, w4
	bne	.L162
	ldrh	w1, [x0, 2]
	strb	wzr, [x0, 6]
	add	w1, w1, 1
	strh	w1, [x0, 2]
.L162:
	ldrb	w1, [x0, 6]
	add	x1, x1, 8
	ldrh	w3, [x0, x1, lsl 1]
	b	.L161
	.size	FtlSlcSuperblockCheck, .-FtlSlcSuperblockCheck
	.align	2
	.global	make_superblock
	.type	make_superblock, %function
make_superblock:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	adrp	x20, .LANCHOR0
	add	x0, x20, :lo12:.LANCHOR0
	ldrh	w1, [x19]
	ldrh	w0, [x0, 244]
	cmp	w1, w0
	bcc	.L165
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2157
	add	x1, x1, 120
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L165:
	add	x0, x20, :lo12:.LANCHOR0
	add	x6, x19, 16
	add	x7, x0, 264
	strh	wzr, [x19, 4]
	strb	wzr, [x19, 7]
	mov	x5, 0
	ldrh	w8, [x0, 236]
	mov	w9, -1
.L166:
	cmp	w8, w5, uxth
	bhi	.L168
	add	x20, x20, :lo12:.LANCHOR0
	ldrb	w0, [x19, 7]
	strb	wzr, [x19, 9]
	ldrh	w1, [x20, 306]
	mul	w0, w0, w1
	ldrh	w1, [x19]
	strh	w0, [x19, 4]
	ldr	x0, [x20, 2600]
	ldrh	w1, [x0, x1, lsl 1]
	mov	w0, 10000
	cmp	w1, w0
	bls	.L169
	mov	w0, 1
	strb	w0, [x19, 9]
.L169:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L168:
	ldrh	w1, [x19]
	ldrb	w0, [x7, x5]
	bl	V2P_block
	mov	w4, w0
	strh	w9, [x6]
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L167
	strh	w4, [x6]
	ldrb	w0, [x19, 7]
	add	w0, w0, 1
	strb	w0, [x19, 7]
.L167:
	add	x5, x5, 1
	add	x6, x6, 2
	b	.L166
	.size	make_superblock, .-make_superblock
	.align	2
	.global	update_multiplier_value
	.type	update_multiplier_value, %function
update_multiplier_value:
	and	w8, w0, 65535
	adrp	x0, .LANCHOR0
	add	x1, x0, :lo12:.LANCHOR0
	mov	x9, 0
	mov	w6, 0
	mov	x5, x0
	add	x7, x1, 264
	ldrh	w10, [x1, 236]
	ldrh	w11, [x1, 306]
	cmp	w10, w9, uxth
	bhi	.L183
	cbz	w6, .L181
	mov	w0, 32768
	sdiv	w6, w0, w6
.L182:
	add	x0, x5, :lo12:.LANCHOR0
	mov	w1, 6
	ldr	x0, [x0, 2560]
	umaddl	x8, w8, w1, x0
	mov	w0, 0
	strh	w6, [x8, 4]
	ret
.L176:
	mov	w6, 0
	b	.L175
.L181:
	mov	w6, 0
	b	.L182
.L183:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
.L174:
	ldrb	w0, [x7, x9]
	mov	w1, w8
	bl	V2P_block
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L173
	add	w6, w6, w11
	and	w6, w6, 65535
.L173:
	add	x9, x9, 1
	cmp	w10, w9, uxth
	bhi	.L174
	cbz	w6, .L176
	mov	w0, 32768
	sdiv	w6, w0, w6
.L175:
	add	x0, x5, :lo12:.LANCHOR0
	mov	w1, 6
	ldr	x0, [x0, 2560]
	umaddl	x8, w8, w1, x0
	mov	w0, 0
	strh	w6, [x8, 4]
	ldp	x29, x30, [sp], 16
	ret
	.size	update_multiplier_value, .-update_multiplier_value
	.align	2
	.global	GetFreeBlockMinEraseCount
	.type	GetFreeBlockMinEraseCount, %function
GetFreeBlockMinEraseCount:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	ldr	x0, [x1, 2592]
	cbz	x0, .L186
	ldr	x2, [x1, 2560]
	ldr	x1, [x1, 2600]
	sub	x0, x0, x2
	mov	x2, -6148914691236517206
	asr	x0, x0, 1
	movk	x2, 0xaaab, lsl 0
	mul	x0, x0, x2
	and	x0, x0, 65535
	ldrh	w0, [x1, x0, lsl 1]
	ret
.L186:
	mov	w0, 0
	ret
	.size	GetFreeBlockMinEraseCount, .-GetFreeBlockMinEraseCount
	.align	2
	.global	GetFreeBlockMaxEraseCount
	.type	GetFreeBlockMaxEraseCount, %function
GetFreeBlockMaxEraseCount:
	adrp	x2, .LANCHOR0
	add	x4, x2, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ldr	x1, [x4, 2592]
	cbz	x1, .L194
	ldrh	w3, [x4, 228]
	mov	w5, 7
	ldr	x4, [x4, 2560]
	mov	w6, 6
	mov	w7, 65535
	sub	x1, x1, x4
	mul	w3, w3, w5
	asr	x1, x1, 1
	asr	w3, w3, 3
	cmp	w0, w3
	csel	w0, w3, w0, gt
	mov	x3, -6148914691236517206
	movk	x3, 0xaaab, lsl 0
	mul	x1, x1, x3
	mov	w3, 0
	and	w1, w1, 65535
.L190:
	cmp	w0, w3
	beq	.L193
	umull	x5, w1, w6
	ldrh	w5, [x4, x5]
	cmp	w5, w7
	bne	.L191
.L193:
	add	x2, x2, :lo12:.LANCHOR0
	ubfiz	x1, x1, 1, 16
	ldr	x0, [x2, 2600]
	ldrh	w0, [x0, x1]
	ret
.L191:
	add	w3, w3, 1
	mov	w1, w5
	and	w3, w3, 65535
	b	.L190
.L194:
	mov	w0, 0
	ret
	.size	GetFreeBlockMaxEraseCount, .-GetFreeBlockMaxEraseCount
	.align	2
	.global	FtlPrintInfo2buf
	.type	FtlPrintInfo2buf, %function
FtlPrintInfo2buf:
	stp	x29, x30, [sp, -96]!
	adrp	x1, .LC9
	add	x1, x1, :lo12:.LC9
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	mov	x22, x0
	stp	x23, x24, [sp, 48]
	adrp	x23, .LANCHOR0
	add	x21, x23, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	stp	x25, x26, [sp, 64]
	add	x20, x0, 12
	str	x27, [sp, 80]
	bl	strcpy
	ldr	w3, [x21, 320]
	adrp	x2, .LC10
	add	x2, x2, :lo12:.LC10
	mov	x1, 64
	mov	x0, x20
	bl	snprintf
	add	x20, x20, x0, sxtw
	mov	x0, x20
	adrp	x1, .LC11
	add	x1, x1, :lo12:.LC11
	bl	strcpy
	ldr	w3, [x21, 2624]
	add	x20, x20, 10
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC12
	add	x2, x2, :lo12:.LC12
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2620]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC13
	add	x2, x2, :lo12:.LC13
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2628]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC14
	add	x2, x2, :lo12:.LC14
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2632]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC15
	add	x2, x2, :lo12:.LC15
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2636]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC16
	add	x2, x2, :lo12:.LC16
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2640]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC17
	add	x2, x2, :lo12:.LC17
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2644]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC18
	add	x2, x2, :lo12:.LC18
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2648]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC19
	add	x2, x2, :lo12:.LC19
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2652]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC20
	add	x2, x2, :lo12:.LC20
	lsr	w3, w3, 11
	bl	snprintf
	ldr	w3, [x21, 2656]
	add	x20, x20, x0, sxtw
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC21
	add	x2, x2, :lo12:.LC21
	lsr	w3, w3, 11
	bl	snprintf
	ldr	w3, [x21, 2660]
	add	x20, x20, x0, sxtw
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC22
	add	x2, x2, :lo12:.LC22
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2664]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC23
	add	x2, x2, :lo12:.LC23
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 366]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC24
	add	x2, x2, :lo12:.LC24
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 228]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC25
	add	x2, x2, :lo12:.LC25
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2668]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC26
	add	x2, x2, :lo12:.LC26
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2672]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC27
	add	x2, x2, :lo12:.LC27
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2676]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC28
	add	x2, x2, :lo12:.LC28
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2680]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC29
	add	x2, x2, :lo12:.LC29
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2684]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC30
	add	x2, x2, :lo12:.LC30
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 2688]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC31
	add	x2, x2, :lo12:.LC31
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 2542]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC32
	add	x2, x2, :lo12:.LC32
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 2540]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC33
	add	x2, x2, :lo12:.LC33
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 344]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC34
	add	x2, x2, :lo12:.LC34
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 336]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC35
	add	x2, x2, :lo12:.LC35
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 232]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC36
	add	x2, x2, :lo12:.LC36
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 462]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC37
	add	x2, x2, :lo12:.LC37
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 244]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC38
	add	x2, x2, :lo12:.LC38
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 2692]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC39
	add	x2, x2, :lo12:.LC39
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w3, [x21, 248]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC40
	add	x2, x2, :lo12:.LC40
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 2696]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC41
	add	x2, x2, :lo12:.LC41
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 360]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC42
	add	x2, x2, :lo12:.LC42
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 26]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC43
	add	x2, x2, :lo12:.LC43
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 30]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC44
	add	x2, x2, :lo12:.LC44
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 24]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC45
	add	x2, x2, :lo12:.LC45
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 32]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC46
	add	x2, x2, :lo12:.LC46
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 28]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC47
	add	x2, x2, :lo12:.LC47
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w1, [x21, 24]
	adrp	x2, .LC48
	ldr	x0, [x21, 72]
	add	x2, x2, :lo12:.LC48
	ldrh	w3, [x0, x1, lsl 1]
	mov	x1, 64
	mov	x0, x20
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 82]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC49
	add	x2, x2, :lo12:.LC49
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 86]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC50
	add	x2, x2, :lo12:.LC50
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 80]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC51
	add	x2, x2, :lo12:.LC51
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 88]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC52
	add	x2, x2, :lo12:.LC52
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 84]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC53
	add	x2, x2, :lo12:.LC53
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w1, [x21, 80]
	adrp	x2, .LC54
	ldr	x0, [x21, 72]
	add	x2, x2, :lo12:.LC54
	ldrh	w3, [x0, x1, lsl 1]
	mov	x1, 64
	mov	x0, x20
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 130]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC55
	add	x2, x2, :lo12:.LC55
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 134]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC56
	add	x2, x2, :lo12:.LC56
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 128]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC57
	add	x2, x2, :lo12:.LC57
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 136]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC58
	add	x2, x2, :lo12:.LC58
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 132]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC59
	add	x2, x2, :lo12:.LC59
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 178]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC60
	add	x2, x2, :lo12:.LC60
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 182]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC61
	add	x2, x2, :lo12:.LC61
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 176]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC62
	add	x2, x2, :lo12:.LC62
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrb	w3, [x21, 184]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC63
	add	x2, x2, :lo12:.LC63
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldrh	w3, [x21, 180]
	mov	x1, 64
	mov	x0, x20
	adrp	x2, .LC64
	add	x2, x2, :lo12:.LC64
	bl	snprintf
	add	x20, x20, x0, sxtw
	ldr	w5, [x21, 2788]
	mov	x1, 64
	ldr	w4, [x21, 2796]
	adrp	x2, .LC65
	ldr	w3, [x21, 2792]
	add	x2, x2, :lo12:.LC65
	mov	x0, x20
	bl	snprintf
	add	x19, x20, x0, sxtw
	ldr	w3, [x21, 2784]
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC66
	add	x2, x2, :lo12:.LC66
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldr	w3, [x21, 2808]
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC67
	add	x2, x2, :lo12:.LC67
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldrh	w3, [x21, 3224]
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC68
	add	x2, x2, :lo12:.LC68
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldrh	w3, [x21, 3226]
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC69
	add	x2, x2, :lo12:.LC69
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldr	w3, [x21, 3228]
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC70
	add	x2, x2, :lo12:.LC70
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldrh	w3, [x21, 3232]
	adrp	x2, .LC71
	add	x2, x2, :lo12:.LC71
	mov	x1, 64
	mov	x0, x19
	bl	snprintf
	add	x19, x19, x0, sxtw
	bl	GetFreeBlockMinEraseCount
	and	w3, w0, 65535
	mov	x1, 64
	mov	x0, x19
	adrp	x2, .LC72
	add	x2, x2, :lo12:.LC72
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldrh	w0, [x21, 228]
	bl	GetFreeBlockMaxEraseCount
	and	w3, w0, 65535
	adrp	x2, .LC73
	mov	x0, x19
	add	x2, x2, :lo12:.LC73
	mov	x1, 64
	bl	snprintf
	add	x19, x19, x0, sxtw
	adrp	x0, .LANCHOR2
	ldr	w0, [x0, #:lo12:.LANCHOR2]
	cmp	w0, 1
	beq	.L196
.L201:
	sub	w0, w19, w22
	ldr	x27, [sp, 80]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 96
	ret
.L196:
	ldrh	w0, [x21, 176]
	mov	w1, 65535
	cmp	w0, w1
	beq	.L198
	ldr	x1, [x21, 72]
	ubfiz	x0, x0, 1, 16
	adrp	x2, .LC74
	add	x2, x2, :lo12:.LC74
	ldrh	w3, [x1, x0]
	mov	x0, x19
	mov	x1, 64
	bl	snprintf
	add	x19, x19, x0, sxtw
.L198:
	mov	w0, 0
	adrp	x24, .LC75
	bl	List_get_gc_head_node
	add	x21, x23, :lo12:.LANCHOR0
	and	w4, w0, 65535
	add	x24, x24, :lo12:.LC75
	mov	w20, 0
	mov	w27, 65535
	mov	w26, 6
.L200:
	cmp	w4, w27
	beq	.L199
	umull	x25, w4, w26
	ldr	x0, [x21, 2560]
	ldr	x2, [x21, 72]
	ubfiz	x1, x4, 1, 16
	ldr	x3, [x21, 2600]
	add	x0, x0, x25
	ldrh	w5, [x2, x1]
	mov	x2, x24
	ldrh	w7, [x3, x1]
	mov	w3, w20
	ldrh	w6, [x0, 4]
	mov	x1, 64
	mov	x0, x19
	add	w20, w20, 1
	bl	snprintf
	add	x19, x19, x0, sxtw
	ldr	x0, [x21, 2560]
	cmp	w20, 16
	ldrh	w4, [x0, x25]
	bne	.L200
.L199:
	add	x23, x23, :lo12:.LANCHOR0
	adrp	x21, .LC76
	add	x21, x21, :lo12:.LC76
	mov	w20, 0
	mov	w25, 65535
	mov	w26, 6
	ldr	x0, [x23, 2560]
	ldr	x4, [x23, 2592]
	sub	x4, x4, x0
	mov	x0, -6148914691236517206
	asr	x4, x4, 1
	movk	x0, 0xaaab, lsl 0
	mul	x4, x4, x0
	and	w4, w4, 65535
.L202:
	cmp	w4, w25
	beq	.L201
	umull	x24, w4, w26
	ldr	x0, [x23, 2560]
	ldr	x2, [x23, 2600]
	ubfiz	x1, x4, 1, 16
	add	x0, x0, x24
	mov	w3, w20
	add	w20, w20, 1
	ldrh	w6, [x2, x1]
	mov	x2, x21
	ldrh	w5, [x0, 4]
	mov	x1, 64
	mov	x0, x19
	bl	snprintf
	add	x19, x19, x0, sxtw
	cmp	w20, 4
	beq	.L201
	ldr	x0, [x23, 2560]
	ldrh	w4, [x0, x24]
	b	.L202
	.size	FtlPrintInfo2buf, .-FtlPrintInfo2buf
	.align	2
	.global	rknand_proc_ftlread
	.type	rknand_proc_ftlread, %function
rknand_proc_ftlread:
	cmp	w0, 2047
	ble	.L210
	stp	x29, x30, [sp, -32]!
	adrp	x3, .LC0
	adrp	x2, .LC77
	add	x3, x3, :lo12:.LC0
	add	x29, sp, 0
	add	x2, x2, :lo12:.LC77
	stp	x19, x20, [sp, 16]
	mov	x20, x1
	mov	x1, 64
	mov	x0, x20
	bl	snprintf
	add	x19, x20, x0, sxtw
	mov	x0, x19
	bl	FtlPrintInfo2buf
	add	x0, x19, x0, sxtw
	sub	w0, w0, w20
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L210:
	mov	w0, 0
	ret
	.size	rknand_proc_ftlread, .-rknand_proc_ftlread
	.align	2
	.global	GetSwlReplaceBlock
	.type	GetSwlReplaceBlock, %function
GetSwlReplaceBlock:
	stp	x29, x30, [sp, -32]!
	adrp	x8, .LANCHOR0
	add	x0, x8, :lo12:.LANCHOR0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	ldr	w2, [x0, 2676]
	ldr	w1, [x0, 2688]
	cmp	w2, w1
	bcs	.L216
	ldrh	w2, [x0, 244]
	mov	x1, 0
	ldr	x4, [x0, 2600]
	str	wzr, [x0, 2668]
.L217:
	cmp	w2, w1
	bhi	.L218
	add	x1, x8, :lo12:.LANCHOR0
	ldr	w0, [x1, 2668]
	udiv	w2, w0, w2
	str	w2, [x1, 2676]
	ldr	w2, [x1, 2672]
	sub	w0, w0, w2
	ldrh	w2, [x1, 296]
	udiv	w0, w0, w2
	str	w0, [x1, 2668]
.L219:
	add	x1, x8, :lo12:.LANCHOR0
	ldr	w9, [x1, 2688]
	ldr	w11, [x1, 2676]
	add	w0, w9, 256
	cmp	w0, w11
	bls	.L224
	ldr	w1, [x1, 2684]
	add	w0, w9, 768
	cmp	w0, w1
	bls	.L224
.L226:
	mov	w20, 65535
.L225:
	mov	w0, w20
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L218:
	ldrh	w3, [x4, x1, lsl 1]
	add	x1, x1, 1
	ldr	w5, [x0, 2668]
	add	w3, w3, w5
	str	w3, [x0, 2668]
	b	.L217
.L216:
	ldr	w1, [x0, 2684]
	cmp	w2, w1
	bls	.L219
	add	w1, w1, 1
	str	w1, [x0, 2684]
	mov	w1, 0
.L221:
	ldrh	w2, [x0, 244]
	cmp	w1, w2
	bcs	.L219
	ldr	x4, [x0, 2600]
	ubfiz	x3, x1, 1, 32
	add	w1, w1, 1
	ldrh	w2, [x4, x3]
	add	w2, w2, 1
	strh	w2, [x4, x3]
	b	.L221
.L224:
	add	x10, x8, :lo12:.LANCHOR0
	ldrh	w0, [x10, 228]
	add	w0, w0, w0, lsl 1
	lsr	w0, w0, 2
	bl	GetFreeBlockMaxEraseCount
	and	w6, w0, 65535
	add	w0, w9, 64
	cmp	w6, w0
	bcc	.L226
	ldr	x0, [x10, 2568]
	cbz	x0, .L226
	mov	w4, 65535
	ldrh	w12, [x10, 244]
	mov	x13, -6148914691236517206
	ldr	x7, [x10, 2560]
	ldr	x5, [x10, 2600]
	mov	w2, w4
	mov	w10, w4
	mov	w1, 0
	movk	x13, 0xaaab, lsl 0
	mov	w14, 6
.L227:
	ldrh	w3, [x0]
	cmp	w3, w10
	bne	.L230
	mov	w20, w2
.L229:
	mov	w0, 65535
	cmp	w20, w0
	beq	.L226
	ubfiz	x7, x20, 1, 32
	ldrh	w3, [x5, x7]
	cmp	w9, w3
	bcs	.L231
	bl	GetFreeBlockMinEraseCount
	cmp	w9, w0, uxth
	bcs	.L231
	add	x0, x8, :lo12:.LANCHOR0
	str	w4, [x0, 2688]
.L231:
	cmp	w11, w3
	bls	.L226
	add	w0, w3, 128
	cmp	w6, w0
	ble	.L226
	add	w0, w3, 256
	cmp	w11, w0
	bhi	.L232
	add	x0, x8, :lo12:.LANCHOR0
	add	w3, w3, 768
	ldr	w0, [x0, 2684]
	cmp	w3, w0
	bcs	.L226
.L232:
	add	x19, x8, :lo12:.LANCHOR0
	ldrh	w5, [x5, x7]
	mov	w2, w11
	mov	w1, w20
	ldr	x0, [x19, 72]
	ldr	w3, [x19, 2684]
	ldrh	w4, [x0, x7]
	adrp	x0, .LC78
	add	x0, x0, :lo12:.LC78
	bl	sftl_printk
	mov	w0, 1
	str	w0, [x19, 3236]
	b	.L225
.L230:
	add	w1, w1, 1
	and	w1, w1, 65535
	cmp	w1, w12
	bhi	.L226
	ldrh	w15, [x0, 4]
	cbz	w15, .L228
	sub	x0, x0, x7
	asr	x0, x0, 1
	mul	x0, x0, x13
	and	w20, w0, 65535
	and	x0, x0, 65535
	ldrh	w0, [x5, x0, lsl 1]
	cmp	w9, w0
	bcs	.L229
	cmp	w4, w0
	bls	.L228
	mov	w4, w0
	mov	w2, w20
.L228:
	umaddl	x0, w3, w14, x7
	b	.L227
	.size	GetSwlReplaceBlock, .-GetSwlReplaceBlock
	.align	2
	.global	free_data_superblock
	.type	free_data_superblock, %function
free_data_superblock:
	and	w0, w0, 65535
	mov	w1, 65535
	cmp	w0, w1
	beq	.L243
	stp	x29, x30, [sp, -16]!
	adrp	x2, .LANCHOR0+72
	ubfiz	x1, x0, 1, 16
	add	x29, sp, 0
	ldr	x2, [x2, #:lo12:.LANCHOR0+72]
	strh	wzr, [x2, x1]
	bl	INSERT_FREE_LIST
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
.L243:
	mov	w0, 0
	ret
	.size	free_data_superblock, .-free_data_superblock
	.align	2
	.global	get_new_active_ppa
	.type	get_new_active_ppa, %function
get_new_active_ppa:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	ldrh	w1, [x0]
	mov	w0, 65535
	cmp	w1, w0
	bne	.L247
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2792
	add	x1, x1, 136
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L247:
	adrp	x20, .LANCHOR0
	add	x0, x20, :lo12:.LANCHOR0
	ldrh	w1, [x19, 2]
	ldrh	w0, [x0, 306]
	cmp	w1, w0
	bne	.L248
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2793
	add	x1, x1, 136
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L248:
	ldrh	w0, [x19, 4]
	cbnz	w0, .L249
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2794
	add	x1, x1, 136
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L249:
	ldrb	w0, [x19, 6]
	add	x1, x20, :lo12:.LANCHOR0
	strb	wzr, [x19, 10]
	add	x0, x0, 8
	ldrh	w3, [x1, 236]
	mov	w1, 65535
	ldrh	w0, [x19, x0, lsl 1]
.L250:
	cmp	w0, w1
	beq	.L252
	add	x20, x20, :lo12:.LANCHOR0
	ldrh	w1, [x19, 2]
	ldrh	w2, [x20, 306]
	cmp	w1, w2
	bcs	.L256
	orr	w20, w1, w0, lsl 10
	ldrh	w1, [x19, 4]
	ldrb	w0, [x19, 6]
	mov	w4, 65535
	sub	w1, w1, #1
	and	w1, w1, 65535
	strh	w1, [x19, 4]
.L255:
	add	w0, w0, 1
	and	w0, w0, 255
	cmp	w3, w0
	bne	.L254
	ldrh	w0, [x19, 2]
	add	w0, w0, 1
	strh	w0, [x19, 2]
	mov	w0, 0
.L254:
	add	x5, x19, x0, sxtw 1
	ldrh	w5, [x5, 16]
	cmp	w5, w4
	beq	.L255
	strb	w0, [x19, 6]
	ldrh	w0, [x19, 2]
	cmp	w0, w2
	bne	.L246
	cbz	w1, .L246
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2817
	add	x1, x1, 136
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L246:
	mov	w0, w20
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L252:
	ldrb	w0, [x19, 6]
	add	w0, w0, 1
	and	w0, w0, 255
	strb	w0, [x19, 6]
	cmp	w0, w3
	bne	.L251
	ldrh	w0, [x19, 2]
	strb	wzr, [x19, 6]
	add	w0, w0, 1
	strh	w0, [x19, 2]
.L251:
	ldrb	w0, [x19, 6]
	add	x0, x0, 8
	ldrh	w0, [x19, x0, lsl 1]
	b	.L250
.L256:
	mov	w20, 65535
	b	.L246
	.size	get_new_active_ppa, .-get_new_active_ppa
	.align	2
	.global	FtlGcBufInit
	.type	FtlGcBufInit, %function
FtlGcBufInit:
	adrp	x0, .LANCHOR0
	add	x1, x0, :lo12:.LANCHOR0
	mov	w3, 0
	mov	w8, 24
	mov	w9, 1
	mov	w5, 4
	str	wzr, [x1, 3240]
.L263:
	ldrh	w2, [x1, 236]
	cmp	w3, w2
	bcc	.L264
	add	x0, x0, :lo12:.LANCHOR0
	mov	w6, 24
	mov	w5, 4
.L265:
	ldr	w1, [x0, 3280]
	cmp	w2, w1
	bcc	.L266
	ret
.L264:
	umull	x4, w3, w8
	ldr	x6, [x1, 3248]
	ldr	x7, [x1, 3256]
	add	x2, x6, x4
	str	w9, [x2, 16]
	ldrh	w2, [x1, 314]
	mul	w2, w2, w3
	sdiv	w2, w2, w5
	add	x2, x7, x2, sxtw 2
	str	x2, [x6, x4]
	ldrh	w2, [x1, 316]
	ldr	x7, [x1, 3248]
	ldr	x10, [x1, 3264]
	add	x6, x7, x4
	mul	w2, w2, w3
	sdiv	w2, w2, w5
	add	x2, x10, x2, sxtw 2
	str	x2, [x6, 8]
	ldr	x2, [x1, 3272]
	ubfiz	x10, x3, 5, 16
	ldr	x4, [x7, x4]
	add	w3, w3, 1
	add	x2, x2, x10
	and	w3, w3, 65535
	str	x4, [x2, 8]
	ldr	x4, [x6, 8]
	str	x4, [x2, 16]
	b	.L263
.L266:
	umull	x4, w2, w6
	ldr	x3, [x0, 3248]
	ldr	x7, [x0, 3256]
	add	x1, x3, x4
	str	wzr, [x1, 16]
	ldrh	w1, [x0, 314]
	mul	w1, w1, w2
	sdiv	w1, w1, w5
	add	x1, x7, x1, sxtw 2
	str	x1, [x3, x4]
	ldrh	w1, [x0, 316]
	ldr	x3, [x0, 3248]
	add	x3, x3, x4
	ldr	x4, [x0, 3264]
	mul	w1, w1, w2
	add	w2, w2, 1
	and	w2, w2, 65535
	sdiv	w1, w1, w5
	add	x1, x4, x1, sxtw 2
	str	x1, [x3, 8]
	b	.L265
	.size	FtlGcBufInit, .-FtlGcBufInit
	.align	2
	.global	FtlGcBufFree
	.type	FtlGcBufFree, %function
FtlGcBufFree:
	adrp	x2, .LANCHOR0
	add	x2, x2, :lo12:.LANCHOR0
	mov	w3, 0
	mov	w8, 24
	ldr	w7, [x2, 3280]
	ldr	x5, [x2, 3248]
.L268:
	cmp	w3, w1
	bcs	.L267
	ubfiz	x4, x3, 5, 16
	mov	w2, 0
	add	x4, x0, x4
	b	.L273
.L269:
	add	w2, w2, 1
	and	w2, w2, 65535
.L273:
	cmp	w2, w7
	bcs	.L270
	umull	x6, w2, w8
	add	x9, x5, x6
	ldr	x10, [x5, x6]
	ldr	x6, [x4, 8]
	cmp	x10, x6
	bne	.L269
	str	wzr, [x9, 16]
.L270:
	add	w3, w3, 1
	and	w3, w3, 65535
	b	.L268
.L267:
	ret
	.size	FtlGcBufFree, .-FtlGcBufFree
	.align	2
	.global	FtlGcBufAlloc
	.type	FtlGcBufAlloc, %function
FtlGcBufAlloc:
	adrp	x2, .LANCHOR0
	add	x2, x2, :lo12:.LANCHOR0
	mov	w3, 0
	mov	w7, 24
	mov	w8, 1
	ldr	w5, [x2, 3280]
	ldr	x6, [x2, 3248]
.L275:
	cmp	w3, w1
	bcs	.L274
	mov	w2, 0
	b	.L280
.L276:
	add	w2, w2, 1
	and	w2, w2, 65535
.L280:
	cmp	w2, w5
	bcs	.L277
	umaddl	x4, w2, w7, x6
	ldr	w9, [x4, 16]
	cbnz	w9, .L276
	ubfiz	x2, x3, 5, 16
	ldr	x9, [x4]
	add	x2, x0, x2
	str	w8, [x4, 16]
	str	x9, [x2, 8]
	ldr	x4, [x4, 8]
	str	x4, [x2, 16]
.L277:
	add	w3, w3, 1
	and	w3, w3, 65535
	b	.L275
.L274:
	ret
	.size	FtlGcBufAlloc, .-FtlGcBufAlloc
	.align	2
	.global	IsBlkInGcList
	.type	IsBlkInGcList, %function
IsBlkInGcList:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ldrh	w2, [x1, 3284]
	ldr	x3, [x1, 3288]
	mov	x1, 0
.L282:
	cmp	w2, w1, uxth
	bhi	.L284
	mov	w0, 0
	ret
.L284:
	add	x1, x1, 1
	add	x4, x3, x1, lsl 1
	ldrh	w4, [x4, -2]
	cmp	w4, w0
	bne	.L282
	mov	w0, 1
	ret
	.size	IsBlkInGcList, .-IsBlkInGcList
	.align	2
	.global	FtlGcUpdatePage
	.type	FtlGcUpdatePage, %function
FtlGcUpdatePage:
	mov	w5, w0
	mov	w8, w1
	mov	w7, w2
	stp	x29, x30, [sp, -16]!
	lsr	w0, w0, 10
	add	x29, sp, 0
	bl	P2V_block_in_plane
	and	w9, w0, 65535
	adrp	x3, .LANCHOR0
	add	x4, x3, :lo12:.LANCHOR0
	ldrh	w1, [x4, 3284]
	ldr	x6, [x4, 3288]
	mov	x4, 0
.L287:
	and	w2, w4, 65535
	cmp	w2, w1
	bcc	.L289
	bne	.L288
	and	x4, x4, 65535
	strh	w0, [x6, x4, lsl 1]
	add	x4, x3, :lo12:.LANCHOR0
	ldrh	w0, [x4, 3284]
	add	w0, w0, 1
	strh	w0, [x4, 3284]
	b	.L288
.L289:
	add	x4, x4, 1
	add	x2, x6, x4, lsl 1
	ldrh	w2, [x2, -2]
	cmp	w2, w9
	bne	.L287
.L288:
	add	x0, x3, :lo12:.LANCHOR0
	mov	w1, 12
	ldrh	w3, [x0, 3296]
	ldr	x4, [x0, 3304]
	umull	x3, w3, w1
	add	x4, x4, x3
	str	w8, [x4, 4]
	ldr	x1, [x0, 3304]
	add	x4, x1, x3
	str	w7, [x4, 8]
	str	w5, [x1, x3]
	ldrh	w1, [x0, 3296]
	add	w1, w1, 1
	strh	w1, [x0, 3296]
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlGcUpdatePage, .-FtlGcUpdatePage
	.align	2
	.global	FtlGcRefreshBlock
	.type	FtlGcRefreshBlock, %function
FtlGcRefreshBlock:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	str	x19, [sp, 16]
	and	w19, w0, 65535
	mov	w1, w19
	adrp	x0, .LC79
	add	x0, x0, :lo12:.LC79
	bl	sftl_printk
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	ldrh	w3, [x0, 226]
	cmp	w19, w3
	beq	.L292
	ldrh	w2, [x0, 224]
	cmp	w19, w2
	beq	.L292
	mov	w1, 65535
	cmp	w3, w1
	bne	.L293
	strh	w19, [x0, 226]
.L292:
	mov	w0, 0
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L293:
	cmp	w2, w1
	bne	.L292
	strh	w19, [x0, 224]
	b	.L292
	.size	FtlGcRefreshBlock, .-FtlGcRefreshBlock
	.align	2
	.global	FtlGcMarkBadPhyBlk
	.type	FtlGcMarkBadPhyBlk, %function
FtlGcMarkBadPhyBlk:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x20, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	and	w21, w0, 65535
	add	x20, x20, 3320
	mov	w0, w21
	bl	P2V_block_in_plane
	ldrh	w1, [x20, -8]
	mov	w2, w21
	mov	w22, w0
	adrp	x0, .LC80
	add	x0, x0, :lo12:.LC80
	bl	sftl_printk
	mov	w0, w22
	bl	FtlGcRefreshBlock
	ldrh	w0, [x20, -8]
	mov	x1, 0
.L296:
	cmp	w0, w1, uxth
	bhi	.L298
	cmp	w0, 15
	bhi	.L297
	add	x19, x19, :lo12:.LANCHOR0
	add	w1, w0, 1
	add	x19, x19, 3320
	strh	w1, [x19, -8]
	strh	w21, [x19, w0, sxtw 1]
	b	.L297
.L298:
	add	x1, x1, 1
	add	x2, x20, x1, lsl 1
	ldrh	w2, [x2, -2]
	cmp	w2, w21
	bne	.L296
.L297:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
	.size	FtlGcMarkBadPhyBlk, .-FtlGcMarkBadPhyBlk
	.align	2
	.global	FtlGcReFreshBadBlk
	.type	FtlGcReFreshBadBlk, %function
FtlGcReFreshBadBlk:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 3312]
	cbz	w1, .L301
	ldrh	w3, [x0, 226]
	mov	w2, 65535
	cmp	w3, w2
	bne	.L301
	ldrh	w2, [x0, 3354]
	cmp	w2, w1
	bcc	.L302
	strh	wzr, [x0, 3354]
.L302:
	add	x19, x19, :lo12:.LANCHOR0
	add	x0, x19, 3320
	ldrh	w1, [x19, 3354]
	ldrh	w0, [x0, x1, lsl 1]
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
	ldrh	w0, [x19, 3354]
	add	w0, w0, 1
	strh	w0, [x19, 3354]
.L301:
	mov	w0, 0
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlGcReFreshBadBlk, .-FtlGcReFreshBadBlk
	.align	2
	.global	ftl_malloc
	.type	ftl_malloc, %function
ftl_malloc:
	stp	x29, x30, [sp, -16]!
	mov	w1, 193
	movk	w1, 0x60, lsl 16
	sxtw	x0, w0
	add	x29, sp, 0
	bl	__kmalloc
	ldp	x29, x30, [sp], 16
	ret
	.size	ftl_malloc, .-ftl_malloc
	.align	2
	.global	ftl_free
	.type	ftl_free, %function
ftl_free:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	kfree
	ldp	x29, x30, [sp], 16
	ret
	.size	ftl_free, .-ftl_free
	.align	2
	.global	rknand_print_hex
	.type	rknand_print_hex, %function
rknand_print_hex:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	mov	x21, x1
	stp	x23, x24, [sp, 48]
	adrp	x24, .LC81
	stp	x25, x26, [sp, 64]
	mov	w23, w2
	stp	x27, x28, [sp, 80]
	adrp	x25, .LC84
	mov	x27, x0
	mov	w26, w3
	add	x24, x24, :lo12:.LC81
	add	x25, x25, :lo12:.LC84
	stp	x19, x20, [sp, 16]
	mov	w22, 0
	mov	x19, 0
	mov	w20, 0
	adrp	x28, .LC83
.L312:
	cmp	w26, w19
	bhi	.L318
	adrp	x0, .LC85
	add	x0, x0, :lo12:.LC85
	bl	sftl_printk
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 96
	ret
.L318:
	cbnz	w20, .L313
	mov	w3, w22
	mov	x2, x21
	mov	x1, x27
	mov	x0, x24
	bl	sftl_printk
.L313:
	cmp	w23, 4
	bne	.L314
	ldr	w1, [x21, x19, lsl 2]
	adrp	x0, .LC82
	add	x0, x0, :lo12:.LC82
.L320:
	bl	sftl_printk
	add	w20, w20, 1
	cmp	w20, 15
	bls	.L317
	mov	w20, 0
	adrp	x0, .LC85
	add	x0, x0, :lo12:.LC85
	bl	sftl_printk
.L317:
	add	x19, x19, 1
	add	w22, w22, w23
	b	.L312
.L314:
	cmp	w23, 2
	bne	.L316
	ldrh	w1, [x21, x19, lsl 1]
	add	x0, x28, :lo12:.LC83
	b	.L320
.L316:
	ldrb	w1, [x21, x19]
	mov	x0, x25
	b	.L320
	.size	rknand_print_hex, .-rknand_print_hex
	.align	2
	.global	FlashEraseBlocks
	.type	FlashEraseBlocks, %function
FlashEraseBlocks:
	stp	x29, x30, [sp, -112]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	ubfiz	x23, x2, 5, 32
	stp	x25, x26, [sp, 64]
	mov	x20, x0
	str	x27, [sp, 80]
	adrp	x21, __stack_chk_guard
	add	x24, x23, 4
	add	x0, x21, :lo12:__stack_chk_guard
	ldrh	w25, [x19, 12]
	add	x22, x20, 4
	add	x24, x20, x24
	ldr	x1, [x0]
	str	x1, [x29, 104]
	mov	x1,0
	mov	w26, -1
	lsl	w27, w25, 3
.L322:
	cmp	x22, x24
	beq	.L326
	ldr	w0, [x22]
	add	x2, x29, 96
	add	x1, x29, 100
	bl	l2p_addr_tran.isra.0
	ldr	w0, [x29, 96]
	cbnz	w0, .L323
	ldr	w1, [x29, 100]
	cmp	w27, w1
	bls	.L323
	mov	x19, x20
	add	x23, x20, x23
	adrp	x20, .LANCHOR1
	add	x20, x20, :lo12:.LANCHOR1
	adrp	x22, .LC86
	add	x20, x20, 160
	add	x22, x22, :lo12:.LC86
.L324:
	cmp	x19, x23
	bne	.L325
	bl	dump_stack
.L326:
	add	x21, x21, :lo12:__stack_chk_guard
	mov	w0, 0
	ldr	x2, [x29, 104]
	ldr	x1, [x21]
	eor	x1, x2, x1
	cbz	x1, .L333
	bl	__stack_chk_fail
.L325:
	ldr	w2, [x29, 100]
	mov	w0, -1
	str	w0, [x19]
	mov	x1, x20
	mov	x0, x22
	add	x19, x19, 32
	bl	sftl_printk
	ldr	x1, [x19, -24]
	mov	w3, 16
	mov	w2, 4
	adrp	x0, .LC87
	add	x0, x0, :lo12:.LC87
	bl	rknand_print_hex
	ldr	x1, [x19, -16]
	mov	w3, 4
	adrp	x0, .LC88
	mov	w2, w3
	add	x0, x0, :lo12:.LC88
	bl	rknand_print_hex
	b	.L324
.L323:
	ldr	w1, [x29, 100]
	ldr	x2, [x19, 3368]
	blr	x2
	cmp	w0, 0
	csel	w0, w26, wzr, ne
	str	w0, [x22, -4]
	ldrh	w0, [x19, 14]
	cmp	w0, 4
	bne	.L330
	ldrb	w0, [x29, 96]
	ldr	x2, [x19, 3368]
	ldr	w1, [x29, 100]
	add	w1, w25, w1
	blr	x2
	cbz	w0, .L330
	str	w26, [x22, -4]
.L330:
	add	x22, x22, 32
	b	.L322
.L333:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldr	x27, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
	.size	FlashEraseBlocks, .-FlashEraseBlocks
	.align	2
	.global	FtlFreeSysBlkQueueIn
	.type	FtlFreeSysBlkQueueIn, %function
FtlFreeSysBlkQueueIn:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	and	w21, w0, 65535
	stp	x19, x20, [sp, 16]
	sub	w2, w21, #1
	mov	w0, 65533
	cmp	w0, w2, uxth
	bcc	.L338
	adrp	x0, .LANCHOR0
	add	x20, x0, :lo12:.LANCHOR0
	mov	x19, x0
	ldrh	w2, [x20, 462]
	cmp	w2, 1024
	beq	.L338
	and	w1, w1, 65535
	cbz	w1, .L340
	mov	w0, w21
	bl	P2V_block_in_plane
	and	w22, w0, 65535
	ldr	x0, [x20, 3408]
	lsl	w1, w21, 10
	mov	w2, 1
	str	w1, [x0, 4]
	mov	w1, w2
	ldr	x0, [x20, 3408]
	bl	FlashEraseBlocks
	ldr	x2, [x20, 2600]
	ubfiz	x0, x22, 1, 16
	ldrh	w1, [x2, x0]
	add	w1, w1, 1
	strh	w1, [x2, x0]
	ldr	w0, [x20, 2680]
	add	w0, w0, 1
	str	w0, [x20, 2680]
.L340:
	add	x0, x19, :lo12:.LANCHOR0
	add	x0, x0, 456
	ldrh	w1, [x0, 6]
	add	w1, w1, 1
	strh	w1, [x0, 6]
	ldrh	w1, [x0, 4]
	add	x2, x0, x1, sxtw 1
	add	w1, w1, 1
	and	w1, w1, 1023
	strh	w1, [x0, 4]
	strh	w21, [x2, 8]
.L338:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
	.size	FtlFreeSysBlkQueueIn, .-FtlFreeSysBlkQueueIn
	.align	2
	.global	FtlFreeSysBlkQueueOut
	.type	FtlFreeSysBlkQueueOut, %function
FtlFreeSysBlkQueueOut:
	stp	x29, x30, [sp, -48]!
	adrp	x0, .LANCHOR0
	add	x29, sp, 0
	str	x21, [sp, 32]
	add	x21, x0, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	ldrh	w2, [x21, 462]
	cbz	w2, .L352
	ldrh	w0, [x21, 458]
	add	x1, x21, 456
	sub	w2, w2, #1
	strh	w2, [x21, 462]
	mov	w2, 1
	add	x3, x1, x0, sxtw 1
	add	w0, w0, 1
	and	w0, w0, 1023
	strh	w0, [x21, 458]
	ldr	x0, [x21, 3408]
	ldrh	w20, [x3, 8]
	lsl	w1, w20, 10
	str	w1, [x0, 4]
	mov	w1, w2
	ldr	x0, [x21, 3408]
	bl	FlashEraseBlocks
	ldr	w0, [x21, 2680]
	add	w0, w0, 1
	str	w0, [x21, 2680]
.L349:
	sub	w0, w20, #1
	mov	w1, 65533
	cmp	w1, w0, uxth
	bcs	.L350
	add	x0, x19, :lo12:.LANCHOR0
	mov	w1, w20
	ldrh	w2, [x0, 462]
	adrp	x0, .LC89
	add	x0, x0, :lo12:.LC89
	bl	sftl_printk
.L351:
	b	.L351
.L352:
	mov	w20, 65535
	b	.L349
.L350:
	mov	w0, w20
	ldr	x21, [sp, 32]
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 48
	ret
	.size	FtlFreeSysBlkQueueOut, .-FtlFreeSysBlkQueueOut
	.align	2
	.global	ftl_map_blk_alloc_new_blk
	.type	ftl_map_blk_alloc_new_blk, %function
ftl_map_blk_alloc_new_blk:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	str	x21, [sp, 32]
	mov	w20, 0
	ldrh	w1, [x0, 10]
	ldr	x0, [x0, 16]
.L355:
	cmp	w20, w1
	beq	.L359
	mov	x21, x0
	ldrh	w2, [x0], 2
	cbnz	w2, .L356
	bl	FtlFreeSysBlkQueueOut
	and	w1, w0, 65535
	strh	w0, [x21]
	sub	w2, w1, #1
	mov	w0, 65533
	cmp	w0, w2, uxth
	bcs	.L357
	adrp	x0, .LANCHOR0+462
	ldrh	w2, [x0, #:lo12:.LANCHOR0+462]
	adrp	x0, .LC90
	add	x0, x0, :lo12:.LC90
	bl	sftl_printk
.L358:
	b	.L358
.L357:
	ldr	w0, [x19, 48]
	strh	wzr, [x19, 2]
	add	w0, w0, 1
	str	w0, [x19, 48]
	ldrh	w0, [x19, 8]
	strh	w20, [x19]
	add	w0, w0, 1
	strh	w0, [x19, 8]
.L359:
	ldrh	w0, [x19, 10]
	cmp	w0, w20
	bhi	.L361
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 578
	add	x1, x1, 184
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L361:
	mov	w0, 0
	ldr	x21, [sp, 32]
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 48
	ret
.L356:
	add	w20, w20, 1
	and	w20, w20, 65535
	b	.L355
	.size	ftl_map_blk_alloc_new_blk, .-ftl_map_blk_alloc_new_blk
	.align	2
	.global	ftl_memset
	.type	ftl_memset, %function
ftl_memset:
	stp	x29, x30, [sp, -16]!
	uxtw	x2, w2
	add	x29, sp, 0
	bl	memset
	ldp	x29, x30, [sp], 16
	ret
	.size	ftl_memset, .-ftl_memset
	.align	2
	.global	FtlMemInit
	.type	FtlMemInit, %function
FtlMemInit:
	stp	x29, x30, [sp, -48]!
	mov	w0, 65535
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	add	x19, x20, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x1, x19, 2560
	add	x2, x19, 2560
	add	x3, x19, 3328
	strh	wzr, [x19, 3232]
	strh	wzr, [x19, 3312]
	stp	wzr, wzr, [x1, 68]
	stp	wzr, wzr, [x1, 76]
	str	wzr, [x19, 2644]
	str	wzr, [x19, 2648]
	stp	wzr, wzr, [x2, 100]
	stp	wzr, wzr, [x2, 108]
	str	wzr, [x19, 2680]
	str	wzr, [x19, 2684]
	str	wzr, [x19, 2688]
	str	wzr, [x19, 3228]
	str	wzr, [x19, 3236]
	stp	wzr, w0, [x3, 88]
	mov	w0, -1
	stp	wzr, wzr, [x3, 96]
	strh	w0, [x19, 226]
	strh	w0, [x19, 224]
	mov	w0, 32
	strh	w0, [x19, 3224]
	mov	w0, 128
	strh	w0, [x19, 3226]
	strh	wzr, [x19, 3354]
	ldrh	w0, [x19, 310]
	lsl	w0, w0, 1
	bl	ftl_malloc
	ldrh	w1, [x19, 310]
	str	x0, [x19, 3288]
	mov	w0, 12
	mul	w0, w1, w0
	bl	ftl_malloc
	ldrh	w21, [x19, 236]
	str	x0, [x19, 3304]
	lsl	w22, w21, 5
	lsl	w21, w21, 7
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3432]
	mov	w0, w22
	bl	ftl_malloc
	str	x0, [x19, 3440]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3448]
	mov	w0, w22
	bl	ftl_malloc
	str	x0, [x19, 3408]
	mov	w0, w22
	bl	ftl_malloc
	str	x0, [x19, 3272]
	ldrh	w0, [x19, 236]
	ldrh	w21, [x19, 314]
	lsl	w0, w0, 1
	add	w0, w0, 1
	str	w0, [x19, 3280]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3456]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3464]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3472]
	ldr	w0, [x19, 3280]
	mul	w0, w21, w0
	bl	ftl_malloc
	str	x0, [x19, 3256]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3480]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3488]
	ldr	w1, [x19, 3280]
	mov	w0, 24
	mul	w0, w1, w0
	bl	ftl_malloc
	str	x0, [x19, 3248]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3496]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3504]
	ldrh	w0, [x19, 262]
	lsl	w0, w0, 2
	bl	ftl_malloc
	ldrh	w21, [x19, 316]
	str	x0, [x19, 3512]
	ldrh	w0, [x19, 236]
	mul	w21, w21, w0
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3520]
	lsl	w0, w21, 2
	bl	ftl_malloc
	str	x0, [x19, 3528]
	ldrh	w1, [x19, 316]
	ldr	w0, [x19, 3280]
	mul	w0, w1, w0
	bl	ftl_malloc
	str	x0, [x19, 3264]
	ldrh	w0, [x19, 246]
	ubfiz	w0, w0, 1, 15
	strh	w0, [x19, 3536]
	and	w0, w0, 65534
	bl	ftl_malloc
	str	x0, [x19, 3544]
	ldrh	w0, [x19, 3536]
	add	x0, x0, 547
	lsr	x0, x0, 9
	strh	w0, [x19, 3536]
	lsl	w0, w0, 9
	bl	ftl_malloc
	ldrh	w21, [x19, 246]
	str	x0, [x19, 3552]
	add	x0, x0, 32
	str	x0, [x19, 2600]
	lsl	w21, w21, 1
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3560]
	mov	w0, w21
	bl	ftl_malloc
	ldr	w21, [x19, 332]
	str	x0, [x19, 72]
	lsl	w21, w21, 1
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3568]
	mov	w0, w21
	bl	ftl_malloc
	str	x0, [x19, 3576]
	ldrh	w0, [x19, 324]
	lsl	w0, w0, 1
	bl	ftl_malloc
	str	x0, [x19, 352]
	ldrh	w0, [x19, 324]
	lsl	w0, w0, 1
	bl	ftl_malloc
	str	x0, [x19, 3584]
	ldrh	w0, [x19, 324]
	lsl	w0, w0, 2
	bl	ftl_malloc
	str	x0, [x19, 3592]
	ldrh	w0, [x19, 326]
	lsl	w0, w0, 2
	bl	ftl_malloc
	ldrh	w2, [x19, 326]
	mov	w1, 0
	str	x0, [x19, 3600]
	lsl	w2, w2, 2
	bl	ftl_memset
	ldrh	w0, [x19, 340]
	lsl	w0, w0, 2
	bl	ftl_malloc
	str	x0, [x19, 3608]
	ldr	w0, [x19, 332]
	lsl	w0, w0, 2
	bl	ftl_malloc
	str	x0, [x19, 3616]
	ldrh	w0, [x19, 342]
	lsl	w0, w0, 4
	bl	ftl_malloc
	ldrh	w1, [x19, 342]
	str	x0, [x19, 2608]
	ldrh	w0, [x19, 314]
	mul	w0, w1, w0
	bl	ftl_malloc
	str	x0, [x19, 3624]
	ldrh	w1, [x19, 246]
	mov	w0, 6
	mul	w0, w1, w0
	bl	ftl_malloc
	str	x0, [x19, 2560]
	ldrh	w0, [x19, 302]
	ldrh	w1, [x19, 258]
	add	w0, w0, 31
	asr	w0, w0, 5
	strh	w0, [x19, 3632]
	mul	w0, w1, w0
	lsl	w0, w0, 2
	bl	ftl_malloc
	ldrh	w4, [x19, 3632]
	add	x5, x19, 400
	ldrh	w6, [x19, 258]
	mov	x1, 1
	str	x0, [x19, 392]
	mov	w2, w4
	mov	x0, x20
.L366:
	cmp	w1, w6
	bcc	.L367
	mov	w2, 8
	add	x4, x0, :lo12:.LANCHOR0
	sub	w2, w2, w1
	add	x4, x4, 360
	add	x2, x2, 1
	mov	x3, 0
.L368:
	add	x3, x3, 1
	cmp	x3, x2
	bne	.L369
	add	x1, x0, :lo12:.LANCHOR0
	ldr	x2, [x1, 3568]
	cbnz	x2, .L370
.L372:
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	add	x1, x1, 216
	adrp	x0, .LC91
	add	x0, x0, :lo12:.LC91
	bl	sftl_printk
	mov	w0, -1
.L365:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
.L367:
	ldr	x3, [x19, 392]
	add	w1, w1, 1
	add	x3, x3, x2, uxtw 2
	add	w2, w2, w4
	str	x3, [x5], 8
	b	.L366
.L369:
	add	x5, x1, x3
	add	x5, x4, x5, lsl 3
	str	xzr, [x5, 24]
	b	.L368
.L370:
	ldr	x2, [x1, 3576]
	cbz	x2, .L372
	ldr	x2, [x1, 3608]
	cbz	x2, .L372
	ldr	x2, [x1, 3616]
	cbz	x2, .L372
	ldr	x2, [x1, 2608]
	cbz	x2, .L372
	ldr	x2, [x1, 3624]
	cbz	x2, .L372
	ldr	x2, [x1, 2560]
	cbz	x2, .L372
	ldr	x2, [x1, 392]
	cbz	x2, .L372
	ldr	x1, [x1, 72]
	cbz	x1, .L372
	add	x1, x0, :lo12:.LANCHOR0
	ldr	x2, [x1, 3288]
	cbz	x2, .L372
	ldr	x2, [x1, 3304]
	cbz	x2, .L372
	ldr	x2, [x1, 3432]
	cbz	x2, .L372
	ldr	x2, [x1, 3448]
	cbz	x2, .L372
	ldr	x2, [x1, 3408]
	cbz	x2, .L372
	ldr	x2, [x1, 3272]
	cbz	x2, .L372
	ldr	x2, [x1, 3440]
	cbz	x2, .L372
	ldr	x2, [x1, 3456]
	cbz	x2, .L372
	ldr	x2, [x1, 3464]
	cbz	x2, .L372
	ldr	x1, [x1, 3472]
	cbz	x1, .L372
	add	x0, x0, :lo12:.LANCHOR0
	ldr	x1, [x0, 3256]
	cbz	x1, .L372
	ldr	x1, [x0, 3480]
	cbz	x1, .L372
	ldr	x1, [x0, 3488]
	cbz	x1, .L372
	ldr	x1, [x0, 3248]
	cbz	x1, .L372
	ldr	x1, [x0, 3520]
	cbz	x1, .L372
	ldr	x1, [x0, 3528]
	cbz	x1, .L372
	ldr	x1, [x0, 3264]
	cbz	x1, .L372
	ldr	x1, [x0, 2600]
	cbz	x1, .L372
	ldr	x1, [x0, 3544]
	cbz	x1, .L372
	ldr	x0, [x0, 352]
	cbz	x0, .L372
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	ldr	x1, [x0, 3584]
	cbz	x1, .L372
	ldr	x1, [x0, 3592]
	cbz	x1, .L372
	ldr	x0, [x0, 3600]
	cbz	x0, .L372
	mov	w0, 0
	b	.L365
	.size	FtlMemInit, .-FtlMemInit
	.align	2
	.global	FtlBbt2Bitmap
	.type	FtlBbt2Bitmap, %function
FtlBbt2Bitmap:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	stp	x23, x24, [sp, 48]
	mov	x23, x0
	add	x0, x21, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR1
	add	x20, x20, :lo12:.LANCHOR1
	adrp	x24, .LC8
	add	x20, x20, 232
	ldrh	w2, [x0, 3632]
	add	x24, x24, :lo12:.LC8
	mov	x19, 0
	mov	x22, x1
	mov	x0, x22
	mov	w1, 0
	lsl	w2, w2, 2
	bl	ftl_memset
.L470:
	ldrh	w0, [x23, x19]
	mov	w1, 65535
	cmp	w0, w1
	beq	.L467
	add	x1, x21, :lo12:.LANCHOR0
	ldrh	w1, [x1, 302]
	cmp	w1, w0
	bhi	.L469
	mov	w2, 74
	mov	x1, x20
	mov	x0, x24
	bl	sftl_printk
.L469:
	ldrh	w2, [x23, x19]
	mov	w1, 1
	add	x19, x19, 2
	cmp	x19, 1024
	ubfx	x0, x2, 5, 11
	lsl	w2, w1, w2
	lsl	x0, x0, 2
	ldr	w1, [x22, x0]
	orr	w1, w1, w2
	str	w1, [x22, x0]
	bne	.L470
.L467:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 64
	ret
	.size	FtlBbt2Bitmap, .-FtlBbt2Bitmap
	.align	2
	.global	FtlBbtMemInit
	.type	FtlBbtMemInit, %function
FtlBbtMemInit:
	stp	x29, x30, [sp, -16]!
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	mov	w1, -1
	add	x29, sp, 0
	mov	w2, 16
	add	x0, x0, 372
	strh	w1, [x0, -12]
	mov	w1, 255
	strh	wzr, [x0, -6]
	bl	ftl_memset
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlBbtMemInit, .-FtlBbtMemInit
	.align	2
	.global	FtlFreeSysBlkQueueInit
	.type	FtlFreeSysBlkQueueInit, %function
FtlFreeSysBlkQueueInit:
	stp	x29, x30, [sp, -16]!
	adrp	x3, .LANCHOR0
	add	x3, x3, :lo12:.LANCHOR0
	mov	w2, 2048
	add	x29, sp, 0
	mov	w1, 0
	strh	w0, [x3, 456]
	add	x0, x3, 464
	strh	wzr, [x3, 458]
	strh	wzr, [x3, 460]
	strh	wzr, [x3, 462]
	bl	ftl_memset
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlFreeSysBlkQueueInit, .-FtlFreeSysBlkQueueInit
	.align	2
	.global	ftl_free_no_use_map_blk
	.type	ftl_free_no_use_map_blk, %function
ftl_free_no_use_map_blk:
	stp	x29, x30, [sp, -80]!
	mov	w1, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	str	x25, [sp, 64]
	ldrh	w2, [x0, 10]
	ldp	x21, x20, [x0, 32]
	ldr	x22, [x0, 16]
	lsl	w2, w2, 1
	mov	x0, x21
	bl	ftl_memset
	mov	w0, 0
.L479:
	ldrh	w1, [x19, 6]
	cmp	w1, w0
	bhi	.L483
	adrp	x0, .LANCHOR0+308
	mov	w23, 0
	mov	w20, 0
	ldrh	w1, [x0, #:lo12:.LANCHOR0+308]
	ldrh	w0, [x19]
	strh	w1, [x21, x0, lsl 1]
	ldrh	w24, [x21]
.L484:
	ldrh	w0, [x19, 10]
	cmp	w0, w20
	bhi	.L488
	mov	w0, w23
	ldr	x25, [sp, 64]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 80
	ret
.L483:
	ubfiz	x1, x0, 2, 16
	ldr	w2, [x20, x1]
	mov	w1, 0
	ubfx	x2, x2, 10, 16
.L480:
	ldrh	w3, [x19, 10]
	cmp	w3, w1
	bhi	.L482
	add	w0, w0, 1
	and	w0, w0, 65535
	b	.L479
.L482:
	ubfiz	x3, x1, 1, 16
	ldrh	w4, [x22, x3]
	cmp	w4, w2
	bne	.L481
	cbz	w2, .L481
	ldrh	w4, [x21, x3]
	add	w4, w4, 1
	strh	w4, [x21, x3]
.L481:
	add	w1, w1, 1
	and	w1, w1, 65535
	b	.L480
.L488:
	ubfiz	x0, x20, 1, 16
	ldrh	w1, [x21, x0]
	cmp	w24, w1
	bls	.L485
	add	x25, x22, x0
	ldrh	w0, [x22, x0]
	cbnz	w0, .L486
.L487:
	add	w20, w20, 1
	and	w20, w20, 65535
	b	.L484
.L485:
	cbnz	w1, .L487
	add	x25, x22, x0
	ldrh	w0, [x22, x0]
	cbz	w0, .L487
.L489:
	mov	w1, 1
	bl	FtlFreeSysBlkQueueIn
	strh	wzr, [x25]
	ldrh	w0, [x19, 8]
	sub	w0, w0, #1
	strh	w0, [x19, 8]
	b	.L487
.L490:
	mov	w24, 0
	b	.L489
.L486:
	mov	w23, w20
	cbz	w1, .L490
	mov	w24, w1
	b	.L487
	.size	ftl_free_no_use_map_blk, .-ftl_free_no_use_map_blk
	.align	2
	.global	FtlL2PDataInit
	.type	FtlL2PDataInit, %function
FtlL2PDataInit:
	stp	x29, x30, [sp, -32]!
	adrp	x0, .LANCHOR0
	mov	w1, 0
	add	x29, sp, 0
	str	x19, [sp, 16]
	add	x19, x0, :lo12:.LANCHOR0
	ldr	x0, [x19, 3576]
	ldr	w2, [x19, 332]
	lsl	w2, w2, 1
	bl	ftl_memset
	ldrh	w0, [x19, 342]
	mov	w1, 255
	ldrh	w2, [x19, 314]
	mul	w2, w2, w0
	ldr	x0, [x19, 3624]
	bl	ftl_memset
	mov	x0, x19
	mov	w1, 0
	mov	w5, -1
.L500:
	ldrh	w2, [x0, 342]
	cmp	w2, w1
	bhi	.L501
	mov	w2, -1
	strh	w2, [x0, 3642]
	strh	w2, [x0, 3640]
	add	x1, x0, 3640
	ldr	w2, [x0, 332]
	strh	w2, [x0, 3650]
	mov	w2, -3902
	strh	w2, [x0, 3644]
	ldrh	w2, [x0, 3704]
	strh	w2, [x0, 3648]
	ldrh	w2, [x0, 340]
	strh	w2, [x0, 3646]
	ldr	x2, [x0, 3568]
	str	x2, [x0, 3656]
	ldr	x2, [x0, 3616]
	str	x2, [x0, 3664]
	ldr	x2, [x0, 3576]
	str	x2, [x0, 3672]
	ldr	x0, [x0, 3608]
	str	x0, [x1, 40]
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L501:
	ldr	x3, [x0, 2608]
	ubfiz	x2, x1, 4, 16
	add	x3, x3, x2
	str	wzr, [x3, 4]
	ldr	x3, [x0, 2608]
	strh	w5, [x3, x2]
	ldr	x3, [x0, 2608]
	ldr	x4, [x0, 3624]
	add	x3, x3, x2
	ldrh	w2, [x0, 314]
	mul	w2, w2, w1
	add	w1, w1, 1
	and	w1, w1, 65535
	sxtw	x2, w2
	and	x2, x2, -4
	add	x2, x4, x2
	str	x2, [x3, 8]
	b	.L500
	.size	FtlL2PDataInit, .-FtlL2PDataInit
	.align	2
	.global	FtlVariablesInit
	.type	FtlVariablesInit, %function
FtlVariablesInit:
	stp	x29, x30, [sp, -32]!
	mov	w0, -1
	mov	w1, 0
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	strh	w0, [x19, 3706]
	mov	w0, -1
	str	w0, [x19, 3716]
	ldr	x0, [x19, 352]
	ldrh	w2, [x19, 324]
	strh	wzr, [x19, 348]
	str	wzr, [x19, 3708]
	lsl	w2, w2, 1
	str	wzr, [x19, 3712]
	bl	ftl_memset
	ldr	x0, [x19, 2600]
	mov	w1, 0
	ldrh	w2, [x19, 246]
	lsl	w2, w2, 1
	bl	ftl_memset
	ldr	x0, [x19, 3544]
	mov	w1, 0
	ldrh	w2, [x19, 246]
	lsl	w2, w2, 1
	bl	ftl_memset
	mov	w2, 48
	mov	w1, 0
	add	x0, x19, 2512
	bl	ftl_memset
	mov	w2, 512
	mov	w1, 0
	add	x0, x19, 2712
	bl	ftl_memset
	bl	FtlGcBufInit
	bl	FtlL2PDataInit
	ldr	x19, [sp, 16]
	mov	w0, 0
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlVariablesInit, .-FtlVariablesInit
	.align	2
	.global	SupperBlkListInit
	.type	SupperBlkListInit, %function
SupperBlkListInit:
	stp	x29, x30, [sp, -64]!
	mov	w0, 6
	mov	w1, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	str	x23, [sp, 48]
	add	x23, x19, 264
	stp	x21, x22, [sp, 32]
	mov	w20, 0
	mov	w22, 0
	ldrh	w2, [x19, 246]
	mov	w21, 0
	mul	w2, w2, w0
	ldr	x0, [x19, 2560]
	bl	ftl_memset
	strh	wzr, [x19, 2584]
	strh	wzr, [x19, 228]
	str	xzr, [x19, 2568]
	str	xzr, [x19, 2576]
	str	xzr, [x19, 2592]
.L506:
	ldrh	w0, [x19, 244]
	cmp	w20, w0
	bcs	.L513
	ldrh	w8, [x19, 236]
	mov	x6, 0
	ldrh	w7, [x19, 306]
	mov	w5, 0
	b	.L514
.L508:
	ldrb	w0, [x23, x6]
	mov	w1, w20
	bl	V2P_block
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L507
	add	w5, w5, w7
	and	w5, w5, 65535
.L507:
	add	x6, x6, 1
.L514:
	cmp	w8, w6, uxth
	bhi	.L508
	cbz	w5, .L509
	mov	w0, 32768
	sdiv	w5, w0, w5
.L510:
	ldr	x1, [x19, 2560]
	mov	w0, 6
	umaddl	x0, w20, w0, x1
	strh	w5, [x0, 4]
	ldrh	w0, [x19, 24]
	cmp	w0, w20
	beq	.L511
	ldrh	w0, [x19, 80]
	cmp	w0, w20
	beq	.L511
	ldrh	w0, [x19, 128]
	cmp	w0, w20
	beq	.L511
	ldr	x1, [x19, 72]
	ubfiz	x0, x20, 1, 16
	ldrh	w0, [x1, x0]
	cbnz	w0, .L512
	add	w22, w22, 1
	mov	w0, w20
	and	w22, w22, 65535
	bl	INSERT_FREE_LIST
.L511:
	add	w20, w20, 1
	and	w20, w20, 65535
	b	.L506
.L509:
	ldr	x1, [x19, 72]
	ubfiz	x0, x20, 1, 16
	mov	w2, -1
	strh	w2, [x1, x0]
	b	.L510
.L512:
	add	w21, w21, 1
	mov	w0, w20
	and	w21, w21, 65535
	bl	INSERT_DATA_LIST
	b	.L511
.L513:
	strh	w21, [x19, 2584]
	add	w21, w21, w22
	strh	w22, [x19, 228]
	cmp	w21, w0
	ble	.L515
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2219
	add	x1, x1, 248
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L515:
	mov	w0, 0
	ldr	x23, [sp, 48]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 64
	ret
	.size	SupperBlkListInit, .-SupperBlkListInit
	.align	2
	.global	FtlGcPageVarInit
	.type	FtlGcPageVarInit, %function
FtlGcPageVarInit:
	stp	x29, x30, [sp, -32]!
	mov	w1, 255
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	ldr	x0, [x19, 3288]
	ldrh	w2, [x19, 310]
	strh	wzr, [x19, 3284]
	strh	wzr, [x19, 3296]
	lsl	w2, w2, 1
	bl	ftl_memset
	ldrh	w2, [x19, 310]
	mov	w0, 12
	mov	w1, 255
	mul	w2, w2, w0
	ldr	x0, [x19, 3304]
	bl	ftl_memset
	bl	FtlGcBufInit
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlGcPageVarInit, .-FtlGcPageVarInit
	.align	2
	.global	FlashGetBadBlockList
	.type	FlashGetBadBlockList, %function
FlashGetBadBlockList:
	stp	x29, x30, [sp, -48]!
	mov	w2, 256
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x20, x0
	str	x21, [sp, 32]
	mov	w21, w1
	mov	w1, 255
	bl	ftl_memset
	adrp	x19, .LANCHOR0
	add	x0, x19, :lo12:.LANCHOR0
	mov	w1, w21
	ldr	x2, [x0, 3360]
	mov	x0, x20
	blr	x2
	and	w0, w0, 65535
	cmp	w0, 50
	bls	.L520
	mov	w2, 256
	mov	w1, 255
	mov	x0, x20
	bl	ftl_memset
	mov	w0, 0
.L520:
	add	x19, x19, :lo12:.LANCHOR0
	ldrh	w1, [x19, 14]
	cmp	w1, 4
	bne	.L524
	mov	x1, 0
.L522:
	cmp	w0, w1, uxth
	bhi	.L523
.L524:
	ldp	x19, x20, [sp, 16]
	ldr	x21, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
.L523:
	ldrh	w2, [x20, x1, lsl 1]
	lsr	w2, w2, 1
	strh	w2, [x20, x1, lsl 1]
	add	x1, x1, 1
	b	.L522
	.size	FlashGetBadBlockList, .-FlashGetBadBlockList
	.align	2
	.global	ftl_memcpy
	.type	ftl_memcpy, %function
ftl_memcpy:
	stp	x29, x30, [sp, -16]!
	uxtw	x2, w2
	add	x29, sp, 0
	bl	memcpy
	ldp	x29, x30, [sp], 16
	ret
	.size	ftl_memcpy, .-ftl_memcpy
	.align	2
	.global	FlashReadPages
	.type	FlashReadPages, %function
FlashReadPages:
	stp	x29, x30, [sp, -112]!
	ubfiz	x1, x1, 5, 32
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x21, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	add	x2, x21, :lo12:__stack_chk_guard
	stp	x19, x20, [sp, 16]
	adrp	x22, .LANCHOR0
	stp	x25, x26, [sp, 64]
	adrp	x23, .LANCHOR1
	stp	x27, x28, [sp, 80]
	add	x23, x23, :lo12:.LANCHOR1
	adrp	x25, .LC8
	mov	x19, x0
	ldr	x3, [x2]
	str	x3, [x29, 104]
	mov	x3,0
	add	x2, x22, :lo12:.LANCHOR0
	add	x24, x0, x1
	add	x23, x23, 272
	add	x25, x25, :lo12:.LC8
	ldrh	w26, [x2, 12]
.L529:
	cmp	x24, x19
	bne	.L542
	add	x21, x21, :lo12:__stack_chk_guard
	mov	w0, 0
	ldr	x2, [x29, 104]
	ldr	x1, [x21]
	eor	x1, x2, x1
	cbz	x1, .L543
	bl	__stack_chk_fail
.L542:
	ldr	x0, [x19, 8]
	cbz	x0, .L530
	ldr	x0, [x19, 16]
	cbnz	x0, .L531
.L530:
	mov	w2, 96
	mov	x1, x23
	mov	x0, x25
	bl	sftl_printk
.L531:
	ldr	w0, [x19, 4]
	add	x2, x29, 96
	add	x1, x29, 100
	bl	l2p_addr_tran.isra.0
	ldr	w0, [x29, 96]
	cmp	w0, 3
	bls	.L532
	mov	w0, -1
	str	w0, [x19]
.L533:
	add	x19, x19, 32
	b	.L529
.L532:
	ldr	x20, [x19, 8]
	tst	x20, 63
	beq	.L534
	add	x1, x22, :lo12:.LANCHOR0
	ldr	x20, [x1, 3496]
.L534:
	add	x28, x22, :lo12:.LANCHOR0
	ldr	w1, [x29, 100]
	ldr	x3, [x19, 16]
	mov	x2, x20
	ldr	x4, [x28, 3384]
	blr	x4
	str	w0, [x19]
	ldrh	w0, [x28, 14]
	cmp	w0, 4
	bne	.L536
	ldrb	w0, [x29, 96]
	add	x2, x20, 2048
	ldr	x4, [x28, 3384]
	ldr	w1, [x29, 100]
	ldr	x3, [x19, 16]
	add	w1, w26, w1
	add	x3, x3, 8
	blr	x4
	cmn	w0, #1
	beq	.L537
	ldr	x1, [x19, 16]
	ldr	w2, [x1, 12]
	cmn	w2, #1
	bne	.L538
	ldr	w2, [x1, 8]
	cmn	w2, #1
	bne	.L538
	ldr	w1, [x1]
	cmn	w1, #1
	beq	.L538
.L537:
	mov	w1, -1
	str	w1, [x19]
.L538:
	ldr	w1, [x19]
	cmn	w1, #1
	beq	.L539
	cmp	w0, 256
	bne	.L539
	str	w0, [x19]
.L539:
	ldr	w3, [x19]
	cmp	w3, 256
	ccmn	w3, #1, 4, ne
	bne	.L536
	ldr	w1, [x19, 4]
	adrp	x0, .LC92
	ldr	w2, [x29, 100]
	add	x0, x0, :lo12:.LC92
	bl	sftl_printk
	ldr	x1, [x19, 8]
	cbz	x1, .L541
	mov	w3, 4
	adrp	x0, .LC93
	mov	w2, w3
	add	x0, x0, :lo12:.LC93
	bl	rknand_print_hex
.L541:
	ldr	x1, [x19, 16]
	cbz	x1, .L536
	mov	w3, 4
	adrp	x0, .LC94
	mov	w2, w3
	add	x0, x0, :lo12:.LC94
	bl	rknand_print_hex
.L536:
	add	x1, x22, :lo12:.LANCHOR0
	ldr	x0, [x1, 3496]
	cmp	x20, x0
	bne	.L533
	ldr	x0, [x19, 8]
	cmp	x20, x0
	beq	.L533
	ldrh	w2, [x1, 262]
	mov	x1, x20
	lsl	w2, w2, 9
	bl	ftl_memcpy
	b	.L533
.L543:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
	.size	FlashReadPages, .-FlashReadPages
	.align	2
	.global	FtlLoadFactoryBbt
	.type	FtlLoadFactoryBbt, %function
FtlLoadFactoryBbt:
	stp	x29, x30, [sp, -80]!
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	add	x22, x0, 372
	stp	x25, x26, [sp, 64]
	mov	w21, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x23, x24, [sp, 48]
	add	x23, x0, 3720
	mov	x25, x23
	mov	w26, 61664
	ldr	x1, [x0, 3456]
	ldr	x24, [x0, 3520]
	stp	x1, x24, [x23, 8]
.L570:
	ldrh	w0, [x19, 258]
	cmp	w21, w0
	bcc	.L575
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
.L575:
	ldrh	w20, [x19, 302]
	mov	w0, -1
	strh	w0, [x22]
.L572:
	ldrh	w0, [x19, 302]
	sub	w20, w20, #1
	and	w20, w20, 65535
	sub	w1, w0, #16
	cmp	w20, w1
	ble	.L573
	madd	w0, w0, w21, w20
	mov	w2, 1
	mov	w1, w2
	lsl	w0, w0, 10
	str	w0, [x25, 4]
	mov	x0, x23
	bl	FlashReadPages
	ldr	w0, [x25]
	cmn	w0, #1
	beq	.L572
	ldrh	w0, [x24]
	cmp	w0, w26
	bne	.L572
	strh	w20, [x22]
.L573:
	add	w21, w21, 1
	add	x22, x22, 2
	b	.L570
	.size	FtlLoadFactoryBbt, .-FtlLoadFactoryBbt
	.align	2
	.global	FtlGetLastWrittenPage
	.type	FtlGetLastWrittenPage, %function
FtlGetLastWrittenPage:
	stp	x29, x30, [sp, -192]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x21, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	mov	w24, w1
	stp	x19, x20, [sp, 16]
	add	x1, x21, :lo12:__stack_chk_guard
	str	x25, [sp, 64]
	cmp	w24, 1
	ldr	x2, [x1]
	str	x2, [x29, 184]
	mov	x2,0
	adrp	x1, .LANCHOR0
	add	x2, x1, :lo12:.LANCHOR0
	bne	.L581
	ldrh	w19, [x2, 308]
.L582:
	add	x1, x1, :lo12:.LANCHOR0
	sub	w19, w19, #1
	sxth	w19, w19
	lsl	w22, w0, 10
	orr	w0, w19, w22
	mov	w2, w24
	ldr	x1, [x1, 3504]
	str	x1, [x29, 96]
	add	x1, x29, 120
	str	w0, [x29, 92]
	add	x0, x29, 88
	str	x1, [x29, 104]
	mov	w1, 1
	bl	FlashReadPages
	ldr	w0, [x29, 120]
	cmn	w0, #1
	bne	.L583
	mov	w23, 0
	mov	w25, 2
.L584:
	cmp	w23, w19
	ble	.L587
.L583:
	add	x21, x21, :lo12:__stack_chk_guard
	mov	w0, w19
	ldr	x2, [x29, 184]
	ldr	x1, [x21]
	eor	x1, x2, x1
	cbz	x1, .L588
	bl	__stack_chk_fail
.L581:
	ldrh	w19, [x2, 306]
	b	.L582
.L587:
	add	w20, w23, w19
	mov	w2, w24
	mov	w1, 1
	sdiv	w20, w20, w25
	sxth	w0, w20
	orr	w0, w0, w22
	str	w0, [x29, 92]
	add	x0, x29, 88
	bl	FlashReadPages
	ldr	w0, [x29, 120]
	cmn	w0, #1
	bne	.L585
	ldr	w0, [x29, 124]
	cmn	w0, #1
	bne	.L585
	ldr	w0, [x29, 88]
	cmn	w0, #1
	beq	.L585
	sub	w19, w20, #1
	sxth	w19, w19
	b	.L584
.L585:
	add	w20, w20, 1
	sxth	w23, w20
	b	.L584
.L588:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldr	x25, [sp, 64]
	ldp	x29, x30, [sp], 192
	ret
	.size	FtlGetLastWrittenPage, .-FtlGetLastWrittenPage
	.align	2
	.global	FtlScanSysBlk
	.type	FtlScanSysBlk, %function
FtlScanSysBlk:
	stp	x29, x30, [sp, -112]!
	mov	w1, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	add	x19, x21, :lo12:.LANCHOR0
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	adrp	x25, .LANCHOR1
	stp	x27, x28, [sp, 80]
	add	x25, x25, :lo12:.LANCHOR1
	ldr	x0, [x19, 3616]
	add	x25, x25, 288
	ldr	w2, [x19, 332]
	strh	wzr, [x19, 3704]
	strh	wzr, [x19, 348]
	lsl	w2, w2, 2
	bl	ftl_memset
	ldr	x0, [x19, 3568]
	mov	w1, 0
	ldr	w2, [x19, 332]
	lsl	w2, w2, 1
	bl	ftl_memset
	ldr	x0, [x19, 3592]
	mov	w1, 0
	ldrh	w2, [x19, 324]
	lsl	w2, w2, 2
	bl	ftl_memset
	ldr	x0, [x19, 352]
	mov	w1, 0
	ldrh	w2, [x19, 324]
	lsl	w2, w2, 1
	bl	ftl_memset
	mov	w2, 16
	mov	w1, 255
	add	x0, x19, 2696
	bl	ftl_memset
	ldrh	w24, [x19, 244]
.L594:
	ldrh	w0, [x19, 246]
	cmp	w0, w24
	bls	.L634
	ldrh	w9, [x19, 236]
	mov	x5, 0
	ldrh	w8, [x19, 316]
	mov	w20, 0
	add	x7, x19, 264
	mov	w6, 4
	b	.L635
.L596:
	ldrb	w0, [x7, x5]
	mov	w1, w24
	bl	V2P_block
	and	w4, w0, 65535
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L595
	ldr	x0, [x19, 3432]
	ubfiz	x2, x20, 5, 16
	lsl	w4, w4, 10
	add	x0, x0, x2
	str	w4, [x0, 4]
	ldr	x1, [x19, 3432]
	ldr	x0, [x19, 3256]
	add	x1, x1, x2
	ldr	x2, [x19, 3264]
	str	x0, [x1, 8]
	mul	w0, w20, w8
	add	w20, w20, 1
	and	w20, w20, 65535
	sdiv	w0, w0, w6
	add	x0, x2, x0, sxtw 2
	str	x0, [x1, 16]
.L595:
	add	x5, x5, 1
.L635:
	cmp	w9, w5, uxth
	bhi	.L596
	cbnz	w20, .L597
.L633:
	add	w24, w24, 1
	and	w24, w24, 65535
	b	.L594
.L597:
	ldr	x0, [x19, 3432]
	mov	w1, w20
	mov	w2, 1
	ubfiz	x20, x20, 5, 16
	mov	x23, 0
	add	x27, x19, 2696
	bl	FlashReadPages
.L632:
	ldr	x0, [x19, 3432]
	add	x1, x0, x23
	ldr	w0, [x0, x23]
	ldr	w22, [x1, 4]
	cmn	w0, #1
	ldr	x26, [x1, 16]
	ubfx	x22, x22, 10, 16
	bne	.L600
	mov	w28, 16
	mov	w3, 65535
.L602:
	ldr	x0, [x19, 3432]
	mov	w2, 1
	str	w3, [x29, 108]
	add	x0, x0, x23
	ldr	w1, [x0, 4]
	add	w1, w1, 1
	str	w1, [x0, 4]
	mov	w1, w2
	ldr	x0, [x19, 3432]
	add	x0, x0, x23
	bl	FlashReadPages
	ldrh	w0, [x26]
	ldr	w3, [x29, 108]
	cmp	w0, w3
	ldr	x0, [x19, 3432]
	bne	.L599
	mov	w1, -1
	str	w1, [x0, x23]
	ldr	x0, [x19, 3432]
	ldr	w0, [x0, x23]
	cmp	w0, w1
	bne	.L600
.L601:
	mov	w1, 1
	b	.L672
.L599:
	ldr	w0, [x0, x23]
	cmn	w0, #1
	bne	.L600
	sub	w28, w28, #1
	ands	w28, w28, 65535
	bne	.L602
	b	.L601
.L600:
	ldr	w1, [x19, 2660]
	ldr	w0, [x26, 4]
	cmn	w1, #1
	beq	.L603
	cmp	w1, w0
	bhi	.L604
.L603:
	cmn	w0, #1
	beq	.L604
	add	w1, w0, 1
	str	w1, [x19, 2660]
.L604:
	ldrh	w1, [x26]
	mov	w2, 61604
	cmp	w1, w2
	beq	.L606
	bhi	.L607
	mov	w0, 61574
	cmp	w1, w0
	beq	.L608
.L605:
	add	x23, x23, 32
	cmp	x20, x23
	bne	.L632
	b	.L633
.L607:
	mov	w0, 61634
	cmp	w1, w0
	beq	.L609
	mov	w0, 65535
	cmp	w1, w0
	bne	.L605
	mov	w1, 0
.L672:
	mov	w0, w22
	bl	FtlFreeSysBlkQueueIn
	b	.L605
.L609:
	ldrh	w1, [x19, 3704]
	ldr	w0, [x19, 332]
	cmp	w1, w0
	bls	.L611
	adrp	x0, .LC8
	mov	w2, 1232
	mov	x1, x25
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L611:
	ldr	w3, [x19, 332]
	ldrh	w1, [x19, 3704]
	and	w2, w3, 65535
	ldr	x4, [x19, 3616]
	sub	w0, w2, #1
	sub	w2, w2, w1
	sub	w2, w2, #1
	sxth	x0, w0
	sxth	w2, w2
.L612:
	cmp	w0, w2
	bgt	.L618
	tbz	w0, #31, .L652
	b	.L605
.L618:
	sxtw	x5, w0
	ldr	w8, [x26, 4]
	lsl	x6, x5, 2
	ldr	w7, [x4, x6]
	cmp	w8, w7
	bls	.L613
	ldr	w2, [x4]
	cbnz	w2, .L614
	cmp	w3, w1
	beq	.L614
	add	w1, w1, 1
	strh	w1, [x19, 3704]
.L614:
	mov	w1, 0
.L615:
	cmp	w1, w0
	bne	.L616
	ldr	x1, [x19, 3616]
	ldr	w2, [x26, 4]
	str	w2, [x1, x6]
	ldr	x1, [x19, 3568]
	strh	w22, [x1, x5, lsl 1]
	tbnz	w0, #31, .L605
	ldrh	w1, [x19, 3704]
	ldr	w2, [x19, 332]
	sub	w2, w2, w1
	sub	w2, w2, #1
	cmp	w0, w2, sxth
	bgt	.L605
.L652:
	add	w1, w1, 1
	strh	w1, [x19, 3704]
	ldr	x1, [x19, 3616]
	ldr	w2, [x26, 4]
	str	w2, [x1, x0, lsl 2]
	ldr	x1, [x19, 3568]
.L670:
	strh	w22, [x1, x0, lsl 1]
	b	.L605
.L616:
	ldr	x4, [x19, 3616]
	sxtw	x2, w1
	lsl	x3, x2, 2
	lsl	x2, x2, 1
	add	x7, x4, x3
	add	w1, w1, 1
	sxth	w1, w1
	ldr	w7, [x7, 4]
	str	w7, [x4, x3]
	ldr	x3, [x19, 3568]
	add	x4, x3, x2
	ldrh	w4, [x4, 2]
	strh	w4, [x3, x2]
	b	.L615
.L613:
	sub	w0, w0, #1
	sxth	x0, w0
	b	.L612
.L608:
	ldrh	w1, [x19, 348]
	ldrh	w0, [x19, 324]
	cmp	w1, w0
	bls	.L621
	adrp	x0, .LC8
	mov	w2, 1273
	mov	x1, x25
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L621:
	ldrh	w3, [x19, 324]
	ldrh	w1, [x19, 348]
	sub	w2, w3, #1
	ldr	x4, [x19, 3592]
	sxth	x0, w2
	sub	w2, w2, w1
.L622:
	cmp	w0, w2
	ble	.L627
	sxtw	x5, w0
	ldr	w8, [x26, 4]
	lsl	x6, x5, 2
	ldr	w7, [x4, x6]
	cmp	w8, w7
	bls	.L623
	ldr	w2, [x4]
	cbnz	w2, .L624
	cmp	w3, w1
	beq	.L624
	add	w1, w1, 1
	strh	w1, [x19, 348]
.L624:
	mov	w1, 0
.L625:
	cmp	w1, w0
	bne	.L626
	ldr	x1, [x19, 3592]
	ldr	w2, [x26, 4]
	str	w2, [x1, x6]
	ldr	x1, [x19, 352]
	strh	w22, [x1, x5, lsl 1]
.L627:
	tbnz	w0, #31, .L605
	ldrh	w1, [x19, 324]
	ldrh	w2, [x19, 348]
	sub	w1, w1, #1
	sub	w1, w1, w2
	cmp	w0, w1, sxth
	bgt	.L605
	ldr	x1, [x19, 3592]
	add	w2, w2, 1
	strh	w2, [x19, 348]
	ldr	w2, [x26, 4]
	str	w2, [x1, x0, lsl 2]
	ldr	x1, [x19, 352]
	b	.L670
.L626:
	ldr	x4, [x19, 3592]
	sxtw	x2, w1
	lsl	x3, x2, 2
	lsl	x2, x2, 1
	add	x7, x4, x3
	add	w1, w1, 1
	sxth	w1, w1
	ldr	w7, [x7, 4]
	str	w7, [x4, x3]
	ldr	x3, [x19, 352]
	add	x4, x3, x2
	ldrh	w4, [x4, 2]
	strh	w4, [x3, x2]
	b	.L625
.L623:
	sub	w0, w0, #1
	sxth	x0, w0
	b	.L622
.L606:
	ldrh	w2, [x27]
	mov	w1, 65535
	cmp	w2, w1
	bne	.L629
	strh	w22, [x27]
.L671:
	str	w0, [x27, 8]
	b	.L605
.L629:
	ldrh	w0, [x27, 4]
	cmp	w0, w1
	beq	.L630
	mov	w1, 1
	bl	FtlFreeSysBlkQueueIn
.L630:
	ldr	w0, [x26, 4]
	ldr	w1, [x27, 8]
	cmp	w1, w0
	bcs	.L631
	ldrh	w0, [x27]
	strh	w0, [x27, 4]
	strh	w22, [x27]
	ldr	w0, [x26, 4]
	b	.L671
.L631:
	strh	w22, [x27, 4]
	b	.L605
.L634:
	ldr	x1, [x19, 3568]
	ldrh	w0, [x1]
	cbz	w0, .L636
.L639:
	add	x0, x21, :lo12:.LANCHOR0
	ldr	x1, [x0, 352]
	ldrh	w2, [x1]
	cbz	w2, .L637
.L638:
	add	x21, x21, :lo12:.LANCHOR0
	ldrh	w1, [x21, 3704]
	ldr	w0, [x21, 332]
	cmp	w1, w0
	bls	.L668
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1398
	add	x1, x1, 288
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L668:
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
.L636:
	ldrh	w0, [x19, 3704]
	cbz	w0, .L639
	ldr	w2, [x19, 332]
	mov	w0, 0
.L640:
	cmp	w0, w2
	bcs	.L639
	ldrh	w3, [x1, w0, sxtw 1]
	cbz	w3, .L641
	mov	w1, w0
	add	x2, x21, :lo12:.LANCHOR0
.L642:
	ldr	w3, [x2, 332]
	cmp	w1, w3
	bcs	.L639
	ldr	x5, [x2, 3568]
	sxtw	x6, w1
	lsl	x4, x6, 1
	sub	w3, w1, w0
	sxtw	x3, w3
	add	w1, w1, 1
	sxth	w1, w1
	ldrh	w7, [x5, x4]
	strh	w7, [x5, x3, lsl 1]
	ldr	x5, [x2, 3616]
	ldr	w6, [x5, x6, lsl 2]
	str	w6, [x5, x3, lsl 2]
	ldr	x3, [x2, 3568]
	strh	wzr, [x3, x4]
	b	.L642
.L641:
	add	w0, w0, 1
	sxth	w0, w0
	b	.L640
.L637:
	ldrh	w2, [x0, 348]
	cbz	w2, .L638
	ldrh	w2, [x0, 324]
	mov	w0, 0
.L647:
	mov	w6, w0
	cmp	w0, w2
	bge	.L638
	ldrh	w3, [x1, w0, sxtw 1]
	cbz	w3, .L648
	add	x1, x21, :lo12:.LANCHOR0
.L649:
	ldrh	w2, [x1, 324]
	cmp	w0, w2
	bge	.L638
	ldr	x4, [x1, 352]
	sxtw	x5, w0
	lsl	x3, x5, 1
	sub	w2, w0, w6
	sxtw	x2, w2
	add	w0, w0, 1
	sxth	w0, w0
	ldrh	w7, [x4, x3]
	strh	w7, [x4, x2, lsl 1]
	ldr	x4, [x1, 3592]
	ldr	w5, [x4, x5, lsl 2]
	str	w5, [x4, x2, lsl 2]
	ldr	x2, [x1, 352]
	strh	wzr, [x2, x3]
	b	.L649
.L648:
	add	w0, w0, 1
	sxth	w0, w0
	b	.L647
	.size	FtlScanSysBlk, .-FtlScanSysBlk
	.align	2
	.global	FtlLoadBbt
	.type	FtlLoadBbt, %function
FtlLoadBbt:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	stp	x23, x24, [sp, 48]
	add	x23, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x21, x23, 3720
	mov	w24, 61649
	ldr	x0, [x23, 3456]
	ldr	x22, [x23, 3520]
	stp	x0, x22, [x21, 8]
	bl	FtlBbtMemInit
	ldrh	w20, [x23, 302]
	sub	w20, w20, #1
	and	w20, w20, 65535
.L674:
	ldrh	w0, [x23, 302]
	sub	w0, w0, #16
	cmp	w20, w0
	ble	.L677
	lsl	w0, w20, 10
	mov	w2, 1
	str	w0, [x21, 4]
	mov	w1, w2
	mov	x0, x21
	bl	FlashReadPages
	ldr	w0, [x21]
	cmn	w0, #1
	bne	.L675
	ldr	w0, [x21, 4]
	mov	w2, 1
	mov	w1, w2
	add	w0, w0, 1
	str	w0, [x21, 4]
	mov	x0, x21
	bl	FlashReadPages
.L675:
	ldr	w0, [x21]
	cmn	w0, #1
	beq	.L676
	ldrh	w0, [x22]
	cmp	w0, w24
	bne	.L676
	ldr	w1, [x22, 4]
	str	w1, [x23, 368]
	strh	w20, [x23, 360]
	ldrh	w1, [x22, 8]
	strh	w1, [x23, 364]
.L677:
	add	x21, x19, :lo12:.LANCHOR0
	mov	w0, 65535
	ldrh	w1, [x21, 360]
	cmp	w1, w0
	beq	.L691
	ldrh	w1, [x21, 364]
	cmp	w1, w0
	beq	.L681
	add	x0, x21, 3720
	lsl	w1, w1, 10
	mov	w2, 1
	str	w1, [x0, 4]
	mov	w1, w2
	bl	FlashReadPages
	ldr	w0, [x21, 3720]
	cmn	w0, #1
	beq	.L681
	ldrh	w1, [x22]
	mov	w0, 61649
	cmp	w1, w0
	bne	.L681
	ldr	w1, [x21, 368]
	ldr	w0, [x22, 4]
	cmp	w0, w1
	bls	.L681
	str	w0, [x21, 368]
	ldrh	w1, [x21, 364]
	ldrh	w0, [x22, 8]
	strh	w1, [x21, 360]
	strh	w0, [x21, 364]
.L681:
	add	x20, x19, :lo12:.LANCHOR0
	mov	w1, 1
	add	x23, x20, 3720
	mov	w24, 61649
	ldrh	w0, [x20, 360]
	bl	FtlGetLastWrittenPage
	sxth	w21, w0
	add	w0, w0, 1
	strh	w0, [x20, 362]
.L683:
	tbz	w21, #31, .L686
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 251
	add	x1, x1, 304
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L685:
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w0, [x22, 10]
	mov	w2, 65535
	strh	w0, [x1, 366]
	ldrh	w0, [x22, 12]
	cmp	w0, w2
	beq	.L688
	ldr	w2, [x1, 232]
	cmp	w0, w2
	beq	.L688
	ldrh	w1, [x1, 246]
	lsr	w1, w1, 2
	cmp	w2, w1
	bcs	.L688
	cmp	w0, w1
	bcs	.L688
	bl	FtlSysBlkNumInit
.L688:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w20, 0
	add	x21, x19, 392
.L689:
	ldrh	w0, [x19, 258]
	cmp	w20, w0
	bcc	.L690
	mov	w0, 0
.L673:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 64
	ret
.L676:
	sub	w20, w20, #1
	and	w20, w20, 65535
	b	.L674
.L686:
	ldrh	w0, [x20, 360]
	mov	w2, 1
	mov	w1, w2
	orr	w0, w21, w0, lsl 10
	str	w0, [x23, 4]
	ldr	x0, [x20, 3456]
	str	x0, [x23, 8]
	mov	x0, x23
	bl	FlashReadPages
	ldr	w0, [x23]
	cmn	w0, #1
	beq	.L684
	ldrh	w0, [x22]
	cmp	w0, w24
	beq	.L685
.L684:
	sub	w21, w21, #1
	sxth	w21, w21
	b	.L683
.L690:
	ldrh	w2, [x19, 3632]
	ldr	x0, [x19, 3728]
	mul	w1, w2, w20
	lsl	w2, w2, 2
	add	w20, w20, 1
	add	x1, x0, x1, lsl 2
	ldr	x0, [x21], 8
	bl	ftl_memcpy
	b	.L689
.L691:
	mov	w0, -1
	b	.L673
	.size	FtlLoadBbt, .-FtlLoadBbt
	.align	2
	.global	FlashProgPages
	.type	FlashProgPages, %function
FlashProgPages:
	stp	x29, x30, [sp, -160]!
	ubfiz	x1, x1, 5, 32
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	adrp	x22, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	adrp	x21, .LANCHOR0
	stp	x25, x26, [sp, 64]
	adrp	x24, .LANCHOR1
	stp	x27, x28, [sp, 80]
	add	x24, x24, :lo12:.LANCHOR1
	str	w2, [x29, 108]
	add	x2, x22, :lo12:__stack_chk_guard
	ldr	x3, [x2]
	str	x3, [x29, 152]
	mov	x3,0
	add	x2, x21, :lo12:.LANCHOR0
	adrp	x25, .LC8
	add	x23, x0, x1
	mov	x20, x0
	add	x24, x24, 320
	ldrh	w26, [x2, 12]
	add	x25, x25, :lo12:.LC8
	lsl	w28, w26, 3
.L704:
	cmp	x20, x23
	bne	.L718
	adrp	x20, .LC95
	adrp	x24, .LC96
	add	x20, x20, :lo12:.LC95
	add	x24, x24, :lo12:.LC96
.L719:
	cmp	x19, x23
	beq	.L712
	add	x0, x21, :lo12:.LANCHOR0
	ldr	w2, [x29, 108]
	ldr	x1, [x0, 3496]
	str	wzr, [x1]
	ldr	x1, [x0, 3512]
	str	wzr, [x1]
	ldr	w1, [x19, 4]
	str	w1, [x29, 124]
	ldr	x1, [x0, 3496]
	ldr	x0, [x0, 3512]
	stp	x1, x0, [x29, 128]
	mov	w1, 1
	add	x0, x29, 120
	bl	FlashReadPages
	ldr	w25, [x29, 120]
	cmn	w25, #1
	bne	.L720
	ldr	w1, [x19, 4]
	mov	x0, x20
	bl	sftl_printk
	str	w25, [x19]
.L720:
	ldr	w25, [x29, 120]
	cmp	w25, 256
	bne	.L721
	ldr	w1, [x19, 4]
	mov	x0, x24
	bl	sftl_printk
	str	w25, [x19]
.L721:
	ldr	x0, [x19, 16]
	cbz	x0, .L722
	add	x1, x21, :lo12:.LANCHOR0
	ldr	w2, [x0]
	ldr	x1, [x1, 3512]
	ldr	w4, [x1]
	cmp	w2, w4
	bne	.L723
	ldr	w5, [x0, 8]
	ldr	w3, [x1, 8]
	cmp	w5, w3
	beq	.L722
.L723:
	ldr	w3, [x0, 4]
	adrp	x0, .LC97
	ldr	w5, [x1, 4]
	add	x0, x0, :lo12:.LC97
	ldr	w1, [x19, 4]
	bl	sftl_printk
	mov	w0, -1
	str	w0, [x19]
.L722:
	ldr	x0, [x19, 8]
	cbz	x0, .L724
	add	x1, x21, :lo12:.LANCHOR0
	ldr	w2, [x0]
	ldr	x1, [x1, 3496]
	ldr	w4, [x1]
	cmp	w2, w4
	bne	.L725
	ldr	w5, [x0, 2048]
	ldr	w3, [x1, 2048]
	cmp	w5, w3
	beq	.L724
.L725:
	ldr	w3, [x0, 4]
	adrp	x0, .LC98
	ldr	w5, [x1, 4]
	add	x0, x0, :lo12:.LC98
	ldr	w1, [x19, 4]
	bl	sftl_printk
	mov	w0, -1
	str	w0, [x19]
.L724:
	add	x19, x19, 32
	b	.L719
.L718:
	ldr	x0, [x20, 8]
	cbz	x0, .L705
	ldr	x0, [x20, 16]
	cbnz	x0, .L706
.L705:
	mov	w2, 148
	mov	x1, x24
	mov	x0, x25
	bl	sftl_printk
.L706:
	ldr	w0, [x20, 4]
	add	x2, x29, 112
	add	x1, x29, 116
	bl	l2p_addr_tran.isra.0
	ldr	w0, [x29, 112]
	cmp	w0, 3
	bls	.L707
.L743:
	mov	w0, -1
	str	w0, [x20]
	b	.L708
.L707:
	cbnz	w0, .L709
	ldr	w0, [x29, 116]
	cmp	w28, w0
	bls	.L709
	adrp	x20, .LANCHOR1
	add	x20, x20, :lo12:.LANCHOR1
	adrp	x21, .LC86
	add	x20, x20, 320
	add	x21, x21, :lo12:.LC86
	b	.L741
.L711:
	ldr	w2, [x19, 4]
	mov	w0, -1
	str	w0, [x19]
	mov	x1, x20
	mov	x0, x21
	add	x19, x19, 32
	bl	sftl_printk
	ldr	x1, [x19, -24]
	mov	w3, 16
	mov	w2, 4
	adrp	x0, .LC87
	add	x0, x0, :lo12:.LC87
	bl	rknand_print_hex
	ldr	x1, [x19, -16]
	mov	w3, 4
	adrp	x0, .LC88
	mov	w2, w3
	add	x0, x0, :lo12:.LC88
	bl	rknand_print_hex
.L741:
	cmp	x19, x23
	bne	.L711
	bl	dump_stack
.L712:
	add	x22, x22, :lo12:__stack_chk_guard
	mov	w0, 0
	ldr	x2, [x29, 152]
	ldr	x1, [x22]
	eor	x1, x2, x1
	cbz	x1, .L727
	bl	__stack_chk_fail
.L709:
	ldr	x1, [x20, 8]
	tst	x1, 63
	beq	.L728
	add	x0, x21, :lo12:.LANCHOR0
	ldr	x27, [x0, 3496]
	cmp	x1, x27
	beq	.L713
	ldrh	w2, [x0, 262]
	mov	x0, x27
	lsl	w2, w2, 9
	bl	ftl_memcpy
.L713:
	add	x0, x21, :lo12:.LANCHOR0
	ldr	w1, [x29, 116]
	ldr	x3, [x20, 16]
	mov	x2, x27
	ldr	x5, [x0, 3376]
	ldrb	w0, [x29, 112]
	blr	x5
	cbnz	w0, .L714
	str	wzr, [x20]
.L715:
	add	x0, x21, :lo12:.LANCHOR0
	ldrh	w1, [x0, 14]
	cmp	w1, 4
	bne	.L708
	ldr	x5, [x0, 3376]
	add	x2, x27, 2048
	ldrb	w0, [x29, 112]
	ldr	w1, [x29, 116]
	ldr	x3, [x20, 16]
	add	w1, w26, w1
	add	x3, x3, 8
	blr	x5
	cbnz	w0, .L743
.L708:
	add	x20, x20, 32
	b	.L704
.L728:
	mov	x27, x1
	b	.L713
.L714:
	mov	w0, -1
	str	w0, [x20]
	b	.L715
.L727:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 160
	ret
	.size	FlashProgPages, .-FlashProgPages
	.align	2
	.global	FtlLowFormatEraseBlock
	.type	FtlLowFormatEraseBlock, %function
FtlLowFormatEraseBlock:
	stp	x29, x30, [sp, -112]!
	mov	w10, 0
	mov	w9, 4
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	add	x5, x21, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	stp	x23, x24, [sp, 48]
	and	w22, w1, 255
	stp	x25, x26, [sp, 64]
	add	x8, x5, 264
	stp	x27, x28, [sp, 80]
	and	w23, w0, 65535
	mov	w20, 0
	mov	w19, 0
	str	w23, [x5, 3428]
.L745:
	ldrh	w0, [x5, 236]
	cmp	w0, w10
	bhi	.L749
	cbz	w20, .L744
	ldr	x0, [x5, 3408]
	mov	w2, w20
	mov	w1, 0
	ubfiz	x20, x20, 5, 16
	mov	x24, 0
	add	x25, x21, :lo12:.LANCHOR0
	bl	FlashEraseBlocks
.L753:
	ldr	x0, [x25, 3408]
	add	x1, x0, x24
	ldr	w0, [x0, x24]
	cmn	w0, #1
	bne	.L752
	ldr	w0, [x1, 4]
	add	w19, w19, 1
	and	w19, w19, 65535
	lsr	w0, w0, 10
	bl	FtlBbmMapBadBlock
.L752:
	add	x24, x24, 32
	cmp	x24, x20
	bne	.L753
	cbz	w22, .L766
	add	x0, x21, :lo12:.LANCHOR0
	mov	w25, 1
	ldrh	w26, [x0, 308]
.L754:
	add	x28, x21, :lo12:.LANCHOR0
	mov	w24, 0
	add	x0, x28, 264
	str	x0, [x29, 104]
.L762:
	mov	w5, 0
	mov	w20, 0
.L755:
	ldrh	w0, [x28, 236]
	cmp	w0, w5
	bhi	.L758
	cbz	w20, .L744
	ldr	x0, [x28, 3408]
	mov	w1, w20
	mov	w3, 1
	mov	w2, w25
	mov	x27, 0
	bl	FlashProgPages
	ubfiz	x1, x20, 5, 16
.L761:
	ldr	x0, [x28, 3408]
	add	x3, x0, x27
	ldr	w0, [x0, x27]
	cbz	w0, .L760
	ldr	w0, [x3, 4]
	add	w19, w19, 1
	str	x1, [x29, 96]
	and	w19, w19, 65535
	lsr	w0, w0, 10
	bl	FtlBbmMapBadBlock
	ldr	x1, [x29, 96]
.L760:
	add	x27, x27, 32
	cmp	x1, x27
	bne	.L761
	add	w24, w24, 1
	cmp	w26, w24, uxth
	bhi	.L762
	add	x26, x21, :lo12:.LANCHOR0
	mov	x24, 0
.L764:
	cbz	w22, .L763
	ldr	x0, [x26, 3408]
	add	x1, x0, x24
	ldr	w0, [x0, x24]
	cbnz	w0, .L763
	ldr	w0, [x1, 4]
	mov	w1, 1
	lsr	w0, w0, 10
	bl	FtlFreeSysBlkQueueIn
.L763:
	add	x24, x24, 32
	cmp	x24, x27
	bne	.L764
	cmp	w23, 63
	ccmp	w22, 0, 0, hi
	beq	.L744
	add	x21, x21, :lo12:.LANCHOR0
	mov	w2, w20
	mov	w1, w25
	ldr	x0, [x21, 3408]
	bl	FlashEraseBlocks
.L744:
	mov	w0, w19
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
.L749:
	ldr	x1, [x5, 3408]
	ubfiz	x0, x10, 5, 16
	str	wzr, [x1, x0]
	mov	w1, w23
	ldrb	w0, [x8, w10, sxtw]
	bl	V2P_block
	and	w6, w0, 65535
	mov	w7, w6
	cbz	w22, .L746
	bl	IsBlkInVendorPart
	cbnz	w0, .L747
.L746:
	mov	w0, w7
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L748
	ldr	x0, [x5, 3408]
	ubfiz	x2, x20, 5, 16
	lsl	w6, w6, 10
	add	x0, x0, x2
	str	w6, [x0, 4]
	ldr	x1, [x5, 3408]
	ldr	x0, [x5, 3488]
	add	x1, x1, x2
	ldr	x2, [x5, 3528]
	str	x0, [x1, 8]
	ldrh	w0, [x5, 316]
	mul	w0, w0, w20
	add	w20, w20, 1
	and	w20, w20, 65535
	sdiv	w0, w0, w9
	add	x0, x2, x0, sxtw 2
	str	x0, [x1, 16]
.L747:
	add	w10, w10, 1
	and	w10, w10, 65535
	b	.L745
.L748:
	add	w19, w19, 1
	and	w19, w19, 65535
	b	.L747
.L766:
	mov	w25, 0
	mov	w26, 2
	b	.L754
.L758:
	ldr	x1, [x28, 3408]
	ubfiz	x0, x5, 5, 16
	str	wzr, [x1, x0]
	mov	w1, w23
	ldr	x0, [x29, 104]
	ldrb	w0, [x0, w5, sxtw]
	bl	V2P_block
	and	w6, w0, 65535
	mov	w7, w6
	cbz	w22, .L756
	bl	IsBlkInVendorPart
	cbnz	w0, .L757
.L756:
	mov	w0, w7
	bl	FtlBbmIsBadBlock
	cbnz	w0, .L757
	ldr	x0, [x28, 3408]
	ubfiz	x2, x20, 5, 16
	add	w6, w24, w6, lsl 10
	add	x0, x0, x2
	str	w6, [x0, 4]
	ldr	x1, [x28, 3408]
	ldr	x0, [x28, 3480]
	add	x1, x1, x2
	mov	w2, 4
	str	x0, [x1, 8]
	ldrh	w0, [x28, 316]
	mul	w0, w0, w20
	add	w20, w20, 1
	and	w20, w20, 65535
	sdiv	w0, w0, w2
	ldr	x2, [x28, 3488]
	add	x0, x2, x0, sxtw 2
	str	x0, [x1, 16]
.L757:
	add	w5, w5, 1
	and	w5, w5, 65535
	b	.L755
	.size	FtlLowFormatEraseBlock, .-FtlLowFormatEraseBlock
	.align	2
	.global	FlashTestBlk
	.type	FlashTestBlk, %function
FlashTestBlk:
	stp	x29, x30, [sp, -160]!
	adrp	x1, __stack_chk_guard
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	and	w19, w0, 65535
	str	x21, [sp, 32]
	add	x0, x1, :lo12:__stack_chk_guard
	cmp	w19, 11
	mov	x21, x1
	ldr	x2, [x0]
	str	x2, [x29, 152]
	mov	x2,0
	bls	.L791
	adrp	x20, .LANCHOR0
	add	x20, x20, :lo12:.LANCHOR0
	mov	w2, 32
	mov	w1, 165
	lsl	w19, w19, 10
	ldr	x0, [x20, 3504]
	str	x0, [x29, 64]
	add	x0, x29, 88
	str	x0, [x29, 72]
	bl	ftl_memset
	ldr	x0, [x20, 3504]
	mov	w2, 8
	mov	w1, 90
	bl	ftl_memset
	str	w19, [x29, 60]
	mov	w2, 1
	add	x0, x29, 56
	mov	w1, w2
	bl	FlashEraseBlocks
	mov	w3, 1
	add	x0, x29, 56
	mov	w2, w3
	mov	w1, w3
	bl	FlashProgPages
	ldr	w0, [x29, 56]
	cbnz	w0, .L792
	add	w0, w19, 1
	mov	w3, 1
	str	w0, [x29, 60]
	mov	w2, w3
	mov	w1, w3
	add	x0, x29, 56
	bl	FlashProgPages
	ldr	w0, [x29, 56]
	cmp	w0, 0
	csetm	w20, ne
.L789:
	mov	w2, 1
	mov	w1, 0
	add	x0, x29, 56
	str	w19, [x29, 60]
	bl	FlashEraseBlocks
.L787:
	add	x1, x21, :lo12:__stack_chk_guard
	mov	w0, w20
	ldr	x2, [x29, 152]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L790
	bl	__stack_chk_fail
.L792:
	mov	w20, -1
	b	.L789
.L791:
	mov	w20, 0
	b	.L787
.L790:
	ldp	x19, x20, [sp, 16]
	ldr	x21, [sp, 32]
	ldp	x29, x30, [sp], 160
	ret
	.size	FlashTestBlk, .-FlashTestBlk
	.align	2
	.global	FtlBbmTblFlush
	.type	FtlBbmTblFlush, %function
FtlBbmTblFlush:
	stp	x29, x30, [sp, -80]!
	mov	w1, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	w19, 0
	stp	x25, x26, [sp, 64]
	adrp	x25, .LANCHOR0
	add	x20, x25, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	add	x21, x20, 392
	ldr	x2, [x20, 3520]
	ldr	x0, [x20, 3456]
	str	x2, [x20, 3736]
	ldrh	w2, [x20, 314]
	str	x0, [x20, 3728]
	bl	ftl_memset
.L795:
	ldrh	w0, [x20, 258]
	add	x1, x20, 3720
	cmp	w19, w0
	blt	.L796
	ldr	x26, [x1, 16]
	add	x19, x20, 360
	mov	w2, 16
	mov	w1, 255
	mov	x0, x26
	bl	ftl_memset
	mov	w0, -3887
	adrp	x23, .LC99
	strh	w0, [x26]
	add	x25, x25, :lo12:.LANCHOR0
	add	x23, x23, :lo12:.LC99
	mov	w21, 0
	ldr	w0, [x19, 8]
	mov	w22, 0
	str	w0, [x26, 4]
	adrp	x24, .LC100
	ldrh	w0, [x20, 360]
	strh	w0, [x26, 2]
	ldrh	w0, [x19, 4]
	strh	w0, [x26, 8]
	ldrh	w0, [x19, 6]
	strh	w0, [x26, 10]
	ldr	w0, [x20, 232]
	strh	w0, [x26, 12]
	ldr	w0, [x20, 2676]
	strh	w0, [x26, 14]
.L797:
	ldr	x0, [x25, 3456]
	str	x0, [x25, 3728]
	ldr	x0, [x25, 3520]
	str	x0, [x25, 3736]
	ldrh	w1, [x19]
	ldrh	w2, [x19, 2]
	ldrh	w3, [x19, 4]
	ldrh	w4, [x26, 10]
	orr	w0, w2, w1, lsl 10
	str	wzr, [x25, 3720]
	str	w0, [x25, 3724]
	mov	x0, x23
	bl	sftl_printk
	ldrh	w0, [x25, 308]
	ldrh	w1, [x19, 2]
	sub	w0, w0, #1
	cmp	w1, w0
	blt	.L798
	ldr	w0, [x19, 8]
	mov	w2, 1
	ldrh	w1, [x19]
	add	w0, w0, 1
	str	w0, [x19, 8]
	str	w0, [x26, 4]
	strh	w1, [x26, 8]
	ldrh	w0, [x19, 4]
	strh	w1, [x19, 4]
	ldr	x1, [x25, 3408]
	strh	w0, [x19]
	lsl	w0, w0, 10
	str	w0, [x25, 3724]
	strh	wzr, [x19, 2]
	str	w0, [x1, 4]
	mov	w1, w2
	ldr	x0, [x25, 3408]
	bl	FlashEraseBlocks
.L798:
	add	x20, x25, 3720
	mov	w3, 1
	mov	x0, x20
	mov	w2, w3
	mov	w1, w3
	bl	FlashProgPages
	ldrh	w0, [x19, 2]
	add	w0, w0, 1
	strh	w0, [x19, 2]
	ldr	w0, [x25, 3720]
	cmn	w0, #1
	bne	.L799
	ldr	w1, [x25, 3724]
	add	w21, w21, 1
	add	x0, x24, :lo12:.LC100
	and	w21, w21, 65535
	bl	sftl_printk
	cmp	w21, 3
	bls	.L797
	ldr	w1, [x25, 3724]
	adrp	x0, .LC101
	mov	w2, w21
	add	x0, x0, :lo12:.LC101
	bl	sftl_printk
.L801:
	b	.L801
.L796:
	ldrh	w2, [x20, 3632]
	ldr	x0, [x1, 8]
	ldr	x1, [x21], 8
	mul	w3, w19, w2
	lsl	w2, w2, 2
	add	w19, w19, 1
	add	x0, x0, x3, sxtw 2
	bl	ftl_memcpy
	b	.L795
.L802:
	mov	w22, 1
	b	.L797
.L799:
	add	w22, w22, 1
	cmp	w22, 1
	ble	.L802
	cmp	w0, 256
	beq	.L797
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
	.size	FtlBbmTblFlush, .-FtlBbmTblFlush
	.align	2
	.global	allocate_data_superblock
	.type	allocate_data_superblock, %function
allocate_data_superblock:
	stp	x29, x30, [sp, -112]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	add	x22, x21, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	stp	x23, x24, [sp, 48]
	mov	x20, x0
	mov	x19, x22
	add	x23, x22, 128
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
.L808:
	ldrh	w1, [x22, 2584]
	ldrh	w0, [x22, 228]
	add	w0, w0, w1
	ldrh	w1, [x22, 244]
	cmp	w0, w1
	ble	.L809
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2667
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L809:
	cmp	x20, x23
	bne	.L835
	ldrh	w0, [x19, 228]
	ldr	w1, [x19, 3236]
	mul	w1, w0, w1
	lsr	w0, w0, 1
	add	w0, w0, 1
	add	w1, w0, w1, lsr 2
	ands	w1, w1, 65535
	beq	.L810
	sub	w1, w1, #1
	and	w1, w1, 65535
.L810:
	add	x0, x19, 2592
	bl	List_pop_index_node
	and	w26, w0, 65535
	ldrh	w0, [x19, 228]
	cbnz	w0, .L811
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2676
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L811:
	ldrh	w0, [x19, 228]
	sub	w0, w0, #1
	strh	w0, [x19, 228]
	ldrh	w0, [x19, 244]
	cmp	w0, w26
	bls	.L808
	ldr	x0, [x19, 72]
	ubfiz	x25, x26, 1, 16
	ldrh	w24, [x0, x25]
	cbnz	w24, .L808
	strh	w26, [x20]
	mov	x0, x20
	bl	make_superblock
	ldrb	w0, [x20, 7]
	cbnz	w0, .L813
	ldr	x0, [x19, 72]
	mov	w1, -1
	strh	w1, [x0, x25]
	mov	w0, w26
	bl	INSERT_DATA_LIST
	ldrh	w1, [x19, 2584]
	ldrh	w0, [x19, 228]
	add	w0, w0, w1
	ldrh	w1, [x19, 244]
	cmp	w0, w1
	ble	.L808
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	mov	w2, 2690
	add	x1, x1, 336
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L808
.L835:
	mov	w1, 0
	b	.L810
.L813:
	ldrh	w1, [x19, 2584]
	ldrh	w0, [x19, 228]
	add	w0, w0, w1
	ldrh	w1, [x19, 244]
	cmp	w0, w1
	ble	.L815
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2693
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L815:
	add	x27, x20, 16
	ldrh	w4, [x19, 236]
	mov	x3, x27
	mov	x0, 0
	mov	w5, 65535
.L816:
	cmp	w4, w0, uxth
	bhi	.L818
	cbnz	w24, .L819
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2704
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L819:
	ldrh	w0, [x19, 176]
	cmp	w0, w26
	bne	.L820
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2706
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L820:
	ldrb	w0, [x20, 8]
	ldr	x1, [x19, 2600]
	cbnz	w0, .L821
	ldrh	w0, [x1, x25]
	cbz	w0, .L822
	ldrh	w2, [x19, 296]
	add	w0, w0, w2
.L843:
	strh	w0, [x1, x25]
	ldr	w0, [x19, 2668]
	add	w0, w0, 1
	str	w0, [x19, 2668]
.L824:
	ldr	x0, [x19, 2600]
	ldr	w1, [x19, 2684]
	ldrh	w0, [x0, x25]
	cmp	w0, w1
	bls	.L825
	str	w0, [x19, 2684]
.L825:
	ldr	w1, [x19, 2672]
	ubfiz	x3, x24, 5, 16
	ldrh	w0, [x19, 296]
	ldr	w2, [x19, 2668]
	madd	w0, w0, w2, w1
	ldrh	w1, [x19, 244]
	udiv	w0, w0, w1
	ldr	x1, [x19, 3552]
	str	w0, [x19, 2676]
	ldr	w0, [x1, 16]
	add	w0, w0, 1
	str	w0, [x1, 16]
	mov	x0, 0
.L826:
	cmp	x3, x0
	bne	.L827
	ldrb	w1, [x20, 8]
	mov	w2, w24
	ldr	x0, [x19, 3408]
	mov	x28, 0
	bl	FlashEraseBlocks
	mov	w1, 0
.L828:
	cmp	w24, w28, uxth
	bhi	.L830
	cmp	w1, 0
	ble	.L831
	mov	w0, w26
	bl	update_multiplier_value
	bl	FtlBbmTblFlush
.L831:
	ldrb	w0, [x20, 7]
	cbnz	w0, .L832
	ldr	x0, [x19, 72]
	mov	w1, -1
	strh	w1, [x0, x25]
	b	.L808
.L818:
	ldr	x1, [x19, 3408]
	add	x2, x1, x0, lsl 5
	stp	xzr, xzr, [x2, 8]
	ldrh	w2, [x3]
	cmp	w2, w5
	beq	.L817
	ubfiz	x6, x24, 5, 16
	add	w24, w24, 1
	add	x1, x1, x6
	and	w24, w24, 65535
	lsl	w2, w2, 10
	str	w2, [x1, 4]
.L817:
	add	x0, x0, 1
	add	x3, x3, 2
	b	.L816
.L822:
	mov	w0, 2
	b	.L843
.L821:
	ldrh	w0, [x1, x25]
	add	w0, w0, 1
	strh	w0, [x1, x25]
	ldr	w0, [x19, 2672]
	add	w0, w0, 1
	str	w0, [x19, 2672]
	b	.L824
.L827:
	ldr	x1, [x19, 3408]
	add	x1, x1, x0
	add	x0, x0, 32
	ldr	w2, [x1, 4]
	and	w2, w2, -1024
	str	w2, [x1, 4]
	b	.L826
.L830:
	ldr	x2, [x19, 3408]
	lsl	x0, x28, 5
	add	x3, x2, x0
	ldr	w2, [x2, x0]
	cmn	w2, #1
	bne	.L829
	add	w1, w1, 1
	ldr	w0, [x3, 4]
	stp	w2, w1, [x29, 104]
	lsr	w0, w0, 10
	bl	FtlBbmMapBadBlock
	ldp	w2, w1, [x29, 104]
	strh	w2, [x27]
	ldrb	w0, [x20, 7]
	sub	w0, w0, #1
	strb	w0, [x20, 7]
.L829:
	add	x28, x28, 1
	add	x27, x27, 2
	b	.L828
.L832:
	add	x21, x21, :lo12:.LANCHOR0
	strh	wzr, [x20, 2]
	strb	wzr, [x20, 6]
	ldrh	w1, [x21, 306]
	strh	w26, [x20]
	mul	w0, w0, w1
	ldr	w1, [x21, 2660]
	str	w1, [x20, 12]
	and	w0, w0, 65535
	add	w1, w1, 1
	strh	w0, [x20, 4]
	str	w1, [x21, 2660]
	ldr	x1, [x21, 72]
	ldrh	w2, [x20]
	strh	w0, [x1, x2, lsl 1]
	ldrh	w0, [x20, 4]
	cbz	w0, .L833
	ldrb	w0, [x20, 7]
	cbnz	w0, .L834
.L833:
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2759
	add	x1, x1, 336
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L834:
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
	.size	allocate_data_superblock, .-allocate_data_superblock
	.align	2
	.global	FtlGcFreeBadSuperBlk
	.type	FtlGcFreeBadSuperBlk, %function
FtlGcFreeBadSuperBlk:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	and	w22, w0, 65535
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	ldrh	w0, [x19, 3312]
	str	x27, [sp, 80]
	cbz	w0, .L845
	add	x23, x19, 264
	add	x24, x19, 3320
	mov	w21, 0
.L846:
	ldrh	w0, [x19, 236]
	cmp	w0, w21
	bhi	.L852
	bl	FtlGcReFreshBadBlk
.L845:
	mov	w0, 0
	ldr	x27, [sp, 80]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 96
	ret
.L852:
	ldrb	w0, [x23, w21, sxtw]
	mov	w1, w22
	adrp	x26, .LC102
	add	x26, x26, :lo12:.LC102
	mov	w20, 0
	bl	V2P_block
	and	w25, w0, 65535
.L847:
	ldrh	w0, [x19, 3312]
	cmp	w0, w20
	bhi	.L851
	add	w21, w21, 1
	and	w21, w21, 65535
	b	.L846
.L851:
	ldrh	w0, [x24, w20, sxtw 1]
	add	w27, w20, 1
	cmp	w0, w25
	bne	.L848
	mov	w1, w25
	mov	x0, x26
	bl	sftl_printk
	mov	w0, w25
	bl	FtlBbmMapBadBlock
	bl	FtlBbmTblFlush
	ldrh	w0, [x19, 3312]
	mov	w1, w27
.L849:
	cmp	w20, w0
	bcc	.L850
	sub	w0, w0, #1
	strh	w0, [x19, 3312]
.L848:
	and	w20, w27, 65535
	b	.L847
.L850:
	sub	w2, w1, #1
	ldrh	w3, [x24, w1, sxtw 1]
	add	w20, w20, 1
	add	w1, w1, 1
	and	w20, w20, 65535
	strh	w3, [x24, w2, sxtw 1]
	b	.L849
	.size	FtlGcFreeBadSuperBlk, .-FtlGcFreeBadSuperBlk
	.align	2
	.global	update_vpc_list
	.type	update_vpc_list, %function
update_vpc_list:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	and	w20, w0, 65535
	adrp	x19, .LANCHOR0
	add	x0, x19, :lo12:.LANCHOR0
	str	x21, [sp, 32]
	ubfiz	x1, x20, 1, 16
	ldr	x2, [x0, 72]
	ldrh	w1, [x2, x1]
	cbnz	w1, .L858
	ldrh	w1, [x0, 226]
	cmp	w1, w20
	bne	.L859
	mov	w1, -1
	strh	w1, [x0, 226]
.L859:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 224]
	cmp	w1, w20
	bne	.L860
	mov	w1, -1
	strh	w1, [x0, 224]
.L860:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 176]
	cmp	w1, w20
	bne	.L861
	mov	w1, -1
	strh	w1, [x0, 176]
.L862:
	add	x21, x19, :lo12:.LANCHOR0
	mov	w1, w20
	add	x0, x21, 2568
	bl	List_remove_node
	ldrh	w0, [x21, 2584]
	cbnz	w0, .L864
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2835
	add	x1, x1, 368
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L864:
	add	x19, x19, :lo12:.LANCHOR0
	ldrh	w0, [x19, 2584]
	sub	w0, w0, #1
	strh	w0, [x19, 2584]
	mov	w0, w20
	bl	free_data_superblock
	mov	w0, w20
	bl	FtlGcFreeBadSuperBlk
	ldrh	w1, [x19, 2584]
	ldrh	w0, [x19, 228]
	add	w0, w0, w1
	ldrh	w1, [x19, 244]
	cmp	w0, w1
	ble	.L868
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2838
	add	x1, x1, 368
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L868:
	mov	w0, 1
	b	.L857
.L861:
	ldrh	w1, [x0, 24]
	cmp	w1, w20
	beq	.L867
	ldrh	w1, [x0, 80]
	cmp	w1, w20
	beq	.L867
	ldrh	w0, [x0, 128]
	cmp	w0, w20
	bne	.L862
.L867:
	mov	w0, 0
.L857:
	ldp	x19, x20, [sp, 16]
	ldr	x21, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
.L858:
	mov	w0, w20
	bl	List_update_data_list
	b	.L867
	.size	update_vpc_list, .-update_vpc_list
	.align	2
	.global	decrement_vpc_count
	.type	decrement_vpc_count, %function
decrement_vpc_count:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	and	w20, w0, 65535
	stp	x21, x22, [sp, 32]
	mov	w0, 65535
	adrp	x19, .LANCHOR0
	cmp	w20, w0
	beq	.L871
	add	x22, x19, :lo12:.LANCHOR0
	ubfiz	x21, x20, 1, 16
	ldr	x1, [x22, 72]
	ldrh	w0, [x1, x21]
	cbnz	w0, .L872
	mov	w2, 0
	mov	w1, w20
	adrp	x0, .LC103
	add	x0, x0, :lo12:.LC103
	bl	sftl_printk
	ldr	x0, [x22, 72]
	ldrh	w0, [x0, x21]
	cbz	w0, .L873
.L878:
	mov	w21, 0
.L870:
	mov	w0, w21
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
.L873:
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	mov	w2, 2853
	add	x1, x1, 384
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L878
.L872:
	sub	w0, w0, #1
	strh	w0, [x1, x21]
.L871:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w1, 65535
	ldrh	w0, [x19, 3706]
	cmp	w0, w1
	bne	.L875
	strh	w20, [x19, 3706]
	b	.L878
.L875:
	cmp	w20, w0
	beq	.L878
	bl	update_vpc_list
	cmp	w0, 0
	ldr	x1, [x19, 2560]
	cset	w21, ne
	ldr	x0, [x19, 2568]
	strh	w20, [x19, 3706]
	sub	x0, x0, x1
	mov	x1, -6148914691236517206
	asr	x0, x0, 1
	movk	x1, 0xaaab, lsl 0
	mul	x0, x0, x1
	ldr	x1, [x19, 72]
	and	x2, x0, 65535
	ldrh	w1, [x1, x2, lsl 1]
	cbnz	w1, .L870
	cmp	w20, w0, uxth
	beq	.L870
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	mov	w2, 2869
	add	x1, x1, 384
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L870
	.size	decrement_vpc_count, .-decrement_vpc_count
	.align	2
	.global	FtlSuperblockPowerLostFix
	.type	FtlSuperblockPowerLostFix, %function
FtlSuperblockPowerLostFix:
	stp	x29, x30, [sp, -112]!
	mov	w2, 61589
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	stp	x23, x24, [sp, 48]
	adrp	x23, __stack_chk_guard
	add	x0, x23, :lo12:__stack_chk_guard
	ldr	x1, [x0]
	str	x1, [x29, 104]
	mov	x1,0
	mov	w0, -1
	str	w0, [x29, 96]
	add	x0, x21, :lo12:.LANCHOR0
	mov	x20, x0
	ldr	x24, [x0, 3520]
	ldr	x1, [x0, 3456]
	stp	x1, x24, [x29, 80]
	mov	w1, -3
	str	w1, [x24, 8]
	mov	w1, -2
	str	w1, [x24, 12]
	ldrh	w1, [x19]
	strh	w1, [x24, 2]
	strh	wzr, [x24]
	ldr	x1, [x0, 3456]
	str	w2, [x1]
	mov	w2, 22136
	movk	w2, 0x1234, lsl 16
	ldr	x1, [x0, 3456]
	str	w2, [x1, 4]
	ldrh	w22, [x19, 4]
	and	w22, w22, 1
	add	w22, w22, 6
.L885:
	ldrh	w0, [x19, 4]
	cbnz	w0, .L881
.L882:
	add	x21, x21, :lo12:.LANCHOR0
	ldrh	w0, [x19]
	ldrh	w3, [x19, 4]
	add	x23, x23, :lo12:__stack_chk_guard
	ldr	x2, [x21, 72]
	lsl	x0, x0, 1
	ldrh	w1, [x2, x0]
	sub	w1, w1, w3
	strh	w1, [x2, x0]
	strb	wzr, [x19, 6]
	ldrh	w0, [x21, 306]
	strh	w0, [x19, 2]
	strh	wzr, [x19, 4]
	ldr	x1, [x29, 104]
	ldr	x0, [x23]
	eor	x0, x1, x0
	cbz	x0, .L886
	bl	__stack_chk_fail
.L881:
	mov	x0, x19
	bl	get_new_active_ppa
	str	w0, [x29, 76]
	cmn	w0, #1
	beq	.L882
	ldr	w0, [x20, 2664]
	mov	w3, 0
	str	w0, [x24, 4]
	mov	w2, 0
	add	w0, w0, 1
	mov	w1, 1
	cmn	w0, #1
	csel	w0, w0, wzr, ne
	str	w0, [x20, 2664]
	add	x0, x29, 72
	bl	FlashProgPages
	ldrh	w0, [x19]
	bl	decrement_vpc_count
	subs	w22, w22, #1
	bne	.L885
	b	.L882
.L886:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 112
	ret
	.size	FtlSuperblockPowerLostFix, .-FtlSuperblockPowerLostFix
	.align	2
	.global	FtlMakeBbt
	.type	FtlMakeBbt, %function
FtlMakeBbt:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	stp	x25, x26, [sp, 64]
	stp	x21, x22, [sp, 32]
	mov	x21, x19
	stp	x23, x24, [sp, 48]
	mov	w24, 0
	stp	x27, x28, [sp, 80]
	bl	FtlBbtMemInit
	bl	FtlLoadFactoryBbt
	mov	w28, -3872
	add	x0, x19, :lo12:.LANCHOR0
	add	x26, x0, 392
	add	x25, x0, 372
	mov	x19, x0
	add	x20, x0, 3720
.L895:
	ldrh	w0, [x19, 258]
	cmp	w24, w0
	bcc	.L901
	add	x20, x21, :lo12:.LANCHOR0
	mov	w19, 0
.L902:
	ldrh	w0, [x20, 318]
	cmp	w0, w19
	bhi	.L903
	add	x20, x20, 360
	ldrh	w19, [x20, 12]
	mov	w22, 65535
	sub	w19, w19, #1
	and	w19, w19, 65535
.L904:
	ldrh	w0, [x20, 12]
	sub	w0, w0, #48
	cmp	w19, w0
	ble	.L908
	mov	w0, w19
	bl	FtlBbmIsBadBlock
	cmp	w0, 1
	beq	.L905
	mov	w0, w19
	bl	FlashTestBlk
	cbz	w0, .L906
	mov	w0, w19
	bl	FtlBbmMapBadBlock
.L905:
	sub	w19, w19, #1
	and	w19, w19, 65535
	b	.L904
.L901:
	ldr	x0, [x19, 3456]
	mov	w2, 65535
	ldr	x27, [x19, 3520]
	ldrh	w1, [x25]
	stp	x0, x27, [x20, 8]
	cmp	w1, w2
	beq	.L896
	ldrh	w23, [x19, 302]
	mov	w2, 1
	madd	w23, w23, w24, w1
	mov	w1, w2
	lsl	w0, w23, 10
	str	w0, [x20, 4]
	mov	x0, x20
	bl	FlashReadPages
	ldr	x1, [x20, 8]
	ldr	x0, [x26]
	ldrh	w2, [x19, 302]
	add	w2, w2, 7
	lsr	w2, w2, 3
	bl	ftl_memcpy
.L897:
	mov	w0, w23
	add	w24, w24, 1
	bl	FtlBbmMapBadBlock
	add	x26, x26, 8
	add	x25, x25, 2
	b	.L895
.L896:
	mov	w1, w24
	bl	FlashGetBadBlockList
	ldr	x0, [x20, 8]
	ldr	x1, [x26]
	bl	FtlBbt2Bitmap
	ldrh	w22, [x19, 302]
.L899:
	sub	w22, w22, #1
	and	w22, w22, 65535
.L898:
	ldrh	w0, [x19, 302]
	madd	w0, w24, w0, w22
	bl	FtlBbmIsBadBlock
	cmp	w0, 1
	beq	.L899
	ldr	x0, [x19, 3520]
	mov	w2, 16
	strh	w22, [x25]
	mov	w1, 0
	bl	ftl_memset
	ldr	x0, [x19, 3456]
	mov	w2, 4096
	mov	w1, 0
	bl	ftl_memset
	strh	w28, [x27]
	str	wzr, [x27, 4]
	ldrh	w23, [x19, 302]
	ldrh	w0, [x25]
	strh	w0, [x27, 2]
	ldrh	w2, [x19, 3632]
	ldrh	w0, [x25]
	ldr	x1, [x26]
	lsl	w2, w2, 2
	madd	w23, w23, w24, w0
	lsl	w0, w23, 10
	str	w0, [x20, 4]
	ldr	x0, [x20, 8]
	bl	ftl_memcpy
	mov	w2, 1
	mov	x0, x20
	mov	w1, w2
	bl	FlashEraseBlocks
	mov	w3, 1
	mov	x0, x20
	mov	w2, w3
	mov	w1, w3
	bl	FlashProgPages
	ldr	w0, [x20]
	cmn	w0, #1
	bne	.L897
	mov	w0, w23
	bl	FtlBbmMapBadBlock
	b	.L898
.L903:
	mov	w0, w19
	add	w19, w19, 1
	bl	FtlBbmMapBadBlock
	and	w19, w19, 65535
	b	.L902
.L906:
	ldrh	w0, [x20]
	cmp	w0, w22
	bne	.L907
	strh	w19, [x20]
	b	.L905
.L907:
	strh	w19, [x20, 4]
.L908:
	add	x21, x21, :lo12:.LANCHOR0
	mov	w2, 2
	ldr	x1, [x21, 3408]
	ldrh	w0, [x21, 360]
	str	wzr, [x21, 368]
	strh	wzr, [x21, 362]
	lsl	w0, w0, 10
	str	w0, [x1, 4]
	ldr	x0, [x21, 3408]
	ldrh	w1, [x21, 364]
	lsl	w1, w1, 10
	str	w1, [x0, 36]
	mov	w1, 1
	bl	FlashEraseBlocks
	ldrh	w0, [x21, 360]
	bl	FtlBbmMapBadBlock
	ldrh	w0, [x21, 364]
	bl	FtlBbmMapBadBlock
	bl	FtlBbmTblFlush
	strh	wzr, [x21, 362]
	ldr	w0, [x21, 368]
	ldrh	w1, [x21, 364]
	add	w0, w0, 1
	str	w0, [x21, 368]
	ldrh	w0, [x21, 360]
	strh	w0, [x21, 364]
	strh	w1, [x21, 360]
	bl	FtlBbmTblFlush
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 96
	ret
	.size	FtlMakeBbt, .-FtlMakeBbt
	.align	2
	.global	ftl_memcmp
	.type	ftl_memcmp, %function
ftl_memcmp:
	stp	x29, x30, [sp, -16]!
	uxtw	x2, w2
	add	x29, sp, 0
	bl	memcmp
	ldp	x29, x30, [sp], 16
	ret
	.size	ftl_memcmp, .-ftl_memcmp
	.align	2
	.global	js_hash
	.type	js_hash, %function
js_hash:
	mov	x4, x0
	mov	w0, 42982
	mov	x3, 0
	movk	w0, 0x47c6, lsl 16
.L917:
	cmp	w1, w3
	bhi	.L918
	ret
.L918:
	lsr	w2, w0, 2
	ldrb	w5, [x4, x3]
	add	w2, w2, w0, lsl 5
	add	x3, x3, 1
	add	w2, w2, w5
	eor	w0, w0, w2
	b	.L917
	.size	js_hash, .-js_hash
	.align	2
	.global	Ftl_write_map_blk_to_last_page
	.type	Ftl_write_map_blk_to_last_page, %function
Ftl_write_map_blk_to_last_page:
	stp	x29, x30, [sp, -64]!
	mov	w1, 65535
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	ldr	x20, [x0, 16]
	ldrh	w0, [x0]
	cmp	w0, w1
	bne	.L920
	ldrh	w0, [x19, 8]
	cbz	w0, .L921
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 641
	add	x1, x1, 408
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L921:
	ldrh	w0, [x19, 8]
	add	w0, w0, 1
	strh	w0, [x19, 8]
	bl	FtlFreeSysBlkQueueOut
	strh	w0, [x20]
	strh	wzr, [x19, 2]
	ldr	w0, [x19, 48]
	strh	wzr, [x19]
	add	w0, w0, 1
	str	w0, [x19, 48]
.L922:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 64
	ret
.L920:
	ubfiz	x0, x0, 1, 16
	adrp	x21, .LANCHOR0
	ldrh	w1, [x19, 2]
	ldr	x24, [x19, 40]
	ldrh	w23, [x20, x0]
	add	x20, x21, :lo12:.LANCHOR0
	ldr	w0, [x19, 48]
	ldr	x22, [x20, 3520]
	orr	w1, w1, w23, lsl 10
	str	w1, [x20, 3724]
	ldr	x1, [x20, 3456]
	str	x1, [x20, 3728]
	str	x22, [x20, 3736]
	mov	w1, 255
	str	w0, [x22, 4]
	mov	w0, -1291
	strh	w0, [x22, 8]
	ldrh	w0, [x19, 4]
	strh	w0, [x22]
	strh	w23, [x22, 2]
	ldrh	w2, [x20, 308]
	ldr	x0, [x20, 3456]
	lsl	w2, w2, 3
	bl	ftl_memset
	mov	w1, 0
	mov	w0, 0
.L923:
	ldrh	w2, [x19, 6]
	cmp	w2, w0
	bhi	.L925
	add	x21, x21, :lo12:.LANCHOR0
	add	x6, x21, 3720
	ldrh	w1, [x21, 314]
	ldr	x0, [x21, 3728]
	bl	js_hash
	str	w0, [x22, 12]
	mov	w2, 1
	mov	w3, 0
	mov	w1, w2
	mov	x0, x6
	bl	FlashProgPages
	ldrh	w0, [x19, 2]
	add	w0, w0, 1
	strh	w0, [x19, 2]
	mov	x0, x19
	bl	ftl_map_blk_gc
	b	.L922
.L925:
	ubfiz	x2, x0, 2, 16
	ldr	w3, [x24, x2]
	cmp	w23, w3, lsr 10
	bne	.L924
	add	w1, w1, 1
	ldr	x4, [x20, 3456]
	and	w1, w1, 65535
	ubfiz	x3, x1, 3, 16
	str	w0, [x4, x3]
	ldr	w4, [x24, x2]
	ldr	x2, [x20, 3456]
	add	x2, x2, x3
	str	w4, [x2, 4]
.L924:
	add	w0, w0, 1
	and	w0, w0, 65535
	b	.L923
	.size	Ftl_write_map_blk_to_last_page, .-Ftl_write_map_blk_to_last_page
	.align	2
	.global	FtlMapWritePage
	.type	FtlMapWritePage, %function
FtlMapWritePage:
	stp	x29, x30, [sp, -112]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	mov	w21, 0
	stp	x23, x24, [sp, 48]
	adrp	x24, .LANCHOR0
	stp	x25, x26, [sp, 64]
	add	x22, x24, :lo12:.LANCHOR0
	mov	w26, w1
	stp	x19, x20, [sp, 16]
	stp	x27, x28, [sp, 80]
	adrp	x23, .LANCHOR1
	mov	x19, x0
	mov	x28, x2
	ubfiz	x27, x26, 2, 32
	mov	x25, x24
	add	x23, x23, :lo12:.LANCHOR1
	add	x20, x22, 3720
.L931:
	add	x0, x24, :lo12:.LANCHOR0
	ldr	w1, [x0, 2644]
	add	w1, w1, 1
	str	w1, [x0, 2644]
	ldrh	w0, [x0, 308]
	ldrh	w1, [x19, 2]
	sub	w0, w0, #1
	cmp	w1, w0
	bge	.L932
	ldrh	w1, [x19]
	mov	w0, 65535
	cmp	w1, w0
	bne	.L933
.L932:
	mov	x0, x19
	bl	Ftl_write_map_blk_to_last_page
.L933:
	ldrh	w1, [x19]
	ldr	x0, [x19, 16]
	ldrh	w0, [x0, x1, lsl 1]
	cbnz	w0, .L934
	adrp	x0, .LC8
	mov	w2, 700
	add	x1, x23, 440
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L934:
	ldrh	w1, [x19]
	ldrh	w0, [x19, 10]
	cmp	w1, w0
	bcc	.L935
	adrp	x0, .LC8
	mov	w2, 701
	add	x1, x23, 440
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L935:
	ldrh	w1, [x19]
	mov	w2, 16
	ldr	x0, [x19, 16]
	ldrh	w3, [x0, x1, lsl 1]
	mov	w1, 0
	ldrh	w0, [x19, 2]
	str	w3, [x29, 108]
	str	x28, [x20, 8]
	orr	w0, w0, w3, lsl 10
	str	w0, [x20, 4]
	ldr	x0, [x22, 3520]
	str	x0, [x20, 16]
	bl	ftl_memset
	ldr	x6, [x20, 16]
	ldr	w0, [x19, 48]
	ldr	w3, [x29, 108]
	strh	w26, [x6, 8]
	str	w0, [x6, 4]
	ldrh	w0, [x19, 4]
	strh	w0, [x6]
	strh	w3, [x6, 2]
	ldrh	w1, [x22, 314]
	ldr	x0, [x20, 8]
	bl	js_hash
	str	w0, [x6, 12]
	mov	w3, 1
	mov	x0, x20
	mov	w1, w3
	mov	w2, w3
	bl	FlashProgPages
	ldrh	w0, [x19, 2]
	add	w0, w0, 1
	strh	w0, [x19, 2]
	ldp	w0, w1, [x20]
	cmn	w0, #1
	bne	.L936
	adrp	x0, .LC104
	add	x0, x0, :lo12:.LC104
	bl	sftl_printk
	add	w21, w21, 1
	ldrh	w0, [x19, 2]
	and	w21, w21, 65535
	cmp	w0, 2
	bhi	.L937
	ldrh	w0, [x22, 308]
	sub	w0, w0, #1
	strh	w0, [x19, 2]
.L937:
	cmp	w21, 3
	bls	.L938
	add	x25, x25, :lo12:.LANCHOR0
	adrp	x0, .LC105
	mov	w2, w21
	add	x0, x0, :lo12:.LC105
	ldr	w1, [x25, 3724]
	bl	sftl_printk
.L939:
	b	.L939
.L938:
	ldr	w0, [x19, 52]
	cbz	w0, .L931
.L954:
	b	.L954
.L936:
	ldr	x0, [x19, 40]
	str	w1, [x0, x27]
	ldrh	w0, [x19, 2]
	cmp	w0, 1
	beq	.L942
	ldr	w0, [x20]
	cmp	w0, 256
	beq	.L942
	ldr	w1, [x19, 56]
	cbz	w1, .L943
.L942:
	str	wzr, [x19, 56]
	b	.L931
.L943:
	cmn	w0, #1
	bne	.L944
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 745
	add	x1, x1, 440
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L944:
	ldp	x19, x20, [sp, 16]
	mov	w0, 0
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 112
	ret
	.size	FtlMapWritePage, .-FtlMapWritePage
	.align	2
	.global	load_l2p_region
	.type	load_l2p_region, %function
load_l2p_region:
	stp	x29, x30, [sp, -80]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	and	w22, w0, 65535
	adrp	x21, .LANCHOR0
	add	x0, x21, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	and	x19, x1, 65535
	stp	x23, x24, [sp, 48]
	str	x25, [sp, 64]
	ldrh	w0, [x0, 340]
	cmp	w0, w22
	bcs	.L957
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 485
	add	x1, x1, 456
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L957:
	add	x20, x21, :lo12:.LANCHOR0
	ubfiz	x0, x22, 2, 16
	ldr	x1, [x20, 3608]
	ldr	w24, [x1, x0]
	cbnz	w24, .L958
	ldr	x0, [x20, 2608]
	lsl	x19, x19, 4
	ldrh	w2, [x20, 314]
	mov	w1, 255
	add	x0, x0, x19
	ldr	x0, [x0, 8]
	bl	ftl_memset
	ldr	x0, [x20, 2608]
	strh	w22, [x0, x19]
	ldr	x0, [x20, 2608]
	add	x19, x0, x19
	str	wzr, [x19, 4]
.L959:
	mov	w0, 0
	ldr	x25, [sp, 64]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x29, x30, [sp], 80
	ret
.L958:
	ldr	x0, [x20, 2608]
	lsl	x19, x19, 4
	add	x23, x20, 3720
	mov	w2, 1
	add	x0, x0, x19
	mov	w1, w2
	str	w24, [x20, 3724]
	ldr	x0, [x0, 8]
	str	x0, [x20, 3728]
	ldr	x0, [x20, 3520]
	str	x0, [x20, 3736]
	mov	x0, x23
	bl	FlashReadPages
	ldr	x25, [x20, 3736]
	ldrh	w0, [x25, 8]
	cmp	w0, w22
	beq	.L960
	mov	w2, w24
	mov	w1, w22
	adrp	x0, .LC106
	add	x0, x0, :lo12:.LC106
	bl	sftl_printk
	ldr	x1, [x20, 3736]
	mov	w3, 4
	adrp	x0, .LC94
	mov	w2, w3
	add	x0, x0, :lo12:.LC94
	bl	rknand_print_hex
	ldrh	w3, [x20, 340]
	adrp	x0, .LC107
	ldr	x1, [x20, 3608]
	mov	w2, 4
	add	x0, x0, :lo12:.LC107
	bl	rknand_print_hex
.L961:
	ldrh	w0, [x25, 8]
	cmp	w0, w22
	beq	.L962
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 508
	add	x1, x1, 456
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L962:
	add	x21, x21, :lo12:.LANCHOR0
	ldr	x0, [x21, 2608]
	add	x0, x0, x19
	str	wzr, [x0, 4]
	ldr	x0, [x21, 2608]
	strh	w22, [x0, x19]
	b	.L959
.L960:
	ldr	w0, [x20, 3720]
	cmp	w0, 256
	bne	.L961
	mov	w2, w24
	mov	w1, w22
	adrp	x0, .LC108
	add	x0, x0, :lo12:.LC108
	bl	sftl_printk
	ldr	x0, [x20, 2608]
	mov	w1, w22
	add	x0, x0, x19
	ldr	x2, [x0, 8]
	add	x0, x20, 3640
	bl	FtlMapWritePage
	b	.L961
	.size	load_l2p_region, .-load_l2p_region
	.align	2
	.global	ftl_map_blk_gc
	.type	ftl_map_blk_gc, %function
ftl_map_blk_gc:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	adrp	x22, .LANCHOR0
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	ldr	x20, [x0, 16]
	ldr	x25, [x0, 40]
	bl	ftl_free_no_use_map_blk
	ldrh	w1, [x19, 10]
	ldrh	w2, [x19, 8]
	sub	w1, w1, #5
	cmp	w2, w1
	blt	.L965
	ubfiz	x0, x0, 1, 16
	ldrh	w26, [x20, x0]
	cbz	w26, .L965
	ldr	w1, [x19, 52]
	cbnz	w1, .L965
	mov	w1, 1
	str	w1, [x19, 52]
	strh	wzr, [x20, x0]
	ldrh	w0, [x19, 8]
	ldrh	w1, [x19, 2]
	sub	w0, w0, #1
	strh	w0, [x19, 8]
	add	x0, x22, :lo12:.LANCHOR0
	ldrh	w0, [x0, 308]
	cmp	w1, w0
	bcc	.L966
	mov	x0, x19
	bl	ftl_map_blk_alloc_new_blk
.L966:
	add	x27, x22, :lo12:.LANCHOR0
	adrp	x23, .LANCHOR1
	add	x23, x23, :lo12:.LANCHOR1
	add	x21, x27, 3720
	add	x23, x23, 472
	mov	w20, 0
.L967:
	ldrh	w0, [x19, 6]
	cmp	w0, w20
	bhi	.L974
	mov	w1, 1
	mov	w0, w26
	bl	FtlFreeSysBlkQueueIn
	str	wzr, [x19, 52]
.L965:
	add	x22, x22, :lo12:.LANCHOR0
	ldrh	w1, [x19, 2]
	ldrh	w0, [x22, 308]
	cmp	w1, w0
	bcc	.L975
	mov	x0, x19
	bl	ftl_map_blk_alloc_new_blk
.L975:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 96
	ret
.L974:
	ubfiz	x0, x20, 2, 16
	add	x28, x25, x0
	ldr	w1, [x25, x0]
	cmp	w26, w1, lsr 10
	bne	.L968
	ldr	x1, [x27, 3464]
	mov	w2, 1
	ldr	x24, [x27, 3520]
	stp	x1, x24, [x21, 8]
	mov	w1, w2
	ldr	w0, [x25, x0]
	str	w0, [x21, 4]
	mov	x0, x21
	bl	FlashReadPages
	ldrh	w0, [x24, 8]
	cmp	w0, w20
	beq	.L969
	adrp	x0, .LC8
	mov	w2, 611
	mov	x1, x23
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L969:
	ldr	w0, [x21]
	cmn	w0, #1
	bne	.L970
.L972:
	str	wzr, [x28]
.L971:
	b	.L971
.L970:
	ldrh	w0, [x24, 8]
	cmp	w0, w20
	bne	.L972
	ldrh	w1, [x24]
	ldrh	w0, [x19, 4]
	cmp	w1, w0
	bne	.L972
	ldr	x2, [x21, 8]
	mov	w1, w20
	mov	x0, x19
	bl	FtlMapWritePage
.L968:
	add	w20, w20, 1
	and	w20, w20, 65535
	b	.L967
	.size	ftl_map_blk_gc, .-ftl_map_blk_gc
	.align	2
	.global	flush_l2p_region
	.type	flush_l2p_region, %function
flush_l2p_region:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	ubfiz	x20, x0, 4, 16
	ldr	x0, [x19, 2608]
	add	x1, x0, x20
	ldr	x2, [x1, 8]
	ldrh	w1, [x0, x20]
	add	x0, x19, 3640
	bl	FtlMapWritePage
	ldr	x0, [x19, 2608]
	add	x0, x0, x20
	ldr	w1, [x0, 4]
	and	w1, w1, 2147483647
	str	w1, [x0, 4]
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	flush_l2p_region, .-flush_l2p_region
	.align	2
	.global	log2phys
	.type	log2phys, %function
log2phys:
	stp	x29, x30, [sp, -80]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	stp	x25, x26, [sp, 64]
	mov	w25, w0
	add	x0, x20, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	mov	x22, x1
	mov	w23, w2
	ldrh	w24, [x0, 312]
	ldr	w0, [x0, 2624]
	cmp	w25, w0
	bcc	.L984
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 820
	add	x1, x1, 488
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L984:
	add	x0, x20, :lo12:.LANCHOR0
	ldr	w1, [x0, 2624]
	cmp	w25, w1
	bcs	.L985
	add	w24, w24, 7
	ldrh	w1, [x0, 342]
	ldr	x2, [x0, 2608]
	mov	x0, 0
	lsr	w21, w25, w24
	and	w21, w21, 65535
.L986:
	and	x19, x0, 65535
	cmp	w19, w1
	bcc	.L991
	bl	select_l2p_ram_region
	and	x19, x0, 65535
	add	x2, x20, :lo12:.LANCHOR0
	ubfiz	x1, x19, 4, 16
	mov	w26, w0
	ldr	x2, [x2, 2608]
	add	x3, x2, x1
	ldrh	w2, [x2, x1]
	mov	w1, 65535
	cmp	w2, w1
	beq	.L992
	ldr	w1, [x3, 4]
	tbz	w1, #31, .L992
	bl	flush_l2p_region
.L992:
	mov	w1, w26
	mov	w0, w21
	bl	load_l2p_region
	b	.L988
.L985:
	mov	w0, -1
	cbnz	w23, .L983
	str	w0, [x22]
.L983:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
.L991:
	add	x0, x0, 1
	add	x3, x2, x0, lsl 4
	ldrh	w3, [x3, -16]
	cmp	w3, w21
	bne	.L986
.L988:
	mov	x0, 1
	lsl	x0, x0, x24
	sub	w0, w0, #1
	and	w0, w0, w25
	and	x0, x0, 65535
	cbnz	w23, .L989
	add	x1, x20, :lo12:.LANCHOR0
	ldr	x1, [x1, 2608]
	add	x1, x1, x19, lsl 4
	ldr	x1, [x1, 8]
	ldr	w0, [x1, x0, lsl 2]
	str	w0, [x22]
.L990:
	add	x20, x20, :lo12:.LANCHOR0
	ldr	x0, [x20, 2608]
	add	x19, x0, x19, lsl 4
	ldr	w0, [x19, 4]
	cmn	w0, #1
	beq	.L994
	add	w0, w0, 1
	str	w0, [x19, 4]
.L994:
	mov	w0, 0
	b	.L983
.L989:
	add	x2, x20, :lo12:.LANCHOR0
	lsl	x1, x19, 4
	ldr	w4, [x22]
	ldr	x3, [x2, 2608]
	add	x3, x3, x1
	ldr	x3, [x3, 8]
	str	w4, [x3, x0, lsl 2]
	strh	w21, [x2, 2616]
	ldr	x0, [x2, 2608]
	add	x0, x0, x1
	ldr	w1, [x0, 4]
	orr	w1, w1, -2147483648
	str	w1, [x0, 4]
	b	.L990
	.size	log2phys, .-log2phys
	.align	2
	.global	FtlWriteDump_data
	.type	FtlWriteDump_data, %function
FtlWriteDump_data:
	stp	x29, x30, [sp, -128]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x20, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	stp	x23, x24, [sp, 48]
	adrp	x22, __stack_chk_guard
	stp	x25, x26, [sp, 64]
	add	x0, x22, :lo12:__stack_chk_guard
	ldrh	w2, [x20, 28]
	ldr	x1, [x0]
	str	x1, [x29, 120]
	mov	x1,0
	cbz	w2, .L998
	ldrb	w1, [x20, 32]
	cbnz	w1, .L998
	ldrb	w1, [x20, 31]
	ldrh	w3, [x20, 306]
	mul	w1, w1, w3
	cmp	w2, w1
	beq	.L998
	ldrb	w0, [x20, 34]
	cbnz	w0, .L997
	ldr	w23, [x20, 2624]
	add	x1, x29, 84
	ldrh	w25, [x20, 236]
	mov	w2, 0
	sub	w23, w23, #1
	mov	w0, w23
	bl	log2phys
	ldr	x21, [x20, 3520]
	ldr	w0, [x29, 84]
	ldr	x1, [x20, 3456]
	str	w0, [x29, 92]
	cmn	w0, #1
	stp	x1, x21, [x29, 96]
	str	w23, [x29, 112]
	str	wzr, [x21, 4]
	beq	.L1000
	mov	w2, 0
	mov	w1, 1
	add	x0, x29, 88
	bl	FlashReadPages
.L1001:
	add	x26, x19, :lo12:.LANCHOR0
	lsl	w25, w25, 2
	add	x20, x26, 24
	mov	w0, -3947
	mov	w24, 0
	strh	w0, [x21]
.L1002:
	cmp	w25, w24
	bne	.L1006
.L1003:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, 1
	strb	w0, [x19, 34]
.L997:
	add	x22, x22, :lo12:__stack_chk_guard
	ldr	x1, [x29, 120]
	ldr	x0, [x22]
	eor	x0, x1, x0
	cbz	x0, .L1008
	bl	__stack_chk_fail
.L1000:
	ldrh	w2, [x20, 314]
	mov	w1, 255
	ldr	x0, [x20, 3456]
	bl	ftl_memset
	b	.L1001
.L1006:
	ldrh	w0, [x20, 4]
	cbz	w0, .L1003
	ldr	w0, [x29, 92]
	add	w24, w24, 1
	stp	w23, w0, [x21, 8]
	ldrh	w0, [x20]
	strh	w0, [x21, 2]
	mov	x0, x20
	bl	get_new_active_ppa
	str	w0, [x29, 92]
	ldr	w0, [x26, 2664]
	mov	w3, 0
	str	w0, [x21, 4]
	mov	w2, 0
	add	w0, w0, 1
	mov	w1, 1
	cmn	w0, #1
	csel	w0, w0, wzr, ne
	str	w0, [x26, 2664]
	add	x0, x29, 88
	bl	FlashProgPages
	ldrh	w0, [x20]
	bl	decrement_vpc_count
	b	.L1002
.L998:
	add	x19, x19, :lo12:.LANCHOR0
	strb	wzr, [x19, 34]
	b	.L997
.L1008:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 128
	ret
	.size	FtlWriteDump_data, .-FtlWriteDump_data
	.align	2
	.global	FtlReUsePrevPpa
	.type	FtlReUsePrevPpa, %function
FtlReUsePrevPpa:
	stp	x29, x30, [sp, -80]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	mov	w21, w0
	stp	x19, x20, [sp, 16]
	lsr	w0, w1, 10
	str	w1, [x29, 76]
	adrp	x22, .LANCHOR0
	str	x23, [sp, 48]
	bl	P2V_block_in_plane
	add	x2, x22, :lo12:.LANCHOR0
	and	w0, w0, 65535
	ubfiz	x20, x0, 1, 16
	ldr	x3, [x2, 72]
	ldrh	w1, [x3, x20]
	cbnz	w1, .L1017
	ldr	x19, [x2, 2592]
	cbz	x19, .L1018
	ldrh	w3, [x2, 228]
	mov	x4, -6148914691236517206
	ldr	x2, [x2, 2560]
	movk	x4, 0xaaab, lsl 0
	mov	w5, 65535
	sub	x19, x19, x2
	asr	x19, x19, 1
	mul	x19, x19, x4
	mov	w4, 6
	and	w19, w19, 65535
.L1019:
	cmp	w1, w3
	beq	.L1018
	cmp	w19, w0
	bne	.L1020
	add	x23, x22, :lo12:.LANCHOR0
	mov	w1, w19
	add	x0, x23, 2592
	bl	List_remove_node
	ldrh	w0, [x23, 228]
	cbnz	w0, .L1021
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1742
	add	x1, x1, 504
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1021:
	add	x22, x22, :lo12:.LANCHOR0
	ldrh	w0, [x22, 228]
	sub	w0, w0, #1
	strh	w0, [x22, 228]
	mov	w0, w19
	bl	INSERT_DATA_LIST
	ldr	x1, [x22, 72]
	ldrh	w0, [x1, x20]
	add	w0, w0, 1
	strh	w0, [x1, x20]
.L1018:
	add	x1, x29, 76
	mov	w2, 1
	mov	w0, w21
	bl	log2phys
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldr	x23, [sp, 48]
	ldp	x29, x30, [sp], 80
	ret
.L1020:
	umull	x19, w19, w4
	ldrh	w19, [x2, x19]
	cmp	w19, w5
	beq	.L1018
	add	w1, w1, 1
	and	w1, w1, 65535
	b	.L1019
.L1017:
	add	w1, w1, 1
	strh	w1, [x3, x20]
	b	.L1018
	.size	FtlReUsePrevPpa, .-FtlReUsePrevPpa
	.align	2
	.global	FtlRecoverySuperblock
	.type	FtlRecoverySuperblock, %function
FtlRecoverySuperblock:
	stp	x29, x30, [sp, -192]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x20, x0
	stp	x21, x22, [sp, 32]
	adrp	x0, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	add	x1, x0, :lo12:__stack_chk_guard
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	str	x0, [x29, 120]
	ldr	x2, [x1]
	str	x2, [x29, 184]
	mov	x2,0
	mov	w1, 65535
	ldrh	w2, [x20]
	cmp	w2, w1
	beq	.L1029
	ldrb	w0, [x20, 6]
	adrp	x19, .LANCHOR0
	str	w0, [x29, 156]
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w26, [x20, 2]
	ldrh	w0, [x0, 306]
	cmp	w0, w26
	bne	.L1030
	strh	wzr, [x20, 4]
.L1156:
	strb	wzr, [x20, 6]
.L1029:
	ldr	x1, [x29, 120]
	mov	w0, 0
	add	x1, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 184]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L1098
	bl	__stack_chk_fail
.L1030:
	ldrh	w0, [x20, 16]
	mov	w21, 0
.L1031:
	cmp	w0, w1
	beq	.L1032
	mov	w1, 1
	bl	FtlGetLastWrittenPage
	mov	w23, w0
	cmn	w0, #1
	beq	.L1033
	add	x0, x19, :lo12:.LANCHOR0
	add	x28, x20, 16
	mov	x4, x28
	mov	w22, 0
	mov	w10, 65535
	mov	w6, 4
	ldrh	w3, [x0, 236]
	ldrh	w8, [x0, 314]
	add	x3, x3, 8
	ldrh	w9, [x0, 316]
	add	x3, x20, x3, lsl 1
.L1034:
	cmp	x3, x4
	bne	.L1038
	add	x24, x19, :lo12:.LANCHOR0
	mov	w2, 0
	mov	w1, w22
	ldr	x0, [x24, 3432]
	bl	FlashReadPages
	ldr	w21, [x24, 2664]
	ldr	x4, [x24, 3432]
	and	w9, w23, 65535
	sub	w21, w21, #1
	mov	w7, 65535
	mov	x6, x4
	mov	w3, 0
.L1039:
	cmp	w22, w3
	bne	.L1044
	add	w22, w23, 1
	ldr	w0, [x4, 4]
	and	w22, w22, 65535
.L1151:
	lsr	w0, w0, 10
	bl	P2V_plane
	and	w27, w0, 65535
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w0, [x0, 306]
	cmp	w0, w22
	bne	.L1046
	strh	w22, [x20, 2]
	strb	wzr, [x20, 6]
	strh	wzr, [x20, 4]
.L1046:
	ldr	w0, [x29, 156]
	cmp	w22, w26
	ccmp	w27, w0, 0, eq
	bne	.L1047
.L1157:
	mov	w2, w27
	mov	w1, w22
	mov	x0, x20
	bl	ftl_sb_update_avl_pages
	b	.L1029
.L1032:
	add	w21, w21, 1
	and	w21, w21, 65535
	add	x0, x20, x21, sxtw 1
	ldrh	w0, [x0, 16]
	b	.L1031
.L1033:
	cbz	w26, .L1035
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1809
	add	x1, x1, 520
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1035:
	ldr	w0, [x29, 156]
	cmp	w0, 0
	ccmp	w21, w0, 4, ne
	beq	.L1036
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1810
	add	x1, x1, 520
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1036:
	strh	wzr, [x20, 2]
	b	.L1156
.L1038:
	ldrh	w1, [x4]
	cmp	w1, w10
	beq	.L1037
	ldr	x5, [x0, 3432]
	ubfiz	x7, x22, 5, 16
	orr	w1, w23, w1, lsl 10
	add	x5, x5, x7
	str	w1, [x5, 4]
	mul	w1, w22, w8
	ldr	x2, [x0, 3432]
	ldr	x5, [x0, 3256]
	sdiv	w1, w1, w6
	add	x2, x2, x7
	add	x1, x5, x1, sxtw 2
	str	x1, [x2, 8]
	mul	w1, w22, w9
	ldr	x5, [x0, 3264]
	add	w22, w22, 1
	and	w22, w22, 65535
	sdiv	w1, w1, w6
	add	x1, x5, x1, sxtw 2
	str	x1, [x2, 16]
.L1037:
	add	x4, x4, 2
	b	.L1034
.L1044:
	ldr	w0, [x6]
	cbnz	w0, .L1040
	ldr	x8, [x6, 16]
	ldr	w5, [x8, 4]
	cmn	w5, #1
	beq	.L1041
	ldr	w1, [x24, 2664]
	mov	w0, w5
	bl	ftl_cmp_data_ver
	cbz	w0, .L1041
	add	w5, w5, 1
	str	w5, [x24, 2664]
.L1041:
	ldr	w0, [x8]
	cmn	w0, #1
	bne	.L1043
	ubfiz	x3, x3, 5, 16
	and	w22, w23, 65535
	add	x3, x4, x3
	ldr	w0, [x3, 4]
	b	.L1151
.L1040:
	mov	w7, w9
.L1043:
	add	w3, w3, 1
	add	x6, x6, 32
	and	w3, w3, 65535
	b	.L1039
.L1047:
	mov	w0, 65535
	cmp	w7, w0
	bne	.L1048
	ldrb	w0, [x20, 8]
	cbnz	w0, .L1049
.L1048:
	add	x0, x19, :lo12:.LANCHOR0
	and	w3, w23, 65535
	ldr	w1, [x0, 3716]
	cmn	w1, #1
	bne	.L1050
	str	w21, [x0, 3716]
.L1050:
	add	w0, w26, 7
	cmp	w0, w23, uxth
	bge	.L1099
	sub	w24, w3, #7
	and	w24, w24, 65535
.L1051:
	add	x25, x19, :lo12:.LANCHOR0
	mov	w4, 65535
.L1052:
	cmp	w24, w3
	bhi	.L1059
	ldrh	w0, [x25, 236]
	mov	x1, x28
	mov	w23, 0
	add	x0, x0, 8
	add	x0, x20, x0, lsl 1
	b	.L1060
.L1099:
	mov	w24, w26
	b	.L1051
.L1054:
	ldrh	w2, [x1]
	cmp	w2, w4
	beq	.L1053
	ldr	x5, [x25, 3432]
	ubfiz	x6, x23, 5, 16
	add	w23, w23, 1
	orr	w2, w24, w2, lsl 10
	add	x5, x5, x6
	and	w23, w23, 65535
	str	w2, [x5, 4]
.L1053:
	add	x1, x1, 2
.L1060:
	cmp	x0, x1
	bne	.L1054
	ldr	x0, [x25, 3432]
	mov	w1, w23
	str	w4, [x29, 136]
	mov	w2, 0
	str	w3, [x29, 144]
	ubfiz	x23, x23, 5, 16
	add	x23, x23, 16
	bl	FlashReadPages
	ldr	x1, [x25, 3432]
	ldr	w4, [x29, 136]
	ldr	w3, [x29, 144]
	add	x0, x1, 16
	add	x23, x1, x23
.L1055:
	cmp	x23, x0
	bne	.L1058
	add	w24, w24, 1
	and	w24, w24, 65535
	b	.L1052
.L1058:
	ldr	w1, [x0, -16]
	cbnz	w1, .L1049
	ldr	x1, [x0]
	ldrh	w2, [x1]
	cmp	w2, w4
	beq	.L1057
	ldr	w1, [x1, 4]
	cmn	w1, #1
	beq	.L1057
	str	w1, [x25, 3716]
.L1057:
	add	x0, x0, 32
	b	.L1055
.L1059:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w1, -1
	str	w1, [x0, 3716]
.L1049:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, 1
	mov	w24, w26
	strh	w0, [x19, 3752]
	add	x0, x19, 3760
	str	x0, [x29, 112]
.L1061:
	ldrh	w0, [x19, 236]
	mov	x1, x28
	mov	w23, 0
	mov	w4, 65535
	add	x0, x0, 8
	add	x0, x20, x0, lsl 1
.L1062:
	cmp	x0, x1
	bne	.L1064
	ldr	x0, [x19, 3432]
	mov	w1, w23
	mov	w2, 0
	bl	FlashReadPages
	ubfiz	x0, x23, 5, 16
	str	x0, [x29, 128]
	adrp	x0, .LANCHOR1
	add	x0, x0, :lo12:.LANCHOR1
	add	x0, x0, 520
	mov	x23, 0
	str	x0, [x29, 136]
.L1065:
	ldr	x0, [x29, 128]
	cmp	x0, x23
	bne	.L1092
	add	w24, w24, 1
	ldrh	w0, [x19, 306]
	and	w24, w24, 65535
	cmp	w0, w24
	bne	.L1061
	ldrh	w1, [x19, 236]
	mov	w0, 0
	strh	w24, [x20, 2]
	mov	w2, 65535
	strh	wzr, [x20, 4]
.L1093:
	cmp	w0, w1
	beq	.L1029
	ldrh	w3, [x28], 2
	cmp	w3, w2
	beq	.L1094
	strb	w0, [x20, 6]
	b	.L1029
.L1064:
	ldrh	w2, [x1]
	cmp	w2, w4
	beq	.L1063
	ldr	x3, [x19, 3432]
	ubfiz	x5, x23, 5, 16
	add	w23, w23, 1
	orr	w2, w24, w2, lsl 10
	add	x3, x3, x5
	and	w23, w23, 65535
	str	w2, [x3, 4]
.L1063:
	add	x1, x1, 2
	b	.L1062
.L1092:
	ldr	x4, [x19, 3432]
	add	x4, x4, x23
	ldr	w5, [x4, 4]
	str	w5, [x29, 180]
	lsr	w0, w5, 10
	bl	P2V_plane
	and	w0, w0, 65535
	cmp	w24, w26
	bcc	.L1066
	ldr	w1, [x29, 156]
	ccmp	w1, w0, 0, eq
	bhi	.L1066
	cmp	w24, w22
	ccmp	w27, w0, 0, eq
	beq	.L1067
	ldr	w0, [x4]
	cmn	w0, #1
	beq	.L1068
	ldr	x3, [x4, 16]
	mov	w0, 61589
	ldrh	w1, [x3]
	cmp	w1, w0
	beq	.L1069
	ldrh	w0, [x20]
.L1153:
	bl	decrement_vpc_count
	b	.L1066
.L1069:
	ldr	w21, [x3, 4]
	cmn	w21, #1
	beq	.L1070
	ldr	w1, [x19, 2664]
	mov	w0, w21
	str	x3, [x29, 144]
	bl	ftl_cmp_data_ver
	ldr	x3, [x29, 144]
	cbz	w0, .L1070
	add	w0, w21, 1
	str	w0, [x19, 2664]
.L1070:
	ldrh	w1, [x3]
	mov	w0, 61589
	cmp	w1, w0
	beq	.L1071
	ldr	x1, [x29, 136]
	mov	w2, 1961
	str	x3, [x29, 144]
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	ldr	x3, [x29, 144]
.L1071:
	ldp	w25, w0, [x3, 8]
	add	x1, x29, 176
	str	w0, [x29, 172]
	mov	w2, 0
	mov	w0, w25
	bl	log2phys
	ldr	w1, [x19, 3716]
	ldr	w3, [x29, 172]
	cmn	w1, #1
	beq	.L1072
	mov	w0, w21
	bl	ftl_cmp_data_ver
	cbz	w0, .L1072
	cmn	w3, #1
	beq	.L1073
	ldr	x0, [x19, 3432]
	mov	w2, 0
	mov	w1, 1
	add	x0, x0, x23
	ldr	x4, [x0, 16]
	str	w3, [x0, 4]
	str	x4, [x29, 144]
	ldr	x0, [x19, 3432]
	add	x0, x0, x23
	bl	FlashReadPages
	ldr	x0, [x19, 3432]
	ldr	x4, [x29, 144]
	add	x3, x0, x23
	ldr	w0, [x0, x23]
	cmn	w0, #1
	bne	.L1074
.L1075:
	mov	w0, -1
	str	w0, [x29, 172]
.L1082:
	ldr	w4, [x29, 172]
	cmn	w4, #1
	beq	.L1066
.L1097:
	lsr	w0, w4, 10
	bl	P2V_block_in_plane
	and	w25, w0, 65535
	ldrh	w0, [x19, 244]
	mov	w3, w25
	cmp	w0, w25
	bhi	.L1088
	ldr	x1, [x29, 136]
	mov	w2, 2066
	adrp	x0, .LC8
	str	w25, [x29, 144]
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	ldr	w3, [x29, 144]
.L1088:
	ldr	x1, [x19, 72]
	ubfiz	x0, x25, 1, 16
	ldrh	w0, [x1, x0]
	cbz	w0, .L1089
	mov	w0, w3
	b	.L1153
.L1073:
	ldp	w1, w0, [x29, 176]
	cmp	w1, w0
	bne	.L1066
	mov	w2, 1
	add	x1, x29, 172
	mov	w0, w25
	bl	log2phys
.L1066:
	add	x23, x23, 32
	b	.L1065
.L1074:
	ldr	w0, [x4, 8]
	cmp	w25, w0
	bne	.L1075
	ldr	w0, [x4, 4]
	str	w0, [x29, 144]
	str	x4, [x29, 104]
	uxtw	x1, w0
	ldr	w0, [x19, 3716]
	bl	ftl_cmp_data_ver
	cbz	w0, .L1075
	ldp	w0, w1, [x29, 176]
	ldr	x4, [x29, 104]
	cmp	w0, w1
	ldr	w1, [x29, 172]
	bne	.L1077
.L1152:
	mov	w0, w25
	bl	FtlReUsePrevPpa
	b	.L1075
.L1077:
	cmp	w0, w1
	beq	.L1075
	cmn	w0, #1
	beq	.L1078
	ldr	x4, [x3, 16]
	mov	w2, 0
	str	w0, [x3, 4]
	mov	w1, 1
	str	x4, [x29, 104]
	ldr	x0, [x19, 3432]
	add	x0, x0, x23
	bl	FlashReadPages
	ldr	x4, [x29, 104]
.L1079:
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	ldr	x1, [x0, 3432]
	ldr	w1, [x1, x23]
	cmn	w1, #1
	beq	.L1080
	ldr	w3, [x4, 4]
	ldr	w0, [x0, 3716]
	mov	w1, w3
	bl	ftl_cmp_data_ver
	cbz	w0, .L1080
	ldr	w0, [x29, 144]
	mov	w1, w3
	bl	ftl_cmp_data_ver
	cbz	w0, .L1075
.L1080:
	ldr	w1, [x29, 172]
	b	.L1152
.L1078:
	str	w0, [x3]
	b	.L1079
.L1072:
	ldp	w1, w0, [x29, 176]
	cmp	w1, w0
	beq	.L1082
	cmn	w3, #1
	beq	.L1084
	ldr	w0, [x19, 252]
	cmp	w0, w3, lsr 10
	bhi	.L1084
	adrp	x0, .LC109
	mov	w1, w3
	add	x0, x0, :lo12:.LC109
.L1155:
	bl	sftl_printk
	b	.L1066
.L1084:
	mov	w2, 1
	add	x1, x29, 180
	mov	w0, w25
	bl	log2phys
	ldr	w4, [x29, 176]
	cmn	w4, #1
	beq	.L1082
	ldr	w0, [x29, 172]
	cmp	w4, w0
	beq	.L1097
	lsr	w0, w4, 10
	bl	P2V_block_in_plane
	ldrh	w1, [x19, 24]
	and	w0, w0, 65535
	cmp	w1, w0
	beq	.L1087
	ldrh	w1, [x19, 80]
	cmp	w1, w0
	beq	.L1087
	ldrh	w1, [x19, 128]
	cmp	w1, w0
	bne	.L1082
.L1087:
	ldr	x0, [x19, 3432]
	mov	w2, 0
	mov	w1, 1
	str	w4, [x0, 4]
	ldr	x3, [x0, 16]
	ldr	x0, [x19, 3432]
	str	x3, [x29, 144]
	bl	FlashReadPages
	ldr	x0, [x19, 3432]
	ldr	w0, [x0]
	cmn	w0, #1
	beq	.L1082
	ldr	x3, [x29, 144]
	mov	w0, w21
	ldr	w1, [x3, 4]
	bl	ftl_cmp_data_ver
	cbnz	w0, .L1082
	mov	w2, 1
	add	x1, x29, 176
	mov	w0, w25
	bl	log2phys
	b	.L1082
.L1089:
	adrp	x0, .LC110
	mov	w1, w25
	add	x0, x0, :lo12:.LC110
	b	.L1155
.L1068:
	ldr	w0, [x19, 3756]
	cmp	w0, 31
	bhi	.L1090
	ldr	x1, [x29, 112]
	str	w5, [x1, w0, uxtw 2]
	add	w0, w0, 1
	str	w0, [x19, 3756]
.L1090:
	ldrh	w0, [x20]
	bl	decrement_vpc_count
	ldr	w0, [x19, 3716]
	cmn	w0, #1
	bne	.L1091
.L1154:
	str	w21, [x19, 3716]
	b	.L1066
.L1091:
	cmp	w21, w0
	bcs	.L1066
	b	.L1154
.L1094:
	add	w0, w0, 1
	and	w0, w0, 65535
	b	.L1093
.L1067:
	strb	w27, [x20, 6]
	strh	w22, [x20, 2]
	b	.L1157
.L1098:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 192
	ret
	.size	FtlRecoverySuperblock, .-FtlRecoverySuperblock
	.align	2
	.global	ftl_check_vpc
	.type	ftl_check_vpc, %function
ftl_check_vpc:
	stp	x29, x30, [sp, -128]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	stp	x21, x22, [sp, 32]
	adrp	x21, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	add	x0, x21, :lo12:__stack_chk_guard
	stp	x25, x26, [sp, 64]
	add	x25, x20, :lo12:.LANCHOR0
	stp	x27, x28, [sp, 80]
	adrp	x22, .LANCHOR1
	mov	w24, 0
	adrp	x23, check_vpc_table
	ldr	x1, [x0]
	str	x1, [x29, 120]
	mov	x1,0
	add	x19, x23, :lo12:check_vpc_table
	add	x1, x22, :lo12:.LANCHOR1
	adrp	x0, .LC111
	add	x1, x1, 544
	add	x0, x0, :lo12:.LC111
	bl	sftl_printk
	mov	w2, 8192
	mov	w1, 0
	mov	x0, x19
	bl	ftl_memset
.L1159:
	ldr	w0, [x25, 2624]
	cmp	w24, w0
	bcc	.L1161
	adrp	x26, .LC112
	add	x24, x20, :lo12:.LANCHOR0
	add	x27, x23, :lo12:check_vpc_table
	add	x26, x26, :lo12:.LC112
	mov	w25, 0
	mov	w19, 0
	mov	w5, 65535
.L1162:
	ldrh	w0, [x24, 244]
	cmp	w0, w19
	bhi	.L1164
	ldr	x19, [x24, 2592]
	cbz	x19, .L1165
	ldr	x0, [x24, 2560]
	adrp	x26, .LC113
	add	x20, x20, :lo12:.LANCHOR0
	add	x23, x23, :lo12:check_vpc_table
	sub	x19, x19, x0
	mov	x0, -6148914691236517206
	asr	x19, x19, 1
	movk	x0, 0xaaab, lsl 0
	add	x26, x26, :lo12:.LC113
	ldrh	w27, [x24, 228]
	mov	w28, 6
	mov	w24, 0
	mul	x19, x19, x0
	and	w19, w19, 65535
.L1166:
	cmp	w24, w27
	bne	.L1168
.L1165:
	cbz	w25, .L1158
	add	x1, x22, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2394
	add	x1, x1, 544
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1158:
	add	x21, x21, :lo12:__stack_chk_guard
	ldr	x1, [x29, 120]
	ldr	x0, [x21]
	eor	x0, x1, x0
	cbz	x0, .L1170
	bl	__stack_chk_fail
.L1161:
	mov	w2, 0
	add	x1, x29, 116
	mov	w0, w24
	bl	log2phys
	ldr	w0, [x29, 116]
	cmn	w0, #1
	beq	.L1160
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	and	x0, x0, 65535
	ldrh	w1, [x19, x0, lsl 1]
	add	w1, w1, 1
	strh	w1, [x19, x0, lsl 1]
.L1160:
	add	w24, w24, 1
	b	.L1159
.L1164:
	ldr	x0, [x24, 72]
	ubfiz	x4, x19, 1, 16
	sxtw	x28, w19
	str	x4, [x29, 96]
	ldrh	w3, [x27, x28, lsl 1]
	ldrh	w2, [x0, x4]
	cmp	w2, w3
	beq	.L1163
	mov	x0, x26
	str	w5, [x29, 108]
	mov	w1, w19
	bl	sftl_printk
	ldr	x0, [x24, 72]
	ldr	x4, [x29, 96]
	ldr	w5, [x29, 108]
	ldrh	w0, [x0, x4]
	cmp	w0, w5
	beq	.L1163
	ldrh	w1, [x27, x28, lsl 1]
	cmp	w1, w0
	csinc	w25, w25, wzr, ls
.L1163:
	add	w19, w19, 1
	and	w19, w19, 65535
	b	.L1162
.L1168:
	ldr	x1, [x20, 72]
	ubfiz	x0, x19, 1, 16
	ldrh	w2, [x1, x0]
	cbz	w2, .L1167
	ldrh	w3, [x23, w19, sxtw 1]
	mov	w25, 1
	mov	w1, w19
	mov	x0, x26
	bl	sftl_printk
.L1167:
	ldr	x0, [x20, 2560]
	umull	x19, w19, w28
	ldrh	w19, [x0, x19]
	mov	w0, 65535
	cmp	w19, w0
	beq	.L1165
	add	w24, w24, 1
	and	w24, w24, 65535
	b	.L1166
.L1170:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 128
	ret
	.size	ftl_check_vpc, .-ftl_check_vpc
	.align	2
	.global	ftl_scan_all_data
	.type	ftl_scan_all_data, %function
ftl_scan_all_data:
	sub	sp, sp, #96
	stp	x29, x30, [sp, 16]
	add	x29, sp, 16
	stp	x23, x24, [sp, 64]
	adrp	x24, .LANCHOR0
	add	x23, x24, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 32]
	add	x19, x23, 3720
	stp	x21, x22, [sp, 48]
	mov	w20, 0
	adrp	x22, __stack_chk_guard
	add	x0, x22, :lo12:__stack_chk_guard
	ldr	x1, [x0]
	str	x1, [x29, 72]
	mov	x1,0
	adrp	x0, .LC114
	add	x0, x0, :lo12:.LC114
	bl	sftl_printk
.L1185:
	add	x0, x24, :lo12:.LANCHOR0
	ldr	w0, [x0, 2624]
	cmp	w20, w0
	bcc	.L1191
	add	x0, x22, :lo12:__stack_chk_guard
	ldr	x1, [x29, 72]
	ldr	x0, [x0]
	eor	x0, x1, x0
	cbz	x0, .L1192
	bl	__stack_chk_fail
.L1191:
	mov	w2, 0
	add	x1, x29, 68
	mov	w0, w20
	bl	log2phys
	tst	x20, 2047
	bne	.L1186
	ldr	w2, [x29, 68]
	adrp	x0, .LC115
	mov	w1, w20
	add	x0, x0, :lo12:.LC115
	bl	sftl_printk
.L1186:
	ldr	w1, [x29, 68]
	cmn	w1, #1
	beq	.L1188
	ldr	x0, [x23, 3456]
	mov	w2, 0
	ldr	x21, [x23, 3520]
	stp	wzr, w1, [x19]
	mov	w1, 1
	stp	x0, x21, [x19, 8]
	mov	x0, x19
	str	w20, [x19, 24]
	bl	FlashReadPages
	ldr	w1, [x19]
	cmp	w1, 256
	ccmn	w1, #1, 4, ne
	beq	.L1189
	ldr	w0, [x21, 8]
	cmp	w20, w0
	beq	.L1188
.L1189:
	ldp	x1, x0, [x19, 8]
	ldr	w2, [x1, 4]
	str	w2, [sp]
	ldp	w3, w4, [x0]
	ldp	w5, w6, [x0, 8]
	adrp	x0, .LC116
	ldr	w7, [x1]
	add	x0, x0, :lo12:.LC116
	ldr	w2, [x19, 4]
	mov	w1, w20
	bl	sftl_printk
.L1188:
	add	w20, w20, 1
	b	.L1185
.L1192:
	ldp	x19, x20, [sp, 32]
	ldp	x21, x22, [sp, 48]
	ldp	x23, x24, [sp, 64]
	ldp	x29, x30, [sp, 16]
	add	sp, sp, 96
	ret
	.size	ftl_scan_all_data, .-ftl_scan_all_data
	.align	2
	.global	FtlGcScanTempBlk
	.type	FtlGcScanTempBlk, %function
FtlGcScanTempBlk:
	stp	x29, x30, [sp, -160]!
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x22, __stack_chk_guard
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x25, x26, [sp, 64]
	add	x0, x22, :lo12:__stack_chk_guard
	stp	x23, x24, [sp, 48]
	mov	w25, w1
	stp	x27, x28, [sp, 80]
	adrp	x21, .LANCHOR2
	str	x22, [x29, 96]
	ldr	x1, [x0]
	str	x1, [x29, 152]
	mov	x1,0
	add	x0, x21, :lo12:.LANCHOR2
	ldrh	w28, [x0, 4]
	mov	w0, 65535
	cmp	w28, w0
	beq	.L1219
	cbnz	w28, .L1195
.L1196:
	bl	FtlGcPageVarInit
	b	.L1197
.L1219:
	mov	w28, 0
.L1195:
	adrp	x0, .LANCHOR0+306
	ldrh	w0, [x0, #:lo12:.LANCHOR0+306]
	cmp	w0, w25
	beq	.L1196
.L1197:
	adrp	x26, .LANCHOR0
	add	x22, x26, :lo12:.LANCHOR0
	mov	w24, 0
.L1212:
	ldrh	w1, [x19]
	mov	w0, 65535
	strb	wzr, [x19, 8]
	cmp	w1, w0
	beq	.L1198
.L1215:
	add	x0, x26, :lo12:.LANCHOR0
	add	x6, x19, 16
	mov	w20, 0
	mov	w11, 65535
	mov	w7, 4
	ldrh	w9, [x0, 314]
	ldrh	w10, [x0, 316]
	ldrh	w0, [x0, 236]
	add	x0, x0, 8
	add	x0, x19, x0, lsl 1
.L1199:
	cmp	x0, x6
	bne	.L1201
	ldr	x0, [x22, 3432]
	mov	w2, 0
	mov	w1, w20
	mov	x23, 0
	bl	FlashReadPages
	ubfiz	x0, x20, 5, 16
	str	x0, [x29, 104]
.L1202:
	ldr	x0, [x29, 104]
	cmp	x0, x23
	bne	.L1213
	add	w4, w28, 1
	add	w24, w24, 1
	and	w28, w4, 65535
	cmp	w25, w24
	bls	.L1214
.L1216:
	ldrh	w0, [x22, 306]
	cmp	w0, w28
	bhi	.L1215
.L1198:
	add	x21, x21, :lo12:.LANCHOR2
	strh	w28, [x19, 2]
	strb	wzr, [x19, 6]
	mov	w0, -1
	mov	w2, 0
	mov	w1, w28
	strh	w0, [x21, 4]
	mov	x0, x19
	bl	ftl_sb_update_avl_pages
	b	.L1217
.L1201:
	ldrh	w1, [x6]
	cmp	w1, w11
	beq	.L1200
	ldr	x5, [x22, 3432]
	ubfiz	x8, x20, 5, 16
	orr	w1, w28, w1, lsl 10
	add	x5, x5, x8
	str	w1, [x5, 4]
	mul	w1, w20, w9
	ldr	x2, [x22, 3432]
	ldr	x5, [x22, 3256]
	sdiv	w1, w1, w7
	add	x2, x2, x8
	add	x1, x5, x1, sxtw 2
	str	x1, [x2, 8]
	mul	w1, w20, w10
	ldr	x5, [x22, 3264]
	add	w20, w20, 1
	and	w20, w20, 65535
	sdiv	w1, w1, w7
	add	x1, x5, x1, sxtw 2
	str	x1, [x2, 16]
.L1200:
	add	x6, x6, 2
	b	.L1199
.L1213:
	ldr	x0, [x22, 3432]
	add	x1, x0, x23
	ldr	w0, [x0, x23]
	ldr	w27, [x1, 4]
	ldr	x20, [x1, 16]
	cbnz	w0, .L1203
	ldrh	w0, [x20]
	mov	w1, 65535
	cmp	w0, w1
	bne	.L1204
.L1229:
	ldrh	w1, [x19]
	mov	w28, 0
	ldr	x0, [x22, 72]
	strh	wzr, [x0, x1, lsl 1]
	ldrh	w0, [x19]
	bl	INSERT_FREE_LIST
	mov	w0, -1
	strh	w0, [x19]
	strh	w0, [x22, 176]
	bl	FtlGcPageVarInit
	b	.L1212
.L1204:
	ldr	w0, [x20, 8]
	ldr	w1, [x22, 2624]
	cmp	w0, w1
	bhi	.L1229
	add	x1, x29, 116
	mov	w2, 0
	bl	log2phys
	ldr	w0, [x20, 12]
	ldr	w1, [x29, 116]
	cmp	w0, w1
	beq	.L1207
.L1209:
	ldr	w2, [x20, 8]
.L1230:
	ldr	w0, [x20, 12]
	mov	w1, w27
	add	x23, x23, 32
	bl	FtlGcUpdatePage
	b	.L1202
.L1207:
	cmn	w0, #1
	beq	.L1209
	str	w0, [x29, 124]
	mov	w2, 0
	ldr	x0, [x22, 3488]
	mov	w1, 1
	str	x0, [x29, 128]
	ldr	x0, [x22, 3528]
	str	x0, [x29, 136]
	add	x0, x29, 120
	bl	FlashReadPages
	ldrh	w1, [x22, 262]
	mov	x0, 0
	ldr	x2, [x22, 3432]
	ldr	x6, [x29, 128]
	ubfiz	x1, x1, 9, 16
	add	x2, x2, x23
.L1210:
	cmp	x0, x1
	beq	.L1209
	ldr	x7, [x2, 8]
	ldr	w8, [x7, x0]
	add	x0, x0, 4
	add	x7, x6, x0
	ldr	w7, [x7, -4]
	cmp	w8, w7
	beq	.L1210
	ldrh	w1, [x19]
	adrp	x0, .LC117
	ldr	w2, [x29, 124]
	add	x0, x0, :lo12:.LC117
	bl	sftl_printk
	b	.L1229
.L1203:
	mov	w2, -1
	b	.L1230
.L1214:
	add	x1, x21, :lo12:.LANCHOR2
	mov	w2, 65535
	ldrh	w0, [x1, 4]
	cmp	w0, w2
	beq	.L1216
	add	w0, w0, w24
	strh	w0, [x1, 4]
	ldrh	w0, [x22, 306]
	cmp	w0, w28
	bls	.L1216
.L1217:
	ldr	x1, [x29, 96]
	mov	w0, -1
	add	x22, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 152]
	ldr	x1, [x22]
	eor	x1, x2, x1
	cbz	x1, .L1218
	bl	__stack_chk_fail
.L1218:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 160
	ret
	.size	FtlGcScanTempBlk, .-FtlGcScanTempBlk
	.align	2
	.global	FtlReadRefresh
	.type	FtlReadRefresh, %function
FtlReadRefresh:
	stp	x29, x30, [sp, -160]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, __stack_chk_guard
	stp	x21, x22, [sp, 32]
	add	x0, x20, :lo12:__stack_chk_guard
	ldr	x1, [x0]
	str	x1, [x29, 152]
	mov	x1,0
	adrp	x1, .LANCHOR0
	add	x0, x1, :lo12:.LANCHOR0
	add	x2, x0, 2712
	ldr	w3, [x2, 80]
	cbz	w3, .L1232
	ldr	w1, [x0, 2624]
	ldr	w3, [x2, 84]
	cmp	w3, w1
	bcs	.L1233
	mov	x19, x0
	mov	w21, 2048
.L1238:
	add	x22, x19, 2712
	ldr	w1, [x19, 2624]
	ldr	w0, [x22, 84]
	cmp	w0, w1
	bcc	.L1234
.L1237:
	mov	w0, -1
.L1231:
	add	x20, x20, :lo12:__stack_chk_guard
	ldr	x2, [x29, 152]
	ldr	x1, [x20]
	eor	x1, x2, x1
	cbz	x1, .L1243
	bl	__stack_chk_fail
.L1234:
	add	x1, x29, 52
	mov	w2, 0
	bl	log2phys
	ldr	w0, [x22, 84]
	ldr	w1, [x29, 52]
	add	w0, w0, 1
	str	w0, [x22, 84]
	cmn	w1, #1
	beq	.L1236
	str	w0, [x29, 80]
	mov	w2, 0
	ldr	x0, [x19, 3504]
	str	x0, [x29, 64]
	add	x0, x29, 88
	str	x0, [x29, 72]
	add	x0, x29, 160
	str	w1, [x29, 60]
	mov	w1, 1
	str	wzr, [x0, -104]!
	bl	FlashReadPages
	ldr	w0, [x29, 56]
	cmp	w0, 256
	bne	.L1237
	ldr	w0, [x29, 52]
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
	b	.L1237
.L1236:
	subs	w21, w21, #1
	bne	.L1238
	b	.L1237
.L1233:
	ldr	w0, [x0, 2628]
	stp	w0, wzr, [x2, 76]
	str	wzr, [x2, 84]
.L1246:
	mov	w0, 0
	b	.L1231
.L1232:
	ldr	w4, [x0, 2684]
	mov	w3, 10000
	ldr	w5, [x0, 2628]
	mov	w6, 31
	cmp	w4, w3
	ldr	w7, [x2, 76]
	mov	w3, 63
	csel	w6, w6, w3, hi
	add	w3, w5, 1048576
	cmp	w7, w3
	bhi	.L1242
	ldr	w3, [x0, 2624]
	mov	w8, 1000
	lsr	w4, w4, 10
	add	w4, w4, 1
	mul	w3, w3, w8
	udiv	w3, w3, w4
	add	w3, w3, w7
	cmp	w5, w3
	bhi	.L1242
	ldrh	w0, [x0, 2540]
	tst	w6, w0
	bne	.L1246
	ldr	w2, [x2, 100]
	cmp	w0, w2
	beq	.L1246
.L1242:
	add	x0, x1, :lo12:.LANCHOR0
	add	x1, x0, 2712
	ldrh	w0, [x0, 2540]
	str	w0, [x1, 100]
	str	w5, [x1, 76]
	mov	w0, 1
	str	wzr, [x1, 84]
	str	w0, [x1, 80]
	b	.L1246
.L1243:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 160
	ret
	.size	FtlReadRefresh, .-FtlReadRefresh
	.align	2
	.global	l2p_flush
	.type	l2p_flush, %function
l2p_flush:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	add	x20, x20, :lo12:.LANCHOR0
	mov	w19, 0
	bl	FtlWriteDump_data
.L1250:
	ldrh	w0, [x20, 342]
	cmp	w0, w19
	bhi	.L1252
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L1252:
	ldr	x1, [x20, 2608]
	ubfiz	x0, x19, 4, 16
	add	x0, x1, x0
	ldr	w0, [x0, 4]
	tbz	w0, #31, .L1251
	mov	w0, w19
	bl	flush_l2p_region
.L1251:
	add	w19, w19, 1
	and	w19, w19, 65535
	b	.L1250
	.size	l2p_flush, .-l2p_flush
	.align	2
	.global	FtlVendorPartWrite
	.type	FtlVendorPartWrite, %function
FtlVendorPartWrite:
	stp	x29, x30, [sp, -224]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x19, x19, :lo12:.LANCHOR0
	stp	x25, x26, [sp, 64]
	adrp	x22, __stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	w26, w0
	stp	x23, x24, [sp, 48]
	add	x0, x22, :lo12:__stack_chk_guard
	mov	w28, w1
	ldr	x1, [x0]
	str	x1, [x29, 216]
	mov	x1,0
	ldrh	w0, [x19, 300]
	add	w1, w26, w28
	cmp	w1, w0
	bhi	.L1263
	ldrh	w23, [x19, 312]
	mov	x25, x2
	add	x27, x19, 3888
	mov	w24, 0
	lsr	w23, w26, w23
.L1256:
	cbnz	w28, .L1261
.L1254:
	add	x22, x22, :lo12:__stack_chk_guard
	mov	w0, w24
	ldr	x2, [x29, 216]
	ldr	x1, [x22]
	eor	x1, x2, x1
	cbz	x1, .L1262
	bl	__stack_chk_fail
.L1261:
	ldrh	w1, [x19, 262]
	ldr	x0, [x19, 3600]
	udiv	w21, w26, w1
	ldr	w2, [x0, w23, uxtw 2]
	and	w0, w28, 65535
	msub	w21, w21, w1, w26
	sub	w20, w1, w21
	and	w20, w20, 65535
	cmp	w28, w20
	csel	w20, w0, w20, cc
	cbz	w2, .L1258
	cmp	w20, w1
	beq	.L1258
	ldr	x0, [x19, 3472]
	str	x0, [x29, 128]
	add	x0, x29, 152
	str	w2, [x29, 124]
	mov	w2, 1
	str	x0, [x29, 136]
	mov	w1, w2
	add	x0, x29, 120
	bl	FlashReadPages
.L1259:
	lsl	w4, w20, 9
	ldr	x0, [x19, 3472]
	lsl	w21, w21, 9
	mov	w2, w4
	asr	w21, w21, 2
	mov	x1, x25
	str	w4, [x29, 108]
	add	x0, x0, x21, sxtw 2
	bl	ftl_memcpy
	sub	w28, w28, w20
	ldr	x2, [x19, 3472]
	mov	w1, w23
	mov	x0, x27
	add	w26, w26, w20
	add	w23, w23, 1
	bl	FtlMapWritePage
	cmn	w0, #1
	ldr	w4, [x29, 108]
	csinv	w24, w24, wzr, ne
	add	x25, x25, x4, sxtw
	b	.L1256
.L1258:
	ldrh	w2, [x19, 314]
	mov	w1, 0
	ldr	x0, [x19, 3472]
	bl	ftl_memset
	b	.L1259
.L1263:
	mov	w24, -1
	b	.L1254
.L1262:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 224
	ret
	.size	FtlVendorPartWrite, .-FtlVendorPartWrite
	.align	2
	.global	Ftl_save_ext_data
	.type	Ftl_save_ext_data, %function
Ftl_save_ext_data:
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	mov	w1, 19539
	movk	w1, 0x4654, lsl 16
	ldr	w3, [x0, 2712]
	cmp	w3, w1
	bne	.L1271
	stp	x29, x30, [sp, -16]!
	add	x2, x0, 2712
	mov	w1, 87
	add	x29, sp, 0
	movk	w1, 0x5000, lsl 16
	str	w1, [x2, 4]
	ldr	w1, [x0, 2652]
	str	w1, [x2, 88]
	ldr	w1, [x0, 2656]
	str	w1, [x2, 92]
	ldr	w1, [x0, 2648]
	str	w1, [x2, 8]
	ldr	w1, [x0, 2636]
	str	w1, [x2, 12]
	ldr	w1, [x0, 2628]
	str	w1, [x2, 16]
	ldr	w1, [x0, 2644]
	str	w1, [x2, 20]
	ldr	w1, [x0, 2672]
	str	w1, [x2, 28]
	ldr	w1, [x0, 2680]
	str	w1, [x2, 32]
	ldr	w1, [x0, 2632]
	str	w1, [x2, 36]
	ldr	w1, [x0, 2640]
	str	w1, [x2, 40]
	ldr	w1, [x0, 2684]
	ldr	w0, [x0, 2688]
	stp	w1, w0, [x2, 44]
	mov	w1, 1
	mov	w0, 0
	bl	FtlVendorPartWrite
	ldp	x29, x30, [sp], 16
	ret
.L1271:
	ret
	.size	Ftl_save_ext_data, .-Ftl_save_ext_data
	.align	2
	.global	FtlEctTblFlush
	.type	FtlEctTblFlush, %function
FtlEctTblFlush:
	adrp	x1, .LANCHOR0
	add	x3, x1, :lo12:.LANCHOR0
	ldrh	w2, [x3, 3952]
	cmp	w2, 31
	bhi	.L1278
	add	w2, w2, 1
	strh	w2, [x3, 3952]
	mov	w2, 1
.L1275:
	cbnz	w0, .L1276
	add	x0, x1, :lo12:.LANCHOR0
	ldr	x0, [x0, 3552]
	ldr	w3, [x0, 20]
	ldr	w0, [x0, 16]
	add	w2, w2, w3
	cmp	w0, w2
	bcc	.L1280
.L1276:
	add	x0, x1, :lo12:.LANCHOR0
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	ldr	x1, [x0, 3552]
	ldr	w2, [x1, 16]
	str	w2, [x1, 20]
	mov	w2, 17221
	movk	w2, 0x4254, lsl 16
	str	w2, [x1]
	ldr	x2, [x0, 3552]
	ldrh	w1, [x0, 3536]
	lsl	w3, w1, 9
	str	wzr, [x2, 4]
	str	w3, [x2, 12]
	ldr	w3, [x2, 8]
	add	w3, w3, 1
	str	w3, [x2, 8]
	ldr	x2, [x0, 3552]
	mov	w0, 64
	bl	FtlVendorPartWrite
	bl	Ftl_save_ext_data
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
.L1278:
	mov	w2, 32
	b	.L1275
.L1280:
	mov	w0, 0
	ret
	.size	FtlEctTblFlush, .-FtlEctTblFlush
	.align	2
	.global	sftl_vendor_write
	.type	sftl_vendor_write, %function
sftl_vendor_write:
	stp	x29, x30, [sp, -16]!
	add	w0, w0, 256
	add	x29, sp, 0
	bl	FtlVendorPartWrite
	ldp	x29, x30, [sp], 16
	ret
	.size	sftl_vendor_write, .-sftl_vendor_write
	.align	2
	.global	FtlVendorPartRead
	.type	FtlVendorPartRead, %function
FtlVendorPartRead:
	stp	x29, x30, [sp, -224]!
	adrp	x3, .LANCHOR0
	add	x3, x3, :lo12:.LANCHOR0
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	mov	w22, w1
	stp	x23, x24, [sp, 48]
	mov	w23, w0
	stp	x25, x26, [sp, 64]
	adrp	x0, __stack_chk_guard
	stp	x19, x20, [sp, 16]
	add	x1, x0, :lo12:__stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	x25, x2
	str	x0, [x29, 104]
	ldr	x2, [x1]
	str	x2, [x29, 216]
	mov	x2,0
	ldrh	w1, [x3, 300]
	add	w2, w23, w22
	cmp	w2, w1
	bhi	.L1294
	ldrh	w21, [x3, 312]
	adrp	x26, .LC118
	mov	x28, x3
	add	x26, x26, :lo12:.LC118
	mov	w24, 0
	lsr	w21, w23, w21
.L1286:
	cbnz	w22, .L1292
.L1284:
	ldr	x1, [x29, 104]
	mov	w0, w24
	add	x1, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 216]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L1293
	bl	__stack_chk_fail
.L1292:
	ldrh	w19, [x28, 262]
	ldr	x0, [x28, 3600]
	udiv	w20, w23, w19
	ldr	w4, [x0, w21, uxtw 2]
	and	w0, w22, 65535
	msub	w20, w20, w19, w23
	sub	w19, w19, w20
	and	w19, w19, 65535
	cmp	w22, w19
	csel	w19, w0, w19, cc
	lsl	w27, w19, 9
	cbz	w4, .L1288
	ldr	x0, [x28, 3472]
	mov	w2, 1
	str	x0, [x29, 128]
	add	x0, x29, 152
	str	w4, [x29, 100]
	mov	w1, w2
	str	w4, [x29, 124]
	str	x0, [x29, 136]
	add	x0, x29, 120
	bl	FlashReadPages
	ldr	w0, [x29, 120]
	ldr	w4, [x29, 100]
	cmn	w0, #1
	ldr	w0, [x28, 3720]
	csinv	w24, w24, wzr, ne
	cmp	w0, 256
	bne	.L1290
	mov	w2, w4
	mov	w1, w21
	mov	x0, x26
	bl	sftl_printk
	ldr	x2, [x28, 3472]
	mov	w1, w21
	add	x0, x28, 3888
	bl	FtlMapWritePage
.L1290:
	ldr	x1, [x28, 3472]
	lsl	w20, w20, 9
	asr	w20, w20, 2
	mov	w2, w27
	mov	x0, x25
	add	x1, x1, x20, sxtw 2
	bl	ftl_memcpy
.L1291:
	add	w21, w21, 1
	sub	w22, w22, w19
	add	w23, w23, w19
	add	x25, x25, x27, sxtw
	b	.L1286
.L1288:
	mov	w2, w27
	mov	w1, 0
	mov	x0, x25
	bl	ftl_memset
	b	.L1291
.L1294:
	mov	w24, -1
	b	.L1284
.L1293:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 224
	ret
	.size	FtlVendorPartRead, .-FtlVendorPartRead
	.align	2
	.global	FtlLoadEctTbl
	.type	FtlLoadEctTbl, %function
FtlLoadEctTbl:
	stp	x29, x30, [sp, -32]!
	mov	w0, 64
	add	x29, sp, 0
	str	x19, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	ldrh	w1, [x19, 3536]
	ldr	x2, [x19, 3552]
	bl	FtlVendorPartRead
	ldr	x0, [x19, 3552]
	ldr	w1, [x0]
	mov	w0, 17221
	movk	w0, 0x4254, lsl 16
	cmp	w1, w0
	beq	.L1297
	adrp	x1, .LC119
	adrp	x0, .LC77
	add	x1, x1, :lo12:.LC119
	add	x0, x0, :lo12:.LC77
	bl	sftl_printk
	ldr	x0, [x19, 3552]
	mov	w1, 0
	ldrh	w2, [x19, 3536]
	lsl	w2, w2, 9
	bl	ftl_memset
.L1297:
	mov	w0, 0
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlLoadEctTbl, .-FtlLoadEctTbl
	.align	2
	.global	Ftl_load_ext_data
	.type	Ftl_load_ext_data, %function
Ftl_load_ext_data:
	stp	x29, x30, [sp, -48]!
	mov	w1, 1
	mov	w0, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x20, x19, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x22, x20, 2712
	mov	w21, 19539
	mov	x2, x22
	bl	FtlVendorPartRead
	ldr	w0, [x20, 2712]
	movk	w21, 0x4654, lsl 16
	cmp	w0, w21
	beq	.L1300
	mov	w2, 512
	mov	w1, 0
	mov	x0, x22
	bl	ftl_memset
	str	w21, [x20, 2712]
.L1300:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w2, 19539
	movk	w2, 0x4654, lsl 16
	ldr	w3, [x0, 2712]
	cmp	w3, w2
	bne	.L1301
	ldr	w2, [x0, 2800]
	str	w2, [x0, 2652]
	ldr	w2, [x0, 2804]
	str	w2, [x0, 2656]
	ldr	w2, [x0, 2720]
	str	w2, [x0, 2648]
	ldr	w2, [x0, 2724]
	str	w2, [x0, 2636]
	ldr	w2, [x0, 2728]
	str	w2, [x0, 2628]
	ldr	w2, [x0, 2732]
	str	w2, [x0, 2644]
	ldr	w2, [x0, 2740]
	str	w2, [x0, 2672]
	ldr	w2, [x0, 2744]
	str	w2, [x0, 2680]
	ldr	w2, [x0, 2748]
	str	w2, [x0, 2632]
	ldr	w2, [x0, 2752]
	ldr	w1, [x0, 2760]
	str	w2, [x0, 2640]
	ldr	w2, [x0, 2756]
	str	w1, [x0, 2688]
	str	w2, [x0, 2684]
.L1301:
	add	x0, x19, :lo12:.LANCHOR0
	ldr	w2, [x0, 2672]
	ldrh	w1, [x0, 296]
	ldr	w3, [x0, 2668]
	madd	w1, w1, w3, w2
	ldrh	w2, [x0, 244]
	udiv	w1, w1, w2
	str	w1, [x0, 2676]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
	.size	Ftl_load_ext_data, .-Ftl_load_ext_data
	.align	2
	.global	sftl_vendor_read
	.type	sftl_vendor_read, %function
sftl_vendor_read:
	stp	x29, x30, [sp, -16]!
	add	w0, w0, 256
	add	x29, sp, 0
	bl	FtlVendorPartRead
	ldp	x29, x30, [sp], 16
	ret
	.size	sftl_vendor_read, .-sftl_vendor_read
	.align	2
	.global	FtlMapBlkWriteDump_data
	.type	FtlMapBlkWriteDump_data, %function
FtlMapBlkWriteDump_data:
	ldr	w1, [x0, 56]
	cbz	w1, .L1313
	stp	x29, x30, [sp, -48]!
	adrp	x1, .LANCHOR0
	add	x2, x1, :lo12:.LANCHOR0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	str	x21, [sp, 32]
	add	x0, x2, 3720
	mov	x21, x1
	ldrh	w20, [x19, 6]
	ldr	x4, [x2, 3520]
	sub	w20, w20, #1
	ldr	x3, [x2, 3464]
	and	w20, w20, 65535
	ldr	x5, [x19, 40]
	stp	x3, x4, [x0, 8]
	ubfiz	x4, x20, 2, 16
	str	wzr, [x19, 56]
	ldr	w4, [x5, x4]
	str	w4, [x0, 4]
	cbz	w4, .L1307
	mov	w2, 1
	mov	w1, w2
	bl	FlashReadPages
.L1308:
	add	x1, x21, :lo12:.LANCHOR0
	mov	x0, x19
	ldr	x2, [x1, 3728]
	mov	w1, w20
	bl	FtlMapWritePage
	ldr	x21, [sp, 32]
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 48
	ret
.L1307:
	ldrh	w2, [x2, 314]
	mov	w1, 255
	mov	x0, x3
	bl	ftl_memset
	b	.L1308
.L1313:
	ret
	.size	FtlMapBlkWriteDump_data, .-FtlMapBlkWriteDump_data
	.align	2
	.global	FtlVpcTblFlush
	.type	FtlVpcTblFlush, %function
FtlVpcTblFlush:
	stp	x29, x30, [sp, -80]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	stp	x21, x22, [sp, 32]
	adrp	x22, .LANCHOR0
	add	x19, x22, :lo12:.LANCHOR0
	stp	x25, x26, [sp, 64]
	stp	x23, x24, [sp, 48]
	add	x20, x19, 2512
	add	x23, x19, 2696
	mov	w21, 0
	ldr	x26, [x19, 3520]
	mov	w24, 65535
	ldr	x0, [x19, 3456]
	str	x0, [x19, 3728]
	ldrh	w0, [x19, 2696]
	str	x26, [x19, 3736]
	strh	w0, [x26, 2]
	mov	w0, -3932
	strh	w0, [x26]
	str	wzr, [x26, 12]
	ldr	w0, [x19, 2704]
	stp	w0, wzr, [x26, 4]
	mov	w0, 19539
	ldrh	w1, [x19, 24]
	movk	w0, 0x4654, lsl 16
	str	w0, [x19, 2512]
	mov	w0, 87
	ldrb	w2, [x19, 30]
	strh	w1, [x19, 2526]
	movk	w0, 0x5000, lsl 16
	ldrh	w1, [x19, 26]
	str	w0, [x19, 2516]
	ldrh	w0, [x19, 2702]
	strh	w0, [x19, 2520]
	orr	w1, w2, w1, lsl 6
	ldrh	w0, [x19, 258]
	strb	w0, [x19, 2522]
	strh	w1, [x19, 2528]
	ldrh	w1, [x19, 80]
	ldrb	w2, [x19, 86]
	strh	w1, [x19, 2530]
	ldrh	w1, [x19, 82]
	ldrb	w0, [x19, 32]
	strb	w0, [x19, 2523]
	orr	w1, w2, w1, lsl 6
	ldrb	w0, [x19, 88]
	strh	w1, [x19, 2532]
	strb	w0, [x19, 2524]
	ldrh	w1, [x19, 128]
	ldrb	w0, [x19, 136]
	ldrb	w2, [x19, 134]
	strh	w1, [x19, 2534]
	strb	w0, [x19, 2525]
	ldrh	w1, [x19, 130]
	ldr	w0, [x19, 2668]
	str	w0, [x19, 2544]
	ldr	w0, [x19, 2660]
	orr	w1, w2, w1, lsl 6
	str	w0, [x19, 2552]
	ldr	w0, [x19, 2664]
	ldrh	w2, [x19, 314]
	str	w0, [x19, 2548]
	ldr	x0, [x19, 3728]
	strh	w1, [x19, 2536]
	mov	w1, 255
	bl	ftl_memset
	ldr	x0, [x19, 3728]
	mov	x1, x20
	mov	w2, 48
	mov	x20, x23
	adrp	x23, .LANCHOR1
	add	x23, x23, :lo12:.LANCHOR1
	add	x23, x23, 560
	bl	ftl_memcpy
	ldrh	w2, [x19, 244]
	ldr	x1, [x19, 72]
	ldr	x0, [x19, 3728]
	lsl	w2, w2, 1
	add	x0, x0, 48
	bl	ftl_memcpy
	mov	w0, 0
	bl	FtlUpdateVaildLpn
.L1317:
	ldrh	w2, [x20, 2]
	ldrh	w1, [x20]
	ldr	x0, [x19, 3456]
	str	x0, [x19, 3728]
	ldr	x0, [x19, 3520]
	str	x0, [x19, 3736]
	orr	w0, w2, w1, lsl 10
	str	w0, [x19, 3724]
	ldrh	w0, [x19, 308]
	sub	w0, w0, #1
	cmp	w2, w0
	blt	.L1318
	ldrh	w24, [x20, 4]
	strh	wzr, [x20, 2]
	strh	w1, [x20, 4]
	bl	FtlFreeSysBlkQueueOut
	ldr	w1, [x19, 2660]
	str	w1, [x20, 8]
	add	w2, w1, 1
	str	w2, [x19, 2660]
	ubfiz	w2, w0, 10, 16
	str	w2, [x19, 3724]
	strh	w0, [x20]
	strh	w0, [x26, 2]
	str	w1, [x26, 4]
.L1318:
	ldrh	w1, [x19, 314]
	ldr	x0, [x19, 3456]
	bl	js_hash
	str	w0, [x26, 12]
	mov	w3, 1
	add	x0, x19, 3720
	mov	w1, w3
	mov	w2, w3
	bl	FlashProgPages
	ldrh	w0, [x20, 2]
	ldr	w1, [x19, 3720]
	add	w0, w0, 1
	and	w0, w0, 65535
	strh	w0, [x20, 2]
	cmn	w1, #1
	bne	.L1319
	cmp	w0, 1
	bne	.L1320
	adrp	x0, .LC8
	mov	w2, 1145
	mov	x1, x23
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1320:
	ldrh	w0, [x20, 2]
	cmp	w0, 1
	bne	.L1321
	ldrh	w0, [x19, 308]
	sub	w0, w0, #1
	strh	w0, [x20, 2]
.L1321:
	add	w21, w21, 1
	and	w21, w21, 65535
	cmp	w21, 3
	bls	.L1317
	add	x22, x22, :lo12:.LANCHOR0
	adrp	x0, .LC120
	mov	w2, w21
	add	x0, x0, :lo12:.LC120
	ldr	w1, [x22, 3724]
	bl	sftl_printk
.L1323:
	b	.L1323
.L1319:
	cmp	w0, 1
	beq	.L1317
	cmp	w1, 256
	beq	.L1317
	mov	w0, 65535
	cmp	w24, w0
	beq	.L1324
	mov	w1, 1
	mov	w0, w24
	bl	FtlFreeSysBlkQueueIn
.L1324:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
	.size	FtlVpcTblFlush, .-FtlVpcTblFlush
	.align	2
	.global	FtlSysFlush
	.type	FtlSysFlush, %function
FtlSysFlush:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	l2p_flush
	mov	w0, 1
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlSysFlush, .-FtlSysFlush
	.align	2
	.global	sftl_deinit
	.type	sftl_deinit, %function
sftl_deinit:
	adrp	x0, .LANCHOR2
	ldr	w0, [x0, #:lo12:.LANCHOR2]
	cmp	w0, 1
	bne	.L1337
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	FtlSysFlush
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
.L1337:
	mov	w0, 0
	ret
	.size	sftl_deinit, .-sftl_deinit
	.align	2
	.global	sftl_discard
	.type	sftl_discard, %function
sftl_discard:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	w20, w1
	stp	x21, x22, [sp, 32]
	adrp	x21, __stack_chk_guard
	add	x1, x21, :lo12:__stack_chk_guard
	adrp	x19, .LANCHOR0
	ldr	x2, [x1]
	str	x2, [x29, 56]
	mov	x2,0
	add	x2, x19, :lo12:.LANCHOR0
	ldr	w1, [x2, 344]
	cmp	w1, w0
	bls	.L1351
	cmp	w1, w20
	bcc	.L1351
	add	w3, w0, w20
	cmp	w1, w3
	bcc	.L1351
	cmp	w20, 31
	bhi	.L1342
.L1359:
	mov	w0, 0
.L1340:
	add	x21, x21, :lo12:__stack_chk_guard
	ldr	x2, [x29, 56]
	ldr	x1, [x21]
	eor	x1, x2, x1
	cbz	x1, .L1348
	bl	__stack_chk_fail
.L1342:
	ldrh	w1, [x2, 262]
	udiv	w22, w0, w1
	msub	w0, w1, w22, w0
	ands	w0, w0, 65535
	beq	.L1343
	sub	w0, w1, w0
	add	w22, w22, 1
	cmp	w0, w20
	csel	w0, w0, w20, ls
	sub	w20, w20, w0, uxth
.L1343:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, -1
	str	w0, [x29, 52]
.L1344:
	ldrh	w0, [x19, 262]
	cmp	w20, w0
	bcs	.L1346
	ldr	w0, [x19, 3956]
	cmp	w0, 32
	bls	.L1359
	str	wzr, [x19, 3956]
	bl	l2p_flush
	bl	FtlVpcTblFlush
	b	.L1359
.L1346:
	mov	w2, 0
	add	x1, x29, 48
	mov	w0, w22
	bl	log2phys
	ldr	w0, [x29, 48]
	cmn	w0, #1
	beq	.L1345
	ldr	w0, [x19, 3956]
	mov	w2, 1
	add	x1, x29, 52
	add	w0, w0, 1
	str	w0, [x19, 3956]
	ldr	w0, [x19, 2632]
	add	w0, w0, 1
	str	w0, [x19, 2632]
	mov	w0, w22
	bl	log2phys
	ldr	w0, [x29, 48]
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	bl	decrement_vpc_count
.L1345:
	ldrh	w0, [x19, 262]
	add	w22, w22, 1
	sub	w20, w20, w0
	b	.L1344
.L1351:
	mov	w0, -1
	b	.L1340
.L1348:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 64
	ret
	.size	sftl_discard, .-sftl_discard
	.align	2
	.global	FtlVpcCheckAndModify
	.type	FtlVpcCheckAndModify, %function
FtlVpcCheckAndModify:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	stp	x21, x22, [sp, 32]
	adrp	x22, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	add	x0, x22, :lo12:__stack_chk_guard
	stp	x25, x26, [sp, 64]
	add	x19, x20, :lo12:.LANCHOR0
	adrp	x23, .LANCHOR1
	mov	w21, 0
	ldr	x1, [x0]
	str	x1, [x29, 88]
	mov	x1,0
	adrp	x0, .LC111
	add	x1, x23, :lo12:.LANCHOR1
	add	x0, x0, :lo12:.LC111
	add	x1, x1, 576
	bl	sftl_printk
	ldr	x0, [x19, 3560]
	mov	w1, 0
	ldrh	w2, [x19, 246]
	lsl	w2, w2, 1
	bl	ftl_memset
.L1361:
	ldr	w0, [x19, 2624]
	cmp	w21, w0
	bcc	.L1363
	add	x20, x20, :lo12:.LANCHOR0
	adrp	x24, .LC121
	add	x26, x20, 2592
	add	x24, x24, :lo12:.LC121
	mov	w19, 0
	mov	w25, 65535
.L1364:
	ldrh	w0, [x20, 244]
	cmp	w0, w19
	bhi	.L1370
	bl	l2p_flush
	add	x22, x22, :lo12:__stack_chk_guard
	bl	FtlVpcTblFlush
	ldr	x1, [x29, 88]
	ldr	x0, [x22]
	eor	x0, x1, x0
	cbz	x0, .L1371
	bl	__stack_chk_fail
.L1363:
	mov	w2, 0
	add	x1, x29, 84
	mov	w0, w21
	bl	log2phys
	ldr	w0, [x29, 84]
	cmn	w0, #1
	beq	.L1362
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	ldr	x2, [x19, 3560]
	ubfiz	x0, x0, 1, 16
	ldrh	w1, [x2, x0]
	add	w1, w1, 1
	strh	w1, [x2, x0]
.L1362:
	add	w21, w21, 1
	b	.L1361
.L1370:
	ldr	x0, [x20, 72]
	ubfiz	x21, x19, 1, 16
	ldrh	w2, [x0, x21]
	ldr	x0, [x20, 3560]
	ldrh	w3, [x0, x21]
	cmp	w2, w3
	beq	.L1366
	cmp	w2, w25
	beq	.L1366
	mov	x0, x24
	mov	w1, w19
	bl	sftl_printk
	ldrh	w0, [x20, 24]
	cmp	w0, w19
	beq	.L1366
	ldrh	w0, [x20, 128]
	cmp	w0, w19
	beq	.L1366
	ldrh	w0, [x20, 80]
	cmp	w0, w19
	beq	.L1366
	ldr	x0, [x20, 72]
	ldrh	w1, [x0, x21]
	cbnz	w1, .L1368
	ldr	x1, [x20, 3560]
	ldrh	w1, [x1, x21]
	strh	w1, [x0, x21]
	mov	w1, w19
	mov	x0, x26
	bl	List_remove_node
	ldrh	w0, [x20, 228]
	cbnz	w0, .L1369
	add	x1, x23, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2330
	add	x1, x1, 576
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1369:
	ldrh	w0, [x20, 228]
	sub	w0, w0, #1
	strh	w0, [x20, 228]
	mov	w0, w19
	bl	INSERT_DATA_LIST
.L1366:
	add	w19, w19, 1
	and	w19, w19, 65535
	b	.L1364
.L1368:
	ldr	x1, [x20, 3560]
	ldrh	w1, [x1, x21]
	strh	w1, [x0, x21]
	mov	w0, w19
	bl	update_vpc_list
	b	.L1366
.L1371:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 96
	ret
	.size	FtlVpcCheckAndModify, .-FtlVpcCheckAndModify
	.align	2
	.global	allocate_new_data_superblock
	.type	allocate_new_data_superblock, %function
allocate_new_data_superblock:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	str	x21, [sp, 32]
	mov	x21, x0
	ldrh	w20, [x0]
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w0, [x0, 244]
	cmp	w0, w20
	bcs	.L1377
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 2766
	add	x1, x1, 600
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1377:
	mov	w0, 65535
	cmp	w20, w0
	beq	.L1378
	add	x1, x19, :lo12:.LANCHOR0
	ubfiz	x0, x20, 1, 16
	ldr	x1, [x1, 72]
	ldrh	w0, [x1, x0]
	cbz	w0, .L1379
	mov	w0, w20
	bl	INSERT_DATA_LIST
.L1378:
	add	x2, x19, :lo12:.LANCHOR0
	mov	w0, 1
	strb	w0, [x21, 8]
	mov	w1, 65535
	ldrh	w0, [x2, 3706]
	cmp	w0, w1
	beq	.L1380
	cmp	w20, w0
	bne	.L1381
	ldr	x2, [x2, 72]
	ubfiz	x1, x0, 1, 16
	ldrh	w1, [x2, x1]
	cbz	w1, .L1382
.L1381:
	bl	update_vpc_list
.L1382:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, -1
	strh	w0, [x19, 3706]
.L1380:
	mov	x0, x21
	bl	allocate_data_superblock
	bl	l2p_flush
	mov	w0, 0
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
	ldr	x21, [sp, 32]
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 48
	ret
.L1379:
	mov	w0, w20
	bl	INSERT_FREE_LIST
	b	.L1378
	.size	allocate_new_data_superblock, .-allocate_new_data_superblock
	.align	2
	.global	FtlProgPages
	.type	FtlProgPages, %function
FtlProgPages:
	stp	x29, x30, [sp, -96]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x20, x3
	stp	x21, x22, [sp, 32]
	mov	w22, w1
	stp	x23, x24, [sp, 48]
	adrp	x21, __stack_chk_guard
	stp	x25, x26, [sp, 64]
	add	x2, x21, :lo12:__stack_chk_guard
	ubfiz	x22, x22, 5, 32
	mov	x23, x0
	ldr	x3, [x2]
	str	x3, [x29, 88]
	mov	x3,0
	add	x22, x22, 4
	ldrb	w3, [x20, 9]
	adrp	x24, .LC122
	adrp	x25, .LANCHOR0
	add	x19, x23, 4
	add	x22, x23, x22
	add	x24, x24, :lo12:.LC122
	add	x26, x25, :lo12:.LANCHOR0
	mov	w2, 0
	bl	FlashProgPages
.L1388:
	cmp	x22, x19
	beq	.L1394
	sub	x23, x19, #4
	b	.L1395
.L1390:
	ldr	w1, [x19]
	mov	x0, x24
	bl	sftl_printk
	ldr	w0, [x19]
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	bl	decrement_vpc_count
	ldrh	w0, [x20, 4]
	cbnz	w0, .L1389
	mov	x0, x20
	bl	allocate_new_data_superblock
.L1389:
	mov	x0, x20
	bl	get_new_active_ppa
	str	w0, [x19]
	mov	w2, 0
	str	w0, [x29, 84]
	mov	w1, 1
	ldrb	w3, [x20, 9]
	mov	x0, x23
	bl	FlashProgPages
.L1395:
	ldr	w2, [x19, -4]
	cmp	w2, 256
	ccmn	w2, #1, 4, ne
	beq	.L1390
	ldrb	w1, [x20, 6]
	ldrh	w0, [x26, 236]
	cmp	w1, w0
	bcc	.L1391
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 985
	add	x1, x1, 632
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1391:
	add	x1, x29, 96
	ldr	w0, [x23, 4]
	mov	w2, 1
	str	w0, [x1, -12]!
	ldr	w0, [x23, 24]
	bl	log2phys
	ldr	x0, [x23, 16]
	ldr	w4, [x0, 12]
	lsr	w0, w4, 10
	bl	P2V_block_in_plane
	and	w1, w0, 65535
	mov	w23, w1
	cmn	w4, #1
	beq	.L1392
	add	x2, x25, :lo12:.LANCHOR0
	ubfiz	x0, x1, 1, 16
	ldr	x2, [x2, 72]
	ldrh	w0, [x2, x0]
	cbnz	w0, .L1393
	adrp	x0, .LC123
	mov	w2, 0
	add	x0, x0, :lo12:.LC123
	bl	sftl_printk
.L1393:
	mov	w0, w23
	bl	decrement_vpc_count
.L1392:
	add	x19, x19, 32
	b	.L1388
.L1394:
	adrp	x0, .LANCHOR0+236
	ldrb	w1, [x20, 6]
	ldrh	w0, [x0, #:lo12:.LANCHOR0+236]
	cmp	w1, w0
	bcc	.L1387
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1000
	add	x1, x1, 632
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1387:
	add	x21, x21, :lo12:__stack_chk_guard
	ldr	x1, [x29, 88]
	ldr	x0, [x21]
	eor	x0, x1, x0
	cbz	x0, .L1397
	bl	__stack_chk_fail
.L1397:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 96
	ret
	.size	FtlProgPages, .-FtlProgPages
	.align	2
	.global	FtlGcFreeTempBlock
	.type	FtlGcFreeTempBlock, %function
FtlGcFreeTempBlock:
	stp	x29, x30, [sp, -96]!
	mov	w3, 65535
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	stp	x21, x22, [sp, 32]
	adrp	x21, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	add	x1, x21, :lo12:__stack_chk_guard
	stp	x25, x26, [sp, 64]
	ldr	x2, [x1]
	str	x2, [x29, 88]
	mov	x2,0
	add	x2, x19, :lo12:.LANCHOR0
	ldrh	w4, [x2, 128]
	ldrh	w1, [x2, 306]
	cmp	w4, w3
	bne	.L1403
.L1410:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w2, 65535
	add	x1, x0, 128
	ldrh	w3, [x0, 128]
	str	wzr, [x0, 3712]
	cmp	w3, w2
	bne	.L1424
.L1428:
	mov	w0, 0
.L1402:
	add	x21, x21, :lo12:__stack_chk_guard
	ldr	x2, [x29, 88]
	ldr	x1, [x21]
	eor	x1, x2, x1
	cbz	x1, .L1419
	bl	__stack_chk_fail
.L1403:
	adrp	x20, .LANCHOR2
	cbz	w0, .L1406
	add	x0, x20, :lo12:.LANCHOR2
	ldrh	w4, [x0, 4]
	cmp	w4, w3
	beq	.L1407
.L1408:
	mov	w1, 2
.L1406:
	add	x20, x20, :lo12:.LANCHOR2
	add	x0, x19, :lo12:.LANCHOR0
	add	x0, x0, 128
	bl	FtlGcScanTempBlk
	str	w0, [x29, 84]
	mov	w0, 65535
	ldrh	w1, [x20, 4]
	cmp	w1, w0
	beq	.L1410
	mov	w0, 1
	b	.L1402
.L1407:
	strh	wzr, [x0, 4]
	ldrh	w0, [x2, 228]
	cmp	w0, 17
	bhi	.L1408
	b	.L1406
.L1424:
	ldrh	w2, [x0, 3296]
	ldrb	w1, [x1, 7]
	ldrh	w0, [x0, 306]
	mul	w0, w1, w0
	cmp	w2, w0
	beq	.L1411
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 164
	add	x1, x1, 648
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1411:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w22, 0
	mov	x20, x0
	mov	w26, 12
	ldrb	w1, [x0, 135]
	ldrh	w4, [x0, 306]
	ldr	x2, [x0, 72]
	ldrh	w3, [x0, 128]
	mul	w1, w1, w4
	strh	w1, [x2, x3, lsl 1]
	ldr	w2, [x0, 2648]
	ldrh	w1, [x0, 3296]
	add	w1, w1, w2
	str	w1, [x0, 2648]
.L1412:
	ldrh	w0, [x20, 3296]
	cmp	w0, w22
	bhi	.L1416
	mov	w0, -1
	bl	decrement_vpc_count
	ldrh	w0, [x20, 128]
	ldr	x2, [x20, 72]
	ubfiz	x1, x0, 1, 16
	ldrh	w1, [x2, x1]
	cbz	w1, .L1417
	bl	INSERT_DATA_LIST
.L1418:
	add	x19, x19, :lo12:.LANCHOR0
	mov	w20, -1
	strh	wzr, [x19, 3296]
	strh	w20, [x19, 128]
	strh	wzr, [x19, 3284]
	bl	l2p_flush
	bl	FtlVpcTblFlush
	strh	w20, [x19, 176]
	ldrh	w0, [x19, 2692]
	ldrh	w1, [x19, 228]
	add	w0, w0, w0, lsl 1
	cmp	w1, w0, lsr 2
	ble	.L1428
	mov	w0, 20
	strh	w0, [x19, 3224]
	b	.L1428
.L1416:
	umull	x24, w22, w26
	ldr	x25, [x20, 3304]
	ldr	w1, [x20, 2624]
	add	x23, x25, x24
	ldr	w0, [x23, 8]
	cmp	w0, w1
	bcc	.L1413
.L1426:
	ldrh	w0, [x20, 128]
	b	.L1427
.L1413:
	add	x1, x29, 84
	mov	w2, 0
	bl	log2phys
	ldr	w0, [x25, x24]
	ldr	w1, [x29, 84]
	cmp	w0, w1
	bne	.L1415
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	mov	w24, w0
	ldr	w0, [x23, 8]
	mov	w2, 1
	add	x1, x23, 4
	bl	log2phys
	mov	w0, w24
.L1427:
	bl	decrement_vpc_count
	b	.L1414
.L1415:
	ldr	w0, [x23, 4]
	cmp	w1, w0
	bne	.L1426
.L1414:
	add	w22, w22, 1
	and	w22, w22, 65535
	b	.L1412
.L1417:
	bl	INSERT_FREE_LIST
	b	.L1418
.L1419:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 96
	ret
	.size	FtlGcFreeTempBlock, .-FtlGcFreeTempBlock
	.align	2
	.global	FtlGcPageRecovery
	.type	FtlGcPageRecovery, %function
FtlGcPageRecovery:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	add	x20, x19, 128
	mov	x0, x20
	ldrh	w1, [x19, 306]
	bl	FtlGcScanTempBlk
	ldrh	w1, [x19, 130]
	ldrh	w0, [x19, 306]
	cmp	w1, w0
	bcc	.L1429
	add	x0, x19, 3640
	bl	FtlMapBlkWriteDump_data
	mov	w0, 0
	bl	FtlGcFreeTempBlock
	str	wzr, [x19, 3712]
.L1429:
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlGcPageRecovery, .-FtlGcPageRecovery
	.align	2
	.global	FtlPowerLostRecovery
	.type	FtlPowerLostRecovery, %function
FtlPowerLostRecovery:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	add	x20, x19, 24
	add	x19, x19, 80
	mov	x0, x20
	str	wzr, [x19, 3676]
	bl	FtlRecoverySuperblock
	mov	x0, x20
	bl	FtlSlcSuperblockCheck
	mov	x0, x19
	bl	FtlRecoverySuperblock
	mov	x0, x19
	bl	FtlSlcSuperblockCheck
	bl	FtlGcPageRecovery
	mov	w0, -1
	bl	decrement_vpc_count
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	FtlPowerLostRecovery, .-FtlPowerLostRecovery
	.align	2
	.global	Ftl_gc_temp_data_write_back
	.type	Ftl_gc_temp_data_write_back, %function
Ftl_gc_temp_data_write_back:
	stp	x29, x30, [sp, -32]!
	adrp	x11, .LANCHOR0
	mov	w3, 0
	mov	w2, 0
	add	x29, sp, 0
	str	x19, [sp, 16]
	add	x19, x11, :lo12:.LANCHOR0
	ldr	w1, [x19, 3240]
	ldr	x0, [x19, 3440]
	bl	FlashProgPages
	mov	w10, 0
.L1435:
	ldr	w1, [x19, 3240]
	cmp	w10, w1
	bcc	.L1438
	ldr	x0, [x19, 3440]
	bl	FtlGcBufFree
	str	wzr, [x19, 3240]
	ldrh	w0, [x19, 132]
	cbnz	w0, .L1440
	mov	w0, 1
	bl	FtlGcFreeTempBlock
	mov	w0, 1
.L1434:
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L1438:
	ldr	x2, [x19, 3440]
	ubfiz	x0, x10, 5, 16
	add	x1, x2, x0
	ldr	w2, [x2, x0]
	ldr	x3, [x1, 16]
	cmn	w2, #1
	bne	.L1436
.L1442:
	ldr	w1, [x1, 4]
	ldr	w0, [x3, 12]
	bl	FtlGcUpdatePage
	add	w10, w10, 1
	and	w10, w10, 65535
	b	.L1435
.L1436:
	ldr	w2, [x3, 8]
	b	.L1442
.L1440:
	mov	w0, 0
	b	.L1434
	.size	Ftl_gc_temp_data_write_back, .-Ftl_gc_temp_data_write_back
	.align	2
	.global	Ftl_get_new_temp_ppa
	.type	Ftl_get_new_temp_ppa, %function
Ftl_get_new_temp_ppa:
	stp	x29, x30, [sp, -32]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	add	x0, x19, :lo12:.LANCHOR0
	add	x1, x0, 128
	ldrh	w2, [x0, 128]
	mov	w0, 65535
	cmp	w2, w0
	beq	.L1444
	ldrh	w0, [x1, 4]
	cbnz	w0, .L1445
.L1444:
	add	x20, x19, :lo12:.LANCHOR0
	mov	w0, 0
	bl	FtlGcFreeTempBlock
	add	x0, x20, 128
	strb	wzr, [x0, 8]
	bl	allocate_data_superblock
	strh	wzr, [x20, 3284]
	strh	wzr, [x20, 3296]
	bl	l2p_flush
	mov	w0, 0
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
.L1445:
	add	x0, x19, :lo12:.LANCHOR0
	add	x0, x0, 128
	bl	get_new_active_ppa
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	Ftl_get_new_temp_ppa, .-Ftl_get_new_temp_ppa
	.align	2
	.global	rk_ftl_garbage_collect
	.type	rk_ftl_garbage_collect, %function
rk_ftl_garbage_collect:
	stp	x29, x30, [sp, -144]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x19, .LANCHOR0
	stp	x23, x24, [sp, 48]
	adrp	x24, __stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	w27, w0
	stp	x21, x22, [sp, 32]
	add	x0, x24, :lo12:__stack_chk_guard
	stp	x25, x26, [sp, 64]
	str	x24, [x29, 112]
	ldr	x1, [x0]
	str	x1, [x29, 136]
	mov	x1,0
	add	x0, x19, :lo12:.LANCHOR0
	ldr	w1, [x0, 3416]
	cbnz	w1, .L1527
	ldrh	w1, [x0, 2584]
	cmp	w1, 47
	bls	.L1527
	adrp	x1, .LANCHOR2+4
	ldrh	w2, [x1, #:lo12:.LANCHOR2+4]
	mov	w1, 65535
	cmp	w2, w1
	bne	.L1449
.L1452:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w2, 65535
	ldrh	w1, [x0, 224]
	cmp	w1, w2
	bne	.L1450
.L1451:
	add	x20, x19, :lo12:.LANCHOR0
	bl	FtlReadRefresh
	mov	w21, 65535
	ldr	w0, [x20, 3228]
	ldrh	w1, [x20, 176]
	add	w0, w0, 1
	add	w0, w0, w27, lsl 7
	str	w0, [x20, 3228]
	cmp	w1, w21
	bne	.L1453
	ldrh	w21, [x20, 128]
	cmp	w21, w1
	bne	.L1530
	ldrh	w22, [x20, 226]
	cmp	w22, w21
	bne	.L1453
	ldrh	w1, [x20, 228]
	mov	w2, 1024
	cmp	w1, 24
	mov	w1, 5120
	csel	w1, w1, w2, cc
	cmp	w0, w1
	bls	.L1533
	strh	wzr, [x20, 3960]
	str	wzr, [x20, 3228]
	bl	GetSwlReplaceBlock
	and	w21, w0, 65535
	cmp	w21, w22
	bne	.L1534
	ldrh	w1, [x20, 228]
	ldrh	w0, [x20, 3226]
	cmp	w1, w0
	bcs	.L1456
	mov	w0, 64
	bl	List_get_gc_head_node
	and	w0, w0, 65535
	cmp	w0, w21
	beq	.L1458
	ldr	x1, [x20, 72]
	ubfiz	x0, x0, 1, 16
	ldrh	w0, [x1, x0]
	cmp	w0, 7
	bhi	.L1459
	mov	w0, 0
	bl	List_get_gc_head_node
	and	w22, w0, 65535
	mov	w0, 128
	strh	w0, [x20, 3226]
	cmp	w22, w21
	bne	.L1455
.L1458:
	bl	FtlGcReFreshBadBlk
.L1453:
	cmp	w27, 0
	mov	w1, 65535
	ccmp	w21, w1, 0, eq
	bne	.L1461
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w0, [x1, 228]
	cmp	w0, 24
	bhi	.L1535
	cmp	w0, 16
	ldrh	w20, [x1, 306]
	bls	.L1463
	lsr	w20, w20, 5
.L1462:
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w2, [x1, 3224]
	cmp	w2, w0
	bcs	.L1466
	ldrh	w0, [x1, 128]
	mov	w2, 65535
	cmp	w0, w2
	bne	.L1467
	ldrh	w2, [x1, 226]
	cmp	w2, w0
	bne	.L1467
	ldrh	w0, [x1, 3960]
	cbnz	w0, .L1468
	ldr	w2, [x1, 2624]
	ldr	w3, [x1, 2620]
	add	w2, w2, w2, lsl 1
	cmp	w3, w2, lsr 2
	bcs	.L1469
.L1468:
	add	x2, x19, :lo12:.LANCHOR0
	ldrh	w1, [x2, 2692]
	add	w1, w1, w1, lsl 1
	asr	w1, w1, 2
	strh	w1, [x2, 3224]
.L1470:
	add	x19, x19, :lo12:.LANCHOR0
	str	wzr, [x19, 3236]
.L1447:
	ldr	x1, [x29, 112]
	add	x24, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 136]
	ldr	x1, [x24]
	eor	x1, x2, x1
	cbz	x1, .L1525
	bl	__stack_chk_fail
.L1449:
	ldrh	w0, [x0, 128]
	cmp	w0, w1
	beq	.L1452
	mov	w0, 1
	bl	FtlGcFreeTempBlock
	cbz	w0, .L1452
	mov	w0, 1
	b	.L1447
.L1450:
	ldrh	w3, [x0, 226]
	cmp	w3, w2
	bne	.L1451
	strh	w1, [x0, 226]
	mov	w1, -1
	strh	w1, [x0, 224]
	b	.L1451
.L1459:
	mov	w0, 64
.L1565:
	strh	w0, [x20, 3226]
	b	.L1458
.L1456:
	mov	w0, 80
	b	.L1565
.L1534:
	mov	w22, w21
.L1455:
	add	x0, x19, :lo12:.LANCHOR0
	ubfiz	x1, x22, 1, 32
	mov	w21, w22
	ldr	x2, [x0, 72]
	ldr	x3, [x0, 2600]
	ldrh	w5, [x0, 3224]
	ldrh	w4, [x3, x1]
	ldrh	w3, [x2, x1]
	mov	w1, w22
	ldrh	w2, [x0, 228]
	adrp	x0, .LC124
	add	x0, x0, :lo12:.LC124
	bl	sftl_printk
	b	.L1458
.L1530:
	mov	w21, w1
	b	.L1453
.L1533:
	mov	w21, w22
	b	.L1453
.L1463:
	cmp	w0, 12
	bls	.L1464
	lsr	w20, w20, 4
	b	.L1462
.L1464:
	mov	w1, w20
	cmp	w0, 9
	lsr	w20, w20, 2
	csel	w20, w20, w1, cs
	b	.L1462
.L1535:
	mov	w20, 1
	b	.L1462
.L1469:
	mov	w2, 18
	strh	w2, [x1, 3224]
	b	.L1470
.L1467:
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w0, [x1, 2692]
	add	w0, w0, w0, lsl 1
	asr	w0, w0, 2
	strh	w0, [x1, 3224]
.L1466:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w0, [x0, 3312]
	cbz	w0, .L1536
	add	w20, w20, 32
	and	w20, w20, 65535
.L1536:
	mov	w21, 65535
.L1472:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w2, 65535
	ldrh	w1, [x0, 176]
	cmp	w1, w2
	bne	.L1481
	cmp	w21, w1
	beq	.L1482
	strh	w21, [x0, 176]
.L1483:
	add	x5, x19, :lo12:.LANCHOR0
	mov	w1, 65535
	ldrh	w0, [x5, 176]
	strb	wzr, [x5, 184]
	cmp	w0, w1
	beq	.L1481
	bl	IsBlkInGcList
	cbz	w0, .L1486
	mov	w0, -1
	strh	w0, [x5, 176]
.L1486:
	add	x22, x19, :lo12:.LANCHOR0
	mov	w0, 65535
	add	x23, x22, 176
	ldrh	w1, [x22, 176]
	cmp	w1, w0
	beq	.L1481
	mov	x0, x23
	bl	make_superblock
	ldrh	w1, [x22, 176]
	ldr	x0, [x22, 72]
	strh	wzr, [x22, 178]
	strb	wzr, [x22, 182]
	strh	wzr, [x22, 3962]
	ldrh	w0, [x0, x1, lsl 1]
	strh	w0, [x22, 3964]
.L1481:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 176]
	ldrh	w2, [x0, 24]
	cmp	w2, w1
	beq	.L1487
	ldrh	w2, [x0, 80]
	cmp	w2, w1
	beq	.L1487
	ldrh	w0, [x0, 128]
	cmp	w0, w1
	bne	.L1488
.L1487:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w1, -1
	strh	w1, [x0, 176]
.L1488:
	adrp	x23, .LANCHOR1
	add	x23, x23, :lo12:.LANCHOR1
	add	x23, x23, 672
.L1523:
	add	x22, x19, :lo12:.LANCHOR0
	mov	w0, 65535
	ldrh	w25, [x22, 176]
	cmp	w25, w0
	bne	.L1489
	mov	w26, 2
	str	wzr, [x22, 3236]
.L1490:
	ldrh	w5, [x22, 3232]
	mov	w0, w5
	bl	List_get_gc_head_node
	and	w6, w0, 65535
	strh	w6, [x22, 176]
	cmp	w6, w25
	bne	.L1491
	strh	wzr, [x22, 3232]
	mov	w0, 8
	b	.L1447
.L1461:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w2, [x0, 128]
	cmp	w2, w1
	bne	.L1539
	ldrh	w1, [x0, 226]
	cmp	w1, w2
	bne	.L1539
	cmp	w21, w1
	bne	.L1539
	ldrh	w1, [x0, 176]
	cmp	w1, w21
	beq	.L1473
.L1539:
	mov	w20, 1
	b	.L1472
.L1473:
	ldrh	w2, [x0, 228]
	ldrh	w1, [x0, 3224]
	str	wzr, [x0, 3236]
	cmp	w2, w1
	bls	.L1474
	ldrh	w1, [x0, 3960]
	cbnz	w1, .L1475
	ldr	w1, [x0, 2624]
	ldr	w2, [x0, 2620]
	add	w1, w1, w1, lsl 1
	cmp	w2, w1, lsr 2
	bcs	.L1476
.L1475:
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w0, [x1, 2692]
	add	w0, w0, w0, lsl 1
	asr	w0, w0, 2
	strh	w0, [x1, 3224]
.L1477:
	bl	FtlReadRefresh
	mov	w0, 0
	bl	List_get_gc_head_node
	add	x1, x19, :lo12:.LANCHOR0
	ubfiz	x0, x0, 1, 16
	ldr	x2, [x1, 72]
	ldrh	w0, [x2, x0]
	cmp	w0, 4
	bls	.L1474
	ldrh	w0, [x1, 3960]
	b	.L1447
.L1476:
	mov	w1, 18
	strh	w1, [x0, 3224]
	b	.L1477
.L1474:
	add	x20, x19, :lo12:.LANCHOR0
	ldrh	w0, [x20, 3960]
	cbnz	w0, .L1539
	ldrh	w5, [x20, 2692]
	add	w0, w5, w5, lsl 1
	asr	w0, w0, 2
	strh	w0, [x20, 3224]
	mov	w0, 0
	bl	List_get_gc_head_node
	ldr	x1, [x20, 72]
	ubfiz	x0, x0, 1, 16
	ldrh	w2, [x20, 236]
	ldrh	w1, [x1, x0]
	ldrh	w0, [x20, 308]
	mul	w0, w0, w2
	mov	w2, 2
	sdiv	w0, w0, w2
	cmp	w1, w0
	ble	.L1479
	ldrh	w0, [x20, 228]
	sub	w5, w5, #1
	cmp	w0, w5
	blt	.L1479
	bl	FtlReadRefresh
	ldrh	w0, [x20, 3960]
	b	.L1447
.L1479:
	cbnz	w1, .L1539
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, -1
	bl	decrement_vpc_count
	ldrh	w0, [x19, 228]
	add	w0, w0, 1
	b	.L1447
.L1482:
	ldrh	w1, [x0, 226]
	cmp	w1, w21
	beq	.L1483
	ldr	x2, [x0, 72]
	ubfiz	x1, x1, 1, 16
	ldrh	w1, [x2, x1]
	cbnz	w1, .L1484
	mov	w1, -1
	strh	w1, [x0, 226]
.L1484:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 226]
	strh	w1, [x0, 176]
	mov	w1, -1
	strh	w1, [x0, 226]
	b	.L1483
.L1491:
	mov	w0, w6
	bl	IsBlkInGcList
	add	w5, w5, 1
	cbz	w0, .L1492
	strh	w5, [x22, 3232]
	b	.L1490
.L1492:
	ldrh	w4, [x22, 236]
	ubfiz	x1, x6, 1, 16
	ldrh	w0, [x22, 306]
	and	w5, w5, 65535
	ldr	x2, [x22, 72]
	strh	w5, [x22, 3232]
	mul	w0, w0, w4
	ldrh	w3, [x2, x1]
	sdiv	w4, w0, w26
	cmp	w3, w4
	bgt	.L1494
	cmp	w5, 48
	bls	.L1495
	cmp	w3, 8
	bls	.L1495
	ldrh	w3, [x22, 3284]
	cmp	w3, 35
	bhi	.L1495
.L1494:
	strh	wzr, [x22, 3232]
.L1495:
	ldrh	w1, [x2, x1]
	cmp	w0, w1
	bgt	.L1496
	cmp	w21, w25
	bne	.L1496
	ldrh	w0, [x22, 3232]
	cmp	w0, 3
	bhi	.L1496
	mov	w0, -1
	strh	wzr, [x22, 3232]
	strh	w0, [x22, 176]
.L1566:
	ldrh	w0, [x22, 3960]
	b	.L1447
.L1496:
	cbnz	w1, .L1497
	mov	w0, -1
	bl	decrement_vpc_count
	ldrh	w0, [x22, 3232]
	add	w0, w0, 1
	strh	w0, [x22, 3232]
	b	.L1490
.L1497:
	add	x0, x19, :lo12:.LANCHOR0
	strb	wzr, [x0, 184]
	ldrh	w0, [x0, 24]
	cmp	w0, w6
	bne	.L1498
	adrp	x0, .LC8
	mov	w2, 717
	mov	x1, x23
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1498:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 176]
	ldrh	w0, [x0, 80]
	cmp	w1, w0
	bne	.L1499
	adrp	x0, .LC8
	mov	w2, 718
	mov	x1, x23
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1499:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 176]
	ldrh	w0, [x0, 128]
	cmp	w1, w0
	bne	.L1500
	adrp	x0, .LC8
	mov	w2, 719
	mov	x1, x23
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1500:
	add	x22, x19, :lo12:.LANCHOR0
	add	x25, x22, 176
	mov	x0, x25
	bl	make_superblock
	ldrh	w1, [x22, 176]
	ldr	x0, [x22, 72]
	strh	wzr, [x22, 3962]
	ldrh	w0, [x0, x1, lsl 1]
	strh	w0, [x22, 3964]
	strh	wzr, [x22, 178]
	strb	wzr, [x22, 182]
.L1489:
	add	x1, x19, :lo12:.LANCHOR0
	mov	w0, 1
	str	w0, [x1, 3416]
	ldrh	w26, [x1, 306]
	cbz	w27, .L1501
	ldrh	w0, [x1, 236]
	ldrh	w2, [x1, 176]
	ldr	x1, [x1, 72]
	mul	w0, w0, w26
	ldrh	w1, [x1, x2, lsl 1]
	sub	w0, w0, w1
	mov	w1, 4
	sdiv	w0, w0, w1
	add	w20, w20, w0
	and	w20, w20, 65535
.L1501:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w0, [x0, 178]
	add	w1, w0, w20
	cmp	w1, w26
	ble	.L1502
	sub	w20, w26, w0
	and	w20, w20, 65535
.L1502:
	mov	w28, 0
.L1503:
	cmp	w20, w28, uxth
	bls	.L1511
	add	x0, x19, :lo12:.LANCHOR0
	mov	w22, 0
	add	x3, x0, 192
	mov	w1, 0
	mov	w6, 65535
	ldrh	w4, [x0, 178]
	ldrh	w7, [x0, 236]
	add	w4, w4, w28
	b	.L1512
.L1505:
	ldrh	w2, [x3]
	cmp	w2, w6
	beq	.L1504
	ldr	x5, [x0, 3272]
	ubfiz	x8, x22, 5, 16
	add	w22, w22, 1
	orr	w2, w4, w2, lsl 10
	add	x5, x5, x8
	and	w22, w22, 65535
	str	w2, [x5, 4]
.L1504:
	add	w1, w1, 1
	add	x3, x3, 2
	and	w1, w1, 65535
.L1512:
	cmp	w1, w7
	bne	.L1505
	add	x24, x19, :lo12:.LANCHOR0
	mov	w1, w22
	mov	x25, 0
	ldrb	w2, [x24, 184]
	ldr	x0, [x24, 3272]
	bl	FlashReadPages
	ubfiz	x0, x22, 5, 16
	mov	x22, x24
	str	x0, [x29, 120]
.L1506:
	ldr	x0, [x29, 120]
	cmp	x0, x25
	bne	.L1510
	add	w28, w28, 1
	b	.L1503
.L1510:
	ldr	x0, [x22, 3272]
	add	x1, x0, x25
	ldr	w0, [x0, x25]
	cmn	w0, #1
	beq	.L1507
	ldr	x24, [x1, 16]
	mov	w0, 61589
	ldrh	w1, [x24]
	cmp	w1, w0
	bne	.L1507
	ldr	w5, [x24, 8]
	cmn	w5, #1
	bne	.L1508
	str	w5, [x29, 104]
	mov	w2, 753
	mov	x1, x23
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	ldr	w5, [x29, 104]
.L1508:
	mov	w2, 0
	add	x1, x29, 128
	mov	w0, w5
	bl	log2phys
	ldr	x0, [x22, 3272]
	ldr	w1, [x29, 128]
	add	x0, x0, x25
	ldr	w2, [x0, 4]
	cmp	w2, w1
	bne	.L1507
	ldrh	w1, [x22, 3962]
	ldr	x2, [x22, 3440]
	add	w1, w1, 1
	strh	w1, [x22, 3962]
	ldr	w1, [x22, 3240]
	ldr	w0, [x0, 24]
	add	x1, x2, x1, lsl 5
	str	x1, [x29, 104]
	str	w0, [x1, 24]
	bl	Ftl_get_new_temp_ppa
	ldr	x1, [x29, 104]
	str	w0, [x1, 4]
	ldr	w0, [x22, 3240]
	ldr	x1, [x22, 3440]
	add	x0, x1, x0, lsl 5
	ldr	x1, [x22, 3272]
	add	x1, x1, x25
	ldr	x2, [x1, 8]
	str	x2, [x0, 8]
	ldr	x1, [x1, 16]
	str	x1, [x0, 16]
	ldr	w0, [x29, 128]
	mov	w1, 1
	str	w0, [x24, 12]
	ldrh	w0, [x22, 128]
	strh	w0, [x24, 2]
	ldr	w0, [x22, 2664]
	str	w0, [x24, 4]
	ldr	w0, [x22, 3240]
	add	w0, w0, 1
	str	w0, [x22, 3240]
	ldr	x0, [x22, 3272]
	add	x0, x0, x25
	bl	FtlGcBufAlloc
	ldrb	w1, [x22, 135]
	ldr	w0, [x22, 3240]
	cmp	w1, w0
	beq	.L1509
	ldrh	w0, [x22, 132]
	cbnz	w0, .L1507
.L1509:
	bl	Ftl_gc_temp_data_write_back
	cbz	w0, .L1507
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, -1
	strh	wzr, [x19, 178]
	strh	w0, [x19, 176]
	str	wzr, [x19, 3416]
	ldrh	w0, [x19, 3960]
	b	.L1447
.L1507:
	add	x25, x25, 32
	b	.L1506
.L1511:
	add	x22, x19, :lo12:.LANCHOR0
	ldrh	w0, [x22, 178]
	add	w20, w20, w0
	and	w20, w20, 65535
	strh	w20, [x22, 178]
	cmp	w26, w20
	bhi	.L1513
	ldr	w0, [x22, 3240]
	cbz	w0, .L1514
	bl	Ftl_gc_temp_data_write_back
	cbz	w0, .L1514
	str	wzr, [x22, 3416]
	b	.L1566
.L1514:
	add	x0, x19, :lo12:.LANCHOR0
	ldrh	w1, [x0, 3962]
	cbnz	w1, .L1515
	ldrh	w2, [x0, 176]
	ldr	x1, [x0, 72]
	ldrh	w1, [x1, x2, lsl 1]
	cbz	w1, .L1515
	mov	x20, x0
	mov	w22, 0
.L1516:
	ldr	w0, [x20, 2624]
	cmp	w22, w0
	bcs	.L1521
	mov	w2, 0
	add	x1, x29, 132
	mov	w0, w22
	bl	log2phys
	ldr	w0, [x29, 132]
	cmn	w0, #1
	beq	.L1517
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	ldrh	w1, [x20, 176]
	cmp	w1, w0, uxth
	bne	.L1517
.L1521:
	add	x0, x19, :lo12:.LANCHOR0
	ldr	w1, [x0, 2624]
	cmp	w22, w1
	bcc	.L1515
	ldrh	w2, [x0, 176]
	ldr	x1, [x0, 72]
	strh	wzr, [x1, x2, lsl 1]
	ldrh	w0, [x0, 176]
	bl	update_vpc_list
	bl	l2p_flush
	bl	FtlVpcTblFlush
.L1515:
	add	x0, x19, :lo12:.LANCHOR0
	mov	w1, -1
	strh	w1, [x0, 176]
.L1513:
	add	x1, x19, :lo12:.LANCHOR0
	ldrh	w0, [x1, 228]
	str	wzr, [x1, 3416]
	cmp	w0, 2
	bhi	.L1522
	ldrh	w20, [x1, 306]
	b	.L1523
.L1517:
	add	w22, w22, 1
	b	.L1516
.L1522:
	ldrh	w1, [x1, 3960]
	cmp	w1, 0
	csinc	w0, w1, w0, ne
	b	.L1447
.L1527:
	mov	w0, 0
	b	.L1447
.L1525:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 144
	ret
	.size	rk_ftl_garbage_collect, .-rk_ftl_garbage_collect
	.align	2
	.global	FtlRead
	.type	FtlRead, %function
FtlRead:
	stp	x29, x30, [sp, -176]!
	and	w0, w0, 255
	cmp	w0, 16
	add	x29, sp, 0
	stp	x25, x26, [sp, 64]
	mov	w26, w1
	stp	x21, x22, [sp, 32]
	adrp	x1, __stack_chk_guard
	stp	x19, x20, [sp, 16]
	mov	w25, w2
	stp	x23, x24, [sp, 48]
	add	x2, x1, :lo12:__stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	x22, x3
	str	x1, [x29, 112]
	ldr	x3, [x2]
	str	x3, [x29, 168]
	mov	x3,0
	bne	.L1568
	mov	x2, x22
	mov	w1, w25
	add	w0, w26, 256
	bl	FtlVendorPartRead
	mov	w19, w0
.L1567:
	ldr	x1, [x29, 112]
	mov	w0, w19
	add	x1, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 168]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L1589
	bl	__stack_chk_fail
.L1568:
	adrp	x1, .LANCHOR0
	add	x3, x1, :lo12:.LANCHOR0
	str	x1, [x29, 104]
	ldr	w0, [x3, 344]
	cmp	w26, w0
	bcs	.L1592
	cmp	w25, w0
	bhi	.L1592
	add	w1, w26, w25
	str	w1, [x29, 144]
	cmp	w0, w1
	bcc	.L1592
	adrp	x0, .LANCHOR2
	ldr	w19, [x0, #:lo12:.LANCHOR2]
	cmn	w19, #1
	beq	.L1567
	ldrh	w0, [x3, 262]
	mov	x27, x3
	mov	w23, 0
	mov	w28, 0
	mov	w19, 0
	stp	wzr, wzr, [x29, 148]
	udiv	w1, w26, w0
	str	w1, [x29, 156]
	add	w1, w26, w25
	sub	w24, w1, #1
	ldr	w20, [x29, 156]
	udiv	w24, w24, w0
	ldr	w0, [x29, 156]
	sub	w21, w24, w0
	ldr	w0, [x3, 2656]
	add	w21, w21, 1
	add	w0, w0, w25
	str	w0, [x3, 2656]
	ldr	w0, [x3, 2628]
	add	w0, w0, w21
	str	w0, [x3, 2628]
.L1570:
	cbnz	w21, .L1587
	ldr	x0, [x29, 104]
	add	x0, x0, :lo12:.LANCHOR0
	ldrh	w1, [x0, 3312]
	cbnz	w1, .L1588
	ldrh	w0, [x0, 228]
	cmp	w0, 31
	bhi	.L1567
.L1588:
	mov	w1, 1
	mov	w0, 0
	bl	rk_ftl_garbage_collect
	b	.L1567
.L1587:
	add	x1, x29, 164
	mov	w2, 0
	mov	w0, w20
	bl	log2phys
	ldr	w1, [x29, 164]
	cmn	w1, #1
	bne	.L1571
	mov	w6, 0
.L1572:
	ldrh	w0, [x27, 262]
	cmp	w6, w0
	bcc	.L1574
.L1575:
	add	w20, w20, 1
	subs	w21, w21, #1
	beq	.L1579
	ldrh	w0, [x27, 236]
	cmp	w28, w0, lsl 2
	bne	.L1570
.L1579:
	cbz	w28, .L1570
	ldr	x0, [x27, 3432]
	mov	w1, w28
	mov	w2, 0
	bl	FlashReadPages
	ldr	w0, [x29, 148]
	lsl	w0, w0, 9
	str	w0, [x29, 128]
	ldr	w0, [x29, 152]
	lsl	w0, w0, 9
	str	x0, [x29, 136]
	lsl	w0, w23, 9
	str	w0, [x29, 132]
	ubfiz	x0, x28, 5, 32
	mov	x28, 0
	str	x0, [x29, 120]
.L1586:
	ldr	x0, [x27, 3432]
	ldr	w2, [x29, 156]
	add	x0, x0, x28
	ldr	w1, [x0, 24]
	cmp	w2, w1
	bne	.L1581
	ldr	x1, [x0, 8]
	ldr	x0, [x27, 3480]
	cmp	x1, x0
	bne	.L1582
	ldr	x0, [x29, 136]
	ldr	w2, [x29, 132]
	add	x1, x1, x0
	mov	x0, x22
.L1602:
	bl	ftl_memcpy
.L1582:
	ldr	x0, [x27, 3432]
	add	x0, x0, x28
	ldr	x1, [x0, 16]
	ldr	w2, [x0, 24]
	ldr	w1, [x1, 8]
	cmp	w2, w1
	beq	.L1583
	ldr	w1, [x27, 2784]
	add	w1, w1, 1
	str	w1, [x27, 2784]
.L1583:
	ldr	w2, [x0]
	cmn	w2, #1
	bne	.L1584
	ldr	w1, [x27, 2784]
	mov	w19, w2
	add	w1, w1, 1
	str	w1, [x27, 2784]
.L1584:
	ldr	w1, [x0]
	cmp	w1, 256
	bne	.L1585
	ldr	w0, [x0, 4]
	lsr	w0, w0, 10
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
.L1585:
	ldr	x0, [x29, 120]
	add	x28, x28, 32
	cmp	x0, x28
	bne	.L1586
	mov	w28, 0
	b	.L1570
.L1574:
	madd	w0, w20, w0, w6
	cmp	w26, w0
	bhi	.L1573
	ldr	w1, [x29, 144]
	cmp	w1, w0
	bls	.L1573
	sub	w0, w0, w26
	str	w6, [x29, 136]
	lsl	w0, w0, 9
	mov	w2, 512
	mov	w1, 0
	add	x0, x22, x0
	bl	ftl_memset
	ldr	w6, [x29, 136]
.L1573:
	add	w6, w6, 1
	b	.L1572
.L1571:
	ldr	x0, [x27, 3432]
	ubfiz	x2, x28, 5, 32
	add	x0, x0, x2
	str	w1, [x0, 4]
	ldr	w0, [x29, 156]
	cmp	w20, w0
	ldrh	w0, [x27, 262]
	bne	.L1576
	ldr	x1, [x27, 3432]
	ldr	x6, [x27, 3480]
	add	x1, x1, x2
	str	x6, [x1, 8]
	udiv	w6, w26, w0
	msub	w3, w6, w0, w26
	str	w3, [x29, 152]
	sub	w23, w0, w3
	cmp	w25, w23
	csel	w23, w25, w23, ls
	cmp	w23, w0
	bne	.L1577
	str	x22, [x1, 8]
.L1577:
	ldr	x0, [x27, 3432]
	ldr	x1, [x27, 3528]
	add	x2, x0, x2
	ldrh	w0, [x27, 316]
	str	w20, [x2, 24]
	mul	w0, w0, w28
	add	w28, w28, 1
	and	x0, x0, 4294967292
	add	x0, x1, x0
	str	x0, [x2, 16]
	b	.L1575
.L1576:
	cmp	w20, w24
	bne	.L1578
	ldr	x6, [x27, 3432]
	ldr	x1, [x27, 3488]
	add	x6, x6, x2
	ldr	w3, [x29, 144]
	str	x1, [x6, 8]
	mul	w1, w20, w0
	sub	w3, w3, w1
	str	w3, [x29, 148]
	cmp	w0, w3
	bne	.L1577
	sub	w1, w1, w26
	lsl	w1, w1, 9
	add	x1, x22, x1
	str	x1, [x6, 8]
	b	.L1577
.L1578:
	ldr	x1, [x27, 3432]
	mul	w0, w0, w20
	add	x1, x1, x2
	sub	w0, w0, w26
	lsl	w0, w0, 9
	add	x0, x22, x0
	str	x0, [x1, 8]
	b	.L1577
.L1581:
	cmp	w24, w1
	bne	.L1582
	ldr	x1, [x0, 8]
	ldr	x0, [x27, 3488]
	cmp	x1, x0
	bne	.L1582
	ldrh	w0, [x27, 262]
	ldr	w2, [x29, 128]
	mul	w0, w0, w24
	sub	w0, w0, w26
	lsl	w0, w0, 9
	add	x0, x22, x0
	b	.L1602
.L1592:
	mov	w19, -1
	b	.L1567
.L1589:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 176
	ret
	.size	FtlRead, .-FtlRead
	.align	2
	.global	sftl_read
	.type	sftl_read, %function
sftl_read:
	stp	x29, x30, [sp, -16]!
	mov	x3, x2
	mov	w2, w1
	mov	w1, w0
	add	x29, sp, 0
	mov	w0, 0
	bl	FtlRead
	ldp	x29, x30, [sp], 16
	ret
	.size	sftl_read, .-sftl_read
	.align	2
	.global	FtlWrite
	.type	FtlWrite, %function
FtlWrite:
	stp	x29, x30, [sp, -240]!
	and	w0, w0, 255
	cmp	w0, 16
	add	x29, sp, 0
	stp	x23, x24, [sp, 48]
	mov	w23, w1
	stp	x25, x26, [sp, 64]
	adrp	x1, __stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	w26, w2
	stp	x19, x20, [sp, 16]
	add	x2, x1, :lo12:__stack_chk_guard
	stp	x21, x22, [sp, 32]
	mov	x27, x3
	str	x1, [x29, 112]
	ldr	x3, [x2]
	str	x3, [x29, 232]
	mov	x3,0
	bne	.L1606
	mov	x2, x27
	mov	w1, w26
	add	w0, w23, 256
	bl	FtlVendorPartWrite
.L1605:
	ldr	x1, [x29, 112]
	add	x1, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 232]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L1642
	bl	__stack_chk_fail
.L1606:
	adrp	x19, .LANCHOR0
	add	x1, x19, :lo12:.LANCHOR0
	ldr	w0, [x1, 344]
	cmp	w23, w0
	bcs	.L1645
	cmp	w26, w0
	bhi	.L1645
	add	w2, w23, w26
	str	w2, [x29, 124]
	cmp	w0, w2
	bcc	.L1645
	adrp	x2, .LANCHOR2
	str	x2, [x29, 152]
	add	x3, x2, :lo12:.LANCHOR2
	ldr	w0, [x2, #:lo12:.LANCHOR2]
	cmn	w0, #1
	beq	.L1605
	mov	w0, 2048
	add	w2, w23, w26
	str	w0, [x1, 3968]
	sub	w2, w2, #1
	ldrh	w0, [x1, 262]
	add	x20, x1, 24
	udiv	w25, w23, w0
	udiv	w0, w2, w0
	str	w0, [x29, 160]
	sub	w0, w0, w25
	add	w24, w0, 1
	str	w0, [x29, 148]
	ldr	w0, [x1, 2652]
	add	w0, w0, w26
	str	w0, [x1, 2652]
	ldr	w0, [x1, 2636]
	add	w0, w0, w24
	str	w0, [x1, 2636]
	ldr	w0, [x3, 8]
	cbz	w0, .L1608
	ldrh	w0, [x20, 4]
	add	x1, x1, 80
	cmp	w0, 0
	csel	x20, x1, x20, eq
.L1608:
	adrp	x0, .LANCHOR1
	mov	w21, w25
	add	x0, x0, :lo12:.LANCHOR1
	add	x0, x0, 696
	str	x0, [x29, 176]
.L1609:
	cbnz	w24, .L1637
	ldr	w1, [x29, 148]
	add	x19, x19, :lo12:.LANCHOR0
	mov	w0, 0
	bl	rk_ftl_garbage_collect
	ldrh	w0, [x19, 228]
	cmp	w0, 5
	bls	.L1638
.L1640:
	mov	w0, 0
	b	.L1605
.L1637:
	add	x0, x19, :lo12:.LANCHOR0
	ldrb	w1, [x20, 6]
	ldrh	w0, [x0, 236]
	cmp	w1, w0
	bcc	.L1610
	ldr	x1, [x29, 176]
	adrp	x0, .LC8
	mov	w2, 1041
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1610:
	ldrh	w0, [x20, 4]
	cbnz	w0, .L1611
	add	x0, x19, :lo12:.LANCHOR0
	add	x1, x0, 24
	cmp	x20, x1
	bne	.L1612
	add	x0, x0, 80
	ldrh	w1, [x0, 4]
	cbnz	w1, .L1613
	bl	allocate_new_data_superblock
	ldr	x0, [x29, 152]
	add	x0, x0, :lo12:.LANCHOR2
	str	wzr, [x0, 8]
.L1613:
	add	x20, x19, :lo12:.LANCHOR0
	add	x22, x20, 24
	add	x20, x20, 80
	mov	x0, x22
	bl	allocate_new_data_superblock
	ldr	x0, [x29, 152]
	add	x0, x0, :lo12:.LANCHOR2
	ldr	w0, [x0, 8]
	cmp	w0, 0
	csel	x20, x20, x22, ne
.L1614:
	ldrh	w0, [x20, 4]
	cbnz	w0, .L1611
	mov	x0, x20
	bl	allocate_new_data_superblock
.L1611:
	ldrh	w0, [x20, 4]
	ldrb	w1, [x20, 7]
	cmp	w0, w24
	csel	w0, w0, w24, ls
	lsl	w1, w1, 2
	cmp	w0, w1
	csel	w0, w0, w1, ls
	str	w0, [x29, 144]
	add	x0, x19, :lo12:.LANCHOR0
	ldrb	w1, [x20, 6]
	ldrh	w0, [x0, 236]
	cmp	w1, w0
	bcc	.L1615
	ldr	x1, [x29, 176]
	adrp	x0, .LC8
	mov	w2, 1074
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1615:
	str	xzr, [x29, 184]
.L1616:
	ldr	w1, [x29, 144]
	ldr	w22, [x29, 184]
	cmp	w22, w1
	bcc	.L1635
	mov	x22, x1
.L1617:
	add	x0, x19, :lo12:.LANCHOR0
	mov	x3, x20
	mov	w2, 0
	mov	w1, w22
	ldr	x0, [x0, 3448]
	bl	FtlProgPages
	cmp	w24, w22
	bcs	.L1636
	ldr	x1, [x29, 176]
	adrp	x0, .LC8
	mov	w2, 1152
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1636:
	sub	w24, w24, w22
	b	.L1609
.L1612:
	ldr	x0, [x29, 152]
	add	x0, x0, :lo12:.LANCHOR2
	str	wzr, [x0, 8]
	ldrh	w0, [x1, 4]
	cbnz	w0, .L1650
	mov	x0, x20
	bl	allocate_new_data_superblock
	b	.L1614
.L1650:
	mov	x20, x1
	b	.L1611
.L1635:
	ldrh	w0, [x20, 4]
	cbz	w0, .L1617
	add	x1, x29, 196
	mov	w2, 0
	mov	w0, w21
	bl	log2phys
	mov	x0, x20
	bl	get_new_active_ppa
	add	x3, x19, :lo12:.LANCHOR0
	ldr	x1, [x29, 184]
	str	x3, [x29, 168]
	lsl	x28, x1, 5
	ldr	x1, [x3, 3448]
	ldrh	w2, [x3, 316]
	add	x1, x1, x28
	str	w0, [x1, 4]
	mul	w22, w22, w2
	ldr	x0, [x3, 3448]
	and	x1, x22, 4294967292
	str	x1, [x29, 136]
	add	x0, x0, x28
	ldr	x1, [x3, 3528]
	ldr	x3, [x29, 136]
	str	x1, [x29, 128]
	add	x22, x1, x3
	str	w21, [x0, 24]
	str	x22, [x0, 16]
	mov	w1, 0
	mov	x0, x22
	bl	ftl_memset
	ldr	w0, [x29, 160]
	cmp	w21, w25
	ldr	x3, [x29, 168]
	ccmp	w21, w0, 4, ne
	bne	.L1618
	cmp	w21, w25
	bne	.L1619
	ldrh	w2, [x3, 262]
	udiv	w0, w23, w2
	msub	w0, w0, w2, w23
	str	w0, [x29, 164]
	sub	w2, w2, w0
	cmp	w2, w26
	csel	w0, w2, w26, ls
	str	w0, [x29, 168]
.L1620:
	add	x0, x19, :lo12:.LANCHOR0
	ldr	w2, [x29, 168]
	ldrh	w1, [x0, 262]
	cmp	w2, w1
	bne	.L1621
	ldr	x0, [x0, 3448]
	cmp	w21, w25
	add	x4, x0, x28
	bne	.L1622
	str	x27, [x4, 8]
.L1623:
	add	x0, x19, :lo12:.LANCHOR0
	ldrb	w1, [x20, 6]
	ldrh	w0, [x0, 236]
	cmp	w1, w0
	bcc	.L1632
	ldr	x1, [x29, 176]
	adrp	x0, .LC8
	mov	w2, 1143
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1632:
	ldp	x1, x2, [x29, 128]
	mov	w0, -3947
	strh	w0, [x1, x2]
	add	x1, x19, :lo12:.LANCHOR0
	ldr	w0, [x1, 2664]
	stp	w0, w21, [x22, 4]
	add	w21, w21, 1
	add	w0, w0, 1
	cmn	w0, #1
	csel	w0, w0, wzr, ne
	str	w0, [x1, 2664]
	ldr	w0, [x29, 196]
	str	w0, [x22, 12]
	ldrh	w0, [x20]
	strh	w0, [x22, 2]
	ldr	x0, [x29, 184]
	add	x0, x0, 1
	str	x0, [x29, 184]
	b	.L1616
.L1619:
	ldr	w0, [x29, 124]
	ldrh	w2, [x3, 262]
	msub	w2, w21, w2, w0
	and	w0, w2, 65535
	stp	wzr, w0, [x29, 164]
	b	.L1620
.L1622:
	ldr	w0, [x29, 168]
.L1657:
	mul	w0, w0, w21
	sub	w0, w0, w23
	lsl	w0, w0, 9
	add	x0, x27, x0
	str	x0, [x4, 8]
	b	.L1623
.L1621:
	ldr	x1, [x0, 3448]
	cmp	w21, w25
	add	x1, x1, x28
	bne	.L1624
	ldr	x0, [x0, 3480]
.L1656:
	str	x0, [x1, 8]
	ldr	w0, [x29, 196]
	cmn	w0, #1
	beq	.L1626
	add	x5, x19, :lo12:.LANCHOR0
	str	w0, [x29, 204]
	str	w21, [x29, 224]
	mov	w2, 0
	str	x5, [x29, 104]
	ldr	x0, [x5, 3448]
	add	x0, x0, x28
	ldp	x1, x0, [x0, 8]
	stp	x1, x0, [x29, 208]
	mov	w1, 1
	add	x0, x29, 200
	bl	FlashReadPages
	ldr	w3, [x29, 200]
	ldr	x5, [x29, 104]
	cmn	w3, #1
	bne	.L1627
	ldr	w0, [x5, 2784]
	mov	w2, w21
	add	w0, w0, 1
	str	w0, [x5, 2784]
	adrp	x0, .LC125
	add	x0, x0, :lo12:.LC125
	ldr	w1, [x22, 8]
	bl	sftl_printk
.L1630:
	ldr	w0, [x29, 168]
	cmp	w21, w25
	lsl	w2, w0, 9
	add	x0, x19, :lo12:.LANCHOR0
	bne	.L1631
	ldr	x0, [x0, 3448]
	mov	x1, x27
	add	x4, x0, x28
	ldr	w0, [x29, 164]
	ldr	x3, [x4, 8]
	lsl	w0, w0, 9
	add	x0, x3, x0
.L1658:
	bl	ftl_memcpy
	b	.L1623
.L1624:
	ldr	x0, [x0, 3488]
	b	.L1656
.L1627:
	ldr	w0, [x22, 8]
	cmp	w21, w0
	beq	.L1629
	ldr	w0, [x5, 2784]
	mov	w2, w21
	add	w0, w0, 1
	str	w0, [x5, 2784]
	adrp	x0, .LC126
	add	x0, x0, :lo12:.LC126
	ldr	w1, [x22, 8]
	bl	sftl_printk
.L1629:
	ldr	w0, [x22, 8]
	cmp	w21, w0
	beq	.L1630
	ldr	x1, [x29, 176]
	mov	w2, 1128
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L1630
.L1626:
	add	x1, x19, :lo12:.LANCHOR0
	ldr	x0, [x1, 3448]
	ldrh	w2, [x1, 314]
	mov	w1, 0
	add	x0, x0, x28
	ldr	x0, [x0, 8]
	bl	ftl_memset
	b	.L1630
.L1631:
	ldrh	w1, [x0, 262]
	ldr	x0, [x0, 3448]
	add	x4, x0, x28
	mul	w1, w1, w21
	sub	w1, w1, w23
	ldr	x0, [x4, 8]
	lsl	w1, w1, 9
	add	x1, x27, x1
	b	.L1658
.L1618:
	ldr	x0, [x3, 3448]
	add	x4, x0, x28
	ldrh	w0, [x3, 262]
	b	.L1657
.L1638:
	mov	w20, 256
	mov	w21, 65535
	mov	w22, 128
.L1641:
	ldrh	w0, [x19, 176]
	cmp	w0, w21
	bne	.L1639
	ldrh	w0, [x19, 226]
	cmp	w0, w21
	bne	.L1639
	mov	w0, 0
	bl	List_get_gc_head_node
	bl	FtlGcRefreshBlock
.L1639:
	mov	w1, 1
	strh	w22, [x19, 3226]
	mov	w0, w1
	strh	w22, [x19, 3224]
	bl	rk_ftl_garbage_collect
	mov	w1, 1
	mov	w0, 0
	bl	rk_ftl_garbage_collect
	ldrh	w0, [x19, 228]
	cmp	w0, 2
	bhi	.L1640
	subs	w20, w20, #1
	bne	.L1641
	b	.L1640
.L1645:
	mov	w0, -1
	b	.L1605
.L1642:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 240
	ret
	.size	FtlWrite, .-FtlWrite
	.align	2
	.global	sftl_gc
	.type	sftl_gc, %function
sftl_gc:
	stp	x29, x30, [sp, -16]!
	mov	w1, 1
	mov	w0, w1
	add	x29, sp, 0
	bl	rk_ftl_garbage_collect
	ldp	x29, x30, [sp], 16
	ret
	.size	sftl_gc, .-sftl_gc
	.align	2
	.global	FtlLoadSysInfo
	.type	FtlLoadSysInfo, %function
FtlLoadSysInfo:
	stp	x29, x30, [sp, -80]!
	mov	w1, 0
	add	x29, sp, 0
	stp	x21, x22, [sp, 32]
	adrp	x22, .LANCHOR0
	add	x21, x22, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	add	x20, x21, 3720
	stp	x23, x24, [sp, 48]
	str	x25, [sp, 64]
	ldr	x0, [x21, 3456]
	str	x0, [x20, 8]
	ldr	x0, [x21, 3520]
	str	x0, [x20, 16]
	ldr	x0, [x21, 72]
	ldrh	w2, [x21, 244]
	lsl	w2, w2, 1
	bl	ftl_memset
	ldrh	w0, [x21, 2696]
	mov	w1, 65535
	cmp	w0, w1
	bne	.L1662
.L1673:
	mov	w0, -1
.L1661:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldr	x25, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
.L1662:
	add	x23, x21, 2696
	mov	w1, 1
	bl	FtlGetLastWrittenPage
	sxth	w19, w0
	adrp	x1, .LANCHOR1
	add	w0, w0, 1
	add	x1, x1, :lo12:.LANCHOR1
	strh	w0, [x23, 2]
	ldrsh	w23, [x21, 2696]
	add	x24, x1, 712
.L1664:
	tbz	w19, #31, .L1670
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	mov	w2, 1474
	add	x1, x1, 712
	adrp	x0, .LC8
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
	b	.L1669
.L1670:
	orr	w0, w19, w23, lsl 10
	str	w0, [x20, 4]
	ldr	x0, [x21, 3456]
	mov	w2, 1
	str	x0, [x20, 8]
	mov	w1, w2
	mov	x0, x20
	bl	FlashReadPages
	ldr	x0, [x20, 16]
	ldr	w7, [x0, 12]
	cbz	w7, .L1665
	ldr	w0, [x20]
	cmn	w0, #1
	beq	.L1665
	ldrh	w1, [x21, 314]
	ldr	x0, [x20, 8]
	bl	js_hash
	cmp	w7, w0
	beq	.L1665
	ldrh	w4, [x21, 2700]
	mov	w6, w0
	mov	w5, w7
	adrp	x0, .LC127
	mov	w3, w19
	mov	w2, w23
	mov	x1, x24
	add	x0, x0, :lo12:.LC127
	bl	sftl_printk
	cbnz	w19, .L1666
	ldrh	w0, [x21, 2700]
	cmp	w23, w0
	beq	.L1666
	ldrh	w19, [x21, 308]
	sxth	w23, w0
.L1668:
	sub	w19, w19, #1
	sxth	w19, w19
	b	.L1664
.L1666:
	mov	w0, -1
	str	w0, [x20]
.L1665:
	ldr	w0, [x20]
	cmn	w0, #1
	beq	.L1668
	ldr	x0, [x21, 3456]
	ldr	w1, [x0]
	mov	w0, 19539
	movk	w0, 0x4654, lsl 16
	cmp	w1, w0
	bne	.L1668
	ldr	x0, [x21, 3520]
	ldrh	w1, [x0]
	mov	w0, 61604
	cmp	w1, w0
	bne	.L1668
.L1669:
	add	x1, x22, :lo12:.LANCHOR0
	ldrh	w0, [x1, 244]
	ldrh	w1, [x1, 314]
	add	x0, x0, 24
	cmp	x1, x0, lsl 1
	bcs	.L1672
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1476
	add	x1, x1, 712
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1672:
	add	x19, x22, :lo12:.LANCHOR0
	mov	w2, 48
	add	x20, x19, 2512
	mov	x0, x20
	ldr	x1, [x19, 3728]
	bl	ftl_memcpy
	ldr	x0, [x19, 72]
	ldrh	w2, [x19, 244]
	ldr	x1, [x19, 3728]
	lsl	w2, w2, 1
	add	x1, x1, 48
	bl	ftl_memcpy
	ldr	w1, [x19, 2512]
	mov	w0, 19539
	movk	w0, 0x4654, lsl 16
	cmp	w1, w0
	bne	.L1673
	ldrh	w1, [x19, 2520]
	ldrb	w2, [x19, 2522]
	ldrh	w0, [x19, 258]
	strh	w1, [x19, 2702]
	cmp	w2, w0
	bne	.L1673
	ldrh	w0, [x19, 306]
	ldrh	w2, [x19, 262]
	ldrh	w3, [x19, 236]
	str	w1, [x19, 3972]
	mul	w0, w0, w1
	str	w0, [x19, 2624]
	mul	w0, w2, w0
	ldr	w2, [x19, 248]
	str	w0, [x19, 344]
	ldrh	w0, [x19, 366]
	cmp	w1, w2
	sub	w0, w2, w0
	sub	w0, w0, w1
	udiv	w0, w0, w3
	strh	w0, [x19, 2692]
	bls	.L1674
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	adrp	x0, .LC8
	mov	w2, 1498
	add	x1, x1, 712
	add	x0, x0, :lo12:.LC8
	bl	sftl_printk
.L1674:
	add	x0, x22, :lo12:.LANCHOR0
	mov	w3, -1
	ldrh	w2, [x0, 2528]
	ldrh	w4, [x0, 2526]
	strh	w4, [x0, 24]
	lsr	w5, w2, 6
	and	w2, w2, 63
	strb	w2, [x0, 30]
	ldrb	w2, [x0, 2523]
	strb	w2, [x0, 32]
	ldrh	w2, [x0, 2530]
	strh	w2, [x0, 80]
	ldrh	w2, [x0, 2532]
	strh	w5, [x0, 26]
	strh	w3, [x0, 176]
	lsr	w5, w2, 6
	and	w2, w2, 63
	strb	w2, [x0, 86]
	ldrb	w2, [x0, 2524]
	strb	w2, [x0, 88]
	ldrh	w2, [x0, 2534]
	strh	w2, [x0, 128]
	ldrh	w2, [x0, 2536]
	strh	w5, [x0, 82]
	strh	wzr, [x0, 178]
	lsr	w5, w2, 6
	and	w2, w2, 63
	strb	w2, [x0, 134]
	ldrb	w2, [x0, 2525]
	strb	w2, [x0, 136]
	ldr	w2, [x0, 2544]
	str	w2, [x0, 2668]
	strb	wzr, [x0, 182]
	strb	wzr, [x0, 184]
	strh	w5, [x0, 130]
	str	wzr, [x0, 2628]
	str	wzr, [x0, 2636]
	str	wzr, [x0, 2644]
	str	wzr, [x0, 2648]
	str	wzr, [x0, 2672]
	str	wzr, [x0, 2684]
	ldr	w1, [x0, 2552]
	ldr	w2, [x0, 2660]
	str	wzr, [x0, 2640]
	cmp	w1, w2
	bls	.L1675
	str	w1, [x0, 2660]
.L1675:
	add	x0, x22, :lo12:.LANCHOR0
	ldr	w1, [x0, 2548]
	ldr	w2, [x0, 2664]
	cmp	w1, w2
	bls	.L1676
	str	w1, [x0, 2664]
.L1676:
	mov	w0, 65535
	cmp	w4, w0
	beq	.L1677
	add	x0, x22, :lo12:.LANCHOR0
	add	x0, x0, 24
	bl	make_superblock
.L1677:
	add	x1, x22, :lo12:.LANCHOR0
	add	x0, x1, 80
	ldrh	w2, [x1, 80]
	mov	w1, 65535
	cmp	w2, w1
	beq	.L1678
	bl	make_superblock
.L1678:
	add	x1, x22, :lo12:.LANCHOR0
	add	x0, x1, 128
	ldrh	w2, [x1, 128]
	mov	w1, 65535
	cmp	w2, w1
	beq	.L1679
	bl	make_superblock
.L1679:
	add	x22, x22, :lo12:.LANCHOR0
	mov	w1, 65535
	add	x0, x22, 176
	ldrh	w2, [x22, 176]
	cmp	w2, w1
	beq	.L1680
	bl	make_superblock
.L1680:
	mov	w0, 0
	b	.L1661
	.size	FtlLoadSysInfo, .-FtlLoadSysInfo
	.align	2
	.global	FtlMapTblRecovery
	.type	FtlMapTblRecovery, %function
FtlMapTblRecovery:
	stp	x29, x30, [sp, -144]!
	mov	w1, 0
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x19, x0
	stp	x21, x22, [sp, 32]
	adrp	x21, .LANCHOR0
	stp	x23, x24, [sp, 48]
	mov	w20, 0
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	ldrh	w25, [x0, 6]
	ldr	x23, [x0, 40]
	ldr	x24, [x0, 16]
	ldr	x0, [x0, 24]
	lsl	w2, w25, 2
	str	x0, [x29, 128]
	ldrh	w0, [x19, 8]
	str	w0, [x29, 136]
	mov	x0, x23
	bl	ftl_memset
	stp	wzr, wzr, [x19, 48]
	add	x0, x21, :lo12:.LANCHOR0
	ldr	x2, [x0, 3456]
	ldr	x22, [x0, 3520]
	str	x2, [x0, 3728]
	str	x22, [x0, 3736]
	mov	w0, -1
	strh	w0, [x19]
	strh	w0, [x19, 2]
	mov	w0, 1
	str	w0, [x19, 56]
	ldr	w0, [x29, 136]
	sub	w0, w0, #1
	str	w0, [x29, 116]
	adrp	x0, .LANCHOR1
	add	x0, x0, :lo12:.LANCHOR1
	add	x0, x0, 728
	str	x0, [x29, 104]
.L1693:
	ldr	w0, [x29, 136]
	cmp	w20, w0
	bge	.L1712
	ldr	w0, [x29, 116]
	sxtw	x28, w20
	cmp	w20, w0
	bne	.L1694
	lsl	x0, x28, 1
	mov	w26, 0
	add	x1, x24, x0
	str	x1, [x29, 136]
	mov	w1, 1
	ldrh	w0, [x24, x0]
	adrp	x24, .LANCHOR1
	add	x24, x24, :lo12:.LANCHOR1
	add	x24, x24, 728
	bl	FtlGetLastWrittenPage
	sxth	w1, w0
	add	w0, w0, 1
	strh	w0, [x19, 2]
	ldr	x0, [x29, 128]
	strh	w20, [x19]
	str	w1, [x29, 120]
	ldr	w0, [x0, x28, lsl 2]
	add	x28, x21, :lo12:.LANCHOR0
	add	x27, x28, 3720
	str	w0, [x19, 48]
.L1695:
	ldr	w0, [x29, 120]
	add	w0, w0, 1
	cmp	w26, w0
	blt	.L1698
.L1712:
	add	x21, x21, :lo12:.LANCHOR0
	mov	x0, x19
	bl	ftl_free_no_use_map_blk
	ldrh	w1, [x19, 2]
	ldrh	w0, [x21, 308]
	cmp	w1, w0
	bne	.L1700
	mov	x0, x19
	bl	ftl_map_blk_alloc_new_blk
.L1700:
	mov	x0, x19
	bl	ftl_map_blk_gc
	mov	x0, x19
	bl	ftl_map_blk_gc
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 144
	ret
.L1698:
	ldr	x0, [x29, 136]
	mov	w2, 1
	mov	w1, w2
	ldrh	w0, [x0]
	orr	w0, w26, w0, lsl 10
	str	w0, [x27, 4]
	mov	x0, x27
	bl	FlashReadPages
	ldr	x0, [x27, 16]
	ldr	w8, [x0, 12]
	cbz	w8, .L1696
	ldr	w0, [x27]
	cmn	w0, #1
	beq	.L1696
	ldrh	w1, [x28, 314]
	ldr	x0, [x27, 8]
	bl	js_hash
	cmp	w8, w0
	beq	.L1696
	mov	w5, w0
	mov	w4, w8
	mov	w3, w26
	mov	w2, w20
	mov	x1, x24
	adrp	x0, .LC128
	add	x0, x0, :lo12:.LC128
	bl	sftl_printk
	mov	w0, -1
	str	w0, [x27]
.L1696:
	ldr	w0, [x27]
	cmn	w0, #1
	beq	.L1697
	ldrh	w0, [x22, 8]
	cmp	w25, w0
	bls	.L1697
	ldrh	w2, [x22]
	ldrh	w1, [x19, 4]
	cmp	w2, w1
	bne	.L1697
	ubfiz	x0, x0, 2, 16
	ldr	w1, [x27, 4]
	str	w1, [x23, x0]
.L1697:
	add	w7, w26, 1
	sxth	w26, w7
	b	.L1695
.L1694:
	add	x26, x21, :lo12:.LANCHOR0
	lsl	x2, x28, 1
	add	x0, x26, 3720
	ldr	x1, [x26, 3456]
	str	x1, [x0, 8]
	add	x1, x24, x2
	str	x1, [x29, 120]
	ldrh	w1, [x24, x2]
	ldrh	w2, [x26, 308]
	sub	w2, w2, #1
	orr	w1, w2, w1, lsl 10
	mov	w2, 1
	str	w1, [x0, 4]
	mov	w1, w2
	bl	FlashReadPages
	ldr	w0, [x26, 3720]
	cmn	w0, #1
	beq	.L1714
	ldrh	w1, [x22]
	ldrh	w0, [x19, 4]
	cmp	w1, w0
	bne	.L1714
	ldrh	w1, [x22, 8]
	mov	w0, 64245
	cmp	w1, w0
	beq	.L1702
.L1714:
	add	x26, x21, :lo12:.LANCHOR0
	mov	w27, 0
	add	x28, x26, 3720
.L1703:
	ldrh	w0, [x26, 308]
	cmp	w27, w0
	bge	.L1710
	ldr	x0, [x29, 120]
	mov	w2, 1
	mov	w1, w2
	ldrh	w0, [x0]
	orr	w0, w27, w0, lsl 10
	str	w0, [x28, 4]
	mov	x0, x28
	bl	FlashReadPages
	ldr	x0, [x28, 16]
	ldr	w7, [x0, 12]
	cbz	w7, .L1707
	ldr	w0, [x28]
	cmn	w0, #1
	beq	.L1707
	ldrh	w1, [x26, 314]
	ldr	x0, [x28, 8]
	bl	js_hash
	cmp	w7, w0
	beq	.L1707
	ldr	x1, [x29, 104]
	mov	w5, w0
	mov	w4, w7
	mov	w3, w27
	mov	w2, w20
	adrp	x0, .LC129
	add	x0, x0, :lo12:.LC129
	bl	sftl_printk
	mov	w0, -1
	str	w0, [x28]
.L1707:
	ldr	w0, [x28]
	cmn	w0, #1
	beq	.L1708
	ldrh	w0, [x22, 8]
	cmp	w25, w0
	bls	.L1708
	ldrh	w2, [x22]
	ldrh	w1, [x19, 4]
	cmp	w2, w1
	bne	.L1708
	ubfiz	x0, x0, 2, 16
	ldr	w1, [x28, 4]
	str	w1, [x23, x0]
.L1708:
	add	w6, w27, 1
	sxth	w27, w6
	b	.L1703
.L1702:
	mov	w0, 0
.L1704:
	ldrh	w1, [x26, 308]
	sub	w1, w1, #1
	cmp	w0, w1
	blt	.L1706
.L1710:
	add	w20, w20, 1
	sxth	w20, w20
	b	.L1693
.L1706:
	ldr	x2, [x26, 3456]
	sbfiz	x3, x0, 3, 32
	ldrh	w1, [x2, x3]
	cmp	w25, w1
	bls	.L1705
	add	x2, x2, x3
	ubfiz	x1, x1, 2, 16
	ldr	w2, [x2, 4]
	str	w2, [x23, x1]
.L1705:
	add	w0, w0, 1
	sxth	w0, w0
	b	.L1704
	.size	FtlMapTblRecovery, .-FtlMapTblRecovery
	.align	2
	.global	FtlLoadVonderInfo
	.type	FtlLoadVonderInfo, %function
FtlLoadVonderInfo:
	adrp	x1, .LANCHOR0
	add	x1, x1, :lo12:.LANCHOR0
	stp	x29, x30, [sp, -16]!
	add	x0, x1, 3888
	add	x29, sp, 0
	ldrh	w2, [x1, 324]
	strh	w2, [x0, 10]
	mov	w2, -3962
	strh	w2, [x0, 4]
	ldrh	w2, [x1, 348]
	strh	w2, [x0, 8]
	ldrh	w2, [x1, 326]
	strh	w2, [x0, 6]
	ldr	x2, [x1, 352]
	str	x2, [x0, 16]
	ldr	x2, [x1, 3592]
	str	x2, [x0, 24]
	ldr	x2, [x1, 3584]
	ldr	x1, [x1, 3600]
	stp	x2, x1, [x0, 32]
	bl	FtlMapTblRecovery
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlLoadVonderInfo, .-FtlLoadVonderInfo
	.align	2
	.global	FtlLoadMapInfo
	.type	FtlLoadMapInfo, %function
FtlLoadMapInfo:
	stp	x29, x30, [sp, -16]!
	add	x29, sp, 0
	bl	FtlL2PDataInit
	adrp	x0, .LANCHOR0
	add	x0, x0, :lo12:.LANCHOR0
	add	x0, x0, 3640
	bl	FtlMapTblRecovery
	mov	w0, 0
	ldp	x29, x30, [sp], 16
	ret
	.size	FtlLoadMapInfo, .-FtlLoadMapInfo
	.align	2
	.global	FtlSysBlkInit
	.type	FtlSysBlkInit, %function
FtlSysBlkInit:
	stp	x29, x30, [sp, -64]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x21, x20, :lo12:.LANCHOR0
	str	x23, [sp, 48]
	ldrh	w0, [x21, 240]
	strh	wzr, [x21, 3752]
	bl	FtlFreeSysBlkQueueInit
	bl	FtlScanSysBlk
	ldrh	w1, [x21, 2696]
	mov	w0, 65535
	cmp	w1, w0
	bne	.L1739
.L1741:
	mov	w23, -1
.L1738:
	mov	w0, w23
	ldr	x23, [sp, 48]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 64
	ret
.L1739:
	bl	FtlLoadSysInfo
	mov	w23, w0
	cbnz	w0, .L1741
	bl	FtlLoadMapInfo
	bl	FtlLoadVonderInfo
	bl	Ftl_load_ext_data
	bl	FtlLoadEctTbl
	bl	FtlFreeSysBLkSort
	bl	SupperBlkListInit
	bl	FtlPowerLostRecovery
	mov	w0, 1
	bl	FtlUpdateVaildLpn
	ldr	x1, [x21, 2608]
	mov	w0, 0
	ldrh	w3, [x21, 342]
	add	x1, x1, 4
.L1742:
	cmp	w0, w3
	bge	.L1747
	ldr	w2, [x1], 16
	tbz	w2, #31, .L1743
.L1747:
	add	x2, x20, :lo12:.LANCHOR0
	cmp	w0, w3
	ldrh	w1, [x2, 2540]
	add	w1, w1, 1
	strh	w1, [x2, 2540]
	bge	.L1753
.L1744:
	add	x19, x20, :lo12:.LANCHOR0
	add	x22, x19, 24
	add	x21, x19, 80
	mov	x0, x22
	bl	FtlSuperblockPowerLostFix
	mov	x0, x21
	bl	FtlSuperblockPowerLostFix
	ldrh	w0, [x19, 24]
	ldr	x2, [x19, 72]
	ldrh	w3, [x19, 28]
	lsl	x0, x0, 1
	ldrh	w1, [x2, x0]
	sub	w1, w1, w3
	strh	w1, [x2, x0]
	strb	wzr, [x19, 30]
	ldrh	w0, [x19, 306]
	strh	w0, [x19, 26]
	ldrh	w0, [x19, 80]
	ldr	x2, [x19, 72]
	strh	wzr, [x19, 28]
	lsl	x0, x0, 1
	ldrh	w3, [x19, 84]
	ldrh	w1, [x2, x0]
	sub	w1, w1, w3
	strh	w1, [x2, x0]
	strb	wzr, [x19, 86]
	ldrh	w0, [x19, 306]
	strh	w0, [x19, 82]
	ldrh	w0, [x19, 2542]
	strh	wzr, [x19, 84]
	add	w0, w0, 1
	strh	w0, [x19, 2542]
	bl	l2p_flush
	bl	FtlVpcTblFlush
	bl	FtlVpcTblFlush
	b	.L1748
.L1743:
	add	w0, w0, 1
	b	.L1742
.L1753:
	ldrh	w0, [x2, 3752]
	cbnz	w0, .L1744
.L1748:
	add	x19, x20, :lo12:.LANCHOR0
	mov	w1, 65535
	add	x21, x19, 24
	ldrh	w0, [x19, 24]
	cmp	w0, w1
	beq	.L1749
	ldrh	w1, [x19, 28]
	cbnz	w1, .L1749
	ldrh	w1, [x19, 84]
	add	x20, x19, 80
	cbnz	w1, .L1749
	bl	FtlGcRefreshBlock
	ldrh	w0, [x19, 80]
	bl	FtlGcRefreshBlock
	bl	FtlVpcTblFlush
	mov	x0, x21
	bl	allocate_new_data_superblock
	mov	x0, x20
	bl	allocate_new_data_superblock
.L1749:
	bl	FtlVpcCheckAndModify
	b	.L1738
	.size	FtlSysBlkInit, .-FtlSysBlkInit
	.align	2
	.global	ftl_low_format
	.type	ftl_low_format, %function
ftl_low_format:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	add	x0, x20, :lo12:.LANCHOR0
	stp	x21, x22, [sp, 32]
	str	wzr, [x0, 2660]
	str	wzr, [x0, 2664]
	str	wzr, [x0, 2676]
	ldrh	w0, [x0, 240]
	bl	FtlFreeSysBlkQueueInit
	bl	FtlLoadBbt
	cbz	w0, .L1756
	bl	FtlMakeBbt
.L1756:
	mov	w4, 23752
	add	x1, x20, :lo12:.LANCHOR0
	mov	w0, 0
	movk	w4, 0xa0f, lsl 16
.L1757:
	ldrh	w2, [x1, 262]
	cmp	w0, w2, lsl 7
	blt	.L1758
	ldrh	w21, [x1, 244]
	add	x22, x20, :lo12:.LANCHOR0
	mov	w19, 0
.L1759:
	ldrh	w0, [x22, 246]
	cmp	w0, w21
	bhi	.L1760
	ldrh	w0, [x22, 236]
	sub	w1, w19, #3
	cmp	w1, w0, lsl 1
	bge	.L1761
.L1765:
	add	x19, x20, :lo12:.LANCHOR0
	mov	w21, 0
	mov	w22, 0
.L1762:
	ldrh	w0, [x19, 244]
	cmp	w0, w22
	bhi	.L1766
	ldrh	w2, [x19, 236]
	ldr	w1, [x19, 248]
	ldrh	w0, [x19, 246]
	str	w0, [x19, 3428]
	udiv	w4, w1, w2
	ubfx	x3, x4, 5, 16
	str	w4, [x19, 2624]
	add	w0, w3, 36
	strh	w0, [x19, 2692]
	mov	w0, 24
	mul	w0, w2, w0
	cmp	w21, w0
	ble	.L1767
	sub	w1, w1, w21
	udiv	w1, w1, w2
	str	w1, [x19, 2624]
	lsr	w1, w1, 5
	add	w1, w1, 24
	strh	w1, [x19, 2692]
.L1767:
	add	x0, x20, :lo12:.LANCHOR0
	ldrh	w1, [x0, 298]
	cbz	w1, .L1769
	ldrh	w5, [x0, 2692]
	add	w5, w5, w1, lsr 1
	strh	w5, [x0, 2692]
	mul	w5, w1, w2
	cmp	w21, w5
	bge	.L1769
	add	w1, w1, 32
	str	w4, [x0, 2624]
	add	w1, w3, w1
	strh	w1, [x0, 2692]
.L1769:
	add	x19, x20, :lo12:.LANCHOR0
	add	x21, x19, 24
	ldrh	w1, [x19, 2692]
	ldr	w0, [x19, 2624]
	sub	w0, w0, w1
	ldrh	w1, [x19, 306]
	mul	w0, w0, w2
	str	w0, [x19, 3972]
	mul	w0, w1, w0
	ldrh	w1, [x19, 262]
	str	w0, [x19, 2624]
	mul	w0, w1, w0
	str	w0, [x19, 344]
	bl	FtlBbmTblFlush
	ldr	x0, [x19, 72]
	mov	w1, 0
	ldrh	w2, [x19, 246]
	lsl	w2, w2, 1
	bl	ftl_memset
	mov	w0, -1
	mov	w22, w0
	mov	w1, 1
	strh	w0, [x19, 176]
	strh	wzr, [x19, 178]
	strb	wzr, [x19, 182]
	strb	wzr, [x19, 184]
	strh	wzr, [x21, 2]
	strb	wzr, [x21, 6]
	strh	wzr, [x19, 24]
	strb	w1, [x21, 8]
	str	wzr, [x19, 2620]
.L1771:
	mov	x0, x21
	bl	make_superblock
	ldrb	w1, [x21, 7]
	ldrh	w0, [x21]
	cbnz	w1, .L1772
	ldr	x1, [x19, 72]
	ubfiz	x0, x0, 1, 16
	strh	w22, [x1, x0]
	ldrh	w0, [x21]
	add	w0, w0, 1
	strh	w0, [x21]
	b	.L1771
.L1758:
	ldr	x5, [x1, 3480]
	ubfiz	x3, x0, 2, 16
	mvn	w2, w0
	orr	w2, w0, w2, lsl 16
	add	w0, w0, 1
	and	w0, w0, 65535
	str	w2, [x5, x3]
	ldr	x2, [x1, 3488]
	str	w4, [x2, x3]
	b	.L1757
.L1760:
	mov	w0, w21
	mov	w1, 1
	add	w21, w21, 1
	bl	FtlLowFormatEraseBlock
	add	w19, w19, w0
	and	w21, w21, 65535
	and	w19, w19, 65535
	b	.L1759
.L1761:
	udiv	w0, w19, w0
	ldr	w19, [x22, 336]
	add	x21, x20, :lo12:.LANCHOR0
	add	w0, w0, w19
	bl	FtlSysBlkNumInit
	ldrh	w0, [x22, 240]
	bl	FtlFreeSysBlkQueueInit
	ldrh	w19, [x22, 244]
.L1763:
	ldrh	w0, [x21, 246]
	cmp	w0, w19
	bls	.L1765
	mov	w0, w19
	add	w19, w19, 1
	mov	w1, 1
	and	w19, w19, 65535
	bl	FtlLowFormatEraseBlock
	b	.L1763
.L1766:
	mov	w0, w22
	mov	w1, 0
	add	w22, w22, 1
	bl	FtlLowFormatEraseBlock
	add	w21, w21, w0
	and	w22, w22, 65535
	and	w21, w21, 65535
	b	.L1762
.L1772:
	ldr	w1, [x19, 2660]
	ubfiz	x0, x0, 1, 16
	str	w1, [x21, 12]
	add	w1, w1, 1
	str	w1, [x19, 2660]
	ldr	x1, [x19, 72]
	ldrh	w2, [x21, 4]
	strh	w2, [x1, x0]
	add	x0, x19, 80
	strh	wzr, [x19, 82]
	ldrh	w1, [x21]
	mov	w21, -1
	strb	wzr, [x19, 86]
	add	w1, w1, 1
	strh	w1, [x19, 80]
	mov	w1, 1
	strb	w1, [x19, 88]
	add	x19, x20, :lo12:.LANCHOR0
	mov	x20, x0
.L1773:
	mov	x0, x20
	bl	make_superblock
	ldrb	w1, [x20, 7]
	ldrh	w0, [x20]
	cbnz	w1, .L1774
	ldr	x1, [x19, 72]
	ubfiz	x0, x0, 1, 16
	strh	w21, [x1, x0]
	ldrh	w0, [x20]
	add	w0, w0, 1
	strh	w0, [x20]
	b	.L1773
.L1774:
	ldr	w1, [x19, 2660]
	ubfiz	x0, x0, 1, 16
	str	w1, [x20, 12]
	add	w1, w1, 1
	str	w1, [x19, 2660]
	ldr	x1, [x19, 72]
	ldrh	w2, [x20, 4]
	mov	w20, -1
	strh	w2, [x1, x0]
	strh	w20, [x19, 128]
	bl	FtlFreeSysBlkQueueOut
	strh	w0, [x19, 2696]
	ldr	w0, [x19, 3972]
	strh	w0, [x19, 2702]
	ldr	w0, [x19, 2660]
	str	w0, [x19, 2704]
	add	w0, w0, 1
	strh	wzr, [x19, 2698]
	strh	w20, [x19, 2700]
	str	w0, [x19, 2660]
	bl	FtlVpcTblFlush
	bl	FtlSysBlkInit
	cbnz	w0, .L1775
	adrp	x0, .LANCHOR2
	mov	w1, 1
	str	w1, [x0, #:lo12:.LANCHOR2]
.L1775:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 48
	ret
	.size	ftl_low_format, .-ftl_low_format
	.align	2
	.global	sftl_init
	.type	sftl_init, %function
sftl_init:
	stp	x29, x30, [sp, -32]!
	adrp	x1, .LC0
	add	x1, x1, :lo12:.LC0
	mov	w0, -1
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR2
	adrp	x19, .LANCHOR0
	add	x19, x19, :lo12:.LANCHOR0
	str	w0, [x20, #:lo12:.LANCHOR2]
	adrp	x0, .LC77
	add	x0, x0, :lo12:.LC77
	bl	sftl_printk
	mov	x0, x19
	bl	FtlConstantsInit
	bl	FtlMemInit
	bl	FtlVariablesInit
	ldrh	w0, [x19, 240]
	bl	FtlFreeSysBlkQueueInit
	bl	FtlLoadBbt
	cbnz	w0, .L1784
	bl	FtlSysBlkInit
	cbnz	w0, .L1784
	mov	w0, 1
	str	w0, [x20, #:lo12:.LANCHOR2]
	ldrh	w0, [x19, 228]
	cmp	w0, 15
	bhi	.L1784
	mov	w19, 8129
.L1783:
	mov	w1, 1
	mov	w0, 0
	bl	rk_ftl_garbage_collect
	subs	w19, w19, #1
	bne	.L1783
.L1784:
	mov	w0, 0
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
	.size	sftl_init, .-sftl_init
	.align	2
	.global	FtlWriteToIDB
	.type	FtlWriteToIDB, %function
FtlWriteToIDB:
	stp	x29, x30, [sp, -240]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	w19, w0
	stp	x21, x22, [sp, 32]
	adrp	x0, __stack_chk_guard
	stp	x23, x24, [sp, 48]
	mov	w21, w1
	stp	x25, x26, [sp, 64]
	add	x1, x0, :lo12:__stack_chk_guard
	stp	x27, x28, [sp, 80]
	mov	x22, x2
	add	w23, w21, w19
	str	x0, [x29, 112]
	ldr	x2, [x1]
	str	x2, [x29, 232]
	mov	x2,0
	sub	w24, w23, #1
	cmp	w24, 63
	adrp	x20, .LANCHOR0
	bls	.L1788
	cmp	w19, 575
	bls	.L1789
.L1788:
	add	x0, x20, :lo12:.LANCHOR0
	ldr	w1, [x0, 3976]
	cbnz	w1, .L1790
.L1855:
	mov	w19, 0
	b	.L1787
.L1790:
	ldr	x24, [x0, 3984]
	mov	w1, 35899
	movk	w1, 0xfcdc, lsl 16
	ldr	w2, [x24]
	cmp	w2, w1
	bne	.L1792
	add	x1, x24, 258048
	ldrh	w2, [x0, 10]
	add	x1, x1, 2044
	mov	w0, 0
	mov	w19, 65023
	mov	w3, 4097
.L1796:
	ldr	w4, [x1]
	cbnz	w4, .L1793
	ldr	w4, [x24, w0, uxtw 2]
	add	w0, w0, 1
	str	w4, [x1, 2048]
	cmp	w0, w3
	sub	w19, w19, #1
	csel	w0, w0, wzr, cc
	sub	x1, x1, #4
	cmp	w19, 4096
	bne	.L1796
	mov	w19, 512
	b	.L1795
.L1793:
	add	w19, w19, 127
	lsr	w19, w19, 7
.L1795:
	ubfiz	w0, w2, 2, 14
	add	w21, w19, 4
	mov	w1, w19
	add	x27, x20, :lo12:.LANCHOR0
	mov	w25, 0
	mov	w22, 0
	udiv	w21, w21, w0
	adrp	x0, .LC130
	add	x0, x0, :lo12:.LC130
	add	w21, w21, 1
	mov	w2, w21
	bl	sftl_printk
	lsl	w0, w19, 7
	str	w0, [x29, 140]
.L1797:
	add	w0, w21, w22
	str	w0, [x29, 144]
	cmp	w0, 8
	bls	.L1819
	cbnz	w25, .L1820
.L1792:
	mov	w19, -1
.L1820:
	add	x20, x20, :lo12:.LANCHOR0
	ldr	x0, [x20, 3984]
	str	wzr, [x20, 3976]
	bl	kfree
	ldr	x0, [x20, 3992]
	bl	kfree
.L1787:
	ldr	x1, [x29, 112]
	mov	w0, w19
	add	x1, x1, :lo12:__stack_chk_guard
	ldr	x2, [x29, 232]
	ldr	x1, [x1]
	eor	x1, x2, x1
	cbz	x1, .L1827
	bl	__stack_chk_fail
.L1819:
	ldr	x0, [x27, 3992]
	mov	w1, 0
	mov	x2, 512
	bl	memset
	ldrh	w28, [x27, 10]
	ldr	x1, [x27, 3392]
	mul	w0, w22, w28
	str	w0, [x29, 152]
	cbz	x1, .L1828
	ldr	x0, [x27, 3400]
	mov	w23, 6
	cmp	x0, 0
	mov	w0, 9
	csel	w23, w23, w0, eq
.L1798:
	mul	w4, w22, w28
	mov	w26, 0
.L1799:
	ldr	x6, [x27, 3368]
	mov	w1, w4
	str	w4, [x29, 148]
	mov	w0, 0
	add	w26, w26, 1
	blr	x6
	ldr	w4, [x29, 148]
	cmp	w21, w26
	add	w4, w4, w28
	bhi	.L1799
	cmp	w23, 9
	bne	.L1830
	ldr	x26, [x27, 3992]
	mov	w2, 1024
	mov	w1, 0
	mov	x0, x26
	bl	ftl_memset
	mov	w0, 18766
	mov	w1, 12
	movk	w0, 0x464e, lsl 16
	stp	w0, w1, [x26]
	mov	w0, 4
	strb	w0, [x26, 17]
	ldrh	w0, [x27, 10]
	strh	w0, [x26, 18]
	strb	wzr, [x26, 16]
	mov	w0, 16
	strb	wzr, [x26, 20]
	strb	w0, [x26, 21]
	add	x0, x26, 12
	strh	wzr, [x26, 22]
	str	wzr, [x26, 12]
	bl	js_hash
	str	w0, [x26, 8]
.L1800:
	mul	w0, w21, w28
	mov	x7, x24
	mov	x28, 0
	str	w0, [x29, 148]
.L1801:
	ldr	w0, [x29, 148]
	mov	w1, w28
	cmp	w28, w0
	bcs	.L1808
	cmp	w23, 9
	bne	.L1802
	add	w0, w1, 1
.L1853:
	str	w0, [x29, 168]
	mov	w0, 61424
	str	w0, [x29, 172]
	cmp	w1, 0
	ccmp	w23, 9, 0, eq
	bne	.L1804
	ldr	x1, [x27, 3392]
	add	x8, x27, 3360
	str	x7, [x29, 104]
	mov	w0, 70
	str	x8, [x29, 128]
	blr	x1
	ldr	x8, [x29, 128]
	add	x3, x29, 168
	ldr	w1, [x29, 152]
	mov	x2, x26
	str	x8, [x29, 120]
	mov	w0, 0
	ldr	x9, [x8, 16]
	blr	x9
	mov	w1, w0
	ldr	x8, [x29, 120]
	ldrb	w0, [x27, 22]
	str	w1, [x29, 128]
	ldr	x2, [x8, 32]
	blr	x2
	ldr	w1, [x29, 128]
	ldr	x7, [x29, 104]
	cmn	w1, #1
	bne	.L1805
.L1808:
	ldrb	w0, [x27, 14]
	str	w0, [x29, 128]
	ldrh	w0, [x27, 10]
	ldr	x28, [x27, 3992]
	mul	w1, w22, w0
	str	w1, [x29, 152]
	add	x1, x27, 3360
	ldr	x2, [x1, 32]
	cbnz	x2, .L1806
	mov	w1, 6
.L1854:
	mul	w0, w21, w0
	mov	x26, 0
	str	w0, [x29, 120]
	str	w1, [x29, 148]
.L1810:
	ldr	w0, [x29, 120]
	mov	w1, w26
	cmp	w0, w26
	bls	.L1814
	ldr	w0, [x29, 148]
	cmp	w26, 0
	ccmp	w0, 9, 0, eq
	bne	.L1811
	ldr	x1, [x27, 3392]
	mov	w0, 70
	blr	x1
	ldr	x1, [x27, 3400]
	mov	w0, 2
	blr	x1
	ldr	x8, [x27, 3384]
	add	x3, x29, 168
	ldr	w1, [x29, 152]
	mov	x2, x28
	mov	w0, 0
	blr	x8
	ldrb	w0, [x29, 128]
	ldr	x1, [x27, 3400]
	blr	x1
	ldrb	w0, [x27, 22]
	ldr	x1, [x27, 3392]
	blr	x1
	ldr	w0, [x28]
	mov	w1, 18766
	movk	w1, 0x464e, lsl 16
	cmp	w0, w1
	beq	.L1812
.L1814:
	ldr	x0, [x27, 3992]
	mov	x1, x24
	mov	w23, 0
	mov	x2, x0
.L1813:
	mov	x26, x2
	mov	x3, x1
	add	x2, x2, 4
	add	x1, x1, 4
	ldr	w4, [x3]
	ldr	w6, [x26]
	cmp	w6, w4
	beq	.L1816
	mov	x2, 512
	mov	w1, 0
	str	x3, [x29, 152]
	bl	memset
	ldr	x3, [x29, 152]
	mov	w1, w22
	ldr	w2, [x26]
	mov	w4, w23
	adrp	x0, .LC131
	add	x0, x0, :lo12:.LC131
	ldr	w3, [x3]
	bl	sftl_printk
	ldrh	w1, [x27, 10]
	mov	w0, 0
	ldr	x2, [x27, 3368]
	mul	w1, w1, w22
	blr	x2
.L1817:
	ldr	w22, [x29, 144]
	b	.L1797
.L1828:
	mov	w23, 6
	b	.L1798
.L1830:
	mov	x26, 0
	b	.L1800
.L1802:
	lsl	w0, w1, 2
	b	.L1853
.L1804:
	ldr	x8, [x27, 3376]
	add	x3, x29, 168
	ldr	w0, [x29, 152]
	mov	x2, x7
	str	x7, [x29, 128]
	add	w1, w1, w0
	mov	w0, 0
	blr	x8
	cmn	w0, #1
	beq	.L1808
	ldr	x7, [x29, 128]
	add	x7, x7, 2048
.L1805:
	add	x28, x28, 1
	b	.L1801
.L1806:
	ldr	x1, [x1, 40]
	mov	w23, 6
	cmp	x1, 0
	mov	w1, 9
	csel	w1, w23, w1, eq
	b	.L1854
.L1811:
	ldr	x7, [x27, 3384]
	add	x3, x29, 168
	ldr	w0, [x29, 152]
	mov	x2, x28
	add	w1, w0, w1
	mov	w0, 0
	blr	x7
	cmn	w0, #1
	beq	.L1814
	ldr	w0, [x29, 172]
	mov	w1, 61424
	cmp	w0, w1
	bne	.L1814
	add	x28, x28, 2048
.L1812:
	add	x26, x26, 1
	b	.L1810
.L1816:
	ldr	w3, [x29, 140]
	add	w23, w23, 1
	cmp	w23, w3
	bne	.L1813
	add	w25, w25, 1
	cmp	w25, 5
	bls	.L1817
	b	.L1820
.L1789:
	cmp	w19, 64
	bne	.L1821
	add	x25, x20, :lo12:.LANCHOR0
	mov	w0, 262144
	bl	ftl_malloc
	str	x0, [x25, 3984]
	mov	w0, 262144
	bl	ftl_malloc
	mov	x1, x0
	ldr	x0, [x25, 3984]
	str	x1, [x25, 3992]
	cbz	x0, .L1822
	cbz	x1, .L1822
	mov	w1, 1
	mov	w2, 262144
	str	w1, [x25, 3976]
	mov	w1, 0
	bl	ftl_memset
.L1821:
	add	x20, x20, :lo12:.LANCHOR0
	ldr	w0, [x20, 3976]
	cbz	w0, .L1855
	cmp	w19, 63
	ldr	x0, [x20, 3984]
	bhi	.L1824
	mov	w1, 64
	sub	w19, w1, w19
	sub	w21, w21, w19
	ubfiz	x19, x19, 9, 25
	add	x1, x22, x19
.L1825:
	cmp	w24, 575
	bls	.L1826
	sub	w21, w21, w23
	sub	w21, w21, #446
.L1826:
	lsl	w2, w21, 9
	bl	ftl_memcpy
	b	.L1855
.L1822:
	adrp	x1, .LANCHOR1
	add	x1, x1, :lo12:.LANCHOR1
	add	x1, x1, 752
	adrp	x0, .LC132
	add	x0, x0, :lo12:.LC132
	bl	sftl_printk
	b	.L1821
.L1824:
	lsl	w2, w19, 7
	mov	x1, x22
	sub	w2, w2, #8192
	add	x0, x0, x2, lsl 2
	b	.L1825
.L1827:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 240
	ret
	.size	FtlWriteToIDB, .-FtlWriteToIDB
	.align	2
	.global	sftl_write
	.type	sftl_write, %function
sftl_write:
	stp	x29, x30, [sp, -48]!
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	w20, w1
	mov	w19, w0
	str	x21, [sp, 32]
	mov	x21, x2
	bl	FtlWriteToIDB
	mov	x3, x21
	mov	w2, w20
	mov	w1, w19
	mov	w0, 0
	bl	FtlWrite
	ldr	x21, [sp, 32]
	ldp	x19, x20, [sp, 16]
	ldp	x29, x30, [sp], 48
	ret
	.size	sftl_write, .-sftl_write
	.align	2
	.global	rk_sftl_vendor_dev_ops_register
	.type	rk_sftl_vendor_dev_ops_register, %function
rk_sftl_vendor_dev_ops_register:
	adrp	x2, .LANCHOR0
	add	x2, x2, :lo12:.LANCHOR0
	ldr	x3, [x2, 4000]
	cbnz	x3, .L1860
	str	x0, [x2, 4000]
	mov	w0, 0
	str	x1, [x2, 4008]
	ret
.L1860:
	mov	w0, -1
	ret
	.size	rk_sftl_vendor_dev_ops_register, .-rk_sftl_vendor_dev_ops_register
	.align	2
	.global	rk_sftl_vendor_storage_init
	.type	rk_sftl_vendor_storage_init, %function
rk_sftl_vendor_storage_init:
	stp	x29, x30, [sp, -80]!
	mov	w0, 65536
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	adrp	x20, .LANCHOR0
	stp	x21, x22, [sp, 32]
	add	x22, x20, :lo12:.LANCHOR0
	stp	x23, x24, [sp, 48]
	stp	x25, x26, [sp, 64]
	bl	ftl_malloc
	str	x0, [x22, 4016]
	cbz	x0, .L1867
	mov	w26, 22084
	mov	w25, 0
	mov	w24, 0
	mov	w23, 0
	movk	w26, 0x524b, lsl 16
.L1865:
	ldr	x3, [x22, 4000]
	mov	w1, 128
	ldr	x2, [x22, 4016]
	lsl	w0, w23, 7
	blr	x3
	mov	w21, w0
	cbnz	w0, .L1863
	ldr	x19, [x22, 4016]
	ldr	w0, [x19]
	cmp	w0, w26
	bne	.L1864
	add	x0, x19, 61440
	ldr	w1, [x19, 4]
	ldr	w0, [x0, 4092]
	cmp	w0, w1
	bne	.L1864
	cmp	w0, w24
	bls	.L1864
	mov	w25, w23
	mov	w24, w0
.L1864:
	add	w23, w23, 1
	cmp	w23, 2
	bne	.L1865
	cbz	w24, .L1866
	add	x0, x20, :lo12:.LANCHOR0
	mov	x2, x19
	mov	w1, 128
	ldr	x3, [x0, 4000]
	lsl	w0, w25, 7
	blr	x3
	mov	w21, w0
	cbz	w0, .L1861
.L1863:
	add	x20, x20, :lo12:.LANCHOR0
	mov	w21, -1
	ldr	x0, [x20, 4016]
	bl	kfree
	str	xzr, [x20, 4016]
	b	.L1861
.L1866:
	mov	w1, 0
	mov	x2, 65536
	mov	x0, x19
	bl	memset
	mov	w1, 22084
	mov	w0, 1
	movk	w1, 0x524b, lsl 16
	stp	w1, w0, [x19]
	add	x1, x19, 61440
	str	w0, [x1, 4092]
	mov	w0, -1032
	strh	w0, [x19, 14]
.L1861:
	mov	w0, w21
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x29, x30, [sp], 80
	ret
.L1867:
	mov	w21, -12
	b	.L1861
	.size	rk_sftl_vendor_storage_init, .-rk_sftl_vendor_storage_init
	.align	2
	.global	rk_sftl_vendor_read
	.type	rk_sftl_vendor_read, %function
rk_sftl_vendor_read:
	adrp	x3, .LANCHOR0+4016
	ldr	x4, [x3, #:lo12:.LANCHOR0+4016]
	cbz	x4, .L1875
	ldrh	w6, [x4, 10]
	add	x5, x4, 16
	mov	w3, 0
.L1872:
	cmp	w3, w6
	bcc	.L1874
.L1875:
	mov	w0, -1
	ret
.L1874:
	ldrh	w7, [x5], 8
	cmp	w7, w0
	bne	.L1873
	stp	x29, x30, [sp, -32]!
	add	x3, x4, x3, uxtw 3
	mov	x0, x1
	add	x29, sp, 0
	str	x19, [sp, 16]
	ldrh	w19, [x3, 20]
	ldrh	w1, [x3, 18]
	cmp	w19, w2
	csel	w19, w19, w2, ls
	add	x1, x1, 1024
	uxtw	x2, w19
	add	x1, x4, x1
	bl	memcpy
	mov	w0, w19
	ldr	x19, [sp, 16]
	ldp	x29, x30, [sp], 32
	ret
.L1873:
	add	w3, w3, 1
	b	.L1872
	.size	rk_sftl_vendor_read, .-rk_sftl_vendor_read
	.align	2
	.global	rk_sftl_vendor_write
	.type	rk_sftl_vendor_write, %function
rk_sftl_vendor_write:
	stp	x29, x30, [sp, -128]!
	add	x29, sp, 0
	stp	x23, x24, [sp, 48]
	adrp	x23, .LANCHOR0
	add	x3, x23, :lo12:.LANCHOR0
	stp	x19, x20, [sp, 16]
	stp	x21, x22, [sp, 32]
	stp	x25, x26, [sp, 64]
	stp	x27, x28, [sp, 80]
	ldr	x19, [x3, 4016]
	cbz	x19, .L1895
	add	w21, w2, 63
	ldrh	w3, [x19, 10]
	ldrh	w24, [x19, 8]
	mov	x4, x1
	mov	w28, w2
	and	w21, w21, -64
	add	x1, x19, 16
	mov	w20, 0
.L1882:
	cmp	w20, w3
	bcc	.L1890
	ldrh	w1, [x19, 14]
	cmp	w21, w1
	bhi	.L1895
	add	x3, x19, x3, uxth 3
	and	w21, w21, 65535
	uxtw	x2, w28
	strh	w0, [x3, 16]
	ldrh	w0, [x19, 12]
	strh	w0, [x3, 18]
	strh	w28, [x3, 20]
	add	w0, w0, w21
	sub	w21, w1, w21
	strh	w0, [x19, 12]
	strh	w21, [x19, 14]
	mov	x1, x4
	ldrh	w0, [x3, 18]
	add	x0, x0, 1024
	add	x0, x19, x0
	bl	memcpy
	ldrh	w0, [x19, 10]
	add	w0, w0, 1
	strh	w0, [x19, 10]
	b	.L1897
.L1890:
	ldrh	w6, [x1], 8
	cmp	w6, w0
	bne	.L1883
	uxtw	x27, w20
	add	x5, x19, 1024
	add	x22, x19, x27, lsl 3
	ldrh	w25, [x22, 20]
	add	w25, w25, 63
	and	w25, w25, -64
	cmp	w28, w25
	bls	.L1884
	ldrh	w0, [x19, 14]
	cmp	w21, w0
	bhi	.L1895
	add	x27, x27, 2
	ldrh	w22, [x22, 18]
	add	x27, x19, x27, lsl 3
	sub	w26, w3, #1
.L1885:
	cmp	w20, w26
	bcc	.L1886
	add	x20, x19, x20, uxtw 3
	and	w22, w22, 65535
	add	x0, x5, x22, uxth
	uxtw	x2, w28
	mov	x1, x4
	and	w21, w21, 65535
	strh	w22, [x20, 18]
	add	w22, w22, w21
	strh	w6, [x20, 16]
	strh	w28, [x20, 20]
	bl	memcpy
	strh	w22, [x19, 12]
	ldrh	w0, [x19, 14]
	sub	w21, w0, w21
	add	w25, w21, w25
	strh	w25, [x19, 14]
.L1897:
	ldr	w0, [x19, 4]
	add	x1, x19, 61440
	add	x23, x23, :lo12:.LANCHOR0
	mov	x2, x19
	add	w0, w0, 1
	str	w0, [x19, 4]
	str	w0, [x1, 4092]
	mov	w1, 128
	ldr	x3, [x23, 4008]
	ldrh	w0, [x19, 8]
	add	w0, w0, 1
	and	w0, w0, 65535
	cmp	w0, 1
	csel	w0, w0, wzr, ls
	strh	w0, [x19, 8]
	lsl	w0, w24, 7
	blr	x3
	mov	w0, 0
.L1880:
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x23, x24, [sp, 48]
	ldp	x25, x26, [sp, 64]
	ldp	x27, x28, [sp, 80]
	ldp	x29, x30, [sp], 128
	ret
.L1886:
	add	w20, w20, 1
	str	x4, [x29, 104]
	add	x0, x19, x20, uxtw 3
	str	x5, [x29, 120]
	add	x27, x27, 8
	ldrh	w1, [x0, 16]
	strh	w1, [x27, -8]
	ldrh	w1, [x0, 20]
	strh	w1, [x27, -4]
	strh	w22, [x27, -6]
	ldrh	w3, [x0, 20]
	ldrh	w1, [x0, 18]
	add	x0, x5, x22, uxtw
	add	w3, w3, 63
	and	w3, w3, -64
	add	x1, x5, x1
	and	x2, x3, 131008
	stp	w6, w3, [x29, 112]
	bl	memcpy
	ldp	w6, w3, [x29, 112]
	ldr	x4, [x29, 104]
	add	w22, w22, w3
	ldr	x5, [x29, 120]
	b	.L1885
.L1884:
	ldrh	w0, [x22, 18]
	uxtw	x2, w28
	mov	x1, x4
	add	x0, x5, x0
	bl	memcpy
	strh	w28, [x22, 20]
	b	.L1897
.L1883:
	add	w20, w20, 1
	b	.L1882
.L1895:
	mov	w0, -1
	b	.L1880
	.size	rk_sftl_vendor_write, .-rk_sftl_vendor_write
	.align	2
	.global	rk_sftl_vendor_storage_ioctl
	.type	rk_sftl_vendor_storage_ioctl, %function
rk_sftl_vendor_storage_ioctl:
	stp	x29, x30, [sp, -64]!
	mov	w0, 4096
	add	x29, sp, 0
	stp	x19, x20, [sp, 16]
	mov	x20, x2
	stp	x21, x22, [sp, 32]
	mov	w21, w1
	str	x23, [sp, 48]
	bl	ftl_malloc
	cbz	x0, .L1920
	mov	x19, x0
	mov	w0, 30209
	movk	w0, 0x4004, lsl 16
	cmp	w21, w0
	beq	.L1901
	add	w0, w0, 1
	cmp	w21, w0
	beq	.L1902
.L1936:
	mov	x20, -14
	b	.L1900
.L1901:
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x0, sp_el0
// 0 "" 2
#NO_APP
	ldr	w2, [x0, 44]
	ldr	x1, [x0, 8]
	tbnz	x2, 21, .L1903
	ldr	x0, [x0]
	tbz	x0, 26, .L1922
.L1903:
	sbfx	x0, x20, 0, 56
	and	x0, x0, x20
.L1904:
#APP
// 89 "./arch/arm64/include/asm/uaccess.h" 1
		adds	x0, x0, 8
	csel	x1, xzr, x1, hi
	csinv	x0, x0, xzr, cc
	sbcs	xzr, x0, x1
	cset	x0, ls

// 0 "" 2
#NO_APP
	cbz	x0, .L1923
	sbfx	x21, x20, 0, 56
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x22, sp_el0
// 0 "" 2
#NO_APP
	and	x21, x21, x20
	ldr	x0, [x22, 8]
#APP
// 249 "./arch/arm64/include/asm/uaccess.h" 1
		bics	xzr, x21, x0
	csel	x1, x20, xzr, eq

// 0 "" 2
// 257 "./arch/arm64/include/asm/uaccess.h" 1
	hint #20
// 0 "" 2
#NO_APP
	mov	x2, 8
	mov	x0, x19
	bl	__arch_copy_from_user
	mov	x2, x0
	cbz	x0, .L1906
.L1905:
	sub	x0, x19, x2
	mov	w1, 0
	add	x0, x0, 8
	bl	memset
	mov	x2, x20
	mov	w1, 256
.L1938:
	adrp	x0, .LC133
	add	x0, x0, :lo12:.LC133
	bl	sftl_printk
	b	.L1936
.L1922:
	mov	x0, x20
	b	.L1904
.L1923:
	mov	x2, 8
	b	.L1905
.L1907:
	strh	w0, [x19, 6]
	and	x2, x0, 65535
	add	x2, x2, 8
	ldr	w0, [x22, 44]
	ldr	x1, [x22, 8]
	tbnz	x0, 21, .L1924
	ldr	x0, [x22]
	tst	x0, 67108864
	csel	x0, x21, x20, ne
.L1908:
#APP
// 89 "./arch/arm64/include/asm/uaccess.h" 1
		adds	x0, x0, x2
	csel	x1, xzr, x1, hi
	csinv	x0, x0, xzr, cc
	sbcs	xzr, x0, x1
	cset	x0, ls

// 0 "" 2
#NO_APP
	cbz	x0, .L1936
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x0, sp_el0
// 0 "" 2
#NO_APP
	ldr	x1, [x0, 8]
#APP
// 249 "./arch/arm64/include/asm/uaccess.h" 1
		bics	xzr, x21, x1
	csel	x0, x20, xzr, eq

// 0 "" 2
// 257 "./arch/arm64/include/asm/uaccess.h" 1
	hint #20
// 0 "" 2
#NO_APP
	mov	x20, 0
	mov	x1, x19
	bl	__arch_copy_to_user
	cbz	x0, .L1900
	b	.L1936
.L1924:
	mov	x0, x21
	b	.L1908
.L1902:
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x0, sp_el0
// 0 "" 2
#NO_APP
	ldr	w2, [x0, 44]
	ldr	x1, [x0, 8]
	tbnz	x2, 21, .L1910
	ldr	x0, [x0]
	tbz	x0, 26, .L1927
.L1910:
	sbfx	x0, x20, 0, 56
	and	x0, x0, x20
.L1911:
#APP
// 89 "./arch/arm64/include/asm/uaccess.h" 1
		adds	x0, x0, 8
	csel	x1, xzr, x1, hi
	csinv	x0, x0, xzr, cc
	sbcs	xzr, x0, x1
	cset	x0, ls

// 0 "" 2
#NO_APP
	cbz	x0, .L1928
	sbfx	x22, x20, 0, 56
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x23, sp_el0
// 0 "" 2
#NO_APP
	and	x22, x22, x20
	ldr	x0, [x23, 8]
#APP
// 249 "./arch/arm64/include/asm/uaccess.h" 1
		bics	xzr, x22, x0
	csel	x1, x20, xzr, eq

// 0 "" 2
// 257 "./arch/arm64/include/asm/uaccess.h" 1
	hint #20
// 0 "" 2
#NO_APP
	mov	x2, 8
	mov	x0, x19
	bl	__arch_copy_from_user
	mov	x2, x0
	cbz	x0, .L1913
.L1912:
	sub	x0, x19, x2
	mov	w1, 0
	add	x0, x0, 8
	bl	memset
	mov	x2, x20
	mov	w1, 276
	b	.L1938
.L1927:
	mov	x0, x20
	b	.L1911
.L1928:
	mov	x2, 8
	b	.L1912
.L1929:
	mov	x0, x22
	b	.L1915
.L1931:
	mov	x0, x21
	b	.L1916
.L1920:
	mov	x20, -1
	b	.L1898
.L1906:
	ldr	w1, [x19]
	mov	w0, 17745
	movk	w0, 0x5652, lsl 16
	cmp	w1, w0
	bne	.L1914
	ldrh	w2, [x19, 6]
	add	x1, x19, 8
	ldrh	w0, [x19, 4]
	bl	rk_sftl_vendor_read
	cmn	w0, #1
	bne	.L1907
.L1914:
	mov	x20, -1
.L1900:
	mov	x0, x19
	bl	kfree
.L1898:
	mov	x0, x20
	ldr	x23, [sp, 48]
	ldp	x19, x20, [sp, 16]
	ldp	x21, x22, [sp, 32]
	ldp	x29, x30, [sp], 64
	ret
.L1913:
	ldr	w1, [x19]
	mov	w0, 17745
	movk	w0, 0x5652, lsl 16
	cmp	w1, w0
	bne	.L1914
	ldrh	w21, [x19, 6]
	cmp	w21, 4087
	bhi	.L1914
	ldr	w0, [x23, 44]
	add	w21, w21, 8
	sxtw	x21, w21
	ldr	x1, [x23, 8]
	tbnz	x0, 21, .L1929
	ldr	x0, [x23]
	tst	x0, 67108864
	csel	x0, x22, x20, ne
.L1915:
#APP
// 89 "./arch/arm64/include/asm/uaccess.h" 1
		adds	x0, x0, x21
	csel	x1, xzr, x1, hi
	csinv	x0, x0, xzr, cc
	sbcs	xzr, x0, x1
	cset	x0, ls

// 0 "" 2
#NO_APP
	cbz	x0, .L1931
#APP
// 19 "./arch/arm64/include/asm/current.h" 1
	mrs x0, sp_el0
// 0 "" 2
#NO_APP
	ldr	x0, [x0, 8]
#APP
// 249 "./arch/arm64/include/asm/uaccess.h" 1
		bics	xzr, x22, x0
	csel	x1, x20, xzr, eq

// 0 "" 2
// 257 "./arch/arm64/include/asm/uaccess.h" 1
	hint #20
// 0 "" 2
#NO_APP
	mov	x2, x21
	mov	x0, x19
	bl	__arch_copy_from_user
	cbz	x0, .L1917
.L1916:
	sub	x21, x21, x0
	mov	x2, x0
	mov	w1, 0
	add	x0, x19, x21
	bl	memset
	mov	x2, x20
	mov	w1, 283
	b	.L1938
.L1917:
	ldrh	w2, [x19, 6]
	add	x1, x19, 8
	ldrh	w0, [x19, 4]
	bl	rk_sftl_vendor_write
	sxtw	x20, w0
	b	.L1900
	.size	rk_sftl_vendor_storage_ioctl, .-rk_sftl_vendor_storage_ioctl
	.align	2
	.global	rk_sftl_vendor_register
	.type	rk_sftl_vendor_register, %function
rk_sftl_vendor_register:
	stp	x29, x30, [sp, -16]!
	adrp	x0, .LANCHOR2
	add	x0, x0, :lo12:.LANCHOR2
	add	x29, sp, 0
	add	x0, x0, 16
	bl	misc_register
	ldp	x29, x30, [sp], 16
	ret
	.size	rk_sftl_vendor_register, .-rk_sftl_vendor_register
	.global	g_nand_ops
	.global	g_nand_phy_info
	.global	gc_ink_free_return_value
	.global	check_vpc_table
	.global	FtlUpdateVaildLpnCount
	.global	g_ect_tbl_power_up_flush
	.global	power_up_flag
	.global	gFtlInitStatus
	.global	DeviceCapacity
	.global	g_power_lost_recovery_flag
	.global	c_mlc_erase_count_value
	.global	g_recovery_ppa_tbl
	.global	g_recovery_page_min_ver
	.global	g_recovery_page_num
	.global	sftl_nand_check_spare_buf
	.global	sftl_temp_buf
	.global	sftl_nand_check_buf
	.global	g_cur_erase_blk
	.global	g_gc_skip_write_count
	.global	g_gc_head_data_block_count
	.global	g_gc_head_data_block
	.global	g_ftl_nand_free_count
	.global	g_in_swl_replace
	.global	g_in_gc_progress
	.global	g_max_erase_count
	.global	g_totle_sys_slc_erase_count
	.global	g_totle_slc_erase_count
	.global	g_min_erase_count
	.global	g_totle_avg_erase_count
	.global	g_totle_mlc_erase_count
	.global	g_totle_l2p_write_count
	.global	g_totle_cache_write_count
	.global	g_tmp_data_superblock_id
	.global	g_totle_read_page_count
	.global	g_totle_discard_page_count
	.global	g_totle_read_sector
	.global	g_totle_write_sector
	.global	g_totle_write_page_count
	.global	g_totle_gc_page_count
	.global	g_gc_blk_index
	.global	g_gc_merge_free_blk_threshold
	.global	g_gc_free_blk_threshold
	.global	g_gc_bad_block_temp_tbl
	.global	g_gc_bad_block_gc_index
	.global	g_gc_bad_block_temp_num
	.global	g_gc_next_blk_1
	.global	g_gc_next_blk
	.global	g_gc_cur_blk_max_valid_pages
	.global	g_gc_cur_blk_valid_pages
	.global	g_gc_page_offset
	.global	g_gc_blk_num
	.global	p_gc_blk_tbl
	.global	p_gc_page_info
	.global	g_sys_ext_data
	.global	g_sys_save_data
	.global	gp_last_act_superblock
	.global	g_gc_superblock
	.global	g_gc_temp_superblock
	.global	g_buffer_superblock
	.global	g_active_superblock
	.global	g_num_data_superblocks
	.global	g_num_free_superblocks
	.global	p_data_block_list_tail
	.global	p_data_block_list_head
	.global	p_free_data_block_list_head
	.global	p_data_block_list_table
	.global	g_l2p_last_update_region_id
	.global	p_l2p_map_buf
	.global	p_l2p_ram_map
	.global	g_totle_vendor_block
	.global	p_vendor_region_ppn_table
	.global	p_vendor_block_ver_table
	.global	p_vendor_block_valid_page_count
	.global	p_vendor_block_table
	.global	g_totle_map_block
	.global	p_map_region_ppn_table
	.global	p_map_block_ver_table
	.global	p_map_block_valid_page_count
	.global	p_map_block_table
	.global	p_valid_page_count_check_table
	.global	p_valid_page_count_table
	.global	g_totle_swl_count
	.global	p_swl_mul_table
	.global	p_erase_count_table
	.global	g_ect_tbl_info_size
	.global	gp_ect_tbl_info
	.global	g_gc_num_req
	.global	c_gc_page_buf_num
	.global	gp_gc_page_buf_info
	.global	p_gc_data_buf
	.global	p_gc_spare_buf
	.global	p_io_spare_buf
	.global	p_io_data_buf_1
	.global	p_io_data_buf_0
	.global	p_sys_spare_buf
	.global	p_vendor_data_buf
	.global	p_sys_data_buf_1
	.global	p_sys_data_buf
	.global	p_plane_order_table
	.global	req_gc_dst
	.global	req_gc
	.global	req_erase
	.global	req_prgm
	.global	req_read
	.global	req_sys
	.global	gVendorBlkInfo
	.global	gL2pMapInfo
	.global	gSysFreeQueue
	.global	gSysInfo
	.global	gBbtInfo
	.global	g_MaxLbn
	.global	g_VaildLpn
	.global	g_MaxLpn
	.global	g_MaxLbaSector
	.global	g_GlobalDataVersion
	.global	g_GlobalSysVersion
	.global	ftl_gc_temp_power_lost_recovery_flag
	.global	c_ftl_nand_max_data_blks
	.global	c_ftl_nand_data_op_blks_per_plane
	.global	c_ftl_nand_data_blks_per_plane
	.global	c_ftl_nand_max_sys_blks
	.global	c_ftl_nand_init_sys_blks_per_plane
	.global	c_ftl_nand_sys_blks_per_plane
	.global	c_ftl_vendor_part_size
	.global	c_ftl_nand_max_vendor_blks
	.global	c_ftl_nand_max_map_blks
	.global	c_ftl_nand_map_blks_per_plane
	.global	c_ftl_nand_vendor_region_num
	.global	c_ftl_nand_l2pmap_ram_region_num
	.global	c_ftl_nand_map_region_num
	.global	c_ftl_nand_totle_phy_blks
	.global	c_ftl_nand_reserved_blks
	.global	c_ftl_nand_byte_pre_oob
	.global	c_ftl_nand_byte_pre_page
	.global	c_ftl_nand_sec_pre_page_shift
	.global	c_ftl_nand_sec_pre_page
	.global	c_ftl_nand_page_pre_super_blk
	.global	c_ftl_nand_page_pre_slc_blk
	.global	c_ftl_nand_page_pre_blk
	.global	c_ftl_nand_bbm_buf_size
	.global	c_ftl_nand_ext_blk_pre_plane
	.global	c_ftl_nand_blk_pre_plane
	.global	c_ftl_nand_planes_num
	.global	c_ftl_nand_blks_per_die_shift
	.global	c_ftl_nand_blks_per_die
	.global	c_ftl_nand_planes_per_die
	.global	c_ftl_nand_die_num
	.global	c_ftl_nand_type
	.section	.rodata
	.align	3
	.set	.LANCHOR1,. + 0
	.type	__func__.5954, %object
	.size	__func__.5954, 17
__func__.5954:
	.string	"INSERT_DATA_LIST"
	.zero	7
	.type	__func__.5949, %object
	.size	__func__.5949, 17
__func__.5949:
	.string	"INSERT_FREE_LIST"
	.zero	7
	.type	__func__.5985, %object
	.size	__func__.5985, 17
__func__.5985:
	.string	"List_remove_node"
	.zero	7
	.type	__func__.6017, %object
	.size	__func__.6017, 22
__func__.6017:
	.string	"List_update_data_list"
	.zero	2
	.type	__func__.6124, %object
	.size	__func__.6124, 22
__func__.6124:
	.string	"select_l2p_ram_region"
	.zero	2
	.type	__func__.6444, %object
	.size	__func__.6444, 16
__func__.6444:
	.string	"make_superblock"
	.type	__func__.6620, %object
	.size	__func__.6620, 19
__func__.6620:
	.string	"get_new_active_ppa"
	.zero	5
	.type	__func__.13529, %object
	.size	__func__.13529, 17
__func__.13529:
	.string	"FlashEraseBlocks"
	.zero	7
	.type	__func__.6059, %object
	.size	__func__.6059, 26
__func__.6059:
	.string	"ftl_map_blk_alloc_new_blk"
	.zero	6
	.type	__func__.5564, %object
	.size	__func__.5564, 11
__func__.5564:
	.string	"FtlMemInit"
	.zero	5
	.type	__func__.5791, %object
	.size	__func__.5791, 14
__func__.5791:
	.string	"FtlBbt2Bitmap"
	.zero	2
	.type	__func__.6465, %object
	.size	__func__.6465, 18
__func__.6465:
	.string	"SupperBlkListInit"
	.zero	6
	.type	__func__.13486, %object
	.size	__func__.13486, 15
__func__.13486:
	.string	"FlashReadPages"
	.zero	1
	.type	__func__.6236, %object
	.size	__func__.6236, 14
__func__.6236:
	.string	"FtlScanSysBlk"
	.zero	2
	.type	__func__.5834, %object
	.size	__func__.5834, 11
__func__.5834:
	.string	"FtlLoadBbt"
	.zero	5
	.type	__func__.13505, %object
	.size	__func__.13505, 15
__func__.13505:
	.string	"FlashProgPages"
	.zero	1
	.type	__func__.6592, %object
	.size	__func__.6592, 25
__func__.6592:
	.string	"allocate_data_superblock"
	.zero	7
	.type	__func__.6633, %object
	.size	__func__.6633, 16
__func__.6633:
	.string	"update_vpc_list"
	.type	__func__.6640, %object
	.size	__func__.6640, 20
__func__.6640:
	.string	"decrement_vpc_count"
	.zero	4
	.type	__func__.6085, %object
	.size	__func__.6085, 31
__func__.6085:
	.string	"Ftl_write_map_blk_to_last_page"
	.zero	1
	.type	__func__.6099, %object
	.size	__func__.6099, 16
__func__.6099:
	.string	"FtlMapWritePage"
	.type	__func__.6026, %object
	.size	__func__.6026, 16
__func__.6026:
	.string	"load_l2p_region"
	.type	__func__.6070, %object
	.size	__func__.6070, 15
__func__.6070:
	.string	"ftl_map_blk_gc"
	.zero	1
	.type	__func__.6141, %object
	.size	__func__.6141, 9
__func__.6141:
	.string	"log2phys"
	.zero	7
	.type	__func__.6353, %object
	.size	__func__.6353, 16
__func__.6353:
	.string	"FtlReUsePrevPpa"
	.type	__func__.6387, %object
	.size	__func__.6387, 22
__func__.6387:
	.string	"FtlRecoverySuperblock"
	.zero	2
	.type	__func__.6507, %object
	.size	__func__.6507, 14
__func__.6507:
	.string	"ftl_check_vpc"
	.zero	2
	.type	__func__.6214, %object
	.size	__func__.6214, 15
__func__.6214:
	.string	"FtlVpcTblFlush"
	.zero	1
	.type	__func__.6490, %object
	.size	__func__.6490, 21
__func__.6490:
	.string	"FtlVpcCheckAndModify"
	.zero	3
	.type	__func__.6613, %object
	.size	__func__.6613, 29
__func__.6613:
	.string	"allocate_new_data_superblock"
	.zero	3
	.type	__func__.5688, %object
	.size	__func__.5688, 13
__func__.5688:
	.string	"FtlProgPages"
	.zero	3
	.type	__func__.6710, %object
	.size	__func__.6710, 19
__func__.6710:
	.string	"FtlGcFreeTempBlock"
	.zero	5
	.type	__func__.6823, %object
	.size	__func__.6823, 23
__func__.6823:
	.string	"rk_ftl_garbage_collect"
	.zero	1
	.type	__func__.5716, %object
	.size	__func__.5716, 9
__func__.5716:
	.string	"FtlWrite"
	.zero	7
	.type	__func__.6285, %object
	.size	__func__.6285, 15
__func__.6285:
	.string	"FtlLoadSysInfo"
	.zero	1
	.type	__func__.6307, %object
	.size	__func__.6307, 18
__func__.6307:
	.string	"FtlMapTblRecovery"
	.zero	6
	.type	__func__.13648, %object
	.size	__func__.13648, 14
__func__.13648:
	.string	"FtlWriteToIDB"
	.zero	2
	.type	rk_sftl_vendor_storage_fops, %object
	.size	rk_sftl_vendor_storage_fops, 288
rk_sftl_vendor_storage_fops:
	.zero	72
	.xword	rk_sftl_vendor_storage_ioctl
	.xword	rk_sftl_vendor_storage_ioctl
	.zero	200
	.data
	.align	3
	.set	.LANCHOR2,. + 0
	.type	gFtlInitStatus, %object
	.size	gFtlInitStatus, 4
gFtlInitStatus:
	.word	-1
	.type	ftl_gc_temp_block_bops_scan_page_addr, %object
	.size	ftl_gc_temp_block_bops_scan_page_addr, 2
ftl_gc_temp_block_bops_scan_page_addr:
	.hword	-1
	.zero	2
	.type	power_up_flag, %object
	.size	power_up_flag, 4
power_up_flag:
	.word	1
	.zero	4
	.type	rkflash_vender_storage_dev, %object
	.size	rkflash_vender_storage_dev, 80
rkflash_vender_storage_dev:
	.word	255
	.zero	4
	.xword	.LC134
	.xword	rk_sftl_vendor_storage_fops
	.zero	56
	.bss
	.align	3
	.set	.LANCHOR0,. + 0
	.type	g_nand_phy_info, %object
	.size	g_nand_phy_info, 24
g_nand_phy_info:
	.zero	24
	.type	g_active_superblock, %object
	.size	g_active_superblock, 48
g_active_superblock:
	.zero	48
	.type	p_valid_page_count_table, %object
	.size	p_valid_page_count_table, 8
p_valid_page_count_table:
	.zero	8
	.type	g_buffer_superblock, %object
	.size	g_buffer_superblock, 48
g_buffer_superblock:
	.zero	48
	.type	g_gc_temp_superblock, %object
	.size	g_gc_temp_superblock, 48
g_gc_temp_superblock:
	.zero	48
	.type	g_gc_superblock, %object
	.size	g_gc_superblock, 48
g_gc_superblock:
	.zero	48
	.type	g_gc_next_blk_1, %object
	.size	g_gc_next_blk_1, 2
g_gc_next_blk_1:
	.zero	2
	.type	g_gc_next_blk, %object
	.size	g_gc_next_blk, 2
g_gc_next_blk:
	.zero	2
	.type	g_num_free_superblocks, %object
	.size	g_num_free_superblocks, 2
g_num_free_superblocks:
	.zero	2
	.zero	2
	.type	c_ftl_nand_sys_blks_per_plane, %object
	.size	c_ftl_nand_sys_blks_per_plane, 4
c_ftl_nand_sys_blks_per_plane:
	.zero	4
	.type	c_ftl_nand_planes_num, %object
	.size	c_ftl_nand_planes_num, 2
c_ftl_nand_planes_num:
	.zero	2
	.zero	2
	.type	c_ftl_nand_max_sys_blks, %object
	.size	c_ftl_nand_max_sys_blks, 4
c_ftl_nand_max_sys_blks:
	.zero	4
	.type	c_ftl_nand_data_blks_per_plane, %object
	.size	c_ftl_nand_data_blks_per_plane, 2
c_ftl_nand_data_blks_per_plane:
	.zero	2
	.type	c_ftl_nand_blk_pre_plane, %object
	.size	c_ftl_nand_blk_pre_plane, 2
c_ftl_nand_blk_pre_plane:
	.zero	2
	.type	c_ftl_nand_max_data_blks, %object
	.size	c_ftl_nand_max_data_blks, 4
c_ftl_nand_max_data_blks:
	.zero	4
	.type	c_ftl_nand_totle_phy_blks, %object
	.size	c_ftl_nand_totle_phy_blks, 4
c_ftl_nand_totle_phy_blks:
	.zero	4
	.type	c_ftl_nand_type, %object
	.size	c_ftl_nand_type, 2
c_ftl_nand_type:
	.zero	2
	.type	c_ftl_nand_die_num, %object
	.size	c_ftl_nand_die_num, 2
c_ftl_nand_die_num:
	.zero	2
	.type	c_ftl_nand_planes_per_die, %object
	.size	c_ftl_nand_planes_per_die, 2
c_ftl_nand_planes_per_die:
	.zero	2
	.type	c_ftl_nand_sec_pre_page, %object
	.size	c_ftl_nand_sec_pre_page, 2
c_ftl_nand_sec_pre_page:
	.zero	2
	.type	p_plane_order_table, %object
	.size	p_plane_order_table, 32
p_plane_order_table:
	.zero	32
	.type	c_mlc_erase_count_value, %object
	.size	c_mlc_erase_count_value, 2
c_mlc_erase_count_value:
	.zero	2
	.type	c_ftl_nand_ext_blk_pre_plane, %object
	.size	c_ftl_nand_ext_blk_pre_plane, 2
c_ftl_nand_ext_blk_pre_plane:
	.zero	2
	.type	c_ftl_vendor_part_size, %object
	.size	c_ftl_vendor_part_size, 2
c_ftl_vendor_part_size:
	.zero	2
	.type	c_ftl_nand_blks_per_die, %object
	.size	c_ftl_nand_blks_per_die, 2
c_ftl_nand_blks_per_die:
	.zero	2
	.type	c_ftl_nand_blks_per_die_shift, %object
	.size	c_ftl_nand_blks_per_die_shift, 2
c_ftl_nand_blks_per_die_shift:
	.zero	2
	.type	c_ftl_nand_page_pre_blk, %object
	.size	c_ftl_nand_page_pre_blk, 2
c_ftl_nand_page_pre_blk:
	.zero	2
	.type	c_ftl_nand_page_pre_slc_blk, %object
	.size	c_ftl_nand_page_pre_slc_blk, 2
c_ftl_nand_page_pre_slc_blk:
	.zero	2
	.type	c_ftl_nand_page_pre_super_blk, %object
	.size	c_ftl_nand_page_pre_super_blk, 2
c_ftl_nand_page_pre_super_blk:
	.zero	2
	.type	c_ftl_nand_sec_pre_page_shift, %object
	.size	c_ftl_nand_sec_pre_page_shift, 2
c_ftl_nand_sec_pre_page_shift:
	.zero	2
	.type	c_ftl_nand_byte_pre_page, %object
	.size	c_ftl_nand_byte_pre_page, 2
c_ftl_nand_byte_pre_page:
	.zero	2
	.type	c_ftl_nand_byte_pre_oob, %object
	.size	c_ftl_nand_byte_pre_oob, 2
c_ftl_nand_byte_pre_oob:
	.zero	2
	.type	c_ftl_nand_reserved_blks, %object
	.size	c_ftl_nand_reserved_blks, 2
c_ftl_nand_reserved_blks:
	.zero	2
	.type	DeviceCapacity, %object
	.size	DeviceCapacity, 4
DeviceCapacity:
	.zero	4
	.type	c_ftl_nand_max_vendor_blks, %object
	.size	c_ftl_nand_max_vendor_blks, 2
c_ftl_nand_max_vendor_blks:
	.zero	2
	.type	c_ftl_nand_vendor_region_num, %object
	.size	c_ftl_nand_vendor_region_num, 2
c_ftl_nand_vendor_region_num:
	.zero	2
	.type	c_ftl_nand_map_blks_per_plane, %object
	.size	c_ftl_nand_map_blks_per_plane, 2
c_ftl_nand_map_blks_per_plane:
	.zero	2
	.zero	2
	.type	c_ftl_nand_max_map_blks, %object
	.size	c_ftl_nand_max_map_blks, 4
c_ftl_nand_max_map_blks:
	.zero	4
	.type	c_ftl_nand_init_sys_blks_per_plane, %object
	.size	c_ftl_nand_init_sys_blks_per_plane, 4
c_ftl_nand_init_sys_blks_per_plane:
	.zero	4
	.type	c_ftl_nand_map_region_num, %object
	.size	c_ftl_nand_map_region_num, 2
c_ftl_nand_map_region_num:
	.zero	2
	.type	c_ftl_nand_l2pmap_ram_region_num, %object
	.size	c_ftl_nand_l2pmap_ram_region_num, 2
c_ftl_nand_l2pmap_ram_region_num:
	.zero	2
	.type	g_MaxLbaSector, %object
	.size	g_MaxLbaSector, 4
g_MaxLbaSector:
	.zero	4
	.type	g_totle_vendor_block, %object
	.size	g_totle_vendor_block, 2
g_totle_vendor_block:
	.zero	2
	.zero	2
	.type	p_vendor_block_table, %object
	.size	p_vendor_block_table, 8
p_vendor_block_table:
	.zero	8
	.type	gBbtInfo, %object
	.size	gBbtInfo, 96
gBbtInfo:
	.zero	96
	.type	gSysFreeQueue, %object
	.size	gSysFreeQueue, 2056
gSysFreeQueue:
	.zero	2056
	.type	g_sys_save_data, %object
	.size	g_sys_save_data, 48
g_sys_save_data:
	.zero	48
	.type	p_data_block_list_table, %object
	.size	p_data_block_list_table, 8
p_data_block_list_table:
	.zero	8
	.type	p_data_block_list_head, %object
	.size	p_data_block_list_head, 8
p_data_block_list_head:
	.zero	8
	.type	p_data_block_list_tail, %object
	.size	p_data_block_list_tail, 8
p_data_block_list_tail:
	.zero	8
	.type	g_num_data_superblocks, %object
	.size	g_num_data_superblocks, 2
g_num_data_superblocks:
	.zero	2
	.zero	6
	.type	p_free_data_block_list_head, %object
	.size	p_free_data_block_list_head, 8
p_free_data_block_list_head:
	.zero	8
	.type	p_erase_count_table, %object
	.size	p_erase_count_table, 8
p_erase_count_table:
	.zero	8
	.type	p_l2p_ram_map, %object
	.size	p_l2p_ram_map, 8
p_l2p_ram_map:
	.zero	8
	.type	g_l2p_last_update_region_id, %object
	.size	g_l2p_last_update_region_id, 2
g_l2p_last_update_region_id:
	.zero	2
	.type	FtlUpdateVaildLpnCount, %object
	.size	FtlUpdateVaildLpnCount, 2
FtlUpdateVaildLpnCount:
	.zero	2
	.type	g_VaildLpn, %object
	.size	g_VaildLpn, 4
g_VaildLpn:
	.zero	4
	.type	g_MaxLpn, %object
	.size	g_MaxLpn, 4
g_MaxLpn:
	.zero	4
	.type	g_totle_read_page_count, %object
	.size	g_totle_read_page_count, 4
g_totle_read_page_count:
	.zero	4
	.type	g_totle_discard_page_count, %object
	.size	g_totle_discard_page_count, 4
g_totle_discard_page_count:
	.zero	4
	.type	g_totle_write_page_count, %object
	.size	g_totle_write_page_count, 4
g_totle_write_page_count:
	.zero	4
	.type	g_totle_cache_write_count, %object
	.size	g_totle_cache_write_count, 4
g_totle_cache_write_count:
	.zero	4
	.type	g_totle_l2p_write_count, %object
	.size	g_totle_l2p_write_count, 4
g_totle_l2p_write_count:
	.zero	4
	.type	g_totle_gc_page_count, %object
	.size	g_totle_gc_page_count, 4
g_totle_gc_page_count:
	.zero	4
	.type	g_totle_write_sector, %object
	.size	g_totle_write_sector, 4
g_totle_write_sector:
	.zero	4
	.type	g_totle_read_sector, %object
	.size	g_totle_read_sector, 4
g_totle_read_sector:
	.zero	4
	.type	g_GlobalSysVersion, %object
	.size	g_GlobalSysVersion, 4
g_GlobalSysVersion:
	.zero	4
	.type	g_GlobalDataVersion, %object
	.size	g_GlobalDataVersion, 4
g_GlobalDataVersion:
	.zero	4
	.type	g_totle_mlc_erase_count, %object
	.size	g_totle_mlc_erase_count, 4
g_totle_mlc_erase_count:
	.zero	4
	.type	g_totle_slc_erase_count, %object
	.size	g_totle_slc_erase_count, 4
g_totle_slc_erase_count:
	.zero	4
	.type	g_totle_avg_erase_count, %object
	.size	g_totle_avg_erase_count, 4
g_totle_avg_erase_count:
	.zero	4
	.type	g_totle_sys_slc_erase_count, %object
	.size	g_totle_sys_slc_erase_count, 4
g_totle_sys_slc_erase_count:
	.zero	4
	.type	g_max_erase_count, %object
	.size	g_max_erase_count, 4
g_max_erase_count:
	.zero	4
	.type	g_min_erase_count, %object
	.size	g_min_erase_count, 4
g_min_erase_count:
	.zero	4
	.type	c_ftl_nand_data_op_blks_per_plane, %object
	.size	c_ftl_nand_data_op_blks_per_plane, 2
c_ftl_nand_data_op_blks_per_plane:
	.zero	2
	.zero	2
	.type	gSysInfo, %object
	.size	gSysInfo, 16
gSysInfo:
	.zero	16
	.type	g_sys_ext_data, %object
	.size	g_sys_ext_data, 512
g_sys_ext_data:
	.zero	512
	.type	g_gc_free_blk_threshold, %object
	.size	g_gc_free_blk_threshold, 2
g_gc_free_blk_threshold:
	.zero	2
	.type	g_gc_merge_free_blk_threshold, %object
	.size	g_gc_merge_free_blk_threshold, 2
g_gc_merge_free_blk_threshold:
	.zero	2
	.type	g_gc_skip_write_count, %object
	.size	g_gc_skip_write_count, 4
g_gc_skip_write_count:
	.zero	4
	.type	g_gc_blk_index, %object
	.size	g_gc_blk_index, 2
g_gc_blk_index:
	.zero	2
	.zero	2
	.type	g_in_swl_replace, %object
	.size	g_in_swl_replace, 4
g_in_swl_replace:
	.zero	4
	.type	g_gc_num_req, %object
	.size	g_gc_num_req, 4
g_gc_num_req:
	.zero	4
	.zero	4
	.type	gp_gc_page_buf_info, %object
	.size	gp_gc_page_buf_info, 8
gp_gc_page_buf_info:
	.zero	8
	.type	p_gc_data_buf, %object
	.size	p_gc_data_buf, 8
p_gc_data_buf:
	.zero	8
	.type	p_gc_spare_buf, %object
	.size	p_gc_spare_buf, 8
p_gc_spare_buf:
	.zero	8
	.type	req_gc, %object
	.size	req_gc, 8
req_gc:
	.zero	8
	.type	c_gc_page_buf_num, %object
	.size	c_gc_page_buf_num, 4
c_gc_page_buf_num:
	.zero	4
	.type	g_gc_blk_num, %object
	.size	g_gc_blk_num, 2
g_gc_blk_num:
	.zero	2
	.zero	2
	.type	p_gc_blk_tbl, %object
	.size	p_gc_blk_tbl, 8
p_gc_blk_tbl:
	.zero	8
	.type	g_gc_page_offset, %object
	.size	g_gc_page_offset, 2
g_gc_page_offset:
	.zero	2
	.zero	6
	.type	p_gc_page_info, %object
	.size	p_gc_page_info, 8
p_gc_page_info:
	.zero	8
	.type	g_gc_bad_block_temp_num, %object
	.size	g_gc_bad_block_temp_num, 2
g_gc_bad_block_temp_num:
	.zero	2
	.zero	6
	.type	g_gc_bad_block_temp_tbl, %object
	.size	g_gc_bad_block_temp_tbl, 34
g_gc_bad_block_temp_tbl:
	.zero	34
	.type	g_gc_bad_block_gc_index, %object
	.size	g_gc_bad_block_gc_index, 2
g_gc_bad_block_gc_index:
	.zero	2
	.zero	4
	.type	g_nand_ops, %object
	.size	g_nand_ops, 48
g_nand_ops:
	.zero	48
	.type	req_erase, %object
	.size	req_erase, 8
req_erase:
	.zero	8
	.type	g_in_gc_progress, %object
	.size	g_in_gc_progress, 4
g_in_gc_progress:
	.zero	4
	.type	g_gc_head_data_block, %object
	.size	g_gc_head_data_block, 4
g_gc_head_data_block:
	.zero	4
	.type	g_gc_head_data_block_count, %object
	.size	g_gc_head_data_block_count, 4
g_gc_head_data_block_count:
	.zero	4
	.type	g_cur_erase_blk, %object
	.size	g_cur_erase_blk, 4
g_cur_erase_blk:
	.zero	4
	.type	req_read, %object
	.size	req_read, 8
req_read:
	.zero	8
	.type	req_gc_dst, %object
	.size	req_gc_dst, 8
req_gc_dst:
	.zero	8
	.type	req_prgm, %object
	.size	req_prgm, 8
req_prgm:
	.zero	8
	.type	p_sys_data_buf, %object
	.size	p_sys_data_buf, 8
p_sys_data_buf:
	.zero	8
	.type	p_sys_data_buf_1, %object
	.size	p_sys_data_buf_1, 8
p_sys_data_buf_1:
	.zero	8
	.type	p_vendor_data_buf, %object
	.size	p_vendor_data_buf, 8
p_vendor_data_buf:
	.zero	8
	.type	p_io_data_buf_0, %object
	.size	p_io_data_buf_0, 8
p_io_data_buf_0:
	.zero	8
	.type	p_io_data_buf_1, %object
	.size	p_io_data_buf_1, 8
p_io_data_buf_1:
	.zero	8
	.type	sftl_nand_check_buf, %object
	.size	sftl_nand_check_buf, 8
sftl_nand_check_buf:
	.zero	8
	.type	sftl_temp_buf, %object
	.size	sftl_temp_buf, 8
sftl_temp_buf:
	.zero	8
	.type	sftl_nand_check_spare_buf, %object
	.size	sftl_nand_check_spare_buf, 8
sftl_nand_check_spare_buf:
	.zero	8
	.type	p_sys_spare_buf, %object
	.size	p_sys_spare_buf, 8
p_sys_spare_buf:
	.zero	8
	.type	p_io_spare_buf, %object
	.size	p_io_spare_buf, 8
p_io_spare_buf:
	.zero	8
	.type	g_ect_tbl_info_size, %object
	.size	g_ect_tbl_info_size, 2
g_ect_tbl_info_size:
	.zero	2
	.zero	6
	.type	p_swl_mul_table, %object
	.size	p_swl_mul_table, 8
p_swl_mul_table:
	.zero	8
	.type	gp_ect_tbl_info, %object
	.size	gp_ect_tbl_info, 8
gp_ect_tbl_info:
	.zero	8
	.type	p_valid_page_count_check_table, %object
	.size	p_valid_page_count_check_table, 8
p_valid_page_count_check_table:
	.zero	8
	.type	p_map_block_table, %object
	.size	p_map_block_table, 8
p_map_block_table:
	.zero	8
	.type	p_map_block_valid_page_count, %object
	.size	p_map_block_valid_page_count, 8
p_map_block_valid_page_count:
	.zero	8
	.type	p_vendor_block_valid_page_count, %object
	.size	p_vendor_block_valid_page_count, 8
p_vendor_block_valid_page_count:
	.zero	8
	.type	p_vendor_block_ver_table, %object
	.size	p_vendor_block_ver_table, 8
p_vendor_block_ver_table:
	.zero	8
	.type	p_vendor_region_ppn_table, %object
	.size	p_vendor_region_ppn_table, 8
p_vendor_region_ppn_table:
	.zero	8
	.type	p_map_region_ppn_table, %object
	.size	p_map_region_ppn_table, 8
p_map_region_ppn_table:
	.zero	8
	.type	p_map_block_ver_table, %object
	.size	p_map_block_ver_table, 8
p_map_block_ver_table:
	.zero	8
	.type	p_l2p_map_buf, %object
	.size	p_l2p_map_buf, 8
p_l2p_map_buf:
	.zero	8
	.type	c_ftl_nand_bbm_buf_size, %object
	.size	c_ftl_nand_bbm_buf_size, 2
c_ftl_nand_bbm_buf_size:
	.zero	2
	.zero	6
	.type	gL2pMapInfo, %object
	.size	gL2pMapInfo, 64
gL2pMapInfo:
	.zero	64
	.type	g_totle_map_block, %object
	.size	g_totle_map_block, 2
g_totle_map_block:
	.zero	2
	.type	g_tmp_data_superblock_id, %object
	.size	g_tmp_data_superblock_id, 2
g_tmp_data_superblock_id:
	.zero	2
	.type	g_totle_swl_count, %object
	.size	g_totle_swl_count, 4
g_totle_swl_count:
	.zero	4
	.type	ftl_gc_temp_power_lost_recovery_flag, %object
	.size	ftl_gc_temp_power_lost_recovery_flag, 4
ftl_gc_temp_power_lost_recovery_flag:
	.zero	4
	.type	g_recovery_page_min_ver, %object
	.size	g_recovery_page_min_ver, 4
g_recovery_page_min_ver:
	.zero	4
	.type	req_sys, %object
	.size	req_sys, 32
req_sys:
	.zero	32
	.type	g_power_lost_recovery_flag, %object
	.size	g_power_lost_recovery_flag, 2
g_power_lost_recovery_flag:
	.zero	2
	.zero	2
	.type	g_recovery_page_num, %object
	.size	g_recovery_page_num, 4
g_recovery_page_num:
	.zero	4
	.type	g_recovery_ppa_tbl, %object
	.size	g_recovery_ppa_tbl, 128
g_recovery_ppa_tbl:
	.zero	128
	.type	gVendorBlkInfo, %object
	.size	gVendorBlkInfo, 64
gVendorBlkInfo:
	.zero	64
	.type	g_ect_tbl_power_up_flush, %object
	.size	g_ect_tbl_power_up_flush, 2
g_ect_tbl_power_up_flush:
	.zero	2
	.zero	2
	.type	gc_discard_updated, %object
	.size	gc_discard_updated, 4
gc_discard_updated:
	.zero	4
	.type	gc_ink_free_return_value, %object
	.size	gc_ink_free_return_value, 2
gc_ink_free_return_value:
	.zero	2
	.type	g_gc_cur_blk_valid_pages, %object
	.size	g_gc_cur_blk_valid_pages, 2
g_gc_cur_blk_valid_pages:
	.zero	2
	.type	g_gc_cur_blk_max_valid_pages, %object
	.size	g_gc_cur_blk_max_valid_pages, 2
g_gc_cur_blk_max_valid_pages:
	.zero	2
	.zero	2
	.type	g_ftl_nand_free_count, %object
	.size	g_ftl_nand_free_count, 4
g_ftl_nand_free_count:
	.zero	4
	.type	g_MaxLbn, %object
	.size	g_MaxLbn, 4
g_MaxLbn:
	.zero	4
	.type	idb_need_write_back, %object
	.size	idb_need_write_back, 4
idb_need_write_back:
	.zero	4
	.zero	4
	.type	idb_buf, %object
	.size	idb_buf, 8
idb_buf:
	.zero	8
	.type	gp_flash_check_buf, %object
	.size	gp_flash_check_buf, 8
gp_flash_check_buf:
	.zero	8
	.type	_flash_read, %object
	.size	_flash_read, 8
_flash_read:
	.zero	8
	.type	_flash_write, %object
	.size	_flash_write, 8
_flash_write:
	.zero	8
	.type	g_vendor, %object
	.size	g_vendor, 8
g_vendor:
	.zero	8
	.type	check_vpc_table, %object
	.size	check_vpc_table, 16384
check_vpc_table:
	.zero	16384
	.type	gp_last_act_superblock, %object
	.size	gp_last_act_superblock, 8
gp_last_act_superblock:
	.zero	8
	.section	.rodata.str1.1,"aMS",@progbits,1
.LC0:
	.string	"SFTL version: 5.0.58 20220814"
.LC1:
	.string	"\n%s\n"
.LC2:
	.string	"act blk: %x %x %x %x %x %x\n"
.LC3:
	.string	"buf blk: %x %x %x %x %x %x\n"
.LC4:
	.string	"tmp blk: %x %x %x %x %x %x\n"
.LC5:
	.string	"gc blk: %x %x %x %x %x %x\n"
.LC6:
	.string	"free blk: %x %x %x\n"
.LC7:
	.string	"phyBlk = 0x%x die = %d block_in_die = 0x%x 0x%8x\n"
.LC8:
	.string	"\n!!!!! error @ func:%s - line:%d\n"
.LC9:
	.string	"FLASH INFO:\n"
.LC10:
	.string	"Device Capacity: %d MB\n"
.LC11:
	.string	"FTL INFO:\n"
.LC12:
	.string	"g_MaxLpn = 0x%x\n"
.LC13:
	.string	"g_VaildLpn = 0x%x\n"
.LC14:
	.string	"read_page_count = 0x%x\n"
.LC15:
	.string	"discard_page_count = 0x%x\n"
.LC16:
	.string	"write_page_count = 0x%x\n"
.LC17:
	.string	"cache_write_count = 0x%x\n"
.LC18:
	.string	"l2p_write_count = 0x%x\n"
.LC19:
	.string	"gc_page_count = 0x%x\n"
.LC20:
	.string	"totle_write = %d MB\n"
.LC21:
	.string	"totle_read = %d MB\n"
.LC22:
	.string	"GSV = 0x%x\n"
.LC23:
	.string	"GDV = 0x%x\n"
.LC24:
	.string	"bad blk num = %d\n"
.LC25:
	.string	"free_superblocks = 0x%x\n"
.LC26:
	.string	"mlc_EC = 0x%x\n"
.LC27:
	.string	"slc_EC = 0x%x\n"
.LC28:
	.string	"avg_EC = 0x%x\n"
.LC29:
	.string	"sys_EC = 0x%x\n"
.LC30:
	.string	"max_EC = 0x%x\n"
.LC31:
	.string	"min_EC = 0x%x\n"
.LC32:
	.string	"PLT = 0x%x\n"
.LC33:
	.string	"POT = 0x%x\n"
.LC34:
	.string	"MaxSector = 0x%x\n"
.LC35:
	.string	"init_sys_blks_pp = 0x%x\n"
.LC36:
	.string	"sys_blks_pp = 0x%x\n"
.LC37:
	.string	"free sysblock = 0x%x\n"
.LC38:
	.string	"data_blks_pp = 0x%x\n"
.LC39:
	.string	"data_op_blks_pp = 0x%x\n"
.LC40:
	.string	"max_data_blks = 0x%x\n"
.LC41:
	.string	"Sys.id = 0x%x\n"
.LC42:
	.string	"Bbt.id = 0x%x\n"
.LC43:
	.string	"ACT.page = 0x%x\n"
.LC44:
	.string	"ACT.plane = 0x%x\n"
.LC45:
	.string	"ACT.id = 0x%x\n"
.LC46:
	.string	"ACT.mode = 0x%x\n"
.LC47:
	.string	"ACT.a_pages = 0x%x\n"
.LC48:
	.string	"ACT VPC = 0x%x\n"
.LC49:
	.string	"BUF.page = 0x%x\n"
.LC50:
	.string	"BUF.plane = 0x%x\n"
.LC51:
	.string	"BUF.id = 0x%x\n"
.LC52:
	.string	"BUF.mode = 0x%x\n"
.LC53:
	.string	"BUF.a_pages = 0x%x\n"
.LC54:
	.string	"BUF VPC = 0x%x\n"
.LC55:
	.string	"TMP.page = 0x%x\n"
.LC56:
	.string	"TMP.plane = 0x%x\n"
.LC57:
	.string	"TMP.id = 0x%x\n"
.LC58:
	.string	"TMP.mode = 0x%x\n"
.LC59:
	.string	"TMP.a_pages = 0x%x\n"
.LC60:
	.string	"GC.page = 0x%x\n"
.LC61:
	.string	"GC.plane = 0x%x\n"
.LC62:
	.string	"GC.id = 0x%x\n"
.LC63:
	.string	"GC.mode = 0x%x\n"
.LC64:
	.string	"GC.a_pages = 0x%x\n"
.LC65:
	.string	"WR_CHK = %x %x %x\n"
.LC66:
	.string	"Read Err Cnt = 0x%x\n"
.LC67:
	.string	"Prog Err Cnt = 0x%x\n"
.LC68:
	.string	"gc_free_blk_th= 0x%x\n"
.LC69:
	.string	"gc_merge_free_blk_th= 0x%x\n"
.LC70:
	.string	"gc_skip_write_count= 0x%x\n"
.LC71:
	.string	"gc_blk_index= 0x%x\n"
.LC72:
	.string	"free min EC= 0x%x\n"
.LC73:
	.string	"free max EC= 0x%x\n"
.LC74:
	.string	"GC__SB VPC = 0x%x\n"
.LC75:
	.string	"%d. [0x%x]=0x%x 0x%x  0x%x\n"
.LC76:
	.string	"free %d. [0x%x] 0x%x  0x%x\n"
.LC77:
	.string	"%s\n"
.LC78:
	.string	"swblk %x ,avg = %x max= %x vpc= %x,ec=%x ,max ec=%x\n"
.LC79:
	.string	"FtlGcRefreshBlock  0x%x\n"
.LC80:
	.string	"FtlGcMarkBadPhyBlk %d 0x%x\n"
.LC81:
	.string	"%s %p + 0x%x:"
.LC82:
	.string	"0x%08x,"
.LC83:
	.string	"0x%04x,"
.LC84:
	.string	"0x%02x,"
.LC85:
	.string	"\n"
.LC86:
	.string	"%s: addr: %x is in id block!!!!!!!!!!\n"
.LC87:
	.string	"not free: w: d:"
.LC88:
	.string	"not free: w: s:"
.LC89:
	.string	"FtlFreeSysBlkQueueOut = %x, free count = %d, error\n"
.LC90:
	.string	"FtlFreeSysBlkQueueOut = %x, free count = %d\n"
.LC91:
	.string	"%s error allocating memory. return -1\n"
.LC92:
	.string	"FlashReadPages %x %x error_ecc_bits %d\n"
.LC93:
	.string	"data:"
.LC94:
	.string	"spare:"
.LC95:
	.string	"prog read error: = %x\n"
.LC96:
	.string	"prog read REFRESH: = %x\n"
.LC97:
	.string	"prog read s error: = %x %x %x %x %x\n"
.LC98:
	.string	"prog read d error: = %x %x %x %x %x\n"
.LC99:
	.string	"FtlBbmTblFlush id=%x,page=%x,previd=%x cnt=%d\n"
.LC100:
	.string	"FtlBbmTblFlush error:%x\n"
.LC101:
	.string	"FtlBbmTblFlush error = %x error count = %d\n"
.LC102:
	.string	"FtlGcFreeBadSuperBlk 0x%x\n"
.LC103:
	.string	"decrement_vpc_count %x = %d\n"
.LC104:
	.string	"FtlMapWritePage error = %x \n"
.LC105:
	.string	"FtlMapWritePage error = %x error count = %d\n"
.LC106:
	.string	"region_id = %x phyAddr = %x\n"
.LC107:
	.string	"map_ppn:"
.LC108:
	.string	"load_l2p_region refresh = %x phyAddr = %x\n"
.LC109:
	.string	"data prev_ppa = %x error...................\n"
.LC110:
	.string	"spuer block %x vpn is 0\n "
.LC111:
	.string	"...%s enter...\n"
.LC112:
	.string	"FtlCheckVpc2 %x = %x  %x\n"
.LC113:
	.string	"free blk vpc error %x = %x  %x\n"
.LC114:
	.string	"ftl_scan_all_data = %x\n"
.LC115:
	.string	"scan lpa = %x ppa= %x\n"
.LC116:
	.string	"lpa = %x,addr= %x,spare= %x %x %x %x data=%x %x\n"
.LC117:
	.string	"FtlGcScanTempBlk Error ID %x %x!!!!!!! \n"
.LC118:
	.string	"FtlVendorPartRead refresh = %x phyAddr = %x\n"
.LC119:
	.string	"no ect"
.LC120:
	.string	"FtlVpcTblFlush error = %x error count = %d\n"
.LC121:
	.string	"FtlCheckVpc %x = %x  %x\n"
.LC122:
	.string	"FtlProgPages error %x = %d\n"
.LC123:
	.string	"Ftlwrite decrement_vpc_count %x = %d\n"
.LC124:
	.string	"SWL %x, FSB = %x vpc= %x,ec=%x th=%x\n"
.LC125:
	.string	"FtlWrite: ecc error:%x %x %x\n"
.LC126:
	.string	"FtlWrite: lpa error:%x %x\n"
.LC127:
	.string	"%s hash error this.id =%x page =%x pre_id =%x hash =%x hash_r =%x\n"
.LC128:
	.string	"%s last blk_id =%x page =%x hash error hash =%x hash_r =%x\n"
.LC129:
	.string	"%s scan blk_id =%x page =%x hash error hash =%x hash_r =%x\n"
.LC130:
	.string	"write_idblock %x %x\n"
.LC131:
	.string	"write_idblock fail! %x %x %x %x\n"
.LC132:
	.string	"%s idb buffer alloc fail\n"
.LC133:
	.string	"copy_from_user error %d %p %p\n"
.LC134:
	.string	"vendor_storage"
