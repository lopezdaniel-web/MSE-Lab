## sources.mk
## ---------------------------------------------------------------------------
## Build inputs: sources and includes for Lab Assignment 03.
## New files: adc_driver.c, sensor.c
## Reused from Lab 02: gpio_driver.c, tim_driver.c, pwm.c, timer.c, led.c
## ---------------------------------------------------------------------------

SRC_DIR         = sources
INCLUDE_DIR     = includes
CMSIS_CORE_DIR  = CMSIS/Core/include
CMSIS_STM32_DIR = CMSIS/STM32F4xx/include

## ── Source files (explicit list) ─────────────────────────────────────────
SRC =                               \
    $(SRC_DIR)/main.c               \
    $(SRC_DIR)/STM32_startup.c      \
    $(SRC_DIR)/system_stm32f4xx.c   \
    $(SRC_DIR)/gpio_driver.c        \
    $(SRC_DIR)/tim_driver.c         \
    $(SRC_DIR)/adc_driver.c         \
    $(SRC_DIR)/sensor.c             \
    $(SRC_DIR)/pwm.c                \
    $(SRC_DIR)/timer.c              \
    $(SRC_DIR)/led.c

## ── Include paths ──────────────────────────────────────────────────────────
INCLUDES =              \
    -I$(INCLUDE_DIR)    \
    -I$(CMSIS_CORE_DIR) \
    -I$(CMSIS_STM32_DIR)
