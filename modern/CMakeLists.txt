cmake_minimum_required(VERSION 3.21)

project(
	psxdoom-modern
	LANGUAGES C ASM
	VERSION 0.1.0
	DESCRIPTION "PSX Doom port for PSn00bSDK"
)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
option(PSXDOOM_SHOW_FPS "Draw the in-game FPS counter" OFF)
option(PSXDOOM_PROFILE "Emit low-rate runtime performance counters" OFF)

add_compile_options(
	-Wall
	-Wextra
	-Wno-unused-parameter
	-march=mips1
	-mtune=r3000
)

psn00bsdk_add_executable(psxdoom-smoke GPREL
	smoke/main.c
	src/psyq_compat.c
)
target_include_directories(psxdoom-smoke PRIVATE ${PROJECT_SOURCE_DIR}/include)

set(DOOM_DIR ${PROJECT_SOURCE_DIR}/../PSXDOOM)
set(DOOM_SOURCES
	d_main.c info.c sprinfo.c g_game.c
	p_base.c p_ceilng.c p_change.c p_doors.c p_enemy.c
	p_floor.c p_inter.c p_lights.c p_map.c p_maputl.c
	p_mobj.c p_move.c p_plats.c p_pspr.c p_setup.c
	p_shoot.c p_sight.c p_slide.c p_spec.c p_switch.c
	p_telept.c p_tick.c p_user.c
	r_phase1.c r_phase2.c r_data.c r_main.c
	vsprintf.c w_wad.c PSX_FILE.C z_zone.c psxmain.c
	l_main.c t_main.c m_main.c c_main.c pw_main.c
	cf_main.c m_password.c st_main.c am_main.c in_main.c
	f_main.c o_main.c m_fixed.c tables.c PSXCD.C s_sound.c
	PSXSPU.C PSXCMD.C WESSAPIP.C WESSAPI.C WESSARC.C
	WESSAPIT.C WESSSEQ.C SEQLOAD.C LCDLOAD.C WESSAPIM.C
	SEQLOADR.C WESSBASE.C
)
list(TRANSFORM DOOM_SOURCES PREPEND "${DOOM_DIR}/")
set_source_files_properties(${DOOM_SOURCES} PROPERTIES LANGUAGE C)
set_source_files_properties(
	"${DOOM_DIR}/r_phase1.c"
	"${DOOM_DIR}/r_phase2.c"
	"${DOOM_DIR}/r_main.c"
	"${DOOM_DIR}/psxmain.c"
	PROPERTIES COMPILE_FLAGS "-O3"
)

psn00bsdk_add_executable(psxdoom GPREL
	${DOOM_SOURCES}
	src/psyq_compat.c
	src/intro_fmv.c
)
target_include_directories(psxdoom PRIVATE
	${PROJECT_SOURCE_DIR}/include
	${DOOM_DIR}
)
target_compile_definitions(psxdoom PRIVATE
	SHOWFPS=$<IF:$<BOOL:${PSXDOOM_SHOW_FPS}>,1,0>
	PSXDOOM_PROFILE=$<IF:$<BOOL:${PSXDOOM_PROFILE}>,1,0>
)
target_compile_options(psxdoom PRIVATE
	-fcommon
	-fno-strict-aliasing
	-fsigned-char
	-fgnu89-inline
	-Wno-implicit-function-declaration
	-Wno-int-conversion
	-Wno-pointer-to-int-cast
	-Wno-int-to-pointer-cast
)

install(FILES
	${PROJECT_BINARY_DIR}/psxdoom-smoke.exe
	${PROJECT_BINARY_DIR}/psxdoom.exe
	TYPE BIN
)
