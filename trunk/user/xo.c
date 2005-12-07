/***************************************
  Крестики-Нолики
  (с) Денис Згурский, 2005 г.

  Порт под HelloOS
  Илья Скрибловский, 2005
 ***************************************/


#include <helloos/user_syscalls.h>
#include <helloos/io.h>


#include <stdio.h>
#include <string.h>

#define bool unsigned char
#define false 0
#define true 1

#define getch  sys_getch

#include "xo.h"



int curx = 0, cury = 0;
int Field[3][3]={{0, 0, 0},                                //   |
   {0, 0, 0},                                // <-|игровое поле с координатами и с X/O
   {0, 0, 0}};                               //   |
char setka[8][8]={{' ', ' ', '1', ' ', '2', ' ', '3', ' '},    //   |
   {' ', '-', '-', '-', '-', '-', '-', '-'},    //   |
   {'1', '|', ' ', '|', ' ', '|', ' ', '|'},    //   |
   {' ', '-', '-', '-', '-', '-', '-', '-'},    //   |
   {'2', '|', ' ', '|', ' ', '|', ' ', '|'},    // <-|игровое поле для вывода на экран
   {' ', '-', '-', '-', '-', '-', '-', '-'},    //   |
   {'3', '|', ' ', '|', ' ', '|', ' ', '|'},    //   |
   {' ', '-', '-', '-', '-', '-', '-', '-'}};   //   |


// Столько строк под заголовок
#define TOP_LINES    7
void draw_field()
{
   int i;
   for (i = 0; i < 8; i++)
   {
      int j;
      for (j = 0; j < 8; j++)
      {
         sys_setchar(j, TOP_LINES+i, setka[i][j]);
         sys_setattr(j, TOP_LINES+i, 0x0e);
      }
   }
   sys_setattr(2+2*curx, TOP_LINES+2+2*cury, 0x1e);
}


int main()
{
   bool fl_end			= false,							// индикатор конца игры
   fl_hod_gamer	= false,							// индикатор хода игрока
   fl_hod_comp		= false,							// индикатор хода компа
   fl_err			= false;							// индикатор ошибки
   int a=0,b=0/*,end*/,                                            // просто переменные
   Npos=-1,                                           // номер известной позиции
   win=0,                                              // победитель
   count=1,                                            // счётчик ходов
   gamerX=-1,                                         // столбик игрока
   gamerY=-1,                                         // строка игрока
   compX=-1,                                          // столбик компа
   compY=-1;                                          // строка компа

   sys_clear_screen();
   printf_color(0x0d, "X-O by Denis Zgursky, 2005\nHelloOS port by Ilya Skriblovsky\n\n");
   printf_color(0x0d, "Controls:\n  W\nA S D   Enter\n\n");

   while(fl_end==false && count<=5)
   {
      // выводим сетку
      draw_field();

      //      int i;
      //      for( i=0;i<8;i++)
      //      {
      //         int j;
      //         for( j=0;j<8;j++)
      //            printf("%c",setka[i][j]);
      //         printf("\n");
      //      }

      // "обнуляем" переменные

      fl_hod_gamer=false;
      fl_hod_comp=false;
      gamerX=-1;
      gamerY=-1;
      compX=-1;
      compY=-1;

      // ход игрока: заполняем Field и setka

      while(fl_hod_gamer==false)
      {
         while (1)
         {
            char k = sys_getch();
            if (k == 0x0d)
            {
               gamerY = cury;
               gamerX = curx;
               break;
            }
            switch (k)
            {
               case 'w': if (cury > 0) cury--; break;
               case 's': if (cury < 2) cury++; break;
               case 'a': if (curx > 0) curx--; break;
               case 'd': if (curx < 2) curx++; break;
            }
            draw_field();
         }


         if(Field[gamerY][gamerX]==0)
         {
            Field[gamerY][gamerX]=2;
            switch(gamerY)
            {
               case 0:
                  b=gamerY+2;
                  break;
               case 1:
                  b=gamerY+3;
                  break;
               case 2:
                  b=gamerY+4;
                  break;
            }
            switch(gamerX)
            {
               case 0:
                  a=gamerX+2;
                  break;
               case 1:
                  a=gamerX+3;
                  break;
               case 2:
                  a=gamerX+4;
                  break;
            }
            setka[b][a]='X';
            fl_hod_gamer=true;
         }
         //         }
   }

   // если пятый ход, то проверяем на возможность победы и заканчиваем

   if(count==5)
   {
      fl_end=true;
      fl_hod_comp=true;
      win=Victory(Field);
   }

   // анализ первого хода

   if(count==1)
   {
      if((gamerY==0 && gamerX==0) ||
            (gamerY==0 && gamerX==2) ||
            (gamerY==2 && gamerX==0) ||
            (gamerY==2 && gamerX==2))
      {
         compY=1;
         compX=1;
      }
      if((gamerY==0 && gamerX==1) ||
            (gamerY==2 && gamerX==1))
      {
         compY=gamerY;
         compX=gamerX-1;
      }
      if((gamerY==1 && gamerX==0) ||
            (gamerY==1 && gamerX==2))
      {
         compY=gamerY+1;
         compX=gamerX;
      }
      if(gamerY==1 && gamerX==1)
      {
         compY=0;
         compX=2;
      }
      fl_hod_comp=true;
   }

   // проверка на выигрыш/проигрыш

   if(fl_hod_comp==false)
   {
      win=Victory(Field);
      if(win==1 || win==2)
      {
         fl_hod_comp=true;
         fl_end=true;
      }
   }

   // проверка на возможность выигрыша

   if(fl_hod_comp==false)
   {
      opr(Field,1,&compY,&compX);
      if(compY!=-1 && compX!=-1) fl_hod_comp=true;
   }

   // прверка на возможность проигрыша

   if(fl_hod_comp==false)
   {
      opr(Field,2,&compY,&compX);
      if(compY!=-1 && compX!=-1) fl_hod_comp=true;
   }

   // прверка на схожесть с известной позицией

   if(fl_hod_comp==false)
   {
      Npos=pos(Field);
      if(Npos!=-1)
      {
         prod(Npos,&compY,&compX);
         fl_hod_comp=true;
      }
   }

   // просто ход

   if(fl_hod_comp==false)
   {
      hod(Field,&compY,&compX);
      fl_hod_comp=true;
   }

   // заносим запись в Field и в setka, иначе ошибка

   if(fl_hod_comp==true && fl_end==false && compY!=-1 && compX!=-1)
   {
      Field[compY][compX]=1;
      switch(compY)
      {
         case 0:
            b=compY+2;
            break;
         case 1:
            b=compY+3;
            break;
         case 2:
            b=compY+4;
            break;
      }
      switch(compX)
      {
         case 0:
            a=compX+2;
            break;
         case 1:
            a=compX+3;
            break;
         case 2:
            a=compX+4;
            break;
      }
      setka[b][a]='O';
   }
   else
   {
      if(fl_end==false)
      {
         fl_err=true;
         fl_end=true;
      }
   }

   // проверяем не победил ли комп после своего хода

   if(fl_err==false && fl_hod_comp==true)
   {
      win=Victory(Field);
      if(win==1 || win==2)
         fl_end=true;
   }
   count++;
}

// если нет ошибки то выводим результат

if(fl_err==false)
{
   int i;
   for( i=0;i<8;i++)
   {
      int j;
      for( j=0;j<8;j++)
         printf("%c",setka[i][j]);
      printf("\n");
   }
   switch(win)
   {
      case 0:
         printf("Drawn game\n");
         break;
      case 1:
         printf("I win! :)\n");
         break;
      case 2:
         printf("Your win! :(\n");
         break;
   }
}
else
{
   printf("Error!Sorry!");
}

return 0;
}
