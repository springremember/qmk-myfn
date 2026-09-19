# qmk-myfn

一个可复用的 QMK 小库，用于实现**自定义「新 Fn 层」**。

它把每个键盘都要重复写的样板代码集中起来：

- 判断新 Fn 层是否激活（`myfn_active()`）；
- 让键盘自身的 Fn 状态标志保持同步（`myfn_fn_status()` 钩子）；
- 把 `<Fn> + <电量键>` 变成电量显示（`myfn_battery()` 钩子）。

库本身是**通用**的；真正的新 Fn 层**内容**（音量 / 无线切换 / 其它）仍留在各键盘的
keymap 里，因为不同键盘的布局并不一样。电量钩子是一个**父类占位**：库提供弱空实现，
键盘各自覆盖；没有电量功能的键盘留空即可（空跑）。

## 目录结构

```
qmk-myfn/
├─ LICENSE      (MIT)
├─ rules.mk
├─ readme.md
└─ src/
   ├─ myfn.h
   └─ myfn.c
```

## API

| 符号 | 类型 | 用途 |
| :-- | :-- | :-- |
| `MYFN_LAYER` | 宏 | 新 Fn 层的层号；**必须显式定义**、无默认值，防止层号冲突 |
| `MYFN_BATTERY_KEY` | 宏 | 触发电量显示的键（默认 `KC_SPC`） |
| `myfn_active()` | 函数 | 当 `MYFN_LAYER` 激活时返回真 |
| `myfn_fn_status(bool on)` | 弱钩子 | 维护键盘自身的 Fn 标志（如 `Key_Fn_Status`） |
| `myfn_battery(bool pressed)` | 弱钩子 | 显示 / 隐藏电量读数 |
| `process_record_myfn()` | 函数 | 在 `process_record_user()` 中、vim 引擎之前调用 |

库提供了一个强符号 `layer_state_set_user()`，内部转调 `myfn_fn_status()`。
如果某个 keymap 自己需要 `layer_state_set_user()`，就必须自行调用
`myfn_fn_status(myfn_active())`，而不能再依赖本库。

## 接入步骤

1. 添加子模块：

   ```
   git submodule add <url> keyboards/<kb>/keymaps/vim/qmk-myfn
   ```

2. 在 keymap 的 `rules.mk`：

   ```
   SRC += qmk-myfn/src/myfn.c
   ```

3. 在 keymap 的 `config.h`：

   ```c
   #define MYFN_LAYER _FN
   ```

4. 在 keymap 中：

   ```c
   #include "qmk-myfn/src/myfn.h"

   void myfn_fn_status(bool on) { Key_Fn_Status = on; }      // 键盘实现
   void myfn_battery(bool pressed) { /* 显示电量 */ }         // 键盘实现

   bool process_record_user(uint16_t keycode, keyrecord_t *record) {
       if (!process_record_myfn(keycode, record)) return false;
       ...
   }
   ```

## 设计说明

- 不依赖网络；纯 `SRC +=` 库。
- 电量钩子默认空实现，保证没有电量功能的键盘也能原样编译。
- 与 `qmk-vim` 同构：弱钩子 + 键盘覆盖，两个分支以子模块指向同一 commit。
