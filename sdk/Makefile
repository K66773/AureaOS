# Aurea OS SDK: builds the examples into out/.
#   make                 build every example
#   make APP=hello       build one example
# Needs clang, ld.lld, ar and python3. Override CC, LD or AR if they have other names.

# GNU make predefines CC and LD; replace those defaults but keep user overrides.
ifeq ($(origin CC),default)
CC := clang
endif
ifeq ($(origin LD),default)
LD := ld.lld
endif
PYTHON ?= python3

SDK := $(abspath .)
OUT := $(SDK)/out
PACK := $(PYTHON) $(SDK)/tools/aurea_pack.py

CFLAGS := --target=x86_64-unknown-none -ffreestanding -fno-stack-protector -fno-pic \
          -mno-red-zone -nostdlib -O2 -Wall -Wextra -I$(SDK)/include
LDFLAGS := -T $(SDK)/linker/user.ld -nostdlib -static

# crt0 is linked first; the rest goes into libaurea.a so apps only pull in what they use.
LIB_SRC := runtime sys_io libc_shim sha256 ui
LIB_OBJ := $(addprefix $(OUT)/rt/,$(addsuffix .o,$(LIB_SRC)))
LIB := $(OUT)/libaurea.a

APPS ?= $(notdir $(wildcard $(SDK)/examples/*))
ifdef APP
APPS := $(APP)
endif

.PHONY: all clean
.SECONDARY:
all: $(foreach a,$(APPS),$(OUT)/$(a).aex $(OUT)/$(a).run)

$(OUT)/rt/%.o: $(SDK)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(OUT)/%/obj.stamp: $(SDK)/examples/%/*.c
	@mkdir -p $(OUT)/$*
	cd $(OUT)/$* && for f in $(SDK)/examples/$*/*.c; do $(CC) $(CFLAGS) -c $$f -o $$(basename $$f .c).o || exit 1; done
	@touch $@

$(LIB): $(LIB_OBJ)
	rm -f $@
	$(AR) rcs $@ $^

$(OUT)/%.elf: $(OUT)/%/obj.stamp $(OUT)/rt/crt0.o $(LIB)
	$(LD) $(LDFLAGS) $(OUT)/rt/crt0.o $(OUT)/$*/*.o $(LIB) -o $@

# Plain AEX executable (no integrity tag).
$(OUT)/%.aex: $(OUT)/%.elf $(SDK)/examples/%/app.toml
	$(PACK) --format aex --manifest $(SDK)/examples/$*/app.toml --image $< --output $@

# RUN2 bundle with an HMAC integrity tag, started directly with "run".
$(OUT)/%.run: $(OUT)/%.elf $(SDK)/examples/%/app.toml
	$(PACK) --format run2 --sign --deployment standalone --installer none \
		--manifest $(SDK)/examples/$*/app.toml --image $< --output $@

clean:
	rm -rf $(OUT)
