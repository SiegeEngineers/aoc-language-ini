#include "aoc_language_ini.h"
#include "hook.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DEBUG
#define dbg_print(...) printf("[aoc-language-ini] " __VA_ARGS__)
#else
#define dbg_print(...)
#endif

typedef struct string_entry {
  int id;
  int size;
  char* value;
} string_entry_t;

typedef struct string_table {
  size_t id;
  char* filename;
  size_t size;
  size_t capacity;
  string_entry_t* entries;
} string_table_t;

static string_table_t* string_tables;
static size_t num_string_tables;
static size_t next_string_table_id;

static char* read_file(char* filename) {
  FILE* file = fopen(filename, "rb");
  if (file == NULL)
    return NULL;
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

static void expand_string_table(string_table_t* string_table) {
  string_table->capacity += 8192;
  string_table->entries = realloc(
      string_table->entries, string_table->capacity * sizeof(string_entry_t));
}

static char* unescape(char* input) {
  int l = strlen(input);
  int j = 0;
  char* buf = calloc(1, l + 1);
  for (int i = 0; i < l; i++) {
    if (input[i] == '\\' && i + 1 < l) {
      switch (input[i + 1]) {
      case 'n':
        i++;
        buf[j++] = '\n';
        break;
      case 'r':
        i++;
        buf[j++] = '\r';
        break;
      default:
        buf[j++] = input[i];
        break;
      }
    } else {
      buf[j++] = input[i];
    }
  }
  buf[j] = '\0';
  return buf;
}

string_table_id aoc_ini_load_strings(char* filename) {
  dbg_print("load strings: %s\n", filename);

  int id;
  char cur_string[4096];

  char* content = read_file(filename);
  if (!content)
    return (string_table_id){SIZE_MAX};

  string_table_t string_table = {.id = next_string_table_id,
                                 .filename = strdup(filename),
                                 .size = 0,
                                 .capacity = 8192,
                                 .entries =
                                     calloc(8192, sizeof(string_entry_t))};

  next_string_table_id++;

  char* read_ptr = strtok(content, "\n");
  while (read_ptr != NULL) {
    if (read_ptr[0] != ';') {
      if (sscanf(read_ptr, "%d=%4096[^\n\r]", &id, cur_string) == 2) {
        dbg_print("found string %d: '%s'\n", id, cur_string);
        string_table.entries[string_table.size].id = id;
        char* unescaped = unescape(cur_string);
        string_table.entries[string_table.size].value = unescaped;
        string_table.entries[string_table.size].size = strlen(unescaped);

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
           (string_table.capacity - string_table.size) *
               sizeof(string_entry_t));
  }

  if (string_tables == NULL) {
    string_tables = calloc(1, sizeof(string_table_t));
    num_string_tables = 1;
  } else {
    num_string_tables += 1;
    string_tables =
        realloc(string_tables, num_string_tables * sizeof(string_table_t));
  }

  memcpy(&string_tables[num_string_tables - 1], &string_table,
         sizeof(string_table));

  return (string_table_id){string_table.id};
}

static void free_string_table_entries(string_table_t* table) {
  dbg_print("free entries: %s\n", table->filename);
  for (size_t i = 0; i < table->size; i++) {
    free(table->entries[i].value);
  }
  free(table->entries);
  table->entries = NULL;
  table->size = 0;
  table->capacity = 0;
}

static void aoc_ini_free_all(void) {
  for (size_t i = 0; i < num_string_tables; i++) {
    free_string_table_entries(&string_tables[i]);
    free(string_tables[i].filename);
  }
  free(string_tables);
  num_string_tables = 0;
  return;
}

void aoc_ini_free_strings(string_table_id table_handle) {
  for (size_t i = 0; i < num_string_tables; i++) {
    if (string_tables[i].id == table_handle.id) {
      free_string_table_entries(&string_tables[i]);

      size_t num_remaining = num_string_tables - (i + 1);
      if (num_remaining == 0) {
        num_string_tables -= 1;
        string_tables =
            realloc(string_tables, num_string_tables * sizeof(string_table_t));
      } else {
        string_table_t* rest = calloc(num_remaining, sizeof(string_table_t));
        memcpy(rest, &string_tables[i + 1],
               num_remaining * sizeof(string_table_t));
        num_string_tables -= 1;
        string_tables =
            realloc(string_tables, num_string_tables * sizeof(string_table_t));
        memcpy(&string_tables[i], rest, num_remaining * sizeof(string_table_t));
        free(rest);
      }

      break;
    }
  }
}

static string_entry_t* find_string_in_table(string_table_t* table, int id) {
  for (size_t i = 0; i < table->size; i++) {
    if (table->entries[i].id == id) {
      return &table->entries[i];
    }
  }
  return NULL;
}

static string_entry_t* find_string(int id) {
  string_entry_t* entry = NULL;
  for (int i = num_string_tables - 1; i >= 0; i--) {
    entry = find_string_in_table(&string_tables[i], id);
    if (entry)
      return entry;
  }
  return NULL;
}

typedef void* HINSTANCE;
typedef void*(__stdcall* fn_load_string)(HINSTANCE, unsigned int, char*, int);
static const fn_load_string aoc_load_string = (fn_load_string)0x58E820;
static char* __stdcall load_string_hook(HINSTANCE dll, unsigned int string_id,
                                        char* buf_out, int buf_size) {
  string_entry_t* entry = find_string(string_id);
  if (entry != NULL) {
    strncpy(buf_out, entry->value, buf_size);
    return buf_out;
  }

  return aoc_load_string(dll, string_id, buf_out, buf_size);
}

static hook_t hooks[10];
void aoc_ini_init(void) {
  dbg_print("init()\n");

  int i = 0;
  hooks[i++] = install_callhook((void*)0x43CF2B, load_string_hook);
  hooks[i] = NULL;
}

void aoc_ini_deinit(void) {
  dbg_print("deinit()\n");

  aoc_ini_free_all();

  for (int i = 0; hooks[i] != NULL; i++) {
    revert_hook(hooks[i]);
  }
}
