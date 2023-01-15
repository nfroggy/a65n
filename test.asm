	titl	"sneed"

foo	equ	$5
bar	equ	$6
	
	org	$8000
string
	db	"hello",0,1,2,3,-1,-2,-3
	db	"goodbye"
	dw	1,2,3,4,-1,-2,-3,-4
	jmp	string
	
start
	lda	#'a'
	sta	bar
	jmp	start
	

	end