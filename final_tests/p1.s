	.file	"p1.c"
	.text
	.globl	func
	.type	func, @function
func:
.LF0:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	subl	$20, %esp
	movl	$1, -8(%ebp)
	movl	$1, -12(%ebp)
	movl	$0, -16(%ebp)
	jmp	.LF1
.LF1:
	movl	-16(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	%ebx, %edx
	cmpl	%ecx, %edx
	jge	.LF3
	jmp	.LF2
.LF2:
	movl	-8(%ebp), %ebx
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%ebx
	call	print
	addl	$4, %esp
	popl	%edx
	popl	%ecx
	popl	%ebx
	movl	-16(%ebp), %ecx
	movl	%ecx, %edx
	addl	$1, %edx
	movl	%edx, -16(%ebp)
	movl	-12(%ebp), %ecx
	movl	%ecx, -20(%ebp)
	movl	%ebx, %edx
	addl	%ecx, %edx
	movl	%edx, -12(%ebp)
	movl	-20(%ebp), %ebx
	movl	%ebx, -8(%ebp)
	jmp	.LF1
.LF3:
	movl	$1, %eax
	popl	%ebx
	leave
	ret
