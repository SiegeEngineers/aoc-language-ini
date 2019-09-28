#pragma once
#include <stdint.h>

typedef struct string_table_id {
  size_t id;
} string_table_id;

string_table_id aoc_ini_load_strings(char* filename);
void aoc_ini_free_strings(string_table_id table_handle);

void aoc_ini_init();
void aoc_ini_deinit();
