_TEXT SEGMENT 'CODE'

db  0h, 0h, 0h, 0h, 0h, 0h, 0h, 0h
db  0h, 0h, 0h, 0h, 0h, 0h, 0h, 0h
PUBLIC cloakTestFunction
cloakTestFunction:
xor eax,eax
nop
nop
nop

PUBLIC cloakTestFunctionInstruction
cloakTestFunctionInstruction:
mov eax,66666666h
nop
nop
nop
nop
ret      ;这里返回了
nop
nop
nop

PUBLIC cloakTestFunctionEnd
cloakTestFunctionEnd:

_TEXT   ENDS
        END	