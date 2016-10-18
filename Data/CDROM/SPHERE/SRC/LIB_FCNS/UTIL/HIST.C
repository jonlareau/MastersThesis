/************************************************************************/
/*    file: hist.c							*/
/*    Desc: General purpose histogram manipulating routines.		*/
/*    Date: Nov 27, 1990						*/
/*									*/
/************************************************************************/
/*
DOC:filename: hist.c
DOC:include:  ../include/hist.h
DOC:package:  numeric histogram package
DOC:purpose:  create, manipulate and de-create histograms
DOC:
*/
#include <stdio.h>
#include <math.h>
#include <util/hist.h>
#include <util/memory.h>
#include <util/macros.h>
#define HIST_C_VERSION "V1.0"

/***********************************************************/
/* DOC-P:init_hist()*/
/* DOC-PS:allocate and init a HIST structure*/
/* Based on the number of bins, and the range of values to */
/* be within the histograms values, build and return a     */
/* histogram.                                              */
init_hist(hist,num_bins,from,to)
HIST ***hist;
int num_bins;
float from, to;
{
    HIST **th;
    int i;
    double dist;

    /* what's the span of possible values */
    dist = (double) (to - from);

    alloc_2dimarr(th,num_bins,1,HIST);
    /* initialize the count, and set up the ranges */
    for (i=0; i<num_bins; i++){
        th[i]->count=0;
        th[i]->from=from+(dist*((double)i/(double)num_bins));
        th[i]->to=from+(dist*((double)(i+1)/(double)num_bins));
    }
    *hist=th;
}

/***********************************************************/
/* DOC-P: free_hist()       */
/* DOC-PS:Free the memory associated with a histogram      */
free_hist(hist,num_bins)
HIST ***hist;
int num_bins;
{
    HIST **th;
    int i;

    th = *hist;
    free_2dimarr(th,num_bins,HIST);
    *hist = (HIST **)0;
}

/************************************************************/
/* DOC-P:   diff_hist()       */
/* DOC-PS:   subtract the counts of hist 2 from hist 1. */
/* subtract the counts of hist 2 from hist 1.  This may     */
/* create negative count values                             */
diff_hist(h1,h2,hd,num_bins)
HIST **h1, **h2, **hd;
int num_bins;
{
    int i;

    for (i=0; i<num_bins; i++)
        hd[i]->count = h1[i]->count - h2[i]->count;
}

/************************************************************/
/* DOC-P:  subtract_hist()       */
/* DOC-PS: subtract h1 from h2, all negative values are set to zero */
subtract_hist(h1,h2,hs,num_bins)
HIST **h1, **h2, **hs;
int num_bins;
{
    int i;

    for (i=0; i<num_bins; i++){
        hs[i]->count = h2[i]->count - h1[i]->count;
        if (hs[i]->count < 0)
            hs[i]->count = 0;
    }
}

/************************************************************/
/* DOC-P:   float percentile_hist()       */
/* DOC-PS: return the mid-point of the bin with the Nth percentile */
/* return the mid-point of the histogram bin containing the */
/* percentile specified.                                    */
float percentile_hist(hist,num_bins,percentile)
HIST **hist;
int num_bins;
float percentile;
{
    int i, pct_area, area=0;

    pct_area = (int)((float)hist_area(hist,num_bins) * percentile);
   
    for (i=0; (i<num_bins) && (area < pct_area); i++){
        area+=hist[i]->count;
    }
    return(hist[i]->from+((hist[i]->to-hist[i]->from)/2.0));
}

/************************************************************/
/* DOC-P:   percentage_left_hist()       */
/* DOC-PS:  return the % of HIST to the left of the bin */
/* return the percentage of the histogram to the left of    */
/* specified bin value, including that bin                  */
float percentage_left_hist(hist,num_bins,value)
HIST **hist;
int num_bins;
float value;
{
    int i, pct_area, area=0, left_area=0;

    area = (float)hist_area(hist,num_bins);
   
    for (i=0; (i<num_bins) &&((hist[i]->to+hist[i]->from)/2.0) <= value;i++){
        left_area+=hist[i]->count;
    }

    return((float)left_area/(float)area*100.0);
}

/***************************************************************/
/* DOC-P:   float do_least_squares()                           */
/* DOC-PS:  returns the square-distance between two histograms */
/*          Used as a distance metric in the direct search.    */
/***************************************************************/

float do_least_squares(noise,normal,num_bins)
HIST **noise, **normal;
int num_bins;
{
    double sqr_sum=0.0, sqr, extend_db=5.0;
    int i, p_cnt=0,end=0;

    i=0;
    while ((i<num_bins) && (normal[i]->count <= 0))
        i++;
    end=i;
    if ((i-=num_bins)<0)
        i=0; 

    for (; end<num_bins && (normal[end]->count > 0); end++)
        ;
    if (end>=num_bins) end=num_bins-1;
    end += ((float)(float)num_bins/(normal[num_bins-1]->to-normal[0]->from)) * extend_db;
    if (end>=num_bins) end=num_bins-1;
    for (; i<end; i++){
        sqr      = (float)(noise[i]->count - normal[i]->count) *
                   (float)(noise[i]->count - normal[i]->count) ;
        if (noise[i]->count == 0)
            sqr_sum += sqr*sqr;
        else
            sqr_sum += sqr;
    }
    return(sqr_sum);
}


/*************************************************************/
/* DOC-P:   hist_copy()                                      */
/* DOC-PS:  copy one histogram to another.                   */
hist_copy(from,to,num_bins,start,end)
HIST **from, **to;
int num_bins, start, end;
{
    int i;
 
    for (i=start; (i<num_bins) && (i<=end); i++)
         to[i]->count = from[i]->count;
}

/*************************************************************/
/* DOC-P:   erase_hist()                                     */
/* DOC-PS:  reset all bin counts to zero.                    */
erase_hist(hist,num_bins)
HIST **hist;
int num_bins;
{
    int i;

    for (i=0; i<num_bins; i++)
        hist[i]->count = 0;
}

/**********************************************************************/
/* DOC-P:   dump_hist()                                               */
/* DOC-PS:  print the contents of the histogram to the file pointer   */
dump_hist(hist,num_bins,fp)
HIST **hist;
int num_bins;
FILE *fp;
{
    int i;

    fprintf(fp,"Dump of a histogram\n");
    for (i=0; i<num_bins; i++)
        fprintf(fp,"%5.3f : %5.3f -> %d\n",
                hist[i]->from,hist[i]->to,hist[i]->count);
}

/*************************************************************/
/* DOC-P:   dump_esps_hist()                                 */
/* DOC-PS:  dump a HIST in ESPS format for aplot             */
/* Print the histogram to the file "fname" in a format       */
/* readable by the entropic 'aplot' program                  */
dump_esps_hist(hist,num_bins,fname)
HIST **hist;
int num_bins;
char *fname;
{
    int i,height;
    FILE *fp;

/*  printf("Dump of a histogram in ESPS format to %s\n",fname); */
    if ((fp=fopen(fname,"w")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for writing\n",fname);
        return;
    }

    fprintf(fp,"%d\n",num_bins*3); /*each bin has three points */
    fprintf(fp,"1\n");             /*I'm not sure why */
    /* the X axis - beg, end, increment*/
    fprintf(fp,"%5.1f %5.1f 10.0\n",hist[0]->from,hist[num_bins-1]->to);
     /* the Y axis - beg, end, increment*/
    height= (int)(max_hist(hist,num_bins)+10)/10*10;
    fprintf(fp,"%d %d %d\n",0,height,height/10);

    for (i=0; i<num_bins; i++){
        fprintf(fp,"%5.1f %d\n", hist[i]->from,hist[i]->count);
        fprintf(fp,"%5.1f %d\n", hist[i]->to,hist[i]->count);
        fprintf(fp,"%5.1f 0\n", hist[i]->to);
    }
    fflush(fp);
    fclose(fp);
}

/**************************************************************/
/* DOC-P:   read_esps_hist()                                  */
/* DOC-PS:  read an ESPS format histogram                     */
/* Read an entropic histogram file, then create a histogram to*/
/* hold it's information and return that histogram            */
read_esps_hist(hist,num_bins,fname)
HIST ***hist;
int *num_bins;
char *fname;
{
    int i,height;
    FILE *fp;
    HIST **t_hist;
    char buff[400];
    float hist_beg, hist_end, hist_incr;

    printf("Reading of a histogram in ESPS format from %s\n",fname);
    if ((fp=fopen(fname,"r")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for reading\n",fname);
        return;
    }

    fscanf(fp,"%d\n",&i);
    *num_bins = i/3;
    printf("number of bins to read %d\n",*num_bins);

    fgets(buff,400,fp);  /* skip a line */
    
    fscanf(fp,"%f %f %f\n",&hist_beg, &hist_end, &hist_incr);
    printf("hist beg %f,  end %f  incr %f\n",hist_beg,hist_end,hist_incr);
    init_hist(&t_hist,*num_bins,hist_beg,hist_end);

    fgets(buff,400,fp); /* skip a line */

    for (i=0; i<*num_bins; i++){
        fscanf(fp,"%*f %d\n",&(t_hist[i]->count));
        fgets(buff,400,fp);  /* skip a line */
        fgets(buff,400,fp);  /* skip a line */
    }

    fclose(fp);
    *hist = t_hist;
}

/*************************************************************/
/* DOC-P:   dump_gnuplot_hist()                              */
/* DOC-PS:  dump a HIST in gnuplot format                    */
/* Write to the file 'fname' the histogram in a form usable  */
/* by gnuplot                                                */
dump_gnuplot_hist(hist,num_bins,fname)
HIST **hist;
int num_bins;
char *fname;
{
    int i,height;
    FILE *fp;

/*  printf("Dump of a histogram in GNUPLOT format to %s\n",fname); */
    if ((fp=fopen(fname,"w")) == NULL){
        fprintf(stderr,"Warning: unable to open %s for writing\n",fname);
        return;
    }

    for (i=0; i<num_bins; i++){
        fprintf(fp,"%5.3f %d\n", (hist[i]->to-hist[i]->from)/2.0+hist[i]->from,
                hist[i]->count);
    }

    fflush(fp);
    fclose(fp);
}

/*****************************************************************/
/* DOC-P:   half_cosine_hist()                                   */
/* DOC-PS:  store the values of cos(3pi/2) to cos(5PI/2) in hist */
half_cosine_hist(hist,num_bins,begin_bin,end_bin,height)
HIST **hist;
int num_bins, begin_bin, end_bin, height;
{
    int i;
    float factor, heightby2;

    factor = 1.0 / (float)(end_bin-begin_bin+1);
    heightby2 = (float)height / 2.0;
    for (i=begin_bin;(i<num_bins) && (i <= end_bin);i++){
        hist[i]->count = heightby2 * (1.0+cos((float)(i-begin_bin)*factor*M_PI+M_PI));
    }
}

/****************************************************************/
/* DOC-P:   full_cosine_hist()                                  */
/* DOC-PS:  store the values of cos(pi/2) to cos(5pi/2) in hist */
full_cosine_hist(hist,num_bins,begin_bin,end_bin,height)
HIST **hist;
int num_bins, begin_bin, end_bin, height;
{
    int i;
    float factor, heightby2;

    factor = 1.0 / (float)(end_bin-begin_bin+1);
    heightby2 = (float)height / 2.0;
    for (i=begin_bin;(i<num_bins) && (i <= end_bin+(end_bin-begin_bin));i++){
        hist[i]->count = heightby2 * (1.0+cos((float)(i-begin_bin)*factor*M_PI+M_PI));
    }
}

/*************************************************************/
/* DOC-P:   hist_area()                                      */
/* DOC-PS:  return the area of the HIST                      */
hist_area(hist,num_bins)
HIST **hist;
int num_bins;
{
    int i, sum=0;

    for (i=0; i<num_bins; i++)
        sum+=hist[i]->count;
    return(sum);
}

/*************************************************************/
/* DOC-P:   hist_character()                                 */
/* DOC-PS:  compute the mean, variance and area of a HIST    */
hist_character(hist,num_bins,mean,vari,area)
HIST **hist;
int num_bins, *area;
float *mean, *vari;
{
    int i;
    float sum=0.0, ave;

    *area = hist_area(hist,num_bins);
    for (i=0; i<num_bins; i++)
        sum+=(hist[i]->from+(hist[i]->to - hist[i]->from)/2.0) * hist[i]->count;
    *mean = sum/(float)*area;
    sum=0.0;
    for (i=0; i<num_bins; i++){
        if (hist[i]->count > 0){
            ave = hist[i]->from + (hist[i]->to - hist[i]->from)/2.0 ;
            sum+= (ave - *mean) * (ave - *mean) * hist[i]->count;
            printf(" var %f ave %f mean %f\n",sum,ave,*mean);
        }
    }
    *vari = sum / (float)*area;
}

/*************************************************************/
/* DOC-P:   max_hist()                                       */
/* DOC-PS:  return the maximum value in a HIST               */
max_hist(hist,num_bins)
HIST **hist;
int num_bins;
{
    int i, max=0;

    for (i=0; i<num_bins; i++)
        if (max<hist[i]->count)
            max=hist[i]->count;
    return(max);
}

/*********************************************************************/
/* DOC-P:   average_hieght_hist()                                    */
/* DOC-PS:  return tge average hieght in a the center of this window */
average_hieght_hist(hist,num_bins,center,window)
HIST **hist;
int num_bins, center, window;
{
    int i, sum=0;

    for (i=center-window; i<=center+window; i++)
        if ((i>=0) && (i<num_bins))
	  sum+=hist[i]->count;
    return(sum/(window*2+1));
}

/*************************************************************/
/* DOC-P:   smooth_hist()                                    */
/* DOC-PS:  compute the average hieghts of the histogram     */
smooth_hist(from,to,num_bins,window)
HIST **from, **to;
int num_bins, window;
{
    int i, value=0,window2=window*2;

    for (value=0,i=(-window); i<(num_bins+window); i++){
        if (i-window>=0)
           value -= from[i-window]->count;
        if (i+window<num_bins)
           value += from[i+window]->count;
        if ((i>=0) && (i<num_bins))
           to[i]->count = value/window2;
    }
}

int comp(a,b)
int *a,*b;
{
  return (*a-*b);
}

median_filter(h,out,num_bins,size)  /* size must be ODD */
HIST **h,**out;                     /* h and out may be the same */
int num_bins,size;
{
  int bin,*out_vals,*temp,i,half_size,index,median;
  int comp(); /* comparison function for sorting */
  
  half_size=size/2;

  out_vals = (int *) malloc (num_bins * sizeof (int));
  temp = (int *) malloc (size * sizeof (int));

  for (bin=0; bin <= num_bins; bin++) { /* loop through all the bins */

    /* update analysis window */
    for (i=0; i<size; i++) {
      index=bin-half_size+i;
      temp[i]=((index >= 0) && (index < num_bins)) ? h[index]->count : 0;
    }
      
    /* find median of window */
    qsort(temp,size,sizeof(int),comp);
    median = temp[half_size];

    out_vals[bin]=median;
  }

  /* write filtered values back into output histogram */
  for (i=0; i<num_bins; i++)
    out[i]->count = out_vals[i];

  free(out_vals);
  free(temp);
}

    
int locate_extremum(h,from,to,type)

HIST **h;
int from,to,type;

{
  int i,j,extremum,swing_loc,k,next_swing_loc,diff1,diff2,pre_swing,post_swing;

  for (i=from+PEAK_WIDTH; i<to-PEAK_WIDTH; i++) {
    if (h[i]->count==0) continue; /* not interested in extrema at 0 */
    extremum=1; /* assume it's an extremum to begin with */
    pre_swing=post_swing=0;
    swing_loc=i-PEAK_WIDTH;
    for (j=i-PEAK_WIDTH; j<i; j++) /* check the preceding samples */
      if (type==PEAK) {
	if (h[j]->count > h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  pre_swing=1;
	}
      }	else { /* type == TROUGH */
	if (h[j]->count < h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  pre_swing=1;
	}
      }

    if (!extremum) continue;

    for (j=i; j<i+PEAK_WIDTH; j++) /* check the subsequent samples */
      if (type==PEAK) {
	if (h[j]->count < h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  post_swing=1;
	}
      }	else { /* type == TROUGH */
	if (h[j]->count > h[j+1]->count) {
	  extremum=0;
	  break;
	}
	if (h[j]->count != h[j+1]->count) {
	  post_swing=1;
	}
      }

    /* check to make sure it isn't a step function */
    /* this kind of check is necessary if the peak is wider than the window */
    if (((pre_swing+post_swing)<=1)&&(extremum)) {
      for (k=i; k>from; k--)
	if ((diff1 = (h[k-1]->count - h[k]->count)) != 0)
	  break;
      swing_loc=k;
      for (k=i; (k<to-1); k++)  /* find next swing */
	if ((diff2 = (h[k]->count - h[k+1]->count)) != 0)
	  break;
      next_swing_loc=k;
      if ((type==PEAK)&&((diff1>0)||(diff2<0))) continue;   /* no dice */
      if ((type==TROUGH)&&((diff1<0)||(diff2>0))) continue; /* ditto   */

      /* otherwise, the peak is at the mid-point of this plateau */
      return (int) (swing_loc+next_swing_loc)/2;
    }

    if (extremum) return i;
  }

  return to;
}

/*************************************************************/
/* DOC-P:   build_normal_hist()                              */
/* DOC-PS:  store the the HIST a normal distribution         */
build_normal_hist(hist,num_bins,mean,variance,total_area)
HIST **hist;
int num_bins;
float mean, variance, total_area;
{
    int i;
    float x, value, con, e;

    printf("Building a histogram for a normal distribution\n");
    printf("       mean %f,  variance %f, area %f\n",mean,variance,total_area);

    con = (1.0 / sqrt(2.0*M_PI*variance));
    /* first generate a N(mean,variance) dist scaled to 10000*/
    for (i=0; i<num_bins; i++){
        x = hist[i]->from + (hist[i]->to - hist[i]->from)/2.0;
        e = exp(-( (x-mean)*(x-mean) / (2*variance)));
        value = 10000.0 * con * e;
        hist[i]->count = value;
    }
    /* now normalize it to the total_area */
    { float scale; int area;
      area = hist_area(hist,num_bins);
      scale = (float)total_area/(float)area;
      for (i=0; i<num_bins; i++)
          hist[i]->count *= scale;
    }
}

/*************************************************************/
/* DOC-P:   hist_slope()       */
/* DOC-PS:  return the slope of histogram the center  */
hist_slope(hist,num_bins,center,factor)
HIST **hist;
int num_bins,center,factor;
{
    int ind, cnt;

    for (ind=0,cnt=0; ind < factor; ind++)
        if (center-ind < 0) 
            cnt -= hist[center+ind]->count;
        else if (ind+center>=num_bins)
            cnt += hist[center-ind]->count;
        else
            cnt += hist[center-ind]->count - hist[center+ind]->count;
    return((int)-(((float)cnt/(float)factor)*1000.0));
}

/*************************************************************/
/* DOC-P:   do_hist()       */
/* DOC-PS:  using an input array, compute a historgram */
do_hist(hist,num_bins,arr,arr_cnt)
HIST **hist;
int num_bins, arr_cnt;
float *arr;
{
    int i, index, out_of_range=0;
    float dist, from;

    from = hist[0]->from;
    dist = hist[num_bins-1]->to - hist[0]->from;
    for (i=0; i<arr_cnt; i++){
        index = (int)((float)num_bins * (*(arr+i) - from) / dist);
        if ((index>=0) && (index<num_bins)) hist[index]->count++;
        else
            out_of_range++;
    }
}
