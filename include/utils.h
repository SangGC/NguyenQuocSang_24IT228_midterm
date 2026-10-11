#ifndef UTILS_H
#define UTILS_H

#include "ls.h"

/* Initializes utility subsystems */
void utils_init(void);

/* Helper functions for EntryList memory management */
void init_entry_list(EntryList *list);
int add_file_entry(EntryList *list, const char *name, const char *path, const struct stat *st, const char *link_target);
void free_entry_list(EntryList *list);

/* Path utility */
char *build_path(const char *dir, const char *file);

#endif /* UTILS_H */
