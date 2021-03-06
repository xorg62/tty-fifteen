/*
 *      TTY-FIFTEEN Main file.
 *      Copyright © 2008 Martin Duquesnoy <xorg62@gmail.com>
 *      All rights reserved.
 *
 *      Redistribution and use in source and binary forms, with or without
 *      modification, are permitted provided that the following conditions are
 *      met:
 *
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above
 *        copyright notice, this list of conditions and the following disclaimer
 *        in the documentation and/or other materials provided with the
 *        distribution.
 *      * Neither the name of the  nor the names of its
 *        contributors may be used to endorse or promote products derived from
 *        this software without specific prior written permission.
 *
 *      THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *      "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *      LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *      A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *      OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *      SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *      LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *      DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *      THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *      (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *      OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define CASE (option.lines * option.rows)
#define RAND(m, M) ((rand() % ((M) - (m) + 1)) + (m))

struct fifteen_option
{
     int lines;
     int rows;
} option = { 4, 4 };

int running = 1;
int *frame = NULL;
int vpos;
unsigned long moves = 0;

void init(void);
void random_frame(void);
void check_victory(void);
void swap(int n);
void key_event(void);
void print_frame(void);

void
init(void)
{
     int bg = COLOR_BLACK;

     /* Init ncurses */
     initscr();
     noecho();
     keypad(stdscr, TRUE);
     curs_set(FALSE);
     start_color();
     clear();

     if(use_default_colors() == OK)
          bg = -1;

     init_pair(0, bg, bg);
     init_pair(1, COLOR_GREEN, bg);
     init_pair(2, COLOR_RED, bg);

     refresh();
     srand(getpid());

     frame = malloc(sizeof(int) * CASE);

     return;
}

void
random_frame(void)
{
     int i, j, n, k = 0;

     /* Set the frame case */
     for(i = 0; i < CASE; ++i)
     {
          do
          {
               k = 0;
               n = RAND(0, CASE - 1);
               for(j = 0; j < i; ++j)
                    if(frame[j] == n)
                         ++k;
          }
          while(k);

          frame[i] = n;
     }

     /* Set the vpos */
     for(i = 0; frame[i] != 0; ++i);
     vpos = i;


     return;
}

void
check_victory(void)
{
     int i, c = 0;

     /* Check the frame order */
     for(i = 1; i < CASE; ++i)
          if(frame[i - 1] == i)
               ++c;

     /* Win \o/ */
     if(c == i - 1)
     {
          print_frame();
          attrset(A_UNDERLINE);
          attron(COLOR_PAIR(2));
          mvprintw(2, (option.rows * 4) + 2, "WIN !");
          attroff(COLOR_PAIR(2));
          attrset(A_NORMAL);
          running = 0;
     }

     return;
}

void
swap(int n)
{
     int i;

     for(i = 0; i < CASE; ++i)
          if(frame[i] == n)
               break;

     frame[i] = 0;
     frame[vpos] = n;

     vpos = i;

     ++moves;

     return;
}

void
key_event(void)
{
     int c;

     switch((c = getch()))
     {
     case KEY_UP:
     case 'k':
     case 'K':
          if((vpos + option.rows) < CASE)
               swap(frame[vpos + option.rows]);
          break;
     case KEY_DOWN:
     case 'j':
     case 'J':
          if((vpos - option.rows) >= 0)
               swap(frame[vpos - option.rows]);
          break;
     case KEY_LEFT:
     case 'h':
     case 'H':
          if((vpos + 1) % option.rows != 0 && (vpos + 1) < CASE)
               swap(frame[vpos + 1]);
          break;
     case KEY_RIGHT:
     case 'l':
     case 'L':
          if(vpos % option.rows)
               swap(frame[vpos - 1]);
          break;
     case 'q':
     case 'Q':
          running = 0;
          break;
     case 'r':
     case 'R':
          running = 1;
          random_frame();
          moves = 0;
          break;
     default:
          break;
     }

     return;
}

void
print_frame(void)
{
     int i;

     move(1, 0);
     for(i = 0; i < CASE; ++i)
     {
          if(frame[i])
          {
               addch('[');
               attron(COLOR_PAIR(1));
               printw("%.2d", frame[i]);
               attroff(COLOR_PAIR(1));
               addch(']');
          }
          else
               printw("    ");

          if(i > 0 && !((i + 1) % option.rows))
               addch('\n');
     }

     mvaddstr(1, (option.rows * 4) + 2, "Moves: ");
     attron(COLOR_PAIR(1));
     printw("%d", moves);
     attroff(COLOR_PAIR(1));

     return;
}

int
main(int argc, char **argv)
{
     int c;

     while((c = getopt(argc, argv, "hl:r:")) != -1)
     {
          if(c == 'l' || c == 'r')
          {
               if(atoi(optarg) <= 9
                  && atoi(optarg) > 1)
                    ((c == 'l') ? (option.lines = atoi(optarg)) : (option.rows = atoi(optarg)));
               else
               {
                    fprintf(stderr, "tty-fifteen: Maximum %1$s: 9, Minimum %1$s: 2.\n",
                            ((c == 'l') ? "lines" : "rows"));
                    exit(EXIT_FAILURE);
               }
          }
          else
          {
               fprintf(stderr, "usage: %s [-option ...]\n"
                       "options:\n"
                       "    -h         Show help.\n"
                       "    -l [2-9]   Set the frame lines.\n"
                       "    -r [2-9]   Set the frame rows.\n\n", argv[0]);
               exit(EXIT_SUCCESS);
          }
     }

     init();
     random_frame();

     while(running)
     {
          print_frame();
          check_victory();
          key_event();
     }

     endwin();
     free(frame);

     return 0;
}
