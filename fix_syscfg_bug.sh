#!/bin/bash
# Post-SysConfig fix: 修正 GPIO_dirA 引脚宏生成 bug
# PA26 → DL_GPIO_PIN_26, PA27 → DL_GPIO_PIN_27
# 在每次 SysConfig 生成后运行此脚本

HEADER="Debug/ti_msp_dl_config.h"

if [ ! -f "$HEADER" ]; then
    echo "Error: $HEADER not found"
    exit 1
fi

sed -i \
    -e 's/GPIO_dirA_PIN_26_PIN[[:space:]]*(DL_GPIO_PIN_0)/GPIO_dirA_PIN_26_PIN                                    (DL_GPIO_PIN_26)/' \
    -e 's/GPIO_dirA_PIN_27_PIN[[:space:]]*(DL_GPIO_PIN_0)/GPIO_dirA_PIN_27_PIN                                    (DL_GPIO_PIN_27)/' \
    -e 's/PIN_26: GPIOA\.0/PIN_26: GPIOA.26/' \
    -e 's/PIN_27: GPIOA\.0/PIN_27: GPIOA.27/' \
    "$HEADER"

echo "Fixed SysConfig GPIO_dirA bug in $HEADER"
