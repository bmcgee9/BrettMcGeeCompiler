	.text
	.file	"p2.c"
	.globl	func                            # -- Begin function func
	.p2align	4, 0x90
	.type	func,@function
func:                                   # @func
	.cfi_startproc
# %bb.0:
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset %ebp, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register %ebp
	pushl	%ebx
	subl	$20, %esp
	.cfi_offset %ebx, -12
	calll	.L0$pb
.L0$pb:
	popl	%ebx
.Ltmp0:
	addl	$_GLOBAL_OFFSET_TABLE_+(.Ltmp0-.L0$pb), %ebx
	movl	8(%ebp), %eax
	imull	$5, 8(%ebp), %eax
	movl	%eax, -8(%ebp)
	movl	8(%ebp), %eax
	addl	$5, %eax
	movl	%eax, -12(%ebp)
	movl	-8(%ebp), %eax
	addl	-12(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	addl	8(%ebp), %eax
	movl	%eax, -20(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, (%esp)
	calll	print@PLT
	movl	-12(%ebp), %eax
	addl	-8(%ebp), %eax
	movl	%eax, 8(%ebp)
	movl	-20(%ebp), %eax
	addl	$20, %esp
	popl	%ebx
	popl	%ebp
	.cfi_def_cfa %esp, 4
	retl
.Lfunc_end0:
	.size	func, .Lfunc_end0-func
	.cfi_endproc
                                        # -- End function
	.ident	"Ubuntu clang version 15.0.7"
	.section	".note.GNU-stack","",@progbits
	.addrsig
	.addrsig_sym print
