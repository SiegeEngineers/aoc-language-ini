#include "aoc_language_ini.h"
#include <mmmod.h>
#include <stdio.h>
#include <windows.h>

#define AOC_LANGUAGE_INI_VERSION "0.2.1"

static char file_exists(char* filename) {
  WIN32_FIND_DATA file_data;
  HANDLE exists = FindFirstFile(filename, &file_data);
  if (exists == INVALID_HANDLE_VALUE) {
    return 0;
  }
  FindClose(exists);
  return 1;
}

__declspec(dllexport) void mmm_setup(mmm_mod_info* info) {
  info->name = "Language INI loader";
  info->version = AOC_LANGUAGE_INI_VERSION;
}

__declspec(dllexport) void mmm_before_setup(mmm_mod_info* info) {
  aoc_ini_init();

  const char* mod_base_dir = info->meta->mod_base_dir;
  if (mod_base_dir == NULL || mod_base_dir[0] == '\0') {
    mod_base_dir = info->meta->game_base_dir;
  }
  char ini_path[MAX_PATH];
  // mod_base_dir contains trailing '\'
  sprintf(ini_path, "%s%s", mod_base_dir, "language.ini");
  printf("ini_path = %s\n", ini_path);
  if (file_exists(ini_path)) {
    aoc_ini_load_strings(ini_path);
  }
}

__declspec(dllexport) void mmm_unload(mmm_mod_info* info) { aoc_ini_deinit(); }

__declspec(dllexport) BOOL WINAPI
    DllMain(HINSTANCE dll, DWORD reason, void* _) {
  switch (reason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(dll);
    break;
  }
  return 1;
}
