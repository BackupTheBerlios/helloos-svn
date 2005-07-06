;
;-= HelloOS Educational Project =-
;-===============================-
;
;  $Id$
;
;  Код загрузочного сектора с загрузкой
;  ядра из файла HELLO на FAT12-дискете.
;
;  Этот загрузчик НЕ является универсальным!
;  Используются следующие предположения:
;   - Размер кластера = 1 сектор
;   - Размер сектора = 512 байт (а может и нет ;)
;   - На разделе FAT12
;  а также то, что на дискете нет bad cluster'ов ;)
;
;  Рекомендуемая процедура установки:
;   - Забить дискету нулями (dd if=/dev/zero of=/dev/fd0)
;   - Записать этот загрузочный сектор (dd if=boot.bin of=/dev/fd0)
;   - Скопировать на дискету ядро (cp hello /mnt/floppy)
;
;  Замечание:
;    По идее этот загрузочный сектор можно установить на уже
;    отформатированную дискету. Но если при этом поля BPB, прописанные
;    ниже не совпадут с теми, что были на дискете, то файловая система
;    на ней может быть повреждена. Впрочем, это маловероятно.
;
;
;  Описание файловой системы FAT12 см. в [4].
;


;
; Далее идет структура BPB для FAT-раздела. Имена полей сохранены так же,
; как и в [4].
;

jmp start         ; Прыжок на начало. В BPB специально
                  ; выделяется 3 байта для этого.
.byte 0x90        ; Но наш jmp занимает лишь 2 байта, поэтому добавляем
                  ; еще один. В [4] рекомендуется добавлять 0x90.

BS_OEMName: .ascii "MSWIN4.1" ; Для совместимости всегда MSWIN4.1

BPB_BytsPerSec:   .word 512   ; Количество байт на сектор

BPB_SecPerClus:   .byte 1     ; Количество секторов на кластер. Для дискет обычно 1.

BPB_RsvdSecCnt:   .word 1     ; Количество зарезервированных секторов
                              ; в начале диска, включая загрузочный. Не рекомендуется
                              ; ставить что-либо кроме 1.

BPB_NumFATs:      .byte 2     ; Количество FAT-таблиц. Не рекомендуется ставить что-либо
                              ; кроме 2.

BPB_RootEntCnt:   .word 224   ; Количество записей в корневом каталоге

BPB_TotSec16:     .word 2880  ; Количество секторов на диске. Для 1.44mb-дискет это 2880

BPB_Media:        .byte 0xf0  ; Тип носителя. Для съемных дисков используется 0xf0.

BPB_FATSz16:      .word 9     ; Размер FAT-таблицы в секторах. Для 1.44mb обычно 9.

BPB_SecPerTrk:    .word 18    ; Количество секторов на дорожке. Этот параметр определяется
                              ; лишь геометрией используемого диска. Для 1.44mb-дискет 18.

BPB_NumHeads:     .word 2     ; Количество головок на диске. Это тоже чисто-геометрический
                              ; параметр. Для дискет 2.

BPB_HiddSec:      .long 0     ; Количество скрытых секторов. Для неразбиваемых носителей, в
                              ; том числе дискет, должно быть равно 0.

BPB_TotSec32:     .long 0     ; 32-разрядное количество секторов на диске. Используется только
                              ; для дисков, на которых больше чем 65535 секторов.

BS_DrvNum:        .byte 0     ; Номер диска для использования прерыванием 0x13. Для дискеты - 0.

BS_Reserved1:     .byte 0     ; Зарезервировано

BS_BootSig:       .byte 0x29  ; Сигнатура, означающая, что следующие три поля присутствуют.

BS_VolID:         .long 0x422a271e  ; ID диска. Любое значение.

BS_VolLab:        .ascii "NO NAME    " ; Метка диска. NO NAME означает ее отсутствие. 

BS_FilSysType:    .ascii "FAT12   " ; Чисто информативное поле. Его значение не определяет тип ФС.



; Вычисление на основе полей BPB.

; Размер корневого каталога.
; RootSize = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec,
; то есть ceil((BPB_RootEntCnt * 32.0) / BPB_BytsPerSec)
RootSize = 14

; Сектор, с которого начинается корневой каталог
; RootStart = BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz16)
RootStart = 19




; Несколько используемых адресов
; Заметьте, что это номера сегментов. Чтобы получить
; из них физические адреса нужно их умножить на 0x10
BOOTSEG        = 0x07c0 ; В этот сегмент BIOS копирует наш бутсектор
INITSEG        = 0x9000 ; Сюда мы первым делом перемещаемся
SYSSEG         = 0x0100 ; Сюда считываем ядро
ROOTSEG        = 0x5000 ; Сюда будем считаем корневой каталог с диска
FATSEG         = 0x4000 ; Сюда будем считаем FAT-таблицу


entry start
start:                  ; Начало!
   mov ax, #BOOTSEG     ;
   mov ds,ax            ;
   mov ax,#INITSEG      ;
   mov es,ax            ; Перемещаем сами себя в INITSEG. Зачем это
   mov cx,#256          ; надо я сам не понял, но все так делают и
   sub si,si            ; без этого вроде бы не работает
   sub di,di            ;
   rep                  ;
   movw                 ;
   jmpi go,INITSEG      ; Прыгаем на метку go в копии


go:
; Заполняем сегменты. Все они становятся равными INITSEG
   mov ax,cs
   mov ds,ax
   mov ss,ax
   mov sp,#0x400        ; А здесь у нас будет стек

; Скажем всем, что мы живы
   mov cx, #boot_msg_len
   mov bp, #boot_msg
   call write_message


; Сбрасываем дисковод
recalibrate:
   mov ah, #0
   mov dl, #0
   int 0x13
   jc recalibrate

   ; Теперь начнем искать наше ядро
   ; Для начала загрузим корневой каталог

   mov ax, #RootStart
   mov cx, #RootSize

   mov bx, #ROOTSEG
   mov es, bx
   xor bx, bx

   ; Считываем корневой каталог
   call ReadSectors

   ; Ok, теперь найдем там нужный файл

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


NotFound:      ; Файл с ядром не найден
   mov ax, cs
   mov es, ax
   mov bp, #not_found_msg     ; Пожалуемся пользователю и повесимся
   mov cx, #not_found_msg_len
   call write_message
   jmp inf



Found:   ; Ура! Мы нашли ядро!
   ; Запомним в dx номер первого кластера ядра
   seg es
   mov dx, (bx+26)
   
   ; Теперь загрузим целиком FAT-таблицу
   ; Она начинается со сектора RsvdSecCnt и имеет размер FATSz16

   mov ax, #FATSEG
   mov es, ax
   xor bx, bx
   mov ax, [BPB_RsvdSecCnt]
   mov cx, [BPB_FATSz16]
   call ReadSectors

   mov ax, #FATSEG   ; запомним сегмент с FAT'ом в fs
   mov fs, ax


   ; А теперь наконец-то будем грузить наше ядро
   ; es:bx - сюда будем считывать кластеры
   ; dx - номер кластера
   mov ax, #SYSSEG
   mov es, ax
   xor bx, bx

ReadKernel:

   cmp dx, #0x0fff   ; Пока dx!=EOC, то есть пока не закончилась цепочка кластеров
   je next_work


   ; переводим номер кластера в номер соотв. сектора
   ; ax = (dx - 2) * BPB_SecPerClus + BPB_RsvdSecCnt + (BPB_FATSz16 * BPB_NumFATs) + Кол-во секторов под корневой каталог
   ; ax = (dx - 2) * 1 + RootStart + RootSize
   mov ax, dx
   sub ax, #2
   add ax, #RootStart
   add ax, #RootSize
   xor cx, cx
   mov cl, [BPB_SecPerClus]

   ; считываем очередной кластер
   call ReadSectors


   ; Ищем в FAT номер следующего кластера
   push bx
   mov bx, dx     ; Умножение
   shr bx, #1     ; на
   add bx, dx     ; 1.5

   ; Извлекаем из FAT номер следующего кластера
   mov cx, dx     ; Запоминаем предыдущий
   seg fs         ;
   mov dx, (bx)   ; Берем следующий. Т.к. в FAT12 элементы FAT занимают 1.5 байта,
   pop bx         ; то нужно проделать следующую операцию: если индекс четный взять
   test cx, #1    ; все кроме старших четырех бит, а если нечетный, то все кроме
   jz Even        ; младших четырех бит. Подробнее см. в [4].
   shr dx, #4
   jmp ReadKernel
Even:
   and dx, #0x0fff
   jmp ReadKernel




; Петля. На ней от тоски вешаются другие участки кода.
inf:        ; Для того чтобы не делать несколько петель, нужно расположить ее приблизительно
   jmp inf  ; в середине сектора, т.к. "ближние" прыжки не длиннее 128 байт


; Ядро загружено. Начинаем подготовку к переходу на него.
next_work:
   mov dx, #0x3f2          ;
   mov al, [BS_DrvNum]     ; Глушим дисковод
   out dx, al              ;

   mov ax, #INITSEG        ; Для write_string нужно, чтобы es=ds
   mov es, ax              ;

   mov bp, #load_msg       ; 
   mov cx, #load_msg_len   ; Рапортуем о том, что ядро считано
   call write_message      ;

   in al, #0x92            ; Включаем A20. Без этого работают только
   or al, #2               ; 20-битовые адреса, то есть 0x100000 = 0x0000
   out #0x92, al           ;


protected_mode:
   mov bp, #loadp_msg      ;
   mov cx, #loadp_msg_len  ; Прощаемся с реальным режимом
   call write_message      ;

   mov ah, #0x03           ;
   xor bh, bh              ; Считываем координаты курсора и прячем
   int #0x10               ; подальше ([0x8000]). Ядро потом заберет их
   mov 0x8000,dx           ; оттуда, дабы не выводить текст где попало.


   cli                     ; Не дышать!

   lgdt GDT_DESCRIPTOR     ; Загружаем дескрипторы сегментов

   mov ax, #1              ; Да здравствует
   lmsw ax                 ; защищенный режим!

; Прыгаем на ядро. Сегмент 0x08 означает сегмент 1 в глобальной таблице
; с уровнем привелегий 0. Все сегменты у нас начинаются с 0x0000, поэтому
; смещение 0x1000.

   jmpi 0x1000, 0x08





; Глобальная таблица дескрипторов сегментов
.align 2
GDT_DESCRIPTOR:
   .word 3 * 8 - 1      ; размер -1
   .word GDT,0x9        ; Адрес таблицы (абсолютный)

; Собственно таблица:
.align 2
GDT:
   .long 0, 0           ; Нулевой сегмент всегда должен быть нулевым

; Два сегмента по 4гб, начинающиеся с 0x0000
   .word 0xffff, 0      ; Первый сегмент - для кода
   .byte 0, 0x9a, 0xcf, 0

   .word 0xffff, 0      ; Второй - для данных
   .byte 0, 0x92, 0xcf, 0




; Далее идут функции

;
; ReadSectors
; Читает CX секторов, начиная с номера AX, записывая их по адресу ES:BX
;

ReadSectors:

ReadLoop:
   call ReadSector
   inc ax
   add bx, [BPB_BytsPerSec]
   loop ReadLoop
   ret



; Сюда передается управление при ошибке чтения
; в функции ReadSector.
bad_read:
   mov ax, cs
   mov es, ax
   mov bp, #error_read_msg       ;
   mov cx, #error_read_msg_len   ; Просто выдаем сообщение об ошибке
   call write_message            ; и вешаемся
inf1:
   jmp inf1


; Считывает сектор номер AX в ES:BX
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





; Функция выводит строку на экран.
; ES:BP - адрес строки
; CX - длина строки
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




; Крайне полезная отладочная функция
; Выводит байт AL в hex-виде и пробел.
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
;; Эта таблица используется для преобразования байта в hex-вид.
;hex_table:
;   .ascii "0123456789ABCDEF"
;
;space:
;   .ascii " "



; Имя файла с ядром в виде 8.3
filename:
   .ascii "HELLO      "


; Дальше идут сообщения
boot_msg:
   .byte 13,10
   .ascii "Loading"     ; О начале загрузки
boot_msg_len = 9

not_found_msg:
   .ascii "not found"   ; О том, что файл с ядром не найден
not_found_msg_len = 9

error_read_msg:
   .ascii "error"       ; Об ошибке чтения с диска
error_read_msg_len = 5

load_msg:
   .ascii "done"        ; О завершении загрузки ядра
   .byte 13,10
load_msg_len = 6

loadp_msg:
   .ascii "Protecting... " ; О подготовке к переходу в защищенный режим
loadp_msg_len = 14



; FAT требует, чтобы в загрузочном секторе адресу 510 распологалась
; сигнатура 0xAA55. Для нас это еще и служит якорем, чтобы размер
; бинарного файла с загрузчиком был ровно 512 байт.
.org 510
.word 0xAA55
