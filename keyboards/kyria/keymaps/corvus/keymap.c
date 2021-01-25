/* Copyright 2020 Benjamin Wolfe <benjamin.e.wolfe@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H
#include <lib/lib8tion/lib8tion.h> // for rgb underglow

// Wanted to use unicode map for U-2212, true minus
// https://docs.qmk.fm/#/feature_unicode?id=unicode-map
// however, firmware was too large.
// see https://github.com/qmk/qmk_firmware/issues/3224
// via https://thomasbaart.nl/2018/12/01/reducing-firmware-size-in-qmk/
uint16_t escape_timer;
bool command_tracker = false;

uint16_t diacritical_mark_timer = 0;
bool need_diacritical = false;
bool turn_off_symbols = false;

#define HSV_DEFAULT 152, 255, 255

// flags. 0 = no change, 1 = increment, -1 = decrement.
int8_t change_hue = 0;
int8_t change_sat = 0;
int8_t change_val = 0;

// timer to control color change speed
uint16_t change_timer = 0;
const uint16_t change_tick  = 15;

char hsl_buffer[3];

// TODO: Make the layers match the enum.
enum layers {
    COLEMAK = 0,
    NUMBERS,
    UNICODE,
    NAV,
    CHROME,
    VSCODE,
    ADJUST
};

// on KC_BKTK_ESCAPE, compare KC_CCCV from https://github.com/BenjaminWolfe/qmk_firmware/blob/master/keyboards/kyria/keymaps/thomasbaart/keymap.c
enum custom_keycodes {
    KC_BKTK_ESCAPE = SAFE_RANGE,
    KC_UNICODE,
    KC_CYCLE_INPUTS,
    KC_TOGGLE_INPUTS,
    KC_SWITCH,
    RGB_RST,
    RGB_HUG,
    RGB_SAG,
    RGB_VAG
};

// to add layers, add them here, but also check `enum layers` above
// and `render_status` below
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * Base Layer: COLEMAK DH(m)
 * Line 2 of each key is what you get when you hold it.
 * Layer names are distinguished by CAPS.
 * Duplicating option and command allows for -shift combinations.
 * I'm experimenting with putting NUMBERS on the pinkies;
 *   it was originally between shift and unicode.
 *   I'm finding that I really prefer to use a modifier on the opposite hand
 *     from the key I'm targeting.
 *   If this works well I might be able to shift zero back to the thumb,
 *     and I might switch UNICODE and ADJST, by the "hand-switching" logic.
 *   Currently I'm worried about the apostrophe working correctly in everyday use.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * | Backtck |   Q   |   W   |   F   |   P   |   B   |                                  |   J   |   L   |   U   |   Y   |  ; :  |   - _   |
 * |   Esc   |       |       |       |       |       |                                  |       |       |       |       |       |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |Caps Lock|   A   |   R   |   S   |   T   |   G   |                                  |   M   |   N   |   E   |   I   |   O   |   ' "   |
 * | NUMBERS |       |       |       |       |       |                                  |       |       |       |       |       | NUMBERS |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |   Z   |   X   |   C   |   D   |   V   |       |       |  |       |       |   K   |   H   |  , <  |  . >  |  / ?  |  Enter  |
 * | Control |       |       |       |       |       |Command|       |  |       |  Opt  |       |       |       |       |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |Backspc|  Del  |  |  Tab  | Space |       |       |       |
 *                           |       | ADJST |  NAV  | Shift |Command|  |  Opt  | Shift |       |UNICODE|       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [COLEMAK] = LAYOUT(
      KC_BKTK_ESCAPE,       KC_Q,    KC_W,    KC_F,    KC_P,       KC_B,                                                                       KC_J,    KC_L,       KC_U,    KC_Y,   KC_SCLN, KC_MINS,
      LT(NUMBERS, KC_CAPS), KC_A,    KC_R,    KC_S,    KC_T,       KC_G,                                                                       KC_M,    KC_N,       KC_E,    KC_I,   KC_O,    LT(NUMBERS, KC_QUOT),
      KC_LCTL,              KC_Z,    KC_X,    KC_C,    KC_D,       KC_V,    KC_LCMD,         _______,         _______,        KC_LOPT,         KC_K,    KC_H,       KC_COMM, KC_DOT, KC_SLSH, KC_ENTER,
                                              _______, MO(ADJUST), MO(NAV), LSFT_T(KC_BSPC), LCMD_T(KC_DEL),  LOPT_T(KC_TAB), LSFT_T(KC_SPC),  _______, KC_UNICODE, _______
    ),
/*
 * Numbers and Symbols
 * number placement mimics that of the traditional number pad
 * some symbols follow a novel logic that makes sense to me
 *   even dollar and carat are paired for regular expressions,
 *     though it may make more sense to put the carat on top.
 * some symbols are moved to the NAV layer to make them easier to type.
 * I really tried to avoid the in-and-down index stretch, which I find unnatural.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |   /   |   +   |   [   |   ]   |   $   |                                  |   %   |   7   |   8   |   9   |       |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |   |   |   =   |   (   |   )   |   ^   |                                  |   &   |   4   |   5   |   6   |   0   |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |   \   |   -   |   {   |   }   |       |       |       |  |       |       |       |   1   |   2   |   3   |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [NUMBERS] = LAYOUT(
      _______, KC_SLASH,   KC_KP_PLUS,  KC_LBRC, KC_RBRC, KC_DLR,                                      KC_PERC, KC_KP_7, KC_KP_8, KC_KP_9, _______, _______,
      _______, S(KC_BSLS), KC_KP_EQUAL, KC_LPRN, KC_RPRN, KC_CIRC,                                     KC_AMPR, KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_0, _______,
      _______, KC_BSLS,    KC_KP_MINUS, KC_LCBR, KC_RCBR, _______, _______, _______, _______, _______, _______, KC_KP_1, KC_KP_2, KC_KP_3, _______, _______,
                                        _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
/*
 * Unicode Number Pad
 * intended for *entering unicode hex values*
 * KC_UNICODE in the base layer, when tapped and held, will activate this layer *and hold the option key*
 * "Cycle inputs" to cycle through keymap inputs. You have to have enabled unicode input on your Mac.
 * "Toggle inputs" to switch back to the one you were just on.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |       |       |       |       |                                  |   A   |   7   |   8   |   9   | Toggl |    D    |
 * |         |       |       |       |       |       |                                  |       |       |       |       | Inpts |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |                                  |   B   |   4   |   5   |   6   |   0   |    E    |
 * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |       |       |  |       |       |   C   |   1   |   2   |   3   | Cycle |    F    |
 * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       | Inpts |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [UNICODE] = LAYOUT(
      _______, _______, _______, _______, _______, _______,                                     KC_A,   KC_KP_7, KC_KP_8, KC_KP_9, KC_TOGGLE_INPUTS, KC_D,
      _______, _______, _______, _______, _______, _______,                                     KC_B,   KC_KP_4, KC_KP_5, KC_KP_6, KC_KP_0,          KC_E,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_C,   KC_KP_1, KC_KP_2, KC_KP_3, KC_CYCLE_INPUTS,  KC_F,
                                 _______, _______, _______, _______, _______, _______, _______,_______, _______, _______
    ),
/*
 * Navigation Layer
 * intended for tab navigation and text selection/manipulation.
 * arrow keys are positioned on the right hand mimicking the traditional placement.
 *   page up and down are not much used on a Mac, or even home and end for that matter,
 *     but I include them for when I most likely eventually have to switch for work.
 *   use CHROME and VSCODE layers with the arrows to navigate between tabs
 *   1Password is command-shift-X to open the 1Password password prompt.
 * command, shift, and option are placed conveniently in the home row on the left hand.
 * the top row of the left hand is left intact with command added.
 *   backtick, q, and w are particularly important on a mac.
 *   p, pressed with shift, is useful in VS code.
 * KC_SWITCH is for switching tabs on a Mac.
 *   When pressed the first time, it depresses command and leaves it down, and then taps tab.
 *   Any other time it will just press tab.
 *   Command will be released when dropping out of the layer
 *   (see layer_state_set_kb below).
 * the bottom row on the left is a modified version of the traditional Cmd-ZXCV pattern.
 *   Cmd-V (for now?) is left in the non-DHm position.
 *   Undo and redo are command-z and command-shift-z, respectively.
 *   Format is the VS Code shortcut, option-shift-F.
 *   Insert follows the VS Code Overtype extension, with command-shift-I.
 *       https://marketplace.visualstudio.com/items?itemName=adammaras.overtype
 *       It's frankly annoying that Mac doesn't use the insert key in general.
 *   Screenshot is the Mac shortcut to capture an area of the screen: command-shift-4.
 *   Escape is left close to its position in the default layer.
 * on the right are also some of the most common symbols, shift 1, 2, 8, and 3.
 *   using a keyboard like the Kyria really spoils you, and you notice key placement.
 *   I didn't want a key as important as the asterisk on the pinky's bottom row.
 *   and I didn't want any of these keys relegated to where they would have been
 *     in the numbers layer.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |Cmd-Bactc| Cmd-Q | Cmd-W | Cmd-F | Cmd-P | Cmd-B |                                  |       | Home  |   ↑   |  End  | PgUp  |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |   Esc   |Scrnsht|Command| Shift |  Opt  |Switch |                                  |       |   ←   |   ↓   |   →   | PgDn  |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |  Redo   | Undo  |  Cut  | Copy  | Paste |Format |Insert |       |  |       |       |       |   !   |   @   |   *   |   #   |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |1Passwd|       |       |       |       |
 *                           |       |       |       |       |       |  |       |CHROME |VSCODE |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [NAV] = LAYOUT(
      G(KC_GRAVE), G(KC_Q),    G(KC_W), G(KC_F), G(KC_P), G(KC_B),                                                _______,    KC_HOME, KC_UP,   KC_END,  KC_PGUP,   _______,
      KC_ESC,      SCMD(KC_4), KC_LCMD, KC_LSFT, KC_LOPT, KC_SWITCH,                                              _______,    KC_LEFT, KC_DOWN, KC_RGHT, KC_PGDOWN, _______,
      SCMD(KC_Z),  G(KC_Z),    G(KC_X), G(KC_C), G(KC_V), LSA(KC_F), SCMD(KC_I), _______, _______,    _______,    _______,    KC_EXLM, KC_AT,   KC_ASTR, KC_HASH,   _______,
                                        _______, _______, _______,   _______,    _______, SCMD(KC_X), MO(CHROME), MO(VSCODE), _______, _______
    ),
 /*
  * Chrome
  *
  * ,-------------------------------------------------.                                  ,-------------------------------------------------.
  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
  * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |                                  |       |  Prv  |       |  Nxt  |       |         |
  * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
  * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           `---------------------------------------'  `---------------------------------------'
  */
     [CHROME] = LAYOUT(
       _______, _______, _______, _______, _______, _______,                                     _______, _______,    _______, _______,    _______, _______,
       _______, _______, _______, _______, _______, _______,                                     _______, C(KC_PGUP), _______, C(KC_PGDN), _______, _______,
       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,    _______, _______,    _______, _______,
                                  _______, _______, _______, _______, _______, _______, _______, _______, _______,    _______
     ),
 /*
  * VS Code
  *
  * ,-------------------------------------------------.                                  ,-------------------------------------------------.
  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
  * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |                                  |       |  Prv  |       |  Nxt  |       |         |
  * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
  * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           `---------------------------------------'  `---------------------------------------'
  */
     [VSCODE] = LAYOUT(
       _______, _______, _______, _______, _______, _______,                                     _______, _______,          _______, _______,          _______, _______,
       _______, _______, _______, _______, _______, _______,                                     _______, LOPT(G(KC_LEFT)), _______, LOPT(G(KC_RGHT)), _______, _______,
       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______, _______,          _______, _______,
                                  _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______
     ),
/*
 * Adjust Layer
 * xxG keys are getters; they print the current hue saturation, or value.
 * There's so much more we can do here and so many ways to organize it.
 * It's worth splitting this into multiple layers and considering the UX.
 * But for now...
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |  HUG  |  SAG  |  VAG  |       |                                  |       |  F7   |  F8   |  F9   |  F10  |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         | Reset |  HUI  |  SAI  |  VAI  |       |                                  |       |  F4   |  F5   |  F6   |  F11  |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |  HUD  |  SAD  |  VAD  |       |       |       |  | VolUp |       |       |  F1   |  F2   |  F3   |  F12  |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |       |       |       |       |       |  | VolDn |  Mute |  Prev |  Play |  Next |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [ADJUST] = LAYOUT(
      _______, _______, RGB_HUG, RGB_SAG, RGB_VAG, _______,                                     _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
      _______, RGB_RST, RGB_HUI, RGB_SAI, RGB_VAI, _______,                                     _______, KC_F4,   KC_F5,   KC_F6,   KC_F11,  _______,
      _______, _______, RGB_HUD, RGB_SAD, RGB_VAD, _______, _______, _______, KC_VOLU, _______, _______, KC_F1,   KC_F2,   KC_F3,   KC_F12,  _______,
                                 _______, _______, _______, _______, _______, KC_VOLD, KC_MUTE, KC_MPRV, KC_MPLY, KC_MNXT
    ),
// /*
//  * Layer template
//  *
//  * ,-------------------------------------------------.                                  ,-------------------------------------------------.
//  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
//  * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
//  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
//  * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
//  * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
//  * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
//  *                           |       |       |       |       |       |  |       |       |       |       |       |
//  *                           |       |       |       |       |       |  |       |       |       |       |       |
//  *                           `---------------------------------------'  `---------------------------------------'
//  */
//     [_LAYERINDEX] = LAYOUT(
//       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
//       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
//                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
//     ),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) {
        // on press...
        switch (keycode) {
            // rgb keys; see also key-release code below
            case RGB_HUI: change_timer = timer_read(); change_hue =  1; return false;
            case RGB_HUD: change_timer = timer_read(); change_hue = -1; return false;
            case RGB_SAI: change_timer = timer_read(); change_sat =  1; return false;
            case RGB_SAD: change_timer = timer_read(); change_sat = -1; return false;
            case RGB_VAI: change_timer = timer_read(); change_val =  1; return false;
            case RGB_VAD: change_timer = timer_read(); change_val = -1; return false;
            case RGB_RST:
                rgblight_sethsv(HSV_DEFAULT);
                break;
            case RGB_HUG:
                send_string(itoa(rgblight_get_hue(), hsl_buffer, 10));
                break;
            case RGB_SAG:
                send_string(itoa(rgblight_get_sat(), hsl_buffer, 10));
                break;
            case RGB_VAG:
                send_string(itoa(rgblight_get_val(), hsl_buffer, 10));
                break;
            case KC_BKTK_ESCAPE:  // One-key backtick/escape (see also key release)
                escape_timer = timer_read();
                break;
            case KC_UNICODE:
                // on press: switch to UNICODE layer, depress option + leave it down
                // see also key release
                layer_on(UNICODE);
                SEND_STRING(SS_DOWN(X_LOPT));
                break;
            case KC_CYCLE_INPUTS:
                // on press: control-option-space
                // only accessed from UNICODE layer, where option is already depressed
                // then option key up and down to reset for unicode keystrokes
                SEND_STRING(SS_LCTL(" ") SS_UP(X_LOPT) SS_DOWN(X_LOPT));
                break;
            case KC_TOGGLE_INPUTS:
                // on press: control-space
                // only accessed from UNICODE layer, where option is already depressed
                SEND_STRING(SS_UP(X_LOPT) SS_LCTL(" ") SS_DOWN(X_LOPT));
                break;
            case KC_SWITCH:
                // on press: depress command if not already depressed; tap tab
                // command is released when switching back to default layer
                if (!command_tracker) {
                    register_code(KC_LCMD);
                    command_tracker = true;
                }
                tap_code(KC_TAB);
                break;
        }
    } else {
        // on release...
        bool rgb_done = false;
        switch (keycode) {
            case RGB_HUI:
            case RGB_HUD: change_hue = 0; rgb_done = true; break;
            case RGB_SAI:
            case RGB_SAD: change_sat = 0; rgb_done = true; break;
            case RGB_VAI:
            case RGB_VAD: change_val = 0; rgb_done = true; break;
            case KC_BKTK_ESCAPE:  // One key backtick/escape
                if (timer_elapsed(escape_timer) > TAPPING_TERM) {  // Hold, escape
                    tap_code16(KC_ESCAPE);
                } else { // Tap, backtick
                    tap_code16(KC_GRAVE);
                }
                break;
            case KC_UNICODE:
                // on release: release option and deactivate UNICODE layer
                SEND_STRING(SS_UP(X_LOPT));
                layer_off(UNICODE);
                break;
        }
        if (rgb_done) {
            HSV final = rgblight_get_hsv();
            rgblight_sethsv(final.h, final.s, final.v);
        }
    }
    return true;
}

void matrix_scan_user(void) {
    if (change_hue != 0 || change_val != 0 || change_sat != 0) {
        if (timer_elapsed(change_timer) > change_tick) {
            HSV hsv = rgblight_get_hsv();
            hsv.h += change_hue;
            hsv.s = change_sat > 0 ? qadd8(hsv.s, (uint8_t) change_sat) : qsub8(hsv.s, (uint8_t) -change_sat);
            hsv.v = change_val > 0 ? qadd8(hsv.v, (uint8_t) change_val) : qsub8(hsv.v, (uint8_t) -change_val);
            rgblight_sethsv_noeeprom(hsv.h, hsv.s, hsv.v);
            change_timer = timer_read();
        }
    }
}

layer_state_t layer_state_set_kb(layer_state_t state) {
    switch (get_highest_layer(state)) {
        // clear command after returning from NAV layer after KC_SWITCH
        case COLEMAK:
            if (command_tracker) {
                unregister_code(KC_LCMD);
                command_tracker = false;
            }
            break;
        }
    return state;
}

#ifdef LEADER_ENABLE
bool is_alt_tab_active = false;
uint16_t alt_tab_timer = 0;

LEADER_EXTERNS();

void matrix_scan_user(void) {
    if (is_alt_tab_active) {
        if (timer_elapsed(alt_tab_timer) > 1000) {
            unregister_code(KC_LALT);
            is_alt_tab_active = false;
        }
    }

    LEADER_DICTIONARY() {
        leading = false;
        leader_end();

        SEQ_ONE_KEY(KC_C) { // Inline Code
            SEND_STRING("`` " SS_TAP(X_LEFT) SS_TAP(X_LEFT));
        }
        SEQ_ONE_KEY(KC_P) { // Invoke Password Manager
            SEND_STRING(SS_LCTRL(SS_LALT("\\")));
        }
        SEQ_ONE_KEY(KC_S) { // Windows screenshot
            SEND_STRING(SS_LGUI("\nS"));
        }
        SEQ_TWO_KEYS(KC_E, KC_P) { // Email personal
            SEND_STRING("benjamin.e.wolfe@gmail.com");
        }
        SEQ_TWO_KEYS(KC_B, KC_C) { // Discord bongocat
            SEND_STRING(":bongocat:\n");
        }
        SEQ_TWO_KEYS(KC_C, KC_B) { // Discord code block
            SEND_STRING("```c" SS_LSFT("\n\n") "``` " SS_TAP(X_UP));
        }
        SEQ_TWO_KEYS(KC_Y, KC_S) { // Greeting
            SEND_STRING("Best,\n\nBenjamin");
        }
    }
}
#endif

#ifdef OLED_DRIVER_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
	return OLED_ROTATION_180;
}

static void render_kyria_logo(void) {
    static const char PROGMEM kyria_logo[] = {
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,128,128,192,224,240,112,120, 56, 60, 28, 30, 14, 14, 14,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7,  7, 14, 14, 14, 30, 28, 60, 56,120,112,240,224,192,128,128,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,192,224,240,124, 62, 31, 15,  7,  3,  1,128,192,224,240,120, 56, 60, 28, 30, 14, 14,  7,  7,135,231,127, 31,255,255, 31,127,231,135,  7,  7, 14, 14, 30, 28, 60, 56,120,240,224,192,128,  1,  3,  7, 15, 31, 62,124,240,224,192,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,240,252,255, 31,  7,  1,  0,  0,192,240,252,254,255,247,243,177,176, 48, 48, 48, 48, 48, 48, 48,120,254,135,  1,  0,  0,255,255,  0,  0,  1,135,254,120, 48, 48, 48, 48, 48, 48, 48,176,177,243,247,255,254,252,240,192,  0,  0,  1,  7, 31,255,252,240,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,255,255,255,  0,  0,  0,  0,  0,254,255,255,  1,  1,  7, 30,120,225,129,131,131,134,134,140,140,152,152,177,183,254,248,224,255,255,224,248,254,183,177,152,152,140,140,134,134,131,131,129,225,120, 30,  7,  1,  1,255,255,254,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0,255,255,  0,  0,192,192, 48, 48,  0,  0,240,240,  0,  0,  0,  0,  0,  0,240,240,  0,  0,240,240,192,192, 48, 48, 48, 48,192,192,  0,  0, 48, 48,243,243,  0,  0,  0,  0,  0,  0, 48, 48, 48, 48, 48, 48,192,192,  0,  0,  0,  0,  0,
        0,  0,  0,255,255,255,  0,  0,  0,  0,  0,127,255,255,128,128,224,120, 30,135,129,193,193, 97, 97, 49, 49, 25, 25,141,237,127, 31,  7,255,255,  7, 31,127,237,141, 25, 25, 49, 49, 97, 97,193,193,129,135, 30,120,224,128,128,255,255,127,  0,  0,  0,  0,  0,255,255,255,  0,  0,  0,  0, 63, 63,  3,  3, 12, 12, 48, 48,  0,  0,  0,  0, 51, 51, 51, 51, 51, 51, 15, 15,  0,  0, 63, 63,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 48, 48, 63, 63, 48, 48,  0,  0, 12, 12, 51, 51, 51, 51, 51, 51, 63, 63,  0,  0,  0,  0,  0,
        0,  0,  0,  0, 15, 63,255,248,224,128,  0,  0,  3, 15, 63,127,255,239,207,141, 13, 12, 12, 12, 12, 12, 12, 12, 30,127,225,128,  0,  0,255,255,  0,  0,128,225,127, 30, 12, 12, 12, 12, 12, 12, 12, 13,141,207,239,255,127, 63, 15,  3,  0,  0,128,224,248,255, 63, 15,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  3,  7, 15, 62,124,248,240,224,192,128,  1,  3,  7, 15, 30, 28, 60, 56,120,112,112,224,224,225,231,254,248,255,255,248,254,231,225,224,224,112,112,120, 56, 60, 28, 30, 15,  7,  3,  1,128,192,224,240,248,124, 62, 15,  7,  3,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  3,  7, 15, 14, 30, 28, 60, 56,120,112,112,112,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,224,112,112,112,120, 56, 60, 28, 30, 14, 15,  7,  3,  1,  1,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
    };
    oled_write_raw_P(kyria_logo, sizeof(kyria_logo));
}

static void render_qmk_logo(void) {
  static const char PROGMEM qmk_logo[] = {
    0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x8f,0x90,0x91,0x92,0x93,0x94,
    0xa0,0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,0xa9,0xaa,0xab,0xac,0xad,0xae,0xaf,0xb0,0xb1,0xb2,0xb3,0xb4,
    0xc0,0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,0xc9,0xca,0xcb,0xcc,0xcd,0xce,0xcf,0xd0,0xd1,0xd2,0xd3,0xd4,0};

  oled_write_P(qmk_logo, false);
}

static void render_status(void) {
    // QMK Logo and version information
    render_qmk_logo();
    oled_write_P(PSTR("       Kyria rev1.0\n\n"), false);

    // Host Keyboard Layer Status
    oled_write_P(PSTR("Layer: "), false);
    switch (get_highest_layer(layer_state)) {
        case COLEMAK:
            oled_write_P(PSTR("Colemak\n"), false);
            break;
        case NUMBERS:
            oled_write_P(PSTR("Numbers\n"), false);
            break;
        case UNICODE:
            oled_write_P(PSTR("Unicode\n"), false);
            break;
        case NAV:
            oled_write_P(PSTR("Navigation\n"), false);
            break;
        case CHROME:
            oled_write_P(PSTR("Chrome\n"), false);
            break;
        case VSCODE:
            oled_write_P(PSTR("VS Code\n"), false);
            break;
        case ADJUST:
            oled_write_P(PSTR("Adjust\n"), false);
            break;
        default:
            oled_write_P(PSTR("Undefined\n"), false);
    }

    // Host Keyboard LED Status
    uint8_t led_usb_state = host_keyboard_leds();
    oled_write_P(IS_LED_ON(led_usb_state, USB_LED_NUM_LOCK)    ? PSTR("NUMLCK ") : PSTR("       "), false);
    oled_write_P(IS_LED_ON(led_usb_state, USB_LED_CAPS_LOCK)   ? PSTR("CAPLCK ") : PSTR("       "), false);
    oled_write_P(IS_LED_ON(led_usb_state, USB_LED_SCROLL_LOCK) ? PSTR("SCRLCK ") : PSTR("       "), false);
}

void oled_task_user(void) {
    if (is_keyboard_master()) {
        render_status(); // Renders the current keyboard state (layer, lock, caps, scroll, etc)
    } else {
        render_kyria_logo();
    }
}
#endif

#ifdef ENCODER_ENABLE
void encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) {
        switch (biton32(layer_state)) {
            case QWERTY:
                // History scrubbing. For Adobe products, hold shift while moving
                // backward to go forward instead.
                if (clockwise) {
                    tap_code16(C(KC_Z));
                } else {
                    tap_code16(C(KC_Y));
                }
                break;
            default:
                // Switch between windows on Windows with alt tab.
                if (clockwise) {
                    if (!is_alt_tab_active) {
                        is_alt_tab_active = true;
                        register_code(KC_LALT);
                    }
                    alt_tab_timer = timer_read();
                    tap_code16(KC_TAB);
                } else {
                    tap_code16(S(KC_TAB));
                }
                break;
        }
    } else if (index == 1) {
        switch (biton32(layer_state)) {
            case QWERTY:
                // Scrolling with PageUp and PgDn.
                if (clockwise) {
                    tap_code(KC_PGDN);
                } else {
                    tap_code(KC_PGUP);
                }
                break;
            default:
                // Volume control.
                if (clockwise) {
                    tap_code(KC_VOLU);
                } else {
                    tap_code(KC_VOLD);
                }
                break;
        }
    }
}
#endif
