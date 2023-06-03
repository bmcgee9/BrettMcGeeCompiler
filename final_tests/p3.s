	.file	"p3.c"
	.text
	.globl	func
	.type	func, @function
func:
.LF0:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$16, %esp
	pushl	%ebx
	movl	$0, -8(%ebp)
	movl	$0, -12(%ebp)
	jmp	.LF1
.LF1:
	movl	-12(%ebp), %ebx
	movl	8(%ebp), %ecx
	movl	%ebx, %edx
	cmpl	%ecx, %edx
	jge	.LF3
	jmp	.LF2
.LF2:
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	call	read
	popl	%edx
	popl	%ecx
	popl	%ebx
	movl	%eax, %ebx
	movl	%ebx, -16(%ebp)
	movl	-8(%ebp), %ebx
	movl	-16(%ebp), %ecx
	addl	%ecx, %ebx
	movl	%ebx, -8(%ebp)
	movl	-12(%ebp), %ebx
	addl	$1, %ebx
	movl	%ebx, -12(%ebp)
	jmp	.LF1
.LF3:
	movl	-8(%ebp), %ebx
	movl	%ebx, %eax
	popl	%ebx
	leave
	ret
