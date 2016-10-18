#include <varargs.h>
#include <stdio.h>

char static_message_buffer[200];

char *rsprintf(va_alist)
va_dcl
{
    va_list args;
    char *format;

    va_start(args);

    format = va_arg(args,char *);
    /*    printf("rsprintf:  format: %s\n",format); */
    vsprintf(static_message_buffer,format,args);
    /*    printf("rsprintf:  message: %s\n",static_message_buffer);*/
    return(static_message_buffer);
}

