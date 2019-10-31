cmake_minimum_required(VERSION 3.9.4)

project(aoc-language-ini)

set(CMAKE_SYSTEM_NAME Windows)

if (UNIX)
  set(CMAKE_RC_COMPILE_OBJECT "i686-w64-mingw32-windres <SOURCE> -O coff -o <OBJECT>")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

add_compile_options(-m32)
add_compile_options(-Wall)

add_compile_definitions(WINVER=0x0501)
add_compile_definitions(WIN32_LEAN_AND_MEAN)
add_compile_definitions(NOMINMAX)

if(MSVC)
  foreach(flag_var
      CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif(${flag_var} MATCHES "/MD")
  endforeach(flag_var)
else()
  add_compile_options(-fno-exceptions)
  if (NOT CMAKE_BUILD_TYPE STREQUAL Debug)
    add_link_options(-Wl,--exclude-all-symbols)
    add_link_options(-s)
  endif()
  add_link_options(-static-libstdc++ -static-libgcc)
  add_compile_options(-Wall -Wsign-compare)
endif()

include_directories(include/)

add_executable(getstrings getstrings.c)

set_source_files_properties(strings.rc PROPERTIES LANGUAGE RC)
set(SOURCE_FILES
  aoc_language_ini.c
  hook.c
  main.c
  strings.rc
)

add_library(aoc-language-ini SHARED ${SOURCE_FILES})

if (UNIX)
  set_target_properties(aoc-language-ini PROPERTIES
      PREFIX ""
      SUFFIX ".dll")
endif()
install(TARGETS aoc-language-ini
        LIBRARY DESTINATION lib)

