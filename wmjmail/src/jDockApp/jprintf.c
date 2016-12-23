#include <stdarg.h>
#include "jDockApp.h"

#define char_width     5
#define char_height    7

#define left_edge      5
#define right_edge    59
#define  top_edge      6
#define stop_edge     12

#define num_start     85
#define Letter_start 135
#define letter_start 400

#define cc(X, Y, Z) case X: copy_start = Y; copy_width = Z; break
#define cw(X, Y)    case X:                 copy_width = Y; break

int start_x;
int start_y;
int line_position;

void jprintf_internal(int, const char*);

void jpprintf(int x, int y, int color, const char *format, ...) {
    va_list arguments;
    char line[80];

    if(y<=5 && y>=1) {
        start_y =  stop_edge + y * char_height + y-1;
    } else if ( !y ) {
        start_y =   top_edge;
    } else {
        printf("You can't jprintf to line %i.\n", y);
        printf("  The range is currently: 0-6.\n");
        exit(1);
    }

    start_x = left_edge+1 +  x * char_width;
    line_position = 0;

    va_start(arguments, format);
     vsnprintf(line, 80, format, arguments);
    va_end(arguments);

    jprintf_internal(color, line);
}

void jprintf(int color, const char *format, ...) {
    va_list arguments;
    char line[80];

    va_start(arguments, format);
     vsnprintf(line, 80, format, arguments);
    va_end(arguments);

    jprintf_internal(color, line);
}

void jprintf_internal(int color, const char *line) {
    char cur_char;
    int string_position;
    int copy_start;
    int copy_width;

    if(!start_x) {
        printf("You must make at least one call to jpprintf()\n");
        exit(1);
    }

    string_position = 0;

    while(line[string_position] != '\0') {
        cur_char       = line[string_position];
        copy_start     = 0;
        copy_width     = char_width;

        if(cur_char>='0' && cur_char<='9') {
            copy_start = (cur_char-'0')*char_width+num_start;
        }
        else if(cur_char>='A' && cur_char<='Z') {
            copy_start = (cur_char-'A')*char_width+Letter_start;
        }
        else if(cur_char>='a' && cur_char<='z') {
            copy_start = (cur_char-'a')*char_width+letter_start;
        }

        switch(cur_char) {
            cw('l', 4);
            cw('T', 4);
            cw('I', 4);
            cw('1', 4);
            cw('Y', 4);
            cw('c', 4);
            cw('0', 4);
            cw('i', 4);
            cw('v', 4);
            cw(' ', 3);

            cc( '!', 267, 2);
            cc( '@', 270, 5);
            cc( '#', 275, 6);
            cc( '$', 281, 6);
            cc( '%', 287, 5);
            cc( '^', 292, 4);
            cc( '&', 296, 4);
            cc( '*', 301, 4);
            cc( '(', 306, 3);
            cc( ')', 311, 3);
            cc( '{', 315, 4);
            cc( '}', 320, 4);
            cc( '[', 325, 4);
            cc( ']', 330, 4);
            cc( '<', 335, 4);
            cc( '>', 340, 4);
            cc( '/', 345, 5);
            cc('\\', 350, 5);
            cc( '+', 355, 6);
            cc( '-', 362, 3);
            cc( '=', 365, 4);
            cc( ':', 371, 2);
            cc( ';', 375, 3);
            cc( ',', 380, 3);
            cc( '.', 386, 3);
            cc('\'', 391, 3);
            cc( '"', 396, 4);
        }

        if(start_x+line_position+char_width > right_edge)
            break;

        if(copy_start) copyXPMArea(
            copy_start,
            color,
            copy_width,
            char_height,
            start_x+line_position,
            start_y
        );

        line_position += copy_width;
        string_position++;
    }
}
