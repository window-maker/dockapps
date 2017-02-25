;=========================================================================
;Trig functions for GFX routines   Copyright (c) 1995 by Mark I Manning IV
;=========================================================================

CosTab          equ SinTab+0800 ;A Right angle foreward in table!

;-------------------------------------------------------------------------

;Return Sin and Cos of angle AX in variables of same name

SinCos:
 and ax,0fff            ;4096 degrees = 0 degrees
 mov bx,ax              ;Put into an indexing reg
 add bx,bx
 mov ax,SinTab[bx]      ;Get Sin(N)
 mov Sin,ax             ;Bah no MOV Sin,[BX]... Crap processor!

;Note.
; Can NOT use mov ax,SinTab[2*ebx] because in following code BX may go
;  negative and 2* a negative number = DOUBLY NEGATIVE!

 cmp bx,01800           ;Was Sin(N) in last quadrant of circle?
 if nl sub bx,02000     ;Cos(N) must be in first quadrant then
 mov bx,CosTab[bx]      ;Get Cos(n)
 mov Cos,bx             ;Exits with Sin in AX and Cos in BX too!
 ret

;=========================================================================

