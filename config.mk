# 编译器选项

# 使用 ?= 允许在命令行覆盖
CC ?= gcc
LDFLAGS ?= -lm

# 使用 := 立即展开值
INCLUDE_DIR := $(abspath .)
INCLUDES := -I$(INCLUDE_DIR)/include

# 基础编译选项
BASE_CFLAGS := -std=gnu99 -g -Wall -Wextra -Wno-unused-parameter -Wno-unused-function

# 调试选项
DEBUG ?= 0		# 修改DEBUG选项配置。

ifeq ($(DEBUG),1)
    OPTIMIZATION := -O0 -DDEBUG
else
    OPTIMIZATION := -O2
endif

# 完整编译选项
CFLAGS ?= $(BASE_CFLAGS) $(OPTIMIZATION) $(INCLUDES)

# 导出所有变量，使它们在子Makefile中可用
export CC LDFLAGS INCLUDES INCLUDE_DIR CFLAGS DEBUG