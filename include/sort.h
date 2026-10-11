#ifndef SORT_H
#define SORT_H

#include "ls.h"

/*
 * Sorts an EntryList based on the specified Options.
 *
 * Parameters:
 *   list: Pointer to EntryList containing file entries
 *   opts: Pointer to Options controlling sort order (-t, -S, -r, -f, -c, -u)
 *
 * TODO: Fully implement sorting comparator functions in Phase 4.
 */
void sort_entries(EntryList *list, const Options *opts);

#endif /* SORT_H */
