	.file	"p2.c"
	.text
	.globl	func
	.type	func, @function
func:
.LF0:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$20, %esp
	pushl	%ebx
	movl	8(%ebp), %ebx
	movl	%ebx, %ecx
	imull	$5, %ecx
	movl	%ecx, -8(%ebp)
	movl	%ebx, %eax
	addl	$5, %eax
	movl	%eax, -12(%ebp)
	movl	-12(%ebp), %eax
	movl	%eax, -12(%ebp)
	movl	-8(%ebp), %ecx
	movl	-12(%ebp), %edx
	movl	%ecx, %eax
	addl	%edx, %eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %eax
	movl	%eax, -16(%ebp)
	movl	-16(%ebp), %ecx
	movl	%ecx, %eax
	addl	%ebx, %eax
	movl	%eax, -20(%ebp)
	movl	-20(%ebp), %eax
	movl	%eax, -20(%ebp)
	pushl	%ebx
	pushl	%ecx
	pushl	%edx
	pushl	%ecx
	call	print
	addl	$4, %esp
	popl	%edx
	popl	%ecx
	popl	%ebx
	movl	%edx, %eax
	addl	%ecx, %eax
	movl	%eax, 8(%ebp)
	movl	8(%ebp), %eax
	movl	%eax, 8(%ebp)
	movl	-20(%ebp), %eax
	popl	%ebx
	leave
	ret
