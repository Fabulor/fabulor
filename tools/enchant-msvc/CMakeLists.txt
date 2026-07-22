cmake_minimum_required(VERSION 3.24)
project(fabulor_enchant LANGUAGES C CXX)

if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "This recipe requires a 64-bit MSVC toolchain")
endif()
if(NOT DEFINED ENCHANT_SOURCE_DIR OR NOT DEFINED GTK_ROOT)
  message(FATAL_ERROR "Set ENCHANT_SOURCE_DIR and GTK_ROOT")
endif()
file(TO_CMAKE_PATH "${ENCHANT_SOURCE_DIR}" ENCHANT_SOURCE_DIR)
file(TO_CMAKE_PATH "${GTK_ROOT}" GTK_ROOT)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

set(enchant_composite_source "${ENCHANT_SOURCE_DIR}/lib/composite.c")
file(READ "${enchant_composite_source}" enchant_composite_content)
set(enchant_composite_assignment "_tmp21_ = _tmp20_;")
string(FIND "${enchant_composite_content}" "${enchant_composite_assignment}"
  enchant_composite_assignment_offset)
if(enchant_composite_assignment_offset EQUAL -1)
  message(FATAL_ERROR "Enchant composite.c no longer matches the reviewed MSVC compatibility rewrite")
endif()
string(LENGTH "${enchant_composite_assignment}" enchant_composite_assignment_length)
math(EXPR enchant_composite_tail_offset
  "${enchant_composite_assignment_offset} + ${enchant_composite_assignment_length}")
string(SUBSTRING "${enchant_composite_content}" ${enchant_composite_tail_offset} -1
  enchant_composite_tail)
string(FIND "${enchant_composite_tail}" "${enchant_composite_assignment}"
  enchant_composite_second_assignment_offset)
if(NOT enchant_composite_second_assignment_offset EQUAL -1)
  message(FATAL_ERROR "Enchant composite.c contains multiple reviewed compatibility assignments")
endif()
string(REPLACE "${enchant_composite_assignment}"
  "_tmp21_ = (gchar**) _tmp20_;"
  enchant_composite_content "${enchant_composite_content}")
set(enchant_composite_source "${CMAKE_CURRENT_BINARY_DIR}/composite-msvc.c")
file(WRITE "${enchant_composite_source}" "${enchant_composite_content}")

set(enchant_sources
  "${ENCHANT_SOURCE_DIR}/lib/api.c"
  "${ENCHANT_SOURCE_DIR}/lib/broker.c"
  "${enchant_composite_source}"
  "${ENCHANT_SOURCE_DIR}/lib/dict.c"
  "${ENCHANT_SOURCE_DIR}/lib/provider.c"
  "${ENCHANT_SOURCE_DIR}/lib/provider-dict.c"
  "${ENCHANT_SOURCE_DIR}/lib/pwl.c"
  "${ENCHANT_SOURCE_DIR}/lib/util.c"
  "${CMAKE_CURRENT_LIST_DIR}/compat/flock.c"
  "${CMAKE_CURRENT_LIST_DIR}/compat/relocatable.c"
)

add_library(enchant_core SHARED ${enchant_sources})
set_target_properties(enchant_core PROPERTIES OUTPUT_NAME "libenchant-2-2" PREFIX "")
target_compile_definitions(enchant_core PRIVATE _ENCHANT_BUILD=1 VALA_EXTERN=extern ENCHANT_VERSION_STRING="2.8.19" ENCHANT_MAJOR_VERSION="2")
target_compile_options(enchant_core PRIVATE /FI"${CMAKE_CURRENT_LIST_DIR}/compat/config.h")
target_include_directories(enchant_core PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/compat"
  "${ENCHANT_SOURCE_DIR}/lib"
  "${GTK_ROOT}/include/glib-2.0"
  "${GTK_ROOT}/lib/glib-2.0/include"
)
target_link_directories(enchant_core PRIVATE "${GTK_ROOT}/lib")
target_link_libraries(enchant_core PRIVATE glib-2.0 gobject-2.0 gmodule-2.0 gio-2.0)

add_library(enchant_winspell SHARED "${ENCHANT_SOURCE_DIR}/providers/enchant_winspell.cpp")
set_target_properties(enchant_winspell PROPERTIES PREFIX "")
target_compile_features(enchant_winspell PRIVATE cxx_std_17)
target_compile_options(enchant_winspell PRIVATE /FI"${CMAKE_CURRENT_LIST_DIR}/compat/config.h")
target_include_directories(enchant_winspell PRIVATE
  "${CMAKE_CURRENT_LIST_DIR}/compat"
  "${ENCHANT_SOURCE_DIR}/lib"
  "${GTK_ROOT}/include/glib-2.0"
  "${GTK_ROOT}/lib/glib-2.0/include"
)
target_link_directories(enchant_winspell PRIVATE "${GTK_ROOT}/lib")
target_link_libraries(enchant_winspell PRIVATE enchant_core glib-2.0 gobject-2.0 ole32 oleaut32)

install(TARGETS enchant_core RUNTIME DESTINATION .)
install(TARGETS enchant_winspell RUNTIME DESTINATION lib/enchant-2)
install(FILES "${CMAKE_CURRENT_LIST_DIR}/enchant.ordering" DESTINATION share/enchant-2)

add_executable(enchant_smoke "${CMAKE_CURRENT_LIST_DIR}/smoke.c")
target_compile_options(enchant_smoke PRIVATE /FI"${CMAKE_CURRENT_LIST_DIR}/compat/config.h")
target_include_directories(enchant_smoke PRIVATE "${CMAKE_CURRENT_LIST_DIR}/compat" "${ENCHANT_SOURCE_DIR}/lib")
target_link_libraries(enchant_smoke PRIVATE enchant_core)
install(TARGETS enchant_smoke RUNTIME DESTINATION .)
