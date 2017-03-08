	.file	"main.c"
	.section	.rodata
.LC0:
	.string	"%d \n"
.LC1:
	.string	"argv is %d \n"
.LC2:
	.string	"argc[%d] is %s \n"
	.text
	.globl	main
	.type	main, @function
main:
.LFB0:
	.cfi_startproc
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$32, %rsp
	movl	%edi, -20(%rbp)
	movq	%rsi, -32(%rbp)
	movl	$6, %esi
	movl	$4, %edi
	call	max@PLT
	movl	%eax, %esi
	leaq	.LC0(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	-20(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC1(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	movl	$0, -16(%rbp)
	jmp	.L2
.L3:
	movl	-16(%rbp), %eax
	cltq
	leaq	0(,%rax,8), %rdx
	movq	-32(%rbp), %rax
	addq	%rdx, %rax
	movq	(%rax), %rdx
	movl	-16(%rbp), %eax
	movl	%eax, %esi
	leaq	.LC2(%rip), %rdi
	movl	$0, %eax
	call	printf@PLT
	addl	$1, -16(%rbp)
.L2:
	movl	-16(%rbp), %eax
	cmpl	-20(%rbp), %eax
	jl	.L3
	movl	$0, -4(%rbp)
	movl	$0, -12(%rbp)
	jmp	.L4
.L7:
	movl	-12(%rbp), %eax
	movl	%eax, -8(%rbp)
	jmp	.L5
.L6:
	movl	-8(%rbp), %eax
	addl	%eax, -4(%rbp)
	subl	$1, -8(%rbp)
.L5:
	cmpl	$0, -8(%rbp)
	jg	.L6
	addl	$1, -12(%rbp)
.L4:
	cmpl	$99999, -12(%rbp)
	jle	.L7
	movl	$0, %eax
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
.LFE0:
	.size	main, .-main
	.ident	"GCC: (Ubuntu 6.2.0-5ubuntu12) 6.2.0 20161005"
	.section	.note.GNU-stack,"",@progbits
