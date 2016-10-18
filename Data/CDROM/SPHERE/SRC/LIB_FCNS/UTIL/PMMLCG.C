/*
      name : pmmlcg.c
      by:    Jonathan G. Fiscus
      For:   605.412 Fall 1990

      This set of routines below implement a PMMLCG linear conguential random
      number generator given the following formula
               5                    31
      Z   =   7  *  Z          mod 2  -1
       i             (i-1)
*/
#include <stdio.h>
#include <math.h>
#include <util/pmmlcg.h>

static int random_seed = 1;
static int a=16807;
static int m=0x7fffffff;
static int q=127773;
static int r=2836;

init_seed(new_seed,verbose)
int new_seed,verbose;
{
    random_seed = new_seed;
    if (verbose){
        printf("PMMLCG Initializing\n\n");
        printf("PMMLCG Parameters:\n");
        printf("    seed: %d\n",random_seed);
        printf("       a: %d\n",a);
        printf("       m: %d (%x)\n",m,m);
        printf("       q: %d\n",q);
        printf("       r: %d\n\n",r);
    }
}

float pmmlcg()
{
    int lo, hi, test;

    hi = random_seed / q;
    lo = random_seed % q;
    test = (a*lo) - (r*hi);
    if (test > 0)
        random_seed = test;
    else
        random_seed = test + m;
    return((float)random_seed/(float)m);
}

pmmlcg_newseed()
{
    pmmlcg();
    return(random_seed);
}

pmmlcg_seed()
{
    return(random_seed);
}

float exp_dist(beta)
float beta;
{
    return(-(beta*log(pmmlcg())));
}

float M_erlang(beta,M)
float beta;
int M;
{
    float product=1.0;
    int i;

    for (i=0; i<M; i++)
        product *= pmmlcg();

    return(-(beta/(float)M) * log(product));
}

arbitrary_discrete_rv(prob_list,num_prob)
float prob_list[];
int num_prob;
{
    float value;
    int i;

    value = pmmlcg();
    for (i=0; i<num_prob; i++)
        if (value <= prob_list[i])
            return(i);
    return(num_prob-1);
}


