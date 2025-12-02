// 64 - address of the matrix
// 128 - address of the result (for optimization the result will be saved into memory backwards)

/*
Input matrix (hex):
01 02 03 04 05
02 02 03 04 05
03 03 03 04 05
04 04 04 04 05
05 05 05 05 05

Result (hex):
37 38 3B 41 4b
38 3a 3e 45 50
3b 3e 44 4d 5a
41 45 4d 59 69
4b 50 5a 69 7d

*/

// Store the input matrix into memory:

set $0, 64
set $2, 1
set $1, 1
stm $0, $1
add $0, $2
set $1, 2
stm $0, $1
add $0, $2
set $1, 3
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 2
stm $0, $1
add $0, $2
set $1, 2
stm $0, $1
add $0, $2
set $1, 3
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 3
stm $0, $1
add $0, $2
set $1, 3
stm $0, $1
add $0, $2
set $1, 3
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 4
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1
add $0, $2
set $1, 5
stm $0, $1

// Start multiplication of th 5x5 matrix by itself:

// Total number of elements to calculate in the result matrix
set $1, 25
// Current column
set $2, 0
// Number of individual cells multiplications to go before going to the next column
set $3, 25
// Row iterator
set $4, 64
// Column iterator
set $5, 64
// Accumulator
set $6, 0

ptl:

	// MULTIPLICATION HERE
	// We will add the multiplication result to accumulator (R6) immediately anyway and we multiply by adding, so we just add straight to R6

	// Saving this register in memory (because we need 1 register extra for multiplication)
	set $0, 5
	stm $0, $4
	// Saving this register in memory (because we need 1 register extra for multiplication)
	set $0, 6
	stm $0, $2

	// Loading 2 elements of the matrix from memory
	ldm $0, $4
	ldm $7, $5

	// Multiplication loop:
	mtpl:
		// If the current lowest bit is 1, we add
		set $4, 1
		and $4, $7
		set $2, 0
		cmp $2, $4
			ne.add $6, $0

		// Shift left
		add $0, $0
		// Shift right
		shr $7, 1
	//If the number we bitshift is equal to 0 we just end
	set $2, 0
	cmp $2, $7
	ne.jmp mtpl

	// Restoring previously saved register
	set $0, 5
    ldm $4, $0
    // Restoring previously saved register
   	set $0, 6
    ldm $2, $0

	// Adding 5 to the column iterator (moving down in the matrix)
    set $0, 5
	add $5, $0

	// If we are out of the matrix (24 elements + address) it means we went through the whole column, so we need to reset column iterator and save the result
	set $0, 89
	cmp $0, $5
	a.jmp skip_saving

		// Decrementing the number of elements left to calculate (we will aso use this number as array index for the output matrix)
		set $0, 1
        cmp $1, $0
        // Base address of the array for the result matrix
		set $0, 128
		stm $01, $6
		// If number of elements left to calculate is 0, then we break the loop
		z.jmp break


		// First element on the n-th column is also n-th element of the first row and the n-th element of the whole matrix, so we add it to the address of the whole matrix (we can use OR, since the address is a power of 2)
		set $5, 64
		mov $5, $25
		// Resetting the accumulator
		set $6, 0
	skip_saving:

	// We will be adding and subtracting 1 a few times, so we set it here
	set $0, 1

	// Adding 1 to the row iterator (moving right in the matrix)
	add $4, $0

	// We multiplied the whole matrix by the current column if this is 0, in such a case we need to move to the next column and reset back to the first row
	cmp $3, $0
	nz.jmp ptl

		// Resetting number of multiplications to run before switching columns to 25
		set $3, 25
		// Switching columns
	 	add $2, $0
	 	// First element on the n-th column is also n-th element of the first row and the n-th element of the whole matrix, so we add it to the address of the whole matrix (we can use OR, since the address is a power of 2)
	 	set $5, 64
	 	mov $5, $25
	 	// Starting with the first row, first element
	 	set $4, 64

jmp ptl

break:

