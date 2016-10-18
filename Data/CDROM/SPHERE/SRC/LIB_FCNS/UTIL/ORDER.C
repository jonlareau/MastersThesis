/**********************************************************************/
/*                                                                    */
/*             FILENAME:  order.c                                     */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  this file contains routines to sort and rank      */
/*                  various types of numeric arrays in INCREASING     */
/*                  of DECREASING order                               */
/*                                                                    */
/**********************************************************************/
#include <util/order.h>

/**********************************************************************/
/*   Sort and rank an integer array                                   */
/*        arr -> the integer array                                    */
/*        ptr_arr -> an integer array of indexes into arr.            */
/*                   using this array to index arr, sorts the arr     */
/*        rank_arr -> an array of mean ranks of the elements in arr   */
/**********************************************************************/
rank_int_arr(arr,num,ptr_arr,rank_arr,order)
int *arr, num, *ptr_arr, order;
float *rank_arr;
{
    int i, j, tmp, count;

    /* init the indexes to 1-n */ 
    for (j=0;j<num;j++)
        ptr_arr[j] = j;
    if (order == DECREASING){
        /*  sort arr[ptr_arr[]] into descending order */
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] < arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
    else{
        /*  sort arr[ptr_arr[]] into ascending order */
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] > arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
    /* mean rank the short arr                                 */
    /*    . . . the average of the ranks for duplicate numbers */
    for (j=0;j<num;j++){
        /*  if the next number is = the the current number */
        if ((j<num-1) && (arr[ptr_arr[j]] == arr[ptr_arr[j+1]])){
            /* find the last duplicate number */
            count = j;
            while((count < num-1) &&
                  (arr[ptr_arr[count]] == arr[ptr_arr[count+1]])){
                count++;
	    }
            /* count the number of duplicates */
            tmp = 0;
            for (i=j;i<count+1;i++)
                tmp+=i;
            /* install the averages in all duplicates */
            for (i=j;i<count+1;i++)
                rank_arr[ptr_arr[i]] = (float)tmp/(float)(count+1-j) + 1.0;
            /* go the the end of the duplicates */
            j=i-1;
	}
        else{
            /* no duplicates, use the index as the rank */
            rank_arr[ptr_arr[j]] = (float)(j+1);
	}
    }
}

/**********************************************************************/
/*   Sort a short array                                               */
/*        arr -> the short array                                      */
/*        ptr_arr -> an integer array of indexes into arr.            */
/*                   using this array to index arr, sorts the arr     */
/**********************************************************************/
sort_short_arr(arr,num,ptr_arr,order)
short *arr;
int num, *ptr_arr, order;
{
    int i, j, tmp, count, dbg=0;

    if (dbg) {
        printf("sort_short_array   %d elements\n",num);
        for (j=0;j<num;j++)
            printf("   %d\n",arr[j]);
    }
    /*  sort arr[ptr_arr[]] into ascending order */
    for (j=0;j<num;j++)
        ptr_arr[j] = j;
    if (order == DECREASING){
        for (j=num;j>0;j--){
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] < arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
/*            for (i=0;i<num;i++)
                printf("%2d ",arr[ptr_arr[i]]);
            printf("\n");*/
	}
    }
    else{
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] > arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
}

/**********************************************************************/
/*   Sort a int array                                                 */
/*        arr -> the int array                                        */
/*        ptr_arr -> an integer array of indexes into arr.            */
/*                   using this array to index arr, sorts the arr     */
/**********************************************************************/
sort_int_arr(arr,num,ptr_arr,order)
int *arr;
int num, *ptr_arr, order;
{
    int i, j, tmp, count;

    /*  sort arr[ptr_arr[]] into ascending order */
    for (j=0;j<num;j++)
        ptr_arr[j] = j;
    if (order == DECREASING){
        for (j=num;j>0;j--){
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] < arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
/*            for (i=0;i<num;i++)
                printf("%2d ",arr[ptr_arr[i]]);
            printf("\n");*/
	}
    }
    else{
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] > arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
}

/**********************************************************************/
/*   Sort an array strings in place                                   */
/*        arr -> the 2 dimarr of chars                                */
/**********************************************************************/
sort_strings_in_place(arr,num,order)
char **arr;
int num, order;
{
    int i, j, count;
    char *tmp;

    if (order == DECREASING){
        for (j=num;j>0;j--){
            for (i=0;i<j-1;i++)
                if (strcmp(arr[i],arr[i+1]) < 0){
                    tmp = arr[i];
                    arr[i] = arr[i+1];
                    arr[i+1] = tmp;
	        }
	}
    }
    else{
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (strcmp(arr[i],arr[i+1]) > 0){
                    tmp = arr[i];
                    arr[i] = arr[i+1];
                    arr[i+1] = tmp;
	        }
    }
/*            for (i=0;i<num;i++)
                printf("%s ",arr[i]);
            printf("\n");*/
}


/**********************************************************************/
/*   Sort and rank an float array                                     */
/*        arr -> the float array                                      */
/*        ptr_arr -> an integer array of indexes into arr.            */
/*                   using this array to index arr, sorts the arr     */
/*        rank_arr -> an array of mean ranks of the elements in arr   */
/*                                                                    */
/*  *** for comments, look up at sort_int_arr                         */
/*                                                                    */
/**********************************************************************/
rank_float_arr(arr,num,ptr_arr,rank_arr,order)
float *arr;
int num, *ptr_arr, order;
float *rank_arr;
{
    int i, j, tmp, count;

    /*  sort arr[ptr_arr[]] into acsending order */
    for (j=0;j<num;j++)
        ptr_arr[j] = j;
    if (order == DECREASING){
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] < arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
    else{
        for (j=num;j>0;j--)
            for (i=0;i<j-1;i++)
                if (arr[ptr_arr[i]] > arr[ptr_arr[i+1]]){
                    tmp = ptr_arr[i];
                    ptr_arr[i] = ptr_arr[i+1];
                    ptr_arr[i+1] = tmp;
	        }
    }
    for (j=0;j<num;j++){
        if ((j<num-1) && (arr[ptr_arr[j]] == arr[ptr_arr[j+1]])){
            count = j;
            while((count < num-1) &&
                  (arr[ptr_arr[count]] == arr[ptr_arr[count+1]])){
                count++;
	    }
            tmp = 0;
            for (i=j;i<count+1;i++)
                tmp+=i;
            for (i=j;i<count+1;i++)
                rank_arr[ptr_arr[i]] = (float)tmp/(float)(count+1-j) + 1.0;
            j=i-1;
	}
        else{
            rank_arr[ptr_arr[j]] = (float)(j+1);
	}
    }
}


