/**********************************************************************/
/*                                                                    */
/*             FILENAME:  word.c                                      */
/*             BY:  Jonathan G. Fiscus                                */
/*                  NATIONAL INSTITUTE OF STANDARDS AND TECHNOLOGY    */
/*                  SPEECH RECOGNITION GROUP                          */
/*                                                                    */
/*           DESC:  This file contains general routines used          */
/*                  throughout the scoring package                    */
/*                                                                    */
/**********************************************************************/
#include <stdio.h>
#include <util/chars.h>
#define WORD_C_VERSION "V1.0"

/***************************************************************/
/*      General word manipulating procedures                   */
/*  definition: a word is a consecutive string of characters   */
/*              surrounded by a space, tab, or newline         */
/***************************************************************/
/* count the number of characters upto the first whitespace    */
/*  modified : aug 8, 1990                                     */
/*             the new_line was added as a delimiter           */
/***************************************************************/
wrdlen(ptr)
char *ptr;
{
    int i=0;
     
    while ((ptr[i] != SPACE) && (ptr[i] != NULL_CHAR) && (ptr[i] != NEWLINE))
        i++;
    return(i);
}

/***************************************************************/
/*  copy the characters upto the first whitespace in from      */
/*  modified : aug 8, 1990                                     */
/*             the new_line was added as a delimiter           */
/***************************************************************/
wrdcpy(to,from)
char *from, *to;
{
    while ((*from != SPACE) && (*from != NULL_CHAR) && 
           (*from != NEWLINE) && (*from != TAB))
        *to++ = *from++;
    *to = NULL_CHAR;
}

/***************************************************************/
/*  move the char pointer to the next word by skipping over the*/
/*  current word                                               */
/*  modified : aug 8, 1990                                     */
/*             the new_line was added as a delimiter           */
/***************************************************************/
find_next_word(ptr)
char **ptr;
{
    while ((**ptr != TAB) && (**ptr != SPACE) && (**ptr != NULL_CHAR)
           && (**ptr != NEWLINE))
        (*ptr)++;
    while ((**ptr == TAB) || (**ptr == SPACE) || (**ptr == NEWLINE))
        (*ptr)++;
}

