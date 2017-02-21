#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/xpm.h>

#include <libdockapp/wmgeneral.h>

#include "wmtetris.xpm"
#include "wmtetris-mask.xbm"

#define TEMPORAL_RESOLUTION 10000
#define INITIAL_DELAY 250000
#define FAST_MODE_DELAY 30000
#define DELAY_INCREMENT 20000

#define BLOCK_SIZE 3
#define BOARD_POS_X 4
#define BOARD_POS_Y 6
#define BOARD_WIDTH 13
#define BOARD_HEIGHT 18
#define NEXT_POS_X 46
#define NEXT_POS_Y 7

#define BUTTONC 6
#define BUTTON_NONE 0
#define BUTTON_ROTATE_LEFT 1
#define BUTTON_ROTATE_RIGHT 2
#define BUTTON_MOVE_LEFT 3
#define BUTTON_MOVE_RIGHT 4
#define BUTTON_MOVE_DOWN 5

static int buttons[6][4] = {
	{ 0,  0, 64, 64},
	{43, 31, 51, 40},
	{52, 31, 60, 40},
	{43, 41, 51, 50},
	{52, 41, 60, 50},
	{43, 51, 60, 60}
};

static int initial_figures[7][4][2] = {
	{ {0, 1}, {1, 1}, {2, 1}, {3, 1} },
	{ {0, 1}, {1, 1}, {2, 1}, {2, 2} },
	{ {0, 2}, {1, 2}, {2, 2}, {2, 1} },
	{ {0, 2}, {1, 1}, {1, 2}, {2, 2} },
	{ {0, 2}, {1, 2}, {1, 1}, {2, 1} },
	{ {0, 1}, {1, 1}, {1, 2}, {2, 2} },
	{ {1, 1}, {2, 1}, {1, 2}, {2, 2} }
};

unsigned char board[BOARD_WIDTH][BOARD_HEIGHT];
int score=0;

int which_button(int x, int y);
int rotate_figure(int direction, int figure[4][2], int fig_x, int fig_y);
void draw_figure(int figure[4][2], int type, int x, int y);
void draw_next_figure(int figure[4][2], int type);
void general_draw_figure(int base_x, int base_y, int figure[4][2],
						 int type, int x, int y);
void full_refresh();
int check_figure_position(int fig_x, int fig_y, int figure[4][2]);

int main(int argc, char *argv[]) {
	int i, x, y, step, input, fast_mode, progress,
		fig_x, fig_y, new_figure=1, figure_type, next_figure_type;
	int figure[4][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} },
		next_figure[4][2] = { {0, 0}, {0, 0}, {0, 0}, {0, 0} };
	XEvent event;

	srand(time(NULL));

	for (y = 0; y < BOARD_HEIGHT; y++)
		for (x = 0; x < BOARD_WIDTH; x++)
			board[x][y] = 0;

	openXwindow(argc, argv, wmtetris_xpm, (char *)wmtetris_mask_bits,
				wmtetris_mask_width, wmtetris_mask_height);
	copyXPMArea(64, 0, 64, 64, 0, 0);
	RedrawWindow();

	fast_mode=0;
	figure_type = random() % 7;
	for (i = 0; i < 4; i++) {
		figure[i][0] = initial_figures[figure_type][i][0];
		figure[i][1] = initial_figures[figure_type][i][1];
	}
	next_figure_type = random() % 7;
	for (i = 0; i < 4; i++) {
		next_figure[i][0] = initial_figures[next_figure_type][i][0];
		next_figure[i][1] = initial_figures[next_figure_type][i][1];
	}
	draw_next_figure(next_figure, next_figure_type);
	fig_x = BOARD_WIDTH / 2 - 1;
	fig_y = 0;
	draw_figure(figure, figure_type, fig_x, fig_y);
	RedrawWindow();

	while (1) {
		if (check_figure_position(fig_x, fig_y + 1, figure)) {
			new_figure = 0;
			draw_figure(figure, -1, fig_x, fig_y);
			fig_y++;
		} else {
			new_figure = 1;
			for (i = 0; i < 4; i++) {
				board [fig_x + figure[i][0]] [fig_y + figure[i][1]] = figure_type + 1;
			}

			progress=0;
			for (y = 0; y < BOARD_HEIGHT; y++) {
				for (x = 0; x < BOARD_WIDTH; x++)
					if (!board[x][y])
						break;
				if (x == BOARD_WIDTH) {
					for (i = y; i > 0; i--)
						for (x = 0; x < BOARD_WIDTH; x++)
							board[x][i] = board[x][i-1];
					progress++;
				}
			}
			score += progress*progress;

			full_refresh();

			i = score;
			for (x = 3; x >= 0; x--) {
				copyXPMArea(4 * (i % 10), 100, 3, 5, 44 + 4*x, 24);
				i /= 10;
			}

			figure_type = next_figure_type;
			next_figure_type = random() % 7;
			for (i = 0; i < 4; i++) {
				figure[i][0] = next_figure[i][0];
				figure[i][1] = next_figure[i][1];
				next_figure[i][0] = initial_figures[next_figure_type][i][0];
				next_figure[i][1] = initial_figures[next_figure_type][i][1];
			}
			draw_next_figure(next_figure, next_figure_type);
			fig_x = BOARD_WIDTH / 2 - 1;
			fig_y = 0;
		}
		draw_figure(figure, figure_type, fig_x, fig_y);
		RedrawWindow();

		if (new_figure) {
			if (!check_figure_position(fig_x, fig_y, figure)) {
				copyXPMArea(64, 64, 23, 15, 12, 24);
				RedrawWindow();
				while (1) {
					XNextEvent(display, &event);
					if (event.type == ButtonPress)
						break;
					if (event.type == Expose)
						RedrawWindow();
				}
				exit(0);
			}
		}

		for (step = 0; step < (fast_mode ? FAST_MODE_DELAY / TEMPORAL_RESOLUTION : INITIAL_DELAY / TEMPORAL_RESOLUTION); step++) {
			while (XPending(display)) {
				input = 0;

				XNextEvent(display, &event);
				switch (event.type) {
				case ButtonRelease:
					fast_mode = 0;
					break;
				case ButtonPress:
					if (!(input =
						  which_button(event.xbutton.x, event.xbutton.y))) {
/*
						switch (event.xbutton.button) {
						case 1:
							input = BUTTON_MOVE_LEFT;
							break;
						case 3:
							input = BUTTON_MOVE_RIGHT;
							break;
						case 2:
							input = BUTTON_ROTATE_RIGHT;
							break;
						}
*/
					}
				}
				if (input) {
					draw_figure(figure, -1, fig_x, fig_y);
					switch (input) {
					case BUTTON_ROTATE_LEFT:
						rotate_figure(0, figure, fig_x, fig_y);
						break;
					case BUTTON_ROTATE_RIGHT:
						rotate_figure(1, figure, fig_x, fig_y);
						break;
					case BUTTON_MOVE_LEFT:
						if (check_figure_position(fig_x - 1, fig_y, figure))
							fig_x--;
						break;
					case BUTTON_MOVE_RIGHT:
						if (check_figure_position(fig_x + 1, fig_y, figure))
							fig_x++;
						break;
					case BUTTON_MOVE_DOWN:
						fast_mode = 1;
						break;
					}
					draw_figure(figure, figure_type, fig_x, fig_y);
					RedrawWindow();
				}
			}
			usleep(TEMPORAL_RESOLUTION);
		}
	}
}

int which_button(int x, int y) {
	int i;
	
	for (i = BUTTONC - 1; i >= 0; i--) {
		if ((buttons[i][0] <= x && x < buttons[i][2]) &&
			(buttons[i][1] <= y && y < buttons[i][3]))
		 break;
	}
	return i;
}

int rotate_figure(int direction, int figure[4][2], int fig_x, int fig_y) {
	int i, temp[4][2];

	for (i = 0; i < 4; i++) {
		temp[i][0] = direction ? 3 - figure[i][1] :     figure[i][1];
		temp[i][1] = direction ?     figure[i][0] : 3 - figure[i][0];
	}
	if (check_figure_position(fig_x, fig_y, temp)) {
		for (i = 0; i < 4; i++) {
			figure[i][0] = temp[i][0];
			figure[i][1] = temp[i][1];
		}
		return 1;
	} else {
		return 0;
	}
}

void draw_figure(int figure[4][2], int type, int x, int y) {
	general_draw_figure(BOARD_POS_X, BOARD_POS_Y, figure, type, x, y);
}

void draw_next_figure(int figure[4][2], int type) {
	copyXPMArea(64 + NEXT_POS_X, NEXT_POS_Y, BLOCK_SIZE * 4, BLOCK_SIZE * 4,
				NEXT_POS_X, NEXT_POS_Y);
	general_draw_figure(NEXT_POS_X, NEXT_POS_Y, figure, type, 0, 0);
}

void general_draw_figure(int base_x, int base_y, int figure[4][2],
						 int type, int x, int y)
{
	int i, block_x, block_y;

	for (i = 0; i < 4; i++) {
		block_x = base_x + BLOCK_SIZE * (x + figure[i][0]);
		block_y = base_y + BLOCK_SIZE * (y + figure[i][1]);
		if (type == -1)
			copyXPMArea(64 + block_x, block_y, BLOCK_SIZE, BLOCK_SIZE, block_x, block_y);
		else
			copyXPMArea(0, 64 + BLOCK_SIZE * type, BLOCK_SIZE, BLOCK_SIZE, block_x, block_y);
	}
}

void full_refresh() {
	int x, y;

	for (y = 0; y < BOARD_HEIGHT; y++)
		for (x = 0; x < BOARD_WIDTH; x++)
			if (board[x][y])
				copyXPMArea(0, 64 + BLOCK_SIZE * (board[x][y] - 1), BLOCK_SIZE, BLOCK_SIZE,
							BOARD_POS_X + BLOCK_SIZE * x, BOARD_POS_Y + BLOCK_SIZE * y);
			else
				copyXPMArea(64 + BOARD_POS_X + BLOCK_SIZE * x, BOARD_POS_Y + BLOCK_SIZE * y, 
							BLOCK_SIZE, BLOCK_SIZE, 
							BOARD_POS_X + BLOCK_SIZE * x, BOARD_POS_Y + BLOCK_SIZE * y);
}

int check_figure_position(int fig_x, int fig_y, int figure[4][2]) {
	int i, x, y;

	for (i = 0; i < 4; i++) {
		x = fig_x + figure[i][0];
		y = fig_y + figure[i][1];
		if ((x < 0) || (x >= BOARD_WIDTH) || (y < 0) || (y >= BOARD_HEIGHT))
			return 0;
		if (board[x][y])
			return 0;
	}
	return 1;
}
