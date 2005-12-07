/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Функции межпроцессорного взаимодействия
 *
 *  Пока будет реализованы только Общие
 *  Переменные, которые можно будет использовать
 *  как семафоры, но которые не будут иметь
 *  соответствующих ограничений и не будут защищенными
 *
 */

#include <helloos/ipc.h>
#include <helloos/scrio.h>
#include <helloos/syscall.h>
#include <helloos/scheduler.h>
#include <config.h>
#include <string.h>



// Эта структура определяет Общую Переменную
typedef struct
{
   bool live;     // Создана
   char name[MAX_VAR_NAME]; // Имя, должно завершаться нулем
   uint value;    // Значение
} ComVar;

ComVar common_vars[CFG_MAX_COM_VARS];



void ipc_init()
{
   puts_color("Initializing IPC tools...\t", 0x0b);

   int i;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      common_vars[i].live = 0;

   printf_color(0x0a, "%d common vars zeroed\n", CFG_MAX_COM_VARS);
}


uint syscall_ipc_info()
{
   printf_color(0x06, "Common variables:\n");
   uint i, cnt = 0;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      if (common_vars[i].live)
      {
         printf_color(0x06, "\t\"%s\" = %d (0x%x)\n",
               common_vars[i].name,
               common_vars[i].value,
               common_vars[i].value);
         cnt++;
      }
   if (cnt == 0)
      printf_color(0x06, "\t<none>\n");
   printf_color(0x06, "%d of %d common vars used\n", cnt, CFG_MAX_COM_VARS);

   return 0;
}


// Системный вызов waitcomvar
// Приостанавливает процесс до тех пор, пока общая переменная
// name не примет значение val
// Возвращает 0 если переменная уже содержит это значение, и в этом
// случае остановка не производится
// Возвращает 1 в противном случае
uint syscall_waitcomvar(char *name, uint val)
{
   if (syscall_comvar_get(name) != val)
   {
      strncpy_from_user(Task[Current]->waitfor.comvar.name, name, MAX_VAR_NAME);
      Task[Current]->waitfor.comvar.value = val;
      Task[Current]->state = PS_WAITCOMVAR;
      CALL_SCHEDULER;
      return 1;
   }
   else
      return 0;
}


// Пробуждает процессы, ожидаюшщие данной переменное
void comvar_wakeup(ComVar *var)
{
   uint i;
   for (i = 0; i < NTasks; i++)
      if ((Task[i]->state == PS_WAITCOMVAR) &&
          (strncmp(Task[i]->waitfor.comvar.name, var->name, MAX_VAR_NAME) == 0) &&
          (Task[i]->waitfor.comvar.value == var->value))
         Task[i]->state = PS_RUNNING;
}


// Системный вызов comvar_init
// Создает общую переменную и устанавливает ее значение
// Если переменная уже существовала, ее значение не меняется
// Возвращает значение переменной
uint syscall_comvar_init(char *name, uint val)
{
   char locname[MAX_VAR_NAME];
   strncpy_from_user(locname, name, MAX_VAR_NAME);
   int i;
   int first_empty = -1;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      if (common_vars[i].live)
      {
         if (strcmp(locname, common_vars[i].name) == 0)
            return common_vars[i].value;
      }
      else
         first_empty = i;


   if (first_empty == -1)
   {
      printf_color(0x0c, "Too many common variables");
      scheduler_kill_current();
      return 0;
   }

   memcpy(common_vars[first_empty].name, locname, MAX_VAR_NAME);
   common_vars[first_empty].value = val;
   common_vars[first_empty].live = 1;
   comvar_wakeup(&common_vars[first_empty]);

   return val;
}

// Системный вызов comvar_get
// Возвращает значение переменной или 0, если ее не существует
uint syscall_comvar_get(char *name)
{
   char locname[MAX_VAR_NAME];
   strncpy_from_user(locname, name, MAX_VAR_NAME);
   int i;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      if (common_vars[i].live)
         if (strcmp(locname, common_vars[i].name) == 0)
            return common_vars[i].value;
   return 0;
}

// Системный вызов comvar_set
// Устанавливает значение общей переменной
// Если ее не было, переменная создается
// Возвращает 1 если переменная была создана и 0 если она уже существовала
uint syscall_comvar_set(char *name, uint val)
{
   char locname[MAX_VAR_NAME];
   strncpy_from_user(locname, name, MAX_VAR_NAME);
   int i;
   int first_empty = -1;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      if (common_vars[i].live)
      {
         if (strcmp(locname, common_vars[i].name) == 0)
         {
            common_vars[i].value = val;
            comvar_wakeup(&common_vars[i]);
            return 0;
         }
      }
      else
         first_empty = i;


   if (first_empty == -1)
   {
      printf_color(0x0c, "Too many common variables");
      scheduler_kill_current();
      return 0;
   }

   memcpy(common_vars[first_empty].name, locname, MAX_VAR_NAME);
   common_vars[first_empty].value = val;
   common_vars[first_empty].live = 1;
   comvar_wakeup(&common_vars[first_empty]);

   return 1;
}

// Системный вызов comvar_add
// Добавляет с значению общей переменной данное значение
// Если переменной не было, то она создается и ей присваивается данное
// значение
// Возвращает 1 если переменная была создана и 0 если она уже существовала
uint syscall_comvar_add(char *name, uint incr)
{
   char locname[MAX_VAR_NAME];
   strncpy_from_user(locname, name, MAX_VAR_NAME);
   int i;
   int first_empty = -1;
   for (i = 0; i < CFG_MAX_COM_VARS; i++)
      if (common_vars[i].live)
      {
         if (strcmp(locname, common_vars[i].name) == 0)
         {
            common_vars[i].value += incr;
            comvar_wakeup(&common_vars[i]);
            return 0;
         }
      }
      else
         first_empty = i;


   if (first_empty == -1)
   {
      printf_color(0x0c, "Too many common variables");
      scheduler_kill_current();
      return 0;
   }

   memcpy(common_vars[first_empty].name, locname, MAX_VAR_NAME);
   common_vars[first_empty].value = incr;
   common_vars[first_empty].live = 1;
   comvar_wakeup(&common_vars[first_empty]);

   return 1;
}
