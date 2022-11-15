
#include <stdio.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include "uart.h"

int _read(int file, char *ptr, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        //ptr[i] = uart8250_getc();
    }

    return len;
}

int _write(int file, char *ptr, int len)
{
    int i;
    for (i = 0; i < len; ++i)
    {
        if (ptr[i] == '\n')
        {
            putchar('\r');
        }
        putchar(ptr[i]);
    }

    return len;
}

int _lseek(int file, int ptr, int dir)
{
    //errno = ESPIPE;
    return -1;
}

int _close(int file)
{
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int fd)
{
    //errno = ENOTTY;
    return 0;
}

void* _sbrk(int increment)
{
    extern char _end; // From linker script
    static char* heap_end = &_end;

    char* current_heap_end = heap_end;
    heap_end += increment;

    return current_heap_end;
}

void _exit(int return_value)
{
    asm ("fence rw, rw" ::: "memory");
    while (1)
    {
        asm ("wfi" ::: "memory");
    }
}

int _getpid(void)
{
  return 1;
}

int _kill(int pid, int sig)
{
    //errno = EINVAL;
    return -1;
}

void _init(){
    //uart_init(uart);
}
