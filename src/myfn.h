/* Copyright 2026 springremember
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include QMK_KEYBOARD_H

/* Layer index that acts as the "new Fn" layer.
 * Override in the keymap's config.h (e.g. #define MYFN_LAYER _FN). */
#ifndef MYFN_LAYER
#    define MYFN_LAYER 4
#endif

/* Key that shows the battery readout while the Fn layer is held. */
#ifndef MYFN_BATTERY_KEY
#    define MYFN_BATTERY_KEY KC_SPC
#endif

/* True while MYFN_LAYER is active. */
bool myfn_active(void);

/* ---- Overridable hooks (weak "parent" defaults live in myfn.c) ---- */

/* Maintain the keyboard's own Fn status flag (e.g. qk61 Key_Fn_Status). */
void myfn_fn_status(bool on);

/* Show / hide the battery readout (called on the battery key press/release). */
void myfn_battery(bool pressed);

/* Call from process_record_user() before the vim engine / mouse handling.
 * Returns false when the event was consumed by the module. */
bool process_record_myfn(uint16_t keycode, keyrecord_t *record);
