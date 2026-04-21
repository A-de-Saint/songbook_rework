#ifndef RECOMPILER_H
#define RECOMPILER_H

#include "quick_reader.h"
#include "pathwork.h"
#include "songbook_manager.h"
#include "song_manager.h"
#include "util.h"

int recompile_songbook(Path *home_path, Songbook *songbook, StringArray *unsuccessful);

#endif