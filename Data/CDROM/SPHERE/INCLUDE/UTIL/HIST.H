/************************************************************************/
/*    file: hist.h							*/
/*    Desc: The structure definition for the histogram structure	*/
/*    Date: Nov 27, 1990						*/
/*									*/
/************************************************************************/
#define HIST_H_VERSION "V1.0"

#define PEAK_WIDTH 3
#define PEAK 0
#define TROUGH 1

typedef struct hist{
       int count;
       float from, to;
     } HIST;

extern float do_least_squares();
extern float percentile_hist();
extern float percentage_left_hist();

#define head_insert_hist(_hist,_num_bins) { \
 register int _index; \
 register int _out_of_range=0; \
 register float _dist; \
 register float _from; \
   _from = _hist[0]->from; \
   _dist = _hist[_num_bins-1]->to - _hist[0]->from;

#define body_insert_hist(_hist,_num_bins,_value) \
   _index = (int)((float)_num_bins * (_value - _from) / _dist); \
   if ((_index>=0) && (_index<_num_bins)){ \
       _hist[_index]->count++; \
   } \
   else{ \
        _out_of_range++; \
   }

#define end_insert_hist(_hist,_num_bins) \
   if (_out_of_range > 0)   \
       printf("Hist Library: %d samples out of range (%4.2f,%4.2f)\n", \
                  _out_of_range,_from,_hist[_num_bins-1]->to); \
}




















