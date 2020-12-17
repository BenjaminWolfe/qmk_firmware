/* Copyright 2019 Benjamin Wolfe <benjamin.e.wolfe@gmail.com>
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

// Wanted to use unicode map for U-2212, true minus
// https://docs.qmk.fm/#/feature_unicode?id=unicode-map
// however, firmware was too large.
// see https://github.com/qmk/qmk_firmware/issues/3224
// via https://thomasbaart.nl/2018/12/01/reducing-firmware-size-in-qmk/

// Tap Dance declarations
// https://docs.qmk.fm/#/feature_tap_dance?id=example-4
// https://www.reddit.com/r/olkb/comments/8i3zq9/qmk_tap_dance_and_momentary_layer_switch/dyzavhu
typedef struct {
    bool is_press_action;
    uint8_t state;
} tap;

enum {
    SINGLE_TAP = 1,
    SINGLE_HOLD,
    DOUBLE_TAP,
    DOUBLE_HOLD,
    DOUBLE_SINGLE_TAP, // Send two single taps
    TRIPLE_TAP,
    TRIPLE_HOLD
};

// Tap dance enums
enum {
    COPY_CUT_PASTE,
    UNDO_REDO,
    FIND_REPLACE
};

uint8_t cur_dance(qk_tap_dance_state_t *state);

// For each advanced tap dance. Put these here so they can be used in any keymap
void copy_cut_paste_finished(qk_tap_dance_state_t *state, void *user_data);
void copy_cut_paste_reset(qk_tap_dance_state_t *state, void *user_data);

uint16_t copy_paste_timer;
static uint8_t command_tracker;

// TODO: Make the layers match the enum.
enum layers {
    COLEMAK = 0,
    _SAFE,
    NUMPAD,
    UNICODE,
    CODING,
    TOP_ROW,
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
    KC_NAV,
    KC_SWITCH
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
/*
 * Base Layer: COLEMAK
 * Line 2 of each key is what you get when you hold it.
 * Layer names are distinguished by CAPS.
 * I try to avoid the inner thumb keys.
 * I find it works really well to have modifiers in the home row.
 * That said, some keys in MacOS already have tap-and-hold functionality;
 *   for those, see the _SAFE layer.
 * Furthermore, since the left hand is home to a lot of command-shortcuts
 *   that are worth being able to do with one hand while holding a mouse,
 *   command has been moved to the thumb.
 * The control-option-shift-toprow order allows easy access to a shifted top row,
 *   and it also works well with the NAV layer.
 *   There, control-option-shift-command allows easy combos
 *   of either command-shift or option-shift, for editing.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * | Backtck |   Q   |   W   |   F   |   P   |   G   |                                  |   J   |   L   |   U   |   Y   |  ; :  |   - _   |
 * |   Esc   | _SAFE |       |       |       |       |                                  |       |       |       |       | _SAFE |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |   Tab   |   A   |   R   |   S   |   T   |   D   |                                  |   H   |   N   |   E   |   I   |   O   |   ' "   |
 * |         |Control|  Opt  | Shift |TOP_ROW|       |                                  |       |TOP_ROW| Shift |  Opt  |Control|         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |Caps Lock|   Z   |   X   |   C   |   V   |   B   |       |       |  |       |       |   K   |   M   |  , <  |  . >  |  / ?  |  Enter  |
 * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       | Space |       |       |  |  Del  |Backspc| Space |       |       |
 *                           |       |       |Command|  NAV  | NUMPD |  | ADJST |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [COLEMAK] = LAYOUT(
      KC_BKTK_ESCAPE, LT(_SAFE, KC_Q), KC_W,         KC_F,         KC_P,              KC_G,                                                             KC_J,   KC_L,              KC_U,         KC_Y,         LT(_SAFE, KC_SCLN), KC_MINS,
      KC_TAB,         LCTL_T(KC_A),    LOPT_T(KC_R), LSFT_T(KC_S), LT(TOP_ROW, KC_T), KC_D,                                                             KC_H,   LT(TOP_ROW, KC_N), RSFT_T(KC_E), ROPT_T(KC_I), RCTL_T(KC_O),       KC_QUOT,
      KC_CAPS,        KC_Z,            KC_X,         KC_C,         KC_V,              KC_B,           _______, _______,    _______,            _______, KC_K,   KC_M,              KC_COMM,      KC_DOT,       KC_SLSH,            KC_ENTER,
                                                     _______,      _______,           LCMD_T(KC_SPC), KC_NAV,  MO(NUMPAD), LT(ADJUST, KC_DEL), KC_BSPC, KC_SPC, _______,           _______
    ),
/*
 * Safe Layer: no mod-tap keys
 * This way you can take advantage of special MacOS tap-and-hold keys:
 * `luy` (top row, RH), `asneio` (home row), and `zc` (bottom row, LH).
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |   Q   |   W   |   F   |   P   |   G   |                                  |   J   |   L   |   U   |   Y   |  ; :  |   - _   |
 * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |   A   |   R   |   S   |   T   |   D   |                                  |   H   |   N   |   E   |   I   |   O   |   ' "   |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |   Z   |   X   |   C   |   V   |   B   |       |       |  |       |       |   K   |   M   |  , <  |  . >  |  / ?  |         |
 * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [_SAFE] = LAYOUT(
      _______, KC_Q, KC_W, KC_F,    KC_P,    KC_G,                                        KC_J,    KC_L,    KC_U,    KC_Y,    KC_SCLN, KC_MINS,
      _______, KC_A, KC_R, KC_S,    KC_T,    KC_D,                                        KC_H,    KC_N,    KC_E,    KC_I,    KC_O,    KC_QUOT,
      _______, KC_Z, KC_X, KC_C,    KC_V,    KC_B,    _______, _______, _______, _______, KC_K,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, _______,
                           _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
/*
 * Modified Number Pad
 * intended primarily for *entering numerical values*
 * all symbols can be found in familiar locations on the TOP_ROW layer
 * KC_KP_MINUS doesn't even allow for en- and em-dash but that's acceptable for numbers
 * use CODING layer for easy entering of parens, brackets, curly braces, slashes, and pipe
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |       |       |       |       |                                  |   =   |   7   |   8   |   9   |   /   |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |       |       |UNICODE| CODNG |       |                                  |   +   |   4   |   5   |   6   |   *   |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |       |       |  |       |       |   (   |   1   |   2   |   3   |   )   |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |   ,   |   -   |   0   |   .   |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [NUMPAD] = LAYOUT(
      _______, _______, _______, _______,    _______,    _______,                                             KC_KP_EQUAL, KC_KP_7,   KC_KP_8, KC_KP_9, KC_KP_SLASH,    _______,
      _______, _______, _______, KC_UNICODE, MO(CODING), _______,                                             KC_KP_PLUS,  KC_KP_4,   KC_KP_5, KC_KP_6, KC_KP_ASTERISK, _______,
      _______, _______, _______, _______,    _______,    _______, _______, _______, _______,     _______,     KC_LPRN,     KC_KP_1,   KC_KP_2, KC_KP_3, KC_RIGHT_PAREN, _______,
                                 _______,    _______,    _______, _______, _______, KC_KP_COMMA, KC_KP_MINUS, KC_KP_0,     KC_KP_DOT, _______
    ),
/*
 * Unicode Pad
 * intended primarily for *entering unicode hex values*
 * KC_UNICODE on the NUMPAD layer, when tapped and held, will activate this layer *and hold the option key*
 * "Cycle inputs" to cycle through keymap inputs. You have to have enabled unicode input on your Mac.
 * "Toggle inputs" to switch back to the one you were just on.
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |       |       |       |       |                                  |   A   |   7   |   8   |   9   |   D   |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |                                  |   B   |   4   |   5   |   6   |   E   |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |       |       |  |       |       |   C   |   1   |   2   |   3   |   F   |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  | Toggl | Cycle |   0   |       |       |
 *                           |       |       |       |       |       |  | Inpts | Inpts |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [UNICODE] = LAYOUT(
      _______, _______, _______, _______, _______, _______,                                                      KC_A,    KC_KP_7, KC_KP_8, KC_KP_9, KC_D, _______,
      _______, _______, _______, _______, _______, _______,                                                      KC_B,    KC_KP_4, KC_KP_5, KC_KP_6, KC_E, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,         KC_C,    KC_KP_1, KC_KP_2, KC_KP_3, KC_F, _______,
                                 _______, _______, _______, _______, _______, KC_TOGGLE_INPUTS, KC_CYCLE_INPUTS, KC_KP_0, _______, _______
    ),
/*
 * Common Coding Symbols
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |       |       |       |       |                                  |       |   [   |   ]   |   \   |       |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |                                  |       |   (   |   )   |   /   |       |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |       |       |       |       |       |       |  |       |       |       |   {   |   }   |   |   |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [CODING] = LAYOUT(
      _______, _______, _______, _______, _______, _______,                                     _______, KC_LBRC, KC_RBRC, KC_BSLS, _______, _______,
      _______, _______, _______, _______, _______, _______,                                     _______, KC_LPRN, KC_RPRN, KC_PSLS, _______, _______,
      _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_LCBR, KC_RCBR, KC_PIPE, _______, _______,
                                 _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
    ),
 /*
  * Numbers and Symbols (traditional top row)
  * Shows numbers and symbols in a modified top-row pattern, for password muscle memory and such
  * Pinky gets plus and equal sign because hyphen/underscore are already on the default layer.
  *
  * ,-------------------------------------------------.                                  ,-------------------------------------------------.
  * |   ` ~   |  1 !  |  2 @  |  3 #  |  4 $  |  5 %  |                                  |  6 ^  |  7 &  |  8 *  |  9 (  |  0 )  |   = +   |
  * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
  * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
  * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           `---------------------------------------'  `---------------------------------------'
  */
     [TOP_ROW] = LAYOUT(
       KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,                                        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_EQUAL,
       _______, _______, _______, _______, _______, _______,                                     _______, _______, _______, _______, _______, _______,
       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,
                                  _______, _______, _______, _______, _______, _______, _______, _______, _______, _______
     ),
/*
 * Navigation Layer
 * Page up and down are not much used on a Mac, or even home and end.
 * Duplicated down arrow because I always forget it's in the home row.
 * Duplicated left and right arrows in the thumb row by analogy to backspace and delete in the default layer.
 * KC_SWITCH is for switching tabs on a Mac.
 *     When pressed the first time, it depresses command and leaves it down, and then taps tab.
 *     Any other time it will just press tab.
 *     Command will be released when dropping out of the layer.
 * use CHROME and VSCODE layers to navigate between tabs
 * COPY_CUT_PASTE: tap to copy, double-tap to cut, hold to paste.
 * UNDO_REDO: tap to undo (command-z), double-tap to redo (command-shift-z).
 * FIND_REPLACE: tap to find (command-f), double-tap to replace (command-option-f, as defined in VS Code defaults).
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |Cmd-Bactc| Cmd-Q | Cmd-W |VSCODE |CHROME |       |                                  |       |       |   ↑   |       |       |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |   Esc   |Control|  Opt  | Shift |Command| Swtch |                                  |       |   ←   |   ↓   |   →   |       |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       | Find  | Undo  |  Cpy  |       |       |       |  |       |       |       |       |       |       |       |         |
 * |         |       | Repl  | Redo  |  Cut  |       |       |       |  |       |       |       |       |   ↓   |       |       |         |
 * |         |       |       |       | Paste |       |       |       |  |       |       |       |       |       |       |       |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |   ←   |       |   →   |       |
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [NAV] = LAYOUT(
      LCMD(KC_GRAVE), LCMD(KC_Q), LCMD(KC_W),       MO(VSCODE),    MO(CHROME),         _______,                                     _______, _______, KC_UP,   _______, _______, _______,
      KC_ESC,         KC_LCTL,    KC_LOPT,          KC_LSFT,       KC_LCMD,            KC_SWITCH,                                   _______, KC_LEFT, KC_DOWN, KC_RGHT, _______, _______,
      _______,        _______,    TD(FIND_REPLACE), TD(UNDO_REDO), TD(COPY_CUT_PASTE), _______, _______, _______, _______, _______, _______, _______, KC_DOWN, _______, _______, _______,
                                                    _______,       _______,            _______, _______, _______, _______, KC_LEFT, _______, KC_RGHT, _______
    ),
 /*
  * Chrome
  *
  * ,-------------------------------------------------.                                  ,-------------------------------------------------.
  * |         |       |       |       |       |       |                                  |       |       |       |       |       |         |
  * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |                                  |       |  Nxt  |       |  Lst  |       |         |
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
  * |         |       |       |       |       |       |                                  |       |  Nxt  |       |  Lst  |       |         |
  * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
  * |         |       |       |       |       |       |       |       |  |       |       |       |       |       |       |       |         |
  * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           |       |       |       |       |       |  |       |       |       |       |       |
  *                           `---------------------------------------'  `---------------------------------------'
  */
     [VSCODE] = LAYOUT(
       _______, _______, _______, _______, _______, _______,                                     _______, _______,             _______, _______,             _______, _______,
       _______, _______, _______, _______, _______, _______,                                     _______, LOPT(LCMD(KC_LEFT)), _______, LOPT(LCMD(KC_RGHT)), _______, _______,
       _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,             _______, _______,             _______, _______,
                                  _______, _______, _______, _______, _______, _______, _______, _______, _______,             _______
     ),
/*
 * Adjust Layer
 * There's so much more we can do here and so many ways to organize it.
 * It's worth splitting this into multiple layers and considering the UX.
 * But for now...
 *
 * ,-------------------------------------------------.                                  ,-------------------------------------------------.
 * |         |       |       |       |       |       |                                  |       |  F7   |  F8   |  F9   |  F10  |         |
 * |---------+-------+-------+-------+-------+-------|                                  |-------+-------+-------+-------+-------+---------|
 * |         |       |  HUI  |  SAI  |  VAI  |       |                                  |       |  F4   |  F5   |  F6   |  F11  |         |
 * |---------+-------+-------+-------+-------+-------+---------------.  ,---------------+-------+-------+-------+-------+-------+---------|
 * |         |       |  HUD  |  SAD  |  VAD  |       |       | VolUp |  |       |       |       |  F1   |  F2   |  F3   |  F12  |         |
 * `-------------------------+-------+-------+-------+-------+-------|  |-------+-------+-------+-------+-------+-------------------------'
 *                           |       |       |       |       |       |  |       |       |       |       |       |
 *                           |  Prev |  Play |  Next |  Mute | VolDn |  |       |       |       |       |       |
 *                           `---------------------------------------'  `---------------------------------------'
 */
    [ADJUST] = LAYOUT(
      _______, _______, _______, _______, _______, _______,                                     _______, KC_F7,   KC_F8,   KC_F9,   KC_F10,  _______,
      _______, _______, RGB_HUI, RGB_SAI, RGB_VAI, _______,                                     _______, KC_F4,   KC_F5,   KC_F6,   KC_F11,  _______,
      _______, _______, RGB_HUD, RGB_SAD, RGB_VAD, _______, _______, KC_VOLU, _______, _______, _______, KC_F1,   KC_F2,   KC_F3,   KC_F12,  _______,
                                 KC_MPRV, KC_MPLY, KC_MNXT, KC_MUTE, KC_VOLD, _______, _______, _______, _______, _______
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

// https://docs.qmk.fm/#/feature_tap_dance?id=example-4
/* Return an integer that corresponds to what kind of tap dance should be executed.
 *
 * How to figure out tap dance state: interrupted and pressed.
 *
 * Interrupted: If the state of a dance dance is "interrupted", that means that another key has been hit
 *  under the tapping term. This is typically indicative that you are trying to "tap" the key.
 *
 * Pressed: Whether or not the key is still being pressed. If this value is true, that means the tapping term
 *  has ended, but the key is still being pressed down. This generally means the key is being "held".
 *
 * One thing that is currently not possible with qmk software in regards to tap dance is to mimic the "permissive hold"
 *  feature. In general, advanced tap dances do not work well if they are used with commonly typed letters.
 *  For example "A". Tap dances are best used on non-letter keys that are not hit while typing letters.
 *
 * Good places to put an advanced tap dance:
 *  z,q,x,j,k,v,b, any function key, home/end, comma, semi-colon
 *
 * Criteria for "good placement" of a tap dance key:
 *  Not a key that is hit frequently in a sentence
 *  Not a key that is used frequently to double tap, for example 'tab' is often double tapped in a terminal, or
 *    in a web form. So 'tab' would be a poor choice for a tap dance.
 *  Letters used in common words as a double. For example 'p' in 'pepper'. If a tap dance function existed on the
 *    letter 'p', the word 'pepper' would be quite frustating to type.
 *
 * For the third point, there does exist the 'DOUBLE_SINGLE_TAP', however this is not fully tested
 *
 */
// To activate SINGLE_HOLD, you will need to hold for 200ms first.
// This tap dance favors keys that are used frequently in typing like 'f'
uint8_t cur_dance(qk_tap_dance_state_t *state) {
    if (state->count == 1) {
        if (state->interrupted || !state->pressed) return SINGLE_TAP;
        // Key has not been interrupted, but the key is still held. Means you want to send a 'HOLD'.
        else return SINGLE_HOLD;
    } else if (state->count == 2) {
        // DOUBLE_SINGLE_TAP is to distinguish between typing "pepper", and actually wanting a double tap
        // action when hitting 'pp'. Suggested use case for this return value is when you want to send two
        // keystrokes of the key, and not the 'double tap' action/macro.
        if (state->interrupted) return DOUBLE_SINGLE_TAP;
        else if (state->pressed) return DOUBLE_HOLD;
        else return DOUBLE_TAP;
    }

    // Assumes no one is trying to type the same letter three times (at least not quickly).
    // If your tap dance key is 'KC_W', and you want to type "www." quickly - then you will need to add
    // an exception here to return a 'TRIPLE_SINGLE_TAP', and define that enum just like 'DOUBLE_SINGLE_TAP'
    if (state->count == 3) {
        if (state->interrupted || !state->pressed) return TRIPLE_TAP;
        else return TRIPLE_HOLD;
    } else return 8; // Magic number. At some point this method will expand to work for more presses
}

// Tap Dance definitions
// Create an instance of 'tap' for each tap dance.
// see QUAD FUNCTION FOR TAB in https://github.com/qmk/qmk_firmware/blob/master/users/gordon/gordon.c
static tap copy_cut_paste_tap_state = {
    .is_press_action = true,
    .state = 0
};

void copy_cut_paste_finished(qk_tap_dance_state_t *state, void *user_data) {
    // DOUBLE_HOLD and DOUBLE_SINGLE_TAP are placeholders
    copy_cut_paste_tap_state.state = cur_dance(state);
    switch (copy_cut_paste_tap_state.state) {
        case SINGLE_TAP: register_code16(LCMD(KC_C)); break;  // command-c: copy
        case SINGLE_HOLD: register_code16(LCMD(KC_V)); break; // command-v: paste
        case DOUBLE_TAP: register_code16(LCMD(KC_X)); break;  // command-x: cut
        case DOUBLE_HOLD: register_code16(LCMD(KC_X)); break; // same as double-tap
        // Last case is for fast typing. Assuming your key is `f`:
        // For example, when typing the word `buffer`, and you want to make sure that you send `ff` and not `Esc`.
        // In order to type `ff` when typing fast, the next character will have to be hit within the `TAPPING_TERM`, which by default is 200ms.
        case DOUBLE_SINGLE_TAP: tap_code16(LCMD(KC_X)); register_code16(LCMD(KC_X)); break; // same as double-tap
    }
}

void copy_cut_paste_reset(qk_tap_dance_state_t *state, void *user_data) {
    // DOUBLE_HOLD and DOUBLE_SINGLE_TAP are placeholders
    switch (copy_cut_paste_tap_state.state) {
        case SINGLE_TAP: unregister_code16(LCMD(KC_C)); break;  // command-c: copy
        case SINGLE_HOLD: unregister_code16(LCMD(KC_V)); break; // command-v: paste
        case DOUBLE_TAP: unregister_code16(LCMD(KC_X)); break;  // command-x: cut
        case DOUBLE_HOLD: unregister_code16(LCMD(KC_X)); break;        // same as double-tap
        case DOUBLE_SINGLE_TAP: unregister_code16(LCMD(KC_X)); break;  // same as double-tap
    }
    copy_cut_paste_tap_state.state = 0;
}

qk_tap_dance_action_t tap_dance_actions[] = {
    [COPY_CUT_PASTE] = ACTION_TAP_DANCE_FN_ADVANCED(NULL, copy_cut_paste_finished, copy_cut_paste_reset),
    [UNDO_REDO] = ACTION_TAP_DANCE_DOUBLE(LCMD(KC_Z), SCMD(KC_Z)),
    [FIND_REPLACE] = ACTION_TAP_DANCE_DOUBLE(LCMD(KC_F), LOPT(LCMD(KC_F))),
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_BKTK_ESCAPE:  // One key backtick/escape
            if (record->event.pressed) {
                copy_paste_timer = timer_read();
            } else {
                if (timer_elapsed(copy_paste_timer) > TAPPING_TERM) {  // Hold, escape
                    tap_code16(KC_ESCAPE);
                } else { // Tap, backtick
                    tap_code16(KC_GRAVE);
                }
            }
            break;
        case KC_UNICODE:
            if (record->event.pressed) {
                // on press: switch to UNICODE layer, depress option and leave it down
                layer_on(UNICODE);
                SEND_STRING(SS_DOWN(X_LOPT));
            } else {
                // on release: release option and deactivate UNICODE layer
                SEND_STRING(SS_UP(X_LOPT));
                layer_off(UNICODE);
            }
            break;
        case KC_CYCLE_INPUTS:
            if (record->event.pressed) {
                // on press: control-option-space
                // only accessed from UNICODE layer, where option is already depressed
                // then option key up and down to reset for unicode keystrokes
                SEND_STRING(SS_LCTL(" ") SS_UP(X_LOPT) SS_DOWN(X_LOPT));
            } else {
                // on release: nothing
            }
            break;
        case KC_TOGGLE_INPUTS:
            if (record->event.pressed) {
                // on press: control-space
                // only accessed from UNICODE layer, where option is already depressed
                SEND_STRING(SS_UP(X_LOPT) SS_LCTL(" ") SS_DOWN(X_LOPT));
            } else {
                // on release: nothing
            }
            break;
        case KC_NAV:
            if (record->event.pressed) {
                // on press: turn NAV layer on
                layer_on(NAV);
            } else {
                // on release: turn NAV layer off, *and release command if needed*,
                // in case KC_SWITCH depressed it for window-switching (see below)
                if (command_tracker) {
                    unregister_code(KC_LCMD);
                    command_tracker--;
                }
                layer_off(NAV);
            }
            break;
        case KC_SWITCH:
            if (record->event.pressed) {
                // on press: depress command if not already depressed; tap tab
                if (!command_tracker) {
                    register_code(KC_LCMD);
                    command_tracker++;
                }
                tap_code(KC_TAB);
            } else {
                // on release: nothing
            }
            break;
    }
    return true;
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

// https://github.com/qmk/qmk_firmware/blob/master/quantum/rgblight_list.h#L55
// picked a nice blue with max saturation and luminance, rest are evenly spaced hues
#define HSV_BLUISH 152, 255, 255
#define HSV_COLOR1 184, 255, 255
#define HSV_COLOR2 216, 255, 255
#define HSV_COLOR3 248, 255, 255
#define HSV_COLOR4  24, 255, 255
#define HSV_COLOR5  56, 255, 255
#define HSV_COLOR6  88, 255, 255
#define HSV_COLOR7 120, 255, 255

#ifdef RGBLIGHT_ENABLE
// Lighting layers
// https://docs.qmk.fm/#/feature_rgblight?id=lighting-layers
/* Popular colors to try:
    HSV_WHITE
    HSV_RED
    HSV_CORAL
    HSV_ORANGE
    HSV_GOLDENROD
    HSV_GOLD
    HSV_YELLOW
    HSV_CHARTREUSE
    HSV_GREEN
    HSV_SPRINGGREEN
    HSV_TURQUOISE
    HSV_TEAL
    HSV_CYAN
    HSV_AZURE
    HSV_BLUE
    HSV_PURPLE
    HSV_MAGENTA
    HSV_PINK
*/
const rgblight_segment_t PROGMEM color_layer_1[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_BLUISH}
);
const rgblight_segment_t PROGMEM color_layer_2[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR1}
);
const rgblight_segment_t PROGMEM color_layer_3[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR2}
);
const rgblight_segment_t PROGMEM color_layer_4[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR3}
);
const rgblight_segment_t PROGMEM color_layer_5[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR4}
);
const rgblight_segment_t PROGMEM color_layer_6[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR5}
);
const rgblight_segment_t PROGMEM color_layer_7[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR6}
);
const rgblight_segment_t PROGMEM color_layer_8[] = RGBLIGHT_LAYER_SEGMENTS(
    {1, 20, HSV_COLOR7}
);

// Now define the array of layers. Later layers take precedence
const rgblight_segment_t* const PROGMEM my_rgb_layers[] = RGBLIGHT_LAYERS_LIST(
    color_layer_1,
    color_layer_2,
    color_layer_3,
    color_layer_4,
    color_layer_5,
    color_layer_6,
    color_layer_7,
    color_layer_8
);

void keyboard_post_init_user(void) {
  rgblight_enable_noeeprom(); // Enables RGB, without saving settings
  rgblight_sethsv_noeeprom(HSV_BLUISH);
  rgblight_mode_noeeprom(RGBLIGHT_MODE_STATIC_LIGHT);
  rgblight_layers = my_rgb_layers; // Enable the LED layers
}

layer_state_t layer_state_set_user(layer_state_t state) {
    // Both layers will light up if both kb layers are active
    rgblight_set_layer_state(0, layer_state_cmp(state, 0));
    rgblight_set_layer_state(1, layer_state_cmp(state, 1));
    rgblight_set_layer_state(2, layer_state_cmp(state, 2));
    rgblight_set_layer_state(3, layer_state_cmp(state, 3));
    rgblight_set_layer_state(4, layer_state_cmp(state, 4));
    rgblight_set_layer_state(5, layer_state_cmp(state, 5));
    rgblight_set_layer_state(6, layer_state_cmp(state, 6));
    rgblight_set_layer_state(7, layer_state_cmp(state, 7));
    rgblight_set_layer_state(7, layer_state_cmp(state, 8));
    return state;
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
        case _SAFE:
            oled_write_P(PSTR("Safety\n"), false);
            break;
        case NUMPAD:
            oled_write_P(PSTR("Numpad\n"), false);
            break;
        case UNICODE:
            oled_write_P(PSTR("Unicode\n"), false);
            break;
        case CODING:
            oled_write_P(PSTR("Coding\n"), false);
            break;
        case TOP_ROW:
            oled_write_P(PSTR("Top Row\n"), false);
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
