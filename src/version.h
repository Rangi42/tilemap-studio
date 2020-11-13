#ifndef VERSION_H
#define VERSION_H

#define PROGRAM_VERSION 3,2,2
#ifdef _DEBUG
#define PROGRAM_VERSION_STRING "3.2.2 [DEBUG]"
#else
#define PROGRAM_VERSION_STRING "3.2.2"
#endif

#define PROGRAM_EXE_NAME "tilemapstudio"

#ifdef _WIN32
#define PROGRAM_EXE PROGRAM_EXE_NAME ".exe"
#else
#define PROGRAM_EXE PROGRAM_EXE_NAME
#endif

#define PROGRAM_NAME "Tilemap Studio"

#define PROGRAM_AUTHOR "Rangi"

#define CURRENT_YEAR "2020"

#endif
