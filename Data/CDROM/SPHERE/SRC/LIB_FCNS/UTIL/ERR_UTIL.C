#include <stdio.h>
#include <util/err_util.h>
#include <string.h>

#define ERROR_UTIL_MESSAGE_LEN 200
static char static_error_util_proc_name[ERROR_UTIL_MESSAGE_LEN];
static char static_error_util_message[ERROR_UTIL_MESSAGE_LEN];
static char static_error_util_message1[ERROR_UTIL_MESSAGE_LEN];
static static_error_util_return_code;
static static_error_util_return_type;

#include <varargs.h>

static int err_util_dbg = 0;

void set_error_util_debug(n)
int n;
{
    err_util_dbg=n;
}

void new_set_return_util(va_alist)
va_dcl
{
    va_list args;
    char *proc_name, *format;
    int return_code;
    int return_type;
    va_start(args);

    proc_name = va_arg(args,char *);
    return_code = va_arg(args, int);
    return_type = va_arg(args, int);
    format = va_arg(args,char *);

    vsprintf(static_error_util_message,format,args);
    strncpy(static_error_util_proc_name,proc_name,ERROR_UTIL_MESSAGE_LEN);
    static_error_util_return_code = return_code;
    static_error_util_return_type = return_type;
}

void print_return_status(fp)
FILE *fp;
{  
    char *offset="";
    fprintf(fp,"Procedure: %s\n",static_error_util_proc_name);
    fprintf(fp,"%s   Status Code:     %d\n",offset,static_error_util_return_code);
    fprintf(fp,"%s   Status Type:     ",offset);
    switch(static_error_util_return_type){
      case RETURN_TYPE_WARNING: fprintf(fp,"Warning\n"); break;
      case RETURN_TYPE_SUCCESS: fprintf(fp,"Success\n"); break;
      case RETURN_TYPE_ERROR:   fprintf(fp,"Error\n"); break;
      default:  fprintf(fp,"UNKNOWN\n"); break;
    }
    fprintf(fp,"%s   Message:         %s\n",offset,static_error_util_message);
}

int return_status()
{  
    return(static_error_util_return_code);
}


void set_return_util(proc_name,return_code,mesg,type)
char *proc_name;
int return_code;
char *mesg;
int type;
{
    char buf[ERROR_UTIL_MESSAGE_LEN];

    if (type != RETURN_TYPE_CHILD){
	strncpy(static_error_util_proc_name,proc_name,ERROR_UTIL_MESSAGE_LEN);
	strncpy(static_error_util_message,mesg,ERROR_UTIL_MESSAGE_LEN);
	static_error_util_return_code = return_code;
	static_error_util_return_type = type;
    } else {
	sprintf(static_error_util_message1,"Child '%s' returns message '%s'",
		static_error_util_proc_name,static_error_util_message);
	strncpy(static_error_util_message,static_error_util_message1,ERROR_UTIL_MESSAGE_LEN);
	strncpy(static_error_util_proc_name,proc_name,ERROR_UTIL_MESSAGE_LEN);
    }
    
    if (err_util_dbg) {
	fprintf(stdout,"set_return_util: proc_name %s code %d message %s type %d\n",
		static_error_util_proc_name,
		static_error_util_return_code,
		static_error_util_message,
		static_error_util_return_type);
    }
}
    
