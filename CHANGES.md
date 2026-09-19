# 变更记录 — qmk-myfn

本库所有重要改动记录于此。

## [未发布] — 2026-09-19

### 新增
- 初始库：`src/myfn.h`、`src/myfn.c`、`rules.mk`、`readme.md`、`CHANGES.md`、`LICENSE`（MIT）。
- `myfn_active()` —— 当 `MYFN_LAYER` 激活时返回真。
- `layer_state_set_user()` —— 强符号实现，转调 `myfn_fn_status()` 弱钩子，键盘无需再自己写。
- `process_record_myfn()` —— 在 Fn 层激活时拦截 `MYFN_BATTERY_KEY`（默认 `KC_SPC`），
  调用 `myfn_battery()` 钩子。
- 弱「父类」占位 `myfn_fn_status()`、`myfn_battery()`，供键盘覆盖。

### 说明
- 不依赖网络；纯 `SRC +=` 库。
- 电量钩子默认空实现，保证没有电量功能的键盘也能原样编译。
