#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "hook.h"

#ifdef DEBUG
#  define dbg_print(...) printf("[aoc-language-ini] " __VA_ARGS__)
#else
#  define dbg_print(...)
#endif

typedef struct string_entry {
  int id;
  int size;
  char* value;
} string_entry_t;

typedef struct string_table {
  size_t id;
  char* filename;
  int size;
  int capacity;
  string_entry_t* entries;
} string_table_t;

static string_table_t* string_tables;
static size_t num_string_tables;
static size_t string_table_id;

static char* read_file(char* filename) {
  FILE* file = fopen(filename, "rb");
  if (file == NULL) return NULL;
  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char* content = calloc(1, size + 1);
  content[size] = '\0';
  int read = fread(content, 1, size, file);
  fclose(file);

  if (size != read) {
    free(content);
    return NULL;
  }

  return content;
}

static void expand_string_table (string_table_t* string_table) {
  string_table->capacity += 8192;
  string_table->entries = realloc(string_table->entries,
      string_table->capacity * sizeof(string_entry_t));
}

HANDLE aoc_ini_load_strings (char* filename) {
  dbg_print("load strings: %s\n", filename);

  string_table_t string_table = {
    .id = string_table_id,
    .filename = strdup(filename),
    .size = 0,
    .capacity = 8192
  };

  string_table_id++;

  int id;
  char cur_string[4096];

  char* content = read_file(filename);
  if (!content) return INVALID_HANDLE_VALUE;

  string_table.entries = calloc(string_table.capacity, sizeof(string_entry_t));

  char* read_ptr = strtok(content, "\n");
  while (read_ptr != NULL) {
    if (read_ptr[0] != ';') {
      if (sscanf(read_ptr, "%d=%4096[^\n\r]", &id, cur_string) == 2) {
        dbg_print("found string %d: '%s'\n", id, cur_string);
        string_table.entries[string_table.size].id = id;
        string_table.entries[string_table.size].size = strlen(cur_string);
        string_table.entries[string_table.size].value = strdup(cur_string);

        string_table.size++;
        if (string_table.size >= string_table.capacity) {
          expand_string_table(&string_table);
        }
      }
    }
    read_ptr = strtok(NULL, "\n");
  }

  if (string_table.size < string_table.capacity) {
    // just to be safe, zero out the rest
    memset(&string_table.entries[string_table.size], 0,
        (string_table.capacity - string_table.size) * sizeof(string_entry_t));
  }

  if (string_tables == NULL) {
    string_tables = calloc(1, sizeof(string_table_t));
    num_string_tables = 1;
  } else {
    num_string_tables += 1;
    string_tables = realloc(string_tables, num_string_tables * sizeof(string_table_t));
  }

  memcpy(&string_tables[num_string_tables - 1], &string_table, sizeof(string_table));

  return (HANDLE) string_table.id;
}

static void free_string_table_entries (string_table_t* table) {
  dbg_print("free entries: %s\n", table->filename);
  for (int i = 0; i < table->size; i++) {
    free(table->entries[i].value);
  }
  free(table->entries);
  table->entries = NULL;
  table->size = 0;
  table->capacity = 0;
}

static void aoc_ini_free_all () {
  for (int i = 0; i < num_string_tables; i++) {
    free_string_table_entries(&string_tables[i]);
    free(string_tables[i].filename);
  }
  free(string_tables);
  num_string_tables = 0;
  return;
}

void aoc_ini_free_strings (HANDLE table_handle) {
  int table_id = (size_t) table_handle;
  for (int i = 0; i < num_string_tables; i++) {
    if (string_tables[i].id == table_id) {
      free_string_table_entries(&string_tables[i]);

      size_t num_remaining = num_string_tables - (i + 1);
      if (num_remaining == 0) {
        num_string_tables -= 1;
        string_tables = realloc(string_tables, num_string_tables * sizeof(string_table_t));
      } else {
        string_table_t* rest = calloc(num_remaining, sizeof(string_table_t));
        memcpy(rest, &string_tables[i + 1], num_remaining * sizeof(string_table_t));
        num_string_tables -= 1;
        string_tables = realloc(string_tables, num_string_tables * sizeof(string_table_t));
        memcpy(&string_tables[i], rest, num_remaining * sizeof(string_table_t));
        free(rest);
      }

      break;
    }
  }
}

static string_entry_t* find_string_in_table (string_table_t* table, int id) {
  for (int i = 0; i < table->size; i++) {
    if (table->entries[i].id == id) {
      return &table->entries[i];
    }
  }
  return NULL;
}

static string_entry_t* find_string (int id) {
  string_entry_t* entry = NULL;
  for (int i = 0; i < num_string_tables; i++) {
    entry = find_string_in_table(&string_tables[i], id);
    if (entry) return entry;
  }
  return NULL;
}

typedef void* __stdcall (*fn_load_string)(HINSTANCE, unsigned int, char*, int);
static const fn_load_string aoc_load_string = (fn_load_string) 0x58E820;
static char* __stdcall load_string_hook (HINSTANCE dll, unsigned int string_id, char* buf_out, int buf_size) {
  string_entry_t* entry = find_string(string_id);
  if (entry != NULL) {
    strncpy(buf_out, entry->value, buf_size);
    return buf_out;
  }

  return aoc_load_string(dll, string_id, buf_out, buf_size);
}

static hook_t hooks[10];
void aoc_ini_init() {
  dbg_print("init()\n");

  int i = 0;
  hooks[i++] = install_callhook((void*) 0x43CF2B, load_string_hook);
  hooks[i] = NULL;
}

void aoc_ini_deinit() {
  dbg_print("deinit()\n");

  aoc_ini_free_all();

  for (int i = 0; hooks[i] != NULL; i++) {
    revert_hook(hooks[i]);
  }
}
