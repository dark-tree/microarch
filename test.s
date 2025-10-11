
set $1, 10
set $2, 1

test:
	nop
	cmp $1, $2

z.jmp test
