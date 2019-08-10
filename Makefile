CC = i686-w64-mingw32-gcc
WINDRES = i686-w64-mingw32-windres

OPTFLAGS = -O3 -s
DBGFLAGS = -DDEBUG -g
SOURCES = aoc_language_ini.c hook.c

FLAGS = -I./include
ifeq ($(RELEASE),1)
  FLAGS += $(OPTFLAGS)
  X1FLAGS = -flto -Wl,--exclude-all-symbols
else
  FLAGS += $(DBGFLAGS)
  X1FLAGS =
endif

all: getstrings.exe aoc-language-ini.dll

format:
	clang-format -style=file -i *.h *.c

clean:
	rm -f aoc-language-ini.dll getstrings.exe

.PHONY: all clean

strings.rc:
	@echo "create strings.rc first by doing:"
	@echo "  getstrings.exe -rc /path/to/language_x1_p1.dll > strings.rc"
	@exit 1

getstrings.exe: getstrings.c
	$(CC) -o $@ -Wall -m32 $(FLAGS) $^

aoc-language-ini.dll: $(SOURCES) main.c strings.rc
	$(WINDRES) strings.rc -O coff -o strings.res
	$(CC) -o $@ -Wall -m32 $(FLAGS) $(X1FLAGS) -shared $(SOURCES) main.c strings.res
