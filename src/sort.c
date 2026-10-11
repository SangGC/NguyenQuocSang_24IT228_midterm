#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "sort.h"

static const Options *current_opts = NULL;

/*
 * Retrieves the appropriate timestamp as a struct timespec
 * based on the selected time type (-c for ctime, -u for atime, default mtime).
 */
static struct timespec get_entry_time(const FileEntry *entry, TimeType time_type)
{
    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0;

    if (entry == NULL || !entry->stat_valid) {
        return ts;
    }

#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__)
    switch (time_type) {
    case TIME_CTIME:
        return entry->st.st_ctimespec;
    case TIME_ATIME:
        return entry->st.st_atimespec;
    case TIME_MTIME:
    default:
        return entry->st.st_mtimespec;
    }
#elif defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L) && !defined(_WIN32)
    switch (time_type) {
    case TIME_CTIME:
        return entry->st.st_ctim;
    case TIME_ATIME:
        return entry->st.st_atim;
    case TIME_MTIME:
    default:
        return entry->st.st_mtim;
    }
#else
    switch (time_type) {
    case TIME_CTIME:
        ts.tv_sec = entry->st.st_ctime;
        break;
    case TIME_ATIME:
        ts.tv_sec = entry->st.st_atime;
        break;
    case TIME_MTIME:
    default:
        ts.tv_sec = entry->st.st_mtime;
        break;
    }
    ts.tv_nsec = 0;
    return ts;
#endif
}

/*
 * Comparator for qsort conforming to POSIX / NetBSD ls behavior.
 */
static int compare_entries(const void *a_ptr, const void *b_ptr)
{
    const FileEntry *a = (const FileEntry *)a_ptr;
    const FileEntry *b = (const FileEntry *)b_ptr;

    if (a == NULL && b == NULL) return 0;
    if (a == NULL) return 1;
    if (b == NULL) return -1;

    int diff = 0;

    if (current_opts != NULL && current_opts->sort_by_size) {
        /* -S: Sort by size, largest file first */
        off_t size_a = (a->stat_valid) ? a->st.st_size : 0;
        off_t size_b = (b->stat_valid) ? b->st.st_size : 0;
        if (size_a > size_b) {
            diff = -1;
        } else if (size_a < size_b) {
            diff = 1;
        } else {
            diff = 0;
        }
    } else if (current_opts != NULL && current_opts->sort_by_time) {
        /* -t: Sort by time, most recent first */
        struct timespec ta = get_entry_time(a, current_opts->time_type);
        struct timespec tb = get_entry_time(b, current_opts->time_type);

        if (ta.tv_sec > tb.tv_sec) {
            diff = -1;
        } else if (ta.tv_sec < tb.tv_sec) {
            diff = 1;
        } else {
            if (ta.tv_nsec > tb.tv_nsec) {
                diff = -1;
            } else if (ta.tv_nsec < tb.tv_nsec) {
                diff = 1;
            } else {
                diff = 0;
            }
        }
    }

    /* Primary default or tie-breaker: lexicographical order by file name */
    if (diff == 0) {
        const char *name_a = (a->name != NULL) ? a->name : "";
        const char *name_b = (b->name != NULL) ? b->name : "";
        diff = strcmp(name_a, name_b);
    }

    /* -r: Reverse sort order (reverses primary criterion and tie-breaker) */
    if (current_opts != NULL && current_opts->reverse_sort) {
        diff = -diff;
    }

    return diff;
}

void sort_entries(EntryList *list, const Options *opts)
{
    if (list == NULL || list->entries == NULL || list->count <= 1) {
        return;
    }

    /* -f: Output is not sorted */
    if (opts != NULL && opts->unsorted) {
        return;
    }

    current_opts = opts;
    qsort(list->entries, list->count, sizeof(FileEntry), compare_entries);
    current_opts = NULL;
}
