	.data
data1:  .word   100
data2:  .word   200
        .text
main:
	la	$1, 0x1000
	la	$5, 0x1000
	subu	$4, $1, $4
	lw	$4, 0($1)
	sw	$4, 0($5)
	addiu	$6, $4, $5
	subu	$5, $4, $6
	lw	$5, 0($5)
	addiu	$6, $5, $2
