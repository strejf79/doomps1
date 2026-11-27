# Convenience wrapper around CMake + PSn00bSDK.
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.
#
# Prerequisites: PSN00BSDK_LIBS set, mipsel-none-elf-gcc on PATH.
# See docs/build-and-run.md and scripts/setup-psn00bsdk.sh.

.PHONY: all configure build clean iso help

BUILD_DIR := build

all: build

help:
	@echo "Targets:"
	@echo "  make configure  - cmake --preset default"
	@echo "  make / make build - build .exe + .cue/.bin"
	@echo "  make iso        - same as build (CD image target)"
	@echo "  make clean      - remove build/"
	@echo ""
	@echo "Requires: PSN00BSDK_LIBS=$(PSN00BSDK_LIBS)"

configure:
	@test -n "$(PSN00BSDK_LIBS)" || (echo "ERROR: set PSN00BSDK_LIBS to .../lib/libpsn00b" && exit 1)
	cmake --preset default .

build: configure
	cmake --build $(BUILD_DIR)

iso: build

clean:
	rm -rf $(BUILD_DIR)
