/* inkvt - VT100 terminal for E-ink devices
 * Copyright (C) 2020 Lennart Landsmeer <lennart@landsmeer.email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <algorithm>
#include <sys/timerfd.h>
#include <linux/fb.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>

#include "../libvterm/include/vterm.h"
#include "../FBInk/fbink.h"
#include "./osk.hpp"

// reset high_throughput_mode check every <n> ms
constexpr int INTERVAL_MS = 100;
// after <n> consecutive ticks of the timer without writes
// disable timer (sleep) mode, to save battery life
constexpr int TIMER_SLEEP_MODE_THRESHOLD = 10;

class VTermToFBInk {
    RoundedRect cursor;
    struct {
        bool SHIFT = 0;
        bool CTRL = 0;
        bool FN = 0;
        bool ALT = 0;
    } OSK;
public:
    VTerm * term;
    VTermScreen * screen;
    VTermScreenCallbacks vtsc;

    // timer to detect huge output streams
    long high_throughput_threshold = 100000;
    int timerfd = -1;
    long nwrites_in_interval = 0;
    bool high_throughput_mode = false;
    VTermPos last_cursor;
    bool timer_is_running = false;
    int nticks_without_output = 0;

    // osk debounce
    timeval osk_last_kp;
    int32_t osk_last_x = -100;
    int32_t osk_last_y = -100;

    itimerspec ts_on;
    itimerspec ts_off = {};
    //ts.it_value.tv_nsec = 100*1000000;
    //ts.it_interval.tv_nsec = 100*1000000;

    int fbfd;
    FBInkConfig config = {};
    FBInkState state = {};

    // Used in the commented out term_moverect() implementation:
    // FBInkDump dump = {};

    bool has_osk = false;
    struct {
        uint8_t canonical_rota = FB_ROTATE_UR;
        bool swap_axes;
        bool mirror_x;
        bool mirror_y;
    } input_quirks;

    unsigned int osk_height() {
        if (!has_osk) return 0u;
        unsigned int osk_height = 400u;
        if (osk_height > state.view_height / 2u) osk_height = state.view_height/2u;
        return osk_height;
    }

    unsigned int nrows() {
        unsigned int kb_height = osk_height();
        unsigned int line_height = state.view_height / state.max_rows;
        return (state.view_height - kb_height) / line_height - 1u;
    }

    unsigned int ncols() {
        return state.max_cols;
    }

    void run_timer() {
        // timer for output debouncing (high throughput mode)
        if (timer_is_running || timerfd == -1 /* we get called before timerfd creation in setup */) {
            return;
        }
        if (timerfd_settime(timerfd, 0, &ts_on, 0) >= 0) {
            timer_is_running = true;
            nticks_without_output = 0;
        }
    }

    void osk() {
        if (has_osk) {
            unsigned int h = osk_height();
            unsigned int osk_y = state.view_height - h;
            osk_setup(state.view_width, h);
            osk_render(fbfd, &config, osk_y, state.view_width, h);
        }
    }

    void refresh_input_quirks() {
        // We need to know the canonical rotation to deal with coordinates translation (and we refresh this on reinit)...
        input_quirks.canonical_rota = fbink_rota_native_to_canonical(state.current_rota);

        // Grab the device-specific panel translation quirks: we end up with flags similar to what is used in KOReader,
        // c.f., https://github.com/koreader/koreader/blob/master/frontend/device/kobo/device.lua
        // These represent what happens at UR (canonical), so we need fresh ones to adjust for the current rotation below.
        input_quirks.swap_axes = state.touch_swap_axes;
        input_quirks.mirror_x = state.touch_mirror_x;
        input_quirks.mirror_y = state.touch_mirror_y;

        // Then, we can handle standard coordinates translation given the current rotation.
        // We'll deal with this by flipping the swap/mirror flags,
        // which will allow us to handle everything at once when processing an input frame.
        // c.f., set_rotation @ https://github.com/rmkit-dev/rmkit/blob/master/src/rmkit/input/events.cpy
        switch (input_quirks.canonical_rota) {
            case FB_ROTATE_CW:
                input_quirks.swap_axes = !input_quirks.swap_axes;
                input_quirks.mirror_y  = !input_quirks.mirror_y;
                break;
            case FB_ROTATE_UD:
                input_quirks.mirror_x = !input_quirks.mirror_x;
                input_quirks.mirror_y = !input_quirks.mirror_y;
                break;
            case FB_ROTATE_CCW:
                input_quirks.swap_axes = !input_quirks.swap_axes;
                input_quirks.mirror_x  = !input_quirks.mirror_x;
                break;
            default:
                // NOP
                break;
        }
    }

    const char * click(int32_t x, int32_t y) {
        // (do not) draw ugly cursor
        if (0) {
            short cfg_row = config.row;
            short cfg_col = config.col;
            config.row = 0;
            config.col = 0;
            fbink_print_raw_data(
                    fbfd,
                    cursor.dst,
                    cursor.width,
                    cursor.height,
                    cursor.width * cursor.height * cursor.bpp,
                    static_cast<short int>(x - cursor.width / 2u),
                    static_cast<short int>(y - cursor.height / 2u),
                    &config
                    );
            config.row = cfg_row;
            config.col = cfg_col;
        }
        // debounce
        if (!has_osk) return "";
        timeval now;
        gettimeofday(&now, 0);
        double elapsedTime = (now.tv_sec - osk_last_kp.tv_sec) * 1000.0
                           + (now.tv_usec - osk_last_kp.tv_usec) / 1000.0;
        if (elapsedTime < 130 &&
                abs(x - osk_last_x) < 50 && abs(y - osk_last_y) < 50
                ) {
            return "";
        }
        osk_last_x = x;
        osk_last_y = y;
        osk_last_kp = now;
        // handle kp
        // this might be some of the ugliest code I have ever written
        // the returned string is only correct up to the next call to this function
        unsigned int h = osk_height();
        unsigned int osk_y = state.view_height - h;
        const kbkey * b = osk_press(state.view_width, osk_height(), static_cast<unsigned int>(x), static_cast<unsigned int>(y - osk_y));
        if (!b) {
            printf("Touch event; but no key @ (%d, %d)\n", x, y);
            return "";
        }
        switch (b->mod) {
            case SHIFT:
                OSK.SHIFT = 1;
                return "";
            case CTRL:
                OSK.CTRL = 1;
                return "";
            case FN:
                OSK.FN = 1;
                return "";
            case ALT:
                OSK.ALT = 1;
                return "";
        }
        const char * out = b->normal;
        if (OSK.FN && strlen(b->fn) > 0) {
            out = b->fn;
        } else if (OSK.SHIFT && strlen(b->shift) > 0) {
            out = b->shift;
        } else if (strlen(out) == 0) {
            return "";
        }
        if (OSK.CTRL && !OSK.ALT && strlen(out) == 1 && out[0] <= 'z' && out[0] >= 'a') {
            static char buf[2] = {(char)(out[0] & 0x1f), 0};
            out = buf;
        } else if (OSK.CTRL || OSK.ALT) {
            static char buf[100];
            assert(strlen(out) < 80);
            // TODO: Implement this
            // CSI u ; see
            //  - http://www.leonerd.org.uk/hacks/fixterms/
            //  - https://github.com/neovim/libvterm/blob/nvim/src/keyboard.c
            int mod = (OSK.CTRL ? VTERM_MOD_CTRL : 0)
                    | (OSK.ALT ? VTERM_MOD_ALT : 0);
            if (strlen(out) == 3 && out[0] == '\033' && out[1] == '[' && out[2] >= 'A' && out[2] <= 'D') {
                // arrow keys ; CSI 1;[modifier] {ABCDFHPQRS}
                sprintf(buf, "\033[1;%d%c", mod+1, out[2]);
                return buf;
            } else if (out[0] != '\033') {
                // `regular' keys
                sprintf(buf, "\033[%s;%du", out, mod+1);
                return buf;
            }
            // I could handle CSI ... ~ => CSI ... ; MOD+1 ~ handling here
            // but the current keyboard doesn't have function keys anyway
        }
        OSK.SHIFT = OSK.CTRL = OSK.ALT = OSK.FN = 0;
        return out;
    }

    void tick() {
        if (high_throughput_mode && nwrites_in_interval < high_throughput_threshold) {
            high_throughput_mode = false;
            VTermRect full_refresh = { 0, 0, 0, 0 };
            full_refresh.end_col = ncols();
            full_refresh.end_row = nrows();
            term_damage(full_refresh, this);
        }
        if (nwrites_in_interval == 0) {
            nticks_without_output += 1;
            if (nticks_without_output > TIMER_SLEEP_MODE_THRESHOLD) {
                timerfd_settime(timerfd, 0, &ts_off, 0);
                timer_is_running = false;
            }
        } else {
            nticks_without_output = 0;
        }
        nwrites_in_interval = 0;
    }

    bool reinit() {
        // only call this from main (yeah bad code...)
        // because we need to resize the pty too
        int res = fbink_reinit(fbfd, &config);
        if (res > EXIT_SUCCESS) {
            if (res & OK_ROTA_CHANGE) {
                /* Update the state to track the new rotation */
                fbink_get_state(&config, &state);
                printf("fbink_reinit w/ ROTA_CHANGE\n");
                /* Recompute input translation */
                refresh_input_quirks();
                /* Update the high throughput threshold */
                high_throughput_threshold = state.max_cols * state.max_rows;
                /* Clear screen and wait to make sure we get rid of potential broken updates
                 * that might have been sent against the wrong state (i.e., race during the rotation).
                 */
                fbink_wait_for_complete(fbfd, LAST_MARKER);
                /* NOTE: This is still potentially racy, and *may* fail. (i.e., we *could* retry on non-zero return codes) */
                fbink_cls(fbfd, &config, nullptr, false);
                fbink_wait_for_complete(fbfd, LAST_MARKER);
            }
            if (res & OK_LAYOUT_CHANGE) {
                /* We only actually care about layout changes */
                printf("fbink_reinit w/ LAYOUT_CHANGE\n");
                vterm_screen_reset(screen, 1);
                vterm_set_size(term, nrows(), ncols());
            }
            if (res & OK_ROTA_CHANGE) {
                /* But we do need to repaint our OSK */
                osk();
            }
            return true;
        }
        return false;
    }

    void update_fg_color(VTermColor * c) {
        vterm_screen_convert_color_to_rgb(screen, c);
#define FG(x) static_cast<uint8_t>((x^0xFFu) / 2u)
#define BG(x) static_cast<uint8_t>(x^0xFFu)
        fbink_set_fg_pen_rgba(FG(c->rgb.red), FG(c->rgb.green), FG(c->rgb.blue), 0xFFu, false, true);
    }

    void update_bg_color(VTermColor * c) {
        vterm_screen_convert_color_to_rgb(screen, c);
        fbink_set_bg_pen_rgba(BG(c->rgb.red), BG(c->rgb.green), BG(c->rgb.blue), 0xFFu, false, true);
#undef BG
#undef FG
    }

    void output_char(const VTermPos & pos) {
        // high throughput stuff
        nwrites_in_interval += 1;
        if (timer_is_running) {
            if (high_throughput_mode) return;
            if (nwrites_in_interval >= high_throughput_threshold) {
                high_throughput_mode = true;
                // fprintf(stdout, "Enabling high_throughput_mode (%ld >= %ld)\n", nwrites_in_interval, high_throughput_threshold);
            }
        } else {
            run_timer();
        }
        // drawing stuff
        VTermScreenCell cell;
        vterm_screen_get_cell(screen, pos, &cell);
        config.col = static_cast<short int>(pos.col);
        config.row = static_cast<short int>(pos.row);
        if (pos.row == last_cursor.row && pos.col == last_cursor.col) {
            update_fg_color(&cell.bg);
            update_bg_color(&cell.fg);
        } else {
            update_fg_color(&cell.fg);
            update_bg_color(&cell.bg);
        }
        VTermRect rect;
        rect.start_row = pos.row;
        rect.start_col = pos.col;
        rect.end_col = pos.col + 1;
        rect.end_row = pos.row + 1;
        char buf[32];
        size_t nread = vterm_screen_get_text(screen, buf, sizeof(buf)-1, rect);
        buf[nread] = 0;
        if (nread == 0) {
            fbink_grid_clear(fbfd, 1U, 1U, &config);
        } else {
            fbink_print(fbfd, buf, &config);
        }
    }

    void write(char byte) {
        vterm_input_write(term, &byte, 1);
    }

    static int term_damage(VTermRect rect, void * user) {
        VTermToFBInk * me = static_cast<VTermToFBInk*>(user);
        VTermPos pos;
        int row, col;

        // fprintf(stdout, "Called term_damage on (%d, %d) to (%d, %d)\n", rect.start_col, rect.start_row, rect.end_col, rect.end_row);
        // NOTE: Optimize large rects by only doing a single refresh call, instead of paired with cell-per-cell drawing.
        me->config.no_refresh = true;

        for (row = rect.start_row; row < rect.end_row; row++) {
            for (col = rect.start_col; col < rect.end_col; col++) {
                pos.col = col;
                pos.row = row;
                // NOTE: And again after the print call
                // if (cell.attrs.reverse) me->config->is_inverted = !me->config->is_inverted;
                me->output_char(pos);
            }
        }

        // Refresh the full rectangle
        me->config.no_refresh = false;
        me->config.col = static_cast<short int>(rect.start_col);
        me->config.row = static_cast<short int>(rect.start_row);
        fbink_grid_refresh(me->fbfd, static_cast<unsigned short int>(rect.end_col - rect.start_col), static_cast<unsigned short int>(rect.end_row - rect.start_row), &me->config);

        return 1;
    }

    static int term_movecursor(VTermPos pos, VTermPos old, int visible __attribute__((unused)), void * user) {
        // fprintf(stdout, "Called term_movecursor from (%d, %d) to (%d, %d)\n", old.col, old.row, pos.col, pos.row);
        VTermToFBInk * me = static_cast<VTermToFBInk*>(user);
        me->last_cursor = pos; // keep track of cursor in high_throughput_mode
        if (me->high_throughput_mode) return 1;
        me->output_char(old); // remove previous cursor
        me->output_char(pos); // add new cursor
        return 1;
    }

    static int term_moverect(VTermRect dst, VTermRect src __attribute__((unused)), void * user) {
        // fprintf(stdout, "Called term_moverect from (%d, %d), (%d, %d) to (%d, %d), (%d, %d)\n", src.start_col, src.start_row, src.end_col, src.end_row, dst.start_col, dst.start_row, dst.end_col, dst.end_row);
        term_damage(dst, user);
        return 1;
        /*
        // 'more effcient memcpy implementation':
        // This works sort of but is very buggy. Especially in the linux
        // console, which likes to overwrite some parts of the screen sometimes...
        VTermToFBInk * me = (VTermToFBInk*)user;
        unsigned short int w, h;
        w = me->state.glyph_width*(src.end_col - src.start_col);
        h = me->state.glyph_height*(src.end_row - src.start_row);
        me->config.col = src.start_col;
        me->config.row = src.start_row;
        fbink_region_dump(me->fbfd, 0, 0, w, h, &me->config, &me->dump);
        me->config.col = dst.start_col;
        me->config.row = dst.start_row;
        fbink_print_raw_data(me->fbfd, me->dump.data,
                w, h, me->dump.size, 0, 0, &me->config);
        return 1;
        */
    }

    static int term_settermprop(VTermProp prop __attribute__((unused)), VTermValue * val __attribute__((unused)), void * user __attribute__((unused))) {
        return 1;
    }

    static int term_bell(void* user __attribute__((unused))) {
        return 0;
    }

    static FONT_INDEX_E get_font(const char * font_cstr) {
        std::string font(font_cstr);
        if (font == "ibm") return FONT_INDEX_E::IBM;
        else if (font == "unscii") return FONT_INDEX_E::UNSCII;
        else if (font == "unscii_alt") return FONT_INDEX_E::UNSCII_ALT;
        else if (font == "unscii_thin") return FONT_INDEX_E::UNSCII_THIN;
        else if (font == "unscii_fantasy") return FONT_INDEX_E::UNSCII_FANTASY;
        else if (font == "unscii_mcr") return FONT_INDEX_E::UNSCII_MCR;
        else if (font == "unscii_tall") return FONT_INDEX_E::UNSCII_TALL;
        else if (font == "block") return FONT_INDEX_E::BLOCK;
        else if (font == "leggie") return FONT_INDEX_E::LEGGIE;
        else if (font == "veggie") return FONT_INDEX_E::VEGGIE;
        else if (font == "kates") return FONT_INDEX_E::KATES;
        else if (font == "fkp") return FONT_INDEX_E::FKP;
        else if (font == "ctrld") return FONT_INDEX_E::CTRLD;
        else if (font == "orp") return FONT_INDEX_E::ORP;
        else if (font == "orpb") return FONT_INDEX_E::ORPB;
        else if (font == "orpi") return FONT_INDEX_E::ORPI;
        else if (font == "scientifica") return FONT_INDEX_E::SCIENTIFICA;
        else if (font == "scientificab") return FONT_INDEX_E::SCIENTIFICAB;
        else if (font == "scientificai") return FONT_INDEX_E::SCIENTIFICAI;
        else if (font == "terminus") return FONT_INDEX_E::TERMINUS;
        else if (font == "terminusb") return FONT_INDEX_E::TERMINUSB;
        else if (font == "fatty") return FONT_INDEX_E::FATTY;
        else if (font == "spleen") return FONT_INDEX_E::SPLEEN;
        else if (font == "tewi") return FONT_INDEX_E::TEWI;
        else if (font == "tewib") return FONT_INDEX_E::TEWIB;
        else if (font == "topaz") return FONT_INDEX_E::TOPAZ;
        else if (font == "microknight") return FONT_INDEX_E::MICROKNIGHT;
        else if (font == "vga") return FONT_INDEX_E::VGA;
        /*
        // Not compiled in by default
        else if (font == "unifont") return FONT_INDEX_E::UNIFONT;
        else if (font == "unifontdw") return FONT_INDEX_E::UNIFONTDW;
        */
        else if (font == "cozette") return FONT_INDEX_E::COZETTE;
        printf("requesting non-existing font '%s'\n", font_cstr);
        return FONT_INDEX_E::TERMINUS;
    }

    void setup(uint8_t fontmult=2u, const char * fontname="terminus") {
        gettimeofday(&osk_last_kp, 0);
        cursor.width = 10u;
        cursor.height = 18u;
        cursor.spacing = 0.f;
        cursor.radius = 5.f;
        cursor.render();
        fbfd = fbink_open();
        if (fbfd == -1) {
            puts("fbink_open()");
            exit(1);
        }
        config.fontname = get_font(fontname);
        config.fontmult = fontmult;
        fbink_init(fbfd, &config);
        fbink_cls(fbfd, &config, nullptr, false);
        fbink_get_state(&config, &state);
        /* Compute input translation */
        refresh_input_quirks();
        config.is_quiet = true;
        config.is_verbose = false;
        fbink_update_verbosity(&config);
        /* Set the high throughput threshold at a screenful */
        high_throughput_threshold = state.max_cols * state.max_rows;

        // None of the dithering mechanisms deal very well with tiny refresh regions, so,
        // this doesn't really work all that well... :/
        /*
        config.wfm_mode = WFM_MODE_INDEX_E::WFM_DU;
        if (strcmp(state.device_platform, "Mark 7") >= 0) {
            config.dithering_mode = HW_DITHER_INDEX_E::HWD_ORDERED;
        } else {
            config.dithering_mode = HW_DITHER_INDEX_E::HWD_LEGACY;
        }
        */

        vtsc = (VTermScreenCallbacks){
            .damage = VTermToFBInk::term_damage,
            .moverect = VTermToFBInk::term_moverect,
            .movecursor = VTermToFBInk::term_movecursor,
            .settermprop = VTermToFBInk::term_settermprop,
            .bell = VTermToFBInk::term_bell,
            .resize = 0,
            .sb_pushline = 0,
            .sb_popline = 0,
            .sb_clear = 0
        };
        term = vterm_new(nrows(), ncols());
        vterm_set_utf8(term, 1);
        screen = vterm_obtain_screen(term);
        vterm_screen_set_callbacks(screen, &vtsc, this);
        vterm_screen_enable_altscreen(screen, 1);
        vterm_screen_reset(screen, 1);

        timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
        if (timerfd == -1) {
            perror("timerfd_create");
            exit(1);
        }
        ts_on.it_value.tv_nsec = INTERVAL_MS*1000000;
        ts_on.it_interval.tv_nsec = INTERVAL_MS*1000000;
        timerfd_settime(timerfd, 0, &ts_off, 0);
        osk();
    }
};
