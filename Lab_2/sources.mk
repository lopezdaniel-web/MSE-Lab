## sources.mk
## ---------------------------------------------------------------------------
## Build inputs: sources and includes for the project.
## To add a new file, add it explicitly to SRC.
## To use all .c files automatically, comment out SRC and uncomment
## the line with $(wildcard ...).
## ---------------------------------------------------------------------------

SRC_DIR         = sources
INCLUDE_DIR     = includes
CMSIS_CORE_DIR  = CMSIS/Core/include
CMSIS_STM32_DIR = CMSIS/STM32F4xx/include

## ── Source files (explicit list – more legible and safe) ─────────────────
SRC =                               \
    $(SRC_DIR)/main.c               \
    $(SRC_DIR)/STM32_startup.c      \
    $(SRC_DIR)/system_stm32f4xx.c   \
    $(SRC_DIR)/gpio_driver.c        \
    $(SRC_DIR)/led.c                \
    $(SRC_DIR)/pwm.c                \
    $(SRC_DIR)/tim_driver.c         \
    $(SRC_DIR)/timer.c

## Auto alternative (decomment if you want to compile all .c files in SRC_DIR, but be careful with unwanted files):
## SRC = $(wildcard $(SRC_DIR)/*.c)

## ── Include paths ──────────────────────────────────────────────────────────
INCLUDES =              \
    -I$(INCLUDE_DIR)    \
    -I$(CMSIS_CORE_DIR) \
    -I$(CMSIS_STM32_DIR)
