cmake_minimum_required(VERSION 3.9)
project(LuaJIT VERSION 2.0.1 DESCRIPTION "Lua Just In Time Libarary.")

set(LUAJIT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/upstream/src)

add_library(LuaJIT_lib STATIC
	 ${LUAJIT_SRC_DIR}/host/minilua.c
	 ${LUAJIT_SRC_DIR}/host/buildvm.c
	 ${LUAJIT_SRC_DIR}/host/buildvm_asm.c
	 ${LUAJIT_SRC_DIR}/host/buildvm_peobj.c
	 ${LUAJIT_SRC_DIR}/host/buildvm_lib.c
	 ${LUAJIT_SRC_DIR}/host/buildvm_fold.c

	 ${LUAJIT_SRC_DIR}/lj_gc.c
	 ${LUAJIT_SRC_DIR}/lj_err.c
	 ${LUAJIT_SRC_DIR}/lj_char.c
	 ${LUAJIT_SRC_DIR}/lj_bc.c
	 ${LUAJIT_SRC_DIR}/lj_obj.c
	 ${LUAJIT_SRC_DIR}/lj_str.c
	 ${LUAJIT_SRC_DIR}/lj_tab.c
	 ${LUAJIT_SRC_DIR}/lj_func.c
	 ${LUAJIT_SRC_DIR}/lj_udata.c
	 ${LUAJIT_SRC_DIR}/lj_meta.c
	 ${LUAJIT_SRC_DIR}/lj_debug.c
	 ${LUAJIT_SRC_DIR}/lj_state.c
	 ${LUAJIT_SRC_DIR}/lj_dispatch.c
	 ${LUAJIT_SRC_DIR}/lj_vmevent.c
	 ${LUAJIT_SRC_DIR}/lj_vmmath.c
	 ${LUAJIT_SRC_DIR}/lj_strscan.c
	 ${LUAJIT_SRC_DIR}/lj_api.c
	 ${LUAJIT_SRC_DIR}/lj_lex.c
	 ${LUAJIT_SRC_DIR}/lj_parse.c
	 ${LUAJIT_SRC_DIR}/lj_bcread.c
	 ${LUAJIT_SRC_DIR}/lj_bcwrite.c
	 ${LUAJIT_SRC_DIR}/lj_load.c
	 ${LUAJIT_SRC_DIR}/lj_ir.c
	 ${LUAJIT_SRC_DIR}/lj_opt_mem.c
	 ${LUAJIT_SRC_DIR}/lj_opt_fold.c
	 ${LUAJIT_SRC_DIR}/lj_opt_narrow.c
	 ${LUAJIT_SRC_DIR}/lj_opt_dce.c
	 ${LUAJIT_SRC_DIR}/lj_opt_loop.c
	 ${LUAJIT_SRC_DIR}/lj_opt_split.c
	 ${LUAJIT_SRC_DIR}/lj_opt_sink.c
	 ${LUAJIT_SRC_DIR}/lj_mcode.c
	 ${LUAJIT_SRC_DIR}/lj_snap.c
	 ${LUAJIT_SRC_DIR}/lj_record.c
	 ${LUAJIT_SRC_DIR}/lj_crecord.c
	 ${LUAJIT_SRC_DIR}/lj_ffrecord.c
	 ${LUAJIT_SRC_DIR}/lj_asm.c
	 ${LUAJIT_SRC_DIR}/lj_trace.c
	 ${LUAJIT_SRC_DIR}/lj_gdbjit.c
	 ${LUAJIT_SRC_DIR}/lj_gdbjit.c
	 ${LUAJIT_SRC_DIR}/lj_ctype.c
	 ${LUAJIT_SRC_DIR}/lj_cdata.c
	 ${LUAJIT_SRC_DIR}/lj_cconv.c
	 ${LUAJIT_SRC_DIR}/lj_ccall.c
	 ${LUAJIT_SRC_DIR}/lj_ccallback.c
	 ${LUAJIT_SRC_DIR}/lj_carith.c
	 ${LUAJIT_SRC_DIR}/lj_clib.c
	 ${LUAJIT_SRC_DIR}/lj_cparse.c
	 ${LUAJIT_SRC_DIR}/lj_lib.c
	 ${LUAJIT_SRC_DIR}/lj_alloc.c
	 ${LUAJIT_SRC_DIR}/lib_aux.c
	 ${LUAJIT_SRC_DIR}/lib_base.c
	 ${LUAJIT_SRC_DIR}/lib_math.c
	 ${LUAJIT_SRC_DIR}/lib_bit.c
	 ${LUAJIT_SRC_DIR}/lib_string.c
	 ${LUAJIT_SRC_DIR}/lib_table.c
	 ${LUAJIT_SRC_DIR}/lib_io.c
	 ${LUAJIT_SRC_DIR}/lib_os.c
	 ${LUAJIT_SRC_DIR}/lib_package.c
	 ${LUAJIT_SRC_DIR}/lib_debug.c
	 ${LUAJIT_SRC_DIR}/lib_jit.c
	 ${LUAJIT_SRC_DIR}/lib_ffi.c
	 ${LUAJIT_SRC_DIR}/lib_init.c
	 ${LUAJIT_SRC_DIR}/luajit.c)


target_include_directories(LuaJIT_lib PRIVATE ${LUAJIT_SRC_DIR})


target_include_directories(LuaJIT_lib PRIVATE ${CMAKE_SOURCE_DIR}/include64)
