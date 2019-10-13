source   = [ "src" ]
include  = [ "src" ]
lflags   = [ "-lSDL2", "-lSDL2main", "-lGL", "-lm" ]
cflags   = [ "-g", "-std=gnu11", "-Wall", "-Werror" ]
output   = "aq"

if "windows" in opt:
    compiler = "x86_64-w64-mingw32-gcc"
    output = "aq.exe"
    cflags += [ "-Iwinlib/SDL2-2.0.5/x86_64-w64-mingw32/include" ]
    lflags += [ "-Lwinlib/SDL2-2.0.5/x86_64-w64-mingw32/lib" ]
    lflags  = [ "-lmingw32", "-lSDL2main", "-lopengl32" ] + lflags
    lflags += [ "-lwinmm" ]
    # lflags += [ "-mwindows" ]
    lflags.remove("-lGL")

if "sanitize" in opt:
    lflags += [ "-fsanitize=address" ]
    cflags += [ "-fsanitize=address" ]

if "release" in opt:
    cflags += [ "-O3", "-ffast-math" ]
    lflags += [ "-s" ]
