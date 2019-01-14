CC = i686-w64-mingw32-gcc
WINDRES = i686-w64-mingw32-windres

OPTFLAGS = -O3 -s
DBGFLAGS = -DDEBUG -g
SOURCES = api.c hook.c

ifeq ($(RELEASE),1)
  FLAGS = $(OPTFLAGS)
else
  FLAGS = $(DBGFLAGS)
endif

all: getstrings.exe aoc-language-ini.dll language_x1_p1.dll

clean:
	rm -f aoc-language-ini.dll language_x1_p1.dll getstrings.exe

.PHONY: all clean

strings.rc:
	@echo "create strings.rc first by doing:"
	@echo "  getstrings.exe -rc /path/to/language_x1_p1.dll > strings.rc"
	@exit 1

getstrings.exe: getstrings.c
	$(CC) -o $@ -Wall -m32 $(FLAGS) $^

aoc-language-ini.dll: $(SOURCES)
	$(CC) -o $@ -Wall -m32 $(FLAGS) -shared $^
language_x1_p1.dll: $(SOURCES) main.c strings.rc
	$(WINDRES) strings.rc -O coff -o strings.res
	$(CC) -o $@ -Wall -m32 $(FLAGS) -shared $(SOURCES) main.c strings.res
