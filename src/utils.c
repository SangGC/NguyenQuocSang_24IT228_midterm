#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void utils_init(void)
{
    /* Reserved for caching user/group and environment initialization */
}

void init_entry_list(EntryList *list)
{
    if (list == NULL) return;
    list->entries = NULL;
    list->count = 0;
    list->capacity = 0;
}

int add_file_entry(EntryList *list, const char *name, const char *path, const struct stat *st, const char *link_target)
{
    if (list == NULL || name == NULL || path == NULL) {
        return -1;
    }

    if (list->count >= list->capacity) {
        size_t new_cap = (list->capacity == 0) ? 16 : list->capacity * 2;
        FileEntry *new_entries = realloc(list->entries, new_cap * sizeof(FileEntry));
        if (new_entries == NULL) {
            return -1;
        }
        list->entries = new_entries;
        list->capacity = new_cap;
    }

    FileEntry *entry = &list->entries[list->count];
    entry->name = strdup(name);
    entry->path = strdup(path);
    if (entry->name == NULL || entry->path == NULL) {
        free(entry->name);
        free(entry->path);
        return -1;
    }

    if (st != NULL) {
        entry->st = *st;
        entry->stat_valid = true;
    } else {
        memset(&entry->st, 0, sizeof(struct stat));
        entry->stat_valid = false;
    }

    if (link_target != NULL) {
        entry->link_target = strdup(link_target);
        if (entry->link_target == NULL) {
            free(entry->name);
            free(entry->path);
            entry->name = NULL;
            entry->path = NULL;
            return -1;
        }
    } else {
        entry->link_target = NULL;
    }

    list->count++;
    return 0;
}

void free_entry_list(EntryList *list)
{
    if (list == NULL || list->entries == NULL) {
        return;
    }

    for (size_t i = 0; i < list->count; i++) {
        free(list->entries[i].name);
        free(list->entries[i].path);
        free(list->entries[i].link_target);
    }
    free(list->entries);
    list->entries = NULL;
    list->count = 0;
    list->capacity = 0;
}

char *build_path(const char *dir, const char *file)
{
    if (dir == NULL || file == NULL) return NULL;
    size_t len_dir = strlen(dir);
    size_t len_file = strlen(file);
    bool needs_slash = (len_dir > 0 && dir[len_dir - 1] != '/' && dir[len_dir - 1] != '\\');

    size_t total_len = len_dir + (needs_slash ? 1 : 0) + len_file + 1;
    char *path = malloc(total_len);
    if (path == NULL) return NULL;

    if (needs_slash) {
        snprintf(path, total_len, "%s/%s", dir, file);
    } else {
        snprintf(path, total_len, "%s%s", dir, file);
    }
    return path;
}
