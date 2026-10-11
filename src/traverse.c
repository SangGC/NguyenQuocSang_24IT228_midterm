#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "traverse.h"
#include "sort.h"
#include "display.h"
#include "utils.h"

static bool should_include_entry(const char *name, const Options *opts)
{
    if (opts->show_all) {
        /* -a: include all entries including . and .. */
        return true;
    }
    if (opts->show_almost_all) {
        /* -A: include hidden entries except . and .. */
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            return false;
        }
        return true;
    }
    /* Default: exclude entries starting with '.' */
    if (name[0] == '.') {
        return false;
    }
    return true;
}

/*
 * Reads, sorts, displays, and recursively (-R) traverses a directory.
 */
static int list_directory(const char *dir_path, const Options *opts, bool print_header, bool *need_leading_newline)
{
    if (print_header) {
        if (*need_leading_newline) {
            printf("\n");
        }
        printf("%s:\n", dir_path);
        *need_leading_newline = true;
    }

    DIR *dir = opendir(dir_path);
    if (dir == NULL) {
        fprintf(stderr, "ls: %s: %s\n", dir_path, strerror(errno));
        return 1;
    }

    EntryList dir_entries;
    init_entry_list(&dir_entries);

    struct dirent *d;
    while ((d = readdir(dir)) != NULL) {
        if (!should_include_entry(d->d_name, opts)) {
            continue;
        }

        char *full_path = build_path(dir_path, d->d_name);
        struct stat st_entry;
        bool stat_ok = false;
        if (full_path != NULL) {
            if (lstat(full_path, &st_entry) == 0) {
                stat_ok = true;
            }
        }

        const char *link_target = NULL;
#ifndef _WIN32
        char target_buf[1024];
        if (stat_ok && S_ISLNK(st_entry.st_mode)) {
            ssize_t len = readlink(full_path, target_buf, sizeof(target_buf) - 1);
            if (len >= 0) {
                target_buf[len] = '\0';
                link_target = target_buf;
            }
        }
#endif

        add_file_entry(&dir_entries, d->d_name,
                       (full_path != NULL) ? full_path : d->d_name,
                       stat_ok ? &st_entry : NULL, link_target);

        free(full_path);
    }

    closedir(dir);

    sort_entries(&dir_entries, opts);
    display_entries(&dir_entries, opts, true);
    *need_leading_newline = true;

    int status = 0;

    /* Handle recursive listing (-R) */
    if (opts->dir_handling == DIR_RECURSIVE) {
        char **subdirs = NULL;
        size_t subdir_count = 0;

        for (size_t i = 0; i < dir_entries.count; i++) {
            const FileEntry *entry = &dir_entries.entries[i];
            if (strcmp(entry->name, ".") == 0 || strcmp(entry->name, "..") == 0) {
                continue;
            }
            /* Recurse only into actual subdirectories (not symlinks to directories) */
            if (entry->stat_valid && S_ISDIR(entry->st.st_mode)) {
                char **tmp = realloc(subdirs, (subdir_count + 1) * sizeof(char *));
                if (tmp != NULL) {
                    subdirs = tmp;
                    subdirs[subdir_count] = strdup(entry->path);
                    if (subdirs[subdir_count] != NULL) {
                        subdir_count++;
                    }
                }
            }
        }

        free_entry_list(&dir_entries);

        for (size_t i = 0; i < subdir_count; i++) {
            int res = list_directory(subdirs[i], opts, true, need_leading_newline);
            if (res != 0) {
                status = 1;
            }
            free(subdirs[i]);
        }
        free(subdirs);
    } else {
        free_entry_list(&dir_entries);
    }

    return status;
}

int process_operands(int argc, char *argv[], int operand_start_idx, const Options *opts)
{
    int exit_status = 0;
    int operand_count = argc - operand_start_idx;

    /* Default operand is "." if no arguments supplied */
    char *default_operands[] = { "." };
    char **operands = (operand_count == 0) ? default_operands : &argv[operand_start_idx];
    if (operand_count == 0) {
        operand_count = 1;
    }

    EntryList files_list;
    EntryList dirs_list;
    init_entry_list(&files_list);
    init_entry_list(&dirs_list);

    /* Phase A: Classify operands into files and directories */
    for (int i = 0; i < operand_count; i++) {
        const char *path = operands[i];
        struct stat st_lstat;

        if (lstat(path, &st_lstat) != 0) {
            fprintf(stderr, "ls: %s: %s\n", path, strerror(errno));
            exit_status = 1;
            continue;
        }

        bool is_dir = false;
        if (opts->dir_handling == DIR_PLAIN) {
            /* -d: list directories as plain files */
            is_dir = false;
        } else {
            if (S_ISDIR(st_lstat.st_mode)) {
                is_dir = true;
            } else if (S_ISLNK(st_lstat.st_mode)) {
                /* For symlinks on CLI, check if target is a directory when -d is not specified */
                struct stat st_stat;
                if (stat(path, &st_stat) == 0 && S_ISDIR(st_stat.st_mode)) {
                    is_dir = true;
                }
            }
        }

        const char *link_target = NULL;
#ifndef _WIN32
        char target_buf[1024];
        if (S_ISLNK(st_lstat.st_mode)) {
            ssize_t len = readlink(path, target_buf, sizeof(target_buf) - 1);
            if (len >= 0) {
                target_buf[len] = '\0';
                link_target = target_buf;
            }
        }
#endif

        if (is_dir) {
            if (add_file_entry(&dirs_list, path, path, &st_lstat, link_target) != 0) {
                fprintf(stderr, "ls: memory allocation failed\n");
                exit_status = 1;
            }
        } else {
            if (add_file_entry(&files_list, path, path, &st_lstat, link_target) != 0) {
                fprintf(stderr, "ls: memory allocation failed\n");
                exit_status = 1;
            }
        }
    }

    size_t total_valid_operands = files_list.count + dirs_list.count;
    bool print_headers = (operand_count > 1 || total_valid_operands > 1);

    bool need_leading_newline = false;

    /* Phase B: Display file operands first (sorted) */
    if (files_list.count > 0) {
        sort_entries(&files_list, opts);
        display_entries(&files_list, opts, false);
        need_leading_newline = true;
    }

    /* Phase C: Process and display directory contents (sorted directory order) */
    if (dirs_list.count > 0) {
        sort_entries(&dirs_list, opts);
    }

    for (size_t i = 0; i < dirs_list.count; i++) {
        const char *dir_path = dirs_list.entries[i].path;
        bool print_this_header = print_headers;

        int res = list_directory(dir_path, opts, print_this_header, &need_leading_newline);
        if (res != 0) {
            exit_status = 1;
        }
    }

    free_entry_list(&files_list);
    free_entry_list(&dirs_list);

    return exit_status;
}
