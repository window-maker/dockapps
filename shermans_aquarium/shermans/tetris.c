
/* 
   Tetris for Sherman's aquarium by Jonas Aaberg <cja@gmx.net>

*/


/*
   On their spare time, some reads a book, some goes shopping, 
   some do sports, some spends time with friends, and one
   sits programming yet another tetris version and listens to
   Sonata Arctica... 

   Strange world, isn't it?

*/
  

#include "tetris.h"
#include "aquarium.h"
#include "over.h"
#include "settings.h"

#include <stdlib.h>
#include <stdio.h>
#include <gdk/gdkkeysyms.h>

#define X 0
#define Y 1
#define PIECES 7
#define FIGURES 10
#define STATUS 4
#define TOP10 0
#define LINES 1
#define SCORE 2
#define LEVEL 3

static int pieces[PIECES][4][2] = {
    { {0, 1}, {1, 1}, {2, 1}, {3, 1} },
    { {0, 1}, {1, 1}, {2, 1}, {2, 2} },
    { {0, 2}, {1, 2}, {2, 2}, {2, 1} },
    { {0, 2}, {1, 1}, {1, 2}, {2, 2} },
    { {0, 2}, {1, 2}, {1, 1}, {2, 1} },
    { {0, 1}, {1, 1}, {1, 2}, {2, 2} },
    { {1, 1}, {2, 1}, {1, 2}, {2, 2} }
};

static int board_x;
static int board_y;

static Tetris_highscore_table highscores[11];
static Tetris_settings tetris_settings;

static int *board;

static int curr_piece_num;
static int next_piece_num;

static int location[2];
static int curr_piece[4][2];
static int gameover, firsttime;
static int show_highscore_upper;

static SA_Image tetris_pieces, tetris_figures, tetris_figures2;
static SA_Image tetris_status, tetris_gameover;

static int delay;
static int counter;
static int has_highscore, use_font2;

static int death = 0;

static unsigned char* tetris_background;

static int score, level, lines;
static int horizontal;

Tetris_highscore_table *tetris_get_highscore_table_ptr(void)
{
    return highscores;
}

Tetris_settings *tetris_get_settings_ptr(void)
{
    return &tetris_settings;
}

void prepare_tetris_background()
{
    int x,y,ypos, R,G,B;
    AquariumData *ad;

    ad= aquarium_get_settings_ptr();

    //printf("prepare tetris background\n");

    tetris_background = g_malloc0(ad->ymax * ad->xmax * 4); 

    if(horizontal){
	for(y = 0; y < ad->ymax; y++){
	    ypos = y * ad->xmax * 4;
	    for(x = 0; x < ad->xmax; x++){
		if((y == 0 || y == ((board_y) * tetris_pieces.height + 1))
		   && x <= (board_x * (tetris_pieces.width))){
		    R = 0xab;
		    G = 0xba;
		    B = 0xc6;
		} else if (x == (board_x * (tetris_pieces.width)) &&
			   y >= 0 && y < (board_y) * tetris_pieces.height + 1){
		    R = 0xab;
		    G = 0xba;
		    B = 0xc6;
		}
		else{
		    R = G = B = 0;
		}

		tetris_background[ypos + x * 4 + 0] = R;
		tetris_background[ypos + x * 4 + 1] = G;
		tetris_background[ypos + x * 4 + 2] = B;
		tetris_background[ypos + x * 4 + 3] = 0xff;
	    }

	}
    }
    else {
	for(y = 0; y < ad->ymax; y++){
	    ypos = y * ad->xmax * 4;
	    for(x = 0; x < ad->xmax; x++){
		if((x == 2 || x == ((board_x + 1) * tetris_pieces.width - 1))
		   && y <= (board_y * (tetris_pieces.height))){
		    R = 0xab;
		    G = 0xba;
		    B = 0xc6;
		} else if (y == (board_y * (tetris_pieces.height)) &&
			   x >= 2 && x < (board_x + 1) * tetris_pieces.width){
		    R = 0xab;
		    G = 0xba;
		    B = 0xc6;


		}
		else{
		    R = G = B = 0;
		}

		tetris_background[ypos + x * 4 + 0] = R;
		tetris_background[ypos + x * 4 + 1] = G;
		tetris_background[ypos + x * 4 + 2] = B;
		tetris_background[ypos + x * 4 + 3] = 0xff;
	    }

	}
    }

}

void tetris_exit(void)
{
    //printf("tetris exit\n");
    if(board != NULL)
	g_free(board);
    if(tetris_background != NULL)
	g_free(tetris_background);

    board = NULL;
    tetris_background = NULL;
}

void tetris_restart(void)
{
    int i,x,y,tbx;
    AquariumData *ad;

    if(board != NULL)
	tetris_exit();

    //    printf("tetris restart\n");

    ad = aquarium_get_settings_ptr();

    //printf("xmax %d ymax %d\n", ad->xmax, ad->ymax);

    board_x = (ad->xmax - 24) / tetris_pieces.width;
    board_y = (ad->ymax - 2) / tetris_pieces.height;

    if(tetris_settings.size_limit){
	if(board_x > tetris_settings.width)
	    board_x = tetris_settings.width;

	if(board_y > tetris_settings.height)
	    board_y = tetris_settings.height;

    }



    //printf("Board_x %d, board_y %d\n", board_x, board_y);

    if(2 * ad->xmax / 3 > ad->ymax){
	horizontal = 1;
	x = Y;
	y = X;
	tbx = board_y;
    }
    else {
	horizontal = 0;
	x = X;
	y = Y;
	tbx = board_x;
    }


    //printf("Board_x %d, board_y %d\n", board_x, board_y);
    board = NULL;

    if(gai_load_int_with_default("tetris/board_x", -1) == board_x &&
       gai_load_int_with_default("tetris/board_y", -1) == board_y && firsttime){
	board = (int *)gai_load_raw_data("tetris/board", NULL);
    }

    if(board == NULL && death){
	//printf("first time or die\n");
	board = g_malloc0(board_x * board_y * sizeof(int));
	gameover = 0;
	score = 0;
	lines = 0;
	level = 1;
	death = 0;
	location[x] = (tbx - 4) / 2 + horizontal;
	location[y] = -4;
	next_piece_num = g_rand_int_range(ad->rnd, 0, PIECES);
	curr_piece_num = g_rand_int_range(ad->rnd, 0, PIECES);
	delay = 16;
	for(i = 0; i < 4; i++){
	    curr_piece[i][x] = pieces[curr_piece_num][i][X];
	    curr_piece[i][y] = pieces[curr_piece_num][i][Y];
	}


    } else {
	//printf("load game over\n");
	if(board == NULL)
	    board = g_malloc0(board_x * board_y * sizeof(int));
	gameover = gai_load_int_with_default("tetris/gameover", 0);
	score = gai_load_int_with_default("tetris/score", 0);
	lines = gai_load_int_with_default("tetris/lines", 0);
	level = gai_load_int_with_default("tetris/level", 1);
	location[x] = gai_load_int_with_default("tetris/location_x", (tbx - 4) / 2 + horizontal);
	location[y] = gai_load_int_with_default("tetris/location_y", -4);
	next_piece_num = gai_load_int_with_default("tetris/next_piece_num", g_rand_int_range(ad->rnd, 0, PIECES));
	curr_piece_num = gai_load_int_with_default("tetris/curr_piece_num", g_rand_int_range(ad->rnd, 0, PIECES));
	delay = gai_load_int_with_default("tetris/delay", 16);
	curr_piece[0][x] = gai_load_int_with_default("tetris/curr_piece0X", 0);
	curr_piece[0][y] = gai_load_int_with_default("tetris/curr_piece0Y", 0);
	curr_piece[1][x] = gai_load_int_with_default("tetris/curr_piece1X", 0);
	curr_piece[1][y] = gai_load_int_with_default("tetris/curr_piece1Y", 0);
	curr_piece[2][x] = gai_load_int_with_default("tetris/curr_piece2X", 0);
	curr_piece[2][y] = gai_load_int_with_default("tetris/curr_piece2Y", 0);
	curr_piece[3][x] = gai_load_int_with_default("tetris/curr_piece3X", 0);
	curr_piece[3][y] = gai_load_int_with_default("tetris/curr_piece3Y", 0);

    }
    firsttime = 0;

    prepare_tetris_background();

    use_font2 = 0;

    counter = delay;
    show_highscore_upper = 0;

    has_highscore = 1979;

}

void tetris_init(void)
{
    death = 0;
    firsttime = 1;
    //printf("tetris init\n");
    tetris_load_highscores();

    load_image("tetris_pieces.png",
	       &tetris_pieces,
	       PIECES);

    load_image("tetris_figures.png",
	       &tetris_figures,
	       FIGURES);

    load_image("tetris_figures2.png",
	       &tetris_figures2,
	       FIGURES);


    load_image("tetris_status.png",
	       &tetris_status,
	       STATUS);


    load_image("tetris_gameover.png",
	       &tetris_gameover,
	       1);

    tetris_restart();

}

void check_highscore(void)
{
    int i, j;

    for(i = 0; i < 10; i++){
	if(highscores[i].score<score){
	    for(j = 9; j > i; j--){
		highscores[i].score = highscores[j - 1].score;
		highscores[i].level = highscores[j - 1].level;
		highscores[i].lines = highscores[j - 1].lines;
	    }
	    has_highscore = i;
	    highscores[i].score = score;
	    highscores[i].level = level;
	    highscores[i].lines = lines;
	    break;
	}
	
    }
    tetris_save_highscores();
    
}

void drawnextpiece(void)
{
    int i, x, y;
    AquariumData *ad;

    if(!tetris_settings.show_next)
	return;
    ad = aquarium_get_settings_ptr();

    x = ad->xmax - (tetris_pieces.width * 4 + 1);
    y = 4;
    

    for(i = 0; i < 4; i++){
	over_draw(x + tetris_pieces.width * pieces[next_piece_num][i][X],
		  y + tetris_pieces.height * pieces[next_piece_num][i][Y],
		  next_piece_num,
		  tetris_pieces.width,
		  tetris_pieces.height,
		  tetris_pieces.image);
    }
}

void drawfigures(int x,int y, int num)
{
    SA_Image *figures;

    if(use_font2) 
	figures = &tetris_figures2;
    else 
	figures = &tetris_figures;


    if(num > 99)
	over_draw(x, y,
		  num / 100,
		  figures->width,
		  figures->height,
		  figures->image);

    if(num > 9)
	over_draw(x + figures->width, y,
		  (num % 100) / 10,
		  figures->width,
		  figures->height,
		  figures->image);

    over_draw(x + figures->width * 2,y,
	      (num % 10),
	      figures->width,
	      figures->height,
	      figures->image);

}

void drawstatus(void)
{
    int x, y, dy;

    AquariumData *ad;

    ad = aquarium_get_settings_ptr();

    x = ad->xmax-tetris_figures.width * 3 - 2;
    y = 20;
    dy = tetris_figures.height + 3;

    drawfigures(x, y, score);
    drawfigures(x, y + dy, level);
    drawfigures(x, y + 2 * dy, lines);

}

void draw_gameover(void)
{
    AquariumData *ad;
    ad = aquarium_get_settings_ptr();

    drawstatus();

    over_draw((ad->xmax-tetris_gameover.width) / 2,
	      (ad->ymax-tetris_gameover.height) / 2,
	      0,
	      tetris_gameover.width,
	      tetris_gameover.height,
	      tetris_gameover.image);
}

void draw_finalscore(void)
{
    int x, y, dy, ypos;

    AquariumData *ad;

    //printf("draw_finalscore\n");

    ad = aquarium_get_settings_ptr();

    for(y = 0; y < ad->ymax; y++){
	ypos = y * ad->xmax * 4;
	for(x = 0; x <ad->xmax; x++){
	    tetris_background[ypos+x * 4 + 0] = 0;
	    tetris_background[ypos+x * 4 + 1] = 0;
	    tetris_background[ypos+x * 4 + 2] = 0;
	    tetris_background[ypos+x * 4 + 3] = 0xff;
	}
    }
    over_draw(0, 0, 0, ad->xmax,ad->ymax,tetris_background);

    x = (ad->xmax-tetris_status.width) / 2;
    y = (ad->ymax-3 * (tetris_figures.height + tetris_status.height)) / 2;
    dy = tetris_figures.height + tetris_status.height;
    over_draw(x, y , SCORE, tetris_status.width, tetris_status.height,
	      tetris_status.image);
    
    over_draw(x, y + dy, LEVEL, tetris_status.width, tetris_status.height,
	      tetris_status.image);

    over_draw(x, y + 2 * dy, LINES, tetris_status.width, tetris_status.height,
	      tetris_status.image);

    drawfigures(x + tetris_status.width, y + tetris_status.height, score);
    drawfigures(x + tetris_status.width, y + tetris_status.height + dy, level);
    drawfigures(x + tetris_status.width, y + tetris_status.height + 2 * dy, lines);
    counter = 0;
}


void draw_highscorelist(void)
{
    int x, y, ypos, i, dx, sx;

    AquariumData *ad;

    ad = aquarium_get_settings_ptr();
    //printf("darw_highscore\n");

    for(y = 0; y < ad->ymax; y++){
	ypos = y * ad->xmax * 4;
	for(x = 0; x < ad->xmax; x++){
	    tetris_background[ypos + x * 4 + 0] = 0;
	    tetris_background[ypos + x * 4 + 1] = 0;
	    tetris_background[ypos + x * 4 + 2] = 0;
	    tetris_background[ypos + x * 4 + 3] = 0xff;
	}
    }
    over_draw(0, 0, 0, ad->xmax, ad->ymax,tetris_background);

    over_draw((ad->xmax - tetris_status.width) / 2, 1, TOP10, tetris_status.width,
	      tetris_status.height, tetris_status.image);

    sx = -tetris_figures.width;

    dx = ((ad->xmax-sx) - 4 * 3 * tetris_figures.width) / 4 + 3 * tetris_figures.width;

    if(ad->ymax > (tetris_status.height + 10 * tetris_figures.height)){
	for(i = 0; i < 10; i++){
	    if(i == has_highscore) 
		use_font2 = TRUE;
	    else 
		use_font2 = FALSE;
	    drawfigures(sx,
			8 + tetris_status.height + i * tetris_figures.height,
			i + 1);
	
	    drawfigures(sx + dx,
			8 + tetris_status.height + i * tetris_figures.height,
			highscores[i].score);

	    drawfigures(sx + 2 * dx,
			8 + tetris_status.height + i * tetris_figures.height,
			highscores[i].lines);

	    drawfigures(sx + 3 * dx,
			8 + tetris_status.height + i * tetris_figures.height,
			highscores[i].level);
	}


    } else {
	for(i = 5 * show_highscore_upper; i < (5 + 5 * show_highscore_upper); i++){
	    if(i == has_highscore) 
		use_font2 = TRUE;
	    else 
		use_font2 = FALSE;

	    drawfigures(sx,
			8 + tetris_status.height + (i - 5 * show_highscore_upper) * tetris_figures.height, i + 1);
	
	    drawfigures(sx + dx,
			 8 + tetris_status.height + (i - 5 * show_highscore_upper) * tetris_figures.height, highscores[i].score);
	    drawfigures(sx + 2 * dx,
			8 + tetris_status.height + (i - 5 * show_highscore_upper) * tetris_figures.height, highscores[i].lines);
	    drawfigures(sx + 3 * dx,
			8 + tetris_status.height + (i - 5 * show_highscore_upper) * tetris_figures.height, highscores[i].level);
	}
	counter++;
	if(counter == 100){
	    show_highscore_upper = !show_highscore_upper;
	    counter = 0;
	}
    }
       


}

void drawboardonscreen(void)
{
    int x, y;

    AquariumData *ad;

    ad = aquarium_get_settings_ptr();
    //    printf("drawongscreen %d\n", gameover);

    switch(gameover){

    /* Normal play */
    case 0:

	over_draw(0 ,0 ,0, ad->xmax, ad->ymax, tetris_background);
    	//printf("%d %d\n", board_x, board_y);
	for(y = 0; y < board_y; y++){
	    for(x = 0;x < board_x; x++){

		if(board[y * board_x + x] != 0)
		    over_draw(x * tetris_pieces.width + 3 * !horizontal,
			      y * tetris_pieces.height + horizontal,
			      board[y * board_x + x] - 1,
			      tetris_pieces.width,
			      tetris_pieces.height,
			      tetris_pieces.image);
	    }
	}
	//printf("for done\n");
	drawnextpiece();
	drawstatus();
	break;

    case 1:
	draw_gameover();
	break;
    case 2:
	draw_finalscore();
	break;
    
    case 3:
	draw_highscorelist();

	break;
    default:
	break;
    }

}

void manageboard(int piece[4][2], int location[2], int colour)
{
    int i;

    //    printf("manageboard\n");

    for(i = 0; i < 4; i++){
	if(location[X] + piece[i][X] >= 0 && location[Y] + piece[i][Y] >=0)
	    board[(location[X] + piece[i][X]) + (location[Y] + piece[i][Y]) * board_x]
	    = colour;
    }
    //    printf("manage board done\n");
}

void removefromboard(int piece[4][2],int location[2])
{
    manageboard(piece, location, 0);
}


void drawonboard(int piece[4][2],int location[2])
{
    manageboard(piece, location, curr_piece_num + 1);
}


/* 
   Returns: 
   0 = Movement ok.
   1 = Landed upon piece.
   2 = Tried to move outside X wise.
   3 = Landed upon piece and tried to move too far X wise. 
   4 = Landed on bottom.
   5 = Landed on bottom & piece.
   6 = landed on bottom & X outside.
   7 = landed on bottom & X outside & piece.
*/

int movepiece(int piece[4][2],int new_location[2])
{
    int i;
    int stuck = 0;

    int x, y, tbx, tby;
	

    //    printf("movepiece\n");

    if(horizontal){
	x = Y;
	y = X;
	tbx = board_y;
	tby = board_x;
	
    } else{
	x = X;
	y = Y;
	tbx = board_x;
	tby = board_y;
    }



    for(i = 0;i < 4; i++){
	/* Edges */
	if(new_location[x] + piece[i][x] >= tbx || new_location[x] + piece[i][x]<0){
	    stuck |= 2;
	    continue;
	}
	/* Bottom */
	if(new_location[y] + piece[i][y] == tby){
	    stuck |= 4;
	    continue;
	}
	/* Upon other pieces */
	if(new_location[y] + piece[i][y] >= 0)
	    if(board[(new_location[X] + piece[i][X]) + (new_location[Y] + piece[i][Y]) * board_x]!=0)
		stuck |= 1;
    }
    return stuck;

}

void rotatepiece(int piece[4][2], int location[2])
{
    int tmp_piece[4][2], i;

    //    printf("rotate piece\n");

    for(i= 0 ; i < 4; i++){
	tmp_piece[i][X] = 3 - piece[i][Y];
	tmp_piece[i][Y] = piece[i][X];
    }

    if(movepiece(tmp_piece, location) == 0){
	for(i = 0; i < 4; i++){
	    piece[i][X] = tmp_piece[i][X];
	    piece[i][Y] = tmp_piece[i][Y];
	}
    }
       
}

void removeline(int line)
{
    int x, y;
    //    printf("removeline\n");

    if(horizontal){
	for(x = line; x > 0; x--){
	    for(y = 0; y < board_y; y++){
		board[x + y * board_x] = board[(x - 1) + y * board_x];
	    }
	}
	for(y = 0; y < board_y; y++)
	    board[y * board_x] = 0;
    }
    else {
	for(y = line; y > 0; y--){
	    for(x = 0; x < board_x; x++){
		board[x + y * board_x] = board[x + (y - 1) * board_x];
	    }
	}
	for(x = 0; x < board_x; x++)
	    board[x] = 0;
    }


}

void checkforline(void)
{

    int gotline = 0, tot_lines = 0;
    int x, y;
    int points[] = {0, 1 , 3 , 5, 9};

    //    printf("checkforline\n");

    if(horizontal){
	for(x = 0; x < board_x; x++){
	    gotline = 1;
	    for(y = 0; y < board_y; y++){
		if(board[x + y * board_x] == 0) 
		    gotline=0;
	    }
	    if(gotline){
		tot_lines++;
		removeline(x);
		/* Restart */
		x = 0;
	    }
	}
    }  
    else {
	for(y = 0; y < board_y; y++){
	    gotline = 1;
	    for(x = 0; x < board_x; x++){
		if(board[x + y * board_x] == 0) gotline = 0;
	    }
	    if(gotline){
		tot_lines++;
		removeline(y);
		/* Restart */
		y = 0;
	    }
	}
    }

    if(tot_lines>0){
	lines += tot_lines;

	score += points[tot_lines];

	if(((lines - tot_lines) / 10) < lines / 10){
	    level++;
	    if(delay > 0) delay--;
	}
	/*	printf("Score %d Lines %d Level %d\n",score,lines,level);*/
    }

}

void tetris_update(void)
{
    int answer;
    int i, x, y, tbx;
    AquariumData *ad;

    ad = aquarium_get_settings_ptr();


    if(gameover != 0 || delay != counter){
	drawboardonscreen();
	counter++;
	return;
    }

    counter = 0;

    removefromboard(curr_piece, location);

    if(horizontal){
	x = Y;
	y = X;
	tbx = board_y;
    }
    else {
	x = X;
	y = Y;
	tbx = board_x;
    }

    location[y]++;

    answer = movepiece(curr_piece, location);

    if(answer == 0){
	drawonboard(curr_piece, location);
    }
    if((answer & 4) == 4 || (answer & 1) == 1){

	location[y]--;

	drawonboard(curr_piece, location);

	checkforline();

	for(i = 0; i < 4; i++){
	    if((curr_piece[i][y] + location[y]) < 0) 
		gameover = 1;
	}

	if(gameover == 0){
	    location[y] = -4;
	    location[x] = (tbx - 4) / 2 + horizontal;
	
	    for(i = 0; i < 4; i++){
		curr_piece[i][x] = pieces[next_piece_num][i][X];
		curr_piece[i][y] = pieces[next_piece_num][i][Y];
	    }
	    curr_piece_num = next_piece_num;
	    next_piece_num = g_rand_int_range(ad->rnd, 0, PIECES);

	    answer = movepiece(curr_piece, location);
	    if(answer != 0) gameover = 1;
	}
	else{
	    check_highscore();
	}

    }
    
    drawboardonscreen();


}


void tetris_keypress(int key)
{
    int x, y;

    removefromboard(curr_piece, location);

    if(gameover == 4){
	death = 1;
	tetris_restart();
	return;
    }

    if(horizontal){
	x = Y;
	y = X;
    }
    else {
	x = X;
	y = Y;
    }

    switch(key){
    case GDK_Up:
	rotatepiece(curr_piece, location);
	break;
    case GDK_Left:
	location[x]--;
	if(movepiece(curr_piece, location)!=0)
	    location[x]++;

	break;
    case GDK_Right:
	location[x]++;
	if(movepiece(curr_piece, location)!=0)
	    location[x]--;
	break;
    case GDK_Down:
	if(gameover != 0) {
	    gameover++;
	}
	location[y]++;
	if(movepiece(curr_piece, location)!=0)
	    location[y]--;

	break;
    default:
	break;
    }

    drawonboard(curr_piece, location);

}

void tetris_joystick(GaiFlagsJoystick js)
{
    int key = 0;
    if(js & GAI_JOYSTICK_LEFT)
	key |= GDK_Left;
    if(js & GAI_JOYSTICK_RIGHT)
	key |= GDK_Right;
    if(js & GAI_JOYSTICK_UP)
	key |= GDK_Up;
    if(js & GAI_JOYSTICK_BUTTON_A)
	key |= GDK_Up;
    if(js & GAI_JOYSTICK_BUTTON_B)
	key |= GDK_Down;
    if(js & GAI_JOYSTICK_DOWN)
	key |= GDK_Down;
    tetris_keypress(key);

}

void tetris_start(void)
{
    //printf("tetris start\n");
}

void tetris_end(void)
{
    int x, y, tbx;
    AquariumData *ad;
    //printf("tetris end\n");

    ad = aquarium_get_settings_ptr();

    if(2 * ad->xmax / 3 > ad->ymax){
	horizontal = 1;
	x = Y;
	y = X;
	tbx = board_y;
    }
    else {
	horizontal = 0;
	x = X;
	y = Y;
	tbx = board_x;
    }

    //printf("Board_x %d, board_y %d\n", board_x, board_y);

    gai_save_int("tetris/board_x", board_x);
    gai_save_int("tetris/board_y", board_y);
    gai_save_int("tetris/gameover", gameover);
    gai_save_int("tetris/score", score);
    gai_save_int("tetris/lines", lines);
    gai_save_int("tetris/level", level);
    gai_save_int("tetris/location_x", location[x]);
    gai_save_int("tetris/location_y", location[y]);
    gai_save_int("tetris/next_piece_num", next_piece_num);
    gai_save_int("tetris/curr_piece_num", curr_piece_num);
    gai_save_int("tetris/delay", delay);
    gai_save_raw_data("tetris/board", (unsigned char *)board, board_y * board_x * sizeof(int));

    gai_save_int("tetris/curr_piece0X", curr_piece[0][x]);
    gai_save_int("tetris/curr_piece0Y", curr_piece[0][y]);
    gai_save_int("tetris/curr_piece1X", curr_piece[1][x]);
    gai_save_int("tetris/curr_piece1Y", curr_piece[1][y]);
    gai_save_int("tetris/curr_piece2X", curr_piece[2][x]);
    gai_save_int("tetris/curr_piece2Y", curr_piece[2][y]);
    gai_save_int("tetris/curr_piece3X", curr_piece[3][x]);
    gai_save_int("tetris/curr_piece3Y", curr_piece[3][y]);

}
