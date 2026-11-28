# doomps1 — from-scratch PS1 Doom-style engine
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.

cmake_minimum_required(VERSION 3.21)

project(
	doomps1
	LANGUAGES C
	VERSION 0.2.0
	DESCRIPTION "From-scratch PS1 Doom-style engine (PAL 320x256)"
)

set(ENGINE_SOURCES
	src/main.c
	src/platform/boot.c
	src/render/gpu.c
	src/pad/pad.c
	src/audio/audio.c
	src/time/tick.c
	src/world/world.c
	src/world/map_tworoom.c
	src/player/player.c
	src/resources/mapbin.c
	src/resources/map_embedded.c
)

psn00bsdk_add_executable(doomps1 GPREL ${ENGINE_SOURCES})

target_include_directories(doomps1 PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(doomps1 PRIVATE ENGINE_PAL=1)

psn00bsdk_add_cd_image(
	doomps1_iso
	doomps1
	iso.xml
	DEPENDS doomps1 system.cnf LICENSE
)

install(FILES ${PROJECT_BINARY_DIR}/doomps1.exe TYPE BIN)
