# 变更记录 — qmk-myfn

本库所有重要改动记录于此。格式参考 [Keep a Changelog](https://keepachangelog.com/)，
版本遵循语义化版本。

---

## [未发布]

### 修复
- **电量释放闭锁**：`myfn.c` 新增 `myfn_battery_held`。Fn 先松开、再松电量键时，
  之前因 `myfn_active()` 已为假而永不调用 `myfn_battery(false)`，导致电量指示常驻。
  现在松开只看 `battery_held`；并在 `layer_state_set_kb()` 离开 `MYFN_LAYER` 时兜底清一次。
- **钩子解耦**：库的强符号由 `layer_state_set_user()` 改为 `layer_state_set_kb()`，
  并**回链** `layer_state_set_user(state)`，保留 keymap 的 user 钩子扩展点
  （原实现会与想自定义 `layer_state_set_user` 的 keymap 链接冲突）。

### 文档
- `MYFN_LAYER` 示例改为数值字面量（`#define MYFN_LAYER 4`），不再写 keymap 内部枚举名 `_FN`。
- 重写 §5.6（kb 钩子 + 回链）、§5.4/§5.5（电量闭锁）、相关 FAQ。

## [0.1.0] — 2026-09-19

首个版本，配合 QK61 / NUT65 的「新 Fn 层」需求。

### 新增

- **目录与工程文件**
  - `src/myfn.h`、`src/myfn.c`
  - `rules.mk`（提示通过 `SRC += qmk-myfn/src/myfn.c` 接入）
  - `readme.md`（完整中文文档）
  - `CHANGES.md`（本文件）
  - `LICENSE`（MIT）

- **编译期宏**
  - `MYFN_LAYER`：新 Fn 层层号。**无默认值**，未定义时 `#error`，避免层号冲突 / 误覆盖。
  - `MYFN_BATTERY_KEY`：电量触发键，默认 `KC_SPC`。

- **运行时 API**
  - `bool myfn_active(void)`：当 `MYFN_LAYER` 激活时返回真。
  - `bool process_record_myfn(uint16_t keycode, keyrecord_t *record)`：
    在 Fn 层激活时拦截 `MYFN_BATTERY_KEY`，调用 `myfn_battery()` 并消费该事件。

- **强符号接线**
  - `layer_state_set_user()`：库提供强符号实现，层变化时转调 `myfn_fn_status()`，
    键盘无需自己写。若 keymap 需要自定义，必须自行调用
    `myfn_fn_status(myfn_active())`（详见 readme §5.6）。

- **弱「父类」占位（键盘覆盖）**
  - `void myfn_fn_status(bool on)`：默认空实现，键盘用于维护自身 Fn 标志
    （QK61 的 `Key_Fn_Status`）。
  - `void myfn_battery(bool pressed)`：默认空实现，键盘提供真正的电量显示逻辑；
    没有电量功能的键盘留空即可（空跑）。

### 设计说明

- 库是**通用**的；新 Fn 层的**内容**（音量 / 无线 / 其它）保留在各键盘 keymap，
  因为布局宏与无线键码因键盘而异。
- 不依赖网络，不依赖 QMK module 系统；纯 `SRC +=` 引入。
- 与 `qmk-vim` 同构：弱钩子 + 键盘覆盖，多分支以子模块指向同一 commit。

### 已知事项

- 新增层后需同步 `DYNAMIC_KEYMAP_LAYER_COUNT`（启用 VIA 时），否则触发 QMK 静态断言。
- 若同一 keymap 需要自己的 `layer_state_set_user()`，会与库的强符号冲突；处理方式见
  readme §5.6。
