    set $1, 240
    set $2, 1
    set $3, 0

l_2:
    add $3, $2
    cmp $1, $2
    ne.jmp l_2
