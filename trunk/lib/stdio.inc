
// ���������� ������������ snprintf
// ������ �������������� �������: c, d, p, s, u, x
// ������ ���� � �������� �� ��������������
int vsnprintf(char *str, size_t maxlen, const char *format, va_list curp)
{
   /*�������� �� ����� �������*/
#define ifend {if(strpos>=(maxlen-1)){str[maxlen-1]='\0';return strpos;}}

   char c;              // ���,
   char *pc;            // �
   //double d;            // �����,
   int i, i1;               // �ӣ
   unsigned int ui, ui1;        // �������
   
   int num;          // ����������� ��� ����ޣ��
   unsigned int fl;        // ���������� ���� � �����
   int j;               // �ޣ����. ������� �����.
   int hexnum[]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
   
   unsigned int count=0;            // ������� ������� ������ � format
   unsigned int strpos=0;           // ������� ������� ������ � str  � ������������ ���������� ��� ���������� ��������
   
//   va_list curp;           // ����� ���������
//   va_start(curp,format);        // ��������������
   

   /* ���� ��ۣ� */
   while(format[count]!='\0' && strpos<(maxlen-1))
   {
/************************************************************************/
/**/     if(format[count]!='%')
      {
         ifend;
         str[strpos]=format[count];    // ����� ����� ������ �
         count++;          // �����������
         strpos++;            // �ޣ�����
         continue;
      }
      switch(format[count+1])
      {
/************************************************************************/
/**/        case '%':
            ifend;
            str[strpos]='%';        // ����� '%'
            count+=2;            // ������� �� ��������� �� ��� ������
            strpos++;            // ����������� �� 1 ������� ������
            break;
/************************************************************************/
/**/        case 'c':
            ifend;
            c=va_arg(curp, char);         // ������ ���
            str[strpos]=c;          // �����
            strpos++;            // ����������� �� 1 ������� ������
            count+=2;            // ������� �� ��������� �� ��� ������
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
//            while(i>=(fl*=10)) num++;     // ������� ���������� ����
//            fl/=10;              // � ���������� fl 10 � ������� num

            i1 = i;
            while (i1/10)
            { i1/=10; fl*=10; num++; }

            j=1;
            /*���� ���� ���� ������ � ��� ������*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(i/fl)+'0';    // ��������� int � char � ����� � str
               i%=fl;            // ����������� �� ��������� �����
               fl/=10;           // �������� ������� 10
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
            while(strpos<=(maxlen-2) && j>=0)            // �� 0 :(
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
            /*�������� pc[] � str[]*/
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
            while (ui1/10)                // ������� ���������� ����
            { ui1/=10; fl*=10; num++; }   // � ���������� fl 10 � ������� num-1

            j=1;
            /*���� ���� ���� ������ � ��� ������*/
            while(strpos<=(maxlen-2) && j<=num) 
            {
               str[strpos]=(ui/fl)+'0';   // ��������� int � char � ����� � str
               ui%=fl;           // ����������� �� ��������� �����
               fl/=10;           // �������� ������� 10
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
               str[strpos]='0';        // ���� 0, �� ������ ����� ���
               count+=2;
               strpos++;
               break;
            }
            else
            {
               j=28;
               while(strpos<=(maxlen-2) && j>=0)   // �� 0 :(
               {
                  if((ui>>j))      // ����� ������ ������� � �������� ţ
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




