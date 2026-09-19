# qmk-myfn

> 一个可复用的 QMK 小库，用于实现**自定义「新 Fn 层」**。
> 设计目标：把每个键盘都要重复写的样板代码集中到一处，键盘侧只保留「层内容」与
> 少量「父类占位」实现。与 [`qmk-vim`](https://github.com/springremember/qmk-vim) 同构。

---

## 目录

1. [为什么需要它](#1-为什么需要它)
2. [设计原则](#2-设计原则)
3. [目录结构](#3-目录结构)
4. [快速开始](#4-快速开始)
5. [API 详解](#5-api-详解)
6. [层内容示例（QK61 `_FN`）](#6-层内容示例qk61-_fn)
7. [钩子实现示例（QK61）](#7-钩子实现示例qk61)
8. [与 qmk-vim 的关系](#8-与-qmk-vim-的关系)
9. [子模块接入（离线）](#9-子模块接入离线)
10. [构建与验证](#10-构建与验证)
11. [常见问题 FAQ](#11-常见问题-faq)
12. [版本记录](#12-版本记录)

---

## 1. 为什么需要它

很多键盘的「Fn 层」只做少数几件事（音量、无线切换、电量查询……），但每个 keymap
都要重复处理同样的问题：

- **Fn 层是否激活**：需要反复写 `layer_state_cmp(layer_state, X)`。
- **键盘自身的 Fn 标志**：厂商代码常用一个全局标志（如 QK61 的 `Key_Fn_Status`）
  去驱动指示灯、睡眠组合键等；一旦把 Fn 键改指到新层，这个标志就会失效。
- **Fn + 某键 = 电量**：多数方案要在 `process_record_user` 里手写拦截。

`qmk-myfn` 把这些收敛成一个库：

- 统一的 `myfn_active()`；
- 用 `layer_state_set_kb()` 自动维护键盘的 Fn 标志（通过弱钩子回填，并回链 `layer_state_set_user()`）；
- 统一的 `<Fn> + <MYFN_BATTERY_KEY>` 电量拦截（通过弱钩子回填具体显示逻辑）。

**层内容不放进库**：不同键盘的布局宏（`LAYOUT` / `LAYOUT_60_ansi`）、无线键码
（`MD_BLE1` vs `KC_BT1`）都不同，因此层数组留在各自的 keymap，库只提供支撑代码。

## 2. 设计原则

- **通用 + 钩子**：库提供强符号逻辑与**弱符号（weak）父类占位**；键盘实现覆盖弱符号。
- **零默认层号**：`MYFN_LAYER` **没有默认值**，keymap 必须显式定义，否则编译报错，
  杜绝与其它键盘层号冲突 / 误覆盖。
- **可空跑**：没有电量功能的键盘，让 `myfn_battery()` 保持空实现即可原样编译。
- **纯 `SRC +=` 库**：不依赖网络，不需要 QMK 的 module 系统，加入源文件即可。

## 3. 目录结构

```
qmk-myfn/
├─ LICENSE      (MIT)
├─ rules.mk
├─ readme.md
├─ CHANGES.md
└─ src/
   ├─ myfn.h
   └─ myfn.c
```

## 4. 快速开始

**第 1 步：作为子模块加入 keymap 目录**

```
git submodule add git@github.com:springremember/qmk-myfn.git \
    keyboards/<kb>/keymaps/vim/qmk-myfn
```

**第 2 步：keymap 的 `rules.mk` 加入源文件**

```
SRC += qmk-myfn/src/myfn.c
```

**第 3 步：keymap 的 `config.h` 定义层号与电量键（可选）**

```c
/* 必须显式定义，无默认值；用「数值字面量」，不要写 keymap 内的枚举名 _FN
 * （config.h 会先被 myfn.c 看到，那时 _FN 还不存在） */
#define MYFN_LAYER 4
/* 电量键，默认 KC_SPC */
#define MYFN_BATTERY_KEY KC_SPC
```

> 新增层后记得同步提高 VIA 动态键位层数上限，否则会触发 QMK 静态断言：
> ```c
> #define DYNAMIC_KEYMAP_LAYER_COUNT 5   /* 层数 = 原层数 + 1（示例） */
> ```

**第 4 步：keymap 中实现钩子并调用**

```c
#include "qmk-myfn/src/myfn.h"

/* 父类占位：键盘实现 */
void myfn_fn_status(bool on) { Key_Fn_Status = on; }
void myfn_battery(bool pressed) { /* 显示 / 隐藏电量 */ }

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    /* 放在最前，保证 Fn+电量键一定先被拦截 */
    if (!process_record_myfn(keycode, record)) {
        return false;
    }
    ...
}
```

## 5. API 详解

### 5.1 编译期宏

| 宏 | 默认 | 说明 |
| :-- | :-- | :-- |
| `MYFN_LAYER` | **无**（必须定义） | 新 Fn 层的层号。未定义则 `#error`。请在 `config.h` 用**数值字面量**（如 `#define MYFN_LAYER 4`），不要写 keymap 内部的枚举名 `_FN`。 |
| `MYFN_BATTERY_KEY` | `KC_SPC` | 在 Fn 层激活时触发电量显示的键码。 |

### 5.2 `bool myfn_active(void)`

返回当前 `MYFN_LAYER` 是否激活，等价于 `layer_state_cmp(layer_state, MYFN_LAYER)`。

- 可在任意位置调用；
- 对 `MO()` / `LT()` 等瞬时层翻转均有效（由 `layer_state` 驱动）。

### 5.3 弱钩子 `void myfn_fn_status(bool on)`

每当 `MYFN_LAYER` 的激活状态发生变化时被调用一次（进入传 `true`，离开传 `false`）。

- 库中的默认实现为空（`__attribute__((weak))`）；
- 键盘侧实现**强符号**即可覆盖，用来维护自身的 Fn 标志，例如 QK61：
  ```c
  void myfn_fn_status(bool on) { Key_Fn_Status = on; }
  ```
- 该标志常用于：厂商 Fn 指示灯、软睡眠组合键、灯效让位判断等。

### 5.4 弱钩子 `void myfn_battery(bool pressed)`

在 Fn 层按下 `MYFN_BATTERY_KEY` 时被调用 `true`，松开时被调用 `false`。

- 库中的默认实现为空，键盘提供真正的显示逻辑即可；
- 即使「先松开 Fn、再松电量键」，库也会用内部闭锁补一次 `false`（见 §5.5），
  所以键盘实现无需自己处理这个边界；
- 没有电量功能的键盘不必实现（按键被库消费，什么都不输出）。

### 5.5 `bool process_record_myfn(uint16_t keycode, keyrecord_t *record)`

在 `process_record_user()` 中调用。返回 `false` 表示本次事件已被库消费。

- 按下：若 `myfn_active() && keycode == MYFN_BATTERY_KEY`，置内部 `battery_held`，
  调用 `myfn_battery(true)` 并返回 `false`；
- 松开：只要 `battery_held` 为真就调用 `myfn_battery(false)` 并返回 `false`
  （**不再**要求 `myfn_active()`，避免 Fn 先松开导致电量常驻）；
- 此外 `layer_state_set_kb()` 在离开 `MYFN_LAYER` 时会兜底清一次（见 §5.6）。

### 5.6 `layer_state_set_kb()`（强符号）

库在 `src/myfn.c` 中定义了一个**强符号** `layer_state_set_kb()`：维护 Fn 标志、并在
离开 Fn 层时兜底关闭电量显示，最后**回链** `layer_state_set_user()`，因此 keymap 仍可
自定义自己的 user 钩子：

```c
layer_state_t layer_state_set_kb(layer_state_t state) {
    bool active = layer_state_cmp(state, MYFN_LAYER);
    myfn_fn_status(active);                 // 用形参 state，不要读全局 layer_state
    if (!active && myfn_battery_held) {     // 离开 Fn 层兜底清电量
        myfn_battery_held = false;
        myfn_battery(false);
    }
    return layer_state_set_user(state);     // 必须回链，否则 keymap 的 user 钩子被吞
}
```

> ⚠️ **注意**：QMK 的调用链是 `layer_state_set_kb()` → `layer_state_set_user()`。
> 库占用的是 **kb** 级钩子；若键盘自己的 `qk61.c` 也要写 `layer_state_set_kb()`，
> 会与库冲突（此时应把库的逻辑内联合并）。keymap 的 `layer_state_set_user()` 不受影响。

## 6. 层内容示例（QK61 `_FN`）

层数组保留在 keymap。QK61 的新 Fn 层（索引 4）只有音量、无线切换；`Space`
保持 `KC_TRNS`（由库拦截为电量），其余全部透传：

```c
enum layers { _WIN_BASE = 0, _MAC_BASE, _WIN_FN, _MAC_FN, _FN = MYFN_LAYER };

[_FN] = LAYOUT_60_ansi(
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, KC_VOLD, KC_VOLU, _______,
    _______, MD_BLE1, MD_BLE2, MD_BLE3, MD_24G,  _______, _______, _______, _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,
    _______, _______, _______, _______, _______, _______, _______, _______, _______, _______,          _______,          _______,
    _______, _______, _______,                     _______,                              _______, _______, _______,          _______
),
```

要点：

- `-` / `=` → `KC_VOLD` / `KC_VOLU`；
- `Q/W/E/R` → `MD_BLE1/MD_BLE2/MD_BLE3/MD_24G`（厂商无线切换键码）；
- `Space` 保持 `KC_TRNS`，由 `process_record_myfn()` 拦截为电量；
- 基座层的 Fn 键从 `MO(原厂层)` 改为 `MO(_FN)`；
- 原厂 Fn 层仍保留（可仅作参考/回退）。

## 7. 钩子实现示例（QK61）

```c
/* 维护厂商 Fn 标志：驱动 Fn 指示灯、软睡眠组合键、灯效让位 */
void myfn_fn_status(bool on) {
    Key_Fn_Status = on;
}

/* Fn+Space：用厂商数字键 LED 1-10 显示电量 */
void myfn_battery(bool pressed) {
    User_Key_Batt_Num_Show = pressed;
    User_Key_Batt_Count    = 0;
}
```

`myfn_battery()` 里具体调用什么，完全由键盘决定（QK61 用厂商全局标志；NUT65 可用
自己的电量查询键码），这正是「父类占位」的意义。

## 8. 与 qmk-vim 的关系

两者都是「弱钩子 + 键盘覆盖」的同构库，互不依赖：

| | qmk-vim | qmk-myfn |
| :-- | :-- | :-- |
| 作用 | Vim 模式状态机 | 新 Fn 层支撑 |
| 入口 | `process_vim_mode()` | `process_record_myfn()` |
| 钩子 | `*_mode_user()` | `myfn_fn_status()` / `myfn_battery()` |

调用顺序建议（在 `process_record_user()` 内）：

```c
if (!process_record_myfn(keycode, record)) return false;  /* 先拦截 Fn+电量 */
...
if (!process_vim_mode(keycode, record)) return false;      /* 再交给 vim */
```

## 9. 子模块接入（离线）

网络受限时，先用本地仓库克隆、再改 `.gitmodules` 的 URL：

```sh
# 1) 用本地路径添加（避免慢网）
git -c protocol.file.allow=always submodule add \
    /path/to/local/qmk-myfn keyboards/<kb>/keymaps/vim/qmk-myfn

# 2) 把 URL 改回 GitHub
git config -f .gitmodules \
    "submodule.keyboards/<kb>/keymaps/vim/qmk-myfn.url" \
    git@github.com:springremember/qmk-myfn.git
git submodule sync keyboards/<kb>/keymaps/vim/qmk-myfn
```

随后 `git add .gitmodules keyboards/<kb>/keymaps/vim/qmk-myfn` 提交即可。
多个分支（qk61 / nut65）以**同一 commit** 指向本库。

## 10. 构建与验证

```sh
export PATH=/path/to/toolchain/usr/bin:$HOME/.local/bin:$PATH
export QMK_HOME=/path/to/qmk_firmware
cd "$QMK_HOME"
make <keyboard>:<keymap>
```

库通过 `SRC += qmk-myfn/src/myfn.c` 编入，无需其它配置。
若报 `Number of keymap layers exceeds maximum ...`，按第 4 步调大
`DYNAMIC_KEYMAP_LAYER_COUNT`。

## 11. 常见问题 FAQ

**Q：为什么 `MYFN_LAYER` 不给默认值？**
A：默认层号会与其它键盘的层定义撞车、造成误判。改为强制显式定义（否则 `#error`），
把冲突在编译期暴露出来。

**Q：我的键盘没有电量功能，能用吗？**
A：可以。不实现 `myfn_battery()` 即可（库的弱符号为空），Fn+电量键会空跑。

**Q：我想在 keymap 里也写 `layer_state_set_user()` 怎么办？**
A：可以直接写。库占用的是 **kb** 级钩子 `layer_state_set_kb()`，并会回链到你的
`layer_state_set_user()`；两者不冲突。若你要自己写 `layer_state_set_kb()`，才会与库冲突。

**Q：Fn 先松开、再松电量键，会不会一直显示电量？**
A：不会。库内部有 `battery_held` 闭锁，松开时无视层状态也会调用 `myfn_battery(false)`；
并且离开 Fn 层时还会兜底清一次。

**Q：Fn 键要指向新层，厂商的 Fn 逻辑会不会坏？**
A：这正是 `myfn_fn_status()` 的用途——把新层的激活状态回灌给厂商标志，维持原有行为。

**Q：电量键想改成别的键？**
A：在 `config.h` 里 `#define MYFN_BATTERY_KEY <键码>`。

## 12. 版本记录

见 [`CHANGES.md`](./CHANGES.md)。

## 许可

MIT，见 [`LICENSE`](./LICENSE)。
