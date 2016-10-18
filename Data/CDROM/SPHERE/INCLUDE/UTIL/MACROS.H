/**********************************************************************/
/*                                                                    */
/*             FILENAME:  gen.h                                       */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general macros used            */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#define strip_newline(_str) \
      { register int _pos; \
        if (_str[_pos=(strlen(_str)-1)] == '\n') \
           _str[_pos] = (char)0; \
      } \

#define skip_white_space(_ptr) \
    while ((*_ptr == ' ') || (*_ptr == '\t')) \
        _ptr++; \

#define char2char(_ptr,_from,_to) \
    { char *_tp; _tp = _ptr; \
      while (*_tp != '\0') {\
         if (*_tp == _from) \
            *_tp = _to; \
         _tp++; \
      } \
    }

/***************************************************************/
/*  move the pointer to the end of the word                    */
/***************************************************************/
#define find_end_of_word(_ptr) \
    while ((*_ptr != ' ') && (*_ptr != '\t') && (*_ptr != (char)0) && \
           (*_ptr != '\n')) \
        _ptr++; 

#define pct(num,dem)	((dem==0) ? 0 : (float)(num)/(float)dem * 100.0)
#define ROUND(_x) ((int) ((_x) + 0.5))
#define INT(_x) ((int) floor((double) (_x)))
#define TRUNC(_x) (floor((double) (_x)))
#define MOD(_x,_y) ((int) ((_x) - TRUNC((float)(_x)/(float)(_y))*(_y)))
#define DIV(_x,_y) ((int) (_x)/(_y))

#define MAX(_a,_b) (((_a) > (_b)) ? (_a) : (_b))

#define swap_short(_p1,_p2) { \
		register short _temp; \
		_temp = _p1; \
		_p1  = _p2; \
		_p2  = _temp; \
		}

#define move_to_null(to) \
    while (*to != NULL_CHAR) \
        to++; 

#define form_feed()	printf("");

/* cause a core dump in a nice manner                          */
#define die() fflush(stderr); fflush(stdout); abort();

/***************************************************************/
/*  make a usable filename from a directory and a filename     */
/***************************************************************/
#define mk_fname(str,dir,name)	sprintf(str,"%s/%s",dir,name);

/***************************************************************/
/*    misc external function definitions                       */
/***************************************************************/
extern float sum_float_singarr();
extern char *center();
extern char *str2up();
extern char *str2low();
extern double seq_mult();
extern double compute_acc_binomial();
extern double n_CHOOSE_r();
extern char *safe_fgets();

/***************************************************************/
/*    Missing Library function Definitons                      */
/***************************************************************/
extern void *malloc();
