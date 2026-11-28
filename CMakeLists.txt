# doomps1 — from-scratch PS1 Doom-style engine
# Copyright (c) 2026 Johannes Brorsson. All Rights Reserved.

cmake_minimum_required(VERSION 3.21)

project(
	doomps1
	LANGUAGES    C ASM
	VERSION 0.6.0
	DESCRIPTION "From-scratch PS1 Doom-style engine (PAL 320x256)"
)

set(ENGINE_SOURCES
	src/main.c
	src/platform/boot.c
	src/platform/disc.c
	src/render/gpu.c
	src/pad/pad.c
	src/audio/audio.c
	src/time/tick.c
	src/world/world.c
	src/world/map_tworoom.c
	src/world/doors.c
	src/player/player.c
	src/resources/mapbin.c
	src/resources/map_embedded.c
	src/entities/monster.c
	src/weapons/hitscan.c
	src/fx/particles.c
	src/items/pickups.c
)

psn00bsdk_add_executable(doomps1 GPREL ${ENGINE_SOURCES})

target_include_directories(doomps1 PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_compile_definitions(doomps1 PRIVATE ENGINE_PAL=1)

# Plan M6: placeholder ADPCM .VAG bank (regenerate via tools/gen_vag_placeholders.py)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_pistol    ${PROJECT_SOURCE_DIR}/assets/sfx/pistol.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_shotgun   ${PROJECT_SOURCE_DIR}/assets/sfx/shotgun.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_chaingun  ${PROJECT_SOURCE_DIR}/assets/sfx/chaingun.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_hurt      ${PROJECT_SOURCE_DIR}/assets/sfx/hurt.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_footstep  ${PROJECT_SOURCE_DIR}/assets/sfx/footstep.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_music_loop ${PROJECT_SOURCE_DIR}/assets/sfx/music_loop.vag)
psn00bsdk_target_incbin(doomps1 PRIVATE sfx_silence   ${PROJECT_SOURCE_DIR}/assets/sfx/silence.vag)

psn00bsdk_add_cd_image(
	doomps1_iso
	doomps1
	iso.xml
	DEPENDS doomps1 system.cnf LICENSE
		${PROJECT_SOURCE_DIR}/assets/disc/map.bin
		${PROJECT_SOURCE_DIR}/assets/disc/prefetch.toc.txt
		${PROJECT_SOURCE_DIR}/assets/disc/sfx_placeholder.bin
)

install(FILES ${PROJECT_BINARY_DIR}/doomps1.exe TYPE BIN)
