/* Copyright 2026 springremember
 * SPDX-License-Identifier: MIT
 */
#include "myfn.h"

/* ---- weak "parent" placeholders: the keyboard overrides these ---- */
__attribute__((weak)) void myfn_fn_status(bool on) {
    (void)on;
}

__attribute__((weak)) void myfn_battery(bool pressed) {
    (void)pressed;
}

bool myfn_active(void) {
    return layer_state_cmp(layer_state, MYFN_LAYER);
}

/* Keep the keyboard's Fn flag in sync with the layer state.
 * (Both current keymaps leave layer_state_set_user to this module.) */
layer_state_t layer_state_set_user(layer_state_t state) {
    myfn_fn_status(layer_state_cmp(state, MYFN_LAYER));
    return state;
}

bool process_record_myfn(uint16_t keycode, keyrecord_t *record) {
    if (myfn_active() && keycode == MYFN_BATTERY_KEY) {
        myfn_battery(record->event.pressed);
        return false;
    }
    return true;
}
