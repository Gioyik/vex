#define _POSIX_SOURCE

#include <signal.h>

#include "../include/all.h"

#define CTRL(key) ((key) & 0x1F)

static void move_line(int n) {
    int index = buf.index + (n * view.bpline);
    buf_setindex(index, buf.nybble);
}

static void move_col(int n) {
    if (buf.mode == HEX) {
        int index = buf.index + (n / 2);
        if (n % 2 != 0) {
            if (buf.nybble) {
                buf_setindex(index + (n > 0 ? 1 : 0), false);
            } else {
                buf_setindex(index - (n > 0 ? 0 : 1), true);
            }
        } else {
            buf_setindex(index, false);
        }
    } else { 
        buf_setindex(buf.index + n, false);
    }
}

static void suspend() {
    kill(getpid(), SIGTSTP);
}

static void toggle_mode() {
    if (buf.mode == HEX) {
        buf.mode = ASCII;
        buf.nybble = false;
    } else {
        buf.mode = HEX;
    }
}

static void set_state(states_t state) {
    buf.state = state;
}

static void replace_char(char ch) {
    bool is_backspace = (ch == 127);
    if (is_backspace) { 
        move_col(-1);
        buf_revertchar();
     } else {
        buf_putchar(ch);
        move_col(+1);
    }
}

static void goto_line_beg() {
    int index = buf.index - (buf.index % view.bpline);
    buf_setindex(index, false);
}

static void goto_line_end() {
    int index = (buf.index - (buf.index % view.bpline)) + (view.bpline - 1);
    if (index >= buf.size)
        buf_setindex(buf.size - 1, true);
    else
        buf_setindex(index, true);
}

static void goto_grp_next() {
    int index = buf.index + (view.bpgrp - (buf.index % view.bpgrp));
    buf_setindex(index, false);
}

static void goto_grp_prev() {
    if (buf.index % view.bpgrp) {
        buf_setindex(buf.index - (buf.index % view.bpgrp), false);
    } else {
        buf_setindex(buf.index - view.bpgrp, false);
    }
}

static void goto_buffer_beg() {
    buf_setindex(0, false);
}

static void goto_buffer_end() {
    int index = buf.size - (buf.size % view.bpline);
    buf_setindex(index, false);
}

static void goto_half_next() {
    int index = buf.index + ((LINES / 2) * view.bpline);
    buf_setindex(index, buf.nybble);
}

static void goto_half_prev() {
    int index = buf.index - ((LINES / 2) * view.bpline);
    buf_setindex(index, buf.nybble);
}

bool route(int ch) {
    switch (ch) {
        case CTRL('c'):
        case CTRL('q'):   return false; break;
        case KEY_SUSPEND:
        case CTRL('z'):   suspend(); break;
        case CTRL('w'):   buf_write(); break;
        case CTRL('r'):   buf_revert(); break;
        case CTRL('['):   set_state(ESCAPE); break;
        case KEY_LEFT:    move_col(-1); break;
        case KEY_UP:      move_line(-1); break;
        case KEY_RIGHT:   move_col(+1); break;
        case KEY_DOWN:    move_line(+1); break;
        case '\t':        toggle_mode(); break;    
        default:
            if (buf.state == REPLACE) {
                replace_char(ch);
                return true;
            }
            break;
    }

    switch (ch) {
        case 'R': set_state(REPLACE); break;
        case 'h': move_col(-1); break; 
        case 'k': move_line(-1); break;
        case 'l': move_col(+1); break; 
        case 'j': move_line(+1); break;
        case 'w': goto_grp_next(); break;
        case 'b': goto_grp_prev(); break;
        case 'g': goto_buffer_beg(); break;
        case 'G': goto_buffer_end(); break;
        case '^': goto_line_beg(); break;
        case '$': goto_line_end(); break;
        case 'd': goto_half_next(); break;
        case CTRL('d'): goto_half_next(); break;
        case CTRL('f'): goto_half_next(); break;
        case 'u': goto_half_prev(); break;
        case CTRL('u'): goto_half_prev(); break;
        case CTRL('b'): goto_half_prev(); break;
        case '?': view.help = !view.help; break;
        default:  break;                                    
    }

    return true;
}

