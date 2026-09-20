# Standalone Stage A SV shell. No legacy objects, generators or executables.
# Invoke from src: make -f makefile.sv tomenet-sv [or tomenet-sv.exe].
.DEFAULT_GOAL := tomenet-sv
PLATFORM ?= linux
CC_LINUX ?= clang
CC_MINGW ?= i686-w64-mingw32-gcc
PKG_CONFIG_LINUX ?= pkg-config
PKG_CONFIG_MINGW ?= i686-w64-mingw32-pkg-config
CFLAGS ?= -O2 -g
CPPFLAGS ?=
LDFLAGS ?=
ifeq ($(PLATFORM),linux)
SV_CC := $(CC_LINUX)
SV_PKG := $(PKG_CONFIG_LINUX)
SV_NAME := tomenet-sv
SV_LINK := -Wl,-z,relro,-z,now -Wl,-rpath,'$$ORIGIN'
else ifeq ($(PLATFORM),mingw)
SV_CC := $(CC_MINGW)
SV_PKG := $(PKG_CONFIG_MINGW)
SV_NAME := tomenet-sv.exe
SV_LINK :=
else
$(error PLATFORM must be linux or mingw)
endif
SV_MODULES := sdl3 sdl3-ttf freetype2
SV_DEPS := $(shell $(SV_PKG) --modversion $(SV_MODULES) 2>/dev/null)
SV_CFLAGS := -std=c99 -Wall -Wextra -Werror -fstack-protector-strong $(CPPFLAGS) $(CFLAGS) $(shell $(SV_PKG) --cflags $(SV_MODULES) 2>/dev/null)
SV_LIBS := $(shell $(SV_PKG) --libs $(SV_MODULES) 2>/dev/null)
# Toolchain, dependency versions and all switches participate in the cache key.
SV_KEY := $(shell printf '%s\n' '$(SV_CC)' '$(shell $(SV_CC) --version | head -1)' '$(SV_CFLAGS)' '$(LDFLAGS)' '$(SV_LIBS)' '$(SV_DEPS)' | sha256sum | cut -c1-20)
SV_OUT := .sv-build/$(PLATFORM)/$(SV_KEY)
SV_OBJECTS := $(addprefix $(SV_OUT)/,main.o font.o)

.PHONY: tomenet-sv tomenet-sv.exe shell check-deps
tomenet-sv:
	$(MAKE) -f makefile.sv PLATFORM=linux shell
tomenet-sv.exe:
	$(MAKE) -f makefile.sv PLATFORM=mingw shell
shell: $(SV_OUT)/$(SV_NAME)
	cp $< $(SV_NAME)
	@echo "SV synthetic shell: $(SV_OUT)/build.txt"
check-deps:
	@$(SV_PKG) --exists 'sdl3 >= 3.2.0' 'sdl3-ttf >= 3.2.0' freetype2 || { echo 'Missing SV SDL3/SDL3_ttf/FreeType development dependencies' >&2; exit 1; }
ifeq ($(PLATFORM),linux)
	@$(SV_CC) -dumpmachine | grep -Eq '^x86_64.*linux' || { echo 'SV Linux target must be amd64' >&2; exit 1; }
else
	@$(SV_CC) -dumpmachine | grep -Eq '^i[3-6]86.*mingw32' || { echo 'SV Windows target must be i686 MinGW32' >&2; exit 1; }
endif
$(SV_OUT)/%.o: client/sv/%.c client/sv/font.h makefile.sv | check-deps
	@mkdir -p $(SV_OUT)
	$(SV_CC) $(SV_CFLAGS) -DSV_BUILD_ID='"$(PLATFORM)-$(SV_KEY)"' -MMD -MP -c $< -o $@
$(SV_OUT)/$(SV_NAME): $(SV_OBJECTS)
	$(SV_CC) $(CFLAGS) $(LDFLAGS) $(SV_LINK) $^ $(SV_LIBS) -o $@
	@printf '%s\n' 'build=$(PLATFORM)-$(SV_KEY)' 'compiler=$(SV_CC)' '$(shell $(SV_CC) --version | head -1)' 'cflags=$(SV_CFLAGS)' 'ldflags=$(LDFLAGS)' 'libs=$(SV_LIBS)' 'dependencies=$(SV_DEPS)' 'features=synthetic-shell,text;network=off;audio=off;terminal=absent' > $(SV_OUT)/build.txt
-include $(SV_OBJECTS:.o=.d)
