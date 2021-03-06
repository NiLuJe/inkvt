
/* autogenerated by keymap.py - do not edit */

#pragma once
#include <deque>
#include <linux/input.h>

class KeycodeTranslation {
    int _is_leftmeta_down;
    int _is_rightmeta_down;
    int _is_leftctrl_down;
    int _is_leftshift_down;
    int _is_rightshift_down;
    int _is_leftalt_down;
    int _is_capslock_down;
    int _is_rightctrl_down;
    int _is_rightalt_down;
    int scrolllock_state;
    int numlock_state;

public:
    bool is_shift() {
        return _is_leftshift_down || _is_rightshift_down;
    }

    bool is_meta() {
        return _is_leftmeta_down || _is_rightmeta_down;
    }

    bool is_ctrl() {
        return _is_capslock_down || _is_leftctrl_down || _is_rightctrl_down;
    }

    bool is_alt() {
        return _is_leftalt_down || _is_rightalt_down;
    }

private:
    void _translate_press(int keycode, std::deque<char> & out) {
        switch (keycode) {
            case KEY_ESC:
                out.push_back('\x1b');
                break;
            case KEY_1:
                if (is_shift()) {
                    out.push_back('!');
                }
                else {
                    out.push_back('1');
                }
                break;
            case KEY_2:
                if (is_shift()) {
                    out.push_back('@');
                }
                else {
                    out.push_back('2');
                }
                break;
            case KEY_3:
                if (is_shift()) {
                    out.push_back('#');
                }
                else {
                    out.push_back('3');
                }
                break;
            case KEY_4:
                if (is_shift()) {
                    out.push_back('$');
                }
                else {
                    out.push_back('4');
                }
                break;
            case KEY_5:
                if (is_shift()) {
                    out.push_back('%');
                }
                else {
                    out.push_back('5');
                }
                break;
            case KEY_6:
                if (is_shift()) {
                    out.push_back('^');
                }
                else {
                    out.push_back('6');
                }
                break;
            case KEY_7:
                if (is_shift()) {
                    out.push_back('&');
                }
                else {
                    out.push_back('7');
                }
                break;
            case KEY_8:
                if (is_shift()) {
                    out.push_back('*');
                }
                else {
                    out.push_back('8');
                }
                break;
            case KEY_9:
                if (is_shift()) {
                    out.push_back('(');
                }
                else {
                    out.push_back('9');
                }
                break;
            case KEY_0:
                if (is_shift()) {
                    out.push_back(')');
                }
                else {
                    out.push_back('0');
                }
                break;
            case KEY_MINUS:
                if (is_shift()) {
                    out.push_back('_');
                }
                else {
                    out.push_back('-');
                }
                break;
            case KEY_EQUAL:
                if (is_shift()) {
                    out.push_back('+');
                }
                else {
                    out.push_back('=');
                }
                break;
            case KEY_BACKSPACE:
                out.push_back('\x7f');
                break;
            case KEY_TAB:
                if (is_shift()) {
                    out.insert(out.end(), {'\x1b', '[', 'Z'});
                }
                else {
                    out.push_back('\t');
                }
                break;
            case KEY_Q:
                if (is_shift()) {
                    out.push_back('Q');
                }
                else {
                    out.push_back('q');
                }
                break;
            case KEY_W:
                if (is_shift()) {
                    out.push_back('W');
                }
                else {
                    out.push_back('w');
                }
                break;
            case KEY_E:
                if (is_shift()) {
                    out.push_back('E');
                }
                else {
                    out.push_back('e');
                }
                break;
            case KEY_R:
                if (is_shift()) {
                    out.push_back('R');
                }
                else {
                    out.push_back('r');
                }
                break;
            case KEY_T:
                if (is_shift()) {
                    out.push_back('T');
                }
                else {
                    out.push_back('t');
                }
                break;
            case KEY_Y:
                if (is_shift()) {
                    out.push_back('Y');
                }
                else {
                    out.push_back('y');
                }
                break;
            case KEY_U:
                if (is_shift()) {
                    out.push_back('U');
                }
                else {
                    out.push_back('u');
                }
                break;
            case KEY_I:
                if (is_shift()) {
                    out.push_back('I');
                }
                else {
                    out.push_back('i');
                }
                break;
            case KEY_O:
                if (is_shift()) {
                    out.push_back('O');
                }
                else {
                    out.push_back('o');
                }
                break;
            case KEY_P:
                if (is_shift()) {
                    out.push_back('P');
                }
                else {
                    out.push_back('p');
                }
                break;
            case KEY_LEFTBRACE:
                if (is_shift()) {
                    out.push_back('{');
                }
                else {
                    out.push_back('[');
                }
                break;
            case KEY_RIGHTBRACE:
                if (is_shift()) {
                    out.push_back('}');
                }
                else {
                    out.push_back(']');
                }
                break;
            case KEY_ENTER:
                out.push_back('\n');
                break;
            case KEY_A:
                if (is_shift()) {
                    out.push_back('A');
                }
                else {
                    out.push_back('a');
                }
                break;
            case KEY_S:
                if (is_shift()) {
                    out.push_back('S');
                }
                else {
                    out.push_back('s');
                }
                break;
            case KEY_D:
                if (is_shift()) {
                    out.push_back('D');
                }
                else {
                    out.push_back('d');
                }
                break;
            case KEY_F:
                if (is_shift()) {
                    out.push_back('F');
                }
                else {
                    out.push_back('f');
                }
                break;
            case KEY_G:
                if (is_shift()) {
                    out.push_back('G');
                }
                else {
                    out.push_back('g');
                }
                break;
            case KEY_H:
                if (is_shift()) {
                    out.push_back('H');
                }
                else {
                    out.push_back('h');
                }
                break;
            case KEY_J:
                if (is_shift()) {
                    out.push_back('J');
                }
                else {
                    out.push_back('j');
                }
                break;
            case KEY_K:
                if (is_shift()) {
                    out.push_back('K');
                }
                else {
                    out.push_back('k');
                }
                break;
            case KEY_L:
                if (is_shift()) {
                    out.push_back('L');
                }
                else {
                    out.push_back('l');
                }
                break;
            case KEY_SEMICOLON:
                if (is_shift()) {
                    out.push_back(':');
                }
                else {
                    out.push_back(';');
                }
                break;
            case KEY_APOSTROPHE:
                if (is_shift()) {
                    out.push_back('"');
                }
                else {
                    out.push_back('\'');
                }
                break;
            case KEY_GRAVE:
                if (is_shift()) {
                    out.push_back('~');
                }
                else {
                    out.push_back('`');
                }
                break;
            case KEY_BACKSLASH:
                if (is_shift()) {
                    out.push_back('|');
                }
                else {
                    out.push_back('\\');
                }
                break;
            case KEY_Z:
                if (is_shift()) {
                    out.push_back('Z');
                }
                else {
                    out.push_back('z');
                }
                break;
            case KEY_X:
                if (is_shift()) {
                    out.push_back('X');
                }
                else {
                    out.push_back('x');
                }
                break;
            case KEY_C:
                if (is_shift()) {
                    out.push_back('C');
                }
                else {
                    out.push_back('c');
                }
                break;
            case KEY_V:
                if (is_shift()) {
                    out.push_back('V');
                }
                else {
                    out.push_back('v');
                }
                break;
            case KEY_B:
                if (is_shift()) {
                    out.push_back('B');
                }
                else {
                    out.push_back('b');
                }
                break;
            case KEY_N:
                if (is_shift()) {
                    out.push_back('N');
                }
                else {
                    out.push_back('n');
                }
                break;
            case KEY_M:
                if (is_shift()) {
                    out.push_back('M');
                }
                else {
                    out.push_back('m');
                }
                break;
            case KEY_COMMA:
                if (is_shift()) {
                    out.push_back('<');
                }
                else {
                    out.push_back(',');
                }
                break;
            case KEY_DOT:
                if (is_shift()) {
                    out.push_back('>');
                }
                else {
                    out.push_back('.');
                }
                break;
            case KEY_SLASH:
                if (is_shift()) {
                    out.push_back('?');
                }
                else {
                    out.push_back('/');
                }
                break;
            case KEY_SPACE:
                out.push_back(' ');
                break;
            case KEY_F1:
                out.insert(out.end(), {'\x1b', 'O', 'P'});
                break;
            case KEY_F2:
                out.insert(out.end(), {'\x1b', 'O', 'Q'});
                break;
            case KEY_F3:
                out.insert(out.end(), {'\x1b', 'O', 'R'});
                break;
            case KEY_F4:
                out.insert(out.end(), {'\x1b', 'O', 'S'});
                break;
            case KEY_F5:
                out.insert(out.end(), {'\x1b', '[', '1', '5', '~'});
                break;
            case KEY_F6:
                out.insert(out.end(), {'\x1b', '[', '1', '7', '~'});
                break;
            case KEY_F7:
                out.insert(out.end(), {'\x1b', '[', '1', '8', '~'});
                break;
            case KEY_F8:
                out.insert(out.end(), {'\x1b', '[', '1', '9', '~'});
                break;
            case KEY_F9:
                out.insert(out.end(), {'\x1b', '[', '2', '0', '~'});
                break;
            case KEY_F10:
                out.insert(out.end(), {'\x1b', '[', '2', '1', '~'});
                break;
            case KEY_F11:
                out.insert(out.end(), {'\x1b', '[', '2', '3', '~'});
                break;
            case KEY_F12:
                out.insert(out.end(), {'\x1b', '[', '2', '4', '~'});
                break;
            case KEY_HOME:
                out.insert(out.end(), {'\x1b', '[', 'H'});
                break;
            case KEY_UP:
                out.insert(out.end(), {'\x1b', '[', 'A'});
                break;
            case KEY_PAGEUP:
                out.insert(out.end(), {'\x1b', '[', '5', '~'});
                break;
            case KEY_LEFT:
                out.insert(out.end(), {'\x1b', '[', 'D'});
                break;
            case KEY_RIGHT:
                out.insert(out.end(), {'\x1b', '[', 'C'});
                break;
            case KEY_END:
                out.insert(out.end(), {'\x1b', '[', '8', '~'});
                break;
            case KEY_DOWN:
                out.insert(out.end(), {'\x1b', '[', 'B'});
                break;
            case KEY_PAGEDOWN:
                out.insert(out.end(), {'\x1b', '[', '6', '~'});
                break;
            case KEY_INSERT:
                out.insert(out.end(), {'\x1b', '[', '2', '~'});
                break;
            case KEY_DELETE:
                out.insert(out.end(), {'\x1b', '[', '3', '~'});
                break;
            case KEY_KPASTERISK:
                out.push_back('*');
                break;
            case KEY_KP0:
                if (numlock_state) {
                    out.push_back('5');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', '2', '~'});
                }
                break;
            case KEY_KP1:
                if (numlock_state) {
                    out.push_back('5');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', '8', '~'});
                }
                break;
            case KEY_KP2:
                if (numlock_state) {
                    out.push_back('5');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', 'B'});
                }
                break;
            case KEY_KP3:
                if (numlock_state) {
                    out.push_back('5');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', '6', '~'});
                }
                break;
            case KEY_KP4:
                if (numlock_state) {
                    out.push_back('5');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', 'D'});
                }
                break;
            case KEY_KP5:
                if (numlock_state) {
                    out.push_back('5');
                }
                break;
            case KEY_KP6:
                if (numlock_state) {
                    out.push_back('6');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', 'C'});
                }
                break;
            case KEY_KP7:
                if (numlock_state) {
                    out.push_back('7');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', 'H'});
                }
                break;
            case KEY_KP8:
                if (numlock_state) {
                    out.push_back('8');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', 'A'});
                }
                break;
            case KEY_KP9:
                if (numlock_state) {
                    out.push_back('9');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', '5', '~'});
                }
                break;
            case KEY_KPMINUS:
                out.push_back('-');
                break;
            case KEY_KPPLUS:
                out.push_back('+');
                break;
            case KEY_KPDOT:
                if (numlock_state) {
                    out.push_back('.');
                }
                else {
                    out.insert(out.end(), {'\x1b', '[', '3', '~'});
                }
                break;
            case KEY_KPENTER:
                out.push_back('\n');
                break;
        }
    }

public:
    void press(int keycode, std::deque<char> & out) {
        if (keycode == KEY_LEFTMETA)   _is_leftmeta_down = 1;
        if (keycode == KEY_RIGHTMETA)  _is_rightmeta_down = 1;
        if (keycode == KEY_LEFTCTRL)   _is_leftctrl_down = 1;
        if (keycode == KEY_LEFTSHIFT)  _is_leftshift_down = 1;
        if (keycode == KEY_RIGHTSHIFT) _is_rightshift_down = 1;
        if (keycode == KEY_LEFTALT)    _is_leftalt_down = 1;
        if (keycode == KEY_CAPSLOCK)   _is_capslock_down = 1;
        if (keycode == KEY_RIGHTCTRL)  _is_rightctrl_down = 1;
        if (keycode == KEY_RIGHTALT)   _is_rightalt_down = 1;
        if (keycode == KEY_SCROLLLOCK) scrolllock_state = !scrolllock_state;
        if (keycode == KEY_NUMLOCK)    numlock_state = !numlock_state;
        _translate_press(keycode, out);
    }

    void release(int keycode, std::deque<char> & out __attribute__((unused))) {
        if (keycode == KEY_LEFTMETA)   _is_leftmeta_down = 0;
        if (keycode == KEY_RIGHTMETA)  _is_rightmeta_down = 0;
        if (keycode == KEY_LEFTCTRL)   _is_leftctrl_down = 0;
        if (keycode == KEY_LEFTSHIFT)  _is_leftshift_down = 0;
        if (keycode == KEY_RIGHTSHIFT) _is_rightshift_down = 0;
        if (keycode == KEY_LEFTALT)    _is_leftalt_down = 0;
        if (keycode == KEY_CAPSLOCK)   _is_capslock_down = 0;
        if (keycode == KEY_RIGHTCTRL)  _is_rightctrl_down = 0;
        if (keycode == KEY_RIGHTALT)   _is_rightalt_down = 0;
    }
};
