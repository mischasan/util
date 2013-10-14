strstr:	push	%ebp
	mov	%esp,%ebp
	mov	%ebx,-12(%ebp)       # save EBX
	mov	%esi,-8(%ebp)        # save ESI
	mov	8(%ebp),%esi
	mov	%edi,-4(%ebp)        # save EDI
	movzbl	(%eax),%edi          # eax: pat -- one register-parameter?
	call	0x480a1ca7 <test_pid+123607>    # Something that thunks strlen/strncmp ?
	add	$0x35aeb,%ebx
	mov	%edi,%edx
	test	%dl,%dl
	jne	WORK
DONE:	mov	%esi,%eax
	mov	-12(%ebp),%ebx
	mov	-8(%ebp),%esi
	mov	-4(%ebp),%edi
	mov	%ebp,%esp
	pop	%ebp
	ret

WORK:	add	$1,%eax
	mov	%eax,-16(%ebp)       # patleft
	mov	%eax,(%esp)
	call	strlen
	mov	%eax,-20(%ebp)       # lenleft
	lea	(%esi),%esi       # ???
LOOP:	movzbl	(%esi),%eax
	test	%al,%al
	je	FAIL
	mov	%edi,%edx
	add	$0x1,%esi
	cmp	%dl,%al
	jne	LOOP

	mov	-20(%ebp),%eax
	mov	-16(%ebp),%edx
	mov	%esi,(%esp)
	mov	%eax,0x8(%esp)
	mov	%edx,0x4(%esp)
	call	strncmp
	test	%eax,%eax
	jne	LOOP <strstr+80>
	sub	$1,%esi
FAIL:	xor	%esi,%esi
	jmp	DONE
