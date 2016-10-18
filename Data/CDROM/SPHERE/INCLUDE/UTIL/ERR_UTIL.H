
/*
 *
 * The Error return utility accessor
 *
 *
 */

#define RETURN_TYPE_ERROR    1
#define RETURN_TYPE_WARNING  2
#define RETURN_TYPE_SUCCESS  3
#define RETURN_TYPE_CHILD    4

extern int return_status();

#define return_err(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_ERROR) ; return(_value); }

#define return_success(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_SUCCESS) ; return(_value); }

#define return_warn(_proc,_code,_value,_mesg) \
        { set_return_util(_proc,_code,_mesg,RETURN_TYPE_WARNING) ; \
	  return(_value); }

#define return_child(_proc,_type, _func) \
        { _type _code; _code = (_type)_func; \
	  set_return_util(_proc,-1,"",RETURN_TYPE_CHILD) ; \
	  return(_code); }

