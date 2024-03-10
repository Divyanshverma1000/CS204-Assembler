.byte 34 35 23 24

#.asciiz "hello"

add x01 x0 x22

.text
for: add x10 x2 x12

mul x12 x10 x12

while:
sw x2 2( x10 )   #0x00A12123
addi x12 , x2 4
sw x10 2(x2)
bne x1 x2 for

bne x2 x1 for

.text
auipc x2 4

jal x3 4

jal x3 for
#.byte 2 3 4 5 6

.data 

hello:

 hi: .byte 2 ,3 ,4, 5 

.text
 add x2 x3 x4
 mul x3 x2 x5
 
 .data 
 #.word 2 3 3 4 
myarray: .byte 2,4 56 4 5 6 7 8 
