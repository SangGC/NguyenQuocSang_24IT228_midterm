#ifndef DISPLAY_H
#define DISPLAY_H

#include "ls.h"

/*
 * Displays an EntryList according to formatting Options.
 *
 * Parameters:
 *   list: Pointer to EntryList containing file entries
 *   opts: Pointer to Options controlling formatting (-l, -n, -i, -s, -F, -h, -k, -q, -w)
 *   is_directory: True if the list represents directory contents
 *
 * TODO: Fully implement formatting and terminal output logic in Phase 5.
 */
void display_entries(const EntryList *list, const Options *opts, bool is_directory);

#endif /* DISPLAY_H */
