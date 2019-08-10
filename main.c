#include <windows.h>
#include <stdio.h>
#include "api.h"
#include <mmmod.h>

#define AOC_LANGUAGE_INI_VERSION "0.2.1"

static char file_exists (char* filename) {
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

  if (info->meta->mod_base_dir[0] != '\0') {
    char ini_path[MAX_PATH];
    // mod_base_dir contains trailing '\'
    sprintf(ini_path, "%s%s", info->meta->mod_base_dir, "language.ini");
    if (file_exists(ini_path)) {
      aoc_ini_load_strings(ini_path);
    }
  }
}

__declspec(dllexport) void mmm_unload(mmm_mod_info* info) {
  aoc_ini_deinit();
}

BOOL WINAPI DllMain (HINSTANCE dll, DWORD reason, void* _) {
  switch (reason) {
    case DLL_PROCESS_ATTACH:
      DisableThreadLibraryCalls(dll);
      break;
  }
  return 1;
}
