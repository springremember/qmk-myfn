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

/* Battery readout latch: the key can be released after Fn, so the release must
 * be handled even when myfn_active() is already false. */
static bool myfn_battery_held = false;

/* Keep the keyboard's Fn flag in sync with the layer state, and force the
 * battery readout off when leaving the Fn layer (belt-and-braces against the
 * "Fn released before the battery key" case).
 * Uses layer_state_set_kb() and chains to layer_state_set_user() so a keymap
 * may still define its own user hook. */
layer_state_t layer_state_set_kb(layer_state_t state) {
    bool active = layer_state_cmp(state, MYFN_LAYER);
    myfn_fn_status(active);
    if (!active && myfn_battery_held) {
        myfn_battery_held = false;
        myfn_battery(false);
    }
    return layer_state_set_user(state);
}

bool process_record_myfn(uint16_t keycode, keyrecord_t *record) {
    if (keycode != MYFN_BATTERY_KEY) {
        return true;
    }
    if (record->event.pressed) {
        if (myfn_active()) {
            myfn_battery_held = true;
            myfn_battery(true);
            return false;
        }
        return true;
    }
    // Release: handle it even if Fn was released first.
    if (myfn_battery_held) {
        myfn_battery_held = false;
        myfn_battery(false);
        return false;
    }
    return true;
}
