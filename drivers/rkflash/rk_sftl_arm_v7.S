/* SPDX-License-Identifier: GPL-2.0 */

/* Copyright (c) 2018 Rockchip Electronics Co. Ltd.
 * date: 2022-08-15
 */
	.arch armv7-a
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 2
	.eabi_attribute 30, 4
	.eabi_attribute 34, 1
	.eabi_attribute 18, 2
	.file	"rk_sftl.c"
	.syntax divided
	.syntax unified
	.global	__aeabi_uidiv
	.arm
	.syntax unified
	.text
	.align	2
	.syntax unified
	.arm
	.fpu softvfp
	.type	l2p_addr_tran, %function
l2p_addr_tran:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L4
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r8, r1
	ldr	r6, [r0, #4]
	mov	r9, r2
	ldrh	r4, [r3, #8]
	ldrh	r5, [r3, #10]
	ldrh	r3, [r3, #14]
	lsr	r7, r6, #10
	ubfx	r6, r6, #0, #10
	uxth	r0, r7
	uxth	r7, r7
	cmp	r3, #4
	lsreq	r4, r4, #1
	lsleq	r5, r5, #1
	mov	r1, r4
	uxtheq	r5, r5
	bl	__aeabi_uidiv
	uxth	r0, r0
	mls	r4, r0, r4, r7
	mla	r4, r5, r4, r6
	str	r4, [r8]
	str	r0, [r9]
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L5:
	.align	2
.L4:
	.word	.LANCHOR0
	.fnend
	.size	l2p_addr_tran, .-l2p_addr_tran
	.align	2
	.global	ftl_print_sblk_info
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_print_sblk_info, %function
ftl_print_sblk_info:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r3, r4, lr}
	.save {r4, lr}
	.pad #16
	ldr	r4, .L8
	ldr	r1, .L8+4
	ldr	r0, .L8+8
	bl	sftl_printk
	ldrh	r1, [r4, #24]
	ldr	ip, [r4, #72]
	ldrb	r3, [r4, #30]	@ zero_extendqisi2
	ldrh	r2, [r4, #26]
	lsl	r0, r1, #1
	ldrh	r0, [ip, r0]
	str	r0, [sp, #8]
	ldrh	r0, [r4, #28]
	str	r0, [sp, #4]
	ldrb	r0, [r4, #32]	@ zero_extendqisi2
	str	r0, [sp]
	ldr	r0, .L8+12
	bl	sftl_printk
	ldrh	r1, [r4, #76]
	ldr	ip, [r4, #72]
	ldrb	r3, [r4, #82]	@ zero_extendqisi2
	ldrh	r2, [r4, #78]
	lsl	r0, r1, #1
	ldrh	r0, [ip, r0]
	str	r0, [sp, #8]
	ldrh	r0, [r4, #80]
	str	r0, [sp, #4]
	ldrb	r0, [r4, #84]	@ zero_extendqisi2
	str	r0, [sp]
	ldr	r0, .L8+16
	bl	sftl_printk
	ldrh	r1, [r4, #124]
	ldr	ip, [r4, #72]
	ldrb	r3, [r4, #130]	@ zero_extendqisi2
	ldrh	r2, [r4, #126]
	lsl	r0, r1, #1
	ldrh	r0, [ip, r0]
	str	r0, [sp, #8]
	ldrh	r0, [r4, #128]
	str	r0, [sp, #4]
	ldrb	r0, [r4, #132]	@ zero_extendqisi2
	str	r0, [sp]
	ldr	r0, .L8+20
	bl	sftl_printk
	ldrh	r1, [r4, #172]
	ldr	ip, [r4, #72]
	ldrb	r3, [r4, #178]	@ zero_extendqisi2
	ldrh	r2, [r4, #174]
	lsl	r0, r1, #1
	ldrh	r0, [ip, r0]
	str	r0, [sp, #8]
	ldrh	r0, [r4, #176]
	str	r0, [sp, #4]
	ldrb	r0, [r4, #180]	@ zero_extendqisi2
	str	r0, [sp]
	ldr	r0, .L8+24
	bl	sftl_printk
	ldrh	r3, [r4, #220]
	ldrh	r2, [r4, #222]
	ldrh	r1, [r4, #224]
	ldr	r0, .L8+28
	add	sp, sp, #16
	@ sp needed
	pop	{r4, lr}
	b	sftl_printk
.L9:
	.align	2
.L8:
	.word	.LANCHOR0
	.word	.LC0
	.word	.LC1
	.word	.LC2
	.word	.LC3
	.word	.LC4
	.word	.LC5
	.word	.LC6
	.fnend
	.size	ftl_print_sblk_info, .-ftl_print_sblk_info
	.align	2
	.global	Ftl_log2
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_log2, %function
Ftl_log2:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r1, #0
	mov	r2, #1
.L11:
	cmp	r2, r0
	uxth	r3, r1
	add	r1, r1, #1
	bls	.L12
	sub	r0, r3, #1
	uxth	r0, r0
	bx	lr
.L12:
	lsl	r2, r2, #1
	b	.L11
	.fnend
	.size	Ftl_log2, .-Ftl_log2
	.align	2
	.global	FtlPrintInfo
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlPrintInfo, %function
FtlPrintInfo:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.fnend
	.size	FtlPrintInfo, .-FtlPrintInfo
	.align	2
	.global	FtlSysBlkNumInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlSysBlkNumInit, %function
FtlSysBlkNumInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L15
	cmp	r0, #24
	movcc	r0, #24
	ldrh	r2, [r3, #232]
	ldrh	r1, [r3, #242]
	str	r0, [r3, #228]
	mul	r2, r0, r2
	sub	r0, r1, r0
	ldr	r1, [r3, #248]
	strh	r0, [r3, #240]	@ movhi
	mov	r0, #0
	str	r2, [r3, #236]
	sub	r2, r1, r2
	str	r2, [r3, #244]
	bx	lr
.L16:
	.align	2
.L15:
	.word	.LANCHOR0
	.fnend
	.size	FtlSysBlkNumInit, .-FtlSysBlkNumInit
	.global	__aeabi_idiv
	.align	2
	.global	FtlConstantsInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlConstantsInit, %function
FtlConstantsInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	mov	r3, r0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	ldrh	ip, [r3, #14]
	movw	r5, #258
	ldr	r4, .L32
	ldrh	r0, [r0]
	cmp	ip, #4
	ldrh	r7, [r3, #2]
	ldrh	lr, [r3, #4]
	add	r1, r4, #256
	ldrh	r2, [r3, #6]
	strh	r0, [r4, #252]	@ movhi
	strh	r7, [r4, #254]	@ movhi
	strh	lr, [r1]	@ movhi
	strh	r2, [r4, #242]	@ movhi
	strh	ip, [r4, r5]	@ movhi
	bne	.L18
	lsr	r2, r2, #1
	strh	r2, [r4, #242]	@ movhi
	mov	r2, #8
	strh	r2, [r4, r5]	@ movhi
.L19:
	ldr	ip, .L32+4
	mov	r2, #0
.L20:
	strb	r2, [r2, ip]
	add	r2, r2, #1
	cmp	r2, #32
	bne	.L20
	ldr	r2, .L32+8
	cmp	r0, #1
	mov	ip, #5
	ldrh	r5, [r4, #242]
	ldr	r8, .L32+12
	mov	lr, #0
	strh	ip, [r2]	@ movhi
	movw	ip, #294
	strheq	r0, [r2]	@ movhi
	mov	r2, #640
	ldrh	r0, [r1]
	mov	r6, r8
	strh	r2, [r6], #4	@ movhi
	movw	r2, #298
	strh	lr, [r4, ip]	@ movhi
	str	r3, [sp, #4]
	smulbb	r7, r7, r0
	smulbb	r0, r0, r5
	uxth	r7, r7
	uxth	r0, r0
	strh	r7, [r4, #232]	@ movhi
	strh	r0, [r4, r2]	@ movhi
	bl	Ftl_log2
	ldr	r3, [sp, #4]
	movw	r2, #302
	strh	r0, [r6]	@ movhi
	mov	r6, r8
	add	r8, r8, #24
	ldrh	r10, [r3, #12]
	strh	r10, [r4, r2]	@ movhi
	movw	r2, #306
	smulbb	r1, r7, r10
	strh	r10, [r8, #-16]	@ movhi
	strh	r1, [r4, r2]	@ movhi
	movw	r2, #258
	ldrh	fp, [r4, r2]
	mov	r0, fp
	bl	Ftl_log2
	lsl	r2, fp, #9
	ldr	r3, [sp, #4]
	movw	r1, #310
	strh	r0, [r6, #12]!	@ movhi
	uxth	r2, r2
	mov	r9, r0
	mov	r0, #5120
	strh	r2, [r4, r1]	@ movhi
	lsr	r2, r2, #8
	mul	r1, r10, fp
	strh	r2, [r8, #-8]	@ movhi
	ldrh	r2, [r3, #20]
	movw	r3, #314
	strh	r2, [r4, r3]	@ movhi
	mul	r3, r5, r7
	lsl	r5, r5, #6
	str	r3, [r4, #248]
	mul	r3, fp, r3
	mul	r3, r10, r3
	asr	r3, r3, #11
	str	r3, [r4, #316]
	bl	__aeabi_idiv
	uxth	r0, r0
	movw	r2, #322
	mov	r1, r7
	cmp	r0, #4
	movls	r3, #4
	strhhi	r0, [r8]	@ movhi
	strhls	r3, [r8]	@ movhi
	mov	r3, #640
	asr	r3, r3, r9
	add	r9, r9, #9
	asr	r5, r5, r9
	ldrh	r0, [r8]
	add	r3, r3, #2
	strh	r3, [r4, r2]	@ movhi
	ldr	r3, .L32+16
	strh	r5, [r3]	@ movhi
	uxth	r5, r5
	mul	r3, r7, r5
	add	r5, r5, #8
	str	r3, [r4, #328]
	bl	__aeabi_uidiv
	uxtah	r0, r5, r0
	cmp	r7, #1
	addeq	r0, r0, #4
	str	r0, [r4, #228]
	ldrh	r0, [r4, #228]
	bl	FtlSysBlkNumInit
	ldr	r3, [r4, #228]
	mov	r0, #0
	str	r0, [r4, #340]
	str	r3, [r4, #332]
	ldr	r3, [r4, #244]
	lsl	r2, r3, #2
	movw	r3, #302
	ldrh	r3, [r4, r3]
	mul	r3, r3, r2
	ldrh	r2, [r6]
	add	r2, r2, #9
	lsr	r3, r3, r2
	ldr	r2, .L32+20
	add	r3, r3, #2
	strh	r3, [r2]	@ movhi
	mov	r2, #32
	movw	r3, #338
	strh	r2, [r4, r3]	@ movhi
	ldrh	r3, [r8]
	add	r3, r3, #3
	strh	r3, [r8]	@ movhi
	ldr	r3, [r4, #328]
	add	r3, r3, #3
	str	r3, [r4, #328]
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L18:
	cmp	lr, #1
	cmpeq	ip, #8
	lsreq	r2, r2, #1
	strheq	r2, [r4, #242]	@ movhi
	moveq	r2, #2
	strheq	r2, [r1]	@ movhi
	b	.L19
.L33:
	.align	2
.L32:
	.word	.LANCHOR0
	.word	.LANCHOR0+260
	.word	.LANCHOR0+292
	.word	.LANCHOR0+296
	.word	.LANCHOR0+324
	.word	.LANCHOR0+336
	.fnend
	.size	FtlConstantsInit, .-FtlConstantsInit
	.align	2
	.global	IsBlkInVendorPart
	.syntax unified
	.arm
	.fpu softvfp
	.type	IsBlkInVendorPart, %function
IsBlkInVendorPart:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L41
	add	r3, r2, #344
	ldrh	r3, [r3]
	cmp	r3, #0
	beq	.L40
	ldr	r3, [r2, #348]
	add	r2, r2, #320
	ldrh	r2, [r2]
	add	r2, r3, r2, lsl #1
.L36:
	cmp	r3, r2
	bne	.L37
.L40:
	mov	r0, #0
	bx	lr
.L37:
	ldrh	r1, [r3], #2
	cmp	r0, r1
	bne	.L36
	mov	r0, #1
	bx	lr
.L42:
	.align	2
.L41:
	.word	.LANCHOR0
	.fnend
	.size	IsBlkInVendorPart, .-IsBlkInVendorPart
	.align	2
	.global	FtlCacheWriteBack
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlCacheWriteBack, %function
FtlCacheWriteBack:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r0, #0
	bx	lr
	.fnend
	.size	FtlCacheWriteBack, .-FtlCacheWriteBack
	.align	2
	.global	sftl_get_density
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_get_density, %function
sftl_get_density:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L45
	ldr	r0, [r3, #340]
	bx	lr
.L46:
	.align	2
.L45:
	.word	.LANCHOR0
	.fnend
	.size	sftl_get_density, .-sftl_get_density
	.global	__aeabi_uidivmod
	.align	2
	.global	FtlBbmMapBadBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbmMapBadBlock, %function
FtlBbmMapBadBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, lr}
	.save {r4, r5, r6, r7, lr}
	.pad #12
	movw	r3, #298
	ldr	r4, .L49
	mov	r5, r0
	ldrh	r7, [r4, r3]
	mov	r1, r7
	bl	__aeabi_uidiv
	uxth	r6, r0
	mov	r1, r7
	mov	r0, r5
	bl	__aeabi_uidivmod
	add	r2, r4, r6, lsl #2
	uxth	r3, r1
	ldr	r2, [r2, #380]
	lsr	r1, r3, #5
	and	ip, r3, #31
	mov	lr, #1
	add	r4, r4, #352
	ldr	r0, [r2, r1, lsl #2]
	orr	r0, r0, lr, lsl ip
	str	r0, [r2, r1, lsl #2]
	mov	r2, r6
	str	r0, [sp]
	mov	r1, r5
	ldr	r0, .L49+4
	bl	sftl_printk
	ldrh	r3, [r4, #6]
	mov	r0, #0
	add	r3, r3, #1
	strh	r3, [r4, #6]	@ movhi
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, pc}
.L50:
	.align	2
.L49:
	.word	.LANCHOR0
	.word	.LC7
	.fnend
	.size	FtlBbmMapBadBlock, .-FtlBbmMapBadBlock
	.align	2
	.global	FtlBbmIsBadBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbmIsBadBlock, %function
FtlBbmIsBadBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	movw	r3, #298
	ldr	r5, .L53
	mov	r7, r0
	ldrh	r6, [r5, r3]
	mov	r1, r6
	bl	__aeabi_uidivmod
	mov	r0, r7
	uxth	r4, r1
	mov	r1, r6
	bl	__aeabi_uidiv
	uxth	r0, r0
	lsr	r2, r4, #5
	add	r5, r5, r0, lsl #2
	and	r4, r4, #31
	ldr	r3, [r5, #380]
	ldr	r0, [r3, r2, lsl #2]
	lsr	r0, r0, r4
	and	r0, r0, #1
	pop	{r4, r5, r6, r7, r8, pc}
.L54:
	.align	2
.L53:
	.word	.LANCHOR0
	.fnend
	.size	FtlBbmIsBadBlock, .-FtlBbmIsBadBlock
	.align	2
	.global	FtlBbtInfoPrint
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbtInfoPrint, %function
FtlBbtInfoPrint:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	bx	lr
	.fnend
	.size	FtlBbtInfoPrint, .-FtlBbtInfoPrint
	.align	2
	.global	FtlBbtCalcTotleCnt
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbtCalcTotleCnt, %function
FtlBbtCalcTotleCnt:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L64
	movw	r2, #298
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r5, #0
	mov	r4, r5
	ldrh	r2, [r3, r2]
	ldrh	r6, [r3, #254]
	mul	r6, r6, r2
.L57:
	uxth	r0, r5
	cmp	r0, r6
	blt	.L59
	mov	r0, r4
	pop	{r4, r5, r6, pc}
.L59:
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	add	r5, r5, #1
	addne	r4, r4, #1
	uxthne	r4, r4
	b	.L57
.L65:
	.align	2
.L64:
	.word	.LANCHOR0
	.fnend
	.size	FtlBbtCalcTotleCnt, .-FtlBbtCalcTotleCnt
	.align	2
	.global	V2P_block
	.syntax unified
	.arm
	.fpu softvfp
	.type	V2P_block, %function
V2P_block:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r5, r1
	ldr	r4, .L68
	mov	r7, r0
	add	r3, r4, #256
	ldrh	r6, [r3]
	mov	r1, r6
	bl	__aeabi_uidiv
	movw	r3, #298
	smulbb	r5, r6, r5
	ldrh	r4, [r4, r3]
	mov	r1, r6
	smulbb	r4, r4, r0
	mov	r0, r7
	bl	__aeabi_uidivmod
	add	r0, r5, r1
	add	r0, r4, r0
	uxth	r0, r0
	pop	{r4, r5, r6, r7, r8, pc}
.L69:
	.align	2
.L68:
	.word	.LANCHOR0
	.fnend
	.size	V2P_block, .-V2P_block
	.align	2
	.global	P2V_plane
	.syntax unified
	.arm
	.fpu softvfp
	.type	P2V_plane, %function
P2V_plane:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L72
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r6, r0
	add	r2, r3, #256
	ldrh	r5, [r2]
	movw	r2, #298
	ldrh	r1, [r3, r2]
	bl	__aeabi_uidiv
	mov	r1, r5
	smulbb	r4, r0, r5
	mov	r0, r6
	bl	__aeabi_uidivmod
	add	r1, r4, r1
	uxth	r0, r1
	pop	{r4, r5, r6, pc}
.L73:
	.align	2
.L72:
	.word	.LANCHOR0
	.fnend
	.size	P2V_plane, .-P2V_plane
	.align	2
	.global	P2V_block_in_plane
	.syntax unified
	.arm
	.fpu softvfp
	.type	P2V_block_in_plane, %function
P2V_block_in_plane:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	movw	r3, #298
	ldr	r4, .L76
	ldrh	r1, [r4, r3]
	add	r4, r4, #256
	bl	__aeabi_uidivmod
	uxth	r0, r1
	ldrh	r1, [r4]
	bl	__aeabi_uidiv
	uxth	r0, r0
	pop	{r4, pc}
.L77:
	.align	2
.L76:
	.word	.LANCHOR0
	.fnend
	.size	P2V_block_in_plane, .-P2V_block_in_plane
	.align	2
	.global	ftl_cmp_data_ver
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_cmp_data_ver, %function
ftl_cmp_data_ver:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	cmp	r0, r1
	bls	.L79
	sub	r0, r0, r1
	cmp	r0, #-2147483648
	movhi	r0, #0
	movls	r0, #1
	bx	lr
.L79:
	sub	r0, r1, r0
	cmp	r0, #-2147483648
	movls	r0, #0
	movhi	r0, #1
	bx	lr
	.fnend
	.size	ftl_cmp_data_ver, .-ftl_cmp_data_ver
	.align	2
	.global	FtlFreeSysBlkQueueEmpty
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBlkQueueEmpty, %function
FtlFreeSysBlkQueueEmpty:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L82
	ldrh	r0, [r3, #6]
	clz	r0, r0
	lsr	r0, r0, #5
	bx	lr
.L83:
	.align	2
.L82:
	.word	.LANCHOR0+412
	.fnend
	.size	FtlFreeSysBlkQueueEmpty, .-FtlFreeSysBlkQueueEmpty
	.align	2
	.global	FtlFreeSysBlkQueueFull
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBlkQueueFull, %function
FtlFreeSysBlkQueueFull:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L85
	ldrh	r0, [r3, #6]
	sub	r0, r0, #1024
	clz	r0, r0
	lsr	r0, r0, #5
	bx	lr
.L86:
	.align	2
.L85:
	.word	.LANCHOR0+412
	.fnend
	.size	FtlFreeSysBlkQueueFull, .-FtlFreeSysBlkQueueFull
	.align	2
	.global	FtlFreeSysBLkSort
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBLkSort, %function
FtlFreeSysBLkSort:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L100
	ldrh	r2, [r3, #6]
	cmp	r2, #0
	bxeq	lr
	ldr	r2, .L100+4
	mov	r0, #0
	push	{r4, lr}
	.save {r4, lr}
	mov	r4, r0
	ldrh	r1, [r3, #2]
	ldrh	lr, [r2, #28]
	ldrh	r2, [r3, #4]
	and	lr, lr, #31
.L89:
	uxth	ip, r0
	add	r0, r0, #1
	cmp	lr, ip
	bgt	.L90
	cmp	r4, #0
	strhne	r1, [r3, #2]	@ movhi
	strhne	r2, [r3, #4]	@ movhi
	pop	{r4, pc}
.L90:
	add	ip, r3, r1, lsl #1
	add	r1, r1, #1
	ubfx	r1, r1, #0, #10
	ldrh	r4, [ip, #8]
	add	ip, r3, r2, lsl #1
	strh	r4, [ip, #8]	@ movhi
	mov	r4, #1
	add	r2, r2, r4
	ubfx	r2, r2, #0, #10
	b	.L89
.L101:
	.align	2
.L100:
	.word	.LANCHOR0+412
	.word	.LANCHOR0+2468
	.fnend
	.size	FtlFreeSysBLkSort, .-FtlFreeSysBLkSort
	.align	2
	.global	IsInFreeQueue
	.syntax unified
	.arm
	.fpu softvfp
	.type	IsInFreeQueue, %function
IsInFreeQueue:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L112
	ldrh	ip, [r3, #6]
	cmp	ip, #1024
	beq	.L106
	str	lr, [sp, #-4]!
	.save {lr}
	mov	r1, #0
	ldrh	lr, [r3, #2]
.L104:
	cmp	r1, ip
	bcc	.L105
	mov	r0, #0
	ldr	pc, [sp], #4
.L105:
	add	r2, r1, lr
	ubfx	r2, r2, #0, #10
	add	r2, r3, r2, lsl #1
	ldrh	r2, [r2, #8]
	cmp	r2, r0
	beq	.L107
	add	r1, r1, #1
	b	.L104
.L106:
	mov	r0, #0
	bx	lr
.L107:
	mov	r0, #1
	ldr	pc, [sp], #4
.L113:
	.align	2
.L112:
	.word	.LANCHOR0+412
	.fnend
	.size	IsInFreeQueue, .-IsInFreeQueue
	.align	2
	.global	insert_data_list
	.syntax unified
	.arm
	.fpu softvfp
	.type	insert_data_list, %function
insert_data_list:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	ldr	r2, .L129
	ldrh	r3, [r2, #240]
	cmp	r3, r0
	bls	.L116
	mov	r4, #6
	ldr	r7, [r2, #2516]
	mul	r4, r4, r0
	mvn	lr, #0
	add	ip, r7, r4
	strh	lr, [ip, #2]	@ movhi
	strh	lr, [r7, r4]	@ movhi
	ldr	r3, [r2, #2520]
	cmp	r3, #0
	streq	ip, [r2, #2520]
	beq	.L116
	ldr	r9, [r2, #72]
	mov	r1, r2
	lsl	r2, r0, #1
	ldr	r8, [r1, #2516]
	movw	r10, #65535
	ldrh	r5, [r9, r2]
	ldrh	r2, [ip, #4]
	cmp	r2, #0
	mulne	lr, r2, r5
	sub	r2, r3, r8
	asr	r5, r2, #1
	ldr	r2, .L129+4
	mul	r2, r2, r5
	ldrh	r5, [r1, #240]
	str	r5, [sp, #4]
	mov	r5, #0
	uxth	r2, r2
.L122:
	ldr	r6, [sp, #4]
	add	r5, r5, #1
	uxth	r5, r5
	cmp	r5, r6
	movls	r6, #0
	movhi	r6, #1
	cmp	r0, r2
	orreq	r6, r6, #1
	cmp	r6, #0
	bne	.L116
	ldrh	r6, [r3, #4]
	cmp	r6, #0
	beq	.L120
	lsl	fp, r2, #1
	ldrh	fp, [r9, fp]
	mul	r6, r6, fp
	cmp	r6, lr
	bcs	.L120
	ldrh	r6, [r3]
	cmp	r6, r10
	bne	.L121
	strh	r2, [ip, #2]	@ movhi
	strh	r0, [r3]	@ movhi
	str	ip, [r1, #2524]
.L116:
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L121:
	mov	r3, #6
	mov	r2, r6
	mla	r3, r3, r6, r8
	b	.L122
.L120:
	strh	r2, [r7, r4]	@ movhi
	ldrh	r2, [r3, #2]
	strh	r2, [ip, #2]	@ movhi
	ldr	r2, [r1, #2520]
	cmp	r3, r2
	ldrhne	ip, [r3, #2]
	movne	r2, #6
	ldrne	r1, [r1, #2516]
	strheq	r0, [r3, #2]	@ movhi
	streq	ip, [r1, #2520]
	mulne	r2, r2, ip
	strhne	r0, [r1, r2]	@ movhi
	strhne	r0, [r3, #2]	@ movhi
	b	.L116
.L130:
	.align	2
.L129:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	insert_data_list, .-insert_data_list
	.align	2
	.global	INSERT_DATA_LIST
	.syntax unified
	.arm
	.fpu softvfp
	.type	INSERT_DATA_LIST, %function
INSERT_DATA_LIST:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	bl	insert_data_list
	ldr	r2, .L134
	add	r1, r2, #2528
	ldrh	r2, [r2, #240]
	ldrh	r3, [r1]
	add	r3, r3, #1
	uxth	r3, r3
	cmp	r2, r3
	strh	r3, [r1]	@ movhi
	popcs	{r4, pc}
	mov	r2, #214
	ldr	r1, .L134+4
	ldr	r0, .L134+8
	pop	{r4, lr}
	b	sftl_printk
.L135:
	.align	2
.L134:
	.word	.LANCHOR0
	.word	.LANCHOR1
	.word	.LC8
	.fnend
	.size	INSERT_DATA_LIST, .-INSERT_DATA_LIST
	.align	2
	.global	insert_free_list
	.syntax unified
	.arm
	.fpu softvfp
	.type	insert_free_list, %function
insert_free_list:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	movw	r4, #65535
	cmp	r0, r4
	beq	.L137
	ldr	r2, .L144
	mov	r1, #6
	mul	r5, r1, r0
	mvn	r3, #0
	ldr	r6, [r2, #2516]
	mov	ip, r2
	add	lr, r6, r5
	strh	r3, [lr, #2]	@ movhi
	strh	r3, [r6, r5]	@ movhi
	ldr	r3, [r2, #2532]
	cmp	r3, #0
	streq	lr, [r2, #2532]
	beq	.L137
	ldr	r8, [r2, #2536]
	lsl	r2, r0, #1
	ldr	r7, [ip, #2516]
	ldrh	r9, [r8, r2]
	sub	r2, r3, r7
	asr	r10, r2, #1
	ldr	r2, .L144+4
	mul	r2, r2, r10
	mov	r10, r1
	uxth	r2, r2
.L141:
	lsl	r1, r2, #1
	ldrh	r1, [r8, r1]
	cmp	r1, r9
	bcs	.L139
	ldrh	r1, [r3]
	cmp	r1, r4
	bne	.L140
	strh	r2, [lr, #2]	@ movhi
	strh	r0, [r3]	@ movhi
.L137:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L140:
	mla	r3, r10, r1, r7
	mov	r2, r1
	b	.L141
.L139:
	ldrh	r1, [r3, #2]
	strh	r1, [lr, #2]	@ movhi
	strh	r2, [r6, r5]	@ movhi
	ldr	r2, [ip, #2532]
	cmp	r3, r2
	ldrhne	lr, [r3, #2]
	movne	r2, #6
	ldrne	r1, [ip, #2516]
	strheq	r0, [r3, #2]	@ movhi
	streq	lr, [ip, #2532]
	mulne	r2, r2, lr
	strhne	r0, [r1, r2]	@ movhi
	strhne	r0, [r3, #2]	@ movhi
	b	.L137
.L145:
	.align	2
.L144:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	insert_free_list, .-insert_free_list
	.align	2
	.global	INSERT_FREE_LIST
	.syntax unified
	.arm
	.fpu softvfp
	.type	INSERT_FREE_LIST, %function
INSERT_FREE_LIST:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	bl	insert_free_list
	ldr	r2, .L149
	ldrh	r3, [r2, #224]
	add	r3, r3, #1
	uxth	r3, r3
	strh	r3, [r2, #224]	@ movhi
	ldrh	r2, [r2, #240]
	cmp	r2, r3
	popcs	{r4, pc}
	mov	r2, #207
	ldr	r1, .L149+4
	ldr	r0, .L149+8
	pop	{r4, lr}
	b	sftl_printk
.L150:
	.align	2
.L149:
	.word	.LANCHOR0
	.word	.LANCHOR1+17
	.word	.LC8
	.fnend
	.size	INSERT_FREE_LIST, .-INSERT_FREE_LIST
	.align	2
	.global	List_remove_node
	.syntax unified
	.arm
	.fpu softvfp
	.type	List_remove_node, %function
List_remove_node:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r6, #6
	ldr	r4, .L158
	mul	r6, r6, r1
	movw	r3, #65535
	mov	r8, r0
	ldr	r7, [r4, #2516]
	add	r5, r7, r6
	ldrh	r2, [r5, #2]
	cmp	r2, r3
	bne	.L152
	ldr	r3, [r0]
	cmp	r5, r3
	beq	.L152
	mov	r2, #372
	ldr	r1, .L158+4
	ldr	r0, .L158+8
	bl	sftl_printk
.L152:
	ldr	r3, [r8]
	movw	r2, #65535
	cmp	r5, r3
	ldrh	r3, [r7, r6]
	bne	.L153
	cmp	r3, r2
	ldrne	r2, [r4, #2516]
	movne	r1, #6
	moveq	r3, #0
	streq	r3, [r8]
	mlane	r3, r1, r3, r2
	mvnne	r2, #0
	strne	r3, [r8]
	strhne	r2, [r3, #2]	@ movhi
.L155:
	mvn	r3, #0
	mov	r0, #0
	strh	r3, [r7, r6]	@ movhi
	strh	r3, [r5, #2]	@ movhi
	pop	{r4, r5, r6, r7, r8, pc}
.L153:
	cmp	r3, r2
	ldrh	r1, [r5, #2]
	bne	.L156
	cmp	r1, r3
	movne	r3, #6
	ldrne	r2, [r4, #2516]
	mulne	r1, r3, r1
	mvnne	r3, #0
	strhne	r3, [r2, r1]	@ movhi
	b	.L155
.L156:
	ldr	r0, [r4, #2516]
	mov	r2, #6
	mla	r3, r2, r3, r0
	strh	r1, [r3, #2]	@ movhi
	ldrh	r0, [r5, #2]
	ldrh	r1, [r7, r6]
	ldr	r3, [r4, #2516]
	mul	r2, r2, r0
	strh	r1, [r3, r2]	@ movhi
	b	.L155
.L159:
	.align	2
.L158:
	.word	.LANCHOR0
	.word	.LANCHOR1+34
	.word	.LC8
	.fnend
	.size	List_remove_node, .-List_remove_node
	.align	2
	.global	List_pop_index_node
	.syntax unified
	.arm
	.fpu softvfp
	.type	List_pop_index_node, %function
List_pop_index_node:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, [r0]
	cmp	r3, #0
	beq	.L166
	ldr	r2, .L171
	push	{r4, lr}
	.save {r4, lr}
	movw	lr, #65535
	mov	r4, #6
	ldr	r2, [r2, #2516]
.L162:
	cmp	r1, #0
	bne	.L163
.L165:
	ldr	r4, .L171+4
	sub	r3, r3, r2
	asr	r3, r3, #1
	mul	r4, r4, r3
	uxth	r1, r4
	bl	List_remove_node
	uxth	r0, r4
	pop	{r4, pc}
.L163:
	ldrh	ip, [r3]
	cmp	ip, lr
	beq	.L165
	sub	r1, r1, #1
	mla	r3, r4, ip, r2
	uxth	r1, r1
	b	.L162
.L166:
	movw	r0, #65535
	bx	lr
.L172:
	.align	2
.L171:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	List_pop_index_node, .-List_pop_index_node
	.align	2
	.global	List_pop_head_node
	.syntax unified
	.arm
	.fpu softvfp
	.type	List_pop_head_node, %function
List_pop_head_node:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r1, #0
	b	List_pop_index_node
	.fnend
	.size	List_pop_head_node, .-List_pop_head_node
	.align	2
	.global	List_get_gc_head_node
	.syntax unified
	.arm
	.fpu softvfp
	.type	List_get_gc_head_node, %function
List_get_gc_head_node:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L180
	ldr	r3, [r2, #2520]
	cmp	r3, #0
	ldrne	r1, [r2, #2516]
	movne	ip, #6
	movwne	r2, #65535
	bne	.L176
.L179:
	movw	r0, #65535
	bx	lr
.L178:
	sub	r0, r0, #1
	mla	r3, ip, r3, r1
	uxth	r0, r0
.L176:
	cmp	r0, #0
	beq	.L177
	ldrh	r3, [r3]
	cmp	r3, r2
	bne	.L178
	b	.L179
.L177:
	ldr	r0, .L180+4
	sub	r3, r3, r1
	asr	r3, r3, #1
	mul	r3, r0, r3
	uxth	r0, r3
	bx	lr
.L181:
	.align	2
.L180:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	List_get_gc_head_node, .-List_get_gc_head_node
	.align	2
	.global	List_update_data_list
	.syntax unified
	.arm
	.fpu softvfp
	.type	List_update_data_list, %function
List_update_data_list:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	ldr	r4, .L194
	ldrh	r3, [r4, #24]
	cmp	r3, r0
	beq	.L184
	ldrh	r3, [r4, #76]
	cmp	r3, r0
	beq	.L184
	ldrh	r3, [r4, #124]
	cmp	r3, r0
	beq	.L184
	mov	r7, #6
	ldr	r9, [r4, #2516]
	mul	r7, r7, r0
	ldr	r3, [r4, #2520]
	add	r8, r9, r7
	cmp	r8, r3
	beq	.L184
	ldr	r2, [r4, #72]
	lsl	r3, r0, #1
	ldrh	r6, [r8, #4]
	mov	r5, r0
	ldrh	r3, [r2, r3]
	movw	r2, #65535
	cmp	r6, #0
	mvneq	r6, #0
	mulne	r6, r6, r3
	ldrh	r3, [r8, #2]
	cmp	r3, r2
	bne	.L187
	ldrh	r2, [r9, r7]
	cmp	r2, r3
	bne	.L187
	movw	r2, #463
	ldr	r1, .L194+4
	ldr	r0, .L194+8
	bl	sftl_printk
.L187:
	ldrh	r3, [r8, #2]
	movw	r2, #65535
	cmp	r3, r2
	bne	.L188
	ldrh	r2, [r9, r7]
	cmp	r2, r3
	beq	.L184
.L188:
	mov	r2, #6
	mul	r2, r2, r3
	ldr	r3, .L194+12
	asr	r1, r2, #1
	mul	r3, r3, r1
	ldr	r1, [r4, #72]
	lsl	r3, r3, #1
	ldrh	r0, [r1, r3]
	ldr	r1, [r4, #2516]
	add	r2, r1, r2
	ldrh	r3, [r2, #4]
	cmp	r3, #0
	mulne	r3, r3, r0
	mvneq	r3, #0
	cmp	r6, r3
	bcs	.L184
	ldr	r4, .L194+16
	mov	r1, r5
	ldr	r0, .L194+20
	bl	List_remove_node
	ldrh	r3, [r4]
	cmp	r3, #0
	bne	.L190
	movw	r2, #474
	ldr	r1, .L194+4
	ldr	r0, .L194+8
	bl	sftl_printk
.L190:
	ldrh	r3, [r4]
	mov	r0, r5
	sub	r3, r3, #1
	strh	r3, [r4]	@ movhi
	bl	INSERT_DATA_LIST
.L184:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L195:
	.align	2
.L194:
	.word	.LANCHOR0
	.word	.LANCHOR1+51
	.word	.LC8
	.word	-1431655765
	.word	.LANCHOR0+2528
	.word	.LANCHOR0+2520
	.fnend
	.size	List_update_data_list, .-List_update_data_list
	.align	2
	.global	select_l2p_ram_region
	.syntax unified
	.arm
	.fpu softvfp
	.type	select_l2p_ram_region, %function
select_l2p_ram_region:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L207
	movw	r2, #338
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r1, #0
	mov	r0, #12
	movw	ip, #65535
	ldrh	r2, [r3, r2]
	ldr	r3, [r3, #2540]
.L197:
	uxth	r4, r1
	cmp	r4, r2
	bcc	.L199
	mov	r4, r2
	mov	r1, #0
	mov	r0, #-2147483648
	mov	r5, #12
.L200:
	uxth	lr, r1
	cmp	lr, r2
	bcc	.L202
	cmp	r4, r2
	bcc	.L198
	ldr	r1, .L207+4
	mov	r4, r2
	mvn	r0, #0
	ldrh	r5, [r1]
	mov	r1, #0
.L203:
	uxth	ip, r1
	cmp	ip, r2
	bcc	.L205
	cmp	r4, r2
	bcc	.L198
	mov	r2, #796
	ldr	r1, .L207+8
	ldr	r0, .L207+12
	bl	sftl_printk
	b	.L198
.L199:
	add	r1, r1, #1
	mla	lr, r0, r1, r3
	ldrh	lr, [lr, #-12]
	cmp	lr, ip
	bne	.L197
.L198:
	mov	r0, r4
	pop	{r4, r5, r6, pc}
.L202:
	mla	ip, r5, r1, r3
	add	r1, r1, #1
	ldr	ip, [ip, #4]
	cmp	r0, ip
	movls	r6, #0
	movhi	r6, #1
	cmp	ip, #0
	movlt	r6, #0
	cmp	r6, #0
	movne	r0, ip
	movne	r4, lr
	b	.L200
.L205:
	ldr	lr, [r3, #4]
	cmp	r0, lr
	bls	.L204
	ldrh	r6, [r3]
	cmp	r6, r5
	movne	r0, lr
	movne	r4, ip
.L204:
	add	r1, r1, #1
	add	r3, r3, #12
	b	.L203
.L208:
	.align	2
.L207:
	.word	.LANCHOR0
	.word	.LANCHOR0+2544
	.word	.LANCHOR1+73
	.word	.LC8
	.fnend
	.size	select_l2p_ram_region, .-select_l2p_ram_region
	.align	2
	.global	FtlUpdateVaildLpn
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlUpdateVaildLpn, %function
FtlUpdateVaildLpn:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	ip, .L218
	movw	r1, #2546
	ldrh	r2, [ip, r1]
	cmp	r2, #4
	cmpls	r0, #0
	bne	.L210
	add	r2, r2, #1
	strh	r2, [ip, r1]	@ movhi
	bx	lr
.L210:
	mov	r2, #0
	str	lr, [sp, #-4]!
	.save {lr}
	strh	r2, [ip, r1]	@ movhi
	mov	r3, ip
	movw	lr, #65535
	str	r2, [ip, #2548]
	ldrh	r1, [ip, #240]
	ldr	r2, [ip, #72]
	add	r1, r2, r1, lsl #1
.L211:
	cmp	r2, r1
	bne	.L213
	ldr	pc, [sp], #4
.L213:
	ldrh	ip, [r2], #2
	cmp	ip, lr
	ldrne	r0, [r3, #2548]
	addne	r0, r0, ip
	strne	r0, [r3, #2548]
	b	.L211
.L219:
	.align	2
.L218:
	.word	.LANCHOR0
	.fnend
	.size	FtlUpdateVaildLpn, .-FtlUpdateVaildLpn
	.align	2
	.global	ftl_sb_update_avl_pages
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_sb_update_avl_pages, %function
ftl_sb_update_avl_pages:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	mov	r3, #0
	push	{r4, r5, lr}
	.save {r4, r5, lr}
	strh	r3, [r0, #4]	@ movhi
	add	ip, r0, r2, lsl #1
	movw	r5, #65535
	ldr	r3, .L228
	add	ip, ip, #14
	ldrh	lr, [r3, #232]
.L221:
	cmp	r2, lr
	bcc	.L223
	movw	r2, #302
	add	ip, r0, #16
	ldrh	r3, [r3, r2]
	movw	r4, #65535
	sub	r3, r3, #1
	sub	r1, r3, r1
	mov	r3, #0
	uxth	r1, r1
.L224:
	uxth	r2, r3
	cmp	lr, r2
	bhi	.L226
	pop	{r4, r5, pc}
.L223:
	ldrh	r4, [ip, #2]!
	add	r2, r2, #1
	uxth	r2, r2
	cmp	r4, r5
	ldrhne	r4, [r0, #4]
	addne	r4, r4, #1
	strhne	r4, [r0, #4]	@ movhi
	b	.L221
.L226:
	ldrh	r2, [ip], #2
	add	r3, r3, #1
	cmp	r2, r4
	ldrhne	r2, [r0, #4]
	addne	r2, r1, r2
	strhne	r2, [r0, #4]	@ movhi
	b	.L224
.L229:
	.align	2
.L228:
	.word	.LANCHOR0
	.fnend
	.size	ftl_sb_update_avl_pages, .-ftl_sb_update_avl_pages
	.align	2
	.global	FtlSlcSuperblockCheck
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlSlcSuperblockCheck, %function
FtlSlcSuperblockCheck:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldrh	r3, [r0, #4]
	cmp	r3, #0
	bxeq	lr
	ldrh	r2, [r0]
	movw	r3, #65535
	cmp	r2, r3
	bxeq	lr
	ldrb	r2, [r0, #6]	@ zero_extendqisi2
	str	lr, [sp, #-4]!
	.save {lr}
	mov	lr, #0
	add	r2, r0, r2, lsl #1
	ldrh	r1, [r2, #16]
	ldr	r2, .L240
	ldrh	ip, [r2, #232]
	mov	r2, r3
.L233:
	cmp	r1, r2
	beq	.L235
	ldr	pc, [sp], #4
.L235:
	ldrb	r3, [r0, #6]	@ zero_extendqisi2
	add	r3, r3, #1
	uxtb	r3, r3
	cmp	r3, ip
	strb	r3, [r0, #6]
	ldrheq	r3, [r0, #2]
	strbeq	lr, [r0, #6]
	addeq	r3, r3, #1
	strheq	r3, [r0, #2]	@ movhi
	ldrb	r3, [r0, #6]	@ zero_extendqisi2
	add	r3, r0, r3, lsl #1
	ldrh	r1, [r3, #16]
	b	.L233
.L241:
	.align	2
.L240:
	.word	.LANCHOR0
	.fnend
	.size	FtlSlcSuperblockCheck, .-FtlSlcSuperblockCheck
	.align	2
	.global	make_superblock
	.syntax unified
	.arm
	.fpu softvfp
	.type	make_superblock, %function
make_superblock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r4, r0
	ldr	r6, .L249
	ldrh	r2, [r0]
	ldrh	r3, [r6, #240]
	cmp	r2, r3
	bcc	.L243
	movw	r2, #2157
	ldr	r1, .L249+4
	ldr	r0, .L249+8
	bl	sftl_printk
.L243:
	ldrh	r9, [r6, #232]
	add	r7, r4, #16
	ldr	r10, .L249+12
	mvn	r8, #0
	mov	r5, #0
	strh	r5, [r4, #4]	@ movhi
	strb	r5, [r4, #7]
.L244:
	uxth	r3, r5
	ldrh	r1, [r4]
	cmp	r9, r3
	bhi	.L246
	movw	r2, #302
	ldrb	r3, [r4, #7]	@ zero_extendqisi2
	ldrh	r2, [r6, r2]
	lsl	r1, r1, #1
	mov	r0, #0
	smulbb	r3, r3, r2
	strh	r3, [r4, #4]	@ movhi
	mov	r3, #0
	strb	r3, [r4, #9]
	ldr	r3, [r6, #2536]
	ldrh	r2, [r3, r1]
	movw	r3, #10000
	cmp	r2, r3
	movhi	r3, #1
	strbhi	r3, [r4, #9]
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L246:
	ldrb	r0, [r10, r5]	@ zero_extendqisi2
	add	r7, r7, #2
	bl	V2P_block
	strh	r8, [r7, #-2]	@ movhi
	mov	fp, r0
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	add	r5, r5, #1
	strheq	fp, [r7, #-2]	@ movhi
	ldrbeq	r3, [r4, #7]	@ zero_extendqisi2
	addeq	r3, r3, #1
	strbeq	r3, [r4, #7]
	b	.L244
.L250:
	.align	2
.L249:
	.word	.LANCHOR0
	.word	.LANCHOR1+95
	.word	.LC8
	.word	.LANCHOR0+260
	.fnend
	.size	make_superblock, .-make_superblock
	.align	2
	.global	update_multiplier_value
	.syntax unified
	.arm
	.fpu softvfp
	.type	update_multiplier_value, %function
update_multiplier_value:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L258
	movw	r2, #302
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r5, #0
	mov	r7, r0
	mov	r4, r5
	mov	r6, r3
	add	r10, r3, #260
	ldrh	r8, [r3, #232]
	ldrh	r9, [r3, r2]
.L252:
	uxth	r3, r5
	cmp	r8, r3
	bhi	.L254
	cmp	r4, #0
	moveq	r0, r4
	beq	.L255
	mov	r1, r4
	mov	r0, #32768
	bl	__aeabi_idiv
.L255:
	ldr	r1, [r6, #2516]
	mov	r3, #6
	mla	r1, r3, r7, r1
	strh	r0, [r1, #4]	@ movhi
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L254:
	mov	r1, r7
	ldrb	r0, [r10, r5]	@ zero_extendqisi2
	bl	V2P_block
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	add	r5, r5, #1
	addeq	r4, r4, r9
	uxtheq	r4, r4
	b	.L252
.L259:
	.align	2
.L258:
	.word	.LANCHOR0
	.fnend
	.size	update_multiplier_value, .-update_multiplier_value
	.align	2
	.global	GetFreeBlockMinEraseCount
	.syntax unified
	.arm
	.fpu softvfp
	.type	GetFreeBlockMinEraseCount, %function
GetFreeBlockMinEraseCount:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L263
	ldr	r0, [r2, #2532]
	cmp	r0, #0
	bxeq	lr
	ldr	r3, [r2, #2516]
	sub	r0, r0, r3
	ldr	r3, .L263+4
	asr	r0, r0, #1
	mul	r0, r3, r0
	ldr	r3, [r2, #2536]
	uxth	r0, r0
	lsl	r0, r0, #1
	ldrh	r0, [r3, r0]
	bx	lr
.L264:
	.align	2
.L263:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	GetFreeBlockMinEraseCount, .-GetFreeBlockMinEraseCount
	.align	2
	.global	GetFreeBlockMaxEraseCount
	.syntax unified
	.arm
	.fpu softvfp
	.type	GetFreeBlockMaxEraseCount, %function
GetFreeBlockMaxEraseCount:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r1, .L277
	ldr	r3, [r1, #2532]
	cmp	r3, #0
	beq	.L271
	push	{r4, r5, lr}
	.save {r4, r5, lr}
	mov	r4, #6
	ldrh	r2, [r1, #224]
	movw	r5, #65535
	ldr	ip, [r1, #2516]
	rsb	r2, r2, r2, lsl #3
	sub	r3, r3, ip
	asr	r2, r2, #3
	asr	r3, r3, #1
	cmp	r0, r2
	uxthgt	r0, r2
	ldr	r2, .L277+4
	mul	r3, r2, r3
	mov	r2, #0
	uxth	r3, r3
.L268:
	uxth	lr, r2
	cmp	r0, lr
	bls	.L270
	mul	lr, r4, r3
	add	r2, r2, #1
	ldrh	lr, [ip, lr]
	cmp	lr, r5
	bne	.L272
.L270:
	ldr	r2, [r1, #2536]
	lsl	r3, r3, #1
	ldrh	r0, [r2, r3]
	pop	{r4, r5, pc}
.L272:
	mov	r3, lr
	b	.L268
.L271:
	mov	r0, r3
	bx	lr
.L278:
	.align	2
.L277:
	.word	.LANCHOR0
	.word	-1431655765
	.fnend
	.size	GetFreeBlockMaxEraseCount, .-GetFreeBlockMaxEraseCount
	.align	2
	.global	FtlPrintInfo2buf
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlPrintInfo2buf, %function
FtlPrintInfo2buf:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r6, r0
	ldr	r5, .L292
	add	r4, r6, #12
	.pad #20
	sub	sp, sp, #20
	ldr	r1, .L292+4
	bl	strcpy
	ldr	r3, [r5, #316]
	mov	r1, #64
	ldr	r2, .L292+8
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r1, .L292+12
	mov	r0, r4
	add	r4, r4, #10
	bl	strcpy
	ldr	r3, [r5, #2552]
	mov	r1, #64
	ldr	r2, .L292+16
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2548]
	ldr	r2, .L292+20
	mov	r1, #64
	mov	r0, r4
	add	r7, r5, #352
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2556]
	ldr	r2, .L292+24
	mov	r1, #64
	mov	r0, r4
	ldr	r8, .L292+28
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2560]
	ldr	r2, .L292+32
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2564]
	ldr	r2, .L292+36
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2568]
	ldr	r2, .L292+40
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2572]
	ldr	r2, .L292+44
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2576]
	ldr	r2, .L292+48
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	ldr	r3, [r5, #2580]
	add	r4, r4, r0
	ldr	r2, .L292+52
	mov	r1, #64
	mov	r0, r4
	lsr	r3, r3, #11
	bl	snprintf
	ldr	r3, [r5, #2584]
	add	r4, r4, r0
	ldr	r2, .L292+56
	mov	r1, #64
	mov	r0, r4
	lsr	r3, r3, #11
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2588]
	ldr	r2, .L292+60
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2592]
	ldr	r2, .L292+64
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r7, #6]
	ldr	r2, .L292+68
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #224]
	ldr	r2, .L292+72
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2596]
	ldr	r2, .L292+76
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2600]
	ldr	r2, .L292+80
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2604]
	ldr	r2, .L292+84
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2608]
	ldr	r2, .L292+88
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2612]
	ldr	r2, .L292+92
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2616]
	ldr	r2, .L292+96
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r8, #30]
	ldr	r2, .L292+100
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r8, #28]
	ldr	r2, .L292+104
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #340]
	ldr	r2, .L292+108
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #332]
	ldr	r2, .L292+112
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #228]
	ldr	r2, .L292+116
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r3, r5, #412
	add	r4, r4, r0
	ldrh	r3, [r3, #6]
	mov	r1, #64
	ldr	r2, .L292+120
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #240]
	ldr	r2, .L292+124
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	movw	r3, #2620
	add	r4, r4, r0
	ldrh	r3, [r5, r3]
	mov	r1, #64
	ldr	r2, .L292+128
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #244]
	ldr	r2, .L292+132
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r3, r5, #2624
	add	r4, r4, r0
	ldrh	r3, [r3]
	mov	r1, #64
	ldr	r2, .L292+136
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r7]
	ldr	r2, .L292+140
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #26]
	ldr	r2, .L292+144
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #30]	@ zero_extendqisi2
	ldr	r2, .L292+148
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #24]
	ldr	r2, .L292+152
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #32]	@ zero_extendqisi2
	ldr	r2, .L292+156
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #28]
	ldr	r2, .L292+160
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	ldrh	r3, [r5, #24]
	add	r4, r4, r0
	ldr	r2, [r5, #72]
	mov	r1, #64
	mov	r0, r4
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	ldr	r2, .L292+164
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #78]
	ldr	r2, .L292+168
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #82]	@ zero_extendqisi2
	ldr	r2, .L292+172
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #76]
	ldr	r2, .L292+176
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #84]	@ zero_extendqisi2
	ldr	r2, .L292+180
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #80]
	ldr	r2, .L292+184
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	ldrh	r3, [r5, #76]
	add	r4, r4, r0
	ldr	r2, [r5, #72]
	mov	r1, #64
	mov	r0, r4
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	ldr	r2, .L292+188
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #126]
	ldr	r2, .L292+192
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #130]	@ zero_extendqisi2
	ldr	r2, .L292+196
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #124]
	ldr	r2, .L292+200
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #132]	@ zero_extendqisi2
	ldr	r2, .L292+204
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #128]
	ldr	r2, .L292+208
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #174]
	ldr	r2, .L292+212
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #178]	@ zero_extendqisi2
	ldr	r2, .L292+216
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #172]
	ldr	r2, .L292+220
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrb	r3, [r5, #180]	@ zero_extendqisi2
	ldr	r2, .L292+224
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r3, [r5, #176]
	ldr	r2, .L292+228
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	ldr	r3, [r5, #2716]
	add	r4, r4, r0
	ldr	r2, .L292+232
	mov	r1, #64
	mov	r0, r4
	str	r3, [sp, #4]
	ldr	r3, [r5, #2724]
	str	r3, [sp]
	ldr	r3, [r5, #2720]
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2712]
	ldr	r2, .L292+236
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #2736]
	ldr	r2, .L292+240
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r3, r5, #3152
	add	r4, r4, r0
	ldrh	r3, [r3]
	mov	r1, #64
	ldr	r2, .L292+244
	mov	r0, r4
	bl	snprintf
	movw	r3, #3154
	add	r4, r4, r0
	ldrh	r3, [r5, r3]
	mov	r1, #64
	ldr	r2, .L292+248
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldr	r3, [r5, #3156]
	ldr	r2, .L292+252
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	movw	r3, #3160
	add	r4, r4, r0
	ldrh	r3, [r5, r3]
	mov	r1, #64
	ldr	r2, .L292+256
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	bl	GetFreeBlockMinEraseCount
	ldr	r2, .L292+260
	mov	r3, r0
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	add	r4, r4, r0
	ldrh	r0, [r5, #224]
	bl	GetFreeBlockMaxEraseCount
	ldr	r2, .L292+264
	mov	r3, r0
	mov	r1, #64
	mov	r0, r4
	bl	snprintf
	ldr	r3, .L292+268
	add	r4, r4, r0
	ldr	r3, [r3]
	cmp	r3, #1
	beq	.L280
.L285:
	sub	r0, r4, r6
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L280:
	ldrh	r3, [r5, #172]
	movw	r2, #65535
	cmp	r3, r2
	beq	.L282
	ldr	r2, [r5, #72]
	lsl	r3, r3, #1
	mov	r0, r4
	mov	r1, #64
	ldrh	r3, [r2, r3]
	ldr	r2, .L292+272
	bl	snprintf
	add	r4, r4, r0
.L282:
	mov	r0, #0
	ldr	r9, .L292+276
	bl	List_get_gc_head_node
	uxth	r0, r0
	mov	r7, #0
	movw	fp, #65535
	mov	r10, #6
.L284:
	cmp	r0, fp
	beq	.L283
	ldr	r3, [r5, #2536]
	lsl	r2, r0, #1
	mul	r8, r10, r0
	mov	r1, #64
	ldrh	r3, [r3, r2]
	str	r3, [sp, #12]
	ldr	r3, [r5, #2516]
	add	r3, r3, r8
	ldrh	r3, [r3, #4]
	str	r3, [sp, #8]
	ldr	r3, [r5, #72]
	ldrh	r3, [r3, r2]
	mov	r2, r9
	stm	sp, {r0, r3}
	mov	r3, r7
	mov	r0, r4
	add	r7, r7, #1
	bl	snprintf
	ldr	r3, [r5, #2516]
	cmp	r7, #16
	add	r4, r4, r0
	ldrh	r0, [r3, r8]
	bne	.L284
.L283:
	ldr	r2, [r5, #2516]
	mov	r7, #0
	ldr	r3, [r5, #2532]
	movw	r9, #65535
	ldr	fp, .L292+280
	mov	r10, #6
	sub	r3, r3, r2
	ldr	r2, .L292+284
	asr	r3, r3, #1
	mul	r3, r2, r3
	uxth	r3, r3
.L286:
	cmp	r3, r9
	beq	.L285
	ldr	r1, [r5, #2536]
	lsl	r2, r3, #1
	mul	r8, r10, r3
	mov	r0, r4
	ldrh	r2, [r1, r2]
	mov	r1, #64
	str	r2, [sp, #8]
	ldr	r2, [r5, #2516]
	add	r2, r2, r8
	ldrh	r2, [r2, #4]
	str	r3, [sp]
	mov	r3, r7
	add	r7, r7, #1
	str	r2, [sp, #4]
	mov	r2, fp
	bl	snprintf
	cmp	r7, #4
	add	r4, r4, r0
	beq	.L285
	ldr	r3, [r5, #2516]
	ldrh	r3, [r3, r8]
	b	.L286
.L293:
	.align	2
.L292:
	.word	.LANCHOR0
	.word	.LC9
	.word	.LC10
	.word	.LC11
	.word	.LC12
	.word	.LC13
	.word	.LC14
	.word	.LANCHOR0+2468
	.word	.LC15
	.word	.LC16
	.word	.LC17
	.word	.LC18
	.word	.LC19
	.word	.LC20
	.word	.LC21
	.word	.LC22
	.word	.LC23
	.word	.LC24
	.word	.LC25
	.word	.LC26
	.word	.LC27
	.word	.LC28
	.word	.LC29
	.word	.LC30
	.word	.LC31
	.word	.LC32
	.word	.LC33
	.word	.LC34
	.word	.LC35
	.word	.LC36
	.word	.LC37
	.word	.LC38
	.word	.LC39
	.word	.LC40
	.word	.LC41
	.word	.LC42
	.word	.LC43
	.word	.LC44
	.word	.LC45
	.word	.LC46
	.word	.LC47
	.word	.LC48
	.word	.LC49
	.word	.LC50
	.word	.LC51
	.word	.LC52
	.word	.LC53
	.word	.LC54
	.word	.LC55
	.word	.LC56
	.word	.LC57
	.word	.LC58
	.word	.LC59
	.word	.LC60
	.word	.LC61
	.word	.LC62
	.word	.LC63
	.word	.LC64
	.word	.LC65
	.word	.LC66
	.word	.LC67
	.word	.LC68
	.word	.LC69
	.word	.LC70
	.word	.LC71
	.word	.LC72
	.word	.LC73
	.word	.LANCHOR2
	.word	.LC74
	.word	.LC75
	.word	.LC76
	.word	-1431655765
	.fnend
	.size	FtlPrintInfo2buf, .-FtlPrintInfo2buf
	.align	2
	.global	rknand_proc_ftlread
	.syntax unified
	.arm
	.fpu softvfp
	.type	rknand_proc_ftlread, %function
rknand_proc_ftlread:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	cmp	r0, #2048
	blt	.L296
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r5, r1
	ldr	r3, .L301
	mov	r1, #64
	mov	r0, r5
	ldr	r2, .L301+4
	bl	snprintf
	add	r4, r5, r0
	mov	r0, r4
	bl	FtlPrintInfo2buf
	add	r0, r4, r0
	sub	r0, r0, r5
	pop	{r4, r5, r6, pc}
.L296:
	mov	r0, #0
	bx	lr
.L302:
	.align	2
.L301:
	.word	.LC0
	.word	.LC77
	.fnend
	.size	rknand_proc_ftlread, .-rknand_proc_ftlread
	.align	2
	.global	GetSwlReplaceBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	GetSwlReplaceBlock, %function
GetSwlReplaceBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #28
	sub	sp, sp, #28
	ldr	r4, .L328
	ldr	r2, [r4, #2604]
	ldr	r3, [r4, #2616]
	cmp	r2, r3
	bcs	.L304
	ldr	r2, [r4, #2536]
	mov	r3, #0
	ldrh	r1, [r4, #240]
	str	r3, [r4, #2596]
	sub	r2, r2, #2
.L305:
	cmp	r3, r1
	bcc	.L306
	ldr	r5, [r4, #2596]
	mov	r0, r5
	bl	__aeabi_uidiv
	ldr	r3, .L328+4
	str	r0, [r4, #2604]
	ldr	r0, [r4, #2600]
	ldrh	r1, [r3]
	sub	r0, r5, r0
	bl	__aeabi_uidiv
	str	r0, [r4, #2596]
.L307:
	ldr	r6, [r4, #2616]
	ldr	r7, [r4, #2604]
	add	r3, r6, #256
	cmp	r3, r7
	bls	.L312
	ldr	r2, [r4, #2612]
	add	r3, r6, #768
	cmp	r3, r2
	bls	.L312
.L314:
	movw	r5, #65535
.L313:
	mov	r0, r5
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L306:
	ldrh	r0, [r2, #2]!
	add	r3, r3, #1
	ldr	ip, [r4, #2596]
	add	r0, r0, ip
	str	r0, [r4, #2596]
	b	.L305
.L304:
	ldr	r3, [r4, #2612]
	cmp	r2, r3
	addhi	r3, r3, #1
	strhi	r3, [r4, #2612]
	movhi	r3, #0
	bls	.L307
.L309:
	ldrh	r2, [r4, #240]
	cmp	r3, r2
	bcs	.L307
	ldr	r0, [r4, #2536]
	lsl	r1, r3, #1
	add	r3, r3, #1
	ldrh	r2, [r0, r1]
	add	r2, r2, #1
	strh	r2, [r0, r1]	@ movhi
	b	.L309
.L312:
	ldrh	r0, [r4, #224]
	add	r0, r0, r0, lsl #1
	ubfx	r0, r0, #2, #16
	bl	GetFreeBlockMaxEraseCount
	add	r3, r6, #64
	mov	r9, r0
	cmp	r0, r3
	bcc	.L314
	ldr	r3, [r4, #2520]
	cmp	r3, #0
	beq	.L314
	ldrh	r2, [r4, #240]
	movw	r1, #65535
	ldr	lr, [r4, #2516]
	mov	r0, #0
	ldr	r8, [r4, #2536]
	mov	fp, #6
	ldr	r10, .L328+8
	str	r2, [sp, #20]
	mov	r2, r1
.L315:
	ldrh	ip, [r3]
	movw	r5, #65535
	cmp	ip, r5
	bne	.L318
	mov	r5, r2
.L317:
	movw	r3, #65535
	cmp	r5, r3
	beq	.L314
	lsl	fp, r5, #1
	ldrh	r10, [r8, fp]
	cmp	r6, r10
	bcs	.L319
	bl	GetFreeBlockMinEraseCount
	cmp	r6, r0
	strcc	r1, [r4, #2616]
.L319:
	cmp	r7, r10
	bls	.L314
	add	r3, r10, #128
	cmp	r9, r3
	ble	.L314
	add	r3, r10, #256
	cmp	r7, r3
	bhi	.L320
	ldr	r3, [r4, #2612]
	add	r10, r10, #768
	cmp	r10, r3
	bcs	.L314
.L320:
	str	r9, [sp, #8]
	mov	r2, r7
	ldrh	r3, [r8, fp]
	mov	r1, r5
	ldr	r0, .L328+12
	str	r3, [sp, #4]
	ldr	r3, [r4, #72]
	ldrh	r3, [r3, fp]
	str	r3, [sp]
	ldr	r3, [r4, #2612]
	bl	sftl_printk
	mov	r3, #1
	str	r3, [r4, #3164]
	b	.L313
.L318:
	add	r0, r0, #1
	ldr	r5, [sp, #20]
	uxth	r0, r0
	cmp	r0, r5
	bhi	.L314
	ldrh	r5, [r3, #4]
	cmp	r5, #0
	beq	.L316
	sub	r3, r3, lr
	asr	r3, r3, #1
	mul	r3, r10, r3
	uxth	r5, r3
	lsl	r3, r5, #1
	ldrh	r3, [r8, r3]
	cmp	r6, r3
	bcs	.L317
	cmp	r1, r3
	movhi	r1, r3
	movhi	r2, r5
.L316:
	mla	r3, fp, ip, lr
	b	.L315
.L329:
	.align	2
.L328:
	.word	.LANCHOR0
	.word	.LANCHOR0+292
	.word	-1431655765
	.word	.LC78
	.fnend
	.size	GetSwlReplaceBlock, .-GetSwlReplaceBlock
	.align	2
	.global	free_data_superblock
	.syntax unified
	.arm
	.fpu softvfp
	.type	free_data_superblock, %function
free_data_superblock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	movw	r2, #65535
	cmp	r0, r2
	beq	.L333
	ldr	r2, .L336
	lsl	r3, r0, #1
	push	{r4, lr}
	.save {r4, lr}
	mov	r1, #0
	ldr	r2, [r2, #72]
	strh	r1, [r2, r3]	@ movhi
	bl	INSERT_FREE_LIST
	mov	r0, #0
	pop	{r4, pc}
.L333:
	mov	r0, #0
	bx	lr
.L337:
	.align	2
.L336:
	.word	.LANCHOR0
	.fnend
	.size	free_data_superblock, .-free_data_superblock
	.align	2
	.global	get_new_active_ppa
	.syntax unified
	.arm
	.fpu softvfp
	.type	get_new_active_ppa, %function
get_new_active_ppa:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	movw	r3, #65535
	ldrh	r2, [r0]
	mov	r4, r0
	cmp	r2, r3
	bne	.L339
	movw	r2, #2792
	ldr	r1, .L354
	ldr	r0, .L354+4
	bl	sftl_printk
.L339:
	ldr	r6, .L354+8
	movw	r3, #302
	ldrh	r2, [r4, #2]
	ldrh	r3, [r6, r3]
	cmp	r2, r3
	bne	.L340
	movw	r2, #2793
	ldr	r1, .L354
	ldr	r0, .L354+4
	bl	sftl_printk
.L340:
	ldrh	r3, [r4, #4]
	cmp	r3, #0
	bne	.L341
	movw	r2, #2794
	ldr	r1, .L354
	ldr	r0, .L354+4
	bl	sftl_printk
.L341:
	ldrb	r2, [r4, #6]	@ zero_extendqisi2
	mov	r3, #0
	ldrh	r1, [r6, #232]
	movw	ip, #65535
	strb	r3, [r4, #10]
	add	r2, r4, r2, lsl #1
	ldrh	r0, [r2, #16]
	mov	r2, r3
.L342:
	cmp	r0, ip
	beq	.L344
	movw	r3, #302
	ldrh	r5, [r4, #2]
	ldrh	ip, [r6, r3]
	cmp	r5, ip
	movwcs	r5, #65535
	bcs	.L338
	ldrh	r2, [r4, #4]
	orr	r5, r5, r0, lsl #10
	ldrb	r3, [r4, #6]	@ zero_extendqisi2
	movw	lr, #65535
	sub	r2, r2, #1
	uxth	r2, r2
	strh	r2, [r4, #4]	@ movhi
.L347:
	add	r3, r3, #1
	uxtb	r3, r3
	cmp	r1, r3
	ldrheq	r0, [r4, #2]
	moveq	r3, #0
	addeq	r0, r0, #1
	strheq	r0, [r4, #2]	@ movhi
	add	r0, r4, r3, lsl #1
	ldrh	r0, [r0, #16]
	cmp	r0, lr
	beq	.L347
	strb	r3, [r4, #6]
	cmp	r2, #0
	ldrh	r3, [r4, #2]
	sub	r3, r3, ip
	clz	r3, r3
	lsr	r3, r3, #5
	moveq	r3, #0
	cmp	r3, #0
	beq	.L338
	movw	r2, #2817
	ldr	r1, .L354
	ldr	r0, .L354+4
	bl	sftl_printk
.L338:
	mov	r0, r5
	pop	{r4, r5, r6, pc}
.L344:
	ldrb	r3, [r4, #6]	@ zero_extendqisi2
	add	r3, r3, #1
	uxtb	r3, r3
	cmp	r3, r1
	strb	r3, [r4, #6]
	ldrheq	r3, [r4, #2]
	strbeq	r2, [r4, #6]
	addeq	r3, r3, #1
	strheq	r3, [r4, #2]	@ movhi
	ldrb	r3, [r4, #6]	@ zero_extendqisi2
	add	r3, r4, r3, lsl #1
	ldrh	r0, [r3, #16]
	b	.L342
.L355:
	.align	2
.L354:
	.word	.LANCHOR1+111
	.word	.LC8
	.word	.LANCHOR0
	.fnend
	.size	get_new_active_ppa, .-get_new_active_ppa
	.align	2
	.global	FtlGcBufInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcBufInit, %function
FtlGcBufInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	mov	lr, #12
	ldr	r5, .L362
	mov	r4, #1
	mov	r6, #20
	mov	r3, #0
	ldr	r2, .L362+4
	str	r3, [r2, #3168]
.L357:
	ldrh	r1, [r2, #232]
	uxth	r0, r3
	add	ip, r3, #1
	cmp	r0, r1
	bcc	.L358
	ldr	r4, .L362
	mov	ip, #12
	mov	lr, #0
.L359:
	ldr	r3, [r2, #3188]
	cmp	r1, r3
	bcc	.L360
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L358:
	uxth	r3, r3
	ldr	r7, [r2, #3172]
	mul	r0, lr, r3
	add	r1, r7, r0
	str	r4, [r1, #8]
	ldrh	r1, [r5]
	mul	r1, r3, r1
	add	r8, r1, #3
	cmp	r1, #0
	movlt	r1, r8
	ldr	r8, [r2, #3176]
	bic	r1, r1, #3
	add	r1, r8, r1
	str	r1, [r7, r0]
	ldr	r1, .L362+8
	ldr	r8, [r2, #3172]
	ldrh	r1, [r1]
	add	r7, r8, r0
	mul	r1, r3, r1
	add	r9, r1, #3
	cmp	r1, #0
	movlt	r1, r9
	ldr	r9, [r2, #3180]
	bic	r1, r1, #3
	add	r1, r9, r1
	str	r1, [r7, #4]
	ldr	r1, [r2, #3184]
	mla	r3, r6, r3, r1
	ldr	r1, [r8, r0]
	str	r1, [r3, #8]
	ldr	r1, [r7, #4]
	str	r1, [r3, #12]
	mov	r3, ip
	b	.L357
.L360:
	mul	r5, ip, r1
	ldr	r6, [r2, #3172]
	add	r3, r6, r5
	str	lr, [r3, #8]
	ldrh	r3, [r4]
	mul	r3, r1, r3
	add	r0, r3, #3
	cmp	r3, #0
	movlt	r3, r0
	ldr	r0, [r2, #3176]
	bic	r3, r3, #3
	add	r3, r0, r3
	str	r3, [r6, r5]
	ldr	r3, .L362+8
	ldr	r0, [r2, #3172]
	ldrh	r3, [r3]
	add	r0, r0, r5
	mul	r3, r1, r3
	add	r1, r1, #1
	uxth	r1, r1
	add	r5, r3, #3
	cmp	r3, #0
	movlt	r3, r5
	ldr	r5, [r2, #3180]
	bic	r3, r3, #3
	add	r3, r5, r3
	str	r3, [r0, #4]
	b	.L359
.L363:
	.align	2
.L362:
	.word	.LANCHOR0+310
	.word	.LANCHOR0
	.word	.LANCHOR0+312
	.fnend
	.size	FtlGcBufInit, .-FtlGcBufInit
	.align	2
	.global	FtlGcBufFree
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcBufFree, %function
FtlGcBufFree:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L372
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	lr, #0
	mov	r5, #20
	mov	r7, #12
	mov	r8, lr
	ldr	r6, [r3, #3188]
	ldr	r4, [r3, #3172]
.L365:
	uxth	r3, lr
	cmp	r1, r3
	popls	{r4, r5, r6, r7, r8, r9, r10, pc}
	mla	ip, r5, r3, r0
	mov	r2, #0
.L366:
	uxth	r3, r2
	cmp	r6, r3
	bls	.L367
	mul	r3, r7, r3
	add	r2, r2, #1
	ldr	r10, [r4, r3]
	add	r9, r4, r3
	ldr	r3, [ip, #8]
	cmp	r10, r3
	bne	.L366
	str	r8, [r9, #8]
.L367:
	add	lr, lr, #1
	b	.L365
.L373:
	.align	2
.L372:
	.word	.LANCHOR0
	.fnend
	.size	FtlGcBufFree, .-FtlGcBufFree
	.align	2
	.global	FtlGcBufAlloc
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcBufAlloc, %function
FtlGcBufAlloc:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L382
	mov	ip, #0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	mov	r6, #12
	mov	r7, #1
	mov	r8, #20
	ldr	r4, [r3, #3188]
	ldr	r5, [r3, #3172]
.L375:
	uxth	r2, ip
	cmp	r1, r2
	bhi	.L379
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L379:
	mov	lr, #0
.L376:
	uxth	r3, lr
	cmp	r4, r3
	bls	.L377
	mla	r3, r6, r3, r5
	add	lr, lr, #1
	ldr	r9, [r3, #8]
	cmp	r9, #0
	bne	.L376
	mla	r2, r8, r2, r0
	ldr	lr, [r3]
	str	r7, [r3, #8]
	str	lr, [r2, #8]
	ldr	r3, [r3, #4]
	str	r3, [r2, #12]
.L377:
	add	ip, ip, #1
	b	.L375
.L383:
	.align	2
.L382:
	.word	.LANCHOR0
	.fnend
	.size	FtlGcBufAlloc, .-FtlGcBufAlloc
	.align	2
	.global	IsBlkInGcList
	.syntax unified
	.arm
	.fpu softvfp
	.type	IsBlkInGcList, %function
IsBlkInGcList:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L389
	movw	r1, #3196
	ldr	r3, [r2, #3192]
	ldrh	r2, [r2, r1]
	add	r2, r3, r2, lsl #1
.L385:
	cmp	r3, r2
	bne	.L387
	mov	r0, #0
	bx	lr
.L387:
	ldrh	r1, [r3], #2
	cmp	r1, r0
	bne	.L385
	mov	r0, #1
	bx	lr
.L390:
	.align	2
.L389:
	.word	.LANCHOR0
	.fnend
	.size	IsBlkInGcList, .-IsBlkInGcList
	.align	2
	.global	FtlGcUpdatePage
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcUpdatePage, %function
FtlGcUpdatePage:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r4, r0
	ubfx	r0, r0, #10, #16
	mov	r5, r1
	mov	r6, r2
	bl	P2V_block_in_plane
	ldr	r3, .L396
	movw	r2, #3196
	mov	ip, #0
	ldrh	lr, [r3, r2]
	ldr	r2, [r3, #3192]
	sub	r1, r2, #2
.L392:
	uxth	r7, ip
	cmp	r7, lr
	bcc	.L394
	bne	.L393
	lsl	ip, r7, #1
	strh	r0, [r2, ip]	@ movhi
	movw	r2, #3196
	ldrh	r0, [r3, r2]
	add	r0, r0, #1
	strh	r0, [r3, r2]	@ movhi
	b	.L393
.L394:
	ldrh	r7, [r1, #2]!
	add	ip, ip, #1
	cmp	r7, r0
	bne	.L392
.L393:
	movw	ip, #3204
	mov	r0, #12
	ldrh	r2, [r3, ip]
	mul	r0, r0, r2
	ldr	r2, [r3, #3200]
	add	r1, r2, r0
	stmib	r1, {r5, r6}
	str	r4, [r2, r0]
	ldrh	r2, [r3, ip]
	add	r2, r2, #1
	strh	r2, [r3, ip]	@ movhi
	pop	{r4, r5, r6, r7, r8, pc}
.L397:
	.align	2
.L396:
	.word	.LANCHOR0
	.fnend
	.size	FtlGcUpdatePage, .-FtlGcUpdatePage
	.align	2
	.global	FtlGcRefreshBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcRefreshBlock, %function
FtlGcRefreshBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	mov	r1, r0
	mov	r4, r0
	ldr	r0, .L402
	bl	sftl_printk
	ldr	r3, .L402+4
	ldrh	r0, [r3, #222]
	cmp	r4, r0
	beq	.L399
	ldrh	r1, [r3, #220]
	cmp	r4, r1
	beq	.L399
	movw	r2, #65535
	cmp	r0, r2
	strheq	r4, [r3, #222]	@ movhi
	beq	.L399
	cmp	r1, r2
	strheq	r4, [r3, #220]	@ movhi
.L399:
	mov	r0, #0
	pop	{r4, pc}
.L403:
	.align	2
.L402:
	.word	.LC79
	.word	.LANCHOR0
	.fnend
	.size	FtlGcRefreshBlock, .-FtlGcRefreshBlock
	.align	2
	.global	FtlGcMarkBadPhyBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcMarkBadPhyBlk, %function
FtlGcMarkBadPhyBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r5, r0
	ldr	r4, .L409
	movw	r6, #3206
	bl	P2V_block_in_plane
	mov	r2, r5
	mov	r7, r0
	ldrh	r1, [r4, r6]
	ldr	r0, .L409+4
	bl	sftl_printk
	mov	r0, r7
	bl	FtlGcRefreshBlock
	ldrh	r3, [r4, r6]
	mov	r2, #0
	ldr	r0, .L409+8
.L405:
	uxth	r1, r2
	cmp	r3, r1
	bhi	.L407
	cmp	r3, #15
	movwls	r2, #3206
	addls	r1, r3, #1
	strhls	r1, [r4, r2]	@ movhi
	lslls	r3, r3, #1
	ldrls	r2, .L409+8
	strhls	r5, [r2, r3]	@ movhi
	b	.L406
.L407:
	add	r2, r2, #1
	add	r1, r0, r2, lsl #1
	ldrh	r1, [r1, #-2]
	cmp	r1, r5
	bne	.L405
.L406:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, pc}
.L410:
	.align	2
.L409:
	.word	.LANCHOR0
	.word	.LC80
	.word	.LANCHOR0+3208
	.fnend
	.size	FtlGcMarkBadPhyBlk, .-FtlGcMarkBadPhyBlk
	.align	2
	.global	FtlGcReFreshBadBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcReFreshBadBlk, %function
FtlGcReFreshBadBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	movw	r3, #3206
	ldr	r4, .L418
	ldrh	r2, [r4, r3]
	cmp	r2, #0
	beq	.L412
	ldrh	r1, [r4, #222]
	movw	r3, #65535
	cmp	r1, r3
	bne	.L412
	movw	r3, #3242
	movw	r5, #3242
	ldrh	r1, [r4, r3]
	cmp	r1, r2
	movcs	r2, #0
	strhcs	r2, [r4, r3]	@ movhi
	ldrh	r3, [r4, r5]
	ldr	r2, .L418+4
	lsl	r3, r3, #1
	ldrh	r0, [r2, r3]
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
	ldrh	r3, [r4, r5]
	add	r3, r3, #1
	strh	r3, [r4, r5]	@ movhi
.L412:
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L419:
	.align	2
.L418:
	.word	.LANCHOR0
	.word	.LANCHOR0+3208
	.fnend
	.size	FtlGcReFreshBadBlk, .-FtlGcReFreshBadBlk
	.align	2
	.global	ftl_malloc
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_malloc, %function
ftl_malloc:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r1, .L421
	b	__kmalloc
.L422:
	.align	2
.L421:
	.word	6291649
	.fnend
	.size	ftl_malloc, .-ftl_malloc
	.align	2
	.global	ftl_free
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_free, %function
ftl_free:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	b	kfree
	.fnend
	.size	ftl_free, .-ftl_free
	.align	2
	.global	rknand_print_hex
	.syntax unified
	.arm
	.fpu softvfp
	.type	rknand_print_hex, %function
rknand_print_hex:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r6, #0
	ldr	fp, .L434
	mov	r10, r0
	mov	r7, r1
	mov	r8, r2
	mov	r9, r3
	mov	r5, r6
	mov	r4, r6
.L425:
	cmp	r4, r9
	bcc	.L431
	ldr	r0, .L434+4
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	b	sftl_printk
.L431:
	cmp	r5, #0
	bne	.L426
	mov	r3, r6
	mov	r2, r7
	mov	r1, r10
	ldr	r0, .L434+8
	bl	sftl_printk
.L426:
	cmp	r8, #4
	ldreq	r1, [r7, r4, lsl #2]
	ldreq	r0, .L434+12
	beq	.L433
	cmp	r8, #2
	lsleq	r3, r4, #1
	ldrbne	r1, [r7, r4]	@ zero_extendqisi2
	ldrne	r0, .L434+16
	moveq	r0, fp
	ldrheq	r1, [r7, r3]
.L433:
	add	r5, r5, #1
	bl	sftl_printk
	cmp	r5, #15
	bls	.L430
	mov	r5, #0
	ldr	r0, .L434+4
	bl	sftl_printk
.L430:
	add	r4, r4, #1
	add	r6, r6, r8
	b	.L425
.L435:
	.align	2
.L434:
	.word	.LC83
	.word	.LC85
	.word	.LC81
	.word	.LC82
	.word	.LC84
	.fnend
	.size	rknand_print_hex, .-rknand_print_hex
	.align	2
	.global	FlashEraseBlocks
	.syntax unified
	.arm
	.fpu softvfp
	.type	FlashEraseBlocks, %function
FlashEraseBlocks:
	.fnstart
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r8, r2
	ldr	r5, .L454
	.pad #28
	sub	sp, sp, #28
	mov	r4, r0
	mov	fp, r0
	ldr	r2, .L454+4
	mov	r7, #0
	ldrh	r9, [r5, #12]
	mvn	r10, #0
	ldr	r3, [r2]
	str	r2, [sp, #4]
	str	r3, [sp, #20]
	lsl	r3, r9, #3
	str	r3, [sp]
.L437:
	cmp	r7, r8
	beq	.L441
	add	r2, sp, #12
	add	r1, sp, #16
	mov	r0, fp
	bl	l2p_addr_tran
	ldr	r6, [sp, #12]
	cmp	r6, #0
	bne	.L438
	ldr	r2, [sp, #16]
	ldr	r3, [sp]
	cmp	r3, r2
	bls	.L438
	ldr	r5, .L454+8
	ldr	r7, .L454+12
	ldr	r9, .L454+16
	b	.L452
.L440:
	mvn	r3, #0
	ldr	r2, [sp, #16]
	str	r3, [r4, #-20]
	mov	r1, r5
	mov	r0, r7
	add	r6, r6, #1
	bl	sftl_printk
	mov	r3, #16
	mov	r2, #4
	ldr	r1, [r4, #-12]
	mov	r0, r9
	bl	rknand_print_hex
	mov	r3, #4
	ldr	r1, [r4, #-8]
	mov	r2, r3
	ldr	r0, .L454+20
	bl	rknand_print_hex
.L452:
	cmp	r6, r8
	add	r4, r4, #20
	bne	.L440
	bl	dump_stack
.L441:
	ldr	r3, [sp, #4]
	mov	r0, #0
	ldr	r2, [sp, #20]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L448
	bl	__stack_chk_fail
.L438:
	ldr	r2, [r5, #3248]
	uxtb	r0, r6
	ldr	r1, [sp, #16]
	blx	r2
	subs	r2, r0, #0
	movne	r2, r10
	str	r2, [fp]
	ldrh	r2, [r5, #14]
	cmp	r2, #4
	bne	.L445
	ldr	r1, [sp, #16]
	ldr	r2, [r5, #3248]
	ldrb	r0, [sp, #12]	@ zero_extendqisi2
	add	r1, r9, r1
	blx	r2
	cmp	r0, #0
	strne	r10, [fp]
.L445:
	add	r7, r7, #1
	add	fp, fp, #20
	b	.L437
.L448:
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L455:
	.align	2
.L454:
	.word	.LANCHOR0
	.word	__stack_chk_guard
	.word	.LANCHOR1+130
	.word	.LC86
	.word	.LC87
	.word	.LC88
	.fnend
	.size	FlashEraseBlocks, .-FlashEraseBlocks
	.align	2
	.global	FtlFreeSysBlkQueueIn
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBlkQueueIn, %function
FtlFreeSysBlkQueueIn:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	sub	r3, r0, #1
	movw	r2, #65533
	uxth	r3, r3
	cmp	r3, r2
	bxhi	lr
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	ldr	r6, .L469
	add	r4, r6, #412
	ldrh	r3, [r4, #6]
	cmp	r3, #1024
	popeq	{r4, r5, r6, r7, r8, pc}
	cmp	r1, #0
	mov	r5, r0
	beq	.L458
	bl	P2V_block_in_plane
	mov	r7, r0
	ldr	r0, [r6, #3268]
	lsl	r3, r5, #10
	mov	r2, #1
	mov	r1, r2
	str	r3, [r0, #4]
	bl	FlashEraseBlocks
	ldr	r2, [r6, #2536]
	lsl	r0, r7, #1
	ldrh	r3, [r2, r0]
	add	r3, r3, #1
	strh	r3, [r2, r0]	@ movhi
	ldr	r3, [r6, #2608]
	add	r3, r3, #1
	str	r3, [r6, #2608]
.L458:
	ldrh	r3, [r4, #6]
	add	r3, r3, #1
	strh	r3, [r4, #6]	@ movhi
	ldrh	r3, [r4, #4]
	add	r2, r4, r3, lsl #1
	add	r3, r3, #1
	ubfx	r3, r3, #0, #10
	strh	r5, [r2, #8]	@ movhi
	strh	r3, [r4, #4]	@ movhi
	pop	{r4, r5, r6, r7, r8, pc}
.L470:
	.align	2
.L469:
	.word	.LANCHOR0
	.fnend
	.size	FtlFreeSysBlkQueueIn, .-FtlFreeSysBlkQueueIn
	.align	2
	.global	FtlFreeSysBlkQueueOut
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBlkQueueOut, %function
FtlFreeSysBlkQueueOut:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	ldr	r6, .L477
	add	r4, r6, #412
	ldrh	r2, [r4, #6]
	cmp	r2, #0
	movweq	r5, #65535
	beq	.L472
	ldrh	r3, [r4, #2]
	sub	r2, r2, #1
	ldr	r0, [r6, #3268]
	strh	r2, [r4, #6]	@ movhi
	mov	r2, #1
	add	r1, r4, r3, lsl #1
	add	r3, r3, #1
	ubfx	r3, r3, #0, #10
	ldrh	r5, [r1, #8]
	mov	r1, r2
	strh	r3, [r4, #2]	@ movhi
	lsl	r3, r5, #10
	str	r3, [r0, #4]
	bl	FlashEraseBlocks
	ldr	r3, [r6, #2608]
	add	r3, r3, #1
	str	r3, [r6, #2608]
.L472:
	sub	r3, r5, #1
	movw	r2, #65533
	uxth	r3, r3
	cmp	r3, r2
	bls	.L473
	ldrh	r2, [r4, #6]
	mov	r1, r5
	ldr	r0, .L477+4
	bl	sftl_printk
.L474:
	b	.L474
.L473:
	mov	r0, r5
	pop	{r4, r5, r6, pc}
.L478:
	.align	2
.L477:
	.word	.LANCHOR0
	.word	.LC89
	.fnend
	.size	FtlFreeSysBlkQueueOut, .-FtlFreeSysBlkQueueOut
	.align	2
	.global	ftl_map_blk_alloc_new_blk
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_map_blk_alloc_new_blk, %function
ftl_map_blk_alloc_new_blk:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r3, #0
	ldrh	r1, [r0, #10]
	mov	r4, r0
	ldr	r2, [r0, #12]
.L480:
	uxth	r5, r3
	cmp	r5, r1
	bcs	.L483
	mov	r7, r2
	add	r3, r3, #1
	ldrh	r6, [r7]
	add	r2, r2, #2
	cmp	r6, #0
	bne	.L480
	bl	FtlFreeSysBlkQueueOut
	sub	r3, r0, #1
	movw	r2, #65533
	uxth	r3, r3
	mov	r1, r0
	strh	r0, [r7]	@ movhi
	cmp	r3, r2
	bls	.L481
	ldr	r3, .L488
	ldr	r0, .L488+4
	ldrh	r2, [r3, #6]
	bl	sftl_printk
.L482:
	b	.L482
.L481:
	ldr	r3, [r4, #28]
	strh	r6, [r4, #2]	@ movhi
	strh	r5, [r4]	@ movhi
	add	r3, r3, #1
	str	r3, [r4, #28]
	ldrh	r3, [r4, #8]
	add	r3, r3, #1
	strh	r3, [r4, #8]	@ movhi
.L483:
	ldrh	r3, [r4, #10]
	cmp	r3, r5
	bhi	.L485
	movw	r2, #578
	ldr	r1, .L488+8
	ldr	r0, .L488+12
	bl	sftl_printk
.L485:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, pc}
.L489:
	.align	2
.L488:
	.word	.LANCHOR0+412
	.word	.LC90
	.word	.LANCHOR1+147
	.word	.LC8
	.fnend
	.size	ftl_map_blk_alloc_new_blk, .-ftl_map_blk_alloc_new_blk
	.align	2
	.global	ftl_memset
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_memset, %function
ftl_memset:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	b	memset
	.fnend
	.size	ftl_memset, .-ftl_memset
	.align	2
	.global	FtlMemInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlMemInit, %function
FtlMemInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	movw	r3, #65535
	ldr	r4, .L593
	movw	r6, #306
	mov	r2, #32
	mov	r5, #0
	mov	r7, #12
	str	r3, [r4, #3276]
	mvn	r3, #0
	ldrh	r0, [r4, r6]
	add	r9, r4, #312
	strh	r3, [r4, #222]	@ movhi
	strh	r3, [r4, #220]	@ movhi
	add	r3, r4, #3152
	strh	r2, [r3]	@ movhi
	mov	r2, #128
	movw	r3, #3154
	lsl	r0, r0, #1
	strh	r2, [r4, r3]	@ movhi
	movw	r3, #3160
	strh	r5, [r4, r3]	@ movhi
	movw	r3, #3206
	strh	r5, [r4, r3]	@ movhi
	movw	r3, #3242
	strh	r5, [r4, r3]	@ movhi
	str	r5, [r4, #2588]
	str	r5, [r4, #2592]
	str	r5, [r4, #2576]
	str	r5, [r4, #2564]
	str	r5, [r4, #2560]
	str	r5, [r4, #2568]
	str	r5, [r4, #2572]
	str	r5, [r4, #2556]
	str	r5, [r4, #2596]
	str	r5, [r4, #2600]
	str	r5, [r4, #2608]
	str	r5, [r4, #2612]
	str	r5, [r4, #2616]
	str	r5, [r4, #3272]
	str	r5, [r4, #3164]
	str	r5, [r4, #3280]
	str	r5, [r4, #3156]
	str	r5, [r4, #3284]
	bl	ftl_malloc
	str	r0, [r4, #3192]
	ldrh	r0, [r4, r6]
	mov	r6, #20
	mul	r0, r7, r0
	bl	ftl_malloc
	ldrh	r3, [r4, #232]
	str	r0, [r4, #3200]
	mul	r6, r6, r3
	lsl	r8, r6, #2
	mov	r0, r8
	bl	ftl_malloc
	str	r0, [r4, #3288]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3292]
	mov	r0, r8
	bl	ftl_malloc
	str	r0, [r4, #3296]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3268]
	mov	r0, r6
	bl	ftl_malloc
	ldrh	r3, [r4, #232]
	movw	r8, #310
	ldrh	r6, [r4, r8]
	str	r0, [r4, #3184]
	lsl	r3, r3, #1
	mov	r0, r6
	add	r3, r3, #1
	str	r3, [r4, #3188]
	bl	ftl_malloc
	str	r0, [r4, #3300]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3304]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3308]
	ldr	r0, [r4, #3188]
	mul	r0, r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3176]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3312]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3316]
	ldr	r0, [r4, #3188]
	mul	r0, r7, r0
	bl	ftl_malloc
	str	r0, [r4, #3172]
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3320]
	mov	r0, r6
	bl	ftl_malloc
	movw	r3, #258
	str	r0, [r4, #3324]
	ldrh	r0, [r4, r3]
	lsl	r0, r0, #2
	bl	ftl_malloc
	ldrh	r3, [r9]
	ldrh	r6, [r4, #232]
	str	r0, [r4, #3328]
	mul	r6, r6, r3
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3332]
	lsl	r0, r6, #2
	movw	r6, #3340
	bl	ftl_malloc
	ldrh	r3, [r9]
	str	r0, [r4, #3336]
	ldr	r0, [r4, #3188]
	mul	r0, r0, r3
	bl	ftl_malloc
	str	r0, [r4, #3180]
	ldrh	r0, [r4, #242]
	lsl	r0, r0, #1
	uxth	r0, r0
	strh	r0, [r4, r6]	@ movhi
	bl	ftl_malloc
	str	r0, [r4, #3344]
	ldrh	r0, [r4, r6]
	ldr	r3, .L593+4
	add	r0, r0, #544
	add	r0, r0, #3
	lsr	r0, r0, #9
	strh	r0, [r4, r6]	@ movhi
	and	r0, r3, r0, lsl #9
	bl	ftl_malloc
	ldrh	r6, [r4, #242]
	str	r0, [r4, #3348]
	add	r0, r0, #32
	str	r0, [r4, #2536]
	lsl	r6, r6, #1
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3352]
	mov	r0, r6
	bl	ftl_malloc
	ldr	r6, [r4, #328]
	str	r0, [r4, #72]
	lsl	r6, r6, #1
	mov	r0, r6
	bl	ftl_malloc
	str	r0, [r4, #3356]
	mov	r0, r6
	add	r6, r4, #320
	bl	ftl_malloc
	str	r0, [r4, #3360]
	ldrh	r0, [r6]
	lsl	r0, r0, #1
	bl	ftl_malloc
	str	r0, [r4, #348]
	ldrh	r0, [r6]
	lsl	r0, r0, #1
	bl	ftl_malloc
	str	r0, [r4, #3364]
	ldrh	r0, [r6]
	movw	r6, #322
	lsl	r0, r0, #2
	bl	ftl_malloc
	str	r0, [r4, #3368]
	ldrh	r0, [r4, r6]
	lsl	r0, r0, #2
	bl	ftl_malloc
	ldrh	r2, [r4, r6]
	mov	r1, r5
	str	r0, [r4, #3372]
	movw	r5, #338
	lsl	r2, r2, #2
	bl	ftl_memset
	add	r3, r4, #336
	ldrh	r0, [r3]
	lsl	r0, r0, #2
	bl	ftl_malloc
	str	r0, [r4, #3376]
	ldr	r0, [r4, #328]
	lsl	r0, r0, #2
	bl	ftl_malloc
	str	r0, [r4, #3380]
	ldrh	r0, [r4, r5]
	mul	r0, r7, r0
	bl	ftl_malloc
	ldrh	r3, [r4, r5]
	movw	r5, #3388
	str	r0, [r4, #2540]
	ldrh	r0, [r4, r8]
	mul	r0, r0, r3
	bl	ftl_malloc
	ldrh	r3, [r4, #242]
	str	r0, [r4, #3384]
	mov	r0, #6
	mul	r0, r0, r3
	bl	ftl_malloc
	movw	r3, #298
	str	r0, [r4, #2516]
	ldrh	r0, [r4, r3]
	ldrh	r3, [r4, #254]
	add	r0, r0, #31
	asr	r0, r0, #5
	strh	r0, [r4, r5]	@ movhi
	mul	r0, r0, r3
	lsl	r0, r0, #2
	bl	ftl_malloc
	ldrh	r2, [r4, r5]
	add	ip, r4, #380
	ldrh	lr, [r4, #254]
	mov	r3, #1
	str	r0, [r4, #380]
	lsl	r2, r2, #2
	mov	r1, r2
.L492:
	cmp	r3, lr
	bcc	.L493
	ldr	r2, .L593+8
	mov	r1, #0
	add	r3, r2, r3, lsl #2
	add	r2, r2, #56
	add	r3, r3, #24
.L494:
	cmp	r2, r3
	bne	.L495
	ldr	r3, [r4, #3356]
	cmp	r3, #0
	bne	.L496
.L498:
	ldr	r1, .L593+12
	ldr	r0, .L593+16
	bl	sftl_printk
	mvn	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L493:
	ldr	r0, [r4, #380]
	add	r3, r3, #1
	add	r0, r0, r1
	add	r1, r1, r2
	str	r0, [ip, #4]!
	b	.L492
.L495:
	str	r1, [r3, #4]!
	b	.L494
.L496:
	ldr	r3, [r4, #3360]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3376]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3380]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #2540]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3384]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #2516]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #380]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #72]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3192]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3200]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3288]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3296]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3268]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3184]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3292]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3300]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3304]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3308]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3176]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3312]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3316]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3172]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3332]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3336]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3180]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #2536]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #3344]
	cmp	r3, #0
	beq	.L498
	ldr	r3, [r4, #348]
	cmp	r3, #0
	beq	.L498
	ldr	r3, .L593
	ldr	r2, [r3, #3364]
	cmp	r2, #0
	beq	.L498
	ldr	r2, [r3, #3368]
	cmp	r2, #0
	beq	.L498
	ldr	r3, [r3, #3372]
	cmp	r3, #0
	beq	.L498
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L594:
	.align	2
.L593:
	.word	.LANCHOR0
	.word	33553920
	.word	.LANCHOR0+352
	.word	.LANCHOR1+173
	.word	.LC91
	.fnend
	.size	FtlMemInit, .-FtlMemInit
	.align	2
	.global	FtlBbt2Bitmap
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbt2Bitmap, %function
FtlBbt2Bitmap:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r2, .L602
	movw	r3, #3388
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r4, r0
	ldr	r7, .L602+4
	sub	r5, r4, #2
	add	r4, r4, #1020
	mov	r6, r1
	ldrh	r2, [r2, r3]
	add	r4, r4, #2
	ldr	r8, .L602+8
	mov	r1, #0
	ldr	r9, .L602+12
	mov	r0, r6
	lsl	r2, r2, #2
	bl	ftl_memset
.L598:
	ldrh	r3, [r5, #2]
	movw	r2, #65535
	cmp	r3, r2
	popeq	{r4, r5, r6, r7, r8, r9, r10, pc}
	ldrh	r2, [r7]
	cmp	r2, r3
	bhi	.L597
	mov	r2, #74
	mov	r1, r8
	mov	r0, r9
	bl	sftl_printk
.L597:
	ldrh	r3, [r5, #2]!
	mov	r0, #1
	cmp	r4, r5
	lsr	r1, r3, #5
	and	r3, r3, #31
	ldr	r2, [r6, r1, lsl #2]
	orr	r3, r2, r0, lsl r3
	str	r3, [r6, r1, lsl #2]
	bne	.L598
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L603:
	.align	2
.L602:
	.word	.LANCHOR0
	.word	.LANCHOR0+298
	.word	.LANCHOR1+184
	.word	.LC8
	.fnend
	.size	FtlBbt2Bitmap, .-FtlBbt2Bitmap
	.align	2
	.global	FtlBbtMemInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbtMemInit, %function
FtlBbtMemInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r0, .L605
	mvn	r2, #0
	mov	r1, #255
	add	r3, r0, #352
	add	r0, r0, #364
	strh	r2, [r3]	@ movhi
	mov	r2, #0
	strh	r2, [r3, #6]	@ movhi
	mov	r2, #16
	b	ftl_memset
.L606:
	.align	2
.L605:
	.word	.LANCHOR0
	.fnend
	.size	FtlBbtMemInit, .-FtlBbtMemInit
	.align	2
	.global	FtlFreeSysBlkQueueInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlFreeSysBlkQueueInit, %function
FtlFreeSysBlkQueueInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	ip, .L609
	mov	r2, #2048
	push	{r4, lr}
	.save {r4, lr}
	mov	r4, #0
	mov	r1, r4
	add	r3, ip, #412
	strh	r0, [r3]	@ movhi
	add	r0, ip, #420
	strh	r4, [r3, #2]	@ movhi
	strh	r4, [r3, #4]	@ movhi
	strh	r4, [r3, #6]	@ movhi
	bl	ftl_memset
	mov	r0, r4
	pop	{r4, pc}
.L610:
	.align	2
.L609:
	.word	.LANCHOR0
	.fnend
	.size	FtlFreeSysBlkQueueInit, .-FtlFreeSysBlkQueueInit
	.align	2
	.global	ftl_free_no_use_map_blk
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_free_no_use_map_blk, %function
ftl_free_no_use_map_blk:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r1, #0
	ldrh	r2, [r0, #10]
	mov	r4, r0
	ldr	r5, [r0, #20]
	ldr	r7, [r0, #12]
	ldr	r6, [r0, #24]
	lsl	r2, r2, #1
	mov	r0, r5
	bl	ftl_memset
	mov	r2, #0
.L612:
	ldrh	r1, [r4, #6]
	uxth	r3, r2
	cmp	r1, r3
	bhi	.L616
	ldr	r3, .L632
	mov	r6, #0
	mov	r8, r6
	mov	r10, r6
	ldrh	r2, [r3]
	ldrh	r3, [r4]
	lsl	r3, r3, #1
	strh	r2, [r5, r3]	@ movhi
	ldrh	r9, [r5]
.L617:
	ldrh	r3, [r4, #10]
	uxth	r1, r6
	cmp	r3, r1
	bhi	.L621
	mov	r0, r8
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L616:
	uxth	r3, r2
	mov	r1, #0
	ldr	r0, [r6, r3, lsl #2]
	ubfx	r0, r0, #10, #16
.L613:
	ldrh	ip, [r4, #10]
	uxth	r3, r1
	cmp	ip, r3
	addls	r2, r2, #1
	bls	.L612
.L615:
	uxth	r3, r1
	add	r1, r1, #1
	lsl	r3, r3, #1
	ldrh	ip, [r7, r3]
	adds	lr, ip, #0
	movne	lr, #1
	cmp	r0, ip
	movne	lr, #0
	cmp	lr, #0
	ldrhne	ip, [r5, r3]
	addne	ip, ip, #1
	strhne	ip, [r5, r3]	@ movhi
	b	.L613
.L621:
	uxth	r3, r6
	lsl	r3, r3, #1
	ldrh	r2, [r5, r3]
	cmp	r9, r2
	bls	.L618
	ldrh	r0, [r7, r3]
	add	fp, r7, r3
	cmp	r0, #0
	bne	.L619
.L620:
	add	r6, r6, #1
	b	.L617
.L618:
	cmp	r2, #0
	bne	.L620
	ldrh	r0, [r7, r3]
	add	fp, r7, r3
	cmp	r0, #0
	beq	.L620
.L622:
	mov	r1, #1
	bl	FtlFreeSysBlkQueueIn
	strh	r10, [fp]	@ movhi
	ldrh	r3, [r4, #8]
	sub	r3, r3, #1
	strh	r3, [r4, #8]	@ movhi
	b	.L620
.L619:
	subs	r9, r2, #0
	mov	r8, r1
	beq	.L622
	b	.L620
.L633:
	.align	2
.L632:
	.word	.LANCHOR0+304
	.fnend
	.size	ftl_free_no_use_map_blk, .-ftl_free_no_use_map_blk
	.align	2
	.global	FtlL2PDataInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlL2PDataInit, %function
FtlL2PDataInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r1, #0
	ldr	r4, .L638
	mvn	r5, #0
	ldr	r2, [r4, #328]
	ldr	r0, [r4, #3360]
	lsl	r2, r2, #1
	bl	ftl_memset
	movw	r3, #310
	movw	r2, #338
	ldrh	r3, [r4, r3]
	mov	r1, #255
	ldrh	r2, [r4, r2]
	ldr	r0, [r4, #3384]
	mul	r2, r2, r3
	bl	ftl_memset
	ldr	r0, .L638+4
	mov	r2, #0
	mov	r3, r4
	mov	lr, #12
	mov	r4, r2
	sub	r6, r0, #28
.L635:
	ldrh	r7, [r0]
	uxth	r1, r2
	add	ip, r2, #1
	cmp	r7, r1
	bhi	.L636
	ldr	r2, .L638+8
	mvn	r1, #0
	strh	r1, [r2, #2]	@ movhi
	strh	r1, [r2]	@ movhi
	ldr	r1, [r3, #328]
	strh	r1, [r2, #10]	@ movhi
	ldr	r1, .L638+12
	strh	r1, [r2, #4]	@ movhi
	movw	r1, #3436
	ldrh	r1, [r3, r1]
	strh	r1, [r2, #8]	@ movhi
	sub	r1, r2, #3056
	ldrh	r1, [r1]
	strh	r1, [r2, #6]	@ movhi
	ldr	r2, [r3, #3356]
	str	r2, [r3, #3404]
	ldr	r2, [r3, #3380]
	str	r2, [r3, #3408]
	ldr	r2, [r3, #3360]
	str	r2, [r3, #3412]
	ldr	r2, [r3, #3376]
	str	r2, [r3, #3416]
	pop	{r4, r5, r6, r7, r8, pc}
.L636:
	uxth	r2, r2
	ldr	r1, [r3, #2540]
	mul	r7, lr, r2
	add	r8, r1, r7
	str	r4, [r8, #4]
	strh	r5, [r1, r7]	@ movhi
	ldr	r1, [r3, #2540]
	add	r1, r1, r7
	ldrh	r7, [r6]
	mul	r2, r2, r7
	ldr	r7, [r3, #3384]
	bic	r2, r2, #3
	add	r2, r7, r2
	str	r2, [r1, #8]
	mov	r2, ip
	b	.L635
.L639:
	.align	2
.L638:
	.word	.LANCHOR0
	.word	.LANCHOR0+338
	.word	.LANCHOR0+3392
	.word	-3902
	.fnend
	.size	FtlL2PDataInit, .-FtlL2PDataInit
	.align	2
	.global	FtlVariablesInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlVariablesInit, %function
FtlVariablesInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mvn	r3, #0
	ldr	r4, .L642
	movw	r2, #3438
	mov	r5, #0
	mov	r1, r5
	strh	r3, [r4, r2]	@ movhi
	str	r3, [r4, #3448]
	add	r3, r4, #344
	strh	r5, [r3]	@ movhi
	add	r3, r4, #320
	ldrh	r2, [r3]
	ldr	r0, [r4, #348]
	str	r5, [r4, #3440]
	str	r5, [r4, #3444]
	lsl	r2, r2, #1
	bl	ftl_memset
	ldrh	r2, [r4, #242]
	mov	r1, r5
	ldr	r0, [r4, #2536]
	lsl	r2, r2, #1
	bl	ftl_memset
	ldrh	r2, [r4, #242]
	mov	r1, r5
	ldr	r0, [r4, #3344]
	lsl	r2, r2, #1
	bl	ftl_memset
	add	r0, r4, #2464
	mov	r1, r5
	mov	r2, #48
	add	r0, r0, #4
	bl	ftl_memset
	mov	r2, #512
	mov	r1, r5
	add	r0, r4, #2640
	bl	ftl_memset
	bl	FtlGcBufInit
	bl	FtlL2PDataInit
	mov	r0, r5
	pop	{r4, r5, r6, pc}
.L643:
	.align	2
.L642:
	.word	.LANCHOR0
	.fnend
	.size	FtlVariablesInit, .-FtlVariablesInit
	.align	2
	.global	SupperBlkListInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	SupperBlkListInit, %function
SupperBlkListInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r2, #6
	ldr	r4, .L656
	mov	r5, #0
	.pad #20
	sub	sp, sp, #20
	mov	r1, #0
	ldr	r10, .L656+4
	mov	r9, r5
	ldrh	r3, [r4, #242]
	mov	r6, r5
	ldr	r0, [r4, #2516]
	add	fp, r4, #260
	mul	r2, r2, r3
	bl	ftl_memset
	add	r3, r4, #2528
	str	r5, [r4, #2532]
	str	r5, [r4, #2520]
	str	r5, [r4, #2524]
	strh	r5, [r3]	@ movhi
	strh	r5, [r4, #224]	@ movhi
	str	r3, [sp, #4]
.L645:
	ldrh	r3, [r4, #240]
	uxth	r8, r5
	cmp	r8, r3
	bcs	.L652
	ldrh	r3, [r10]
	ldrh	r2, [r4, #232]
	str	r3, [sp]
	mov	r3, #0
	mov	r7, r3
	b	.L653
.L647:
	mov	r1, r8
	ldrb	r0, [fp, r3]	@ zero_extendqisi2
	str	r2, [sp, #12]
	str	r3, [sp, #8]
	bl	V2P_block
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	ldr	r3, [sp, #8]
	ldreq	r1, [sp]
	ldr	r2, [sp, #12]
	add	r3, r3, #1
	addeq	r7, r7, r1
	uxtheq	r7, r7
.L653:
	uxth	r1, r3
	cmp	r2, r1
	bhi	.L647
	cmp	r7, #0
	uxth	r3, r5
	beq	.L648
	mov	r1, r7
	mov	r0, #32768
	str	r3, [sp]
	bl	__aeabi_idiv
	ldr	r3, [sp]
	uxth	r7, r0
.L649:
	ldr	r1, [r4, #2516]
	mov	r2, #6
	mla	r2, r2, r3, r1
	strh	r7, [r2, #4]	@ movhi
	ldrh	r2, [r4, #24]
	cmp	r2, r8
	beq	.L650
	ldrh	r2, [r4, #76]
	cmp	r2, r8
	beq	.L650
	ldrh	r2, [r4, #124]
	cmp	r2, r8
	beq	.L650
	ldr	r2, [r4, #72]
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	bne	.L651
	add	r9, r9, #1
	mov	r0, r8
	uxth	r9, r9
	bl	INSERT_FREE_LIST
.L650:
	add	r5, r5, #1
	b	.L645
.L648:
	ldr	r1, [r4, #72]
	lsl	r2, r3, #1
	mvn	r0, #0
	strh	r0, [r1, r2]	@ movhi
	b	.L649
.L651:
	add	r6, r6, #1
	mov	r0, r8
	uxth	r6, r6
	bl	INSERT_DATA_LIST
	b	.L650
.L652:
	ldr	r2, [sp, #4]
	strh	r9, [r4, #224]	@ movhi
	strh	r6, [r2]	@ movhi
	add	r6, r6, r9
	cmp	r6, r3
	ble	.L654
	movw	r2, #2219
	ldr	r1, .L656+8
	ldr	r0, .L656+12
	bl	sftl_printk
.L654:
	mov	r0, #0
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L657:
	.align	2
.L656:
	.word	.LANCHOR0
	.word	.LANCHOR0+302
	.word	.LANCHOR1+198
	.word	.LC8
	.fnend
	.size	SupperBlkListInit, .-SupperBlkListInit
	.align	2
	.global	FtlGcPageVarInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcPageVarInit, %function
FtlGcPageVarInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r3, #0
	ldr	r4, .L660
	movw	r2, #3196
	movw	r5, #306
	mov	r1, #255
	strh	r3, [r4, r2]	@ movhi
	movw	r2, #3204
	strh	r3, [r4, r2]	@ movhi
	ldrh	r2, [r4, r5]
	ldr	r0, [r4, #3192]
	lsl	r2, r2, #1
	bl	ftl_memset
	ldrh	r3, [r4, r5]
	mov	r2, #12
	ldr	r0, [r4, #3200]
	mov	r1, #255
	mul	r2, r2, r3
	bl	ftl_memset
	pop	{r4, r5, r6, lr}
	b	FtlGcBufInit
.L661:
	.align	2
.L660:
	.word	.LANCHOR0
	.fnend
	.size	FtlGcPageVarInit, .-FtlGcPageVarInit
	.align	2
	.global	FlashGetBadBlockList
	.syntax unified
	.arm
	.fpu softvfp
	.type	FlashGetBadBlockList, %function
FlashGetBadBlockList:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r2, #256
	ldr	r5, .L670
	mov	r4, r0
	mov	r6, r1
	mov	r1, #255
	bl	ftl_memset
	ldr	r3, [r5, #3244]
	mov	r1, r6
	mov	r0, r4
	blx	r3
	uxth	r0, r0
	cmp	r0, #50
	bls	.L663
	mov	r2, #256
	mov	r1, #255
	mov	r0, r4
	bl	ftl_memset
	mov	r0, #0
.L663:
	ldrh	r3, [r5, #14]
	cmp	r3, #4
	moveq	r3, r4
	addeq	r1, r3, r0, lsl #1
	beq	.L665
	pop	{r4, r5, r6, pc}
.L666:
	ldrh	r2, [r3]
	lsr	r2, r2, #1
	strh	r2, [r3], #2	@ movhi
.L665:
	cmp	r3, r1
	bne	.L666
	pop	{r4, r5, r6, pc}
.L671:
	.align	2
.L670:
	.word	.LANCHOR0
	.fnend
	.size	FlashGetBadBlockList, .-FlashGetBadBlockList
	.align	2
	.global	ftl_memcpy
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_memcpy, %function
ftl_memcpy:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	b	memcpy
	.fnend
	.size	ftl_memcpy, .-ftl_memcpy
	.align	2
	.global	FlashReadPages
	.syntax unified
	.arm
	.fpu softvfp
	.type	FlashReadPages, %function
FlashReadPages:
	.fnstart
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L714
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #28
	sub	sp, sp, #28
	ldr	r5, .L714+4
	mov	r8, r1
	mov	r4, r0
	mov	r7, #0
	ldr	r2, [r3]
	mov	fp, r3
	ldr	r9, .L714+8
	ldr	r10, .L714+12
	str	r2, [sp, #20]
	ldrh	r2, [r5, #12]
	str	r2, [sp, #4]
.L674:
	cmp	r7, r8
	bne	.L687
	ldr	r2, [sp, #20]
	mov	r0, #0
	ldr	r3, [fp]
	cmp	r2, r3
	beq	.L688
	bl	__stack_chk_fail
.L687:
	ldr	r3, [r4, #8]
	cmp	r3, #0
	beq	.L675
	ldr	r3, [r4, #12]
	cmp	r3, #0
	bne	.L676
.L675:
	mov	r2, #96
	ldr	r1, .L714+16
	mov	r0, r9
	bl	sftl_printk
.L676:
	add	r2, sp, #12
	add	r1, sp, #16
	mov	r0, r4
	bl	l2p_addr_tran
	ldr	r0, [sp, #12]
	cmp	r0, #3
	mvnhi	r3, #0
	strhi	r3, [r4]
	bhi	.L678
	ldr	r6, [r4, #8]
	uxtb	r0, r0
	ldr	r3, [r4, #12]
	ldr	r1, [sp, #16]
	tst	r6, #63
	ldr	ip, [r5, #3256]
	ldrne	r6, [r5, #3320]
	mov	r2, r6
	blx	ip
	str	r0, [r4]
	ldrh	r3, [r5, #14]
	cmp	r3, #4
	bne	.L681
	ldr	r0, [sp, #4]
	add	r2, r6, #2048
	ldr	r3, [r4, #12]
	ldr	r1, [sp, #16]
	ldr	ip, [r5, #3256]
	add	r3, r3, #8
	add	r1, r0, r1
	ldrb	r0, [sp, #12]	@ zero_extendqisi2
	blx	ip
	cmn	r0, #1
	mov	r1, r0
	beq	.L682
	ldr	r3, [r4, #12]
	ldr	r2, [r3, #12]
	cmn	r2, #1
	bne	.L683
	ldr	r2, [r3, #8]
	cmn	r2, #1
	bne	.L683
	ldr	r3, [r3]
	cmn	r3, #1
	beq	.L683
.L682:
	mvn	r3, #0
	str	r3, [r4]
.L683:
	ldr	r3, [r4]
	sub	r0, r1, #256
	clz	r0, r0
	lsr	r0, r0, #5
	cmn	r3, #1
	moveq	r0, #0
	cmp	r0, #0
	movne	r3, #256
	strne	r3, [r4]
	ldr	r3, [r4]
	cmn	r3, #1
	cmpne	r3, #256
	bne	.L681
	ldr	r1, [r4, #4]
	ldr	r2, [sp, #16]
	ldr	r0, .L714+20
	bl	sftl_printk
	ldr	r1, [r4, #8]
	cmp	r1, #0
	beq	.L686
	mov	r3, #4
	ldr	r0, .L714+24
	mov	r2, r3
	bl	rknand_print_hex
.L686:
	ldr	r1, [r4, #12]
	cmp	r1, #0
	beq	.L681
	mov	r3, #4
	ldr	r0, .L714+28
	mov	r2, r3
	bl	rknand_print_hex
.L681:
	ldr	r3, [r5, #3320]
	cmp	r6, r3
	bne	.L678
	ldr	r0, [r4, #8]
	cmp	r6, r0
	beq	.L678
	ldrh	r2, [r10]
	mov	r1, r6
	lsl	r2, r2, #9
	bl	ftl_memcpy
.L678:
	add	r7, r7, #1
	add	r4, r4, #20
	b	.L674
.L688:
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L715:
	.align	2
.L714:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LC8
	.word	.LANCHOR0+258
	.word	.LANCHOR1+216
	.word	.LC92
	.word	.LC93
	.word	.LC94
	.fnend
	.size	FlashReadPages, .-FlashReadPages
	.align	2
	.global	FtlLoadFactoryBbt
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadFactoryBbt, %function
FtlLoadFactoryBbt:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r7, #0
	ldr	r5, .L727
	ldr	r10, .L727+4
	add	r6, r5, #360
	ldr	r3, [r5, #3300]
	ldr	r8, [r5, #3332]
	add	r6, r6, #2
	sub	r9, r6, #64
	str	r3, [r5, #3460]
	str	r8, [r5, #3464]
.L717:
	ldrh	r3, [r5, #254]
	cmp	r7, r3
	bcc	.L722
	mov	r0, #0
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L722:
	ldrh	r4, [r9]
	mvn	r3, #0
	movw	fp, #61664
	strh	r3, [r6, #2]!	@ movhi
	add	r4, r4, r3
	uxth	r4, r4
.L718:
	ldrh	r3, [r9]
	sub	r2, r3, #16
	cmp	r4, r2
	ble	.L720
	mla	r3, r7, r3, r4
	mov	r2, #1
	mov	r1, r2
	mov	r0, r10
	lsl	r3, r3, #10
	str	r3, [r5, #3456]
	bl	FlashReadPages
	ldr	r3, [r5, #3452]
	cmn	r3, #1
	beq	.L719
	ldrh	r3, [r8]
	cmp	r3, fp
	bne	.L719
	strh	r4, [r6]	@ movhi
.L720:
	add	r7, r7, #1
	b	.L717
.L719:
	sub	r4, r4, #1
	uxth	r4, r4
	b	.L718
.L728:
	.align	2
.L727:
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.fnend
	.size	FtlLoadFactoryBbt, .-FtlLoadFactoryBbt
	.align	2
	.global	FtlGetLastWrittenPage
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGetLastWrittenPage, %function
FtlGetLastWrittenPage:
	.fnstart
	@ args = 0, pretend = 0, frame = 88
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	.pad #92
	sub	sp, sp, #92
	ldr	r6, .L742
	cmp	r1, #1
	movwne	r2, #302
	lsl	r8, r0, #10
	mov	r7, r1
	ldr	r3, [r6]
	mov	r0, sp
	str	r3, [sp, #84]
	ldr	r3, .L742+4
	addeq	r2, r3, #304
	ldrhne	r5, [r3, r2]
	ldrheq	r5, [r2]
	mov	r2, r1
	ldr	r3, [r3, #3324]
	mov	r1, #1
	sub	r5, r5, #1
	sxth	r5, r5
	str	r3, [sp, #8]
	add	r3, sp, #20
	str	r3, [sp, #12]
	orr	r3, r5, r8
	str	r3, [sp, #4]
	bl	FlashReadPages
	ldr	r3, [sp, #20]
	cmn	r3, #1
	moveq	r9, #0
	beq	.L733
.L732:
	ldr	r2, [sp, #84]
	mov	r0, r5
	ldr	r3, [r6]
	cmp	r2, r3
	beq	.L737
	bl	__stack_chk_fail
.L736:
	add	r3, r9, r5
	mov	r2, r7
	add	r3, r3, r3, lsr #31
	mov	r1, #1
	mov	r0, sp
	asr	r4, r3, #1
	sxth	r3, r4
	orr	r3, r3, r8
	str	r3, [sp, #4]
	bl	FlashReadPages
	ldr	r3, [sp, #20]
	cmn	r3, #1
	bne	.L734
	ldr	r3, [sp, #24]
	cmn	r3, #1
	bne	.L734
	ldr	r3, [sp]
	cmn	r3, #1
	subne	r4, r4, #1
	sxthne	r5, r4
	bne	.L733
.L734:
	add	r4, r4, #1
	sxth	r9, r4
.L733:
	cmp	r9, r5
	ble	.L736
	b	.L732
.L737:
	add	sp, sp, #92
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L743:
	.align	2
.L742:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.fnend
	.size	FtlGetLastWrittenPage, .-FtlGetLastWrittenPage
	.align	2
	.global	FtlScanSysBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlScanSysBlk, %function
FtlScanSysBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r5, #0
	ldr	r4, .L826
	movw	r3, #3436
	mov	r1, r5
	.pad #36
	sub	sp, sp, #36
	ldr	r2, [r4, #328]
	add	r7, r4, #344
	ldr	r0, [r4, #3380]
	add	r6, r4, #320
	strh	r5, [r4, r3]	@ movhi
	strh	r5, [r7]	@ movhi
	lsl	r2, r2, #2
	bl	ftl_memset
	ldr	r2, [r4, #328]
	mov	r1, r5
	ldr	r0, [r4, #3356]
	lsl	r2, r2, #1
	bl	ftl_memset
	ldrh	r2, [r6]
	mov	r1, r5
	ldr	r0, [r4, #3368]
	lsl	r2, r2, #2
	bl	ftl_memset
	ldrh	r2, [r6]
	mov	r1, r5
	ldr	r0, [r4, #348]
	add	r5, r4, #2624
	lsl	r2, r2, #1
	bl	ftl_memset
	mov	r2, #16
	mov	r1, #255
	add	r0, r4, #2624
	bl	ftl_memset
	ldrh	r3, [r4, #240]
	str	r7, [sp, #20]
	str	r6, [sp, #12]
	str	r3, [sp, #4]
.L745:
	ldrh	r3, [r4, #242]
	ldr	r2, [sp, #4]
	cmp	r3, r2
	bls	.L786
	ldr	r1, [r4, #3180]
	mov	r6, #0
	ldr	r7, .L826+4
	mov	fp, r6
	ldrh	r2, [r4, #232]
	mov	r9, #20
	ldr	r8, [r4, #3288]
	ldr	r3, [r4, #3176]
	str	r1, [sp, #8]
	ldrh	r10, [r7], #-52
	b	.L787
.L747:
	ldr	r1, [sp, #4]
	ldrb	r0, [r7, r6]	@ zero_extendqisi2
	str	r3, [sp, #28]
	str	r2, [sp, #24]
	bl	V2P_block
	str	r0, [sp, #16]
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	ldr	r2, [sp, #24]
	ldr	r3, [sp, #28]
	bne	.L746
	ldr	r1, [sp, #16]
	mla	r0, r9, fp, r8
	lsl	r1, r1, #10
	stmib	r0, {r1, r3}
	mul	r1, r10, fp
	add	ip, r1, #3
	cmp	r1, #0
	movlt	r1, ip
	ldr	ip, [sp, #8]
	bic	r1, r1, #3
	add	r1, ip, r1
	str	r1, [r0, #12]
	add	r1, fp, #1
	uxth	fp, r1
.L746:
	add	r6, r6, #1
.L787:
	uxth	r1, r6
	cmp	r2, r1
	bhi	.L747
	cmp	fp, #0
	bne	.L748
.L785:
	ldr	r3, [sp, #4]
	add	r3, r3, #1
	uxth	r3, r3
	str	r3, [sp, #4]
	b	.L745
.L748:
	mov	r0, r8
	ldr	r8, .L826+8
	mov	r2, #1
	mov	r1, fp
	bl	FlashReadPages
	mov	r3, #0
	str	r3, [sp, #8]
.L749:
	ldrh	r3, [sp, #8]
	cmp	fp, r3
	bls	.L785
	ldr	r3, [sp, #8]
	mov	r9, #20
	mul	r9, r9, r3
	ldr	r3, [r4, #3288]
	add	r2, r3, r9
	ldr	r3, [r3, r9]
	ldr	r6, [r2, #4]
	ldr	r7, [r2, #12]
	cmn	r3, #1
	ubfx	r6, r6, #10, #16
	bne	.L752
	mov	r10, #16
.L754:
	ldr	r0, [r4, #3288]
	mov	r2, #1
	mov	r1, r2
	add	r0, r0, r9
	ldr	r3, [r0, #4]
	add	r3, r3, #1
	str	r3, [r0, #4]
	bl	FlashReadPages
	ldrh	r3, [r7]
	movw	r2, #65535
	cmp	r3, r2
	ldr	r3, [r4, #3288]
	bne	.L751
	mvn	r2, #0
	str	r2, [r3, r9]
	ldr	r3, [r4, #3288]
	ldr	r3, [r3, r9]
	cmp	r3, r2
	bne	.L752
.L753:
	mov	r1, #1
	b	.L825
.L751:
	ldr	r3, [r3, r9]
	cmn	r3, #1
	bne	.L752
	sub	r10, r10, #1
	uxth	r10, r10
	cmp	r10, #0
	bne	.L754
	b	.L753
.L752:
	ldr	r2, [r4, #2588]
	ldr	r3, [r7, #4]
	cmn	r2, #1
	beq	.L755
	cmp	r2, r3
	bhi	.L756
.L755:
	cmn	r3, #1
	addne	r2, r3, #1
	strne	r2, [r4, #2588]
.L756:
	ldrh	r2, [r7]
	movw	r1, #61604
	cmp	r2, r1
	beq	.L758
	bhi	.L759
	movw	r3, #61574
	cmp	r2, r3
	beq	.L760
.L757:
	ldr	r3, [sp, #8]
	add	r3, r3, #1
	str	r3, [sp, #8]
	b	.L749
.L759:
	movw	r3, #61634
	cmp	r2, r3
	beq	.L761
	movw	r3, #65535
	cmp	r2, r3
	moveq	r1, #0
	bne	.L757
.L825:
	mov	r0, r6
	bl	FtlFreeSysBlkQueueIn
	b	.L757
.L761:
	ldrh	r2, [r8]
	ldr	r3, [r4, #328]
	cmp	r2, r3
	bls	.L763
	mov	r2, #1232
	ldr	r1, .L826+12
	ldr	r0, .L826+16
	bl	sftl_printk
.L763:
	ldr	r0, [r4, #328]
	ldrh	r2, [r8]
	ldr	ip, [r4, #3380]
	uxth	r1, r0
	sub	r3, r1, #1
	sub	r1, r1, r2
	sub	r1, r1, #1
	sxth	r3, r3
	sxth	r1, r1
.L764:
	cmp	r3, r1
	bgt	.L770
	cmp	r3, #0
	bge	.L802
	b	.L757
.L770:
	ldr	r10, [r7, #4]
	lsl	lr, r3, #2
	ldr	r9, [ip, r3, lsl #2]
	cmp	r10, r9
	bls	.L765
	ldr	r1, [ip]
	cmp	r1, #0
	bne	.L766
	cmp	r0, r2
	addne	r2, r2, #1
	strhne	r2, [r8]	@ movhi
.L766:
	uxth	ip, r3
	mov	r1, #0
.L767:
	uxth	r0, r1
	sxth	r2, r1
	cmp	ip, r0
	bhi	.L768
	ldr	r1, [r7, #4]
	cmp	r3, #0
	ldr	r2, [r4, #3380]
	str	r1, [r2, lr]
	lsl	r2, r3, #1
	ldr	r1, [r4, #3356]
	strh	r6, [r1, r2]	@ movhi
	blt	.L757
	ldrh	r2, [r8]
	ldr	r1, [r4, #328]
	sub	r1, r1, r2
	sub	r1, r1, #1
	sxth	r1, r1
	cmp	r3, r1
	bgt	.L757
.L802:
	add	r2, r2, #1
	ldr	r1, [r7, #4]
	strh	r2, [r8]	@ movhi
	ldr	r2, [r4, #3380]
	str	r1, [r2, r3, lsl #2]
	lsl	r3, r3, #1
	ldr	r2, [r4, #3356]
.L823:
	strh	r6, [r2, r3]	@ movhi
	b	.L757
.L768:
	ldr	r0, [r4, #3380]
	add	r1, r1, #1
	add	r9, r0, r2, lsl #2
	ldr	r9, [r9, #4]
	str	r9, [r0, r2, lsl #2]
	lsl	r2, r2, #1
	ldr	r0, [r4, #3356]
	add	r9, r0, r2
	ldrh	r9, [r9, #2]
	strh	r9, [r0, r2]	@ movhi
	b	.L767
.L765:
	sub	r3, r3, #1
	sxth	r3, r3
	b	.L764
.L760:
	ldr	r3, [sp, #20]
	ldrh	r2, [r3]
	ldr	r3, [sp, #12]
	ldrh	r3, [r3]
	cmp	r2, r3
	bls	.L773
	movw	r2, #1273
	ldr	r1, .L826+12
	ldr	r0, .L826+16
	bl	sftl_printk
.L773:
	ldr	r3, [sp, #12]
	ldr	lr, .L826+20
	ldr	ip, [r4, #3368]
	ldrh	r2, [r3]
	ldrh	r1, [lr]
	sub	r0, r2, #1
	sxth	r3, r0
	sub	r0, r0, r1
	str	r0, [sp, #16]
.L774:
	ldr	r0, [sp, #16]
	cmp	r3, r0
	ble	.L779
	ldr	r10, [r7, #4]
	lsl	r9, r3, #2
	ldr	r0, [ip, r3, lsl #2]
	cmp	r10, r0
	bls	.L775
	sub	r2, r2, r1
	ldr	r0, [ip]
	clz	r2, r2
	uxth	ip, r3
	lsr	r2, r2, #5
	cmp	r0, #0
	orrne	r2, r2, #1
	cmp	r2, #0
	addeq	r1, r1, #1
	strheq	r1, [lr]	@ movhi
	mov	r1, #0
.L777:
	uxth	r0, r1
	sxth	r2, r1
	cmp	ip, r0
	bhi	.L778
	ldr	r1, [r7, #4]
	ldr	r2, [r4, #3368]
	str	r1, [r2, r9]
	lsl	r2, r3, #1
	ldr	r1, [r4, #348]
	strh	r6, [r1, r2]	@ movhi
.L779:
	cmp	r3, #0
	blt	.L757
	ldr	r0, .L826+20
	ldrh	r2, [r0, #-24]
	ldrh	r1, [r0]
	sub	r2, r2, #1
	sub	r2, r2, r1
	sxth	r2, r2
	cmp	r3, r2
	bgt	.L757
	add	r1, r1, #1
	ldr	r2, [r4, #3368]
	strh	r1, [r0]	@ movhi
	ldr	r1, [r7, #4]
	str	r1, [r2, r3, lsl #2]
	lsl	r3, r3, #1
	ldr	r2, [r4, #348]
	b	.L823
.L778:
	ldr	r0, [r4, #3368]
	add	r1, r1, #1
	add	lr, r0, r2, lsl #2
	ldr	lr, [lr, #4]
	str	lr, [r0, r2, lsl #2]
	lsl	r2, r2, #1
	ldr	r0, [r4, #348]
	add	lr, r0, r2
	ldrh	lr, [lr, #2]
	strh	lr, [r0, r2]	@ movhi
	b	.L777
.L775:
	sub	r3, r3, #1
	sxth	r3, r3
	b	.L774
.L758:
	ldrh	r1, [r5]
	movw	r2, #65535
	cmp	r1, r2
	strheq	r6, [r5]	@ movhi
	beq	.L824
	ldrh	r0, [r5, #4]
	cmp	r0, r2
	beq	.L782
	mov	r1, #1
	bl	FtlFreeSysBlkQueueIn
.L782:
	ldr	r3, [r7, #4]
	ldr	r2, [r4, #2632]
	cmp	r2, r3
	strhcs	r6, [r5, #4]	@ movhi
	bcs	.L757
	ldrh	r3, [r5]
	strh	r6, [r5]	@ movhi
	strh	r3, [r5, #4]	@ movhi
	ldr	r3, [r7, #4]
.L824:
	str	r3, [r4, #2632]
	b	.L757
.L786:
	ldr	r1, [r4, #3356]
	ldrh	r3, [r1]
	cmp	r3, #0
	beq	.L788
.L791:
	ldr	r1, [r4, #348]
	ldrh	r2, [r1]
	cmp	r2, #0
	beq	.L789
.L790:
	movw	r3, #3436
	ldrh	r2, [r4, r3]
	ldr	r3, [r4, #328]
	cmp	r2, r3
	bls	.L821
	movw	r2, #1398
	ldr	r1, .L826+12
	ldr	r0, .L826+16
	bl	sftl_printk
.L821:
	mov	r0, #0
	add	sp, sp, #36
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L788:
	movw	r2, #3436
	ldrh	r2, [r4, r2]
	cmp	r2, #0
	ldrne	r0, [r4, #328]
	beq	.L791
.L792:
	sxth	r2, r3
	cmp	r2, r0
	bcs	.L791
	lsl	ip, r2, #1
	add	r3, r3, #1
	ldrh	ip, [r1, ip]
	cmp	ip, #0
	beq	.L792
	mov	r3, r2
	mov	lr, #0
.L793:
	ldr	r1, [r4, #328]
	cmp	r3, r1
	bcs	.L791
	ldr	r0, [r4, #3356]
	lsl	r1, r3, #1
	sub	ip, r3, r2
	lsl	r5, ip, #1
	ldrh	r6, [r0, r1]
	strh	r6, [r0, r5]	@ movhi
	ldr	r0, [r4, #3380]
	ldr	r5, [r0, r3, lsl #2]
	add	r3, r3, #1
	sxth	r3, r3
	str	r5, [r0, ip, lsl #2]
	ldr	r0, [r4, #3356]
	strh	lr, [r0, r1]	@ movhi
	b	.L793
.L789:
	ldr	r3, .L826+20
	ldrh	r0, [r3]
	cmp	r0, #0
	ldrhne	r0, [r3, #-24]!
	movne	lr, r3
	beq	.L790
.L798:
	sxth	r3, r2
	cmp	r3, r0
	mov	r5, r3
	bge	.L790
	lsl	ip, r3, #1
	add	r2, r2, #1
	ldrh	ip, [r1, ip]
	cmp	ip, #0
	beq	.L798
	mov	ip, #0
.L799:
	ldrh	r2, [lr]
	cmp	r3, r2
	bge	.L790
	ldr	r1, [r4, #348]
	lsl	r2, r3, #1
	sub	r0, r3, r5
	lsl	r6, r0, #1
	ldrh	r7, [r1, r2]
	strh	r7, [r1, r6]	@ movhi
	ldr	r1, [r4, #3368]
	ldr	r6, [r1, r3, lsl #2]
	str	r6, [r1, r0, lsl #2]
	ldr	r1, [r4, #348]
	strh	ip, [r1, r2]	@ movhi
	add	r2, r3, #1
	sxth	r3, r2
	b	.L799
.L827:
	.align	2
.L826:
	.word	.LANCHOR0
	.word	.LANCHOR0+312
	.word	.LANCHOR0+3436
	.word	.LANCHOR1+231
	.word	.LC8
	.word	.LANCHOR0+344
	.fnend
	.size	FtlScanSysBlk, .-FtlScanSysBlk
	.align	2
	.global	FtlLoadBbt
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadBbt, %function
FtlLoadBbt:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	ldr	r4, .L861
	ldr	r8, .L861+4
	ldr	r3, [r4, #3300]
	ldr	r6, [r4, #3332]
	ldr	r7, .L861+8
	str	r3, [r4, #3460]
	str	r6, [r4, #3464]
	bl	FtlBbtMemInit
	movw	r3, #298
	ldrh	r5, [r4, r3]
	sub	r5, r5, #1
	uxth	r5, r5
.L829:
	ldrh	r3, [r8]
	sub	r3, r3, #16
	cmp	r5, r3
	ble	.L832
	lsl	r3, r5, #10
	mov	r2, #1
	mov	r1, r2
	mov	r0, r7
	str	r3, [r4, #3456]
	bl	FlashReadPages
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	bne	.L830
	ldr	r3, [r4, #3456]
	mov	r2, #1
	mov	r1, r2
	mov	r0, r7
	add	r3, r3, #1
	str	r3, [r4, #3456]
	bl	FlashReadPages
.L830:
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	beq	.L831
	ldrh	r2, [r6]
	movw	r3, #61649
	cmp	r2, r3
	bne	.L831
	ldr	r2, [r6, #4]
	ldr	r3, .L861+12
	str	r2, [r4, #360]
	ldrh	r2, [r6, #8]
	strh	r5, [r3]	@ movhi
	strh	r2, [r3, #4]	@ movhi
.L832:
	ldr	r5, .L861+12
	movw	r2, #65535
	ldrh	r3, [r5]
	cmp	r3, r2
	beq	.L846
	ldrh	r3, [r5, #4]
	cmp	r3, r2
	beq	.L836
	lsl	r3, r3, #10
	mov	r2, #1
	mov	r1, r2
	ldr	r0, .L861+8
	str	r3, [r4, #3456]
	bl	FlashReadPages
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	beq	.L836
	ldrh	r2, [r6]
	movw	r3, #61649
	cmp	r2, r3
	bne	.L836
	ldr	r3, [r6, #4]
	ldr	r2, [r4, #360]
	cmp	r3, r2
	ldrhhi	r2, [r5, #4]
	strhi	r3, [r4, #360]
	ldrhhi	r3, [r6, #8]
	strhhi	r2, [r5]	@ movhi
	strhhi	r3, [r5, #4]	@ movhi
.L836:
	ldr	r8, .L861+8
	mov	r1, #1
	ldrh	r0, [r5]
	movw	r9, #61649
	bl	FtlGetLastWrittenPage
	sxth	r7, r0
	add	r0, r0, #1
	strh	r0, [r5, #2]	@ movhi
.L838:
	cmp	r7, #0
	bge	.L841
	mov	r2, #251
	ldr	r1, .L861+16
	ldr	r0, .L861+20
	bl	sftl_printk
.L840:
	ldrh	r3, [r6, #10]
	ldrh	r0, [r6, #12]
	strh	r3, [r5, #6]	@ movhi
	movw	r3, #65535
	cmp	r0, r3
	beq	.L843
	ldr	r2, [r4, #228]
	cmp	r0, r2
	beq	.L843
	ldrh	r3, [r4, #242]
	lsr	r3, r3, #2
	cmp	r0, r3
	cmpcc	r2, r3
	bcs	.L843
	bl	FtlSysBlkNumInit
.L843:
	ldr	r6, .L861+24
	mov	r5, #0
	ldr	r7, .L861+28
.L844:
	ldrh	r3, [r4, #254]
	cmp	r5, r3
	bcc	.L845
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L831:
	sub	r5, r5, #1
	uxth	r5, r5
	b	.L829
.L841:
	ldrh	r3, [r5]
	mov	r2, #1
	mov	r1, r2
	mov	r0, r8
	orr	r3, r7, r3, lsl #10
	str	r3, [r4, #3456]
	ldr	r3, [r4, #3300]
	str	r3, [r4, #3460]
	bl	FlashReadPages
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	beq	.L839
	ldrh	r3, [r6]
	cmp	r3, r9
	beq	.L840
.L839:
	sub	r7, r7, #1
	sxth	r7, r7
	b	.L838
.L845:
	ldrh	r2, [r7]
	ldr	r1, [r4, #3460]
	ldr	r0, [r6, #4]!
	lsl	r2, r2, #2
	mla	r1, r5, r2, r1
	add	r5, r5, #1
	bl	ftl_memcpy
	b	.L844
.L846:
	mvn	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L862:
	.align	2
.L861:
	.word	.LANCHOR0
	.word	.LANCHOR0+298
	.word	.LANCHOR0+3452
	.word	.LANCHOR0+352
	.word	.LANCHOR1+245
	.word	.LC8
	.word	.LANCHOR0+376
	.word	.LANCHOR0+3388
	.fnend
	.size	FtlLoadBbt, .-FtlLoadBbt
	.align	2
	.global	FlashProgPages
	.syntax unified
	.arm
	.fpu softvfp
	.type	FlashProgPages, %function
FlashProgPages:
	.fnstart
	@ args = 0, pretend = 0, frame = 48
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #60
	sub	sp, sp, #60
	ldr	r6, .L904
	mov	r10, r1
	mov	r4, r0
	mov	r5, r0
	ldr	r3, .L904+4
	mov	r8, #0
	ldrh	fp, [r6, #12]
	mov	r9, r6
	str	r2, [sp, #16]
	ldr	r2, [r3]
	str	r3, [sp, #20]
	str	r2, [sp, #52]
	lsl	r2, fp, #3
	str	r2, [sp, #12]
.L864:
	cmp	r8, r10
	bne	.L878
	ldr	r7, .L904+8
	mov	r5, #0
	ldr	r9, .L904+12
	ldr	r10, .L904+16
.L879:
	cmp	r8, r5
	beq	.L872
	ldr	r2, [r6, #3320]
	mov	r3, #0
	mov	r1, #1
	add	r0, sp, #32
	str	r3, [r2]
	ldr	r2, [r6, #3328]
	str	r3, [r2]
	ldr	r3, [r4, #4]
	ldr	r2, [sp, #16]
	str	r3, [sp, #36]
	ldr	r3, [r6, #3320]
	str	r3, [sp, #40]
	ldr	r3, [r6, #3328]
	str	r3, [sp, #44]
	bl	FlashReadPages
	ldr	fp, [sp, #32]
	cmn	fp, #1
	bne	.L880
	ldr	r1, [r4, #4]
	mov	r0, r7
	bl	sftl_printk
	str	fp, [r4]
.L880:
	ldr	fp, [sp, #32]
	cmp	fp, #256
	bne	.L881
	ldr	r1, [r4, #4]
	mov	r0, r9
	bl	sftl_printk
	str	fp, [r4]
.L881:
	ldr	r3, [r4, #12]
	cmp	r3, #0
	beq	.L882
	ldr	r1, [r6, #3328]
	ldr	r2, [r3]
	ldr	r0, [r1]
	cmp	r2, r0
	bne	.L883
	ldr	lr, [r3, #8]
	ldr	ip, [r1, #8]
	cmp	lr, ip
	beq	.L882
.L883:
	ldr	r1, [r1, #4]
	stm	sp, {r0, r1}
	mov	r0, r10
	ldr	r3, [r3, #4]
	ldr	r1, [r4, #4]
	bl	sftl_printk
	mvn	r3, #0
	str	r3, [r4]
.L882:
	ldr	r3, [r4, #8]
	cmp	r3, #0
	beq	.L884
	ldr	r1, [r6, #3320]
	ldr	r2, [r3]
	ldr	r0, [r1]
	cmp	r2, r0
	bne	.L885
	ldr	lr, [r3, #2048]
	ldr	ip, [r1, #2048]
	cmp	lr, ip
	beq	.L884
.L885:
	ldr	r1, [r1, #4]
	stm	sp, {r0, r1}
	ldr	r0, .L904+20
	ldr	r3, [r3, #4]
	ldr	r1, [r4, #4]
	bl	sftl_printk
	mvn	r3, #0
	str	r3, [r4]
.L884:
	add	r5, r5, #1
	add	r4, r4, #20
	b	.L879
.L878:
	ldr	r3, [r5, #8]
	cmp	r3, #0
	beq	.L865
	ldr	r3, [r5, #12]
	cmp	r3, #0
	bne	.L866
.L865:
	mov	r2, #148
	ldr	r1, .L904+24
	ldr	r0, .L904+28
	bl	sftl_printk
.L866:
	add	r2, sp, #24
	add	r1, sp, #28
	mov	r0, r5
	bl	l2p_addr_tran
	ldr	r7, [sp, #24]
	cmp	r7, #3
	bls	.L867
.L903:
	mvn	r3, #0
	str	r3, [r5]
	b	.L868
.L867:
	cmp	r7, #0
	bne	.L869
	ldr	r3, [sp, #28]
	ldr	r2, [sp, #12]
	cmp	r2, r3
	bls	.L869
	ldr	r5, .L904+24
	ldr	r6, .L904+32
	ldr	r8, .L904+36
	b	.L901
.L871:
	mvn	r3, #0
	ldr	r2, [r4, #-16]
	str	r3, [r4, #-20]
	mov	r1, r5
	mov	r0, r6
	add	r7, r7, #1
	bl	sftl_printk
	mov	r3, #16
	mov	r2, #4
	ldr	r1, [r4, #-12]
	mov	r0, r8
	bl	rknand_print_hex
	mov	r3, #4
	ldr	r1, [r4, #-8]
	mov	r2, r3
	ldr	r0, .L904+40
	bl	rknand_print_hex
.L901:
	cmp	r7, r10
	add	r4, r4, #20
	bne	.L871
	bl	dump_stack
.L872:
	ldr	r3, [sp, #20]
	mov	r0, #0
	ldr	r2, [sp, #52]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L887
	bl	__stack_chk_fail
.L869:
	ldr	r1, [r5, #8]
	tst	r1, #63
	moveq	r7, r1
	beq	.L873
	ldr	r7, [r9, #3320]
	cmp	r1, r7
	beq	.L873
	ldr	r3, .L904+44
	mov	r0, r7
	ldrh	r2, [r3]
	lsl	r2, r2, #9
	bl	ftl_memcpy
.L873:
	ldr	r3, [r5, #12]
	mov	r2, r7
	ldr	r1, [sp, #28]
	ldrb	r0, [sp, #24]	@ zero_extendqisi2
	ldr	ip, [r9, #3252]
	blx	ip
	cmp	r0, #0
	mvnne	r3, #0
	streq	r0, [r5]
	strne	r3, [r5]
	ldrh	r3, [r9, #14]
	cmp	r3, #4
	bne	.L868
	ldr	r3, [r5, #12]
	add	r2, r7, #2048
	ldr	r1, [sp, #28]
	ldrb	r0, [sp, #24]	@ zero_extendqisi2
	add	r3, r3, #8
	ldr	ip, [r9, #3252]
	add	r1, fp, r1
	blx	ip
	cmp	r0, #0
	bne	.L903
.L868:
	add	r8, r8, #1
	add	r5, r5, #20
	b	.L864
.L887:
	add	sp, sp, #60
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L905:
	.align	2
.L904:
	.word	.LANCHOR0
	.word	__stack_chk_guard
	.word	.LC95
	.word	.LC96
	.word	.LC97
	.word	.LC98
	.word	.LANCHOR1+256
	.word	.LC8
	.word	.LC86
	.word	.LC87
	.word	.LC88
	.word	.LANCHOR0+258
	.fnend
	.size	FlashProgPages, .-FlashProgPages
	.align	2
	.global	FtlLowFormatEraseBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLowFormatEraseBlock, %function
FtlLowFormatEraseBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r10, #0
	ldr	r4, .L949
	mov	r6, r1
	mov	r7, r10
	mov	r5, r10
	mov	r9, #20
	.pad #20
	sub	sp, sp, #20
	add	fp, r4, #312
	str	r0, [sp, #4]
	str	r0, [r4, #3284]
.L907:
	ldrh	r1, [r4, #232]
	uxth	r3, r10
	cmp	r1, r3
	bhi	.L911
	cmp	r7, #0
	beq	.L906
	mov	r8, #0
	mov	r9, #20
	mov	r2, r7
	mov	r1, #0
	ldr	r0, [r4, #3268]
	bl	FlashEraseBlocks
.L914:
	uxth	r3, r8
	cmp	r7, r3
	bhi	.L916
.L917:
	cmp	r6, #0
	mov	r8, #0
	ldrne	r3, .L949+4
	mov	fp, #20
	moveq	r3, #2
	streq	r6, [sp, #8]
	streq	r3, [sp, #12]
	ldrhne	r3, [r3]
	strne	r3, [sp, #12]
	movne	r3, #1
	strne	r3, [sp, #8]
.L926:
	mov	r10, #0
	mov	r7, r10
.L918:
	ldrh	r1, [r4, #232]
	uxth	r3, r10
	cmp	r1, r3
	bhi	.L921
	cmp	r7, #0
	beq	.L906
	mov	r9, #0
	mov	r3, #1
	ldr	r2, [sp, #8]
	mov	r1, r7
	ldr	r0, [r4, #3268]
	bl	FlashProgPages
.L923:
	uxth	r3, r9
	cmp	r7, r3
	bhi	.L925
	add	r8, r8, #1
	ldr	r2, [sp, #12]
	uxth	r3, r8
	cmp	r2, r3
	bhi	.L926
	mov	r8, #0
	mov	r9, #20
.L927:
	uxth	r3, r8
	cmp	r7, r3
	bhi	.L929
	ldr	r3, [sp, #4]
	adds	r6, r6, #0
	movne	r6, #1
	cmp	r3, #63
	orrls	r6, r6, #1
	cmp	r6, #0
	beq	.L906
	mov	r2, r7
	ldr	r1, [sp, #8]
	ldr	r0, [r4, #3268]
	bl	FlashEraseBlocks
.L906:
	mov	r0, r5
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L911:
	uxth	r3, r10
	ldr	r0, [r4, #3268]
	mov	r2, #0
	mul	r1, r9, r3
	add	r3, r4, r3
	str	r2, [r0, r1]
	ldr	r1, [sp, #4]
	ldrb	r0, [r3, #260]	@ zero_extendqisi2
	bl	V2P_block
	cmp	r6, #0
	mov	r8, r0
	beq	.L908
	bl	IsBlkInVendorPart
	cmp	r0, #0
	bne	.L909
.L908:
	mov	r0, r8
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	addne	r5, r5, #1
	uxthne	r5, r5
	bne	.L909
	ldr	r1, [r4, #3268]
	lsl	r8, r8, #10
	ldr	r3, [r4, #3316]
	mla	r1, r9, r7, r1
	str	r3, [r1, #8]
	ldrh	r3, [fp]
	str	r8, [r1, #4]
	mul	r3, r7, r3
	add	r7, r7, #1
	uxth	r7, r7
	add	r0, r3, #3
	cmp	r3, #0
	movlt	r3, r0
	ldr	r0, [r4, #3336]
	bic	r3, r3, #3
	add	r3, r0, r3
	str	r3, [r1, #12]
.L909:
	add	r10, r10, #1
	b	.L907
.L916:
	mul	r3, r9, r8
	ldr	r2, [r4, #3268]
	add	r1, r2, r3
	ldr	r3, [r2, r3]
	cmn	r3, #1
	bne	.L915
	ldr	r0, [r1, #4]
	add	r5, r5, #1
	uxth	r5, r5
	ubfx	r0, r0, #10, #16
	bl	FtlBbmMapBadBlock
.L915:
	add	r8, r8, #1
	b	.L914
.L921:
	uxth	r3, r10
	ldr	r0, [r4, #3268]
	mov	r2, #0
	mul	r1, fp, r3
	add	r3, r4, r3
	str	r2, [r0, r1]
	ldr	r1, [sp, #4]
	ldrb	r0, [r3, #260]	@ zero_extendqisi2
	bl	V2P_block
	cmp	r6, #0
	mov	r9, r0
	beq	.L919
	bl	IsBlkInVendorPart
	cmp	r0, #0
	bne	.L920
.L919:
	mov	r0, r9
	bl	FtlBbmIsBadBlock
	cmp	r0, #0
	bne	.L920
	ldr	r1, [r4, #3268]
	add	r9, r8, r9, lsl #10
	ldr	r3, [r4, #3312]
	mla	r1, fp, r7, r1
	str	r3, [r1, #8]
	ldr	r3, .L949+8
	str	r9, [r1, #4]
	ldrh	r3, [r3]
	mul	r3, r7, r3
	add	r7, r7, #1
	uxth	r7, r7
	add	r0, r3, #3
	cmp	r3, #0
	movlt	r3, r0
	ldr	r0, [r4, #3316]
	bic	r3, r3, #3
	add	r3, r0, r3
	str	r3, [r1, #12]
.L920:
	add	r10, r10, #1
	b	.L918
.L925:
	mul	r3, fp, r9
	ldr	r2, [r4, #3268]
	add	r1, r2, r3
	ldr	r3, [r2, r3]
	cmp	r3, #0
	beq	.L924
	ldr	r0, [r1, #4]
	add	r5, r5, #1
	uxth	r5, r5
	ubfx	r0, r0, #10, #16
	bl	FtlBbmMapBadBlock
.L924:
	add	r9, r9, #1
	b	.L923
.L929:
	cmp	r6, #0
	beq	.L928
	mul	r3, r9, r8
	ldr	r2, [r4, #3268]
	add	r1, r2, r3
	ldr	r3, [r2, r3]
	cmp	r3, #0
	bne	.L928
	ldr	r0, [r1, #4]
	mov	r1, #1
	ubfx	r0, r0, #10, #16
	bl	FtlFreeSysBlkQueueIn
.L928:
	add	r8, r8, #1
	b	.L927
.L950:
	.align	2
.L949:
	.word	.LANCHOR0
	.word	.LANCHOR0+304
	.word	.LANCHOR0+312
	.fnend
	.size	FtlLowFormatEraseBlock, .-FtlLowFormatEraseBlock
	.align	2
	.global	FlashTestBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FlashTestBlk, %function
FlashTestBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 88
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L958
	cmp	r0, #11
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	.pad #88
	sub	sp, sp, #88
	movls	r4, #0
	mov	r6, r3
	ldr	r2, [r3]
	str	r2, [sp, #84]
	bls	.L951
	ldr	r4, .L958+4
	mov	r5, r0
	mov	r2, #32
	add	r0, sp, #20
	mov	r1, #165
	str	r0, [sp, #12]
	ldr	r3, [r4, #3324]
	lsl	r5, r5, #10
	str	r3, [sp, #8]
	bl	ftl_memset
	mov	r2, #8
	mov	r1, #90
	ldr	r0, [r4, #3324]
	bl	ftl_memset
	mov	r2, #1
	mov	r0, sp
	mov	r1, r2
	str	r5, [sp, #4]
	bl	FlashEraseBlocks
	mov	r3, #1
	mov	r0, sp
	mov	r2, r3
	mov	r1, r3
	bl	FlashProgPages
	ldr	r3, [sp]
	cmp	r3, #0
	mvnne	r4, #0
	bne	.L953
	add	r3, r5, #1
	mov	r0, sp
	str	r3, [sp, #4]
	mov	r3, #1
	mov	r2, r3
	mov	r1, r3
	bl	FlashProgPages
	ldr	r4, [sp]
	adds	r4, r4, #0
	movne	r4, #1
	rsb	r4, r4, #0
.L953:
	mov	r2, #1
	mov	r1, #0
	mov	r0, sp
	str	r5, [sp, #4]
	bl	FlashEraseBlocks
.L951:
	ldr	r2, [sp, #84]
	mov	r0, r4
	ldr	r3, [r6]
	cmp	r2, r3
	beq	.L954
	bl	__stack_chk_fail
.L954:
	add	sp, sp, #88
	@ sp needed
	pop	{r4, r5, r6, pc}
.L959:
	.align	2
.L958:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.fnend
	.size	FlashTestBlk, .-FlashTestBlk
	.align	2
	.global	FtlBbmTblFlush
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlBbmTblFlush, %function
FtlBbmTblFlush:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	mov	r5, #0
	ldr	r4, .L973
	mov	r1, #0
	ldr	r7, .L973+4
	ldr	r3, [r4, #3332]
	add	r6, r4, #376
	ldr	r0, [r4, #3300]
	str	r3, [r4, #3464]
	movw	r3, #310
	ldrh	r2, [r4, r3]
	str	r0, [r4, #3460]
	bl	ftl_memset
.L961:
	ldrh	r3, [r4, #254]
	cmp	r5, r3
	blt	.L962
	ldr	r7, [r4, #3464]
	mov	r2, #16
	mov	r1, #255
	ldr	r6, .L973+8
	ldr	r10, .L973+12
	mov	r5, #0
	mov	r0, r7
	mov	r8, r5
	bl	ftl_memset
	ldr	r3, .L973+16
	sub	r9, r6, #48
	strh	r3, [r7]	@ movhi
	ldr	r3, [r4, #360]
	str	r3, [r7, #4]
	ldrh	r3, [r6]
	strh	r3, [r7, #2]	@ movhi
	ldrh	r3, [r6, #4]
	strh	r3, [r7, #8]	@ movhi
	ldrh	r3, [r6, #6]
	strh	r3, [r7, #10]	@ movhi
	ldr	r3, [r4, #228]
	strh	r3, [r7, #12]	@ movhi
	ldr	r3, [r4, #2604]
	strh	r3, [r7, #14]	@ movhi
.L963:
	ldr	r3, [r4, #3300]
	mov	fp, #0
	ldrh	r2, [r6, #2]
	ldrh	r1, [r6]
	str	r3, [r4, #3460]
	ldr	r3, [r4, #3332]
	str	fp, [r4, #3452]
	str	r3, [r4, #3464]
	orr	r3, r2, r1, lsl #10
	ldrh	r0, [r7, #10]
	str	r3, [r4, #3456]
	ldrh	r3, [r6, #4]
	str	r0, [sp]
	mov	r0, r10
	bl	sftl_printk
	ldrh	r3, [r9]
	ldrh	r2, [r6, #2]
	sub	r3, r3, #1
	cmp	r2, r3
	blt	.L964
	ldr	r3, [r4, #360]
	ldrh	r2, [r6]
	ldr	r0, [r4, #3268]
	add	r3, r3, #1
	strh	fp, [r6, #2]	@ movhi
	str	r3, [r4, #360]
	str	r3, [r7, #4]
	ldrh	r3, [r6, #4]
	strh	r2, [r7, #8]	@ movhi
	strh	r2, [r6, #4]	@ movhi
	mov	r2, #1
	strh	r3, [r6]	@ movhi
	mov	r1, r2
	lsl	r3, r3, #10
	str	r3, [r4, #3456]
	str	r3, [r0, #4]
	bl	FlashEraseBlocks
.L964:
	mov	r3, #1
	ldr	r0, .L973+20
	mov	r2, r3
	mov	r1, r3
	bl	FlashProgPages
	ldrh	r3, [r6, #2]
	add	r3, r3, #1
	strh	r3, [r6, #2]	@ movhi
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	bne	.L965
	add	r5, r5, #1
	ldr	r1, [r4, #3456]
	uxth	r5, r5
	ldr	r0, .L973+24
	bl	sftl_printk
	cmp	r5, #3
	bls	.L963
	mov	r2, r5
	ldr	r1, [r4, #3456]
	ldr	r0, .L973+28
	bl	sftl_printk
.L967:
	b	.L967
.L962:
	ldrh	r2, [r7]
	ldr	r3, [r4, #3460]
	ldr	r1, [r6, #4]!
	mul	r0, r2, r5
	lsl	r2, r2, #2
	add	r5, r5, #1
	add	r0, r3, r0, lsl #2
	bl	ftl_memcpy
	b	.L961
.L968:
	mov	r8, #1
	b	.L963
.L965:
	add	r8, r8, #1
	cmp	r8, #1
	ble	.L968
	cmp	r3, #256
	beq	.L963
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L974:
	.align	2
.L973:
	.word	.LANCHOR0
	.word	.LANCHOR0+3388
	.word	.LANCHOR0+352
	.word	.LC99
	.word	-3887
	.word	.LANCHOR0+3452
	.word	.LC100
	.word	.LC101
	.fnend
	.size	FtlBbmTblFlush, .-FtlBbmTblFlush
	.align	2
	.global	allocate_data_superblock
	.syntax unified
	.arm
	.fpu softvfp
	.type	allocate_data_superblock, %function
allocate_data_superblock:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r5, r0
	ldr	r4, .L1012
	.pad #20
	sub	sp, sp, #20
	add	fp, r4, #2528
.L976:
	ldr	r2, .L1012+4
	ldrh	r3, [r4, #224]
	ldrh	r2, [r2]
	add	r3, r3, r2
	ldrh	r2, [r4, #240]
	cmp	r3, r2
	ble	.L977
	movw	r2, #2667
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L977:
	ldr	r3, .L1012+16
	cmp	r5, r3
	movne	r1, #0
	bne	.L978
	ldrh	r3, [r4, #224]
	ldr	r1, [r4, #3164]
	mul	r2, r1, r3
	lsr	r1, r3, #1
	add	r1, r1, #1
	add	r1, r1, r2, lsr #2
	uxth	r1, r1
	cmp	r1, #0
	subne	r1, r1, #1
	uxthne	r1, r1
.L978:
	ldr	r0, .L1012+20
	bl	List_pop_index_node
	ldrh	r3, [r4, #224]
	mov	r6, r0
	uxth	r8, r0
	cmp	r3, #0
	bne	.L979
	movw	r2, #2676
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L979:
	ldrh	r3, [r4, #224]
	sub	r3, r3, #1
	strh	r3, [r4, #224]	@ movhi
	ldrh	r3, [r4, #240]
	cmp	r3, r8
	bls	.L976
	uxth	r6, r6
	ldr	r3, [r4, #72]
	lsl	r6, r6, #1
	ldrh	r7, [r3, r6]
	cmp	r7, #0
	bne	.L976
	strh	r8, [r5]	@ movhi
	mov	r0, r5
	bl	make_superblock
	ldrb	r3, [r5, #7]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L981
	ldr	r3, [r4, #72]
	mvn	r2, #0
	mov	r0, r8
	strh	r2, [r3, r6]	@ movhi
	bl	INSERT_DATA_LIST
	ldrh	r2, [fp]
	ldrh	r3, [r4, #224]
	add	r3, r3, r2
	ldrh	r2, [r4, #240]
	cmp	r3, r2
	ble	.L976
	movw	r2, #2690
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
	b	.L976
.L981:
	ldrh	r2, [fp]
	ldrh	r3, [r4, #224]
	add	r3, r3, r2
	ldrh	r2, [r4, #240]
	cmp	r3, r2
	ble	.L983
	movw	r2, #2693
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L983:
	ldr	r1, [r4, #3268]
	mov	r9, #20
	ldrh	r2, [r4, #232]
	add	r10, r5, #16
	mov	r0, r10
	mov	ip, #0
	mov	r3, r1
	mla	r2, r9, r2, r1
	str	r2, [sp, #4]
.L984:
	ldr	r2, [sp, #4]
	cmp	r2, r3
	bne	.L986
	cmp	r7, #0
	bne	.L987
	mov	r2, #2704
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L987:
	ldrh	r3, [r4, #172]
	cmp	r3, r8
	bne	.L988
	movw	r2, #2706
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L988:
	ldrb	r3, [r5, #8]	@ zero_extendqisi2
	ldr	r1, [r4, #2536]
	ldr	r2, .L1012+24
	cmp	r3, #0
	ldrh	r3, [r1, r6]
	bne	.L989
	cmp	r3, #0
	ldrhne	r0, [r2]
	moveq	r3, #2
	addne	r3, r3, r0
	strh	r3, [r1, r6]	@ movhi
	ldr	r3, [r4, #2596]
	add	r3, r3, #1
	str	r3, [r4, #2596]
.L992:
	ldr	r3, [r4, #2536]
	ldr	r1, [r4, #2612]
	ldrh	r2, [r2]
	ldrh	r3, [r3, r6]
	ldr	r0, [r4, #2596]
	cmp	r3, r1
	ldrh	r1, [r4, #240]
	strhi	r3, [r4, #2612]
	ldr	r3, [r4, #2600]
	mla	r0, r0, r2, r3
	bl	__aeabi_uidiv
	ldr	r2, [r4, #3348]
	ldr	r1, [r4, #3268]
	str	r0, [r4, #2604]
	ldr	r3, [r2, #16]
	ldr	r0, .L1012+28
	add	r3, r3, #1
	str	r3, [r2, #16]
	mov	r2, #20
	mla	r2, r2, r7, r1
	add	r3, r1, #4
	add	r2, r2, #24
.L994:
	add	r3, r3, #20
	cmp	r2, r3
	bne	.L995
	ldrb	r1, [r5, #8]	@ zero_extendqisi2
	mov	r2, r7
	ldr	r0, [r4, #3268]
	mov	r9, #0
	bl	FlashEraseBlocks
	mov	r3, r9
	mov	r1, #20
.L996:
	uxth	r2, r9
	cmp	r7, r2
	bhi	.L998
	cmp	r3, #0
	ble	.L999
	mov	r0, r8
	bl	update_multiplier_value
	bl	FtlBbmTblFlush
.L999:
	ldrb	r2, [r5, #7]	@ zero_extendqisi2
	cmp	r2, #0
	bne	.L1000
	ldr	r3, [r4, #72]
	mvn	r2, #0
	strh	r2, [r3, r6]	@ movhi
	b	.L976
.L986:
	str	ip, [r3, #8]
	movw	lr, #65535
	str	ip, [r3, #12]
	add	r3, r3, #20
	ldrh	r2, [r0], #2
	cmp	r2, lr
	mlane	lr, r9, r7, r1
	lslne	r2, r2, #10
	addne	r7, r7, #1
	uxthne	r7, r7
	strne	r2, [lr, #4]
	b	.L984
.L989:
	add	r3, r3, #1
	strh	r3, [r1, r6]	@ movhi
	ldr	r3, [r4, #2600]
	add	r3, r3, #1
	str	r3, [r4, #2600]
	b	.L992
.L995:
	ldr	r1, [r3, #-20]
	and	r1, r1, r0
	str	r1, [r3, #-20]
	b	.L994
.L998:
	mul	r2, r1, r9
	ldr	r0, [r4, #3268]
	add	ip, r0, r2
	ldr	r2, [r0, r2]
	cmn	r2, #1
	bne	.L997
	ldr	r0, [ip, #4]
	add	r3, r3, #1
	str	r1, [sp, #12]
	str	r2, [sp, #8]
	ubfx	r0, r0, #10, #16
	str	r3, [sp, #4]
	bl	FtlBbmMapBadBlock
	ldr	r2, [sp, #8]
	ldr	r1, [sp, #12]
	ldr	r3, [sp, #4]
	strh	r2, [r10]	@ movhi
	ldrb	r2, [r5, #7]	@ zero_extendqisi2
	sub	r2, r2, #1
	strb	r2, [r5, #7]
.L997:
	add	r9, r9, #1
	add	r10, r10, #2
	b	.L996
.L1000:
	movw	r3, #302
	ldrh	r3, [r4, r3]
	strh	r8, [r5]	@ movhi
	smulbb	r3, r3, r2
	mov	r2, #0
	strh	r2, [r5, #2]	@ movhi
	strb	r2, [r5, #6]
	ldr	r2, [r4, #2588]
	uxth	r3, r3
	ldr	r1, [r4, #72]
	strh	r3, [r5, #4]	@ movhi
	str	r2, [r5, #12]
	add	r2, r2, #1
	str	r2, [r4, #2588]
	ldrh	r2, [r5]
	lsl	r2, r2, #1
	strh	r3, [r1, r2]	@ movhi
	ldrh	r3, [r5, #4]
	cmp	r3, #0
	beq	.L1001
	ldrb	r3, [r5, #7]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L1002
.L1001:
	movw	r2, #2759
	ldr	r1, .L1012+8
	ldr	r0, .L1012+12
	bl	sftl_printk
.L1002:
	mov	r0, #0
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1013:
	.align	2
.L1012:
	.word	.LANCHOR0
	.word	.LANCHOR0+2528
	.word	.LANCHOR1+271
	.word	.LC8
	.word	.LANCHOR0+124
	.word	.LANCHOR0+2532
	.word	.LANCHOR0+292
	.word	-1024
	.fnend
	.size	allocate_data_superblock, .-allocate_data_superblock
	.align	2
	.global	FtlGcFreeBadSuperBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcFreeBadSuperBlk, %function
FtlGcFreeBadSuperBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	movw	r3, #3206
	ldr	r6, .L1027
	ldrh	r3, [r6, r3]
	cmp	r3, #0
	beq	.L1015
	ldr	fp, .L1027+4
	mov	r7, #0
	str	r0, [sp, #4]
	add	r10, fp, #2
.L1016:
	ldrh	r2, [r6, #232]
	uxth	r3, r7
	cmp	r2, r3
	bhi	.L1022
	bl	FtlGcReFreshBadBlk
.L1015:
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1022:
	uxtah	r3, r6, r7
	ldr	r1, [sp, #4]
	mov	r8, #0
	ldrb	r0, [r3, #260]	@ zero_extendqisi2
	bl	V2P_block
	mov	r9, r0
.L1017:
	ldrh	r3, [fp]
	uxth	r4, r8
	cmp	r3, r4
	addls	r7, r7, #1
	bls	.L1016
.L1021:
	uxth	r5, r8
	lsl	r3, r5, #1
	ldrh	r3, [r10, r3]
	cmp	r3, r9
	bne	.L1018
	mov	r1, r9
	ldr	r0, .L1027+8
	bl	sftl_printk
	mov	r0, r9
	bl	FtlBbmMapBadBlock
	bl	FtlBbmTblFlush
	ldrh	r1, [fp]
	add	r3, r10, r5, lsl #1
.L1019:
	cmp	r4, r1
	bcc	.L1020
	sub	r1, r1, #1
	strh	r1, [fp]	@ movhi
.L1018:
	add	r8, r8, #1
	b	.L1017
.L1020:
	ldrh	r0, [r3, #2]!
	add	r4, r4, #1
	uxth	r4, r4
	strh	r0, [r3, #-2]	@ movhi
	b	.L1019
.L1028:
	.align	2
.L1027:
	.word	.LANCHOR0
	.word	.LANCHOR0+3206
	.word	.LC102
	.fnend
	.size	FtlGcFreeBadSuperBlk, .-FtlGcFreeBadSuperBlk
	.align	2
	.global	update_vpc_list
	.syntax unified
	.arm
	.fpu softvfp
	.type	update_vpc_list, %function
update_vpc_list:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	lsl	r3, r0, #1
	ldr	r4, .L1042
	ldr	r2, [r4, #72]
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	bne	.L1030
	ldrh	r3, [r4, #222]
	mov	r5, r0
	cmp	r3, r0
	mvneq	r3, #0
	strheq	r3, [r4, #222]	@ movhi
	ldrh	r3, [r4, #220]
	cmp	r3, r0
	mvneq	r3, #0
	strheq	r3, [r4, #220]	@ movhi
	ldrh	r3, [r4, #172]
	cmp	r3, r0
	mvneq	r3, #0
	strheq	r3, [r4, #172]	@ movhi
	beq	.L1034
	ldrh	r3, [r4, #24]
	cmp	r3, r0
	beq	.L1039
	ldrh	r3, [r4, #76]
	cmp	r3, r0
	beq	.L1039
	ldrh	r3, [r4, #124]
	cmp	r3, r0
	beq	.L1039
.L1034:
	ldr	r6, .L1042+4
	mov	r1, r5
	ldr	r0, .L1042+8
	bl	List_remove_node
	ldrh	r3, [r6]
	cmp	r3, #0
	bne	.L1036
	movw	r2, #2835
	ldr	r1, .L1042+12
	ldr	r0, .L1042+16
	bl	sftl_printk
.L1036:
	ldrh	r3, [r6]
	mov	r0, r5
	sub	r3, r3, #1
	strh	r3, [r6]	@ movhi
	bl	free_data_superblock
	mov	r0, r5
	bl	FtlGcFreeBadSuperBlk
	ldrh	r2, [r6]
	ldrh	r3, [r4, #224]
	add	r3, r3, r2
	ldrh	r2, [r4, #240]
	cmp	r3, r2
	ble	.L1040
	movw	r2, #2838
	ldr	r1, .L1042+12
	ldr	r0, .L1042+16
	bl	sftl_printk
.L1040:
	mov	r0, #1
	pop	{r4, r5, r6, pc}
.L1030:
	bl	List_update_data_list
.L1039:
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L1043:
	.align	2
.L1042:
	.word	.LANCHOR0
	.word	.LANCHOR0+2528
	.word	.LANCHOR0+2520
	.word	.LANCHOR1+296
	.word	.LC8
	.fnend
	.size	update_vpc_list, .-update_vpc_list
	.align	2
	.global	decrement_vpc_count
	.syntax unified
	.arm
	.fpu softvfp
	.type	decrement_vpc_count, %function
decrement_vpc_count:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	movw	r3, #65535
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	cmp	r0, r3
	mov	r6, r0
	ldr	r4, .L1057
	beq	.L1045
	ldr	r3, [r4, #72]
	lsl	r5, r0, #1
	ldrh	r2, [r3, r5]
	cmp	r2, #0
	subne	r2, r2, #1
	strhne	r2, [r3, r5]	@ movhi
	bne	.L1045
	mov	r1, r0
	ldr	r0, .L1057+4
	bl	sftl_printk
	ldr	r3, [r4, #72]
	ldrh	r5, [r3, r5]
	cmp	r5, #0
	beq	.L1047
.L1055:
	mov	r5, #0
.L1044:
	mov	r0, r5
	pop	{r4, r5, r6, r7, r8, pc}
.L1047:
	movw	r2, #2853
.L1056:
	ldr	r1, .L1057+8
	ldr	r0, .L1057+12
	bl	sftl_printk
	b	.L1044
.L1045:
	movw	r7, #3438
	movw	r3, #65535
	ldrh	r0, [r4, r7]
	cmp	r0, r3
	strheq	r6, [r4, r7]	@ movhi
	beq	.L1055
.L1049:
	cmp	r6, r0
	beq	.L1055
	bl	update_vpc_list
	ldr	r2, [r4, #2516]
	adds	r5, r0, #0
	ldr	r3, [r4, #2520]
	movne	r5, #1
	ldr	r1, [r4, #72]
	strh	r6, [r4, r7]	@ movhi
	sub	r3, r3, r2
	asr	r2, r3, #1
	ldr	r3, .L1057+16
	mul	r3, r3, r2
	uxth	r2, r3
	uxth	r3, r3
	lsl	r2, r2, #1
	cmp	r3, r6
	ldrh	r2, [r1, r2]
	clz	r2, r2
	lsr	r2, r2, #5
	moveq	r2, #0
	cmp	r2, #0
	beq	.L1044
	movw	r2, #2869
	b	.L1056
.L1058:
	.align	2
.L1057:
	.word	.LANCHOR0
	.word	.LC103
	.word	.LANCHOR1+312
	.word	.LC8
	.word	-1431655765
	.fnend
	.size	decrement_vpc_count, .-decrement_vpc_count
	.align	2
	.global	FtlSuperblockPowerLostFix
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlSuperblockPowerLostFix, %function
FtlSuperblockPowerLostFix:
	.fnstart
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	.pad #28
	sub	sp, sp, #28
	ldr	r7, .L1074
	mov	r8, #0
	movw	r2, #61589
	mov	r4, r0
	ldr	r5, .L1074+4
	ldr	r3, [r7]
	ldr	r6, [r5, #3332]
	str	r3, [sp, #20]
	mvn	r3, #0
	str	r3, [sp, #16]
	ldr	r3, [r5, #3300]
	str	r6, [sp, #12]
	str	r3, [sp, #8]
	mvn	r3, #2
	str	r3, [r6, #8]
	mvn	r3, #1
	str	r3, [r6, #12]
	ldrh	r3, [r0]
	strh	r8, [r6]	@ movhi
	strh	r3, [r6, #2]	@ movhi
	ldr	r3, [r5, #3300]
	str	r2, [r3]
	ldr	r2, .L1074+8
	ldr	r3, [r5, #3300]
	str	r2, [r3, #4]
	ldrh	r3, [r0, #4]
	tst	r3, #1
	moveq	r9, #6
	movne	r9, #7
.L1065:
	ldrh	r3, [r4, #4]
	cmp	r3, #0
	bne	.L1061
.L1062:
	ldrh	r3, [r4]
	ldr	r1, [r5, #72]
	ldrh	r0, [r4, #4]
	lsl	r3, r3, #1
	ldrh	r2, [r1, r3]
	sub	r2, r2, r0
	strh	r2, [r1, r3]	@ movhi
	movw	r3, #302
	ldrh	r3, [r5, r3]
	ldr	r2, [sp, #20]
	strh	r3, [r4, #2]	@ movhi
	mov	r3, #0
	strb	r3, [r4, #6]
	strh	r3, [r4, #4]	@ movhi
	ldr	r3, [r7]
	cmp	r2, r3
	beq	.L1066
	bl	__stack_chk_fail
.L1061:
	mov	r0, r4
	bl	get_new_active_ppa
	cmn	r0, #1
	str	r0, [sp, #4]
	beq	.L1062
	ldr	r3, [r5, #2592]
	mov	r1, #1
	mov	r0, sp
	str	r3, [r6, #4]
	add	r3, r3, #1
	cmn	r3, #1
	moveq	r3, r8
	str	r3, [r5, #2592]
	mov	r3, #0
	mov	r2, r3
	bl	FlashProgPages
	ldrh	r0, [r4]
	bl	decrement_vpc_count
	subs	r9, r9, #1
	bne	.L1065
	b	.L1062
.L1066:
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L1075:
	.align	2
.L1074:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	305419896
	.fnend
	.size	FtlSuperblockPowerLostFix, .-FtlSuperblockPowerLostFix
	.align	2
	.global	FtlMakeBbt
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlMakeBbt, %function
FtlMakeBbt:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	mov	r9, #0
	ldr	r8, .L1097
	bl	FtlBbtMemInit
	sub	r4, r8, #28
	sub	r6, r8, #380
	sub	r10, r8, #18
	mov	r5, r6
	sub	fp, r4, #54
	bl	FtlLoadFactoryBbt
.L1077:
	ldrh	r3, [r6, #254]
	cmp	r9, r3
	bcc	.L1083
	ldr	r7, .L1097+4
	mov	r6, #0
.L1084:
	ldrh	r3, [r7]
	uxth	r0, r6
	add	r6, r6, #1
	cmp	r3, r0
	bhi	.L1085
	ldrh	r6, [r4, #12]
	movw	r7, #65535
	sub	r6, r6, #1
	uxth	r6, r6
.L1086:
	ldrh	r3, [r4, #12]
	sub	r3, r3, #48
	cmp	r6, r3
	ble	.L1090
	mov	r0, r6
	bl	FtlBbmIsBadBlock
	cmp	r0, #1
	beq	.L1087
	mov	r0, r6
	bl	FlashTestBlk
	cmp	r0, #0
	beq	.L1088
	mov	r0, r6
	bl	FtlBbmMapBadBlock
.L1087:
	sub	r6, r6, #1
	uxth	r6, r6
	b	.L1086
.L1083:
	ldr	r3, [r6, #3332]
	movw	r2, #65535
	ldr	r0, [r6, #3300]
	str	r3, [sp, #4]
	str	r3, [r6, #3464]
	ldrh	r3, [r10, #2]!
	str	r0, [r6, #3460]
	cmp	r3, r2
	beq	.L1078
	ldrh	r7, [fp]
	mov	r2, #1
	mov	r1, r2
	ldr	r0, .L1097+8
	mla	r7, r9, r7, r3
	lsl	r3, r7, #10
	str	r3, [r6, #3456]
	bl	FlashReadPages
	ldrh	r2, [fp]
	ldr	r1, [r6, #3460]
	ldr	r0, [r8]
	add	r2, r2, #7
	asr	r2, r2, #3
	bl	ftl_memcpy
.L1079:
	uxth	r0, r7
	add	r9, r9, #1
	add	r8, r8, #4
	bl	FtlBbmMapBadBlock
	b	.L1077
.L1078:
	mov	r1, r9
	bl	FlashGetBadBlockList
	ldr	r1, [r8]
	ldr	r0, [r6, #3460]
	bl	FtlBbt2Bitmap
	ldrh	r3, [fp]
.L1096:
	sub	r3, r3, #1
	uxth	r3, r3
	str	r3, [sp]
.L1080:
	ldr	r3, [sp]
	ldrh	r0, [fp]
	smlabb	r0, r0, r9, r3
	uxth	r0, r0
	bl	FtlBbmIsBadBlock
	cmp	r0, #1
	beq	.L1081
	ldrh	r3, [sp]
	mov	r2, #16
	mov	r1, #0
	ldr	r0, [r5, #3332]
	strh	r3, [r10]	@ movhi
	bl	ftl_memset
	mov	r2, #4096
	mov	r1, #0
	ldr	r0, [r5, #3300]
	bl	ftl_memset
	ldr	r3, [sp, #4]
	movw	r2, 61664	@ movhi
	strh	r2, [r3]	@ movhi
	mov	r3, #0
	ldr	r2, [sp, #4]
	ldrh	r7, [fp]
	str	r3, [r2, #4]
	ldrh	r3, [r10]
	strh	r3, [r2, #2]	@ movhi
	ldrh	r3, [r10]
	ldr	r1, [r8]
	ldr	r0, [r5, #3460]
	mla	r7, r9, r7, r3
	lsl	r3, r7, #10
	str	r3, [r5, #3456]
	ldr	r3, .L1097+12
	ldrh	r2, [r3]
	lsl	r2, r2, #2
	bl	ftl_memcpy
	mov	r2, #1
	ldr	r0, .L1097+8
	mov	r1, r2
	bl	FlashEraseBlocks
	mov	r3, #1
	ldr	r0, .L1097+8
	mov	r2, r3
	mov	r1, r3
	bl	FlashProgPages
	ldr	r3, [r5, #3452]
	cmn	r3, #1
	bne	.L1079
	uxth	r0, r7
	bl	FtlBbmMapBadBlock
	b	.L1080
.L1081:
	ldr	r3, [sp]
	b	.L1096
.L1085:
	bl	FtlBbmMapBadBlock
	b	.L1084
.L1088:
	ldrh	r3, [r4]
	cmp	r3, r7
	strheq	r6, [r4]	@ movhi
	beq	.L1087
.L1089:
	strh	r6, [r4, #4]	@ movhi
.L1090:
	ldrh	r3, [r4]
	mov	r6, #0
	ldr	r0, [r5, #3268]
	mov	r1, #1
	str	r6, [r5, #360]
	mov	r2, #2
	strh	r6, [r4, #2]	@ movhi
	lsl	r3, r3, #10
	str	r3, [r0, #4]
	ldrh	r3, [r4, #4]
	lsl	r3, r3, #10
	str	r3, [r0, #24]
	bl	FlashEraseBlocks
	ldrh	r0, [r4]
	bl	FtlBbmMapBadBlock
	ldrh	r0, [r4, #4]
	bl	FtlBbmMapBadBlock
	bl	FtlBbmTblFlush
	ldr	r3, [r5, #360]
	ldrh	r2, [r4, #4]
	strh	r6, [r4, #2]	@ movhi
	add	r3, r3, #1
	str	r3, [r5, #360]
	ldrh	r3, [r4]
	strh	r2, [r4]	@ movhi
	strh	r3, [r4, #4]	@ movhi
	bl	FtlBbmTblFlush
	mov	r0, r6
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1098:
	.align	2
.L1097:
	.word	.LANCHOR0+380
	.word	.LANCHOR0+314
	.word	.LANCHOR0+3452
	.word	.LANCHOR0+3388
	.fnend
	.size	FtlMakeBbt, .-FtlMakeBbt
	.align	2
	.global	ftl_memcmp
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_memcmp, %function
ftl_memcmp:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	b	memcmp
	.fnend
	.size	ftl_memcmp, .-ftl_memcmp
	.align	2
	.global	js_hash
	.syntax unified
	.arm
	.fpu softvfp
	.type	js_hash, %function
js_hash:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r3, .L1103
	add	r1, r0, r1
.L1101:
	cmp	r0, r1
	bne	.L1102
	mov	r0, r3
	bx	lr
.L1102:
	lsr	r2, r3, #2
	ldrb	ip, [r0], #1	@ zero_extendqisi2
	add	r2, r2, r3, lsl #5
	add	r2, r2, ip
	eor	r3, r3, r2
	b	.L1101
.L1104:
	.align	2
.L1103:
	.word	1204201446
	.fnend
	.size	js_hash, .-js_hash
	.align	2
	.global	Ftl_write_map_blk_to_last_page
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_write_map_blk_to_last_page, %function
Ftl_write_map_blk_to_last_page:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	movw	r2, #65535
	ldrh	r3, [r0]
	mov	r4, r0
	ldr	r5, [r0, #12]
	cmp	r3, r2
	bne	.L1106
	ldrh	r3, [r0, #8]
	cmp	r3, #0
	beq	.L1107
	movw	r2, #641
	ldr	r1, .L1116
	ldr	r0, .L1116+4
	bl	sftl_printk
.L1107:
	ldrh	r3, [r4, #8]
	add	r3, r3, #1
	strh	r3, [r4, #8]	@ movhi
	bl	FtlFreeSysBlkQueueOut
	mov	r3, #0
	strh	r0, [r5]	@ movhi
	strh	r3, [r4, #2]	@ movhi
	strh	r3, [r4]	@ movhi
	ldr	r3, [r4, #28]
	add	r3, r3, #1
	str	r3, [r4, #28]
.L1108:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, pc}
.L1106:
	lsl	r3, r3, #1
	ldr	r7, [r0, #24]
	mov	r1, #255
	ldrh	r8, [r5, r3]
	ldrh	r3, [r0, #2]
	ldr	r5, .L1116+8
	orr	r3, r3, r8, lsl #10
	ldr	r6, [r5, #3332]
	str	r3, [r5, #3456]
	ldr	r3, [r5, #3300]
	str	r6, [r5, #3464]
	str	r3, [r5, #3460]
	ldr	r3, [r0, #28]
	str	r3, [r6, #4]
	ldr	r3, .L1116+12
	strh	r3, [r6, #8]	@ movhi
	ldrh	r3, [r0, #4]
	strh	r8, [r6, #2]	@ movhi
	strh	r3, [r6]	@ movhi
	add	r3, r5, #304
	ldrh	r2, [r3]
	ldr	r0, [r5, #3300]
	lsl	r2, r2, #3
	bl	ftl_memset
	mov	r2, #0
	mov	r3, r2
.L1109:
	ldrh	r0, [r4, #6]
	uxth	r1, r2
	cmp	r0, r1
	bhi	.L1111
	movw	r3, #310
	ldr	r0, [r5, #3460]
	ldrh	r1, [r5, r3]
	bl	js_hash
	mov	r2, #1
	str	r0, [r6, #12]
	mov	r3, #0
	mov	r1, r2
	ldr	r0, .L1116+16
	bl	FlashProgPages
	ldrh	r3, [r4, #2]
	mov	r0, r4
	add	r3, r3, #1
	strh	r3, [r4, #2]	@ movhi
	bl	ftl_map_blk_gc
	b	.L1108
.L1111:
	uxth	r1, r2
	ldr	r0, [r7, r1, lsl #2]
	cmp	r8, r0, lsr #10
	bne	.L1110
	ldr	r0, [r5, #3300]
	add	r3, r3, #1
	uxth	r3, r3
	str	r1, [r0, r3, lsl #3]
	ldr	r0, [r7, r1, lsl #2]
	ldr	r1, [r5, #3300]
	add	r1, r1, r3, lsl #3
	str	r0, [r1, #4]
.L1110:
	add	r2, r2, #1
	b	.L1109
.L1117:
	.align	2
.L1116:
	.word	.LANCHOR1+332
	.word	.LC8
	.word	.LANCHOR0
	.word	-1291
	.word	.LANCHOR0+3452
	.fnend
	.size	Ftl_write_map_blk_to_last_page, .-Ftl_write_map_blk_to_last_page
	.align	2
	.global	FtlMapWritePage
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlMapWritePage, %function
FtlMapWritePage:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	lsl	r9, r1, #2
	ldr	r5, .L1144
	mov	r4, r0
	mov	r7, r1
	mov	r10, r2
	mov	r6, #0
	add	r8, r5, #304
	mov	fp, r8
.L1119:
	ldr	r3, [r5, #2572]
	add	r3, r3, #1
	str	r3, [r5, #2572]
	ldrh	r3, [r8]
	ldrh	r2, [r4, #2]
	sub	r3, r3, #1
	cmp	r2, r3
	bge	.L1120
	ldrh	r2, [r4]
	movw	r3, #65535
	cmp	r2, r3
	bne	.L1121
.L1120:
	mov	r0, r4
	bl	Ftl_write_map_blk_to_last_page
.L1121:
	ldrh	r3, [r4]
	ldr	r2, [r4, #12]
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	bne	.L1122
	mov	r2, #700
	ldr	r1, .L1144+4
	ldr	r0, .L1144+8
	bl	sftl_printk
.L1122:
	ldrh	r2, [r4]
	ldrh	r3, [r4, #10]
	cmp	r2, r3
	bcc	.L1123
	movw	r2, #701
	ldr	r1, .L1144+4
	ldr	r0, .L1144+8
	bl	sftl_printk
.L1123:
	ldrh	r3, [r4]
	mov	r1, #0
	ldr	r2, [r4, #12]
	ldr	r0, [r5, #3332]
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	ldrh	r2, [r4, #2]
	str	r10, [r5, #3460]
	str	r3, [sp, #4]
	str	r0, [r5, #3464]
	orr	r2, r2, r3, lsl #10
	str	r2, [r5, #3456]
	mov	r2, #16
	bl	ftl_memset
	ldr	r2, [r5, #3464]
	ldr	r1, [r4, #28]
	ldr	r3, [sp, #4]
	strh	r7, [r2, #8]	@ movhi
	str	r1, [r2, #4]
	ldrh	r1, [r4, #4]
	strh	r3, [r2, #2]	@ movhi
	ldr	r3, .L1144+12
	strh	r1, [r2]	@ movhi
	ldr	r0, [r5, #3460]
	ldrh	r1, [r3]
	str	r2, [sp, #4]
	bl	js_hash
	ldr	r2, [sp, #4]
	mov	r3, #1
	mov	r1, r3
	str	r0, [r2, #12]
	mov	r2, r3
	ldr	r0, .L1144+16
	bl	FlashProgPages
	ldrh	r3, [r4, #2]
	add	r3, r3, #1
	strh	r3, [r4, #2]	@ movhi
	ldr	r3, [r5, #3452]
	cmn	r3, #1
	bne	.L1124
	ldr	r1, [r5, #3456]
	add	r6, r6, #1
	ldr	r0, .L1144+20
	uxth	r6, r6
	bl	sftl_printk
	ldrh	r3, [r4, #2]
	cmp	r3, #2
	ldrhls	r3, [fp]
	subls	r3, r3, #1
	strhls	r3, [r4, #2]	@ movhi
	cmp	r6, #3
	bls	.L1126
	mov	r2, r6
	ldr	r1, [r5, #3456]
	ldr	r0, .L1144+24
	bl	sftl_printk
.L1127:
	b	.L1127
.L1126:
	ldr	r3, [r4, #32]
	cmp	r3, #0
	beq	.L1119
.L1142:
	b	.L1142
.L1124:
	ldr	r3, [r4, #24]
	ldr	r2, [r5, #3456]
	str	r2, [r3, r9]
	ldrh	r3, [r4, #2]
	cmp	r3, #1
	beq	.L1130
	ldr	r3, [r5, #3452]
	cmp	r3, #256
	beq	.L1130
	ldr	r2, [r4, #36]
	cmp	r2, #0
	beq	.L1131
.L1130:
	mov	r3, #0
	str	r3, [r4, #36]
	b	.L1119
.L1131:
	cmn	r3, #1
	bne	.L1132
	movw	r2, #745
	ldr	r1, .L1144+4
	ldr	r0, .L1144+8
	bl	sftl_printk
.L1132:
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1145:
	.align	2
.L1144:
	.word	.LANCHOR0
	.word	.LANCHOR1+363
	.word	.LC8
	.word	.LANCHOR0+310
	.word	.LANCHOR0+3452
	.word	.LC104
	.word	.LC105
	.fnend
	.size	FtlMapWritePage, .-FtlMapWritePage
	.align	2
	.global	load_l2p_region
	.syntax unified
	.arm
	.fpu softvfp
	.type	load_l2p_region, %function
load_l2p_region:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r6, r0
	ldr	r9, .L1154
	mov	r10, r1
	ldrh	r3, [r9]
	cmp	r3, r0
	bcs	.L1147
	movw	r2, #485
	ldr	r1, .L1154+4
	ldr	r0, .L1154+8
	bl	sftl_printk
.L1147:
	ldr	r7, .L1154+12
	mov	r5, #12
	ldr	r3, [r7, #3376]
	mov	r4, r7
	ldr	r8, [r3, r6, lsl #2]
	cmp	r8, #0
	bne	.L1148
	mul	r5, r5, r10
	ldr	r3, [r7, #2540]
	movw	r2, #310
	mov	r1, #255
	ldrh	r2, [r7, r2]
	add	r3, r3, r5
	ldr	r0, [r3, #8]
	bl	ftl_memset
	ldr	r3, [r7, #2540]
	strh	r6, [r3, r5]	@ movhi
	ldr	r3, [r7, #2540]
	add	r5, r3, r5
	str	r8, [r5, #4]
.L1149:
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L1148:
	mul	r5, r5, r10
	ldr	r3, [r7, #2540]
	mov	r2, #1
	ldr	r0, .L1154+16
	mov	r1, r2
	str	r8, [r7, #3456]
	add	r3, r3, r5
	ldr	r3, [r3, #8]
	str	r3, [r7, #3460]
	ldr	r3, [r7, #3332]
	str	r3, [r7, #3464]
	bl	FlashReadPages
	ldr	r7, [r7, #3464]
	ldrh	r3, [r7, #8]
	cmp	r3, r6
	beq	.L1150
	mov	r2, r8
	mov	r1, r6
	ldr	r0, .L1154+20
	bl	sftl_printk
	mov	r3, #4
	ldr	r1, [r4, #3464]
	mov	r2, r3
	ldr	r0, .L1154+24
	bl	rknand_print_hex
	ldrh	r3, [r9]
	mov	r2, #4
	ldr	r1, [r4, #3376]
	ldr	r0, .L1154+28
	bl	rknand_print_hex
.L1151:
	ldrh	r3, [r7, #8]
	cmp	r3, r6
	beq	.L1152
	mov	r2, #508
	ldr	r1, .L1154+4
	ldr	r0, .L1154+8
	bl	sftl_printk
.L1152:
	ldr	r3, [r4, #2540]
	mov	r1, #0
	add	r2, r3, r5
	str	r1, [r2, #4]
	strh	r6, [r3, r5]	@ movhi
	b	.L1149
.L1150:
	ldr	r3, [r4, #3452]
	cmp	r3, #256
	bne	.L1151
	mov	r2, r8
	mov	r1, r6
	ldr	r0, .L1154+32
	bl	sftl_printk
	ldr	r3, [r4, #2540]
	mov	r1, r6
	add	r0, r4, #3392
	add	r3, r3, r5
	ldr	r2, [r3, #8]
	bl	FtlMapWritePage
	b	.L1151
.L1155:
	.align	2
.L1154:
	.word	.LANCHOR0+336
	.word	.LANCHOR1+379
	.word	.LC8
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.word	.LC106
	.word	.LC94
	.word	.LC107
	.word	.LC108
	.fnend
	.size	load_l2p_region, .-load_l2p_region
	.align	2
	.global	ftl_map_blk_gc
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_map_blk_gc, %function
ftl_map_blk_gc:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	mov	r4, r0
	ldr	r5, [r0, #12]
	ldr	r10, [r0, #24]
	bl	ftl_free_no_use_map_blk
	ldrh	r3, [r4, #10]
	ldrh	r2, [r4, #8]
	sub	r3, r3, #5
	cmp	r2, r3
	blt	.L1157
	uxth	r0, r0
	lsl	r0, r0, #1
	ldrh	r9, [r5, r0]
	cmp	r9, #0
	beq	.L1157
	ldr	r3, [r4, #32]
	cmp	r3, #0
	bne	.L1157
	mov	r2, #1
	str	r2, [r4, #32]
	strh	r3, [r5, r0]	@ movhi
	ldrh	r3, [r4, #8]
	ldrh	r2, [r4, #2]
	sub	r3, r3, #1
	strh	r3, [r4, #8]	@ movhi
	ldr	r3, .L1173
	ldrh	r3, [r3]
	cmp	r2, r3
	bcc	.L1158
	mov	r0, r4
	bl	ftl_map_blk_alloc_new_blk
.L1158:
	ldr	r5, .L1173+4
	mov	r6, #0
.L1159:
	ldrh	r2, [r4, #6]
	uxth	fp, r6
	cmp	r2, fp
	bhi	.L1166
	mov	r1, #1
	mov	r0, r9
	bl	FtlFreeSysBlkQueueIn
	mov	r3, #0
	str	r3, [r4, #32]
.L1157:
	ldr	r3, .L1173
	ldrh	r2, [r4, #2]
	ldrh	r3, [r3]
	cmp	r2, r3
	bcc	.L1167
	mov	r0, r4
	bl	ftl_map_blk_alloc_new_blk
.L1167:
	mov	r0, #0
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1166:
	uxth	r7, r6
	ldr	r2, [r10, r7, lsl #2]
	add	r3, r10, r7, lsl #2
	str	r3, [sp, #4]
	cmp	r9, r2, lsr #10
	bne	.L1160
	ldr	r2, [r5, #3304]
	ldr	r8, [r5, #3332]
	ldr	r0, .L1173+8
	str	r2, [r5, #3460]
	str	r8, [r5, #3464]
	ldr	r2, [r10, r7, lsl #2]
	str	r2, [r5, #3456]
	mov	r2, #1
	mov	r1, r2
	bl	FlashReadPages
	ldrh	r2, [r8, #8]
	cmp	r2, fp
	beq	.L1161
	movw	r2, #611
	ldr	r1, .L1173+12
	ldr	r0, .L1173+16
	bl	sftl_printk
.L1161:
	ldr	r2, [r5, #3452]
	cmn	r2, #1
	bne	.L1162
.L1164:
	ldr	r2, [sp, #4]
	mov	r3, #0
	str	r3, [r2]
.L1163:
	b	.L1163
.L1162:
	ldrh	r2, [r8, #8]
	cmp	r2, fp
	bne	.L1164
	ldrh	r2, [r8]
	ldrh	r3, [r4, #4]
	cmp	r2, r3
	bne	.L1164
	ldr	r2, [r5, #3460]
	mov	r1, r7
	mov	r0, r4
	bl	FtlMapWritePage
.L1160:
	add	r6, r6, #1
	b	.L1159
.L1174:
	.align	2
.L1173:
	.word	.LANCHOR0+304
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.word	.LANCHOR1+395
	.word	.LC8
	.fnend
	.size	ftl_map_blk_gc, .-ftl_map_blk_gc
	.align	2
	.global	flush_l2p_region
	.syntax unified
	.arm
	.fpu softvfp
	.type	flush_l2p_region, %function
flush_l2p_region:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r4, #12
	ldr	r5, .L1177
	mul	r4, r4, r0
	ldr	r3, [r5, #2540]
	add	r0, r5, #3392
	add	r2, r3, r4
	ldrh	r1, [r3, r4]
	ldr	r2, [r2, #8]
	bl	FtlMapWritePage
	ldr	r3, [r5, #2540]
	mov	r0, #0
	add	r4, r3, r4
	ldr	r3, [r4, #4]
	bic	r3, r3, #-2147483648
	str	r3, [r4, #4]
	pop	{r4, r5, r6, pc}
.L1178:
	.align	2
.L1177:
	.word	.LANCHOR0
	.fnend
	.size	flush_l2p_region, .-flush_l2p_region
	.align	2
	.global	log2phys
	.syntax unified
	.arm
	.fpu softvfp
	.type	log2phys, %function
log2phys:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	fp, r0
	ldr	r4, .L1193
	mov	r6, r1
	mov	r10, r2
	add	r3, r4, #308
	ldrh	r9, [r3]
	ldr	r3, [r4, #2552]
	cmp	r0, r3
	bcc	.L1180
	mov	r2, #820
	ldr	r1, .L1193+4
	ldr	r0, .L1193+8
	bl	sftl_printk
.L1180:
	ldr	r3, [r4, #2552]
	cmp	fp, r3
	bcs	.L1181
	add	r9, r9, #7
	ldr	r1, [r4, #2540]
	lsr	r7, fp, r9
	movw	r3, #338
	ldrh	r2, [r4, r3]
	mov	r8, #12
	uxth	r7, r7
	mov	r3, #0
.L1182:
	uxth	r5, r3
	cmp	r5, r2
	bcc	.L1187
	bl	select_l2p_ram_region
	mul	r8, r8, r0
	ldr	r3, [r4, #2540]
	mov	r5, r0
	ldrh	r1, [r3, r8]
	add	r2, r3, r8
	movw	r3, #65535
	cmp	r1, r3
	beq	.L1188
	ldr	r3, [r2, #4]
	cmp	r3, #0
	bge	.L1188
	bl	flush_l2p_region
.L1188:
	mov	r1, r5
	mov	r0, r7
	bl	load_l2p_region
	b	.L1184
.L1181:
	mvn	r0, #0
	cmp	r10, #0
	streq	r0, [r6]
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1187:
	add	r3, r3, #1
	mla	r0, r8, r3, r1
	ldrh	r0, [r0, #-12]
	cmp	r0, r7
	bne	.L1182
.L1184:
	mvn	r0, #0
	cmp	r10, #0
	bic	r0, fp, r0, lsl r9
	mov	r3, #12
	uxth	r0, r0
	bne	.L1185
	ldr	r2, [r4, #2540]
	mla	r3, r3, r5, r2
	ldr	r3, [r3, #8]
	ldr	r3, [r3, r0, lsl #2]
	str	r3, [r6]
.L1186:
	ldr	r2, [r4, #2540]
	mov	r3, #12
	mov	r0, #0
	mla	r5, r3, r5, r2
	ldr	r3, [r5, #4]
	cmn	r3, #1
	addne	r3, r3, #1
	strne	r3, [r5, #4]
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1185:
	mul	r3, r3, r5
	ldr	r2, [r4, #2540]
	ldr	r1, [r6]
	add	r2, r2, r3
	ldr	r2, [r2, #8]
	str	r1, [r2, r0, lsl #2]
	ldr	r2, [r4, #2540]
	add	r3, r2, r3
	ldr	r2, [r3, #4]
	orr	r2, r2, #-2147483648
	str	r2, [r3, #4]
	ldr	r3, .L1193+12
	strh	r7, [r3]	@ movhi
	b	.L1186
.L1194:
	.align	2
.L1193:
	.word	.LANCHOR0
	.word	.LANCHOR1+410
	.word	.LC8
	.word	.LANCHOR0+2544
	.fnend
	.size	log2phys, .-log2phys
	.align	2
	.global	FtlWriteDump_data
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlWriteDump_data, %function
FtlWriteDump_data:
	.fnstart
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #36
	sub	sp, sp, #36
	ldr	r4, .L1215
	ldr	r7, .L1215+4
	ldrh	r2, [r4, #28]
	ldr	r3, [r7]
	cmp	r2, #0
	str	r3, [sp, #28]
	beq	.L1196
	ldrb	r3, [r4, #32]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L1196
	movw	r3, #302
	ldrb	r1, [r4, #31]	@ zero_extendqisi2
	ldrh	r3, [r4, r3]
	mul	r3, r3, r1
	cmp	r2, r3
	beq	.L1196
	ldrb	r8, [r4, #34]	@ zero_extendqisi2
	cmp	r8, #0
	bne	.L1195
	ldr	r6, [r4, #2552]
	mov	r2, r8
	add	r1, sp, #4
	ldrh	r9, [r4, #232]
	sub	r6, r6, #1
	mov	r0, r6
	bl	log2phys
	ldr	r3, [sp, #4]
	ldr	r5, [r4, #3332]
	ldr	r0, [r4, #3300]
	cmn	r3, #1
	str	r3, [sp, #12]
	str	r6, [sp, #24]
	str	r0, [sp, #16]
	str	r5, [sp, #20]
	str	r8, [r5, #4]
	beq	.L1198
	mov	r2, r8
	mov	r1, #1
	add	r0, sp, #8
	bl	FlashReadPages
.L1199:
	ldr	r10, .L1215+8
	mov	r8, #0
	ldr	r3, .L1215+12
	lsl	r9, r9, #2
	mov	fp, r8
	strh	r3, [r5]	@ movhi
.L1200:
	cmp	r9, r8
	bne	.L1204
.L1201:
	mov	r3, #1
.L1214:
	strb	r3, [r4, #34]
.L1195:
	ldr	r2, [sp, #28]
	ldr	r3, [r7]
	cmp	r2, r3
	beq	.L1206
	bl	__stack_chk_fail
.L1198:
	movw	r3, #310
	mov	r1, #255
	ldrh	r2, [r4, r3]
	bl	ftl_memset
	b	.L1199
.L1204:
	ldrh	r3, [r4, #28]
	cmp	r3, #0
	beq	.L1201
	ldr	r3, [sp, #12]
	mov	r0, r10
	str	r6, [r5, #8]
	add	r8, r8, #1
	str	r3, [r5, #12]
	ldrh	r3, [r4, #24]
	strh	r3, [r5, #2]	@ movhi
	bl	get_new_active_ppa
	ldr	r3, [r4, #2592]
	mov	r1, #1
	str	r0, [sp, #12]
	add	r0, sp, #8
	str	r3, [r5, #4]
	add	r3, r3, #1
	cmn	r3, #1
	moveq	r3, fp
	str	r3, [r4, #2592]
	mov	r3, #0
	mov	r2, r3
	bl	FlashProgPages
	ldrh	r0, [r4, #24]
	bl	decrement_vpc_count
	b	.L1200
.L1196:
	mov	r3, #0
	b	.L1214
.L1206:
	add	sp, sp, #36
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1216:
	.align	2
.L1215:
	.word	.LANCHOR0
	.word	__stack_chk_guard
	.word	.LANCHOR0+24
	.word	-3947
	.fnend
	.size	FtlWriteDump_data, .-FtlWriteDump_data
	.align	2
	.global	FtlReUsePrevPpa
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlReUsePrevPpa, %function
FtlReUsePrevPpa:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	.pad #8
	mov	r7, r0
	ldr	r5, .L1228
	ubfx	r0, r1, #10, #16
	str	r1, [sp, #4]
	bl	P2V_block_in_plane
	ldr	r2, [r5, #72]
	lsl	r6, r0, #1
	ldrh	r3, [r2, r6]
	cmp	r3, #0
	bne	.L1218
	ldr	r4, [r5, #2532]
	cmp	r4, #0
	beq	.L1219
	ldr	r1, [r5, #2516]
	mov	lr, #6
	ldr	r2, .L1228+4
	movw	r8, #65535
	ldrh	ip, [r5, #224]
	sub	r4, r4, r1
	asr	r4, r4, #1
	mul	r4, r2, r4
	uxth	r4, r4
.L1220:
	uxth	r2, r3
	cmp	ip, r2
	bls	.L1219
	cmp	r4, r0
	bne	.L1221
	mov	r1, r4
	ldr	r0, .L1228+8
	bl	List_remove_node
	ldrh	r3, [r5, #224]
	cmp	r3, #0
	bne	.L1222
	movw	r2, #1742
	ldr	r1, .L1228+12
	ldr	r0, .L1228+16
	bl	sftl_printk
.L1222:
	ldrh	r3, [r5, #224]
	mov	r0, r4
	sub	r3, r3, #1
	strh	r3, [r5, #224]	@ movhi
	bl	INSERT_DATA_LIST
	ldr	r2, [r5, #72]
	ldrh	r3, [r2, r6]
.L1218:
	add	r3, r3, #1
	strh	r3, [r2, r6]	@ movhi
	b	.L1219
.L1221:
	mul	r4, lr, r4
	add	r3, r3, #1
	ldrh	r4, [r1, r4]
	cmp	r4, r8
	bne	.L1220
.L1219:
	mov	r2, #1
	add	r1, sp, #4
	mov	r0, r7
	bl	log2phys
	add	sp, sp, #8
	@ sp needed
	pop	{r4, r5, r6, r7, r8, pc}
.L1229:
	.align	2
.L1228:
	.word	.LANCHOR0
	.word	-1431655765
	.word	.LANCHOR0+2532
	.word	.LANCHOR1+419
	.word	.LC8
	.fnend
	.size	FtlReUsePrevPpa, .-FtlReUsePrevPpa
	.align	2
	.global	FtlRecoverySuperblock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlRecoverySuperblock, %function
FtlRecoverySuperblock:
	.fnstart
	@ args = 0, pretend = 0, frame = 48
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1364
	movw	r2, #65535
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #52
	sub	sp, sp, #52
	ldr	r3, [r3]
	str	r3, [sp, #44]
	ldrh	r3, [r0]
	cmp	r3, r2
	beq	.L1232
	ldrh	r3, [r0, #2]
	mov	fp, r0
	ldr	r5, .L1364+4
	str	r3, [sp, #8]
	ldrb	r3, [r0, #6]	@ zero_extendqisi2
	ldr	r1, [sp, #8]
	str	r3, [sp, #28]
	movw	r3, #302
	ldrh	r3, [r5, r3]
	cmp	r3, r1
	mov	r3, #0
	strheq	r3, [r0, #4]	@ movhi
	ldrhne	r0, [r0, #16]
	bne	.L1234
.L1362:
	strb	r3, [fp, #6]
.L1232:
	ldr	r3, .L1364
	mov	r0, #0
	ldr	r2, [sp, #44]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1300
	bl	__stack_chk_fail
.L1235:
	uxth	r1, r3
	add	r1, fp, r1, lsl #1
	ldrh	r0, [r1, #16]
.L1234:
	cmp	r0, r2
	uxth	r6, r3
	add	r3, r3, #1
	beq	.L1235
	mov	r1, #1
	bl	FtlGetLastWrittenPage
	cmn	r0, #1
	mov	r4, r0
	beq	.L1236
	ldrh	r3, [r5, #232]
	mov	r2, #0
	ldr	r0, [r5, #3288]
	mov	r6, r2
	movw	r8, #65535
	mov	r9, #20
	str	r3, [sp, #4]
	ldr	r3, [r5, #3176]
	str	r3, [sp, #12]
	movw	r3, #310
	ldrh	lr, [r5, r3]
	ldr	r3, .L1364+8
	ldr	r5, [r5, #3180]
	ldrh	r7, [r3]
	add	r3, fp, #16
	mov	ip, r3
	str	r3, [sp, #24]
.L1237:
	ldr	r1, [sp, #4]
	uxth	r3, r2
	cmp	r3, r1
	bcc	.L1241
	ldr	r5, .L1364+4
	mov	r2, #0
	mov	r1, r6
	movw	r9, #65535
	bl	FlashReadPages
	ldr	r3, [r5, #2592]
	uxth	r2, r4
	ldr	r8, [r5, #3288]
	str	r2, [sp, #12]
	sub	r3, r3, #1
	str	r3, [sp, #4]
	mov	r7, r8
	mov	r3, #0
.L1242:
	uxth	r2, r3
	cmp	r6, r2
	bhi	.L1247
	addeq	r10, r4, #1
	uxtheq	r10, r10
	bne	.L1245
.L1357:
	ldr	r0, [r8, #4]
	ubfx	r0, r0, #10, #16
	bl	P2V_plane
	movw	r3, #302
	str	r0, [sp, #12]
	ldrh	r3, [r5, r3]
	ldr	r2, [sp, #12]
	ldr	r1, [sp, #28]
	cmp	r3, r10
	moveq	r3, #0
	strheq	r10, [fp, #2]	@ movhi
	strbeq	r3, [fp, #6]
	strheq	r3, [fp, #4]	@ movhi
	ldr	r3, [sp, #8]
	cmp	r10, r3
	cmpeq	r2, r1
	bne	.L1250
.L1363:
	mov	r1, r10
	mov	r0, fp
	bl	ftl_sb_update_avl_pages
	b	.L1232
.L1236:
	ldr	r3, [sp, #8]
	cmp	r3, #0
	beq	.L1238
	movw	r2, #1809
	ldr	r1, .L1364+12
	ldr	r0, .L1364+16
	bl	sftl_printk
.L1238:
	ldr	r3, [sp, #28]
	cmp	r3, #0
	cmpne	r6, r3
	beq	.L1239
	movw	r2, #1810
	ldr	r1, .L1364+12
	ldr	r0, .L1364+16
	bl	sftl_printk
.L1239:
	mov	r3, #0
	strh	r3, [fp, #2]	@ movhi
	b	.L1362
.L1241:
	ldrh	r3, [ip], #2
	cmp	r3, r8
	beq	.L1240
	mla	r1, r9, r6, r0
	orr	r3, r4, r3, lsl #10
	str	r3, [r1, #4]
	mul	r3, lr, r6
	add	r10, r3, #3
	cmp	r3, #0
	movlt	r3, r10
	ldr	r10, [sp, #12]
	bic	r3, r3, #3
	add	r3, r10, r3
	str	r3, [r1, #8]
	mul	r3, r7, r6
	add	r6, r6, #1
	uxth	r6, r6
	add	r10, r3, #3
	cmp	r3, #0
	movlt	r3, r10
	bic	r3, r3, #3
	add	r3, r5, r3
	str	r3, [r1, #12]
.L1240:
	add	r2, r2, #1
	b	.L1237
.L1247:
	ldr	r2, [r7]
	cmp	r2, #0
	bne	.L1243
	ldr	r10, [r7, #12]
	ldr	r2, [r10, #4]
	cmn	r2, #1
	beq	.L1244
	ldr	r1, [r5, #2592]
	mov	r0, r2
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	addne	r2, r2, #1
	strne	r2, [r5, #2592]
.L1244:
	ldr	r2, [r10]
	cmn	r2, #1
	bne	.L1246
.L1245:
	uxth	r3, r3
	mov	r2, #20
	uxth	r10, r4
	mla	r8, r2, r3, r8
	b	.L1357
.L1243:
	ldr	r9, [sp, #12]
.L1246:
	add	r3, r3, #1
	add	r7, r7, #20
	b	.L1242
.L1250:
	movw	r3, #65535
	cmp	r9, r3
	bne	.L1251
	ldrb	r3, [fp, #8]	@ zero_extendqisi2
	cmp	r3, #0
	bne	.L1252
.L1251:
	ldr	r3, [r5, #3448]
	uxth	r7, r4
	uxth	r4, r4
	ldr	r6, .L1364+4
	movw	r8, #65535
	cmn	r3, #1
	ldreq	r3, [sp, #4]
	streq	r3, [r5, #3448]
	ldr	r3, [sp, #8]
	add	r3, r3, #7
	cmp	r4, r3
	subgt	r4, r7, #7
	ldrle	r4, [sp, #8]
	uxthgt	r4, r4
.L1255:
	cmp	r4, r7
	bhi	.L1262
	mov	r3, #0
	ldrh	lr, [r6, #232]
	ldr	r0, [r6, #3288]
	mov	r5, r3
	ldr	r1, [sp, #24]
	mov	ip, #20
	b	.L1263
.L1257:
	ldrh	r2, [r1], #2
	add	r3, r3, #1
	cmp	r2, r8
	mlane	r9, ip, r5, r0
	addne	r5, r5, #1
	orrne	r2, r4, r2, lsl #10
	uxthne	r5, r5
	strne	r2, [r9, #4]
.L1263:
	uxth	r2, r3
	cmp	lr, r2
	bhi	.L1257
	mov	r2, #0
	mov	r1, r5
	bl	FlashReadPages
	ldr	r3, [r6, #3288]
	mov	r2, #20
	mla	r5, r2, r5, r3
.L1258:
	cmp	r5, r3
	addeq	r4, r4, #1
	uxtheq	r4, r4
	beq	.L1255
.L1261:
	ldr	r2, [r3]
	cmp	r2, #0
	bne	.L1252
	ldr	r2, [r3, #12]
	ldrh	r1, [r2]
	cmp	r1, r8
	beq	.L1260
	ldr	r2, [r2, #4]
	cmn	r2, #1
	strne	r2, [r6, #3448]
.L1260:
	add	r3, r3, #20
	b	.L1258
.L1262:
	mvn	r3, #0
	str	r3, [r6, #3448]
.L1252:
	ldr	r4, .L1364+20
	mov	r3, #1
	ldr	r9, [sp, #8]
	strh	r3, [r4]	@ movhi
	sub	r4, r4, #3472
.L1264:
	ldrh	ip, [r4, #232]
	movw	lr, #65535
	ldr	r0, [r4, #3288]
	mov	r5, #20
	ldr	r1, [sp, #24]
	mov	r3, #0
	str	r3, [sp, #16]
.L1265:
	uxth	r2, r3
	cmp	ip, r2
	bhi	.L1267
	mov	r2, #0
	ldr	r1, [sp, #16]
	bl	FlashReadPages
	mov	r3, #0
.L1361:
	str	r3, [sp, #20]
	ldr	r2, [sp, #16]
	ldrh	r3, [sp, #20]
	cmp	r2, r3
	bhi	.L1295
	ldr	r3, .L1364+24
	add	r9, r9, #1
	uxth	r9, r9
	ldrh	r3, [r3]
	cmp	r3, r9
	bne	.L1264
	ldrh	r2, [r4, #232]
	movw	r0, #65535
	mov	r3, #0
	strh	r9, [fp, #2]	@ movhi
	strh	r3, [fp, #4]	@ movhi
.L1296:
	uxth	r1, r3
	cmp	r1, r2
	bcs	.L1232
	ldr	r1, [sp, #24]
	ldrh	ip, [r1], #2
	cmp	ip, r0
	str	r1, [sp, #24]
	add	r1, r3, #1
	bne	.L1362
	mov	r3, r1
	b	.L1296
.L1267:
	ldrh	r2, [r1], #2
	cmp	r2, lr
	beq	.L1266
	ldr	r6, [sp, #16]
	orr	r2, r9, r2, lsl #10
	mla	r6, r5, r6, r0
	str	r2, [r6, #4]
	ldr	r2, [sp, #16]
	add	r2, r2, #1
	uxth	r2, r2
	str	r2, [sp, #16]
.L1266:
	add	r3, r3, #1
	b	.L1265
.L1295:
	ldr	r3, [sp, #20]
	mov	r6, #20
	ldr	r8, [r4, #3288]
	mul	r6, r6, r3
	add	r7, r8, r6
	ldr	r5, [r7, #4]
	ubfx	r0, r5, #10, #16
	str	r5, [sp, #40]
	bl	P2V_plane
	ldr	r3, [sp, #8]
	cmp	r9, r3
	bcc	.L1269
	ldr	r2, [sp, #28]
	moveq	r3, #1
	movne	r3, #0
	cmp	r2, r0
	movls	r3, #0
	andhi	r3, r3, #1
	cmp	r3, #0
	bne	.L1269
	ldr	r3, [sp, #12]
	cmp	r9, r10
	cmpeq	r3, r0
	beq	.L1270
	ldr	r3, [r8, r6]
	cmn	r3, #1
	beq	.L1271
	ldr	r7, [r7, #12]
	movw	r3, #61589
	ldrh	r2, [r7]
	cmp	r2, r3
	ldrhne	r0, [fp]
	bne	.L1358
	ldr	r3, [r7, #4]
	cmn	r3, #1
	str	r3, [sp, #4]
	beq	.L1273
	mov	r0, r3
	ldr	r1, [r4, #2592]
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	ldrne	r3, [sp, #4]
	addne	r3, r3, #1
	strne	r3, [r4, #2592]
.L1273:
	ldrh	r2, [r7]
	movw	r3, #61589
	cmp	r2, r3
	beq	.L1274
	movw	r2, #1961
	ldr	r1, .L1364+12
	ldr	r0, .L1364+16
	bl	sftl_printk
.L1274:
	ldr	r5, [r7, #8]
	add	r1, sp, #36
	ldr	r3, [r7, #12]
	mov	r2, #0
	mov	r0, r5
	str	r3, [sp, #32]
	bl	log2phys
	ldr	r1, [r4, #3448]
	cmn	r1, #1
	beq	.L1275
	ldr	r0, [sp, #4]
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	beq	.L1275
	ldr	r3, [sp, #32]
	cmn	r3, #1
	beq	.L1276
	ldr	r0, [r4, #3288]
	mov	r2, #0
	mov	r1, #1
	add	r0, r0, r6
	str	r3, [r0, #4]
	ldr	r7, [r0, #12]
	bl	FlashReadPages
	ldr	r2, [r4, #3288]
	ldr	r1, [r2, r6]
	add	r3, r2, r6
	cmn	r1, #1
	bne	.L1277
.L1278:
	mvn	r3, #0
	str	r3, [sp, #32]
.L1285:
	ldr	r7, [sp, #32]
	cmn	r7, #1
	beq	.L1269
.L1299:
	ubfx	r0, r7, #10, #16
	bl	P2V_block_in_plane
	ldrh	r3, [r4, #240]
	mov	r5, r0
	cmp	r3, r0
	bhi	.L1291
	movw	r2, #2066
	ldr	r1, .L1364+12
	ldr	r0, .L1364+16
	bl	sftl_printk
.L1291:
	ldr	r2, [r4, #72]
	lsl	r3, r5, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	beq	.L1292
	mov	r0, r5
.L1358:
	bl	decrement_vpc_count
	b	.L1269
.L1276:
	ldr	r3, [sp, #40]
	ldr	r2, [sp, #36]
	cmp	r2, r3
	bne	.L1269
	mov	r2, #1
	add	r1, sp, #32
	mov	r0, r5
	bl	log2phys
.L1269:
	ldr	r3, [sp, #20]
	add	r3, r3, #1
	b	.L1361
.L1277:
	ldr	r1, [r7, #8]
	cmp	r5, r1
	bne	.L1278
	ldr	r8, [r7, #4]
	ldr	r0, [r4, #3448]
	mov	r1, r8
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	beq	.L1278
	ldr	r1, [sp, #36]
	ldr	r0, [sp, #40]
	cmp	r1, r0
	bne	.L1280
.L1283:
	ldr	r1, [sp, #32]
	mov	r0, r5
	bl	FtlReUsePrevPpa
	b	.L1278
.L1280:
	ldr	r0, [sp, #32]
	cmp	r1, r0
	beq	.L1278
	cmn	r1, #1
	streq	r1, [r2, r6]
	beq	.L1282
	str	r1, [r3, #4]
	mov	r2, #0
	mov	r1, #1
	mov	r0, r3
	ldr	r7, [r3, #12]
	bl	FlashReadPages
.L1282:
	ldr	r3, [r4, #3288]
	ldr	r3, [r3, r6]
	cmn	r3, #1
	beq	.L1283
	ldr	r3, [r7, #4]
	ldr	r0, [r4, #3448]
	mov	r1, r3
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	beq	.L1283
	mov	r1, r3
	mov	r0, r8
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	beq	.L1278
	b	.L1283
.L1275:
	ldr	r3, [sp, #40]
	ldr	r2, [sp, #36]
	cmp	r2, r3
	beq	.L1285
	ldr	r1, [sp, #32]
	cmn	r1, #1
	beq	.L1287
	ldr	r3, [r4, #248]
	cmp	r3, r1, lsr #10
	ldrls	r0, .L1364+28
	bls	.L1360
.L1287:
	mov	r2, #1
	add	r1, sp, #40
	mov	r0, r5
	bl	log2phys
	ldr	r7, [sp, #36]
	cmn	r7, #1
	beq	.L1285
	ldr	r3, [sp, #32]
	cmp	r7, r3
	beq	.L1299
	ubfx	r0, r7, #10, #16
	bl	P2V_block_in_plane
	ldrh	r3, [r4, #24]
	cmp	r3, r0
	beq	.L1290
	ldrh	r3, [r4, #76]
	cmp	r3, r0
	beq	.L1290
	ldrh	r3, [r4, #124]
	cmp	r3, r0
	bne	.L1285
.L1290:
	ldr	r0, [r4, #3288]
	mov	r2, #0
	mov	r1, #1
	str	r7, [r0, #4]
	ldr	r6, [r0, #12]
	bl	FlashReadPages
	ldr	r3, [r4, #3288]
	ldr	r3, [r3]
	cmn	r3, #1
	beq	.L1285
	ldr	r1, [r6, #4]
	ldr	r0, [sp, #4]
	bl	ftl_cmp_data_ver
	cmp	r0, #0
	bne	.L1285
	mov	r2, #1
	add	r1, sp, #36
	mov	r0, r5
	bl	log2phys
	b	.L1285
.L1292:
	ldr	r0, .L1364+32
	mov	r1, r5
.L1360:
	bl	sftl_printk
	b	.L1269
.L1271:
	ldr	r3, [r4, #3476]
	cmp	r3, #31
	addls	r2, r4, r3, lsl #2
	addls	r3, r3, #1
	strls	r3, [r4, #3476]
	strls	r5, [r2, #3480]
	ldrh	r0, [fp]
	bl	decrement_vpc_count
	ldr	r3, [r4, #3448]
	cmn	r3, #1
	ldreq	r3, [sp, #4]
	beq	.L1359
	ldr	r2, [sp, #4]
	cmp	r2, r3
	bcs	.L1269
	mov	r3, r2
.L1359:
	str	r3, [r4, #3448]
	b	.L1269
.L1270:
	ldrb	r3, [sp, #12]	@ zero_extendqisi2
	strh	r10, [fp, #2]	@ movhi
	ldr	r2, [sp, #12]
	strb	r3, [fp, #6]
	b	.L1363
.L1300:
	add	sp, sp, #52
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1365:
	.align	2
.L1364:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR0+312
	.word	.LANCHOR1+435
	.word	.LC8
	.word	.LANCHOR0+3472
	.word	.LANCHOR0+302
	.word	.LC109
	.word	.LC110
	.fnend
	.size	FtlRecoverySuperblock, .-FtlRecoverySuperblock
	.align	2
	.global	ftl_check_vpc
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_check_vpc, %function
ftl_check_vpc:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #20
	sub	sp, sp, #20
	ldr	r5, .L1392
	mov	r4, #0
	ldr	r6, .L1392+4
	ldr	r3, [r5]
	ldr	r1, .L1392+8
	ldr	r0, .L1392+12
	ldr	r7, .L1392+16
	str	r3, [sp, #12]
	bl	sftl_printk
	mov	r2, #8192
	mov	r1, #0
	ldr	r0, .L1392+16
	bl	ftl_memset
	str	r5, [sp, #4]
	mov	r5, r6
.L1367:
	ldr	r3, [r6, #2552]
	cmp	r4, r3
	bcc	.L1369
	ldr	r8, .L1392+16
	mov	r4, #0
	ldr	r10, .L1392+20
	mov	r6, r4
	movw	r9, #65535
.L1370:
	ldrh	r2, [r5, #240]
	uxth	r3, r4
	cmp	r2, r3
	bhi	.L1372
	ldr	r4, [r5, #2532]
	cmp	r4, #0
	beq	.L1373
	ldr	r3, [r5, #2516]
	mov	r7, #0
	ldrh	r8, [r5, #224]
	mov	fp, #6
	ldr	r9, .L1392+16
	sub	r4, r4, r3
	ldr	r3, .L1392+24
	asr	r4, r4, #1
	ldr	r10, .L1392+28
	mul	r4, r3, r4
	uxth	r4, r4
.L1374:
	uxth	r3, r7
	cmp	r8, r3
	bls	.L1373
	ldr	r2, [r5, #72]
	lsl	r3, r4, #1
	ldrh	r2, [r2, r3]
	cmp	r2, #0
	beq	.L1375
	mov	r6, #1
	ldrh	r3, [r9, r3]
	mov	r1, r4
	mov	r0, r10
	bl	sftl_printk
.L1375:
	mul	r4, fp, r4
	ldr	r3, [r5, #2516]
	add	r7, r7, #1
	ldrh	r4, [r3, r4]
	movw	r3, #65535
	cmp	r4, r3
	bne	.L1374
.L1373:
	cmp	r6, #0
	beq	.L1366
	movw	r2, #2394
	ldr	r1, .L1392+8
	ldr	r0, .L1392+32
	bl	sftl_printk
.L1366:
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #12]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1378
	bl	__stack_chk_fail
.L1369:
	mov	r2, #0
	add	r1, sp, #8
	mov	r0, r4
	bl	log2phys
	ldr	r0, [sp, #8]
	cmn	r0, #1
	beq	.L1368
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	lsl	r0, r0, #1
	ldrh	r3, [r7, r0]
	add	r3, r3, #1
	strh	r3, [r7, r0]	@ movhi
.L1368:
	add	r4, r4, #1
	b	.L1367
.L1372:
	uxth	r1, r4
	ldr	r3, [r5, #72]
	lsl	r7, r1, #1
	ldrh	r2, [r3, r7]
	ldrh	r3, [r8, r7]
	cmp	r2, r3
	beq	.L1371
	mov	r0, r10
	bl	sftl_printk
	ldr	r3, [r5, #72]
	ldrh	r3, [r3, r7]
	cmp	r3, r9
	beq	.L1371
	ldrh	r2, [r8, r7]
	cmp	r2, r3
	movhi	r6, #1
.L1371:
	add	r4, r4, #1
	b	.L1370
.L1378:
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1393:
	.align	2
.L1392:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR1+457
	.word	.LC111
	.word	check_vpc_table
	.word	.LC112
	.word	-1431655765
	.word	.LC113
	.word	.LC8
	.fnend
	.size	ftl_check_vpc, .-ftl_check_vpc
	.align	2
	.global	ftl_scan_all_data
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_scan_all_data, %function
ftl_scan_all_data:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	mov	r5, #0
	ldr	r6, .L1404
	.pad #36
	sub	sp, sp, #36
	mov	r1, #0
	ldr	r7, .L1404+4
	ldr	r9, .L1404+8
	ldr	r3, [r6]
	mov	r4, r7
	ldr	r0, .L1404+12
	str	r3, [sp, #28]
	bl	sftl_printk
.L1395:
	ldr	r3, [r7, #2552]
	cmp	r5, r3
	bcc	.L1401
	ldr	r2, [sp, #28]
	ldr	r3, [r6]
	cmp	r2, r3
	beq	.L1402
	bl	__stack_chk_fail
.L1401:
	mov	r2, #0
	add	r1, sp, #24
	mov	r0, r5
	bl	log2phys
	ubfx	r3, r5, #0, #11
	cmp	r3, #0
	bne	.L1396
	ldr	r2, [sp, #24]
	mov	r1, r5
	mov	r0, r9
	bl	sftl_printk
.L1396:
	ldr	r3, [sp, #24]
	cmn	r3, #1
	beq	.L1398
	str	r3, [r4, #3456]
	mov	r2, #0
	ldr	r3, [r4, #3300]
	mov	r1, #1
	ldr	r8, [r4, #3332]
	ldr	r0, .L1404+16
	str	r3, [r4, #3460]
	str	r5, [r4, #3468]
	str	r8, [r4, #3464]
	str	r2, [r4, #3452]
	bl	FlashReadPages
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	cmpne	r3, #256
	beq	.L1399
	ldr	r3, [r8, #8]
	cmp	r5, r3
	beq	.L1398
.L1399:
	ldr	r2, [r4, #3460]
	ldr	r3, [r4, #3464]
	ldr	r0, .L1404+20
	ldr	r1, [r2, #4]
	str	r1, [sp, #16]
	mov	r1, r5
	ldr	r2, [r2]
	str	r2, [sp, #12]
	ldr	r2, [r3, #12]
	str	r2, [sp, #8]
	ldr	r2, [r3, #8]
	str	r2, [sp, #4]
	ldr	r2, [r3, #4]
	str	r2, [sp]
	ldr	r3, [r3]
	ldr	r2, [r4, #3456]
	bl	sftl_printk
.L1398:
	add	r5, r5, #1
	b	.L1395
.L1402:
	add	sp, sp, #36
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L1405:
	.align	2
.L1404:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LC115
	.word	.LC114
	.word	.LANCHOR0+3452
	.word	.LC116
	.fnend
	.size	ftl_scan_all_data, .-ftl_scan_all_data
	.align	2
	.global	FtlGcScanTempBlk
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcScanTempBlk, %function
FtlGcScanTempBlk:
	.fnstart
	@ args = 0, pretend = 0, frame = 56
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1440
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #60
	sub	sp, sp, #60
	mov	r4, r0
	str	r1, [sp, #12]
	ldr	r3, [r3]
	str	r3, [sp, #52]
	ldr	r3, .L1440+4
	ldrh	r5, [r3, #4]
	movw	r3, #65535
	cmp	r5, r3
	beq	.L1432
	cmp	r5, #0
	bne	.L1407
.L1408:
	bl	FtlGcPageVarInit
	b	.L1409
.L1432:
	mov	r5, #0
.L1407:
	ldr	r2, .L1440+8
	movw	r3, #302
	ldrh	r3, [r2, r3]
	ldr	r2, [sp, #12]
	cmp	r3, r2
	beq	.L1408
.L1409:
	ldr	r6, .L1440+8
	mov	r3, #0
	str	r3, [sp, #4]
.L1425:
	ldrh	r2, [r4]
	mov	r3, #0
	strb	r3, [r4, #8]
	movw	r3, #65535
	cmp	r2, r3
	beq	.L1410
.L1428:
	ldrh	r3, [r6, #232]
	mov	r2, #0
	ldr	r0, [r6, #3288]
	add	ip, r4, #16
	ldr	fp, [r6, #3180]
	mov	r8, r2
	str	r3, [sp, #8]
	movw	r7, #65535
	ldr	r3, [r6, #3176]
	mov	r9, #20
	str	r3, [sp, #16]
	ldr	r3, .L1440+12
	ldrh	r3, [r3]
	str	r3, [sp, #20]
	ldr	r3, .L1440+16
	ldrh	lr, [r3]
.L1411:
	ldr	r1, [sp, #8]
	uxth	r3, r2
	cmp	r1, r3
	bhi	.L1413
	mov	r10, #0
	mov	r2, #0
	mov	r1, r8
	bl	FlashReadPages
.L1414:
	uxth	r3, r10
	cmp	r8, r3
	bhi	.L1426
	ldr	r3, [sp, #4]
	add	r5, r5, #1
	uxth	r5, r5
	add	r3, r3, #1
	str	r3, [sp, #4]
	ldr	r2, [sp, #4]
	ldr	r3, [sp, #12]
	cmp	r3, r2
	ldr	r2, .L1440+20
	bls	.L1427
.L1429:
	ldrh	r3, [r2]
	cmp	r3, r5
	bhi	.L1428
.L1410:
	ldr	r3, .L1440+4
	mvn	r2, #0
	strh	r5, [r4, #2]	@ movhi
	mov	r1, r5
	mov	r0, r4
	strh	r2, [r3, #4]	@ movhi
	mov	r2, #0
	strb	r2, [r4, #6]
	bl	ftl_sb_update_avl_pages
	b	.L1430
.L1413:
	ldrh	r3, [ip], #2
	cmp	r3, r7
	beq	.L1412
	mla	r1, r9, r8, r0
	orr	r3, r5, r3, lsl #10
	str	r3, [r1, #4]
	ldr	r3, [sp, #20]
	mul	r3, r3, r8
	add	r10, r3, #3
	cmp	r3, #0
	movlt	r3, r10
	ldr	r10, [sp, #16]
	bic	r3, r3, #3
	add	r3, r10, r3
	str	r3, [r1, #8]
	mul	r3, lr, r8
	add	r8, r8, #1
	uxth	r8, r8
	add	r10, r3, #3
	cmp	r3, #0
	movlt	r3, r10
	bic	r3, r3, #3
	add	r3, fp, r3
	str	r3, [r1, #12]
.L1412:
	add	r2, r2, #1
	b	.L1411
.L1426:
	mov	r3, #20
	ldr	r2, [r6, #3288]
	mul	fp, r3, r10
	ldr	r7, [r2, fp]
	add	r1, r2, fp
	ldr	r3, [r1, #4]
	ldr	r9, [r1, #12]
	cmp	r7, #0
	str	r3, [sp, #8]
	bne	.L1415
	ldrh	r1, [r9]
	movw	r2, #65535
	cmp	r1, r2
	bne	.L1416
.L1438:
	ldrh	r3, [r4]
	mov	r1, #0
	ldr	r2, [r6, #72]
	mov	r5, #0
	lsl	r3, r3, #1
	strh	r1, [r2, r3]	@ movhi
	ldrh	r0, [r4]
	bl	INSERT_FREE_LIST
	mvn	r3, #0
	strh	r3, [r4]	@ movhi
	strh	r3, [r6, #172]	@ movhi
	bl	FtlGcPageVarInit
	b	.L1425
.L1416:
	ldr	r0, [r9, #8]
	ldr	r2, [r6, #2552]
	cmp	r0, r2
	bhi	.L1438
	mov	r2, r7
	add	r1, sp, #28
	bl	log2phys
	ldr	r2, [r9, #12]
	ldr	r1, [sp, #28]
	sub	r0, r2, r1
	cmn	r1, #1
	clz	r0, r0
	lsr	r0, r0, #5
	moveq	r0, #0
	cmp	r0, #0
	bne	.L1419
.L1424:
	ldr	r2, [r9, #8]
.L1439:
	ldr	r1, [sp, #8]
	add	r10, r10, #1
	ldr	r0, [r9, #12]
	bl	FtlGcUpdatePage
	b	.L1414
.L1419:
	str	r2, [sp, #36]
	mov	r1, #1
	ldr	r2, [r6, #3316]
	add	r0, sp, #32
	str	r2, [sp, #40]
	ldr	r2, [r6, #3336]
	str	r2, [sp, #44]
	mov	r2, r7
	bl	FlashReadPages
	ldr	r3, .L1440+24
	ldr	r1, [r6, #3288]
	ldrh	r2, [r3]
	add	r3, r1, fp
	ldr	r1, [sp, #40]
	lsl	r2, r2, #7
.L1421:
	cmp	r7, r2
	beq	.L1424
	ldr	r0, [r3, #8]
	ldr	ip, [r0, r7, lsl #2]
	ldr	r0, [r1, r7, lsl #2]
	cmp	ip, r0
	beq	.L1422
	ldr	r2, [sp, #36]
	ldrh	r1, [r4]
	ldr	r0, .L1440+28
	bl	sftl_printk
	b	.L1438
.L1422:
	add	r7, r7, #1
	b	.L1421
.L1415:
	mvn	r2, #0
	b	.L1439
.L1427:
	ldr	r1, .L1440+4
	movw	r0, #65535
	ldrh	r3, [r1, #4]
	cmp	r3, r0
	beq	.L1429
	ldr	r0, [sp, #4]
	add	r3, r3, r0
	strh	r3, [r1, #4]	@ movhi
	ldrh	r3, [r2]
	cmp	r3, r5
	bls	.L1429
.L1430:
	ldr	r3, .L1440
	mvn	r0, #0
	ldr	r2, [sp, #52]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1431
	bl	__stack_chk_fail
.L1431:
	add	sp, sp, #60
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1441:
	.align	2
.L1440:
	.word	__stack_chk_guard
	.word	.LANCHOR2
	.word	.LANCHOR0
	.word	.LANCHOR0+310
	.word	.LANCHOR0+312
	.word	.LANCHOR0+302
	.word	.LANCHOR0+258
	.word	.LC117
	.fnend
	.size	FtlGcScanTempBlk, .-FtlGcScanTempBlk
	.align	2
	.global	FtlReadRefresh
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlReadRefresh, %function
FtlReadRefresh:
	.fnstart
	@ args = 0, pretend = 0, frame = 96
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #100
	sub	sp, sp, #100
	ldr	r6, .L1460
	ldr	r7, .L1460+4
	ldr	r10, [r6, #2720]
	mov	r5, r6
	ldr	r3, [r7]
	cmp	r10, #0
	str	r3, [sp, #92]
	beq	.L1443
	ldr	r2, [r6, #2724]
	ldr	r3, [r6, #2552]
	cmp	r2, r3
	bcs	.L1444
	mov	r4, #2048
.L1449:
	ldr	r0, [r5, #2724]
	ldr	r3, [r5, #2552]
	cmp	r0, r3
	bcc	.L1445
.L1448:
	mvn	r0, #0
.L1442:
	ldr	r2, [sp, #92]
	ldr	r3, [r7]
	cmp	r2, r3
	beq	.L1454
	bl	__stack_chk_fail
.L1445:
	mov	r2, #0
	add	r1, sp, #4
	bl	log2phys
	ldr	r2, [sp, #4]
	ldr	r3, [r5, #2724]
	cmn	r2, #1
	add	r3, r3, #1
	str	r3, [r5, #2724]
	beq	.L1447
	str	r3, [sp, #24]
	add	r0, sp, #96
	ldr	r3, [r5, #3324]
	mov	r1, #1
	str	r2, [sp, #12]
	mov	r2, #0
	str	r2, [r0, #-88]!
	str	r3, [sp, #16]
	add	r3, sp, #28
	str	r3, [sp, #20]
	bl	FlashReadPages
	ldr	r3, [sp, #8]
	cmp	r3, #256
	bne	.L1448
	ldr	r0, [sp, #4]
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
	b	.L1448
.L1447:
	subs	r4, r4, #1
	bne	.L1449
	b	.L1448
.L1444:
	ldr	r3, [r6, #2556]
	mov	r0, #0
	str	r0, [r6, #2720]
	str	r0, [r6, #2724]
	str	r3, [r6, #2716]
	b	.L1442
.L1443:
	ldr	r1, [r6, #2612]
	movw	r4, #10000
	ldr	r9, [r6, #2556]
	ldr	r8, [r6, #2716]
	cmp	r1, r4
	ldr	fp, .L1460+8
	add	r3, r9, #1048576
	movhi	r4, #31
	movls	r4, #63
	cmp	r8, r3
	bhi	.L1453
	ldr	r3, [r6, #2552]
	lsr	r1, r1, #10
	mov	r0, #1000
	add	r1, r1, #1
	mul	r0, r0, r3
	bl	__aeabi_uidiv
	add	r0, r0, r8
	cmp	r9, r0
	bhi	.L1453
	ldrh	r3, [fp, #28]
	ands	r0, r4, r3
	movne	r0, r10
	bne	.L1442
	ldr	r2, [r6, #2740]
	cmp	r3, r2
	beq	.L1442
.L1453:
	ldrh	r3, [fp, #28]
	mov	r0, #0
	str	r0, [r5, #2724]
	str	r9, [r5, #2716]
	str	r3, [r5, #2740]
	mov	r3, #1
	str	r3, [r5, #2720]
	b	.L1442
.L1454:
	add	sp, sp, #100
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1461:
	.align	2
.L1460:
	.word	.LANCHOR0
	.word	__stack_chk_guard
	.word	.LANCHOR0+2468
	.fnend
	.size	FtlReadRefresh, .-FtlReadRefresh
	.align	2
	.global	l2p_flush
	.syntax unified
	.arm
	.fpu softvfp
	.type	l2p_flush, %function
l2p_flush:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r4, #0
	ldr	r6, .L1467
	mov	r7, #12
	ldr	r5, .L1467+4
	bl	FtlWriteDump_data
.L1463:
	ldrh	r3, [r5]
	uxth	r0, r4
	cmp	r3, r0
	bhi	.L1465
	mov	r0, #0
	pop	{r4, r5, r6, r7, r8, pc}
.L1465:
	ldr	r2, [r6, #2540]
	uxth	r3, r4
	mla	r3, r7, r3, r2
	ldr	r3, [r3, #4]
	cmp	r3, #0
	bge	.L1464
	bl	flush_l2p_region
.L1464:
	add	r4, r4, #1
	b	.L1463
.L1468:
	.align	2
.L1467:
	.word	.LANCHOR0
	.word	.LANCHOR0+338
	.fnend
	.size	l2p_flush, .-l2p_flush
	.align	2
	.global	FtlVendorPartWrite
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlVendorPartWrite, %function
FtlVendorPartWrite:
	.fnstart
	@ args = 0, pretend = 0, frame = 104
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1480
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r10, r2
	ldr	r4, .L1480+4
	.pad #108
	sub	sp, sp, #108
	mov	r6, r1
	add	r1, r0, r1
	ldr	r2, [r3]
	str	r3, [sp, #4]
	str	r2, [sp, #100]
	ldrh	r2, [r4]
	cmp	r1, r2
	mvnhi	r9, #0
	bhi	.L1469
	sub	r4, r4, #296
	mov	r8, r0
	add	r3, r4, #308
	mov	r9, #0
	ldrh	r7, [r3]
	lsr	r7, r0, r7
	lsl	fp, r7, #2
.L1471:
	cmp	r6, #0
	bne	.L1476
.L1469:
	ldr	r3, [sp, #4]
	mov	r0, r9
	ldr	r2, [sp, #100]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1477
	bl	__stack_chk_fail
.L1476:
	ldr	r3, [r4, #3372]
	mov	r0, r8
	ldr	r2, [r3, fp]
	ldr	r3, .L1480+8
	str	r2, [sp, #12]
	ldrh	r3, [r3]
	mov	r1, r3
	str	r3, [sp, #8]
	bl	__aeabi_uidivmod
	ldr	r3, [sp, #8]
	ldr	r2, [sp, #12]
	str	r1, [sp]
	sub	r5, r3, r1
	uxth	r5, r5
	cmp	r6, r5
	uxthcc	r5, r6
	cmp	r2, #0
	cmpne	r5, r3
	movne	r1, #1
	moveq	r1, #0
	beq	.L1473
	ldr	r3, [r4, #3308]
	add	r0, sp, #16
	str	r2, [sp, #20]
	mov	r2, #1
	mov	r1, r2
	str	r3, [sp, #24]
	add	r3, sp, #36
	str	r3, [sp, #28]
	bl	FlashReadPages
.L1474:
	lsl	r3, r5, #9
	ldr	r0, [r4, #3308]
	mov	r1, r10
	mov	r2, r3
	str	r3, [sp, #8]
	ldr	r3, [sp]
	sub	r6, r6, r5
	add	r8, r8, r5
	add	fp, fp, #4
	add	r0, r0, r3, lsl #9
	bl	ftl_memcpy
	mov	r1, r7
	ldr	r2, [r4, #3308]
	ldr	r0, .L1480+12
	add	r7, r7, #1
	bl	FtlMapWritePage
	ldr	r3, [sp, #8]
	cmn	r0, #1
	mvneq	r9, #0
	add	r10, r10, r3
	b	.L1471
.L1473:
	ldr	r3, .L1480+16
	ldr	r0, [r4, #3308]
	ldrh	r2, [r3]
	bl	ftl_memset
	b	.L1474
.L1477:
	add	sp, sp, #108
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1481:
	.align	2
.L1480:
	.word	__stack_chk_guard
	.word	.LANCHOR0+296
	.word	.LANCHOR0+258
	.word	.LANCHOR0+3608
	.word	.LANCHOR0+310
	.fnend
	.size	FtlVendorPartWrite, .-FtlVendorPartWrite
	.align	2
	.global	Ftl_save_ext_data
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_save_ext_data, %function
Ftl_save_ext_data:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L1484
	ldr	r3, .L1484+4
	ldr	r1, [r2, #2640]
	cmp	r1, r3
	bxne	lr
	ldr	r3, .L1484+8
	add	r2, r2, #2640
	mov	r1, #1
	mov	r0, #0
	str	r3, [r2, #4]
	ldr	r3, [r2, #-60]
	str	r3, [r2, #88]
	ldr	r3, [r2, #-56]
	str	r3, [r2, #92]
	ldr	r3, [r2, #-64]
	str	r3, [r2, #8]
	ldr	r3, [r2, #-76]
	str	r3, [r2, #12]
	ldr	r3, [r2, #-84]
	str	r3, [r2, #16]
	ldr	r3, [r2, #-68]
	str	r3, [r2, #20]
	ldr	r3, [r2, #-40]
	str	r3, [r2, #28]
	ldr	r3, [r2, #-32]
	str	r3, [r2, #32]
	ldr	r3, [r2, #-80]
	str	r3, [r2, #36]
	ldr	r3, [r2, #-72]
	str	r3, [r2, #40]
	ldr	r3, [r2, #-28]
	str	r3, [r2, #44]
	ldr	r3, [r2, #-24]
	str	r3, [r2, #48]
	b	FtlVendorPartWrite
.L1485:
	.align	2
.L1484:
	.word	.LANCHOR0
	.word	1179929683
	.word	1342177367
	.fnend
	.size	Ftl_save_ext_data, .-Ftl_save_ext_data
	.align	2
	.global	FtlEctTblFlush
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlEctTblFlush, %function
FtlEctTblFlush:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1494
	movw	r1, #3652
	ldrh	r2, [r3, r1]
	cmp	r2, #31
	addls	r2, r2, #1
	movhi	r2, #32
	strhls	r2, [r3, r1]	@ movhi
	movls	r2, #1
	cmp	r0, #0
	bne	.L1488
	ldr	r1, [r3, #3348]
	ldr	r0, [r1, #20]
	ldr	r1, [r1, #16]
	add	r2, r2, r0
	cmp	r1, r2
	bcc	.L1492
.L1488:
	push	{r4, lr}
	.save {r4, lr}
	mov	r0, #64
	ldr	r2, [r3, #3348]
	ldr	r1, [r2, #16]
	str	r1, [r2, #20]
	ldr	r1, .L1494+4
	str	r1, [r2]
	movw	r1, #3340
	ldrh	r1, [r3, r1]
	ldr	r2, [r3, #3348]
	lsl	r3, r1, #9
	str	r3, [r2, #12]
	ldr	r3, [r2, #8]
	add	r3, r3, #1
	str	r3, [r2, #8]
	mov	r3, #0
	str	r3, [r2, #4]
	bl	FtlVendorPartWrite
	bl	Ftl_save_ext_data
	mov	r0, #0
	pop	{r4, pc}
.L1492:
	mov	r0, #0
	bx	lr
.L1495:
	.align	2
.L1494:
	.word	.LANCHOR0
	.word	1112818501
	.fnend
	.size	FtlEctTblFlush, .-FtlEctTblFlush
	.align	2
	.global	sftl_vendor_write
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_vendor_write, %function
sftl_vendor_write:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	add	r0, r0, #256
	b	FtlVendorPartWrite
	.fnend
	.size	sftl_vendor_write, .-sftl_vendor_write
	.align	2
	.global	FtlVendorPartRead
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlVendorPartRead, %function
FtlVendorPartRead:
	.fnstart
	@ args = 0, pretend = 0, frame = 104
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1509
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r10, r2
	ldr	r5, .L1509+4
	.pad #108
	sub	sp, sp, #108
	mov	r7, r1
	add	r1, r0, r1
	ldr	r2, [r3]
	str	r3, [sp, #8]
	str	r2, [sp, #100]
	ldrh	r2, [r5]
	cmp	r1, r2
	mvnhi	r9, #0
	bhi	.L1497
	sub	r5, r5, #296
	mov	r8, r0
	add	r3, r5, #308
	mov	r9, #0
	ldrh	r6, [r3]
	lsr	r6, r0, r6
	lsl	fp, r6, #2
.L1499:
	cmp	r7, #0
	bne	.L1505
.L1497:
	ldr	r3, [sp, #8]
	mov	r0, r9
	ldr	r2, [sp, #100]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1506
	bl	__stack_chk_fail
.L1505:
	ldr	r3, [r5, #3372]
	mov	r0, r8
	ldr	r3, [r3, fp]
	str	r3, [sp, #12]
	ldr	r3, .L1509+8
	ldrh	r4, [r3]
	mov	r1, r4
	bl	__aeabi_uidivmod
	sub	r4, r4, r1
	ldr	r3, [sp, #12]
	uxth	r4, r4
	str	r1, [sp, #4]
	cmp	r7, r4
	uxthcc	r4, r7
	cmp	r3, #0
	lsl	r2, r4, #9
	str	r2, [sp, #12]
	beq	.L1501
	ldr	r2, [r5, #3308]
	add	r0, sp, #16
	str	r3, [sp, #20]
	str	r3, [sp, #12]
	str	r2, [sp, #24]
	add	r2, sp, #36
	str	r2, [sp, #28]
	mov	r2, #1
	mov	r1, r2
	bl	FlashReadPages
	ldr	r2, [sp, #16]
	ldr	r3, [sp, #12]
	cmn	r2, #1
	ldr	r2, [r5, #3452]
	mvneq	r9, #0
	cmp	r2, #256
	bne	.L1503
	mov	r2, r3
	mov	r1, r6
	ldr	r0, .L1509+12
	bl	sftl_printk
	ldr	r2, [r5, #3308]
	mov	r1, r6
	ldr	r0, .L1509+16
	bl	FtlMapWritePage
.L1503:
	ldr	r1, [r5, #3308]
	lsl	r2, r4, #9
	ldr	r3, [sp, #4]
	mov	r0, r10
	add	r1, r1, r3, lsl #9
	bl	ftl_memcpy
.L1504:
	add	r6, r6, #1
	sub	r7, r7, r4
	add	r8, r8, r4
	add	r10, r10, r4, lsl #9
	add	fp, fp, #4
	b	.L1499
.L1501:
	lsl	r2, r4, #9
	mov	r1, r3
	mov	r0, r10
	bl	ftl_memset
	b	.L1504
.L1506:
	add	sp, sp, #108
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1510:
	.align	2
.L1509:
	.word	__stack_chk_guard
	.word	.LANCHOR0+296
	.word	.LANCHOR0+258
	.word	.LC118
	.word	.LANCHOR0+3608
	.fnend
	.size	FtlVendorPartRead, .-FtlVendorPartRead
	.align	2
	.global	FtlLoadEctTbl
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadEctTbl, %function
FtlLoadEctTbl:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	movw	r5, #3340
	ldr	r4, .L1514
	mov	r0, #64
	ldr	r2, [r4, #3348]
	ldrh	r1, [r4, r5]
	bl	FtlVendorPartRead
	ldr	r3, [r4, #3348]
	ldr	r2, [r3]
	ldr	r3, .L1514+4
	cmp	r2, r3
	beq	.L1512
	ldr	r1, .L1514+8
	ldr	r0, .L1514+12
	bl	sftl_printk
	ldrh	r2, [r4, r5]
	mov	r1, #0
	ldr	r0, [r4, #3348]
	lsl	r2, r2, #9
	bl	ftl_memset
.L1512:
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L1515:
	.align	2
.L1514:
	.word	.LANCHOR0
	.word	1112818501
	.word	.LC119
	.word	.LC77
	.fnend
	.size	FtlLoadEctTbl, .-FtlLoadEctTbl
	.align	2
	.global	Ftl_load_ext_data
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_load_ext_data, %function
Ftl_load_ext_data:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r1, #1
	ldr	r4, .L1520
	mov	r0, #0
	ldr	r5, .L1520+4
	add	r2, r4, #2640
	bl	FtlVendorPartRead
	ldr	r3, [r4, #2640]
	cmp	r3, r5
	beq	.L1517
	mov	r2, #512
	mov	r1, #0
	add	r0, r4, #2640
	bl	ftl_memset
	str	r5, [r4, #2640]
.L1517:
	ldr	r3, [r4, #2640]
	cmp	r3, r5
	bne	.L1518
	ldr	r3, [r4, #2728]
	str	r3, [r4, #2580]
	ldr	r3, [r4, #2732]
	str	r3, [r4, #2584]
	ldr	r3, [r4, #2648]
	str	r3, [r4, #2576]
	ldr	r3, [r4, #2652]
	str	r3, [r4, #2564]
	ldr	r3, [r4, #2656]
	str	r3, [r4, #2556]
	ldr	r3, [r4, #2660]
	str	r3, [r4, #2572]
	ldr	r3, [r4, #2668]
	str	r3, [r4, #2600]
	ldr	r3, [r4, #2672]
	str	r3, [r4, #2608]
	ldr	r3, [r4, #2676]
	str	r3, [r4, #2560]
	ldr	r3, [r4, #2680]
	str	r3, [r4, #2568]
	ldr	r3, [r4, #2684]
	str	r3, [r4, #2612]
	ldr	r3, [r4, #2688]
	str	r3, [r4, #2616]
.L1518:
	ldr	r3, .L1520+8
	ldr	r0, [r4, #2596]
	ldrh	r1, [r4, #240]
	ldrh	r2, [r3]
	ldr	r3, [r4, #2600]
	mla	r0, r0, r2, r3
	bl	__aeabi_uidiv
	str	r0, [r4, #2604]
	pop	{r4, r5, r6, pc}
.L1521:
	.align	2
.L1520:
	.word	.LANCHOR0
	.word	1179929683
	.word	.LANCHOR0+292
	.fnend
	.size	Ftl_load_ext_data, .-Ftl_load_ext_data
	.align	2
	.global	sftl_vendor_read
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_vendor_read, %function
sftl_vendor_read:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	add	r0, r0, #256
	b	FtlVendorPartRead
	.fnend
	.size	sftl_vendor_read, .-sftl_vendor_read
	.align	2
	.global	FtlMapBlkWriteDump_data
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlMapBlkWriteDump_data, %function
FtlMapBlkWriteDump_data:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, [r0, #36]
	cmp	r3, #0
	bxeq	lr
	mov	r3, #0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	str	r3, [r0, #36]
	mov	r5, r0
	ldr	r3, .L1530
	ldrh	r6, [r0, #6]
	ldr	r2, [r0, #24]
	ldr	r1, [r3, #3332]
	mov	r4, r3
	ldr	r0, [r3, #3304]
	sub	r6, r6, #1
	uxth	r6, r6
	str	r1, [r3, #3464]
	str	r0, [r3, #3460]
	ldr	r2, [r2, r6, lsl #2]
	cmp	r2, #0
	str	r2, [r3, #3456]
	beq	.L1525
	mov	r2, #1
	ldr	r0, .L1530+4
	mov	r1, r2
	bl	FlashReadPages
.L1526:
	ldr	r2, [r4, #3460]
	mov	r1, r6
	mov	r0, r5
	pop	{r4, r5, r6, lr}
	b	FtlMapWritePage
.L1525:
	movw	r3, #310
	mov	r1, #255
	ldrh	r2, [r4, r3]
	bl	ftl_memset
	b	.L1526
.L1531:
	.align	2
.L1530:
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.fnend
	.size	FtlMapBlkWriteDump_data, .-FtlMapBlkWriteDump_data
	.align	2
	.global	FtlVpcTblFlush
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlVpcTblFlush, %function
FtlVpcTblFlush:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r3, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r6, #0
	ldr	r4, .L1542
	mov	r1, #255
	ldr	r3, [r4, #3300]
	add	r5, r4, #2624
	ldr	r7, [r4, #3332]
	sub	r8, r5, #156
	add	r9, r4, #304
	str	r3, [r4, #3460]
	mov	fp, r9
	ldrh	r3, [r5]
	add	r10, r9, #6
	str	r7, [r4, #3464]
	str	r6, [r7, #12]
	strh	r3, [r7, #2]	@ movhi
	ldr	r3, .L1542+4
	strh	r3, [r7]	@ movhi
	ldr	r3, [r4, #2632]
	str	r6, [r7, #8]
	ldrh	r2, [r4, #26]
	str	r3, [r7, #4]
	ldr	r3, .L1542+8
	str	r3, [r4, #2468]
	ldr	r3, .L1542+12
	str	r3, [r4, #2472]
	ldrh	r3, [r5, #6]
	strh	r3, [r5, #-148]	@ movhi
	ldrh	r3, [r4, #254]
	strb	r3, [r4, #2478]
	ldrh	r3, [r4, #24]
	strh	r3, [r5, #-142]	@ movhi
	ldrb	r3, [r4, #30]	@ zero_extendqisi2
	orr	r3, r3, r2, lsl #6
	ldrh	r2, [r4, #78]
	strh	r3, [r5, #-140]	@ movhi
	ldrb	r3, [r4, #32]	@ zero_extendqisi2
	strb	r3, [r4, #2479]
	ldrh	r3, [r4, #76]
	strh	r3, [r5, #-138]	@ movhi
	ldrb	r3, [r4, #82]	@ zero_extendqisi2
	orr	r3, r3, r2, lsl #6
	strh	r3, [r5, #-136]	@ movhi
	ldrb	r3, [r4, #84]	@ zero_extendqisi2
	strb	r3, [r4, #2480]
	ldrh	r3, [r4, #124]
	ldrh	r2, [r4, #126]
	ldr	r0, [r4, #3460]
	strh	r3, [r5, #-134]	@ movhi
	ldrb	r3, [r4, #130]	@ zero_extendqisi2
	orr	r3, r3, r2, lsl #6
	strh	r3, [r5, #-132]	@ movhi
	ldrb	r3, [r4, #132]	@ zero_extendqisi2
	strb	r3, [r4, #2481]
	ldr	r3, [r4, #2596]
	str	r3, [r4, #2500]
	ldr	r3, [r4, #2588]
	str	r3, [r4, #2508]
	ldr	r3, [r4, #2592]
	str	r3, [r4, #2504]
	movw	r3, #310
	ldrh	r2, [r4, r3]
	bl	ftl_memset
	mov	r1, r8
	mov	r2, #48
	ldr	r0, [r4, #3460]
	movw	r8, #65535
	bl	ftl_memcpy
	ldrh	r2, [r4, #240]
	ldr	r0, [r4, #3460]
	ldr	r1, [r4, #72]
	lsl	r2, r2, #1
	add	r0, r0, #48
	bl	ftl_memcpy
	mov	r0, r6
	bl	FtlUpdateVaildLpn
.L1533:
	ldr	r3, [r4, #3300]
	ldrh	r1, [r5, #2]
	ldrh	r2, [r5]
	str	r3, [r4, #3460]
	ldr	r3, [r4, #3332]
	str	r3, [r4, #3464]
	orr	r3, r1, r2, lsl #10
	str	r3, [r4, #3456]
	ldrh	r3, [r9]
	sub	r3, r3, #1
	cmp	r1, r3
	blt	.L1534
	mov	r3, #0
	ldrh	r8, [r5, #4]
	strh	r3, [r5, #2]	@ movhi
	strh	r2, [r5, #4]	@ movhi
	bl	FtlFreeSysBlkQueueOut
	ldr	r3, [r4, #2588]
	strh	r0, [r5]	@ movhi
	add	r2, r3, #1
	str	r3, [r4, #2632]
	str	r2, [r4, #2588]
	lsl	r2, r0, #10
	str	r2, [r4, #3456]
	str	r3, [r7, #4]
	strh	r0, [r7, #2]	@ movhi
.L1534:
	ldrh	r1, [r10]
	ldr	r0, [r4, #3300]
	bl	js_hash
	mov	r3, #1
	str	r0, [r7, #12]
	mov	r2, r3
	mov	r1, r3
	ldr	r0, .L1542+16
	bl	FlashProgPages
	ldrh	r3, [r5, #2]
	ldr	r2, [r4, #3452]
	add	r3, r3, #1
	uxth	r3, r3
	cmn	r2, #1
	strh	r3, [r5, #2]	@ movhi
	bne	.L1535
	cmp	r3, #1
	bne	.L1536
	movw	r2, #1145
	ldr	r1, .L1542+20
	ldr	r0, .L1542+24
	bl	sftl_printk
.L1536:
	ldrh	r3, [r5, #2]
	add	r6, r6, #1
	uxth	r6, r6
	cmp	r3, #1
	ldrheq	r3, [fp]
	subeq	r3, r3, #1
	strheq	r3, [r5, #2]	@ movhi
	cmp	r6, #3
	bls	.L1533
	mov	r2, r6
	ldr	r1, [r4, #3456]
	ldr	r0, .L1542+28
	bl	sftl_printk
.L1539:
	b	.L1539
.L1535:
	cmp	r2, #256
	cmpne	r3, #1
	beq	.L1533
	movw	r3, #65535
	cmp	r8, r3
	beq	.L1540
	mov	r1, #1
	mov	r0, r8
	bl	FtlFreeSysBlkQueueIn
.L1540:
	mov	r0, #0
	pop	{r3, r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1543:
	.align	2
.L1542:
	.word	.LANCHOR0
	.word	-3932
	.word	1179929683
	.word	1342177367
	.word	.LANCHOR0+3452
	.word	.LANCHOR1+471
	.word	.LC8
	.word	.LC120
	.fnend
	.size	FtlVpcTblFlush, .-FtlVpcTblFlush
	.align	2
	.global	FtlSysFlush
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlSysFlush, %function
FtlSysFlush:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	bl	l2p_flush
	mov	r0, #1
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
	mov	r0, #0
	pop	{r4, pc}
	.fnend
	.size	FtlSysFlush, .-FtlSysFlush
	.align	2
	.global	sftl_deinit
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_deinit, %function
sftl_deinit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1552
	ldr	r3, [r3]
	cmp	r3, #1
	bne	.L1549
	push	{r4, lr}
	.save {r4, lr}
	bl	FtlSysFlush
	mov	r0, #0
	pop	{r4, pc}
.L1549:
	mov	r0, #0
	bx	lr
.L1553:
	.align	2
.L1552:
	.word	.LANCHOR2
	.fnend
	.size	sftl_deinit, .-sftl_deinit
	.align	2
	.global	sftl_discard
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_discard, %function
sftl_discard:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, lr}
	.save {r4, r5, r6, r7, r8, r9, lr}
	.pad #20
	sub	sp, sp, #20
	ldr	r8, .L1572
	ldr	r4, .L1572+4
	ldr	r3, [r8]
	str	r3, [sp, #12]
	ldr	r3, [r4, #340]
	cmp	r3, r1
	cmpcs	r3, r0
	bls	.L1564
	add	r2, r0, r1
	mov	r7, r0
	cmp	r3, r2
	mov	r5, r1
	bcc	.L1564
	cmp	r1, #31
	bhi	.L1556
.L1561:
	mov	r0, #0
.L1554:
	ldr	r2, [sp, #12]
	ldr	r3, [r8]
	cmp	r2, r3
	beq	.L1562
	bl	__stack_chk_fail
.L1556:
	movw	r3, #258
	ldrh	r6, [r4, r3]
	mov	r1, r6
	bl	__aeabi_uidiv
	smulbb	r3, r0, r6
	mov	r9, r0
	sub	r7, r7, r3
	uxth	r7, r7
	cmp	r7, #0
	beq	.L1557
	sub	r6, r6, r7
	add	r9, r0, #1
	cmp	r6, r5
	movcs	r6, r5
	uxth	r6, r6
	sub	r5, r5, r6
.L1557:
	ldr	r6, .L1572+8
	mvn	r3, #0
	str	r3, [sp, #8]
	mov	r7, r6
.L1558:
	ldrh	r3, [r6]
	cmp	r5, r3
	bcs	.L1560
	ldr	r3, [r4, #3656]
	cmp	r3, #32
	bls	.L1561
	mov	r5, #0
	str	r5, [r4, #3656]
	bl	l2p_flush
	bl	FtlVpcTblFlush
	b	.L1561
.L1560:
	mov	r2, #0
	add	r1, sp, #4
	mov	r0, r9
	bl	log2phys
	ldr	r3, [sp, #4]
	cmn	r3, #1
	beq	.L1559
	ldr	r3, [r4, #3656]
	mov	r2, #1
	add	r1, sp, #8
	mov	r0, r9
	add	r3, r3, #1
	str	r3, [r4, #3656]
	ldr	r3, [r4, #2560]
	add	r3, r3, #1
	str	r3, [r4, #2560]
	bl	log2phys
	ldr	r0, [sp, #4]
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	bl	decrement_vpc_count
.L1559:
	ldrh	r3, [r7]
	add	r9, r9, #1
	sub	r5, r5, r3
	b	.L1558
.L1564:
	mvn	r0, #0
	b	.L1554
.L1562:
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, pc}
.L1573:
	.align	2
.L1572:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR0+258
	.fnend
	.size	sftl_discard, .-sftl_discard
	.align	2
	.global	FtlVpcCheckAndModify
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlVpcCheckAndModify, %function
FtlVpcCheckAndModify:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	mov	r5, #0
	ldr	r7, .L1590
	ldr	r4, .L1590+4
	ldr	r3, [r7]
	ldr	r1, .L1590+8
	ldr	r0, .L1590+12
	str	r3, [sp, #4]
	bl	sftl_printk
	ldrh	r2, [r4, #242]
	mov	r1, #0
	ldr	r0, [r4, #3352]
	lsl	r2, r2, #1
	bl	ftl_memset
.L1575:
	ldr	r3, [r4, #2552]
	cmp	r5, r3
	bcc	.L1577
	ldr	r9, .L1590+16
	mov	r8, #0
	ldr	r10, .L1590+20
	ldr	fp, .L1590+8
.L1578:
	ldrh	r3, [r4, #240]
	uxth	r5, r8
	cmp	r3, r5
	bhi	.L1584
	bl	l2p_flush
	bl	FtlVpcTblFlush
	ldr	r2, [sp, #4]
	ldr	r3, [r7]
	cmp	r2, r3
	beq	.L1585
	bl	__stack_chk_fail
.L1577:
	mov	r2, #0
	mov	r1, sp
	mov	r0, r5
	bl	log2phys
	ldr	r0, [sp]
	cmn	r0, #1
	beq	.L1576
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	ldr	r2, [r4, #3352]
	lsl	r0, r0, #1
	ldrh	r3, [r2, r0]
	add	r3, r3, #1
	strh	r3, [r2, r0]	@ movhi
.L1576:
	add	r5, r5, #1
	b	.L1575
.L1584:
	uxth	r1, r8
	ldr	r3, [r4, #72]
	movw	r0, #65535
	lsl	r6, r1, #1
	ldrh	r2, [r3, r6]
	ldr	r3, [r4, #3352]
	ldrh	r3, [r3, r6]
	cmp	r2, r0
	cmpne	r2, r3
	beq	.L1580
	mov	r0, r9
	bl	sftl_printk
	ldrh	r3, [r4, #24]
	cmp	r3, r5
	beq	.L1580
	ldrh	r3, [r4, #124]
	cmp	r3, r5
	beq	.L1580
	ldrh	r3, [r4, #76]
	cmp	r3, r5
	beq	.L1580
	ldr	r3, [r4, #72]
	ldrh	r2, [r3, r6]
	cmp	r2, #0
	ldr	r2, [r4, #3352]
	ldrh	r2, [r2, r6]
	strh	r2, [r3, r6]	@ movhi
	bne	.L1582
	mov	r1, r5
	mov	r0, r10
	bl	List_remove_node
	ldrh	r3, [r4, #224]
	cmp	r3, #0
	bne	.L1583
	movw	r2, #2330
	mov	r1, fp
	ldr	r0, .L1590+24
	bl	sftl_printk
.L1583:
	ldrh	r3, [r4, #224]
	mov	r0, r5
	sub	r3, r3, #1
	strh	r3, [r4, #224]	@ movhi
	bl	INSERT_DATA_LIST
.L1580:
	add	r8, r8, #1
	b	.L1578
.L1582:
	mov	r0, r5
	bl	update_vpc_list
	b	.L1580
.L1585:
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1591:
	.align	2
.L1590:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR1+486
	.word	.LC111
	.word	.LC121
	.word	.LANCHOR0+2532
	.word	.LC8
	.fnend
	.size	FtlVpcCheckAndModify, .-FtlVpcCheckAndModify
	.align	2
	.global	allocate_new_data_superblock
	.syntax unified
	.arm
	.fpu softvfp
	.type	allocate_new_data_superblock, %function
allocate_new_data_superblock:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r6, r0
	ldr	r4, .L1603
	ldrh	r5, [r0]
	ldrh	r3, [r4, #240]
	cmp	r3, r5
	bcs	.L1593
	movw	r2, #2766
	ldr	r1, .L1603+4
	ldr	r0, .L1603+8
	bl	sftl_printk
.L1593:
	movw	r3, #65535
	cmp	r5, r3
	beq	.L1594
	ldr	r2, [r4, #72]
	lsl	r3, r5, #1
	mov	r0, r5
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	beq	.L1595
	bl	INSERT_DATA_LIST
.L1594:
	mov	r3, #1
	strb	r3, [r6, #8]
	movw	r3, #3438
	ldrh	r0, [r4, r3]
	movw	r3, #65535
	cmp	r0, r3
	beq	.L1596
	cmp	r5, r0
	bne	.L1597
	ldr	r2, [r4, #72]
	lsl	r3, r0, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	beq	.L1598
.L1597:
	bl	update_vpc_list
.L1598:
	mvn	r2, #0
	movw	r3, #3438
	strh	r2, [r4, r3]	@ movhi
.L1596:
	mov	r0, r6
	bl	allocate_data_superblock
	bl	l2p_flush
	mov	r0, #0
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L1595:
	bl	INSERT_FREE_LIST
	b	.L1594
.L1604:
	.align	2
.L1603:
	.word	.LANCHOR0
	.word	.LANCHOR1+507
	.word	.LC8
	.fnend
	.size	allocate_new_data_superblock, .-allocate_new_data_superblock
	.align	2
	.global	FtlProgPages
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlProgPages, %function
FtlProgPages:
	.fnstart
	@ args = 0, pretend = 0, frame = 16
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r5, r3
	ldr	r6, .L1620
	.pad #20
	sub	sp, sp, #20
	mov	r7, #0
	mov	r2, #0
	ldr	fp, .L1620+4
	mov	r4, r0
	ldr	r3, [r6]
	mov	r10, r1
	ldr	r9, .L1620+8
	str	r3, [sp, #12]
	ldrb	r3, [r5, #9]	@ zero_extendqisi2
	bl	FlashProgPages
.L1606:
	cmp	r7, r10
	bne	.L1613
	ldr	r3, .L1620+8
	ldrb	r2, [r5, #6]	@ zero_extendqisi2
	ldrh	r3, [r3, #232]
	cmp	r2, r3
	bcc	.L1605
	mov	r2, #1000
	ldr	r1, .L1620+12
	ldr	r0, .L1620+16
	bl	sftl_printk
.L1605:
	ldr	r2, [sp, #12]
	ldr	r3, [r6]
	cmp	r2, r3
	beq	.L1615
	bl	__stack_chk_fail
.L1608:
	ldr	r1, [r4, #4]
	mov	r0, fp
	bl	sftl_printk
	ldr	r0, [r4, #4]
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	bl	decrement_vpc_count
	ldrh	r3, [r5, #4]
	cmp	r3, #0
	bne	.L1607
	mov	r0, r5
	bl	allocate_new_data_superblock
.L1607:
	mov	r0, r5
	bl	get_new_active_ppa
	mov	r2, #0
	str	r0, [r4, #4]
	str	r0, [sp, #8]
	mov	r1, #1
	ldrb	r3, [r5, #9]	@ zero_extendqisi2
	mov	r0, r4
	bl	FlashProgPages
.L1613:
	ldr	r2, [r4]
	cmn	r2, #1
	cmpne	r2, #256
	beq	.L1608
	ldrb	r2, [r5, #6]	@ zero_extendqisi2
	ldrh	r3, [r9, #232]
	cmp	r2, r3
	bcc	.L1609
	movw	r2, #985
	ldr	r1, .L1620+12
	ldr	r0, .L1620+16
	bl	sftl_printk
.L1609:
	ldr	r3, [r4, #4]
	add	r1, sp, #16
	mov	r2, #1
	ldr	r0, [r4, #16]
	str	r3, [r1, #-8]!
	bl	log2phys
	ldr	r3, [r4, #12]
	ldr	r3, [r3, #12]
	ubfx	r0, r3, #10, #16
	str	r3, [sp, #4]
	bl	P2V_block_in_plane
	ldr	r3, [sp, #4]
	mov	r8, r0
	cmn	r3, #1
	beq	.L1610
	ldr	r2, [r9, #72]
	lsl	r3, r0, #1
	ldrh	r2, [r2, r3]
	cmp	r2, #0
	bne	.L1611
	mov	r1, r0
	ldr	r0, .L1620+20
	bl	sftl_printk
.L1611:
	mov	r0, r8
	bl	decrement_vpc_count
.L1610:
	add	r7, r7, #1
	add	r4, r4, #20
	b	.L1606
.L1615:
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1621:
	.align	2
.L1620:
	.word	__stack_chk_guard
	.word	.LC122
	.word	.LANCHOR0
	.word	.LANCHOR1+536
	.word	.LC8
	.word	.LC123
	.fnend
	.size	FtlProgPages, .-FtlProgPages
	.align	2
	.global	FtlGcFreeTempBlock
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcFreeTempBlock, %function
FtlGcFreeTempBlock:
	.fnstart
	@ args = 0, pretend = 0, frame = 8
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r0, r1, r2, r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #12
	ldr	r6, .L1648
	ldr	r4, .L1648+4
	ldr	r3, [r6]
	ldrh	r2, [r4, #124]
	str	r3, [sp, #4]
	movw	r3, #302
	ldrh	r1, [r4, r3]
	movw	r3, #65535
	cmp	r2, r3
	bne	.L1623
.L1630:
	ldrh	r2, [r4, #124]
	mov	r3, #0
	str	r3, [r4, #3444]
	movw	r3, #65535
	cmp	r2, r3
	bne	.L1644
.L1624:
	mov	r0, #0
	b	.L1622
.L1623:
	cmp	r0, #0
	ldr	r5, .L1648+8
	beq	.L1626
	ldrh	r2, [r5, #4]
	cmp	r2, r3
	beq	.L1627
.L1628:
	mov	r1, #2
.L1626:
	ldr	r0, .L1648+12
	bl	FtlGcScanTempBlk
	ldrh	r2, [r5, #4]
	movw	r3, #65535
	str	r0, [sp]
	cmp	r2, r3
	movne	r0, #1
	beq	.L1630
.L1622:
	ldr	r2, [sp, #4]
	ldr	r3, [r6]
	cmp	r2, r3
	beq	.L1639
	bl	__stack_chk_fail
.L1627:
	mov	r3, #0
	strh	r3, [r5, #4]	@ movhi
	ldrh	r3, [r4, #224]
	cmp	r3, #17
	bhi	.L1628
	b	.L1626
.L1644:
	movw	r3, #3204
	ldrb	r1, [r4, #131]	@ zero_extendqisi2
	ldrh	r2, [r4, r3]
	movw	r3, #302
	ldrh	r3, [r4, r3]
	mul	r3, r3, r1
	cmp	r2, r3
	beq	.L1631
	mov	r2, #164
	ldr	r1, .L1648+16
	ldr	r0, .L1648+20
	bl	sftl_printk
.L1631:
	movw	r0, #302
	ldrb	r2, [r4, #131]	@ zero_extendqisi2
	ldrh	r0, [r4, r0]
	mov	r5, #0
	ldrh	r3, [r4, #124]
	mov	r10, #12
	ldr	r1, [r4, #72]
	ldr	r9, .L1648+24
	smulbb	r2, r2, r0
	lsl	r3, r3, #1
	strh	r2, [r1, r3]	@ movhi
	movw	r3, #3204
	ldr	r2, [r4, #2576]
	ldrh	r3, [r4, r3]
	add	r3, r3, r2
	str	r3, [r4, #2576]
.L1632:
	ldrh	r2, [r9]
	uxth	r3, r5
	cmp	r2, r3
	bhi	.L1636
	movw	r0, #65535
	bl	decrement_vpc_count
	ldrh	r0, [r4, #124]
	ldr	r2, [r4, #72]
	lsl	r3, r0, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	beq	.L1637
	bl	INSERT_DATA_LIST
.L1638:
	mov	r5, #0
	movw	r3, #3204
	strh	r5, [r4, r3]	@ movhi
	mvn	r7, #0
	movw	r3, #3196
	strh	r7, [r4, #124]	@ movhi
	strh	r5, [r4, r3]	@ movhi
	bl	l2p_flush
	bl	FtlVpcTblFlush
	movw	r3, #2620
	ldrh	r2, [r4, #224]
	ldrh	r3, [r4, r3]
	strh	r7, [r4, #172]	@ movhi
	add	r3, r3, r3, lsl #1
	cmp	r2, r3, asr #2
	ldrgt	r3, .L1648+28
	movgt	r2, #20
	strhgt	r2, [r3]	@ movhi
	b	.L1624
.L1636:
	uxth	r8, r5
	ldr	fp, [r4, #3200]
	ldr	r3, [r4, #2552]
	mul	r8, r10, r8
	add	r7, fp, r8
	ldr	r0, [r7, #8]
	cmp	r0, r3
	bcc	.L1633
.L1646:
	ldrh	r0, [r4, #124]
	b	.L1647
.L1633:
	mov	r2, #0
	mov	r1, sp
	bl	log2phys
	ldr	r0, [fp, r8]
	ldr	r3, [sp]
	cmp	r0, r3
	bne	.L1635
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	mov	r2, #1
	mov	r8, r0
	add	r1, r7, #4
	ldr	r0, [r7, #8]
	bl	log2phys
	mov	r0, r8
.L1647:
	bl	decrement_vpc_count
	b	.L1634
.L1635:
	ldr	r2, [r7, #4]
	cmp	r3, r2
	bne	.L1646
.L1634:
	add	r5, r5, #1
	b	.L1632
.L1637:
	bl	INSERT_FREE_LIST
	b	.L1638
.L1639:
	add	sp, sp, #12
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1649:
	.align	2
.L1648:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR2
	.word	.LANCHOR0+124
	.word	.LANCHOR1+549
	.word	.LC8
	.word	.LANCHOR0+3204
	.word	.LANCHOR0+3152
	.fnend
	.size	FtlGcFreeTempBlock, .-FtlGcFreeTempBlock
	.align	2
	.global	FtlGcPageRecovery
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlGcPageRecovery, %function
FtlGcPageRecovery:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	movw	r5, #302
	ldr	r4, .L1653
	ldrh	r1, [r4, r5]
	add	r0, r4, #124
	bl	FtlGcScanTempBlk
	ldrh	r2, [r4, #126]
	ldrh	r3, [r4, r5]
	cmp	r2, r3
	popcc	{r4, r5, r6, pc}
	add	r0, r4, #3392
	bl	FtlMapBlkWriteDump_data
	mov	r0, #0
	bl	FtlGcFreeTempBlock
	mov	r3, #0
	str	r3, [r4, #3444]
	pop	{r4, r5, r6, pc}
.L1654:
	.align	2
.L1653:
	.word	.LANCHOR0
	.fnend
	.size	FtlGcPageRecovery, .-FtlGcPageRecovery
	.align	2
	.global	FtlPowerLostRecovery
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlPowerLostRecovery, %function
FtlPowerLostRecovery:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r5, #0
	ldr	r4, .L1657
	add	r6, r4, #24
	str	r5, [r4, #3476]
	mov	r0, r6
	add	r4, r4, #76
	bl	FtlRecoverySuperblock
	mov	r0, r6
	bl	FtlSlcSuperblockCheck
	mov	r0, r4
	bl	FtlRecoverySuperblock
	mov	r0, r4
	bl	FtlSlcSuperblockCheck
	bl	FtlGcPageRecovery
	movw	r0, #65535
	bl	decrement_vpc_count
	mov	r0, r5
	pop	{r4, r5, r6, pc}
.L1658:
	.align	2
.L1657:
	.word	.LANCHOR0
	.fnend
	.size	FtlPowerLostRecovery, .-FtlPowerLostRecovery
	.align	2
	.global	Ftl_gc_temp_data_write_back
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_gc_temp_data_write_back, %function
Ftl_gc_temp_data_write_back:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r3, #0
	ldr	r4, .L1668
	mov	r5, #0
	mov	r6, #20
	mov	r2, r3
	ldr	r1, [r4, #3168]
	ldr	r0, [r4, #3292]
	bl	FlashProgPages
.L1660:
	ldr	r1, [r4, #3168]
	uxth	r3, r5
	cmp	r1, r3
	bhi	.L1663
	ldr	r0, [r4, #3292]
	bl	FtlGcBufFree
	ldrh	r3, [r4, #128]
	mov	r0, #0
	str	r0, [r4, #3168]
	cmp	r3, r0
	popne	{r4, r5, r6, pc}
	mov	r0, #1
	bl	FtlGcFreeTempBlock
	mov	r0, #1
	pop	{r4, r5, r6, pc}
.L1663:
	mul	r3, r6, r3
	ldr	r2, [r4, #3292]
	add	r5, r5, #1
	add	r1, r2, r3
	ldr	r2, [r2, r3]
	ldr	r0, [r1, #12]
	ldr	r1, [r1, #4]
	cmn	r2, #1
	ldrne	r2, [r0, #8]
	ldr	r0, [r0, #12]
	bl	FtlGcUpdatePage
	b	.L1660
.L1669:
	.align	2
.L1668:
	.word	.LANCHOR0
	.fnend
	.size	Ftl_gc_temp_data_write_back, .-Ftl_gc_temp_data_write_back
	.align	2
	.global	Ftl_get_new_temp_ppa
	.syntax unified
	.arm
	.fpu softvfp
	.type	Ftl_get_new_temp_ppa, %function
Ftl_get_new_temp_ppa:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	movw	r3, #65535
	ldr	r4, .L1674
	ldrh	r2, [r4, #124]
	cmp	r2, r3
	beq	.L1671
	ldrh	r3, [r4, #128]
	cmp	r3, #0
	bne	.L1672
.L1671:
	mov	r0, #0
	mov	r5, #0
	bl	FtlGcFreeTempBlock
	ldr	r0, .L1674+4
	strb	r5, [r4, #132]
	bl	allocate_data_superblock
	movw	r3, #3196
	strh	r5, [r4, r3]	@ movhi
	movw	r3, #3204
	strh	r5, [r4, r3]	@ movhi
	bl	l2p_flush
	mov	r0, r5
	bl	FtlEctTblFlush
	bl	FtlVpcTblFlush
.L1672:
	ldr	r0, .L1674+4
	pop	{r4, r5, r6, lr}
	b	get_new_active_ppa
.L1675:
	.align	2
.L1674:
	.word	.LANCHOR0
	.word	.LANCHOR0+124
	.fnend
	.size	Ftl_get_new_temp_ppa, .-Ftl_get_new_temp_ppa
	.align	2
	.global	rk_ftl_garbage_collect
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_ftl_garbage_collect, %function
rk_ftl_garbage_collect:
	.fnstart
	@ args = 0, pretend = 0, frame = 40
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1803
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #52
	sub	sp, sp, #52
	str	r0, [sp, #20]
	ldr	r3, [r3]
	str	r3, [sp, #44]
	ldr	r3, .L1803+4
	ldr	r0, [r3, #3272]
	cmp	r0, #0
	movne	r0, #0
	bne	.L1676
	add	r2, r3, #2528
	ldrh	r2, [r2]
	cmp	r2, #47
	bls	.L1676
	ldr	r2, .L1803+8
	mov	r4, r3
	ldrh	r1, [r2, #4]
	movw	r2, #65535
	cmp	r1, r2
	bne	.L1678
.L1681:
	ldrh	r3, [r4, #220]
	movw	r2, #65535
	cmp	r3, r2
	bne	.L1679
.L1680:
	bl	FtlReadRefresh
	ldr	r3, [r4, #3156]
	movw	r7, #65535
	ldr	r2, [sp, #20]
	add	r3, r3, #1
	add	r3, r3, r2, lsl #7
	ldrh	r2, [r4, #172]
	str	r3, [r4, #3156]
	cmp	r2, r7
	bne	.L1682
	ldrh	r7, [r4, #124]
	cmp	r7, r2
	movne	r7, r2
	bne	.L1682
	ldrh	r5, [r4, #222]
	cmp	r5, r7
	bne	.L1682
	ldrh	r2, [r4, #224]
	cmp	r2, #24
	movcc	r2, #5120
	movcs	r2, #1024
	cmp	r3, r2
	movls	r7, r5
	bls	.L1682
	mov	r8, #0
	movw	r3, #3660
	str	r8, [r4, #3156]
	strh	r8, [r4, r3]	@ movhi
	bl	GetSwlReplaceBlock
	cmp	r0, r5
	mov	r7, r0
	movne	r5, r0
	bne	.L1684
	movw	r6, #3154
	ldrh	r2, [r4, #224]
	ldrh	r3, [r4, r6]
	cmp	r2, r3
	bcs	.L1685
	mov	r0, #64
	bl	List_get_gc_head_node
	uxth	r3, r0
	cmp	r3, r7
	beq	.L1687
	mov	r0, r3
	ldr	r3, [r4, #72]
	lsl	r0, r0, #1
	ldrh	r3, [r3, r0]
	cmp	r3, #7
	bhi	.L1688
	mov	r0, r8
	bl	List_get_gc_head_node
	uxth	r5, r0
	mov	r3, #128
	strh	r3, [r4, r6]	@ movhi
	cmp	r5, r7
	bne	.L1684
.L1687:
	bl	FtlGcReFreshBadBlk
.L1682:
	movw	r1, #65535
	ldr	r2, [sp, #20]
	sub	r3, r7, r1
	clz	r3, r3
	lsr	r3, r3, #5
	cmp	r2, #0
	movne	r2, #0
	andeq	r2, r3, #1
	cmp	r2, #0
	beq	.L1690
	ldrh	r3, [r4, #224]
	cmp	r3, #24
	movhi	r6, #1
	bhi	.L1691
	movw	r2, #302
	cmp	r3, #16
	ldrh	r6, [r4, r2]
	lsrhi	r6, r6, #5
	bhi	.L1691
	cmp	r3, #12
	lsrhi	r6, r6, #4
	bhi	.L1691
	cmp	r3, #8
	lsrhi	r6, r6, #2
.L1691:
	ldr	r2, .L1803+12
	ldrh	r1, [r2]
	cmp	r1, r3
	mov	r1, r2
	bcs	.L1695
	ldrh	r3, [r4, #124]
	movw	r0, #65535
	cmp	r3, r0
	bne	.L1696
	ldrh	r0, [r4, #222]
	cmp	r0, r3
	bne	.L1696
	movw	r3, #3660
	ldrh	r0, [r4, r3]
	cmp	r0, #0
	bne	.L1697
	ldr	r3, [r4, #2552]
	ldr	ip, [r4, #2548]
	add	r3, r3, r3, lsl #1
	cmp	ip, r3, lsr #2
	movcs	r3, #18
	strhcs	r3, [r2]	@ movhi
	bcs	.L1699
.L1697:
	movw	r3, #2620
	ldrh	r3, [r4, r3]
	add	r3, r3, r3, lsl #1
	asr	r3, r3, #2
	strh	r3, [r1]	@ movhi
.L1699:
	mov	r3, #0
	str	r3, [r4, #3164]
.L1676:
	ldr	r3, .L1803
	ldr	r2, [sp, #44]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1754
	bl	__stack_chk_fail
.L1678:
	ldrh	r3, [r3, #124]
	cmp	r3, r2
	beq	.L1681
	mov	r0, #1
	bl	FtlGcFreeTempBlock
	cmp	r0, #0
	beq	.L1681
	mov	r0, #1
	b	.L1676
.L1679:
	ldrh	r1, [r4, #222]
	cmp	r1, r2
	strheq	r3, [r4, #222]	@ movhi
	mvneq	r3, #0
	strheq	r3, [r4, #220]	@ movhi
	b	.L1680
.L1688:
	mov	r3, #64
.L1799:
	strh	r3, [r4, r6]	@ movhi
	b	.L1687
.L1685:
	mov	r3, #80
	b	.L1799
.L1684:
	ldr	r0, .L1803+12
	lsl	r1, r5, #1
	ldr	r3, [r4, #72]
	mov	r7, r5
	ldrh	r2, [r4, #224]
	ldrh	r0, [r0]
	ldrh	r3, [r3, r1]
	str	r0, [sp, #4]
	ldr	r0, [r4, #2536]
	ldrh	r1, [r0, r1]
	ldr	r0, .L1803+16
	str	r1, [sp]
	mov	r1, r5
	bl	sftl_printk
	b	.L1687
.L1696:
	movw	r3, #2620
	ldrh	r3, [r4, r3]
	add	r3, r3, r3, lsl #1
	asr	r3, r3, #2
	strh	r3, [r1]	@ movhi
.L1695:
	movw	r3, #3206
	movw	r7, #65535
	ldrh	r3, [r4, r3]
	cmp	r3, #0
	addne	r6, r6, #32
	uxthne	r6, r6
.L1701:
	ldrh	r3, [r4, #172]
	movw	r2, #65535
	cmp	r3, r2
	bne	.L1710
	cmp	r7, r3
	strhne	r7, [r4, #172]	@ movhi
	bne	.L1712
	ldrh	r3, [r4, #222]
	cmp	r3, r7
	beq	.L1712
	ldr	r2, [r4, #72]
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	mvneq	r3, #0
	strheq	r3, [r4, #222]	@ movhi
	ldrh	r3, [r4, #222]
	strh	r3, [r4, #172]	@ movhi
	mvn	r3, #0
	strh	r3, [r4, #222]	@ movhi
.L1712:
	ldrh	r0, [r4, #172]
	mov	r3, #0
	strb	r3, [r4, #180]
	movw	r3, #65535
	cmp	r0, r3
	beq	.L1710
	bl	IsBlkInGcList
	cmp	r0, #0
	mvnne	r3, #0
	strhne	r3, [r4, #172]	@ movhi
	movw	r3, #65535
	ldrh	r2, [r4, #172]
	cmp	r2, r3
	beq	.L1710
	ldr	r0, .L1803+20
	bl	make_superblock
	mov	r3, #0
	movw	r2, #3662
	strh	r3, [r4, r2]	@ movhi
	strh	r3, [r4, #174]	@ movhi
	strb	r3, [r4, #178]
	ldrh	r3, [r4, #172]
	ldr	r2, [r4, #72]
	lsl	r3, r3, #1
	ldrh	r2, [r2, r3]
	ldr	r3, .L1803+24
	strh	r2, [r3]	@ movhi
.L1710:
	ldrh	r3, [r4, #172]
	ldrh	r2, [r4, #24]
	cmp	r2, r3
	beq	.L1716
	ldrh	r2, [r4, #76]
	cmp	r2, r3
	beq	.L1716
	ldrh	r2, [r4, #124]
	cmp	r2, r3
	bne	.L1717
.L1716:
	mvn	r3, #0
	strh	r3, [r4, #172]	@ movhi
.L1717:
	ldr	r5, .L1803+4
	mov	r4, r5
.L1752:
	ldrh	r2, [r5, #172]
	movw	r3, #65535
	cmp	r2, r3
	bne	.L1718
	ldr	fp, .L1803+28
	mov	r3, #0
	ldr	r10, .L1803+32
	str	r3, [r5, #3164]
	mov	r9, fp
.L1719:
	ldrh	r8, [fp]
	mov	r0, r8
	bl	List_get_gc_head_node
	uxth	r2, r0
	movw	r1, #65535
	cmp	r2, r1
	strh	r2, [r4, #172]	@ movhi
	bne	.L1720
	mov	r3, #0
	mov	r0, #8
	strh	r3, [fp]	@ movhi
	b	.L1676
.L1690:
	ldrh	r6, [r4, #124]
	cmp	r6, r1
	bne	.L1767
	ldrh	r1, [r4, #222]
	cmp	r1, r6
	movne	r3, #0
	andeq	r3, r3, #1
	cmp	r3, #0
	beq	.L1767
	ldrh	r3, [r4, #172]
	cmp	r3, r6
	beq	.L1702
.L1707:
	mov	r7, r6
.L1767:
	mov	r6, #1
	b	.L1701
.L1702:
	ldr	r5, .L1803+12
	str	r2, [r4, #3164]
	ldrh	r2, [r4, #224]
	ldrh	r3, [r5]
	cmp	r2, r3
	bls	.L1703
	movw	r3, #3660
	ldrh	r3, [r4, r3]
	cmp	r3, #0
	bne	.L1704
	ldr	r3, [r4, #2552]
	ldr	r2, [r4, #2548]
	add	r3, r3, r3, lsl #1
	cmp	r2, r3, lsr #2
	movcs	r3, #18
	bcs	.L1800
.L1704:
	movw	r3, #2620
	ldrh	r3, [r4, r3]
	add	r3, r3, r3, lsl #1
	asr	r3, r3, #2
.L1800:
	strh	r3, [r5]	@ movhi
	bl	FtlReadRefresh
	mov	r0, #0
	bl	List_get_gc_head_node
	uxth	r0, r0
	ldr	r3, [r4, #72]
	lsl	r0, r0, #1
	ldrh	r3, [r3, r0]
	cmp	r3, #4
	bls	.L1703
.L1802:
	movw	r3, #3660
	ldrh	r0, [r4, r3]
	b	.L1676
.L1703:
	movw	r7, #3660
	ldrh	r0, [r4, r7]
	cmp	r0, #0
	bne	.L1707
	movw	r3, #2620
	ldrh	r8, [r4, r3]
	add	r3, r8, r8, lsl #1
	asr	r3, r3, #2
	strh	r3, [r5]	@ movhi
	bl	List_get_gc_head_node
	uxth	r0, r0
	ldr	r3, [r4, #72]
	lsl	r0, r0, #1
	ldrh	r2, [r3, r0]
	ldr	r3, .L1803+36
	ldrh	r1, [r3]
	ldrh	r3, [r4, #232]
	mul	r3, r3, r1
	add	r3, r3, r3, lsr #31
	cmp	r2, r3, asr #1
	ble	.L1708
	ldrh	r3, [r4, #224]
	sub	r8, r8, #1
	cmp	r3, r8
	blt	.L1708
	bl	FtlReadRefresh
	ldrh	r0, [r4, r7]
	b	.L1676
.L1708:
	cmp	r2, #0
	bne	.L1707
	movw	r0, #65535
	bl	decrement_vpc_count
	ldrh	r0, [r4, #224]
	add	r0, r0, #1
	b	.L1676
.L1720:
	str	r0, [sp, #16]
	mov	r0, r2
	str	r2, [sp, #12]
	add	r8, r8, #1
	bl	IsBlkInGcList
	cmp	r0, #0
	ldr	r2, [sp, #12]
	ldr	r3, [sp, #16]
	strhne	r8, [fp]	@ movhi
	bne	.L1719
	uxth	r3, r3
	ldrh	lr, [r4, #232]
	ldr	r0, [r4, #72]
	uxth	r8, r8
	lsl	r1, r3, #1
	ldrh	r3, [r10]
	strh	r8, [fp]	@ movhi
	ldrh	ip, [r0, r1]
	mul	r3, lr, r3
	add	lr, r3, r3, lsr #31
	cmp	ip, lr, asr #1
	bgt	.L1723
	cmp	r8, #48
	cmphi	ip, #8
	bls	.L1724
	ldr	ip, .L1803+40
	ldrh	ip, [ip]
	cmp	ip, #35
	bhi	.L1724
.L1723:
	mov	ip, #0
	strh	ip, [r9]	@ movhi
.L1724:
	ldrh	r1, [r0, r1]
	movw	r0, #65535
	cmp	r3, r1
	cmple	r7, r0
	bne	.L1725
	ldrh	r3, [r9]
	cmp	r3, #3
	mvnls	r3, #0
	strhls	r3, [r4, #172]	@ movhi
	movls	r3, #0
	strhls	r3, [r9]	@ movhi
	bls	.L1802
.L1725:
	cmp	r1, #0
	bne	.L1726
	movw	r0, #65535
	bl	decrement_vpc_count
	ldrh	r3, [r9]
	add	r3, r3, #1
	strh	r3, [r9]	@ movhi
	b	.L1719
.L1726:
	mov	r3, #0
	strb	r3, [r4, #180]
	ldrh	r3, [r4, #24]
	cmp	r3, r2
	bne	.L1727
	movw	r2, #717
	ldr	r1, .L1803+44
	ldr	r0, .L1803+48
	bl	sftl_printk
.L1727:
	ldrh	r2, [r4, #172]
	ldrh	r3, [r4, #76]
	cmp	r2, r3
	bne	.L1728
	movw	r2, #718
	ldr	r1, .L1803+44
	ldr	r0, .L1803+48
	bl	sftl_printk
.L1728:
	ldrh	r2, [r4, #172]
	ldrh	r3, [r4, #124]
	cmp	r2, r3
	bne	.L1729
	movw	r2, #719
	ldr	r1, .L1803+44
	ldr	r0, .L1803+48
	bl	sftl_printk
.L1729:
	ldr	r0, .L1803+20
	bl	make_superblock
	ldrh	r2, [r4, #172]
	mov	r3, #0
	ldr	r1, .L1803+52
	ldr	r0, [r4, #72]
	lsl	r2, r2, #1
	strh	r3, [r1]	@ movhi
	ldrh	r2, [r0, r2]
	strh	r3, [r4, #174]	@ movhi
	strb	r3, [r4, #178]
	strh	r2, [r1, #2]	@ movhi
.L1718:
	mov	r3, #1
	str	r3, [r4, #3272]
	ldr	r3, .L1803+32
	ldrh	r3, [r3]
	str	r3, [sp, #12]
	ldr	r3, [sp, #20]
	cmp	r3, #0
	beq	.L1730
	ldr	r2, [sp, #12]
	ldrh	r3, [r4, #232]
	ldr	r1, [r4, #72]
	mul	r3, r2, r3
	ldrh	r2, [r4, #172]
	lsl	r2, r2, #1
	ldrh	r2, [r1, r2]
	sub	r3, r3, r2
	add	r2, r3, #3
	cmp	r3, #0
	movlt	r3, r2
	add	r6, r6, r3, asr #2
	uxth	r6, r6
.L1730:
	ldrh	r3, [r4, #174]
	ldr	r1, [sp, #12]
	add	r2, r3, r6
	cmp	r2, r1
	movgt	r2, r1
	subgt	r6, r2, r3
	mov	r3, #0
	uxthgt	r6, r6
.L1801:
	str	r3, [sp, #16]
	ldrh	r3, [sp, #16]
	cmp	r6, r3
	bls	.L1740
	ldr	r3, [sp, #16]
	movw	r9, #65535
	ldrh	r8, [r5, #174]
	mov	r10, #20
	ldrh	ip, [r5, #232]
	ldr	r0, [r5, #3184]
	add	r8, r8, r3
	mov	r3, #0
	ldr	r1, .L1803+56
	mov	fp, r3
	b	.L1741
.L1734:
	ldrh	r2, [r1, #2]!
	add	r3, r3, #1
	cmp	r2, r9
	mlane	lr, r10, fp, r0
	orrne	r2, r8, r2, lsl #10
	strne	r2, [lr, #4]
	addne	r2, fp, #1
	uxthne	fp, r2
.L1741:
	uxth	r2, r3
	cmp	ip, r2
	bhi	.L1734
	mov	r10, #0
	ldrb	r2, [r5, #180]	@ zero_extendqisi2
	mov	r1, fp
	bl	FlashReadPages
.L1735:
	uxth	r3, r10
	cmp	fp, r3
	ldrls	r3, [sp, #16]
	addls	r3, r3, #1
	bls	.L1801
.L1739:
	mov	r8, #20
	ldr	r3, [r5, #3184]
	mul	r8, r8, r10
	add	r2, r3, r8
	ldr	r3, [r3, r8]
	cmn	r3, #1
	beq	.L1736
	ldr	r9, [r2, #12]
	movw	r3, #61589
	ldrh	r2, [r9]
	cmp	r2, r3
	bne	.L1736
	ldr	r3, [r9, #8]
	cmn	r3, #1
	bne	.L1737
	movw	r2, #753
	ldr	r1, .L1803+44
	ldr	r0, .L1803+48
	str	r3, [sp, #24]
	bl	sftl_printk
	ldr	r3, [sp, #24]
.L1737:
	mov	r2, #0
	add	r1, sp, #36
	mov	r0, r3
	bl	log2phys
	ldr	r1, [r4, #3184]
	ldr	r3, [sp, #36]
	add	r1, r1, r8
	ldr	r2, [r1, #4]
	cmp	r2, r3
	bne	.L1736
	ldr	r3, .L1803+52
	ldr	r2, .L1803+52
	ldr	r0, [r4, #3168]
	ldrh	r3, [r3]
	ldr	r1, [r1, #16]
	add	r3, r3, #1
	strh	r3, [r2]	@ movhi
	mov	r2, #20
	ldr	r3, [r4, #3292]
	str	r2, [sp, #28]
	mla	r3, r2, r0, r3
	str	r1, [r3, #16]
	str	r3, [sp, #24]
	bl	Ftl_get_new_temp_ppa
	ldr	r3, [sp, #24]
	ldr	r1, [r4, #3292]
	ldr	r2, [sp, #28]
	str	r0, [r3, #4]
	ldr	r3, [r4, #3168]
	mla	r3, r2, r3, r1
	ldr	r2, [r4, #3184]
	add	r2, r2, r8
	ldr	r1, [r2, #8]
	str	r1, [r3, #8]
	mov	r1, #1
	ldr	r2, [r2, #12]
	str	r2, [r3, #12]
	ldr	r3, [sp, #36]
	str	r3, [r9, #12]
	ldrh	r3, [r4, #124]
	strh	r3, [r9, #2]	@ movhi
	ldr	r3, [r4, #2592]
	ldr	r0, [r4, #3184]
	str	r3, [r9, #4]
	ldr	r3, [r4, #3168]
	add	r0, r0, r8
	add	r3, r3, #1
	str	r3, [r4, #3168]
	bl	FtlGcBufAlloc
	ldrb	r2, [r4, #131]	@ zero_extendqisi2
	ldr	r3, [r4, #3168]
	cmp	r2, r3
	beq	.L1738
	ldrh	r3, [r4, #128]
	cmp	r3, #0
	bne	.L1736
.L1738:
	bl	Ftl_gc_temp_data_write_back
	cmp	r0, #0
	beq	.L1736
	ldr	r3, .L1803+4
	mov	r2, #0
	mvn	r1, #0
	str	r2, [r3, #3272]
	strh	r2, [r3, #174]	@ movhi
	movw	r2, #3660
	strh	r1, [r3, #172]	@ movhi
	ldrh	r0, [r3, r2]
	b	.L1676
.L1736:
	add	r10, r10, #1
	b	.L1735
.L1740:
	ldrh	r3, [r5, #174]
	add	r6, r6, r3
	ldr	r3, [sp, #12]
	uxth	r6, r6
	cmp	r3, r6
	strh	r6, [r5, #174]	@ movhi
	bhi	.L1742
	ldr	r3, [r5, #3168]
	cmp	r3, #0
	beq	.L1743
	bl	Ftl_gc_temp_data_write_back
	cmp	r0, #0
	movne	r3, #0
	strne	r3, [r5, #3272]
	movwne	r3, #3660
	ldrhne	r0, [r5, r3]
	bne	.L1676
.L1743:
	ldr	r3, .L1803+52
	ldrh	r6, [r3]
	cmp	r6, #0
	bne	.L1744
	ldrh	r3, [r5, #172]
	ldr	r2, [r5, #72]
	lsl	r3, r3, #1
	ldrh	r3, [r2, r3]
	cmp	r3, #0
	beq	.L1744
.L1745:
	ldr	r3, [r5, #2552]
	cmp	r6, r3
	bcs	.L1750
	mov	r2, #0
	add	r1, sp, #40
	mov	r0, r6
	bl	log2phys
	ldr	r0, [sp, #40]
	cmn	r0, #1
	beq	.L1746
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	ldrh	r3, [r5, #172]
	cmp	r3, r0
	bne	.L1746
.L1750:
	ldr	r3, [r5, #2552]
	cmp	r6, r3
	bcc	.L1744
	ldrh	r3, [r5, #172]
	mov	r1, #0
	ldr	r2, [r5, #72]
	lsl	r3, r3, #1
	strh	r1, [r2, r3]	@ movhi
	ldrh	r0, [r5, #172]
	bl	update_vpc_list
	bl	l2p_flush
	bl	FtlVpcTblFlush
.L1744:
	mvn	r3, #0
	strh	r3, [r5, #172]	@ movhi
.L1742:
	mov	r3, #0
	str	r3, [r5, #3272]
	ldrh	r3, [r5, #224]
	cmp	r3, #2
	bhi	.L1751
	ldr	r3, .L1803+32
	ldrh	r6, [r3]
	b	.L1752
.L1746:
	add	r6, r6, #1
	b	.L1745
.L1751:
	movw	r2, #3660
	ldrh	r0, [r5, r2]
	cmp	r0, #0
	addeq	r0, r3, #1
	b	.L1676
.L1754:
	add	sp, sp, #52
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1804:
	.align	2
.L1803:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR2
	.word	.LANCHOR0+3152
	.word	.LC124
	.word	.LANCHOR0+172
	.word	.LANCHOR0+3664
	.word	.LANCHOR0+3160
	.word	.LANCHOR0+302
	.word	.LANCHOR0+304
	.word	.LANCHOR0+3196
	.word	.LANCHOR1+568
	.word	.LC8
	.word	.LANCHOR0+3662
	.word	.LANCHOR0+186
	.fnend
	.size	rk_ftl_garbage_collect, .-rk_ftl_garbage_collect
	.align	2
	.global	FtlRead
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlRead, %function
FtlRead:
	.fnstart
	@ args = 0, pretend = 0, frame = 56
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r10, r3
	ldr	r3, .L1844
	cmp	r0, #16
	.pad #60
	sub	sp, sp, #60
	mov	r5, r1
	str	r2, [sp, #4]
	ldr	r3, [r3]
	str	r3, [sp, #52]
	bne	.L1806
	mov	r2, r10
	ldr	r1, [sp, #4]
	add	r0, r5, #256
	bl	FtlVendorPartRead
	mov	r6, r0
.L1805:
	ldr	r3, .L1844
	mov	r0, r6
	ldr	r2, [sp, #52]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1827
	bl	__stack_chk_fail
.L1806:
	ldr	r4, .L1844+4
	ldr	r2, [sp, #4]
	ldr	r3, [r4, #340]
	cmp	r2, r3
	cmpls	r1, r3
	movcs	r9, #1
	movcc	r9, #0
	bcs	.L1829
	add	r2, r1, r2
	cmp	r3, r2
	str	r2, [sp, #12]
	bcc	.L1829
	ldr	r3, .L1844+8
	ldr	r6, [r3]
	cmn	r6, #1
	beq	.L1805
	movw	r3, #258
	mov	r0, r5
	ldrh	r6, [r4, r3]
	mov	r8, r9
	mov	r1, r6
	bl	__aeabi_uidiv
	ldr	r3, [sp, #12]
	mov	r1, r6
	str	r0, [sp, #16]
	mov	r6, r9
	sub	r0, r3, #1
	bl	__aeabi_uidiv
	ldr	r3, [sp, #16]
	ldr	r2, [sp, #4]
	ldr	r7, [sp, #16]
	rsb	r3, r3, #1
	str	r0, [sp, #20]
	add	r3, r3, r0
	str	r9, [sp, #28]
	str	r3, [sp, #8]
	ldr	r3, [r4, #2584]
	str	r9, [sp, #24]
	add	r3, r3, r2
	ldr	r2, [sp, #8]
	str	r3, [r4, #2584]
	ldr	r3, [r4, #2556]
	add	r3, r3, r2
	str	r3, [r4, #2556]
.L1808:
	ldr	r3, [sp, #8]
	cmp	r3, #0
	bne	.L1825
	ldr	r3, .L1844+4
	movw	r2, #3206
	ldrh	r2, [r3, r2]
	cmp	r2, #0
	bne	.L1826
	ldrh	r3, [r3, #224]
	cmp	r3, #31
	bhi	.L1805
.L1826:
	mov	r1, #1
	mov	r0, #0
	bl	rk_ftl_garbage_collect
	b	.L1805
.L1825:
	mov	r2, #0
	add	r1, sp, #48
	mov	r0, r7
	bl	log2phys
	ldr	r3, [sp, #48]
	cmn	r3, #1
	moveq	fp, #0
	beq	.L1810
	ldr	r2, [r4, #3288]
	mov	fp, #20
	mla	fp, fp, r8, r2
	str	r3, [fp, #4]
	ldr	r3, [sp, #16]
	cmp	r7, r3
	bne	.L1814
	ldr	r3, [r4, #3312]
	mov	r0, r5
	str	r3, [fp, #8]
	ldr	r3, .L1844+12
	ldrh	r3, [r3]
	mov	r1, r3
	str	r3, [sp, #28]
	bl	__aeabi_uidivmod
	ldr	r3, [sp, #28]
	str	r1, [sp, #24]
	sub	r2, r3, r1
	ldr	r1, [sp, #4]
	cmp	r1, r2
	movcc	r2, r1
	cmp	r2, r3
	str	r2, [sp, #28]
	streq	r10, [fp, #8]
.L1815:
	ldr	r3, .L1844+16
	ldr	r2, [r4, #3336]
	str	r7, [fp, #16]
	ldrh	r3, [r3]
	mul	r3, r8, r3
	add	r8, r8, #1
	bic	r3, r3, #3
	add	r3, r2, r3
	str	r3, [fp, #12]
	b	.L1813
.L1812:
	mla	r0, r0, r7, fp
	ldr	r2, [sp, #12]
	cmp	r5, r0
	movls	r3, #1
	movhi	r3, #0
	cmp	r2, r0
	movls	r3, #0
	cmp	r3, #0
	beq	.L1811
	sub	r0, r0, r5
	mov	r2, #512
	mov	r1, #0
	add	r0, r10, r0, lsl #9
	bl	ftl_memset
.L1811:
	add	fp, fp, #1
.L1810:
	ldr	r3, .L1844+12
	ldrh	r0, [r3]
	cmp	fp, r0
	bcc	.L1812
.L1813:
	ldr	r3, [sp, #8]
	add	r7, r7, #1
	subs	r3, r3, #1
	str	r3, [sp, #8]
	beq	.L1817
	ldrh	r3, [r4, #232]
	cmp	r8, r3, lsl #2
	bne	.L1808
.L1817:
	cmp	r8, #0
	beq	.L1808
	mov	r2, #0
	mov	r1, r8
	ldr	r0, [r4, #3288]
	mov	fp, #0
	bl	FlashReadPages
	lsl	r3, r9, #9
	str	r3, [sp, #40]
	ldr	r3, [sp, #24]
	lsl	r3, r3, #9
	str	r3, [sp, #32]
	ldr	r3, [sp, #28]
	lsl	r3, r3, #9
	str	r3, [sp, #36]
.L1824:
	mov	r3, #20
	ldr	r2, [r4, #3288]
	mul	r3, r3, fp
	ldr	r0, [sp, #16]
	add	r2, r2, r3
	ldr	r1, [r2, #16]
	cmp	r0, r1
	bne	.L1819
	ldr	r1, [r2, #8]
	ldr	r2, [r4, #3312]
	cmp	r1, r2
	bne	.L1820
	str	r3, [sp, #44]
	mov	r0, r10
	ldr	r3, [sp, #32]
	ldr	r2, [sp, #36]
	add	r1, r1, r3
.L1843:
	bl	ftl_memcpy
	ldr	r3, [sp, #44]
.L1820:
	ldr	r1, [r4, #3288]
	add	r2, r1, r3
	ldr	r0, [r2, #12]
	ldr	ip, [r2, #16]
	ldr	r0, [r0, #8]
	cmp	ip, r0
	ldrne	r0, [r4, #2712]
	addne	r0, r0, #1
	strne	r0, [r4, #2712]
	ldr	ip, [r1, r3]
	cmn	ip, #1
	ldreq	r0, [r4, #2712]
	moveq	r6, ip
	addeq	r0, r0, #1
	streq	r0, [r4, #2712]
	ldr	r3, [r1, r3]
	cmp	r3, #256
	bne	.L1823
	ldr	r0, [r2, #4]
	ubfx	r0, r0, #10, #16
	bl	P2V_block_in_plane
	bl	FtlGcRefreshBlock
.L1823:
	add	fp, fp, #1
	cmp	r8, fp
	bne	.L1824
	mov	r8, #0
	b	.L1808
.L1814:
	ldr	r3, [sp, #20]
	cmp	r7, r3
	bne	.L1816
	ldr	r3, [r4, #3316]
	ldr	r1, [sp, #12]
	str	r3, [fp, #8]
	ldr	r3, .L1844+12
	ldrh	r2, [r3]
	mul	r3, r2, r7
	sub	r9, r1, r3
	cmp	r2, r9
	bne	.L1815
.L1842:
	sub	r3, r3, r5
	add	r3, r10, r3, lsl #9
	str	r3, [fp, #8]
	b	.L1815
.L1816:
	ldr	r3, .L1844+12
	ldrh	r3, [r3]
	mul	r3, r7, r3
	b	.L1842
.L1819:
	ldr	r0, [sp, #20]
	cmp	r0, r1
	bne	.L1820
	ldr	r1, [r2, #8]
	ldr	r2, [r4, #3316]
	cmp	r1, r2
	bne	.L1820
	str	r3, [sp, #44]
	ldr	r3, .L1844+12
	ldr	r2, [sp, #40]
	ldrh	r0, [r3]
	ldr	r3, [sp, #20]
	mul	r0, r3, r0
	sub	r0, r0, r5
	add	r0, r10, r0, lsl #9
	b	.L1843
.L1829:
	mvn	r6, #0
	b	.L1805
.L1827:
	add	sp, sp, #60
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1845:
	.align	2
.L1844:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR2
	.word	.LANCHOR0+258
	.word	.LANCHOR0+312
	.fnend
	.size	FtlRead, .-FtlRead
	.align	2
	.global	sftl_read
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_read, %function
sftl_read:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r3, r2
	mov	r2, r1
	mov	r1, r0
	mov	r0, #0
	b	FtlRead
	.fnend
	.size	sftl_read, .-sftl_read
	.align	2
	.global	FtlWrite
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlWrite, %function
FtlWrite:
	.fnstart
	@ args = 0, pretend = 0, frame = 72
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #76
	sub	sp, sp, #76
	str	r3, [sp, #12]
	cmp	r0, #16
	mov	r10, r1
	ldr	r3, .L1902
	str	r2, [sp, #8]
	ldr	r3, [r3]
	str	r3, [sp, #68]
	bne	.L1848
	ldr	r2, [sp, #12]
	add	r0, r10, #256
	ldr	r1, [sp, #8]
	bl	FtlVendorPartWrite
.L1847:
	ldr	r3, .L1902
	ldr	r2, [sp, #68]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L1884
	bl	__stack_chk_fail
.L1848:
	ldr	r4, .L1902+4
	ldr	r2, [sp, #8]
	ldr	r3, [r4, #340]
	cmp	r2, r3
	cmpls	r1, r3
	bcs	.L1886
	add	r6, r1, r2
	cmp	r3, r6
	bcc	.L1886
	ldr	r5, .L1902+8
	ldr	r0, [r5]
	cmn	r0, #1
	beq	.L1847
	mov	r3, #2048
	mov	r0, r10
	str	r3, [r4, #3668]
	movw	r3, #258
	ldrh	r7, [r4, r3]
	mov	r1, r7
	bl	__aeabi_uidiv
	mov	r1, r7
	str	r0, [sp, #4]
	sub	r0, r6, #1
	bl	__aeabi_uidiv
	ldr	r2, [sp, #4]
	str	r0, [sp, #20]
	sub	r3, r0, r2
	ldr	r2, [sp, #8]
	add	fp, r3, #1
	str	r3, [sp, #24]
	ldr	r3, [r4, #2580]
	add	r3, r3, r2
	str	r3, [r4, #2580]
	ldr	r3, [r4, #2564]
	add	r3, r3, fp
	str	r3, [r4, #2564]
	ldr	r3, [r5, #8]
	cmp	r3, #0
	addeq	r4, r4, #24
	beq	.L1850
	ldrh	r3, [r4, #28]
	cmp	r3, #0
	addne	r4, r4, #24
	addeq	r4, r4, #76
.L1850:
	ldr	r7, [sp, #4]
	ldr	r5, .L1902+4
.L1851:
	cmp	fp, #0
	bne	.L1879
	ldr	r1, [sp, #24]
	mov	r0, fp
	bl	rk_ftl_garbage_collect
	ldrh	r3, [r5, #224]
	cmp	r3, #5
	movls	r4, #256
	ldrls	r6, .L1902+12
	bls	.L1883
.L1882:
	mov	r0, #0
	b	.L1847
.L1879:
	ldrb	r2, [r4, #6]	@ zero_extendqisi2
	ldrh	r3, [r5, #232]
	cmp	r2, r3
	bcc	.L1852
	movw	r2, #1041
	ldr	r1, .L1902+16
	ldr	r0, .L1902+20
	bl	sftl_printk
.L1852:
	ldrh	r2, [r4, #4]
	cmp	r2, #0
	bne	.L1853
	ldr	r3, .L1902+24
	ldr	r6, .L1902+8
	cmp	r4, r3
	bne	.L1854
	ldrh	r8, [r5, #80]
	cmp	r8, #0
	bne	.L1855
	add	r0, r4, #52
	bl	allocate_new_data_superblock
	str	r8, [r6, #8]
.L1855:
	ldr	r0, .L1902+24
	bl	allocate_new_data_superblock
	ldr	r4, .L1902+24
	ldr	r2, [r6, #8]
	add	r3, r4, #52
	cmp	r2, #0
	movne	r4, r3
.L1856:
	ldrh	r3, [r4, #4]
	cmp	r3, #0
	bne	.L1853
	mov	r0, r4
	bl	allocate_new_data_superblock
.L1853:
	ldrb	r2, [r4, #7]	@ zero_extendqisi2
	ldrh	r3, [r4, #4]
	lsl	r2, r2, #2
	cmp	r3, fp
	movcs	r3, fp
	cmp	r2, r3
	movcc	r3, r2
	ldrb	r2, [r4, #6]	@ zero_extendqisi2
	str	r3, [sp, #36]
	ldrh	r3, [r5, #232]
	cmp	r2, r3
	bcc	.L1857
	movw	r2, #1074
	ldr	r1, .L1902+16
	ldr	r0, .L1902+20
	bl	sftl_printk
.L1857:
	mov	r3, #0
.L1901:
	str	r3, [sp]
	ldr	r3, [sp]
	ldr	r2, [sp, #36]
	cmp	r3, r2
	bne	.L1877
.L1859:
	mov	r3, r4
	mov	r2, #0
	ldr	r1, [sp]
	ldr	r0, [r5, #3296]
	bl	FtlProgPages
	ldr	r3, [sp]
	cmp	r3, fp
	bls	.L1878
	mov	r2, #1152
	ldr	r1, .L1902+16
	ldr	r0, .L1902+20
	bl	sftl_printk
.L1878:
	ldr	r3, [sp]
	sub	fp, fp, r3
	b	.L1851
.L1854:
	str	r2, [r6, #8]
	ldrh	r2, [r5, #28]
	cmp	r2, #0
	movne	r4, r3
	bne	.L1853
	mov	r0, r4
	bl	allocate_new_data_superblock
	b	.L1856
.L1877:
	ldrh	r3, [r4, #4]
	cmp	r3, #0
	beq	.L1859
	mov	r2, #0
	add	r1, sp, #44
	mov	r0, r7
	mov	r8, #20
	bl	log2phys
	mov	r0, r4
	bl	get_new_active_ppa
	ldr	r2, .L1902+28
	ldr	r1, [sp]
	ldr	r3, [sp]
	ldrh	r2, [r2]
	mul	r8, r8, r3
	ldr	r3, [r5, #3296]
	mul	r1, r2, r1
	add	r3, r3, r8
	str	r0, [r3, #4]
	bic	r1, r1, #3
	str	r7, [r3, #16]
	str	r1, [sp, #28]
	ldr	r0, [sp, #28]
	ldr	r1, [r5, #3336]
	add	r9, r1, r0
	str	r1, [sp, #32]
	str	r9, [r3, #12]
	mov	r1, #0
	mov	r0, r9
	bl	ftl_memset
	ldr	r3, [sp, #4]
	ldr	r2, [sp, #20]
	cmp	r7, r2
	cmpne	r7, r3
	bne	.L1860
	cmp	r7, r3
	bne	.L1861
	ldr	r3, .L1902+32
	mov	r0, r10
	ldrh	r6, [r3]
	mov	r1, r6
	bl	__aeabi_uidivmod
	ldr	r3, [sp, #8]
	sub	r6, r6, r1
	str	r1, [sp, #16]
	cmp	r6, r3
	movcs	r6, r3
.L1862:
	ldr	r3, .L1902+32
	ldrh	r3, [r3]
	cmp	r6, r3
	ldr	r3, [sp, #4]
	bne	.L1863
	cmp	r7, r3
	ldr	r3, [r5, #3296]
	add	r8, r3, r8
	ldreq	r3, [sp, #12]
	beq	.L1898
	mul	r6, r6, r7
	ldr	r3, [sp, #12]
	sub	r6, r6, r10
	add	r6, r3, r6, lsl #9
	str	r6, [r8, #8]
.L1865:
	ldrb	r2, [r4, #6]	@ zero_extendqisi2
	ldrh	r3, [r5, #232]
	cmp	r2, r3
	bcc	.L1874
	movw	r2, #1143
	ldr	r1, .L1902+16
	ldr	r0, .L1902+20
	bl	sftl_printk
.L1874:
	ldr	r3, .L1902+36
	ldr	r2, [sp, #32]
	ldr	r1, [sp, #28]
	strh	r3, [r2, r1]	@ movhi
	ldr	r3, [r5, #2592]
	str	r7, [r9, #8]
	add	r7, r7, #1
	str	r3, [r9, #4]
	add	r3, r3, #1
	cmn	r3, #1
	moveq	r3, #0
	str	r3, [r5, #2592]
	ldr	r3, [sp, #44]
	str	r3, [r9, #12]
	ldrh	r3, [r4]
	strh	r3, [r9, #2]	@ movhi
	ldr	r3, [sp]
	add	r3, r3, #1
	b	.L1901
.L1861:
	ldr	r3, [sp, #8]
	add	r6, r10, r3
	ldr	r3, .L1902+32
	ldrh	r2, [r3]
	mov	r3, #0
	str	r3, [sp, #16]
	smulbb	r2, r2, r7
	sub	r6, r6, r2
	uxth	r6, r6
	b	.L1862
.L1863:
	cmp	r7, r3
	ldr	r3, [r5, #3296]
	ldreq	r2, [r5, #3312]
	ldrne	r2, [r5, #3316]
	add	r3, r3, r8
	str	r2, [r3, #8]
	ldr	r3, [sp, #44]
	cmn	r3, #1
	beq	.L1868
	str	r3, [sp, #52]
	mov	r1, #1
	ldr	r3, [r5, #3296]
	add	r0, sp, #48
	str	r7, [sp, #64]
	add	r3, r3, r8
	ldr	r2, [r3, #8]
	ldr	r3, [r3, #12]
	str	r2, [sp, #56]
	mov	r2, #0
	str	r3, [sp, #60]
	bl	FlashReadPages
	ldr	r3, [sp, #48]
	cmn	r3, #1
	bne	.L1869
	ldr	r2, [r5, #2712]
	ldr	r0, .L1902+40
	add	r2, r2, #1
	str	r2, [r5, #2712]
	mov	r2, r7
	ldr	r1, [r9, #8]
	bl	sftl_printk
.L1872:
	ldr	r3, [sp, #4]
	lsl	r2, r6, #9
	cmp	r7, r3
	bne	.L1873
	ldr	r3, [r5, #3296]
	ldr	r1, [sp, #12]
	add	r8, r3, r8
	ldr	r3, [sp, #16]
	ldr	r0, [r8, #8]
	add	r0, r0, r3, lsl #9
.L1899:
	bl	ftl_memcpy
	b	.L1865
.L1869:
	ldr	r3, [r9, #8]
	cmp	r7, r3
	beq	.L1871
	ldr	r3, [r5, #2712]
	mov	r2, r7
	ldr	r0, .L1902+44
	add	r3, r3, #1
	str	r3, [r5, #2712]
	ldr	r1, [r9, #8]
	bl	sftl_printk
.L1871:
	ldr	r3, [r9, #8]
	cmp	r7, r3
	beq	.L1872
	movw	r2, #1128
	ldr	r1, .L1902+16
	ldr	r0, .L1902+20
	bl	sftl_printk
	b	.L1872
.L1868:
	ldr	r3, [r5, #3296]
	mov	r1, #0
	ldr	r2, .L1902+48
	add	r3, r3, r8
	ldrh	r2, [r2]
	ldr	r0, [r3, #8]
	bl	ftl_memset
	b	.L1872
.L1873:
	ldr	r3, .L1902+32
	ldrh	r1, [r3]
	ldr	r3, [r5, #3296]
	mul	r1, r7, r1
	add	r8, r3, r8
	ldr	r3, [sp, #12]
	ldr	r0, [r8, #8]
	sub	r1, r1, r10
	add	r1, r3, r1, lsl #9
	b	.L1899
.L1860:
	ldr	r3, [r5, #3296]
	ldr	r2, [sp, #12]
	add	r8, r3, r8
	ldr	r3, .L1902+32
	ldrh	r3, [r3]
	mul	r3, r7, r3
	sub	r3, r3, r10
	add	r3, r2, r3, lsl #9
.L1898:
	str	r3, [r8, #8]
	b	.L1865
.L1883:
	ldrh	r3, [r5, #172]
	movw	r2, #65535
	cmp	r3, r2
	bne	.L1881
	ldrh	r2, [r5, #222]
	cmp	r2, r3
	bne	.L1881
	mov	r0, #0
	bl	List_get_gc_head_node
	uxth	r0, r0
	bl	FtlGcRefreshBlock
.L1881:
	ldr	r2, .L1902+52
	mov	r1, #1
	mov	r3, #128
	mov	r0, r1
	strh	r3, [r6]	@ movhi
	strh	r3, [r2]	@ movhi
	bl	rk_ftl_garbage_collect
	mov	r1, #1
	mov	r0, #0
	bl	rk_ftl_garbage_collect
	ldrh	r3, [r5, #224]
	cmp	r3, #2
	bhi	.L1882
	subs	r4, r4, #1
	bne	.L1883
	b	.L1882
.L1886:
	mvn	r0, #0
	b	.L1847
.L1884:
	add	sp, sp, #76
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1903:
	.align	2
.L1902:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	.LANCHOR2
	.word	.LANCHOR0+3154
	.word	.LANCHOR1+591
	.word	.LC8
	.word	.LANCHOR0+24
	.word	.LANCHOR0+312
	.word	.LANCHOR0+258
	.word	-3947
	.word	.LC125
	.word	.LC126
	.word	.LANCHOR0+310
	.word	.LANCHOR0+3152
	.fnend
	.size	FtlWrite, .-FtlWrite
	.align	2
	.global	sftl_gc
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_gc, %function
sftl_gc:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	mov	r1, #1
	mov	r0, r1
	b	rk_ftl_garbage_collect
	.fnend
	.size	sftl_gc, .-sftl_gc
	.align	2
	.global	FtlLoadSysInfo
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadSysInfo, %function
FtlLoadSysInfo:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	mov	r1, #0
	ldr	r4, .L1936
	.pad #20
	sub	sp, sp, #20
	ldr	r3, [r4, #3300]
	add	r6, r4, #2624
	ldrh	r2, [r4, #240]
	ldr	r0, [r4, #72]
	str	r3, [r4, #3460]
	ldr	r3, [r4, #3332]
	lsl	r2, r2, #1
	str	r3, [r4, #3464]
	bl	ftl_memset
	ldrh	r0, [r6]
	movw	r3, #65535
	cmp	r0, r3
	bne	.L1906
.L1917:
	mvn	r0, #0
.L1905:
	add	sp, sp, #20
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1906:
	mov	r1, #1
	ldr	r8, .L1936+4
	bl	FtlGetLastWrittenPage
	ldr	r9, .L1936+8
	sxth	r5, r0
	ldrsh	r7, [r6]
	add	r0, r0, #1
	ldr	r10, .L1936+12
	strh	r0, [r6, #2]	@ movhi
.L1908:
	cmp	r5, #0
	bge	.L1914
	movw	r2, #1474
	ldr	r1, .L1936+16
	ldr	r0, .L1936+20
	bl	sftl_printk
	b	.L1913
.L1914:
	orr	r3, r5, r7, lsl #10
	mov	r2, #1
	mov	r1, r2
	str	r3, [r4, #3456]
	mov	r0, r8
	ldr	r3, [r4, #3300]
	str	r3, [r4, #3460]
	bl	FlashReadPages
	ldr	r3, [r4, #3464]
	ldr	fp, [r3, #12]
	cmp	fp, #0
	beq	.L1909
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	beq	.L1909
	ldrh	r1, [r10]
	ldr	r0, [r4, #3460]
	bl	js_hash
	cmp	fp, r0
	beq	.L1909
	str	r0, [sp, #8]
	mov	r2, r7
	str	fp, [sp, #4]
	ldrh	r3, [r6, #4]
	ldr	r1, .L1936+16
	ldr	r0, .L1936+24
	str	r3, [sp]
	mov	r3, r5
	bl	sftl_printk
	cmp	r5, #0
	bne	.L1910
	ldrh	r3, [r6, #4]
	cmp	r7, r3
	beq	.L1910
	sxth	r7, r3
	ldr	r3, .L1936+28
	ldrh	r5, [r3]
.L1912:
	sub	r5, r5, #1
	sxth	r5, r5
	b	.L1908
.L1910:
	mvn	r3, #0
	str	r3, [r4, #3452]
.L1909:
	ldr	r3, [r4, #3452]
	cmn	r3, #1
	beq	.L1912
	ldr	r3, [r4, #3300]
	ldr	r3, [r3]
	cmp	r3, r9
	bne	.L1912
	ldr	r3, [r4, #3332]
	ldrh	r2, [r3]
	movw	r3, #61604
	cmp	r2, r3
	bne	.L1912
.L1913:
	movw	r2, #310
	ldrh	r3, [r4, #240]
	ldrh	r2, [r4, r2]
	add	r3, r3, #24
	cmp	r2, r3, lsl #1
	bcs	.L1916
	movw	r2, #1476
	ldr	r1, .L1936+16
	ldr	r0, .L1936+20
	bl	sftl_printk
.L1916:
	ldr	r5, .L1936+32
	mov	r2, #48
	ldr	r1, [r4, #3460]
	mov	r0, r5
	bl	ftl_memcpy
	ldrh	r2, [r4, #240]
	ldr	r1, [r4, #3460]
	ldr	r0, [r4, #72]
	lsl	r2, r2, #1
	add	r1, r1, #48
	bl	ftl_memcpy
	ldr	r2, [r4, #2468]
	ldr	r3, .L1936+8
	cmp	r2, r3
	bne	.L1917
	ldrb	r2, [r4, #2478]	@ zero_extendqisi2
	ldrh	r3, [r4, #254]
	ldrh	r7, [r5, #8]
	cmp	r2, r3
	strh	r7, [r6, #6]	@ movhi
	bne	.L1917
	movw	r3, #302
	movw	r2, #258
	ldrh	r3, [r4, r3]
	ldrh	r2, [r4, r2]
	ldr	r6, [r4, #244]
	str	r7, [r4, #3672]
	mul	r3, r7, r3
	ldrh	r1, [r4, #232]
	str	r3, [r4, #2552]
	mul	r3, r3, r2
	str	r3, [r4, #340]
	ldr	r3, .L1936+36
	ldrh	r0, [r3, #6]
	sub	r0, r6, r0
	sub	r0, r0, r7
	bl	__aeabi_uidiv
	cmp	r7, r6
	movw	r3, #2620
	strh	r0, [r4, r3]	@ movhi
	bls	.L1918
	movw	r2, #1498
	ldr	r1, .L1936+16
	ldr	r0, .L1936+20
	bl	sftl_printk
.L1918:
	ldrh	r3, [r5, #16]
	ldrh	r1, [r5, #14]
	lsr	r2, r3, #6
	and	r3, r3, #63
	strb	r3, [r4, #30]
	strh	r2, [r4, #26]	@ movhi
	ldrh	r2, [r5, #18]
	ldrb	r3, [r4, #2479]	@ zero_extendqisi2
	strh	r1, [r4, #24]	@ movhi
	strh	r2, [r4, #76]	@ movhi
	ldrh	r2, [r5, #20]
	strb	r3, [r4, #32]
	mvn	r3, #0
	strh	r3, [r4, #172]	@ movhi
	mov	r3, #0
	strh	r3, [r4, #174]	@ movhi
	lsr	r0, r2, #6
	and	r2, r2, #63
	strb	r2, [r4, #82]
	ldrb	r2, [r4, #2480]	@ zero_extendqisi2
	strh	r0, [r4, #78]	@ movhi
	strb	r3, [r4, #178]
	strb	r2, [r4, #84]
	ldrh	r2, [r5, #22]
	strb	r3, [r4, #180]
	str	r3, [r4, #2576]
	strh	r2, [r4, #124]	@ movhi
	ldrh	r2, [r5, #24]
	str	r3, [r4, #2564]
	str	r3, [r4, #2556]
	str	r3, [r4, #2572]
	lsr	r0, r2, #6
	and	r2, r2, #63
	strb	r2, [r4, #130]
	ldrb	r2, [r4, #2481]	@ zero_extendqisi2
	str	r3, [r4, #2600]
	strh	r0, [r4, #126]	@ movhi
	strb	r2, [r4, #132]
	ldr	r2, [r4, #2500]
	str	r2, [r4, #2596]
	str	r3, [r4, #2612]
	ldr	r2, [r4, #2588]
	str	r3, [r4, #2568]
	ldr	r3, [r4, #2508]
	cmp	r3, r2
	ldr	r2, [r4, #2592]
	strhi	r3, [r4, #2588]
	ldr	r3, [r4, #2504]
	cmp	r3, r2
	strhi	r3, [r4, #2592]
	movw	r3, #65535
	cmp	r1, r3
	beq	.L1921
	ldr	r0, .L1936+40
	bl	make_superblock
.L1921:
	ldrh	r2, [r4, #76]
	movw	r3, #65535
	cmp	r2, r3
	beq	.L1922
	ldr	r0, .L1936+44
	bl	make_superblock
.L1922:
	ldrh	r2, [r4, #124]
	movw	r3, #65535
	cmp	r2, r3
	beq	.L1923
	ldr	r0, .L1936+48
	bl	make_superblock
.L1923:
	ldrh	r2, [r4, #172]
	movw	r3, #65535
	cmp	r2, r3
	beq	.L1924
	ldr	r0, .L1936+52
	bl	make_superblock
.L1924:
	mov	r0, #0
	b	.L1905
.L1937:
	.align	2
.L1936:
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.word	1179929683
	.word	.LANCHOR0+310
	.word	.LANCHOR1+600
	.word	.LC8
	.word	.LC127
	.word	.LANCHOR0+304
	.word	.LANCHOR0+2468
	.word	.LANCHOR0+352
	.word	.LANCHOR0+24
	.word	.LANCHOR0+76
	.word	.LANCHOR0+124
	.word	.LANCHOR0+172
	.fnend
	.size	FtlLoadSysInfo, .-FtlLoadSysInfo
	.align	2
	.global	FtlMapTblRecovery
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlMapTblRecovery, %function
FtlMapTblRecovery:
	.fnstart
	@ args = 0, pretend = 0, frame = 32
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #44
	sub	sp, sp, #44
	ldr	r3, [r0, #24]
	mov	r4, r0
	mov	r1, #0
	mov	r8, #0
	ldr	r6, .L1980
	str	r3, [sp, #12]
	ldr	r3, [r0, #16]
	ldr	r9, [r0, #12]
	add	fp, r6, #304
	str	r3, [sp, #24]
	ldrh	r3, [r0, #6]
	str	r3, [sp, #16]
	ldrh	r3, [r0, #8]
	ldr	r0, [sp, #12]
	str	r3, [sp, #20]
	ldr	r3, [sp, #16]
	lsl	r2, r3, #2
	bl	ftl_memset
	ldr	r3, [r6, #3300]
	ldr	r5, [r6, #3332]
	str	r8, [r4, #32]
	str	r3, [r6, #3460]
	mvn	r3, #0
	str	r5, [r6, #3464]
	strh	r3, [r4]	@ movhi
	strh	r3, [r4, #2]	@ movhi
	mov	r3, #1
	str	r8, [r4, #28]
	str	r3, [r4, #36]
.L1939:
	ldr	r3, [sp, #20]
	sxth	r7, r8
	cmp	r7, r3
	bge	.L1958
	ldr	r3, [sp, #20]
	sub	r3, r3, #1
	cmp	r7, r3
	lsl	r3, r7, #1
	bne	.L1940
	ldrh	r0, [r9, r3]
	mov	r1, #1
	add	r10, r9, r3
	ldr	r6, .L1980
	bl	FtlGetLastWrittenPage
	sxth	r3, r0
	ldr	fp, .L1980+4
	add	r0, r0, #1
	strh	r8, [r4]	@ movhi
	str	r3, [sp, #20]
	mov	r8, #0
	ldr	r3, [sp, #24]
	strh	r0, [r4, #2]	@ movhi
	ldr	r3, [r3, r7, lsl #2]
	str	r3, [r4, #28]
.L1941:
	ldr	r3, [sp, #20]
	sxth	r9, r8
	add	r2, r3, #1
	cmp	r9, r2
	blt	.L1944
.L1958:
	mov	r0, r4
	bl	ftl_free_no_use_map_blk
	ldr	r3, .L1980+8
	ldrh	r2, [r4, #2]
	ldrh	r3, [r3]
	cmp	r2, r3
	bne	.L1946
	mov	r0, r4
	bl	ftl_map_blk_alloc_new_blk
.L1946:
	mov	r0, r4
	bl	ftl_map_blk_gc
	mov	r0, r4
	bl	ftl_map_blk_gc
	mov	r0, #0
	add	sp, sp, #44
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L1944:
	ldrh	r2, [r10]
	mov	r0, fp
	orr	r2, r9, r2, lsl #10
	str	r2, [r6, #3456]
	mov	r2, #1
	mov	r1, r2
	bl	FlashReadPages
	ldr	r2, [r6, #3464]
	ldr	r2, [r2, #12]
	cmp	r2, #0
	str	r2, [sp, #24]
	beq	.L1942
	ldr	r1, [r6, #3452]
	cmn	r1, #1
	beq	.L1942
	ldr	r1, .L1980+12
	ldr	r0, [r6, #3460]
	ldrh	r1, [r1]
	bl	js_hash
	ldr	r2, [sp, #24]
	cmp	r2, r0
	beq	.L1942
	str	r0, [sp, #4]
	mov	r3, r9
	str	r2, [sp]
	mov	r2, r7
	ldr	r1, .L1980+16
	ldr	r0, .L1980+20
	bl	sftl_printk
	mvn	r3, #0
	str	r3, [r6, #3452]
.L1942:
	ldr	r3, [r6, #3452]
	cmn	r3, #1
	beq	.L1943
	ldrh	r3, [r5, #8]
	ldr	r2, [sp, #16]
	cmp	r2, r3
	bls	.L1943
	ldrh	r1, [r5]
	ldrh	r2, [r4, #4]
	cmp	r1, r2
	ldreq	r2, [r6, #3456]
	ldreq	r1, [sp, #12]
	streq	r2, [r1, r3, lsl #2]
.L1943:
	add	r8, r8, #1
	b	.L1941
.L1940:
	ldr	r2, [r6, #3300]
	ldr	r0, .L1980+4
	str	r2, [r6, #3460]
	add	r2, r9, r3
	str	r2, [sp, #28]
	ldrh	r2, [r9, r3]
	ldrh	r3, [fp]
	sub	r3, r3, #1
	orr	r3, r3, r2, lsl #10
	mov	r2, #1
	mov	r1, r2
	str	r3, [r6, #3456]
	bl	FlashReadPages
	ldr	r3, [r6, #3452]
	cmn	r3, #1
	beq	.L1960
	ldrh	r2, [r5]
	ldrh	r3, [r4, #4]
	cmp	r2, r3
	bne	.L1960
	ldrh	r2, [r5, #8]
	movw	r3, #64245
	cmp	r2, r3
	beq	.L1948
.L1960:
	mov	r10, #0
.L1949:
	ldrh	r2, [fp]
	sxth	r3, r10
	cmp	r3, r2
	bge	.L1956
	ldr	r2, [sp, #28]
	ldr	r0, .L1980+4
	str	r3, [sp, #36]
	ldrh	r2, [r2]
	orr	r2, r3, r2, lsl #10
	str	r2, [r6, #3456]
	mov	r2, #1
	mov	r1, r2
	bl	FlashReadPages
	ldr	r2, [r6, #3464]
	ldr	r2, [r2, #12]
	cmp	r2, #0
	str	r2, [sp, #32]
	beq	.L1953
	ldr	r1, [r6, #3452]
	cmn	r1, #1
	beq	.L1953
	ldr	r1, .L1980+12
	ldr	r0, [r6, #3460]
	ldrh	r1, [r1]
	bl	js_hash
	ldr	r2, [sp, #32]
	cmp	r2, r0
	beq	.L1953
	str	r0, [sp, #4]
	str	r2, [sp]
	mov	r2, r7
	ldr	r3, [sp, #36]
	ldr	r1, .L1980+16
	ldr	r0, .L1980+24
	bl	sftl_printk
	mvn	r3, #0
	str	r3, [r6, #3452]
.L1953:
	ldr	r3, [r6, #3452]
	cmn	r3, #1
	beq	.L1954
	ldrh	r3, [r5, #8]
	ldr	r2, [sp, #16]
	cmp	r2, r3
	bls	.L1954
	ldrh	r1, [r5]
	ldrh	r2, [r4, #4]
	cmp	r1, r2
	ldreq	r2, [r6, #3456]
	ldreq	r1, [sp, #12]
	streq	r2, [r1, r3, lsl #2]
.L1954:
	add	r10, r10, #1
	b	.L1949
.L1948:
	mov	r1, #0
	mov	r0, #4
.L1950:
	ldrh	r2, [fp]
	sxth	r3, r1
	sub	r2, r2, #1
	cmp	r3, r2
	blt	.L1952
.L1956:
	add	r8, r8, #1
	b	.L1939
.L1952:
	ldr	ip, [r6, #3300]
	add	r1, r1, #1
	ldr	r7, [sp, #16]
	ldr	r2, [ip, r3, lsl #3]
	uxth	lr, r2
	cmp	r7, lr
	addhi	r3, r0, r3, lsl #3
	movhi	r2, lr
	ldrhi	r3, [ip, r3]
	ldrhi	ip, [sp, #12]
	strhi	r3, [ip, r2, lsl #2]
	b	.L1950
.L1981:
	.align	2
.L1980:
	.word	.LANCHOR0
	.word	.LANCHOR0+3452
	.word	.LANCHOR0+304
	.word	.LANCHOR0+310
	.word	.LANCHOR1+615
	.word	.LC128
	.word	.LC129
	.fnend
	.size	FtlMapTblRecovery, .-FtlMapTblRecovery
	.align	2
	.global	FtlLoadVonderInfo
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadVonderInfo, %function
FtlLoadVonderInfo:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L1984
	push	{r4, lr}
	.save {r4, lr}
	add	r2, r3, #320
	add	r0, r3, #3600
	ldrh	r2, [r2]
	add	r0, r0, #8
	strh	r2, [r0, #10]	@ movhi
	ldr	r2, .L1984+4
	strh	r2, [r0, #4]	@ movhi
	add	r2, r3, #344
	ldrh	r2, [r2]
	strh	r2, [r0, #8]	@ movhi
	movw	r2, #322
	ldrh	r2, [r3, r2]
	strh	r2, [r0, #6]	@ movhi
	ldr	r2, [r3, #348]
	str	r2, [r3, #3620]
	ldr	r2, [r3, #3368]
	str	r2, [r3, #3624]
	ldr	r2, [r3, #3364]
	str	r2, [r3, #3628]
	ldr	r2, [r3, #3372]
	str	r2, [r3, #3632]
	bl	FtlMapTblRecovery
	mov	r0, #0
	pop	{r4, pc}
.L1985:
	.align	2
.L1984:
	.word	.LANCHOR0
	.word	-3962
	.fnend
	.size	FtlLoadVonderInfo, .-FtlLoadVonderInfo
	.align	2
	.global	FtlLoadMapInfo
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlLoadMapInfo, %function
FtlLoadMapInfo:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, lr}
	.save {r4, lr}
	bl	FtlL2PDataInit
	ldr	r0, .L1988
	bl	FtlMapTblRecovery
	mov	r0, #0
	pop	{r4, pc}
.L1989:
	.align	2
.L1988:
	.word	.LANCHOR0+3392
	.fnend
	.size	FtlLoadMapInfo, .-FtlLoadMapInfo
	.align	2
	.global	FtlSysBlkInit
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlSysBlkInit, %function
FtlSysBlkInit:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r3, #0
	ldr	r4, .L2007
	add	r7, r4, #3472
	ldrh	r0, [r4, #236]
	strh	r3, [r7]	@ movhi
	bl	FtlFreeSysBlkQueueInit
	bl	FtlScanSysBlk
	add	r3, r4, #2624
	ldrh	r2, [r3]
	movw	r3, #65535
	cmp	r2, r3
	bne	.L1991
.L1993:
	mvn	r6, #0
.L1990:
	mov	r0, r6
	pop	{r4, r5, r6, r7, r8, pc}
.L1991:
	bl	FtlLoadSysInfo
	subs	r6, r0, #0
	bne	.L1993
	bl	FtlLoadMapInfo
	bl	FtlLoadVonderInfo
	bl	Ftl_load_ext_data
	bl	FtlLoadEctTbl
	bl	FtlFreeSysBLkSort
	bl	SupperBlkListInit
	bl	FtlPowerLostRecovery
	mov	r0, #1
	bl	FtlUpdateVaildLpn
	ldr	r2, [r4, #2540]
	movw	r3, #338
	ldrh	r1, [r4, r3]
	mov	r0, #12
	mov	r3, r6
.L1994:
	cmp	r3, r1
	bge	.L1999
	mla	ip, r0, r3, r2
	ldr	ip, [ip, #4]
	cmp	ip, #0
	bge	.L1995
.L1999:
	ldr	r5, .L2007+4
	cmp	r3, r1
	ldrh	r2, [r5, #28]
	add	r2, r2, #1
	strh	r2, [r5, #28]	@ movhi
	bge	.L2005
.L1996:
	ldr	r0, .L2007+8
	bl	FtlSuperblockPowerLostFix
	ldr	r0, .L2007+12
	bl	FtlSuperblockPowerLostFix
	ldrh	r3, [r4, #24]
	ldr	r1, [r4, #72]
	ldrh	r0, [r4, #28]
	lsl	r3, r3, #1
	ldrh	r2, [r1, r3]
	sub	r2, r2, r0
	movw	r0, #302
	strh	r2, [r1, r3]	@ movhi
	ldrh	r2, [r4, #76]
	ldrh	r3, [r4, r0]
	ldr	ip, [r4, #72]
	ldrh	lr, [r4, #80]
	lsl	r2, r2, #1
	strh	r3, [r4, #26]	@ movhi
	mov	r3, #0
	strb	r3, [r4, #30]
	strh	r3, [r4, #28]	@ movhi
	ldrh	r1, [ip, r2]
	sub	r1, r1, lr
	strh	r1, [ip, r2]	@ movhi
	strb	r3, [r4, #82]
	strh	r3, [r4, #80]	@ movhi
	ldrh	r3, [r5, #30]
	ldrh	r2, [r4, r0]
	add	r3, r3, #1
	strh	r2, [r4, #78]	@ movhi
	strh	r3, [r5, #30]	@ movhi
	bl	l2p_flush
	bl	FtlVpcTblFlush
	bl	FtlVpcTblFlush
	b	.L2000
.L1995:
	add	r3, r3, #1
	b	.L1994
.L2005:
	ldrh	r3, [r7]
	cmp	r3, #0
	bne	.L1996
.L2000:
	ldrh	r0, [r4, #24]
	movw	r3, #65535
	cmp	r0, r3
	beq	.L2001
	ldrh	r3, [r4, #28]
	cmp	r3, #0
	bne	.L2001
	ldrh	r3, [r4, #80]
	cmp	r3, #0
	bne	.L2001
	bl	FtlGcRefreshBlock
	ldrh	r0, [r4, #76]
	bl	FtlGcRefreshBlock
	bl	FtlVpcTblFlush
	ldr	r0, .L2007+8
	bl	allocate_new_data_superblock
	ldr	r0, .L2007+12
	bl	allocate_new_data_superblock
.L2001:
	bl	FtlVpcCheckAndModify
	b	.L1990
.L2008:
	.align	2
.L2007:
	.word	.LANCHOR0
	.word	.LANCHOR0+2468
	.word	.LANCHOR0+24
	.word	.LANCHOR0+76
	.fnend
	.size	FtlSysBlkInit, .-FtlSysBlkInit
	.align	2
	.global	ftl_low_format
	.syntax unified
	.arm
	.fpu softvfp
	.type	ftl_low_format, %function
ftl_low_format:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r3, #0
	ldr	r4, .L2034
	ldrh	r0, [r4, #236]
	str	r3, [r4, #2588]
	str	r3, [r4, #2592]
	str	r3, [r4, #2604]
	bl	FtlFreeSysBlkQueueInit
	bl	FtlLoadBbt
	cmp	r0, #0
	beq	.L2010
	bl	FtlMakeBbt
.L2010:
	ldr	r0, .L2034+4
	mov	r2, #0
	ldr	ip, .L2034+8
.L2011:
	ldrh	r1, [r0]
	uxth	r3, r2
	add	r2, r2, #1
	cmp	r3, r1, lsl #7
	blt	.L2012
	ldrh	r6, [r4, #240]
	mov	r5, #0
.L2013:
	ldrh	r3, [r4, #242]
	cmp	r3, r6
	bhi	.L2014
	ldrh	r1, [r4, #232]
	sub	r3, r5, #3
	cmp	r3, r1, lsl #1
	bge	.L2015
.L2019:
	mov	r5, #0
	mov	r6, r5
.L2016:
	ldrh	r3, [r4, #240]
	uxth	r0, r5
	add	r5, r5, #1
	cmp	r3, r0
	bhi	.L2020
	ldrh	r3, [r4, #242]
	movw	r9, #2620
	ldrh	r5, [r4, #232]
	ldr	r10, [r4, #244]
	str	r3, [r4, #3284]
	mov	r1, r5
	mov	r0, r10
	bl	__aeabi_uidiv
	ubfx	r7, r0, #5, #16
	mov	r8, r0
	add	r3, r7, #36
	str	r0, [r4, #2552]
	strh	r3, [r4, r9]	@ movhi
	mov	r3, #24
	mul	r3, r3, r5
	cmp	r6, r3
	ble	.L2021
	mov	r1, r5
	sub	r0, r10, r6
	bl	__aeabi_uidiv
	str	r0, [r4, #2552]
	lsr	r0, r0, #5
	add	r0, r0, #24
	strh	r0, [r4, r9]	@ movhi
.L2021:
	movw	r3, #294
	ldrh	r3, [r4, r3]
	cmp	r3, #0
	beq	.L2023
	movw	r2, #2620
	ldrh	r1, [r4, r2]
	add	r1, r1, r3, lsr #1
	strh	r1, [r4, r2]	@ movhi
	mul	r1, r5, r3
	cmp	r6, r1
	addlt	r3, r3, #32
	strlt	r8, [r4, #2552]
	addlt	r3, r7, r3
	strhlt	r3, [r4, r2]	@ movhi
.L2023:
	movw	r3, #2620
	ldr	r6, .L2034+12
	ldrh	r2, [r4, r3]
	ldr	r3, [r4, #2552]
	sub	r3, r3, r2
	mul	r5, r5, r3
	movw	r3, #302
	ldrh	r3, [r4, r3]
	str	r5, [r4, #3672]
	mul	r5, r5, r3
	movw	r3, #258
	ldrh	r3, [r4, r3]
	str	r5, [r4, #2552]
	mul	r5, r5, r3
	str	r5, [r4, #340]
	bl	FtlBbmTblFlush
	ldrh	r2, [r4, #242]
	mov	r1, #0
	ldr	r0, [r4, #72]
	lsl	r2, r2, #1
	bl	ftl_memset
	mvn	r2, #0
	mov	r3, #0
	mov	r5, r2
	str	r3, [r4, #2548]
	strh	r3, [r4, #174]	@ movhi
	strb	r3, [r4, #178]
	strb	r3, [r4, #180]
	strh	r3, [r4, #26]	@ movhi
	strb	r3, [r4, #30]
	strh	r3, [r4, #24]	@ movhi
	mov	r3, #1
	strh	r2, [r4, #172]	@ movhi
	strb	r3, [r4, #32]
.L2025:
	mov	r0, r6
	bl	make_superblock
	ldrb	r3, [r4, #31]	@ zero_extendqisi2
	cmp	r3, #0
	ldrh	r3, [r4, #24]
	bne	.L2026
	ldr	r2, [r4, #72]
	lsl	r3, r3, #1
	strh	r5, [r2, r3]	@ movhi
	ldrh	r3, [r4, #24]
	add	r3, r3, #1
	strh	r3, [r4, #24]	@ movhi
	b	.L2025
.L2012:
	ldr	lr, [r4, #3312]
	mvn	r1, r3
	orr	r1, r3, r1, lsl #16
	str	r1, [lr, r3, lsl #2]
	ldr	r1, [r4, #3316]
	str	ip, [r1, r3, lsl #2]
	b	.L2011
.L2014:
	mov	r0, r6
	mov	r1, #1
	bl	FtlLowFormatEraseBlock
	add	r6, r6, #1
	add	r5, r5, r0
	uxth	r5, r5
	uxth	r6, r6
	b	.L2013
.L2015:
	mov	r0, r5
	bl	__aeabi_uidiv
	ldr	r3, [r4, #332]
	add	r0, r0, r3
	uxth	r0, r0
	bl	FtlSysBlkNumInit
	ldrh	r0, [r4, #236]
	bl	FtlFreeSysBlkQueueInit
	ldrh	r5, [r4, #240]
.L2017:
	ldrh	r3, [r4, #242]
	cmp	r3, r5
	bls	.L2019
	mov	r0, r5
	mov	r1, #1
	add	r5, r5, #1
	bl	FtlLowFormatEraseBlock
	uxth	r5, r5
	b	.L2017
.L2020:
	mov	r1, #0
	bl	FtlLowFormatEraseBlock
	add	r6, r6, r0
	uxth	r6, r6
	b	.L2016
.L2026:
	ldr	r2, [r4, #2588]
	lsl	r3, r3, #1
	ldrh	r1, [r4, #28]
	mvn	r5, #0
	ldr	r6, .L2034+16
	str	r2, [r4, #36]
	add	r2, r2, #1
	str	r2, [r4, #2588]
	ldr	r2, [r4, #72]
	strh	r1, [r2, r3]	@ movhi
	mov	r3, #0
	strh	r3, [r4, #78]	@ movhi
	strb	r3, [r4, #82]
	ldrh	r3, [r4, #24]
	add	r3, r3, #1
	strh	r3, [r4, #76]	@ movhi
	mov	r3, #1
	strb	r3, [r4, #84]
.L2027:
	mov	r0, r6
	bl	make_superblock
	ldrb	r3, [r4, #83]	@ zero_extendqisi2
	cmp	r3, #0
	ldrh	r3, [r4, #76]
	bne	.L2028
	ldr	r2, [r4, #72]
	lsl	r3, r3, #1
	strh	r5, [r2, r3]	@ movhi
	ldrh	r3, [r4, #76]
	add	r3, r3, #1
	strh	r3, [r4, #76]	@ movhi
	b	.L2027
.L2028:
	ldr	r2, [r4, #2588]
	lsl	r3, r3, #1
	ldrh	r1, [r4, #80]
	mvn	r5, #0
	str	r2, [r4, #88]
	add	r2, r2, #1
	str	r2, [r4, #2588]
	ldr	r2, [r4, #72]
	strh	r1, [r2, r3]	@ movhi
	strh	r5, [r4, #124]	@ movhi
	bl	FtlFreeSysBlkQueueOut
	ldr	r3, .L2034+20
	mov	r2, #0
	strh	r2, [r3, #2]	@ movhi
	ldr	r2, [r4, #3672]
	strh	r5, [r3, #4]	@ movhi
	strh	r0, [r3]	@ movhi
	strh	r2, [r3, #6]	@ movhi
	ldr	r3, [r4, #2588]
	str	r3, [r4, #2632]
	add	r3, r3, #1
	str	r3, [r4, #2588]
	bl	FtlVpcTblFlush
	bl	FtlSysBlkInit
	cmp	r0, #0
	mov	r0, #0
	ldreq	r3, .L2034+24
	moveq	r2, #1
	streq	r2, [r3]
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L2035:
	.align	2
.L2034:
	.word	.LANCHOR0
	.word	.LANCHOR0+258
	.word	168778952
	.word	.LANCHOR0+24
	.word	.LANCHOR0+76
	.word	.LANCHOR0+2624
	.word	.LANCHOR2
	.fnend
	.size	ftl_low_format, .-ftl_low_format
	.align	2
	.global	sftl_init
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_init, %function
sftl_init:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mvn	r3, #0
	ldr	r4, .L2043
	ldr	r5, .L2043+4
	ldr	r1, .L2043+8
	ldr	r0, .L2043+12
	str	r3, [r5]
	bl	sftl_printk
	mov	r0, r4
	bl	FtlConstantsInit
	bl	FtlMemInit
	bl	FtlVariablesInit
	ldrh	r0, [r4, #236]
	bl	FtlFreeSysBlkQueueInit
	bl	FtlLoadBbt
	cmp	r0, #0
	bne	.L2040
	bl	FtlSysBlkInit
	cmp	r0, #0
	bne	.L2040
	mov	r3, #1
	str	r3, [r5]
	ldrh	r3, [r4, #224]
	cmp	r3, #15
	bhi	.L2040
	movw	r4, #8129
.L2039:
	mov	r1, #1
	mov	r0, #0
	bl	rk_ftl_garbage_collect
	subs	r4, r4, #1
	bne	.L2039
.L2040:
	mov	r0, #0
	pop	{r4, r5, r6, pc}
.L2044:
	.align	2
.L2043:
	.word	.LANCHOR0
	.word	.LANCHOR2
	.word	.LC0
	.word	.LC77
	.fnend
	.size	sftl_init, .-sftl_init
	.align	2
	.global	FtlWriteToIDB
	.syntax unified
	.arm
	.fpu softvfp
	.type	FtlWriteToIDB, %function
FtlWriteToIDB:
	.fnstart
	@ args = 0, pretend = 0, frame = 104
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	add	r8, r1, r0
	ldr	r3, .L2106
	sub	r10, r8, #1
	cmp	r10, #63
	.pad #116
	sub	sp, sp, #116
	movhi	r9, #0
	movls	r9, #1
	ldr	r3, [r3]
	cmp	r0, #576
	orrcs	r9, r9, #1
	cmp	r9, #0
	ldr	r5, .L2106+4
	str	r3, [sp, #108]
	beq	.L2046
	ldr	r3, [r5, #3676]
	cmp	r3, #0
	bne	.L2047
.L2105:
	mov	r4, #0
	b	.L2045
.L2047:
	ldr	r7, [r5, #3680]
	ldr	r3, .L2106+8
	ldr	r2, [r7]
	cmp	r2, r3
	bne	.L2049
	ldrh	r1, [r5, #10]
	add	r0, r7, #260096
	mov	r2, #0
	movw	r3, #65023
.L2053:
	ldr	ip, [r0, #-4]!
	cmp	ip, #0
	bne	.L2050
	ldr	ip, [r7, r2, lsl #2]
	add	r2, r2, #1
	cmp	r2, #4096
	sub	r3, r3, #1
	movhi	r2, #0
	cmp	r3, #4096
	str	ip, [r0, #2048]
	bne	.L2053
	mov	fp, #512
	b	.L2052
.L2050:
	add	r3, r3, #127
	lsr	fp, r3, #7
.L2052:
	lsl	r1, r1, #2
	add	r0, fp, #4
	ldr	r4, .L2106+4
	uxth	r1, r1
	bl	__aeabi_uidiv
	add	r3, r0, #1
	mov	r1, fp
	mov	r2, r3
	ldr	r0, .L2106+12
	str	r3, [sp, #16]
	bl	sftl_printk
	lsl	r3, fp, #7
	str	r3, [sp, #28]
	mov	r3, #0
	str	r3, [sp, #20]
	str	r3, [sp, #12]
.L2054:
	ldr	r3, [sp, #16]
	ldr	r2, [sp, #12]
	add	r3, r3, r2
	cmp	r3, #8
	str	r3, [sp, #24]
	bls	.L2076
	ldr	r3, [sp, #20]
	cmp	r3, #0
	bne	.L2077
.L2049:
	mvn	fp, #0
.L2077:
	ldr	r5, .L2106+4
	mov	r3, #0
	mov	r4, fp
	ldr	r0, [r5, #3680]
	str	r3, [r5, #3676]
	bl	kfree
	ldr	r0, [r5, #3684]
	bl	kfree
.L2045:
	ldr	r3, .L2106
	mov	r0, r4
	ldr	r2, [sp, #108]
	ldr	r3, [r3]
	cmp	r2, r3
	beq	.L2084
	bl	__stack_chk_fail
.L2076:
	mov	r2, #512
	mov	r1, #0
	ldr	r0, [r4, #3684]
	bl	memset
	ldrh	r6, [r4, #10]
	ldr	r3, [sp, #12]
	mul	r9, r6, r3
	ldr	r3, [r4, #3260]
	cmp	r3, #0
	moveq	r8, #6
	beq	.L2055
	ldr	r3, [r4, #3264]
	cmp	r3, #0
	moveq	r8, #6
	movne	r8, #9
.L2055:
	mov	r10, r9
	mov	r5, #0
.L2056:
	ldr	r3, [r4, #3248]
	mov	r1, r10
	mov	r0, #0
	add	r5, r5, #1
	blx	r3
	ldr	r3, [sp, #16]
	add	r10, r10, r6
	cmp	r3, r5
	bhi	.L2056
	cmp	r8, #9
	movne	r5, #0
	bne	.L2057
	ldr	r5, [r4, #3684]
	mov	r2, #1024
	mov	r1, #0
	mov	r0, r5
	bl	ftl_memset
	ldr	r3, .L2106+16
	mov	r1, #12
	mov	r2, #4
	str	r1, [r5, #4]
	strb	r2, [r5, #17]
	add	r0, r5, r1
	str	r3, [r5]
	mov	r3, #0
	ldrh	r2, [r4, #10]
	str	r3, [r5, #12]
	strb	r3, [r5, #16]
	strh	r2, [r5, #18]	@ movhi
	mov	r2, #16
	strb	r3, [r5, #20]
	strb	r2, [r5, #21]
	strh	r3, [r5, #22]	@ movhi
	bl	js_hash
	str	r0, [r5, #8]
.L2057:
	ldr	r3, [sp, #16]
	mov	r10, r7
	mul	r3, r6, r3
	mov	r6, #0
	str	r3, [sp, #32]
.L2058:
	ldr	r3, [sp, #32]
	cmp	r6, r3
	beq	.L2065
	cmp	r8, #9
	lslne	r3, r6, #2
	addeq	r3, r6, #1
	cmp	r6, #0
	cmpeq	r8, #9
	str	r3, [sp, #44]
	movw	r3, #61424
	str	r3, [sp, #48]
	moveq	r0, #1
	movne	r0, #0
	bne	.L2061
	ldr	r3, [r4, #3260]
	mov	r0, #70
	blx	r3
	mov	r2, r5
	add	r3, sp, #44
	mov	r1, r9
	ldr	ip, [r4, #3252]
	mov	r0, #0
	blx	ip
	ldr	r3, [r4, #3260]
	str	r0, [sp, #36]
	ldrb	r0, [r4, #22]	@ zero_extendqisi2
	blx	r3
	ldr	r2, [sp, #36]
	cmn	r2, #1
	bne	.L2062
.L2065:
	ldrb	r3, [r4, #14]	@ zero_extendqisi2
	ldr	r2, [sp, #12]
	ldr	r6, [r4, #3684]
	str	r3, [sp, #32]
	ldrh	r3, [r4, #10]
	mul	r9, r3, r2
	ldr	r2, [r4, #3260]
	cmp	r2, #0
	moveq	r8, #6
	beq	.L2064
	ldr	r2, [r4, #3264]
	cmp	r2, #0
	moveq	r8, #6
	movne	r8, #9
.L2064:
	ldr	r2, [sp, #16]
	mov	r5, #0
	mul	r10, r3, r2
.L2067:
	cmp	r5, r10
	beq	.L2071
	cmp	r5, #0
	cmpeq	r8, #9
	moveq	r0, #1
	movne	r0, #0
	bne	.L2068
	ldr	r3, [r4, #3260]
	mov	r0, #70
	blx	r3
	ldr	r3, [r4, #3264]
	mov	r0, #2
	blx	r3
	mov	r2, r6
	mov	r1, r9
	ldr	ip, [r4, #3256]
	add	r3, sp, #44
	mov	r0, #0
	blx	ip
	ldr	r3, [r4, #3264]
	ldr	r0, [sp, #32]
	blx	r3
	ldr	r3, [r4, #3260]
	ldrb	r0, [r4, #22]	@ zero_extendqisi2
	blx	r3
	ldr	r3, [r6]
	ldr	r2, .L2106+16
	cmp	r3, r2
	beq	.L2069
.L2071:
	ldr	r0, [r4, #3684]
	mov	r3, r7
	mov	r5, #0
	mov	r2, r0
.L2070:
	mov	r6, r2
	mov	r8, r3
	ldr	ip, [r6]
	add	r2, r2, #4
	ldr	r1, [r8]
	add	r3, r3, #4
	cmp	ip, r1
	beq	.L2073
	mov	r2, #512
	mov	r1, #0
	bl	memset
	ldr	r0, .L2106+20
	str	r5, [sp]
	ldr	r1, [sp, #12]
	ldr	r3, [r8]
	ldr	r2, [r6]
	bl	sftl_printk
	ldrh	r1, [r4, #10]
	mov	r0, #0
	ldr	r2, [sp, #12]
	ldr	r3, [r4, #3248]
	mul	r1, r2, r1
	blx	r3
.L2074:
	ldr	r3, [sp, #24]
	str	r3, [sp, #12]
	b	.L2054
.L2061:
	add	r3, sp, #44
	mov	r2, r10
	add	r1, r9, r6
	ldr	ip, [r4, #3252]
	blx	ip
	cmn	r0, #1
	beq	.L2065
	add	r10, r10, #2048
.L2062:
	add	r6, r6, #1
	b	.L2058
.L2068:
	add	r3, sp, #44
	mov	r2, r6
	add	r1, r9, r5
	ldr	ip, [r4, #3256]
	blx	ip
	cmn	r0, #1
	beq	.L2071
	ldr	r2, [sp, #48]
	movw	r3, #61424
	cmp	r2, r3
	bne	.L2071
	add	r6, r6, #2048
.L2069:
	add	r5, r5, #1
	b	.L2067
.L2073:
	ldr	r1, [sp, #28]
	add	r5, r5, #1
	cmp	r5, r1
	bne	.L2070
	ldr	r3, [sp, #20]
	add	r3, r3, #1
	cmp	r3, #5
	str	r3, [sp, #20]
	bls	.L2074
	b	.L2077
.L2046:
	cmp	r0, #64
	mov	r4, r0
	mov	r6, r1
	mov	r7, r2
	bne	.L2078
	mov	r0, #262144
	bl	ftl_malloc
	str	r0, [r5, #3680]
	mov	r0, #262144
	bl	ftl_malloc
	ldr	r3, [r5, #3680]
	str	r0, [r5, #3684]
	cmp	r3, #0
	cmpne	r0, #0
	beq	.L2079
	mov	r2, #1
	mov	r1, r9
	str	r2, [r5, #3676]
	mov	r0, r3
	mov	r2, #262144
	bl	ftl_memset
.L2078:
	ldr	r3, [r5, #3676]
	cmp	r3, #0
	beq	.L2105
	cmp	r4, #63
	ldr	r0, [r5, #3680]
	ldrhi	r3, .L2106+24
	rsbls	r1, r4, #64
	subls	r6, r6, r1
	movhi	r1, r7
	addls	r1, r7, r1, lsl #9
	addhi	r3, r4, r3
	addhi	r0, r0, r3, lsl #9
	cmp	r10, #576
	subcs	r6, r6, r8
	subcs	r6, r6, #444
	subcs	r6, r6, #2
	lsl	r2, r6, #9
	bl	ftl_memcpy
	b	.L2105
.L2079:
	ldr	r1, .L2106+28
	ldr	r0, .L2106+32
	bl	sftl_printk
	b	.L2078
.L2084:
	add	sp, sp, #116
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L2107:
	.align	2
.L2106:
	.word	__stack_chk_guard
	.word	.LANCHOR0
	.word	-52655045
	.word	.LC130
	.word	1179535694
	.word	.LC131
	.word	8388544
	.word	.LANCHOR1+633
	.word	.LC132
	.fnend
	.size	FtlWriteToIDB, .-FtlWriteToIDB
	.align	2
	.global	sftl_write
	.syntax unified
	.arm
	.fpu softvfp
	.type	sftl_write, %function
sftl_write:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r5, r1
	mov	r6, r2
	mov	r4, r0
	bl	FtlWriteToIDB
	mov	r3, r6
	mov	r2, r5
	mov	r1, r4
	mov	r0, #0
	pop	{r4, r5, r6, lr}
	b	FtlWrite
	.fnend
	.size	sftl_write, .-sftl_write
	.align	2
	.global	rk_sftl_vendor_dev_ops_register
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_dev_ops_register, %function
rk_sftl_vendor_dev_ops_register:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r2, .L2113
	ldr	r3, [r2, #3688]
	cmp	r3, #0
	streq	r0, [r2, #3688]
	moveq	r0, r3
	streq	r1, [r2, #3692]
	mvnne	r0, #0
	bx	lr
.L2114:
	.align	2
.L2113:
	.word	.LANCHOR0
	.fnend
	.size	rk_sftl_vendor_dev_ops_register, .-rk_sftl_vendor_dev_ops_register
	.align	2
	.global	rk_sftl_vendor_storage_init
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_storage_init, %function
rk_sftl_vendor_storage_init:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, lr}
	mov	r0, #65536
	ldr	r5, .L2125
	bl	ftl_malloc
	cmp	r0, #0
	str	r0, [r5, #3696]
	mvneq	r9, #11
	beq	.L2115
	ldr	r10, .L2125+4
	mov	r7, #0
	mov	r6, r7
	mov	r8, r7
.L2119:
	ldr	r3, [r5, #3688]
	mov	r1, #128
	ldr	r2, [r5, #3696]
	lsl	r0, r8, #7
	blx	r3
	subs	r9, r0, #0
	bne	.L2117
	ldr	r4, [r5, #3696]
	ldr	r3, [r4]
	cmp	r3, r10
	bne	.L2118
	add	r2, r4, #61440
	ldr	r3, [r4, #4]
	ldr	r2, [r2, #4092]
	cmp	r3, r6
	sub	r2, r2, r3
	clz	r2, r2
	lsr	r2, r2, #5
	movls	r2, #0
	cmp	r2, #0
	movne	r7, r8
	movne	r6, r3
.L2118:
	add	r8, r8, #1
	cmp	r8, #2
	bne	.L2119
	cmp	r6, #0
	beq	.L2120
	ldr	r3, [r5, #3688]
	mov	r2, r4
	mov	r1, #128
	lsl	r0, r7, #7
	blx	r3
	subs	r9, r0, #0
	beq	.L2115
.L2117:
	ldr	r0, [r5, #3696]
	mvn	r9, #0
	bl	kfree
	mov	r3, #0
	str	r3, [r5, #3696]
	b	.L2115
.L2120:
	mov	r2, #65536
	mov	r1, r6
	mov	r0, r4
	bl	memset
	mov	r3, #1
	add	r2, r4, #61440
	str	r3, [r4, #4]
	str	r10, [r4]
	str	r3, [r2, #4092]
	ldr	r3, .L2125+8
	strh	r6, [r4, #12]	@ movhi
	strh	r3, [r4, #14]	@ movhi
.L2115:
	mov	r0, r9
	pop	{r4, r5, r6, r7, r8, r9, r10, pc}
.L2126:
	.align	2
.L2125:
	.word	.LANCHOR0
	.word	1380668996
	.word	-1032
	.fnend
	.size	rk_sftl_vendor_storage_init, .-rk_sftl_vendor_storage_init
	.align	2
	.global	rk_sftl_vendor_read
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_read, %function
rk_sftl_vendor_read:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	ldr	r3, .L2137
	ldr	ip, [r3, #3696]
	cmp	ip, #0
	beq	.L2132
	push	{r4, r5, r6, lr}
	.save {r4, r5, r6, lr}
	mov	r3, #0
	ldrh	r4, [ip, #10]
.L2129:
	cmp	r3, r4
	bcc	.L2131
	mvn	r0, #0
	pop	{r4, r5, r6, pc}
.L2131:
	add	lr, ip, r3, lsl #3
	ldrh	r5, [lr, #16]
	cmp	r5, r0
	bne	.L2130
	ldrh	r4, [lr, #20]
	mov	r0, r1
	ldrh	r1, [lr, #18]
	cmp	r4, r2
	movcs	r4, r2
	add	r1, r1, #1024
	mov	r2, r4
	add	r1, ip, r1
	bl	memcpy
	mov	r0, r4
	pop	{r4, r5, r6, pc}
.L2130:
	add	r3, r3, #1
	b	.L2129
.L2132:
	mvn	r0, #0
	bx	lr
.L2138:
	.align	2
.L2137:
	.word	.LANCHOR0
	.fnend
	.size	rk_sftl_vendor_read, .-rk_sftl_vendor_read
	.align	2
	.global	rk_sftl_vendor_write
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_write, %function
rk_sftl_vendor_write:
	.fnstart
	@ args = 0, pretend = 0, frame = 24
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.save {r4, r5, r6, r7, r8, r9, r10, fp, lr}
	.pad #28
	sub	sp, sp, #28
	ldr	r9, .L2159
	ldr	r4, [r9, #3696]
	cmp	r4, #0
	beq	.L2154
	mov	r8, r2
	ldrh	r2, [r4, #10]
	add	r6, r8, #63
	ldrh	r3, [r4, #8]
	mov	fp, r1
	bic	r6, r6, #63
	mov	r7, #0
	str	r3, [sp, #4]
.L2141:
	cmp	r7, r2
	bcc	.L2149
	ldrh	r1, [r4, #14]
	cmp	r6, r1
	bhi	.L2154
	add	r3, r4, r2, lsl #3
	uxth	r6, r6
	strh	r0, [r3, #16]	@ movhi
	ldrh	r2, [r4, #12]
	strh	r8, [r3, #20]	@ movhi
	strh	r2, [r3, #18]	@ movhi
	add	r2, r2, r6
	sub	r6, r1, r6
	strh	r2, [r4, #12]	@ movhi
	strh	r6, [r4, #14]	@ movhi
	mov	r2, r8
	ldrh	r0, [r3, #18]
	mov	r1, fp
	add	r0, r0, #1024
	add	r0, r4, r0
	bl	memcpy
	ldrh	r3, [r4, #10]
	add	r2, r4, #61440
	add	r3, r3, #1
	strh	r3, [r4, #10]	@ movhi
	ldr	r3, [r4, #4]
	add	r3, r3, #1
	str	r3, [r4, #4]
	str	r3, [r2, #4092]
	ldrh	r3, [r4, #8]
	add	r3, r3, #1
	uxth	r3, r3
	cmp	r3, #1
	movhi	r3, #0
	strh	r3, [r4, #8]	@ movhi
	ldr	r3, [r9, #3692]
	b	.L2158
.L2149:
	add	r5, r4, r7, lsl #3
	ldrh	r3, [r5, #16]
	cmp	r3, r0
	str	r3, [sp, #8]
	bne	.L2142
	ldrh	r1, [r5, #20]
	add	r3, r4, #1024
	add	r1, r1, #63
	bic	r1, r1, #63
	cmp	r8, r1
	str	r1, [sp, #12]
	bls	.L2143
	ldrh	r1, [r4, #14]
	cmp	r6, r1
	subls	r2, r2, #1
	ldrhls	r10, [r5, #18]
	strls	r2, [sp, #16]
	bls	.L2144
.L2154:
	mvn	r0, #0
	b	.L2139
.L2145:
	ldrh	r9, [r5, #20]
	add	r0, r3, r10
	ldrh	r2, [r5, #16]
	add	r7, r7, #1
	ldrh	r1, [r5, #18]
	strh	r9, [r5, #12]	@ movhi
	add	r9, r9, #63
	bic	r9, r9, #63
	strh	r2, [r5, #8]	@ movhi
	strh	r10, [r5, #10]	@ movhi
	add	r1, r3, r1
	mov	r2, r9
	str	r3, [sp, #20]
	bl	memcpy
	ldr	r3, [sp, #20]
	add	r10, r10, r9
.L2144:
	ldr	r2, [sp, #16]
	add	r5, r5, #8
	cmp	r7, r2
	bcc	.L2145
	ldrh	r2, [sp, #8]
	add	r7, r4, r7, lsl #3
	uxth	r5, r10
	uxtah	r0, r3, r10
	strh	r8, [r7, #20]	@ movhi
	strh	r2, [r7, #16]	@ movhi
	mov	r1, fp
	strh	r5, [r7, #18]	@ movhi
	mov	r2, r8
	bl	memcpy
	uxth	r3, r6
	ldrh	r6, [r4, #14]
	add	r5, r5, r3
	sub	r6, r6, r3
	ldr	r3, [sp, #12]
	strh	r5, [r4, #12]	@ movhi
	add	r6, r6, r3
	strh	r6, [r4, #14]	@ movhi
.L2146:
	ldr	r3, [r4, #4]
	add	r2, r4, #61440
	add	r3, r3, #1
	str	r3, [r4, #4]
	str	r3, [r2, #4092]
	ldrh	r3, [r4, #8]
	add	r3, r3, #1
	uxth	r3, r3
	cmp	r3, #1
	movhi	r3, #0
	strh	r3, [r4, #8]	@ movhi
	ldr	r3, .L2159
	ldr	r3, [r3, #3692]
.L2158:
	ldr	r0, [sp, #4]
	mov	r2, r4
	mov	r1, #128
	lsl	r0, r0, #7
	blx	r3
	mov	r0, #0
.L2139:
	add	sp, sp, #28
	@ sp needed
	pop	{r4, r5, r6, r7, r8, r9, r10, fp, pc}
.L2143:
	ldrh	r0, [r5, #18]
	mov	r2, r8
	mov	r1, fp
	add	r0, r3, r0
	bl	memcpy
	strh	r8, [r5, #20]	@ movhi
	b	.L2146
.L2142:
	add	r7, r7, #1
	b	.L2141
.L2160:
	.align	2
.L2159:
	.word	.LANCHOR0
	.fnend
	.size	rk_sftl_vendor_write, .-rk_sftl_vendor_write
	.align	2
	.global	rk_sftl_vendor_storage_ioctl
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_storage_ioctl, %function
rk_sftl_vendor_storage_ioctl:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	push	{r4, r5, r6, r7, r8, lr}
	.save {r4, r5, r6, r7, r8, lr}
	mov	r0, #4096
	mov	r5, r2
	mov	r6, r1
	bl	ftl_malloc
	subs	r4, r0, #0
	mvneq	r5, #0
	beq	.L2161
	ldr	r3, .L2186
	cmp	r6, r3
	beq	.L2164
	add	r3, r3, #1
	cmp	r6, r3
	beq	.L2165
.L2183:
	mvn	r5, #13
	b	.L2163
.L2164:
	ldr	r6, .L2186+4
	mov	r3, sp
	and	r3, r3, r6
	ldr	r3, [r3, #8]
	.syntax divided
@ 114 "./include/linux/uaccess.h" 1
	adds r2, r5, #8; sbcccs r2, r2, r3; movcc r3, #0
@ 0 "" 2
	.arm
	.syntax unified
	cmp	r3, #0
	mov	r2, #8
	bne	.L2166
	mov	r1, r5
	bl	arm_copy_from_user
	subs	r2, r0, #0
	beq	.L2167
.L2166:
	rsb	r0, r2, #8
	mov	r1, #0
	add	r0, r4, r0
	bl	memset
	mov	r2, r5
	mov	r1, #256
.L2185:
	ldr	r0, .L2186+8
	bl	sftl_printk
	b	.L2183
.L2168:
	mov	r3, sp
	uxth	r2, r0
	and	r6, r6, r3
	strh	r0, [r4, #6]	@ movhi
	add	r2, r2, #8
	ldr	r3, [r6, #8]
	.syntax divided
@ 132 "./include/linux/uaccess.h" 1
	adds r1, r5, r2; sbcccs r1, r1, r3; movcc r3, #0
@ 0 "" 2
	.arm
	.syntax unified
	cmp	r3, #0
	bne	.L2183
	mov	r0, r5
	mov	r1, r4
	bl	arm_copy_to_user
	subs	r5, r0, #0
	beq	.L2163
	b	.L2183
.L2165:
	ldr	r7, .L2186+4
	mov	r3, sp
	and	r3, r3, r7
	ldr	r3, [r3, #8]
	.syntax divided
@ 114 "./include/linux/uaccess.h" 1
	adds r2, r5, #8; sbcccs r2, r2, r3; movcc r3, #0
@ 0 "" 2
	.arm
	.syntax unified
	cmp	r3, #0
	mov	r2, #8
	bne	.L2170
	mov	r1, r5
	bl	arm_copy_from_user
	subs	r2, r0, #0
	beq	.L2171
.L2170:
	rsb	r0, r2, #8
	mov	r1, #0
	add	r0, r4, r0
	bl	memset
	mov	r2, r5
	mov	r1, #276
	b	.L2185
.L2167:
	ldr	r2, [r4]
	ldr	r3, .L2186+12
	cmp	r2, r3
	bne	.L2172
	ldrh	r2, [r4, #6]
	add	r1, r4, #8
	ldrh	r0, [r4, #4]
	bl	rk_sftl_vendor_read
	cmn	r0, #1
	bne	.L2168
.L2172:
	mvn	r5, #0
.L2163:
	mov	r0, r4
	bl	kfree
.L2161:
	mov	r0, r5
	pop	{r4, r5, r6, r7, r8, pc}
.L2171:
	ldr	r2, [r4]
	ldr	r3, .L2186+12
	cmp	r2, r3
	bne	.L2172
	ldrh	r6, [r4, #6]
	movw	r3, #4087
	cmp	r6, r3
	bhi	.L2172
	mov	r3, sp
	add	r6, r6, #8
	and	r7, r7, r3
	ldr	r3, [r7, #8]
	.syntax divided
@ 114 "./include/linux/uaccess.h" 1
	adds r2, r5, r6; sbcccs r2, r2, r3; movcc r3, #0
@ 0 "" 2
	.arm
	.syntax unified
	cmp	r3, #0
	mov	r2, r6
	bne	.L2173
	mov	r1, r5
	mov	r0, r4
	bl	arm_copy_from_user
	subs	r2, r0, #0
	beq	.L2174
.L2173:
	sub	r6, r6, r2
	mov	r1, #0
	add	r0, r4, r6
	bl	memset
	mov	r2, r5
	movw	r1, #283
	b	.L2185
.L2174:
	ldrh	r2, [r4, #6]
	add	r1, r4, #8
	ldrh	r0, [r4, #4]
	bl	rk_sftl_vendor_write
	mov	r5, r0
	b	.L2163
.L2187:
	.align	2
.L2186:
	.word	1074034177
	.word	-8192
	.word	.LC133
	.word	1448232273
	.fnend
	.size	rk_sftl_vendor_storage_ioctl, .-rk_sftl_vendor_storage_ioctl
	.align	2
	.global	rk_sftl_vendor_register
	.syntax unified
	.arm
	.fpu softvfp
	.type	rk_sftl_vendor_register, %function
rk_sftl_vendor_register:
	.fnstart
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
	@ link register save eliminated.
	ldr	r0, .L2189
	b	misc_register
.L2190:
	.align	2
.L2189:
	.word	.LANCHOR2+12
	.fnend
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
	.type	__func__.7563, %object
	.size	__func__.7563, 17
__func__.7563:
	.ascii	"INSERT_DATA_LIST\000"
	.type	__func__.7558, %object
	.size	__func__.7558, 17
__func__.7558:
	.ascii	"INSERT_FREE_LIST\000"
	.type	__func__.7594, %object
	.size	__func__.7594, 17
__func__.7594:
	.ascii	"List_remove_node\000"
	.type	__func__.7626, %object
	.size	__func__.7626, 22
__func__.7626:
	.ascii	"List_update_data_list\000"
	.type	__func__.7733, %object
	.size	__func__.7733, 22
__func__.7733:
	.ascii	"select_l2p_ram_region\000"
	.type	__func__.8053, %object
	.size	__func__.8053, 16
__func__.8053:
	.ascii	"make_superblock\000"
	.type	__func__.8229, %object
	.size	__func__.8229, 19
__func__.8229:
	.ascii	"get_new_active_ppa\000"
	.type	__func__.13983, %object
	.size	__func__.13983, 17
__func__.13983:
	.ascii	"FlashEraseBlocks\000"
	.type	__func__.7668, %object
	.size	__func__.7668, 26
__func__.7668:
	.ascii	"ftl_map_blk_alloc_new_blk\000"
	.type	__func__.7173, %object
	.size	__func__.7173, 11
__func__.7173:
	.ascii	"FtlMemInit\000"
	.type	__func__.7400, %object
	.size	__func__.7400, 14
__func__.7400:
	.ascii	"FtlBbt2Bitmap\000"
	.type	__func__.8074, %object
	.size	__func__.8074, 18
__func__.8074:
	.ascii	"SupperBlkListInit\000"
	.type	__func__.13940, %object
	.size	__func__.13940, 15
__func__.13940:
	.ascii	"FlashReadPages\000"
	.type	__func__.7845, %object
	.size	__func__.7845, 14
__func__.7845:
	.ascii	"FtlScanSysBlk\000"
	.type	__func__.7443, %object
	.size	__func__.7443, 11
__func__.7443:
	.ascii	"FtlLoadBbt\000"
	.type	__func__.13959, %object
	.size	__func__.13959, 15
__func__.13959:
	.ascii	"FlashProgPages\000"
	.type	__func__.8201, %object
	.size	__func__.8201, 25
__func__.8201:
	.ascii	"allocate_data_superblock\000"
	.type	__func__.8242, %object
	.size	__func__.8242, 16
__func__.8242:
	.ascii	"update_vpc_list\000"
	.type	__func__.8249, %object
	.size	__func__.8249, 20
__func__.8249:
	.ascii	"decrement_vpc_count\000"
	.type	__func__.7694, %object
	.size	__func__.7694, 31
__func__.7694:
	.ascii	"Ftl_write_map_blk_to_last_page\000"
	.type	__func__.7708, %object
	.size	__func__.7708, 16
__func__.7708:
	.ascii	"FtlMapWritePage\000"
	.type	__func__.7635, %object
	.size	__func__.7635, 16
__func__.7635:
	.ascii	"load_l2p_region\000"
	.type	__func__.7679, %object
	.size	__func__.7679, 15
__func__.7679:
	.ascii	"ftl_map_blk_gc\000"
	.type	__func__.7750, %object
	.size	__func__.7750, 9
__func__.7750:
	.ascii	"log2phys\000"
	.type	__func__.7962, %object
	.size	__func__.7962, 16
__func__.7962:
	.ascii	"FtlReUsePrevPpa\000"
	.type	__func__.7996, %object
	.size	__func__.7996, 22
__func__.7996:
	.ascii	"FtlRecoverySuperblock\000"
	.type	__func__.8116, %object
	.size	__func__.8116, 14
__func__.8116:
	.ascii	"ftl_check_vpc\000"
	.type	__func__.7823, %object
	.size	__func__.7823, 15
__func__.7823:
	.ascii	"FtlVpcTblFlush\000"
	.type	__func__.8099, %object
	.size	__func__.8099, 21
__func__.8099:
	.ascii	"FtlVpcCheckAndModify\000"
	.type	__func__.8222, %object
	.size	__func__.8222, 29
__func__.8222:
	.ascii	"allocate_new_data_superblock\000"
	.type	__func__.7297, %object
	.size	__func__.7297, 13
__func__.7297:
	.ascii	"FtlProgPages\000"
	.type	__func__.8319, %object
	.size	__func__.8319, 19
__func__.8319:
	.ascii	"FtlGcFreeTempBlock\000"
	.type	__func__.8432, %object
	.size	__func__.8432, 23
__func__.8432:
	.ascii	"rk_ftl_garbage_collect\000"
	.type	__func__.7325, %object
	.size	__func__.7325, 9
__func__.7325:
	.ascii	"FtlWrite\000"
	.type	__func__.7894, %object
	.size	__func__.7894, 15
__func__.7894:
	.ascii	"FtlLoadSysInfo\000"
	.type	__func__.7916, %object
	.size	__func__.7916, 18
__func__.7916:
	.ascii	"FtlMapTblRecovery\000"
	.type	__func__.14102, %object
	.size	__func__.14102, 14
__func__.14102:
	.ascii	"FtlWriteToIDB\000"
	.space	1
	.type	rk_sftl_vendor_storage_fops, %object
	.size	rk_sftl_vendor_storage_fops, 160
rk_sftl_vendor_storage_fops:
	.space	36
	.word	rk_sftl_vendor_storage_ioctl
	.word	rk_sftl_vendor_storage_ioctl
	.space	116
	.data
	.align	2
	.set	.LANCHOR2,. + 0
	.type	gFtlInitStatus, %object
	.size	gFtlInitStatus, 4
gFtlInitStatus:
	.word	-1
	.type	ftl_gc_temp_block_bops_scan_page_addr, %object
	.size	ftl_gc_temp_block_bops_scan_page_addr, 2
ftl_gc_temp_block_bops_scan_page_addr:
	.short	-1
	.space	2
	.type	power_up_flag, %object
	.size	power_up_flag, 4
power_up_flag:
	.word	1
	.type	rkflash_vender_storage_dev, %object
	.size	rkflash_vender_storage_dev, 40
rkflash_vender_storage_dev:
	.word	255
	.word	.LC134
	.word	rk_sftl_vendor_storage_fops
	.space	28
	.bss
	.align	2
	.set	.LANCHOR0,. + 0
	.type	g_nand_phy_info, %object
	.size	g_nand_phy_info, 24
g_nand_phy_info:
	.space	24
	.type	g_active_superblock, %object
	.size	g_active_superblock, 48
g_active_superblock:
	.space	48
	.type	p_valid_page_count_table, %object
	.size	p_valid_page_count_table, 4
p_valid_page_count_table:
	.space	4
	.type	g_buffer_superblock, %object
	.size	g_buffer_superblock, 48
g_buffer_superblock:
	.space	48
	.type	g_gc_temp_superblock, %object
	.size	g_gc_temp_superblock, 48
g_gc_temp_superblock:
	.space	48
	.type	g_gc_superblock, %object
	.size	g_gc_superblock, 48
g_gc_superblock:
	.space	48
	.type	g_gc_next_blk_1, %object
	.size	g_gc_next_blk_1, 2
g_gc_next_blk_1:
	.space	2
	.type	g_gc_next_blk, %object
	.size	g_gc_next_blk, 2
g_gc_next_blk:
	.space	2
	.type	g_num_free_superblocks, %object
	.size	g_num_free_superblocks, 2
g_num_free_superblocks:
	.space	2
	.space	2
	.type	c_ftl_nand_sys_blks_per_plane, %object
	.size	c_ftl_nand_sys_blks_per_plane, 4
c_ftl_nand_sys_blks_per_plane:
	.space	4
	.type	c_ftl_nand_planes_num, %object
	.size	c_ftl_nand_planes_num, 2
c_ftl_nand_planes_num:
	.space	2
	.space	2
	.type	c_ftl_nand_max_sys_blks, %object
	.size	c_ftl_nand_max_sys_blks, 4
c_ftl_nand_max_sys_blks:
	.space	4
	.type	c_ftl_nand_data_blks_per_plane, %object
	.size	c_ftl_nand_data_blks_per_plane, 2
c_ftl_nand_data_blks_per_plane:
	.space	2
	.type	c_ftl_nand_blk_pre_plane, %object
	.size	c_ftl_nand_blk_pre_plane, 2
c_ftl_nand_blk_pre_plane:
	.space	2
	.type	c_ftl_nand_max_data_blks, %object
	.size	c_ftl_nand_max_data_blks, 4
c_ftl_nand_max_data_blks:
	.space	4
	.type	c_ftl_nand_totle_phy_blks, %object
	.size	c_ftl_nand_totle_phy_blks, 4
c_ftl_nand_totle_phy_blks:
	.space	4
	.type	c_ftl_nand_type, %object
	.size	c_ftl_nand_type, 2
c_ftl_nand_type:
	.space	2
	.type	c_ftl_nand_die_num, %object
	.size	c_ftl_nand_die_num, 2
c_ftl_nand_die_num:
	.space	2
	.type	c_ftl_nand_planes_per_die, %object
	.size	c_ftl_nand_planes_per_die, 2
c_ftl_nand_planes_per_die:
	.space	2
	.type	c_ftl_nand_sec_pre_page, %object
	.size	c_ftl_nand_sec_pre_page, 2
c_ftl_nand_sec_pre_page:
	.space	2
	.type	p_plane_order_table, %object
	.size	p_plane_order_table, 32
p_plane_order_table:
	.space	32
	.type	c_mlc_erase_count_value, %object
	.size	c_mlc_erase_count_value, 2
c_mlc_erase_count_value:
	.space	2
	.type	c_ftl_nand_ext_blk_pre_plane, %object
	.size	c_ftl_nand_ext_blk_pre_plane, 2
c_ftl_nand_ext_blk_pre_plane:
	.space	2
	.type	c_ftl_vendor_part_size, %object
	.size	c_ftl_vendor_part_size, 2
c_ftl_vendor_part_size:
	.space	2
	.type	c_ftl_nand_blks_per_die, %object
	.size	c_ftl_nand_blks_per_die, 2
c_ftl_nand_blks_per_die:
	.space	2
	.type	c_ftl_nand_blks_per_die_shift, %object
	.size	c_ftl_nand_blks_per_die_shift, 2
c_ftl_nand_blks_per_die_shift:
	.space	2
	.type	c_ftl_nand_page_pre_blk, %object
	.size	c_ftl_nand_page_pre_blk, 2
c_ftl_nand_page_pre_blk:
	.space	2
	.type	c_ftl_nand_page_pre_slc_blk, %object
	.size	c_ftl_nand_page_pre_slc_blk, 2
c_ftl_nand_page_pre_slc_blk:
	.space	2
	.type	c_ftl_nand_page_pre_super_blk, %object
	.size	c_ftl_nand_page_pre_super_blk, 2
c_ftl_nand_page_pre_super_blk:
	.space	2
	.type	c_ftl_nand_sec_pre_page_shift, %object
	.size	c_ftl_nand_sec_pre_page_shift, 2
c_ftl_nand_sec_pre_page_shift:
	.space	2
	.type	c_ftl_nand_byte_pre_page, %object
	.size	c_ftl_nand_byte_pre_page, 2
c_ftl_nand_byte_pre_page:
	.space	2
	.type	c_ftl_nand_byte_pre_oob, %object
	.size	c_ftl_nand_byte_pre_oob, 2
c_ftl_nand_byte_pre_oob:
	.space	2
	.type	c_ftl_nand_reserved_blks, %object
	.size	c_ftl_nand_reserved_blks, 2
c_ftl_nand_reserved_blks:
	.space	2
	.type	DeviceCapacity, %object
	.size	DeviceCapacity, 4
DeviceCapacity:
	.space	4
	.type	c_ftl_nand_max_vendor_blks, %object
	.size	c_ftl_nand_max_vendor_blks, 2
c_ftl_nand_max_vendor_blks:
	.space	2
	.type	c_ftl_nand_vendor_region_num, %object
	.size	c_ftl_nand_vendor_region_num, 2
c_ftl_nand_vendor_region_num:
	.space	2
	.type	c_ftl_nand_map_blks_per_plane, %object
	.size	c_ftl_nand_map_blks_per_plane, 2
c_ftl_nand_map_blks_per_plane:
	.space	2
	.space	2
	.type	c_ftl_nand_max_map_blks, %object
	.size	c_ftl_nand_max_map_blks, 4
c_ftl_nand_max_map_blks:
	.space	4
	.type	c_ftl_nand_init_sys_blks_per_plane, %object
	.size	c_ftl_nand_init_sys_blks_per_plane, 4
c_ftl_nand_init_sys_blks_per_plane:
	.space	4
	.type	c_ftl_nand_map_region_num, %object
	.size	c_ftl_nand_map_region_num, 2
c_ftl_nand_map_region_num:
	.space	2
	.type	c_ftl_nand_l2pmap_ram_region_num, %object
	.size	c_ftl_nand_l2pmap_ram_region_num, 2
c_ftl_nand_l2pmap_ram_region_num:
	.space	2
	.type	g_MaxLbaSector, %object
	.size	g_MaxLbaSector, 4
g_MaxLbaSector:
	.space	4
	.type	g_totle_vendor_block, %object
	.size	g_totle_vendor_block, 2
g_totle_vendor_block:
	.space	2
	.space	2
	.type	p_vendor_block_table, %object
	.size	p_vendor_block_table, 4
p_vendor_block_table:
	.space	4
	.type	gBbtInfo, %object
	.size	gBbtInfo, 60
gBbtInfo:
	.space	60
	.type	gSysFreeQueue, %object
	.size	gSysFreeQueue, 2056
gSysFreeQueue:
	.space	2056
	.type	g_sys_save_data, %object
	.size	g_sys_save_data, 48
g_sys_save_data:
	.space	48
	.type	p_data_block_list_table, %object
	.size	p_data_block_list_table, 4
p_data_block_list_table:
	.space	4
	.type	p_data_block_list_head, %object
	.size	p_data_block_list_head, 4
p_data_block_list_head:
	.space	4
	.type	p_data_block_list_tail, %object
	.size	p_data_block_list_tail, 4
p_data_block_list_tail:
	.space	4
	.type	g_num_data_superblocks, %object
	.size	g_num_data_superblocks, 2
g_num_data_superblocks:
	.space	2
	.space	2
	.type	p_free_data_block_list_head, %object
	.size	p_free_data_block_list_head, 4
p_free_data_block_list_head:
	.space	4
	.type	p_erase_count_table, %object
	.size	p_erase_count_table, 4
p_erase_count_table:
	.space	4
	.type	p_l2p_ram_map, %object
	.size	p_l2p_ram_map, 4
p_l2p_ram_map:
	.space	4
	.type	g_l2p_last_update_region_id, %object
	.size	g_l2p_last_update_region_id, 2
g_l2p_last_update_region_id:
	.space	2
	.type	FtlUpdateVaildLpnCount, %object
	.size	FtlUpdateVaildLpnCount, 2
FtlUpdateVaildLpnCount:
	.space	2
	.type	g_VaildLpn, %object
	.size	g_VaildLpn, 4
g_VaildLpn:
	.space	4
	.type	g_MaxLpn, %object
	.size	g_MaxLpn, 4
g_MaxLpn:
	.space	4
	.type	g_totle_read_page_count, %object
	.size	g_totle_read_page_count, 4
g_totle_read_page_count:
	.space	4
	.type	g_totle_discard_page_count, %object
	.size	g_totle_discard_page_count, 4
g_totle_discard_page_count:
	.space	4
	.type	g_totle_write_page_count, %object
	.size	g_totle_write_page_count, 4
g_totle_write_page_count:
	.space	4
	.type	g_totle_cache_write_count, %object
	.size	g_totle_cache_write_count, 4
g_totle_cache_write_count:
	.space	4
	.type	g_totle_l2p_write_count, %object
	.size	g_totle_l2p_write_count, 4
g_totle_l2p_write_count:
	.space	4
	.type	g_totle_gc_page_count, %object
	.size	g_totle_gc_page_count, 4
g_totle_gc_page_count:
	.space	4
	.type	g_totle_write_sector, %object
	.size	g_totle_write_sector, 4
g_totle_write_sector:
	.space	4
	.type	g_totle_read_sector, %object
	.size	g_totle_read_sector, 4
g_totle_read_sector:
	.space	4
	.type	g_GlobalSysVersion, %object
	.size	g_GlobalSysVersion, 4
g_GlobalSysVersion:
	.space	4
	.type	g_GlobalDataVersion, %object
	.size	g_GlobalDataVersion, 4
g_GlobalDataVersion:
	.space	4
	.type	g_totle_mlc_erase_count, %object
	.size	g_totle_mlc_erase_count, 4
g_totle_mlc_erase_count:
	.space	4
	.type	g_totle_slc_erase_count, %object
	.size	g_totle_slc_erase_count, 4
g_totle_slc_erase_count:
	.space	4
	.type	g_totle_avg_erase_count, %object
	.size	g_totle_avg_erase_count, 4
g_totle_avg_erase_count:
	.space	4
	.type	g_totle_sys_slc_erase_count, %object
	.size	g_totle_sys_slc_erase_count, 4
g_totle_sys_slc_erase_count:
	.space	4
	.type	g_max_erase_count, %object
	.size	g_max_erase_count, 4
g_max_erase_count:
	.space	4
	.type	g_min_erase_count, %object
	.size	g_min_erase_count, 4
g_min_erase_count:
	.space	4
	.type	c_ftl_nand_data_op_blks_per_plane, %object
	.size	c_ftl_nand_data_op_blks_per_plane, 2
c_ftl_nand_data_op_blks_per_plane:
	.space	2
	.space	2
	.type	gSysInfo, %object
	.size	gSysInfo, 16
gSysInfo:
	.space	16
	.type	g_sys_ext_data, %object
	.size	g_sys_ext_data, 512
g_sys_ext_data:
	.space	512
	.type	g_gc_free_blk_threshold, %object
	.size	g_gc_free_blk_threshold, 2
g_gc_free_blk_threshold:
	.space	2
	.type	g_gc_merge_free_blk_threshold, %object
	.size	g_gc_merge_free_blk_threshold, 2
g_gc_merge_free_blk_threshold:
	.space	2
	.type	g_gc_skip_write_count, %object
	.size	g_gc_skip_write_count, 4
g_gc_skip_write_count:
	.space	4
	.type	g_gc_blk_index, %object
	.size	g_gc_blk_index, 2
g_gc_blk_index:
	.space	2
	.space	2
	.type	g_in_swl_replace, %object
	.size	g_in_swl_replace, 4
g_in_swl_replace:
	.space	4
	.type	g_gc_num_req, %object
	.size	g_gc_num_req, 4
g_gc_num_req:
	.space	4
	.type	gp_gc_page_buf_info, %object
	.size	gp_gc_page_buf_info, 4
gp_gc_page_buf_info:
	.space	4
	.type	p_gc_data_buf, %object
	.size	p_gc_data_buf, 4
p_gc_data_buf:
	.space	4
	.type	p_gc_spare_buf, %object
	.size	p_gc_spare_buf, 4
p_gc_spare_buf:
	.space	4
	.type	req_gc, %object
	.size	req_gc, 4
req_gc:
	.space	4
	.type	c_gc_page_buf_num, %object
	.size	c_gc_page_buf_num, 4
c_gc_page_buf_num:
	.space	4
	.type	p_gc_blk_tbl, %object
	.size	p_gc_blk_tbl, 4
p_gc_blk_tbl:
	.space	4
	.type	g_gc_blk_num, %object
	.size	g_gc_blk_num, 2
g_gc_blk_num:
	.space	2
	.space	2
	.type	p_gc_page_info, %object
	.size	p_gc_page_info, 4
p_gc_page_info:
	.space	4
	.type	g_gc_page_offset, %object
	.size	g_gc_page_offset, 2
g_gc_page_offset:
	.space	2
	.type	g_gc_bad_block_temp_num, %object
	.size	g_gc_bad_block_temp_num, 2
g_gc_bad_block_temp_num:
	.space	2
	.type	g_gc_bad_block_temp_tbl, %object
	.size	g_gc_bad_block_temp_tbl, 34
g_gc_bad_block_temp_tbl:
	.space	34
	.type	g_gc_bad_block_gc_index, %object
	.size	g_gc_bad_block_gc_index, 2
g_gc_bad_block_gc_index:
	.space	2
	.type	g_nand_ops, %object
	.size	g_nand_ops, 24
g_nand_ops:
	.space	24
	.type	req_erase, %object
	.size	req_erase, 4
req_erase:
	.space	4
	.type	g_in_gc_progress, %object
	.size	g_in_gc_progress, 4
g_in_gc_progress:
	.space	4
	.type	g_gc_head_data_block, %object
	.size	g_gc_head_data_block, 4
g_gc_head_data_block:
	.space	4
	.type	g_gc_head_data_block_count, %object
	.size	g_gc_head_data_block_count, 4
g_gc_head_data_block_count:
	.space	4
	.type	g_cur_erase_blk, %object
	.size	g_cur_erase_blk, 4
g_cur_erase_blk:
	.space	4
	.type	req_read, %object
	.size	req_read, 4
req_read:
	.space	4
	.type	req_gc_dst, %object
	.size	req_gc_dst, 4
req_gc_dst:
	.space	4
	.type	req_prgm, %object
	.size	req_prgm, 4
req_prgm:
	.space	4
	.type	p_sys_data_buf, %object
	.size	p_sys_data_buf, 4
p_sys_data_buf:
	.space	4
	.type	p_sys_data_buf_1, %object
	.size	p_sys_data_buf_1, 4
p_sys_data_buf_1:
	.space	4
	.type	p_vendor_data_buf, %object
	.size	p_vendor_data_buf, 4
p_vendor_data_buf:
	.space	4
	.type	p_io_data_buf_0, %object
	.size	p_io_data_buf_0, 4
p_io_data_buf_0:
	.space	4
	.type	p_io_data_buf_1, %object
	.size	p_io_data_buf_1, 4
p_io_data_buf_1:
	.space	4
	.type	sftl_nand_check_buf, %object
	.size	sftl_nand_check_buf, 4
sftl_nand_check_buf:
	.space	4
	.type	sftl_temp_buf, %object
	.size	sftl_temp_buf, 4
sftl_temp_buf:
	.space	4
	.type	sftl_nand_check_spare_buf, %object
	.size	sftl_nand_check_spare_buf, 4
sftl_nand_check_spare_buf:
	.space	4
	.type	p_sys_spare_buf, %object
	.size	p_sys_spare_buf, 4
p_sys_spare_buf:
	.space	4
	.type	p_io_spare_buf, %object
	.size	p_io_spare_buf, 4
p_io_spare_buf:
	.space	4
	.type	g_ect_tbl_info_size, %object
	.size	g_ect_tbl_info_size, 2
g_ect_tbl_info_size:
	.space	2
	.space	2
	.type	p_swl_mul_table, %object
	.size	p_swl_mul_table, 4
p_swl_mul_table:
	.space	4
	.type	gp_ect_tbl_info, %object
	.size	gp_ect_tbl_info, 4
gp_ect_tbl_info:
	.space	4
	.type	p_valid_page_count_check_table, %object
	.size	p_valid_page_count_check_table, 4
p_valid_page_count_check_table:
	.space	4
	.type	p_map_block_table, %object
	.size	p_map_block_table, 4
p_map_block_table:
	.space	4
	.type	p_map_block_valid_page_count, %object
	.size	p_map_block_valid_page_count, 4
p_map_block_valid_page_count:
	.space	4
	.type	p_vendor_block_valid_page_count, %object
	.size	p_vendor_block_valid_page_count, 4
p_vendor_block_valid_page_count:
	.space	4
	.type	p_vendor_block_ver_table, %object
	.size	p_vendor_block_ver_table, 4
p_vendor_block_ver_table:
	.space	4
	.type	p_vendor_region_ppn_table, %object
	.size	p_vendor_region_ppn_table, 4
p_vendor_region_ppn_table:
	.space	4
	.type	p_map_region_ppn_table, %object
	.size	p_map_region_ppn_table, 4
p_map_region_ppn_table:
	.space	4
	.type	p_map_block_ver_table, %object
	.size	p_map_block_ver_table, 4
p_map_block_ver_table:
	.space	4
	.type	p_l2p_map_buf, %object
	.size	p_l2p_map_buf, 4
p_l2p_map_buf:
	.space	4
	.type	c_ftl_nand_bbm_buf_size, %object
	.size	c_ftl_nand_bbm_buf_size, 2
c_ftl_nand_bbm_buf_size:
	.space	2
	.space	2
	.type	gL2pMapInfo, %object
	.size	gL2pMapInfo, 44
gL2pMapInfo:
	.space	44
	.type	g_totle_map_block, %object
	.size	g_totle_map_block, 2
g_totle_map_block:
	.space	2
	.type	g_tmp_data_superblock_id, %object
	.size	g_tmp_data_superblock_id, 2
g_tmp_data_superblock_id:
	.space	2
	.type	g_totle_swl_count, %object
	.size	g_totle_swl_count, 4
g_totle_swl_count:
	.space	4
	.type	ftl_gc_temp_power_lost_recovery_flag, %object
	.size	ftl_gc_temp_power_lost_recovery_flag, 4
ftl_gc_temp_power_lost_recovery_flag:
	.space	4
	.type	g_recovery_page_min_ver, %object
	.size	g_recovery_page_min_ver, 4
g_recovery_page_min_ver:
	.space	4
	.type	req_sys, %object
	.size	req_sys, 20
req_sys:
	.space	20
	.type	g_power_lost_recovery_flag, %object
	.size	g_power_lost_recovery_flag, 2
g_power_lost_recovery_flag:
	.space	2
	.space	2
	.type	g_recovery_page_num, %object
	.size	g_recovery_page_num, 4
g_recovery_page_num:
	.space	4
	.type	g_recovery_ppa_tbl, %object
	.size	g_recovery_ppa_tbl, 128
g_recovery_ppa_tbl:
	.space	128
	.type	gVendorBlkInfo, %object
	.size	gVendorBlkInfo, 44
gVendorBlkInfo:
	.space	44
	.type	g_ect_tbl_power_up_flush, %object
	.size	g_ect_tbl_power_up_flush, 2
g_ect_tbl_power_up_flush:
	.space	2
	.space	2
	.type	gc_discard_updated, %object
	.size	gc_discard_updated, 4
gc_discard_updated:
	.space	4
	.type	gc_ink_free_return_value, %object
	.size	gc_ink_free_return_value, 2
gc_ink_free_return_value:
	.space	2
	.type	g_gc_cur_blk_valid_pages, %object
	.size	g_gc_cur_blk_valid_pages, 2
g_gc_cur_blk_valid_pages:
	.space	2
	.type	g_gc_cur_blk_max_valid_pages, %object
	.size	g_gc_cur_blk_max_valid_pages, 2
g_gc_cur_blk_max_valid_pages:
	.space	2
	.space	2
	.type	g_ftl_nand_free_count, %object
	.size	g_ftl_nand_free_count, 4
g_ftl_nand_free_count:
	.space	4
	.type	g_MaxLbn, %object
	.size	g_MaxLbn, 4
g_MaxLbn:
	.space	4
	.type	idb_need_write_back, %object
	.size	idb_need_write_back, 4
idb_need_write_back:
	.space	4
	.type	idb_buf, %object
	.size	idb_buf, 4
idb_buf:
	.space	4
	.type	gp_flash_check_buf, %object
	.size	gp_flash_check_buf, 4
gp_flash_check_buf:
	.space	4
	.type	_flash_read, %object
	.size	_flash_read, 4
_flash_read:
	.space	4
	.type	_flash_write, %object
	.size	_flash_write, 4
_flash_write:
	.space	4
	.type	g_vendor, %object
	.size	g_vendor, 4
g_vendor:
	.space	4
	.type	check_vpc_table, %object
	.size	check_vpc_table, 16384
check_vpc_table:
	.space	16384
	.type	gp_last_act_superblock, %object
	.size	gp_last_act_superblock, 4
gp_last_act_superblock:
	.space	4
	.section	.rodata.str1.1,"aMS",%progbits,1
.LC0:
	.ascii	"SFTL version: 5.0.58 20220814\000"
.LC1:
	.ascii	"\012%s\012\000"
.LC2:
	.ascii	"act blk: %x %x %x %x %x %x\012\000"
.LC3:
	.ascii	"buf blk: %x %x %x %x %x %x\012\000"
.LC4:
	.ascii	"tmp blk: %x %x %x %x %x %x\012\000"
.LC5:
	.ascii	"gc blk: %x %x %x %x %x %x\012\000"
.LC6:
	.ascii	"free blk: %x %x %x\012\000"
.LC7:
	.ascii	"phyBlk = 0x%x die = %d block_in_die = 0x%x 0x%8x\012"
	.ascii	"\000"
.LC8:
	.ascii	"\012!!!!! error @ func:%s - line:%d\012\000"
.LC9:
	.ascii	"FLASH INFO:\012\000"
.LC10:
	.ascii	"Device Capacity: %d MB\012\000"
.LC11:
	.ascii	"FTL INFO:\012\000"
.LC12:
	.ascii	"g_MaxLpn = 0x%x\012\000"
.LC13:
	.ascii	"g_VaildLpn = 0x%x\012\000"
.LC14:
	.ascii	"read_page_count = 0x%x\012\000"
.LC15:
	.ascii	"discard_page_count = 0x%x\012\000"
.LC16:
	.ascii	"write_page_count = 0x%x\012\000"
.LC17:
	.ascii	"cache_write_count = 0x%x\012\000"
.LC18:
	.ascii	"l2p_write_count = 0x%x\012\000"
.LC19:
	.ascii	"gc_page_count = 0x%x\012\000"
.LC20:
	.ascii	"totle_write = %d MB\012\000"
.LC21:
	.ascii	"totle_read = %d MB\012\000"
.LC22:
	.ascii	"GSV = 0x%x\012\000"
.LC23:
	.ascii	"GDV = 0x%x\012\000"
.LC24:
	.ascii	"bad blk num = %d\012\000"
.LC25:
	.ascii	"free_superblocks = 0x%x\012\000"
.LC26:
	.ascii	"mlc_EC = 0x%x\012\000"
.LC27:
	.ascii	"slc_EC = 0x%x\012\000"
.LC28:
	.ascii	"avg_EC = 0x%x\012\000"
.LC29:
	.ascii	"sys_EC = 0x%x\012\000"
.LC30:
	.ascii	"max_EC = 0x%x\012\000"
.LC31:
	.ascii	"min_EC = 0x%x\012\000"
.LC32:
	.ascii	"PLT = 0x%x\012\000"
.LC33:
	.ascii	"POT = 0x%x\012\000"
.LC34:
	.ascii	"MaxSector = 0x%x\012\000"
.LC35:
	.ascii	"init_sys_blks_pp = 0x%x\012\000"
.LC36:
	.ascii	"sys_blks_pp = 0x%x\012\000"
.LC37:
	.ascii	"free sysblock = 0x%x\012\000"
.LC38:
	.ascii	"data_blks_pp = 0x%x\012\000"
.LC39:
	.ascii	"data_op_blks_pp = 0x%x\012\000"
.LC40:
	.ascii	"max_data_blks = 0x%x\012\000"
.LC41:
	.ascii	"Sys.id = 0x%x\012\000"
.LC42:
	.ascii	"Bbt.id = 0x%x\012\000"
.LC43:
	.ascii	"ACT.page = 0x%x\012\000"
.LC44:
	.ascii	"ACT.plane = 0x%x\012\000"
.LC45:
	.ascii	"ACT.id = 0x%x\012\000"
.LC46:
	.ascii	"ACT.mode = 0x%x\012\000"
.LC47:
	.ascii	"ACT.a_pages = 0x%x\012\000"
.LC48:
	.ascii	"ACT VPC = 0x%x\012\000"
.LC49:
	.ascii	"BUF.page = 0x%x\012\000"
.LC50:
	.ascii	"BUF.plane = 0x%x\012\000"
.LC51:
	.ascii	"BUF.id = 0x%x\012\000"
.LC52:
	.ascii	"BUF.mode = 0x%x\012\000"
.LC53:
	.ascii	"BUF.a_pages = 0x%x\012\000"
.LC54:
	.ascii	"BUF VPC = 0x%x\012\000"
.LC55:
	.ascii	"TMP.page = 0x%x\012\000"
.LC56:
	.ascii	"TMP.plane = 0x%x\012\000"
.LC57:
	.ascii	"TMP.id = 0x%x\012\000"
.LC58:
	.ascii	"TMP.mode = 0x%x\012\000"
.LC59:
	.ascii	"TMP.a_pages = 0x%x\012\000"
.LC60:
	.ascii	"GC.page = 0x%x\012\000"
.LC61:
	.ascii	"GC.plane = 0x%x\012\000"
.LC62:
	.ascii	"GC.id = 0x%x\012\000"
.LC63:
	.ascii	"GC.mode = 0x%x\012\000"
.LC64:
	.ascii	"GC.a_pages = 0x%x\012\000"
.LC65:
	.ascii	"WR_CHK = %x %x %x\012\000"
.LC66:
	.ascii	"Read Err Cnt = 0x%x\012\000"
.LC67:
	.ascii	"Prog Err Cnt = 0x%x\012\000"
.LC68:
	.ascii	"gc_free_blk_th= 0x%x\012\000"
.LC69:
	.ascii	"gc_merge_free_blk_th= 0x%x\012\000"
.LC70:
	.ascii	"gc_skip_write_count= 0x%x\012\000"
.LC71:
	.ascii	"gc_blk_index= 0x%x\012\000"
.LC72:
	.ascii	"free min EC= 0x%x\012\000"
.LC73:
	.ascii	"free max EC= 0x%x\012\000"
.LC74:
	.ascii	"GC__SB VPC = 0x%x\012\000"
.LC75:
	.ascii	"%d. [0x%x]=0x%x 0x%x  0x%x\012\000"
.LC76:
	.ascii	"free %d. [0x%x] 0x%x  0x%x\012\000"
.LC77:
	.ascii	"%s\012\000"
.LC78:
	.ascii	"swblk %x ,avg = %x max= %x vpc= %x,ec=%x ,max ec=%x"
	.ascii	"\012\000"
.LC79:
	.ascii	"FtlGcRefreshBlock  0x%x\012\000"
.LC80:
	.ascii	"FtlGcMarkBadPhyBlk %d 0x%x\012\000"
.LC81:
	.ascii	"%s %p + 0x%x:\000"
.LC82:
	.ascii	"0x%08x,\000"
.LC83:
	.ascii	"0x%04x,\000"
.LC84:
	.ascii	"0x%02x,\000"
.LC85:
	.ascii	"\012\000"
.LC86:
	.ascii	"%s: addr: %x is in id block!!!!!!!!!!\012\000"
.LC87:
	.ascii	"not free: w: d:\000"
.LC88:
	.ascii	"not free: w: s:\000"
.LC89:
	.ascii	"FtlFreeSysBlkQueueOut = %x, free count = %d, error\012"
	.ascii	"\000"
.LC90:
	.ascii	"FtlFreeSysBlkQueueOut = %x, free count = %d\012\000"
.LC91:
	.ascii	"%s error allocating memory. return -1\012\000"
.LC92:
	.ascii	"FlashReadPages %x %x error_ecc_bits %d\012\000"
.LC93:
	.ascii	"data:\000"
.LC94:
	.ascii	"spare:\000"
.LC95:
	.ascii	"prog read error: = %x\012\000"
.LC96:
	.ascii	"prog read REFRESH: = %x\012\000"
.LC97:
	.ascii	"prog read s error: = %x %x %x %x %x\012\000"
.LC98:
	.ascii	"prog read d error: = %x %x %x %x %x\012\000"
.LC99:
	.ascii	"FtlBbmTblFlush id=%x,page=%x,previd=%x cnt=%d\012\000"
.LC100:
	.ascii	"FtlBbmTblFlush error:%x\012\000"
.LC101:
	.ascii	"FtlBbmTblFlush error = %x error count = %d\012\000"
.LC102:
	.ascii	"FtlGcFreeBadSuperBlk 0x%x\012\000"
.LC103:
	.ascii	"decrement_vpc_count %x = %d\012\000"
.LC104:
	.ascii	"FtlMapWritePage error = %x \012\000"
.LC105:
	.ascii	"FtlMapWritePage error = %x error count = %d\012\000"
.LC106:
	.ascii	"region_id = %x phyAddr = %x\012\000"
.LC107:
	.ascii	"map_ppn:\000"
.LC108:
	.ascii	"load_l2p_region refresh = %x phyAddr = %x\012\000"
.LC109:
	.ascii	"data prev_ppa = %x error...................\012\000"
.LC110:
	.ascii	"spuer block %x vpn is 0\012 \000"
.LC111:
	.ascii	"...%s enter...\012\000"
.LC112:
	.ascii	"FtlCheckVpc2 %x = %x  %x\012\000"
.LC113:
	.ascii	"free blk vpc error %x = %x  %x\012\000"
.LC114:
	.ascii	"ftl_scan_all_data = %x\012\000"
.LC115:
	.ascii	"scan lpa = %x ppa= %x\012\000"
.LC116:
	.ascii	"lpa = %x,addr= %x,spare= %x %x %x %x data=%x %x\012"
	.ascii	"\000"
.LC117:
	.ascii	"FtlGcScanTempBlk Error ID %x %x!!!!!!! \012\000"
.LC118:
	.ascii	"FtlVendorPartRead refresh = %x phyAddr = %x\012\000"
.LC119:
	.ascii	"no ect\000"
.LC120:
	.ascii	"FtlVpcTblFlush error = %x error count = %d\012\000"
.LC121:
	.ascii	"FtlCheckVpc %x = %x  %x\012\000"
.LC122:
	.ascii	"FtlProgPages error %x = %d\012\000"
.LC123:
	.ascii	"Ftlwrite decrement_vpc_count %x = %d\012\000"
.LC124:
	.ascii	"SWL %x, FSB = %x vpc= %x,ec=%x th=%x\012\000"
.LC125:
	.ascii	"FtlWrite: ecc error:%x %x %x\012\000"
.LC126:
	.ascii	"FtlWrite: lpa error:%x %x\012\000"
.LC127:
	.ascii	"%s hash error this.id =%x page =%x pre_id =%x hash "
	.ascii	"=%x hash_r =%x\012\000"
.LC128:
	.ascii	"%s last blk_id =%x page =%x hash error hash =%x has"
	.ascii	"h_r =%x\012\000"
.LC129:
	.ascii	"%s scan blk_id =%x page =%x hash error hash =%x has"
	.ascii	"h_r =%x\012\000"
.LC130:
	.ascii	"write_idblock %x %x\012\000"
.LC131:
	.ascii	"write_idblock fail! %x %x %x %x\012\000"
.LC132:
	.ascii	"%s idb buffer alloc fail\012\000"
.LC133:
	.ascii	"copy_from_user error %d %p %p\012\000"
.LC134:
	.ascii	"vendor_storage\000"
