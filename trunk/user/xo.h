// массив известных позиций
int KnownPos[22][9]={{0,0,2,1,1,1,2,2,2},
                     {0,2,2,1,1,1,2,0,2},
                     {0,0,1,1,1,2,2,2,2},
                     {0,0,2,1,1,2,2,2,1},
                     {0,2,1,1,1,2,2,0,2},
                     {0,2,2,1,1,2,2,0,1},
                     {0,0,1,0,1,2,1,2,2},
                     {0,1,2,0,2,1,1,0,2},
                     {0,1,2,1,0,2,2,0,1},
                     {0,0,1,1,0,2,2,1,2},
                     {1,2,2,2,0,1,2,1,2},
                     {1,0,2,2,1,2,2,2,1},
                     {0,1,2,1,2,2,2,2,1},
                     {0,2,1,1,2,2,2,1,2},
                     {0,0,1,0,1,2,2,2,2},
                     {0,1,2,0,2,1,2,0,2},
                     {0,2,2,1,0,2,2,0,1},
                     {0,0,1,1,0,2,2,2,2},
                     {0,2,2,2,0,1,2,1,2},
                     {0,0,2,2,1,2,2,2,1},
                     {0,0,2,1,2,2,2,2,1},
                     {0,2,1,1,2,2,2,0,2}};

int FindZero(int *m)
{
   int i;
   for(i=0; i<3; i++)
      if(m[i]==0)
         return i;
   return -1;
}

/*******************************************************
Функция, определяющая возможность выигрыша или проигрыша
*******************************************************/

void opr(int mas[][3], int zn, int *k_str, int *k_stl)
{
   int  i,j,                // счётчики
        per,                // просто переменная
        x = 0,y = 0;                // координаты: x-номер столбца, y-номер строки
   bool flag;               // индикатор нахождения координат
     
   // поиск по горизонтали
     
   i=0;
   flag=false;
   while(flag==false && i<3)
   {
      j=0;
      while(flag==false && j<3)
      {
         per=(j==2)? 0:(j+1);
         if(mas[i][j]==zn && mas[i][per]==zn)
         {
            x=(per==2)? 0:(per+1);
            y=i;
            if(mas[y][x]==0) flag=true;
         }
         j++;
      }
      i++;
   }

   // поиск по вертикали
     
   j=0;
   while(flag==false && j<3)
   {
      i=0;
      while(flag==false && i<3)
      {
         per=(i==2)? 0:(i+1);
         if(mas[i][j]==zn && mas[per][j]==zn)
         {
            x=j;
            y=(per==2)? 0:(per+1);
            if(mas[y][x]==0) flag=true;
         }
         i++;
      }
      j++;
   }

   // прямая диагональ (слева направо)
     
   i=0;
   while(flag==false && i<3)
   {
      per=(i==2)? 0:(i+1);
      if(mas[i][i]==zn && mas[per][per]==zn)
      {
         x=(per==2)? 0:(per+1);
         y=x;
         if(mas[y][x]==0) flag=true;
      }
      i++;
   }

   // обратная диагональ (справа налево)
   
   if(flag==false && mas[0][2]==zn && mas[1][1]==zn && mas[2][0]==0)
   {
      x=0;
      y=2;
      flag=true;
   }
   if(flag==false && mas[0][2]==0 && mas[1][1]==zn && mas[2][0]==zn)
   {
      x=2;
      y=0;
      flag=true;
   }
   if(flag==false && mas[0][2]==zn && mas[1][1]==0 && mas[2][0]==zn)
   {
      x=1;
      y=1;
      flag=true;
   }

   // результат: номер строки и столбца, иначе -1 и -1

   if(flag==true)
   {
      *k_str=y;
      *k_stl=x;
   }
   else
   {
      *k_str=-1;
      *k_stl=-1;
   }
}

/**************************************
Функция, определяющая есть ли выигрыщ
возвращает: 0-нет, 1-нолики, 2-крестики
**************************************/

int Victory(int mas[][3])
{
   int  i,              // счётчик
        zn = 0;             // результат
   bool flag;           // индикатор нахождения строки
   
   // поиск по горизонтали
   
   i=0;
   flag=false;
   while(flag==false && i<3)
   {
      if(mas[i][0]==mas[i][1] && mas[i][0]==mas[i][2])
      {
         if(mas[i][0]!=0)
         {
            flag=true;
            zn=mas[i][0];
         }
      }
      i++;
   }
   
   // поиск по вертекали
   
   i=0;
   while(flag==false && i<3)
   {
      if(mas[0][i]==mas[1][i] && mas[0][i]==mas[2][i])
      {
         if(mas[0][i]!=0)
         {
            flag=true;
            zn=mas[0][i];
         }
      }
      i++;
   }
   
   // проверка по прямой диагонали
   
   if(flag==false && mas[0][0]==mas[1][1] && mas[0][0]==mas[2][2])
   {
      if(mas[0][0]!=0)
      {
         flag=true;
         zn=mas[0][0];
      }
   }
   
   // проверка обратной диагонали
   
   if(flag==false && mas[0][2]==mas[1][1] && mas[0][2]==mas[2][0])
   {
      if(mas[0][2]!=0)
      {
         flag=true;
         zn=mas[0][2];
      }
   }
   
   // результат
   
   if(flag==false) zn=0;
   return zn;
}

/*************************************************
Функция ищет позицию среди заранее известных
возвращает номер позиции в случае успеха, иначе -1
*************************************************/

int pos(int mas[][3])
{
   int  count,        // счётчик позиций
        res = 0;          // результат
   int  m[3][3];      // массив, представляющий нужную позицию
   bool flag;         // индикатор нужной позиции
   count=0;
   flag=false;
   while(flag==false && count<22)
   {
      int i, j;
      for(i=0;i<3;i++)
         for(j=0;j<3;j++)
            m[i][j]=0;
      for(i=0;i<8;i+=3)
         m[KnownPos[count][i]][KnownPos[count][i+1]]=KnownPos[count][i+2];
      flag=true;
      for(i=0;i<3;i++)
         for(j=0;j<3;j++)
            if(mas[i][j]!=m[i][j])
               flag=false;
      if(flag==true)
         res=count;
      count++;
   }
   if(flag==false)res=-1;
   return res;
}

/*****************************************************
Функция, которая выбирает ход, если не конец игры, нет
возможной победы или поражения, позиция неизвестна
возвращает координаты колетки, в случае ошибки -1,-1
*****************************************************/

void hod(int mas[][3], int *k_str, int *k_stl)
{
   int  i,j,             // счётчики
        x=0,y=0;             // координаты хода
   bool flag;            // индикатор нахождения координат
   int m[3];
   int c;
   i=0;
   flag=false;
   while(flag==false && i<3)
   {
      if(flag==false && (mas[i][0]+mas[i][1]+mas[i][2])==1)
      {
         flag=true;
         for(c=0; c<3; c++)
				m[c]=mas[i][c];
			x=i;
         y=FindZero(m);
      }
      if(flag==false && (mas[0][i]+mas[1][i]+mas[2][i])==1)
      {
         flag=true;
         for(c=0; c<3; c++)
				m[c]=mas[c][i];
         x=FindZero(m);
         y=i;
      }
      i++;
   }
   if(flag==false && (mas[0][0]+mas[1][1]+mas[2][2])==1)
   {
      flag=true;
		for(c=0; c<3; c++)
			m[c]=mas[c][c];
      x=FindZero(m);
      y=x;
   }

   if(flag==false && mas[0][2]==1 && mas[1][1]==0 && mas[2][0]==0)
   {
      flag=true;
      x=1;
      y=1;
   }
   if(flag==false && mas[0][2]==0 && mas[1][1]==1 && mas[2][0]==0)
   {
      flag=true;
      x=2;
      y=0;
   }
   if(flag==false && mas[0][2]==0 && mas[1][1]==0 && mas[2][0]==1)
   {
      flag=true;
      x=1;
      y=1;
   }
   if(flag==false)
   {
      i=0;
      while(flag==false && i<3)
      {
         j=0;
         while(flag==false && j<3)
         {
            if(mas[i][j]==0)
            {
               flag=true;
               x=j;
               y=i;
            }
            j++;
         }
         i++;
      }
   }
   
   // результат

   if(flag==true)
   {
      *k_str=y;
      *k_stl=x;
   }
   else
   {
      *k_str=-1;
      *k_stl=-1;
   }
}

/*************************************************
Функция выбирает продолжение для известной позиции
*************************************************/

void prod(int res, int *k_str, int *k_stl)
{
   int x,y;       // координаты хода
   switch(res)
   {
      case 0:
             x=1;
             y=0;
             break;
      case 1:
             x=0;
             y=1;
             break;
      case 2: case 3: case 6: case 12:
             x=0;
             y=2;
             break;
      case 4: case 5: case 10: case 13:
             x=0;
             y=0;
             break;
      case 7: case 8:
             x=2;
             y=2;
             break;
      case 9: case 11:
             x=2;
             y=0;
             break;
      case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21:
             x=1;
             y=1;
             break;
      default:
             x=-1;
             y=-1;
             break;
   }
   *k_str=y;
   *k_stl=x;
}
