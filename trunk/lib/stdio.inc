
// Реализация стандартного snprintf
// Сейчас поддерживаются форматы: c, d, p, s, u, x
// Ширина поля и точность НЕ поддерживаются
int vsnprintf(char *str, size_t maxlen, const char *format, va_list curp)
{
   /*Проверка на конец буффера*/
#define ifend {if(strpos>=(maxlen-1)){str[maxlen-1]='\0';return strpos;}}

   char c;              // тут,
   char *pc;            // я
   //double d;            // думаю,
   int i, i1;               // всё
   unsigned int ui, ui1;        // понятно
   
   int num;          // понадобятся для подсчёта
   unsigned int fl;        // количества цифр в числе
   int j;               // счётчик. юзается часто.
   int hexnum[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
   
   unsigned int count=0;            // текущая позиция чтения в format
   unsigned int strpos=0;           // текущая позиция записи в str  и одновременно количество уже записанных символов
   
//   va_list curp;           // читай Кернигана
//   va_start(curp,format);        // инициализируем
   

   /* Цикл пошёл */
   while(format[count]!='\0' && strpos<(maxlen-1))
   {
/************************************************************************/
/**/     if(format[count]!='%')
      {
         ifend;
         str[strpos]=format[count];    // иначе пишем символ и
         count++;          // увеличиваем
         strpos++;            // счётчики
         continue;
      }
      switch(format[count+1])
      {
/************************************************************************/
/**/        case '%':
            ifend;
            str[strpos]='%';        // пишем '%'
            count+=2;            // прыгаем на следующий за ним символ
            strpos++;            // увеличиваем на 1 позицию записи
            break;
/************************************************************************/
/**/        case 'c':
            ifend;
            c=va_arg(curp, char);         // читаем его
            str[strpos]=c;          // пишем
            strpos++;            // увеличиваем на 1 позицию записи
            count+=2;            // прыгаем на следующий за ним символ
            break;
/************************************************************************/
/**/        case 'd':
            ifend;
            i=va_arg(curp, int);
            if(i<0)
            {
               str[strpos]='-';
               strpos++;
               i=-i;
            }
            num=1;
            fl=1;
//            while(i>=(fl*=10)) num++;     // считаем количество цифр
//            fl/=10;              // в переменной fl 10 в степени num

            i1 = i;
            while (i1/10)
            { i1/=10; fl*=10; num++; }

            j=1;
            /*пока есть куда писать и что писать*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(i/fl)+'0';    // переводим int в char и пишем в str
               i%=fl;            // избавляемся от считанной цифры
               fl/=10;           // понижаем степень 10
               j++;
               strpos++;
            }
            count+=2;
            break;
/************************************************************************/
/*       case 'f':
            ifend;
            d=va_arg(curp, double);
*/
/************************************************************************/
         case 'p':
            ifend;
            ui=va_arg(curp, unsigned int);
            j=28;
            while(strpos<=(maxlen-2) && j>=0)            // не 0 :(
            {
               str[strpos]=hexnum[(ui>>j)&0xf];
               strpos++;
               j-=4;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 's':
            ifend;
            pc=va_arg(curp, char *);
            j=0;
            /*копируем pc[] в str[]*/
            while(strpos<=(maxlen-2) && pc[j]!=0)
            {
               str[strpos]=pc[j];
               strpos++;
               j++;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 'u':
            ifend;
            ui=va_arg(curp, unsigned int);
            num=1;
            fl=1;
//            while(ui>=(fl*=10)) num++;    
//            fl/=10;              

            ui1 = ui;
            while (ui1/10)                // считаем количество цифр
            { ui1/=10; fl*=10; num++; }   // в переменной fl 10 в степени num-1

            j=1;
            /*пока есть куда писать и что писать*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(ui/fl)+'0';   // переводим int в char и пишем в str
               ui%=fl;           // избавляемся от считанной цифры
               fl/=10;           // понижаем степень 10
               j++;
               strpos++;
            }
            count+=2;
            break;
/************************************************************************/
/**/        case 'x':
            ifend;
            ui=va_arg(curp, unsigned int);
            if(ui==0)
            {
               str[strpos]='0';        // если 0, то просто пишем его
               count+=2;
               strpos++;
               break;
            }
            else
            {
               j=28;
               while(strpos<=(maxlen-2) && j>=0)   // не 0 :(
               {
                  if((ui>>j))      // цифру делаем младшей и отделяем её
                  {
                     str[strpos]=hexnum[(ui>>j)&0xf];
                     strpos++;
                  }
                  j-=4;
               }
               count+=2;
               break;
            }
/************************************************************************/
         default:
            break;
      }
   }
   ifend;
   str[strpos]='\0';
   return strpos;
}

int snprintf(char *str, size_t maxlen, const char *format, ...)
{
   va_list curp;
   va_start(curp, format);
   uint res = vsnprintf(str, maxlen, format, curp);
   va_end(curp);
   return res;
}




