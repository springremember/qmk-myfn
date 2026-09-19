# qmk-myfn

> **myfn 约定**：一份与具体键盘无关的约定，规定「按住 Fn（myfn 键）时，各个键做什么、在什么条件下才做」。
> 本仓库**只有约定，没有代码、没有库**。是否遵守由各键盘自行实现。

---

## 1. 定位

- 本仓库是一份**约定**，不是代码库。
- 键盘的 Fn 键必须改成 **myfn 键**（按住即进入 myfn 层）。
- 键盘**只实现自己能实现的功能**：能力/条件不满足的功能就不实现（见规则）。
- 约定里不含任何具体键盘内容；键位、动作由键盘自行实现。

## 2. 约束表

| 键位 | 功能 | 前置条件 |
| :-- | :-- | :-- |
| Fn+`-` / Fn+`=` | 音量 | 无前置 |
| Fn+Q / W / E | 蓝牙 1 / 2 / 3 | 支持蓝牙；有物理开关须在蓝牙挡 |
| Fn+R | 2.4G | 支持 2.4G；有物理开关须在 2.4G 挡 |
| Fn+T | 切有线 | 键盘无物理切换按键；有物理开关 → 空跑 |
| Fn+Space | 电量 | 有电池 |
| Fn+Esc | 初始化配置 | 无前置 |
| Fn+Caps | 退出 / 开关 Vim | 键盘实现了 Vim |
| 其余键 | 透传 | — |

## 3. 三条规则

1. **前置条件满足** → 执行该功能；
2. **前置条件不满足** → 不执行（吞键 / 空跑，不输出任何东西）；
3. **未在约束表中声明的键** → 透传（照常输出原键）。

## 4. 约束

- 键盘**必须把 Fn 键改成 myfn 键**（按住进入 myfn 层）。
- 键盘**只实现自己能实现的功能**；实现不了的按规则 2 处理。
- 表中“前置条件”是键盘自身的能力/状态，由键盘判断。

## 5. 键盘实现示例

以下为通用伪代码（不含任何具体键盘名），说明约定如何落地。

```c
/* 1) 把 Fn 键改成 myfn 键：base 层该键 = MO(MYFN_LAYER) */

/* 2) myfn 层：键盘能实现的功能才放对应键，其余保持透传 */

/* 3) 按住 myfn 时按三条规则处理 */
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (!myfn_layer_active()) {
        return true;                       /* 不在 myfn 层 → 透传 */
    }

    switch (keycode) {
        case KC_Q:                          /* 蓝牙 1 */
            if (支持蓝牙() && 挡位正确()) {
                切蓝牙(1, record->event.pressed);
            }
            return false;                   /* 前置不满足也吞键 */

        case KC_R:                          /* 2.4G */
            if (支持2G4() && 挡位正确()) {
                切2G4(record->event.pressed);
            }
            return false;

        case KC_T:                          /* 切有线 */
            if (无物理切换按键()) {
                切有线(record->event.pressed);
            }
            return false;                   /* 有物理开关 → 空跑 */

        case KC_SPC:                        /* 电量 */
            if (有电池()) {
                显示电量(record->event.pressed);
            }
            return false;

        case KC_CAPS:                       /* 退出 / 开关 Vim */
            if (有Vim()) {
                开关Vim(record->event.pressed);
            }
            return false;

        /* 音量（- / =）、初始化（Esc）同理 */

        default:
            return true;                    /* 表中没有的键 → 透传 */
    }
}
```

> 说明：以上 `支持蓝牙()`、`挡位正确()`、`有电池()`…… 都是**键盘自己**的判断；
> 约定只规定“什么时候该做、什么时候不做”，不规定“怎么做”。

## 6. 版本记录

见 [`CHANGES.md`](./CHANGES.md)。

## 许可

MIT，见 [`LICENSE`](./LICENSE)。
