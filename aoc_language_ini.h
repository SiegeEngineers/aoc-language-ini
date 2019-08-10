#pragma once
#include <windows.h>

HANDLE aoc_ini_load_strings(char* filename);
void aoc_ini_free_strings(HANDLE table_handle);

void aoc_ini_init();
void aoc_ini_deinit();
