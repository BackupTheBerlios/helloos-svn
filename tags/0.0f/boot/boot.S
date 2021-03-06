;
;-= HelloOS Educational Project =-
;-===============================-
;
;  $Id$
;
;  ��� ������������ ������� � ���������
;  ���� �� ����� HELLO �� FAT12-�������.
;
;  ���� ��������� �� �������� �������������!
;  ������������ ��������� �������������:
;   - ������ �������� = 1 ������
;   - ������ ������� = 512 ���� (� ����� � ��� ;)
;   - �� ������� FAT12
;  � ����� ��, ��� �� ������� ��� bad cluster'�� ;)
;
;  ������������� ��������� ���������:
;   - ������ ������� ������ (dd if=/dev/zero of=/dev/fd0)
;   - �������� ���� ����������� ������ (dd if=boot.bin of=/dev/fd0)
;   - ����������� �� ������� ���� (cp hello /mnt/floppy)
;
;  ���������:
;    �� ���� ���� ����������� ������ ����� ���������� �� ���
;    ����������������� �������. �� ���� ��� ���� ���� BPB, �����������
;    ���� �� �������� � ����, ��� ���� �� �������, �� �������� �������
;    �� ��� ����� ���� ����������. �������, ��� ������������.
;
;
;  �������� �������� ������� FAT12 ��. � [4].
;


;
; ����� ���� ��������� BPB ��� FAT-�������. ����� ����� ��������� ��� ��,
; ��� � � [4].
;

jmp start         ; ������ �� ������. � BPB ����������
                  ; ���������� 3 ����� ��� �����.
.byte 0x90        ; �� ��� jmp �������� ���� 2 �����, ������� ���������
                  ; ��� ����. � [4] ������������� ��������� 0x90.

BS_OEMName: .ascii "MSWIN4.1" ; ��� ������������� ������ MSWIN4.1

BPB_BytsPerSec:   .word 512   ; ���������� ���� �� ������

BPB_SecPerClus:   .byte 1     ; ���������� �������� �� �������. ��� ������ ������ 1.

BPB_RsvdSecCnt:   .word 1     ; ���������� ����������������� ��������
                              ; � ������ �����, ������� �����������. �� �������������
                              ; ������� ���-���� ����� 1.

BPB_NumFATs:      .byte 2     ; ���������� FAT-������. �� ������������� ������� ���-����
                              ; ����� 2.

BPB_RootEntCnt:   .word 224   ; ���������� ������� � �������� ��������

BPB_TotSec16:     .word 2880  ; ���������� �������� �� �����. ��� 1.44mb-������ ��� 2880

BPB_Media:        .byte 0xf0  ; ��� ��������. ��� ������� ������ ������������ 0xf0.

BPB_FATSz16:      .word 9     ; ������ FAT-������� � ��������. ��� 1.44mb ������ 9.

BPB_SecPerTrk:    .word 18    ; ���������� �������� �� �������. ���� �������� ������������
                              ; ���� ���������� ������������� �����. ��� 1.44mb-������ 18.

BPB_NumHeads:     .word 2     ; ���������� ������� �� �����. ��� ���� �����-��������������
                              ; ��������. ��� ������ 2.

BPB_HiddSec:      .long 0     ; ���������� ������� ��������. ��� ������������� ���������, �
                              ; ��� ����� ������, ������ ���� ����� 0.

BPB_TotSec32:     .long 0     ; 32-��������� ���������� �������� �� �����. ������������ ������
                              ; ��� ������, �� ������� ������ ��� 65535 ��������.

BS_DrvNum:        .byte 0     ; ����� ����� ��� ������������� ����������� 0x13. ��� ������� - 0.

BS_Reserved1:     .byte 0     ; ���������������

BS_BootSig:       .byte 0x29  ; ���������, ����������, ��� ��������� ��� ���� ������������.

BS_VolID:         .long 0x422a271e  ; ID �����. ����� ��������.

BS_VolLab:        .ascii "NO NAME    " ; ����� �����. NO NAME �������� �� ����������. 

BS_FilSysType:    .ascii "FAT12   " ; ����� ������������� ����. ��� �������� �� ���������� ��� ��.



; ���������� �� ������ ����� BPB.

; ������ ��������� ��������.
; RootSize = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec,
; �� ���� ceil((BPB_RootEntCnt * 32.0) / BPB_BytsPerSec)
RootSize = 14

; ������, � �������� ���������� �������� �������
; RootStart = BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz16)
RootStart = 19




; ��������� ������������ �������
; ��������, ��� ��� ������ ���������. ����� ��������
; �� ��� ���������� ������ ����� �� �������� �� 0x10
BOOTSEG        = 0x07c0 ; � ���� ������� BIOS �������� ��� ���������
INITSEG        = 0x9000 ; ���� �� ������ ����� ������������
SYSSEG         = 0x0500 ; ���� ��������� ����
ROOTSEG        = 0x5000 ; ���� ����� ������� �������� ������� � �����
FATSEG         = 0x4000 ; ���� ����� ������� FAT-�������


entry start
start:                  ; ������!
   mov ax, #BOOTSEG     ;
   mov ds,ax            ;
   mov ax,#INITSEG      ;
   mov es,ax            ; ���������� ���� ���� � INITSEG. ����� ���
   mov cx,#256          ; ���� � ��� �� �����, �� ��� ��� ������ �
   sub si,si            ; ��� ����� ����� �� �� ��������
   sub di,di            ;
   rep                  ;
   movw                 ;
   jmpi go,INITSEG      ; ������� �� ����� go � �����


go:
; ��������� ��������. ��� ��� ���������� ������� INITSEG
   mov ax,cs
   mov ds,ax
   mov ss,ax
   mov sp,#0x400        ; � ����� � ��� ����� ����

; ������ ����, ��� �� ����
   mov cx, #boot_msg_len
   mov bp, #boot_msg
   call write_message


; ���������� ��������
recalibrate:
   mov ah, #0
   mov dl, #0
   int 0x13
   jc recalibrate

   ; ������ ������ ������ ���� ����
   ; ��� ������ �������� �������� �������

   mov ax, #RootStart
   mov cx, #RootSize

   mov bx, #ROOTSEG
   mov es, bx
   xor bx, bx

   ; ��������� �������� �������
   call ReadSectors

   ; Ok, ������ ������ ��� ������ ����

   mov ax, #ROOTSEG
   mov es, ax
   xor bx, bx

FindLoop:
   seg es
   cmp (bx), #0
   je NotFound

   mov si, #filename
   mov di, bx

   mov cx, #11
   repe
   cmpsb
   cmp cx, #0
   je Found

   add bx, #32

   jmp FindLoop


NotFound:      ; ���� � ����� �� ������
   mov ax, cs
   mov es, ax
   mov bp, #not_found_msg     ; ���������� ������������ � ���������
   mov cx, #not_found_msg_len
   call write_message
   jmp inf



Found:   ; ���! �� ����� ����!
   ; �������� � dx ����� ������� �������� ����
   seg es
   mov dx, (bx+26)
   
   ; ������ �������� ������� FAT-�������
   ; ��� ���������� �� ������� RsvdSecCnt � ����� ������ FATSz16

   mov ax, #FATSEG
   mov es, ax
   xor bx, bx
   mov ax, [BPB_RsvdSecCnt]
   mov cx, [BPB_FATSz16]
   call ReadSectors

   mov ax, #FATSEG   ; �������� ������� � FAT'�� � fs
   mov fs, ax


   ; � ������ �������-�� ����� ������� ���� ����
   ; es:bx - ���� ����� ��������� ��������
   ; dx - ����� ��������
   mov ax, #SYSSEG
   mov es, ax
   xor bx, bx

ReadKernel:

   cmp dx, #0x0fff   ; ���� dx!=EOC, �� ���� ���� �� ����������� ������� ���������
   je next_work


   ; ��������� ����� �������� � ����� �����. �������
   ; ax = (dx - 2) * BPB_SecPerClus + BPB_RsvdSecCnt + (BPB_FATSz16 * BPB_NumFATs) + ���-�� �������� ��� �������� �������
   ; ax = (dx - 2) * 1 + RootStart + RootSize
   mov ax, dx
   sub ax, #2
   add ax, #RootStart
   add ax, #RootSize
   xor cx, cx
   mov cl, [BPB_SecPerClus]

   ; ��������� ��������� �������
   call ReadSectors


   ; ���� � FAT ����� ���������� ��������
   push bx
   mov bx, dx     ; ���������
   shr bx, #1     ; ��
   add bx, dx     ; 1.5

   ; ��������� �� FAT ����� ���������� ��������
   mov cx, dx     ; ���������� ����������
   seg fs         ;
   mov dx, (bx)   ; ����� ���������. �.�. � FAT12 �������� FAT �������� 1.5 �����,
   pop bx         ; �� ����� ��������� ��������� ��������: ���� ������ ������ �����
   test cx, #1    ; ��� ����� ������� ������� ���, � ���� ��������, �� ��� �����
   jz Even        ; ������� ������� ���. ��������� ��. � [4].
   shr dx, #4
   jmp ReadKernel
Even:
   and dx, #0x0fff
   jmp ReadKernel




; �����. �� ��� �� ����� �������� ������ ������� ����.
inf:        ; ��� ���� ����� �� ������ ��������� ������, ����� ����������� �� ��������������
   jmp inf  ; � �������� �������, �.�. "�������" ������ �� ������� 128 ����


; ���� ���������. �������� ���������� � �������� �� ����.
next_work:
   mov dx, #0x3f2          ;
   mov al, [BS_DrvNum]     ; ������ ��������
   out dx, al              ;

   mov ax, #INITSEG        ; ��� write_string �����, ����� es=ds
   mov es, ax              ;

   mov bp, #load_msg       ; 
   mov cx, #load_msg_len   ; ��������� � ���, ��� ���� �������
   call write_message      ;

   in al, #0x92            ; �������� A20. ��� ����� �������� ������
   or al, #2               ; 20-������� ������, �� ���� 0x100000 = 0x0000
   out #0x92, al           ;


protected_mode:
   mov bp, #loadp_msg      ;
   mov cx, #loadp_msg_len  ; ��������� � �������� �������
   call write_message      ;

   mov ah, #0x03           ;
   xor bh, bh              ; ��������� ���������� ������� � ������
   int #0x10               ; �������� ([0x8000]). ���� ����� ������� ��
   mov 0x8000,dx           ; ������, ���� �� �������� ����� ��� ������.


   cli                     ; �� ������!

   lgdt GDT_DESCRIPTOR     ; ��������� ����������� ���������

   mov ax, #1              ; �� �����������
   lmsw ax                 ; ���������� �����!

; ������� �� ����. ������� 0x08 �������� ������� 1 � ���������� �������
; � ������� ���������� 0. ��� �������� � ��� ���������� � 0x0000, �������
; �������� 0x1000.

   jmpi 0x5000, 0x08





; ���������� ������� ������������ ���������
.align 2
GDT_DESCRIPTOR:
   .word 3 * 8 - 1      ; ������ -1
   .word GDT,0x9        ; ����� ������� (����������)

; ���������� �������:
.align 2
GDT:
   .long 0, 0           ; ������� ������� ������ ������ ���� �������

; ��� �������� �� 4��, ������������ � 0x0000
   .word 0xffff, 0      ; ������ ������� - ��� ����
   .byte 0, 0x9a, 0xcf, 0

   .word 0xffff, 0      ; ������ - ��� ������
   .byte 0, 0x92, 0xcf, 0




; ����� ���� �������

;
; ReadSectors
; ������ CX ��������, ������� � ������ AX, ��������� �� �� ������ ES:BX
;

ReadSectors:

ReadLoop:
   call ReadSector
   inc ax
   add bx, [BPB_BytsPerSec]
   loop ReadLoop
   ret



; ���� ���������� ���������� ��� ������ ������
; � ������� ReadSector.
bad_read:
   mov ax, cs
   mov es, ax
   mov bp, #error_read_msg       ;
   mov cx, #error_read_msg_len   ; ������ ������ ��������� �� ������
   call write_message            ; � ��������
inf1:
   jmp inf1


; ��������� ������ ����� AX � ES:BX
;

ReadSector:
   pusha
   push bx

   mov bx, [BPB_SecPerTrk]
   xor dx, dx
   div bx
   ; ax = LBA / SPT
   ; dx = LBA % SPT = sector - 1

   mov cx, dx
   inc cx   ; cx = sector

   xor dx, dx
   div [BPB_NumHeads]
   ; ax = (LBA / SPT) / HDC = cylinder
   ; dx = (LBA / SPT) % HDC = head

   mov ch, al
   ; ch = LSB 0..7 of cylinder no

   shl ah, #6
   or cl, ah
   ; cl = MSB 8..9 of cylinder no. + sector no.

   mov dh, dl
      ; dh = head no.

   mov dl, [BS_DrvNum]

   mov ax, #0x0201
   pop bx

   int #0x13
   jc bad_read

   mov ah, #0x0e
   mov al, #46
   mov bl, #0x07
   int 0x10

   popa
   ret





; ������� ������� ������ �� �����.
; ES:BP - ����� ������
; CX - ����� ������
write_message:
   pusha
   push cx
   mov ah,#0x03
   xor bh,bh
   int 0x10
   pop cx
   mov bx,#0x000e
   mov ax,#0x1301
   int 0x10
   popa
   ret




; ������ �������� ���������� �������
; ������� ���� AL � hex-���� � ������.
;print_byte:
;   pusha
;   push es
;   mov bx, cs
;   mov es, bx
;
;   mov bp, #space
;   mov cx, #1
;   call write_message
;
;   xor bx, bx
;   mov bl, al
;   shr bx, #4
;   mov bp, #hex_table
;   add bp, bx
;   mov cx, #1
;   call write_message
;
;   xor bx, bx
;   mov bl, al
;   and bl, #0x0f
;   mov bp, #hex_table
;   add bp, bx
;   mov cx, #1
;   call write_message
;
;   pop es
;   popa
;   ret
;
;; ��� ������� ������������ ��� �������������� ����� � hex-���.
;hex_table:
;   .ascii "0123456789ABCDEF"
;
;space:
;   .ascii " "



; ��� ����� � ����� � ���� 8.3
filename:
   .ascii "HELLO      "


; ������ ���� ���������
boot_msg:
   .byte 13,10
   .ascii "Loading"     ; � ������ ��������
boot_msg_len = 9

not_found_msg:
   .ascii "not found"   ; � ���, ��� ���� � ����� �� ������
not_found_msg_len = 9

error_read_msg:
   .ascii "error"       ; �� ������ ������ � �����
error_read_msg_len = 5

load_msg:
   .ascii "done"        ; � ���������� �������� ����
   .byte 13,10
load_msg_len = 6

loadp_msg:
   .ascii "Protecting... " ; � ���������� � �������� � ���������� �����
loadp_msg_len = 14



; FAT �������, ����� � ����������� ������� ������ 510 �������������
; ��������� 0xAA55. ��� ��� ��� ��� � ������ ������, ����� ������
; ��������� ����� � ����������� ��� ����� 512 ����.
.org 510
.word 0xAA55
