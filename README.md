# aoc-language-ini

Use human-readable and -writable `language.ini` files for your mod's custom strings.

This "mod module" can be used as part of a larger mod. It can be used in two ways:

 - Standalone, where it replaces your mod's `language_x1_p1.dll` file, and loads
   your language.ini instead. This method does not require any code.
 - Programmatically, for when you already have custom exe mods or are already
   loading a custom DLL. This method allows loading any language file from any
   location, but you need to compile some code to do it.

## Standalone Usage

Put `language_x1_p1.dll` from the [releases](https://github.com/SiegeEngineers/aoc-language-ini)
page in the `Games\$YOUR_MOD_NAME\Data\` folder.
If you already have one, see [below](#migrate) for migration steps—do that _before_ replacing your `language_x1_p1.dll` file.

Create a `language.ini` in the `Games\$YOUR_MOD_NAME` folder.

The language.ini should be contain one string per line, in this format:

```ini
; id=value
9505=History
```

Optionally, use `;` at the start of a line to turn it into a comment.
```ini
; random map names
10914=Acropolis
10915=Budapest
10916=Cenotes
10917=City of Lakes
10918=Golden Pit
10919=Hideout
10920=Hill Fort
10921=Lombardia
```

For strings containing multiple lines, use \n in place of `<enter>`:

```
12345=First Line.\nSecond Line.
```

<a name="migrate"></a>
## Convert Existing language.dll to language.ini

Using the `language_x1_p1.dll` proxy from this project means your `Data\language_x1_p1.dll` file will not contain your custom strings anymore. You can migrate them to a `language.ini` using the `getstrings` tool.

Please make sure to backup your `language_x1_p1.dll` in case something goes wrong.

Download `getstrings.exe` from the [releases](https://github.com/SiegeEngineers/aoc-language-ini)
page. Put it in your mod directory, then navigate to that directory in the command prompt and do:

```
.\getstrings.exe Data\language_x1_p1.dll > language.ini
```

You can then delete `getstrings.exe` and replace your old `language_x1_p1.dll` with the one from this project.

## Programmatic usage

If you are already doing exe mods or have some other way of injecting a DLL for your mod, you can avoid the `language_x1_p1.dll` proxy thing.

Instead, download `aoc-language-ini.dll` from the [releases](https://github.com/SiegeEngineers/aoc-language-ini)
page.
You can load it using `LoadLibrary()` or link to it during your build process (if you are already writing C/C++ code).
Then, call `aoc_ini_load_strings` to load a .ini file.
You can load a single file, or split your strings into multiple categories and load multiple files, if you prefer.
If your mod is also uploaded to Voobly, I recommend using a single `language.ini` anyway, because then you can reuse the same file the Voobly and offline versions of your mod.

```c
HINSTANCE aoc_ini; // will hold our DLL file
HANDLE aoc_string_inis[50]; // reserve some space for string table handles
void (*aoc_ini_init)();
HANDLE (*aoc_ini_load_strings)(char*);
void (*aoc_ini_free_strings)(HANDLE);
void (*aoc_ini_deinit)();

void load_aoc_ini () {
  // AoC's working directory is the main game dir, so you can load stuff
  // from Games\ModName\ like this
  aoc_ini = LoadLibrary("Games\\MyModName\\aoc-language-ini.dll");
  aoc_ini_init = GetProcAddress(aoc_ini, "aoc_ini_init");
  aoc_ini_load_strings = GetProcAddress(aoc_ini, "aoc_ini_load_strings");
  aoc_ini_free_strings = GetProcAddress(aoc_ini, "aoc_ini_free_strings");
  aoc_ini_deinit = GetProcAddress(aoc_ini, "aoc_ini_deinit");

  // Install the hooks that allow using custom strings.
  aoc_ini_init();

  // Storing string table references in an array makes them easy to deallocate later
  int i = 0;
  aoc_string_inis[i++] = aoc_ini_load_strings("Games\\MyModName\\map_names.ini");
  aoc_string_inis[i++] = aoc_ini_load_strings("Games\\MyModName\\campaign_strings.ini");
  aoc_string_inis[i] = NULL;
}

void unload_aoc_ini () {
  // Free all the string tables
  for (int i = 0; aoc_string_inis[i] != NULL; i++) {
    aoc_ini_free_strings(aoc_string_inis[i]);
  }

  // Remove the hooks.
  aoc_ini_deinit();

  FreeLibrary(aoc_ini);
}
```

You can also load different files depending on some conditions, like the language.
```c
if (language_is_english) {
  aoc_ini_load_strings("Games\\MyModName\\en\\strings.ini");
} else if (language_is_russian) {
  aoc_ini_load_strings("Games\\MyModName\\ru\\strings.ini");
}
```

If multiple files contain the same string IDs, the file that was loaded _last_ takes precedence.

## License

[LGPL-3.0](./LICENSE.md)
