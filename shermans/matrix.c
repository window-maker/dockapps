#include <stdio.h>
#include <stdlib.h>


#include "draw.h"
#include "aquarium.h"
#include "over.h"

#include "matrix.h"

/*
  - Matrix scroller "plugin" for Sherman's aquarium -

  Yes, I know this is really out of the subject.
  But one afternoon I was bored and felt for doing 
  this and nothing else.
*/

#define LETTERS 28



static gboolean *pauselist = NULL;
static int *charnum = NULL;

static unsigned char **fallindata = NULL;


static int bright_x[8];
static int bright_y[8];
static int bright_num;
static int xrows, yrows;


static SA_Image matrix_dark, matrix_bright;


void make_matrix_line(int row, int start, int length)
{
    int i,k,j;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    for(i = start; i < (start + length); i++){
	j = g_rand_int_range(ad->rnd, 0, LETTERS+5);
	if(j < LETTERS){
	    j = g_rand_int_range(ad->rnd, 2, 14);
	    for( k = i; ((k < (i + j)) && (k < (start + length))); k++)
		fallindata[row][k] = g_rand_int_range(ad->rnd, 1, LETTERS);
	}
	else {
	    j = g_rand_int_range(ad->rnd, 10, 15);
	    for(k = i; ((k < (i + j)) && (k < (length + start))); k++)
		fallindata[row][k] = 0;
	} 
	i = k;
    }
}


void matrix_init(void)
{
  
    if(matrix_dark.pixbuf != NULL)
      g_object_unref(matrix_dark.pixbuf);

    if(matrix_bright.pixbuf != NULL)
      g_object_unref(matrix_dark.pixbuf);

    load_image("matrix1.png",
	       &matrix_dark,
	       LETTERS);

    load_image("matrix2.png",
	       &matrix_bright,
	       LETTERS);

}

void matrix_exit(void)
{

    matrix_end();

    if(matrix_dark.pixbuf != NULL)
	g_object_unref(matrix_dark.pixbuf);

    if(matrix_bright.pixbuf != NULL)
	g_object_unref(matrix_bright.pixbuf);

    matrix_dark.pixbuf = NULL;
    matrix_bright.pixbuf = NULL;


}

void matrix_start(void)
{
    int i;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    xrows = (ad->xmax / matrix_dark.width) + 2;
    yrows = (ad->ymax / matrix_dark.height) + 2;

    if(pauselist != NULL)
      g_free(pauselist);

    if(charnum != NULL)
      g_free(charnum);

    if(fallindata != NULL)
      g_free(fallindata);

    pauselist = g_malloc0(sizeof(gboolean) * xrows);
    charnum = g_malloc0(sizeof(int) * xrows);
    fallindata = g_malloc0(sizeof(char *) * xrows);


    bright_num = g_rand_int_range(ad->rnd, 0, 8);

    for(i = 0; i < bright_num; i++){
	bright_x[i] = g_rand_int_range(ad->rnd, 0, xrows);
	bright_y[i] = g_rand_int_range(ad->rnd, 0, yrows);
    }

    for(i = 0; i < xrows; i++){
	pauselist[i] = FALSE;
	fallindata[i] = g_malloc0(2 * yrows + 2);
	make_matrix_line(i, 0, yrows);
	make_matrix_line(i, yrows, yrows);
	charnum[i] = g_rand_int_range(ad->rnd, 0, 2 * yrows);
    }

}


void matrix_end(void)
{
    int i;

    if(fallindata != NULL){
	for(i = 0; i < xrows; i++){
	    if(fallindata[i] != NULL)
		g_free(fallindata[i]);
	}
	g_free(fallindata);
    }

    if(pauselist != NULL)
	g_free(pauselist);

    if(charnum != NULL)
	g_free(charnum);

    charnum = NULL;
    pauselist = NULL;
    fallindata = NULL;
       
}

void matrix_update(void)	
{
    AquariumData *ad;
    int j, i, k, y, x = -4, c, a = 0;
    int sum = 0;

    ad = aquarium_get_settings_ptr();

    for(j = 0;j < xrows; j++){
	y=0;

	if(charnum[j] < (2 * yrows - 5)) 
	  a = 5;
	else 
	  a = 2 * yrows-charnum[j];
    
	for(k = charnum[j]; k < (charnum[j] + a); k++)
	    sum += (int)fallindata[j][k];
    
	if(sum == 0 && pauselist[j] == -79){
	    if(g_rand_int_range(ad->rnd, 0, 5) == 4) 
		pauselist[j] = 10;
	}
	else
	    pauselist[j] = -79;
     
	if(pauselist[j] <= 0){
	    charnum[j]--;
	    if(charnum[j] <= 0) charnum[j] = 2 * yrows;
	}
	else
	    pauselist[j]--;

	c = charnum[j];

	/* Draw all falling signs in a row */

	for(;;){ /* */

	    over_draw(x, y, (int)fallindata[j][c],
		      matrix_dark.width,
		      matrix_dark.height,
		      matrix_dark.image);
      
	    if(c == yrows / 2)
		make_matrix_line(j, yrows, yrows);
     
	    if(c == 3 * yrows / 2)
		make_matrix_line(j,0, yrows);

	    if(c >= 2 * yrows) 
	      c = 0;

	    c++;
	    y += matrix_dark.height;
	    if(y > ad->ymax) 
	      break;
	}
	x += matrix_dark.width;

    }

    for(i = 0; i < bright_num; i++){
	over_draw(bright_x[i] * matrix_bright.width - 4, 
		  bright_y[i] * matrix_bright.height,
		  g_rand_int_range(ad->rnd, 1, LETTERS),
		  matrix_bright.width,
		  matrix_bright.height,
		  matrix_bright.image);
    }

}

