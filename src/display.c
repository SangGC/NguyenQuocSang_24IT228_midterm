#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef _WIN32
#include <pwd.h>
#include <grp.h>
#endif

#include "display.h"

#if defined(_WIN32)
typedef unsigned int uid_t;
typedef unsigned int gid_t;
#endif

/* Portability definitions for stdout fd and permission bits */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

#ifndef S_ISUID
#define S_ISUID 0004000
#endif
#ifndef S_ISGID
#define S_ISGID 0002000
#endif
#ifndef S_ISVTX
#define S_ISVTX 0001000
#endif

#ifndef S_IRUSR
#define S_IRUSR 0000400
#endif
#ifndef S_IWUSR
#define S_IWUSR 0000200
#endif
#ifndef S_IXUSR
#define S_IXUSR 0000100
#endif
#ifndef S_IRGRP
#define S_IRGRP 0000040
#endif
#ifndef S_IWGRP
#define S_IWGRP 0000020
#endif
#ifndef S_IXGRP
#define S_IXGRP 0000010
#endif
#ifndef S_IROTH
#define S_IROTH 0000004
#endif
#ifndef S_IWOTH
#define S_IWOTH 0000002
#endif
#ifndef S_IXOTH
#define S_IXOTH 0000001
#endif

#ifndef S_ISCHR
#define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#endif
#ifndef S_ISSOCK
#ifdef S_IFSOCK
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
#else
#define S_ISSOCK(m) (0)
#endif
#endif
#ifndef S_ISWHT
#ifdef S_IFWHT
#define S_ISWHT(m) (((m) & S_IFMT) == S_IFWHT)
#else
#define S_ISWHT(m) (0)
#endif
#endif

#ifndef major
#define major(dev) (((dev) >> 8) & 0xff)
#endif
#ifndef minor
#define minor(dev) ((dev) & 0xff)
#endif

/* Pre-formatted row for column-aligned output */
typedef struct {
    char inode_str[32];
    char blocks_str[32];
    char mode_str[12];
    char nlink_str[16];
    char owner_str[64];
    char group_str[64];
    char size_str[32];
    char time_str[32];
    char classify_char;
    const char *name;
    const char *link_target;
} DisplayRow;

/* Formats 10-character file mode string according to NetBSD ls specification */
static void format_mode(mode_t mode, char *out)
{
    if (S_ISDIR(mode)) {
        out[0] = 'd';
    } else if (S_ISLNK(mode)) {
        out[0] = 'l';
    } else if (S_ISCHR(mode)) {
        out[0] = 'c';
    } else if (S_ISBLK(mode)) {
        out[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        out[0] = 'p';
    } else if (S_ISSOCK(mode)) {
        out[0] = 's';
    } else if (S_ISWHT(mode)) {
        out[0] = 'w';
    } else {
        out[0] = '-';
    }

    out[1] = (mode & S_IRUSR) ? 'r' : '-';
    out[2] = (mode & S_IWUSR) ? 'w' : '-';
    if (mode & S_ISUID) {
        out[3] = (mode & S_IXUSR) ? 's' : 'S';
    } else {
        out[3] = (mode & S_IXUSR) ? 'x' : '-';
    }

    out[4] = (mode & S_IRGRP) ? 'r' : '-';
    out[5] = (mode & S_IWGRP) ? 'w' : '-';
    if (mode & S_ISGID) {
        out[6] = (mode & S_IXGRP) ? 's' : 'S';
    } else {
        out[6] = (mode & S_IXGRP) ? 'x' : '-';
    }

    out[7] = (mode & S_IROTH) ? 'r' : '-';
    out[8] = (mode & S_IWOTH) ? 'w' : '-';
    if (mode & S_ISVTX) {
        out[9] = (mode & S_IXOTH) ? 't' : 'T';
    } else {
        out[9] = (mode & S_IXOTH) ? 'x' : '-';
    }

    out[10] = '\0';
}

/* Formats human-readable sizes (-h option) */
static void format_human_size(unsigned long long bytes, char *buf, size_t bufsize)
{
    if (bytes < 1024ULL) {
        snprintf(buf, bufsize, "%lluB", bytes);
    } else if (bytes < 1024ULL * 1024ULL) {
        if (bytes < 10ULL * 1024ULL) {
            snprintf(buf, bufsize, "%.1fK", (double)bytes / 1024.0);
        } else {
            snprintf(buf, bufsize, "%lluK", (bytes + 512ULL) / 1024ULL);
        }
    } else if (bytes < 1024ULL * 1024ULL * 1024ULL) {
        if (bytes < 10ULL * 1024ULL * 1024ULL) {
            snprintf(buf, bufsize, "%.1fM", (double)bytes / (1024.0 * 1024.0));
        } else {
            snprintf(buf, bufsize, "%lluM", (bytes + 512ULL * 1024ULL) / (1024ULL * 1024ULL));
        }
    } else if (bytes < 1024ULL * 1024ULL * 1024ULL * 1024ULL) {
        if (bytes < 10ULL * 1024ULL * 1024ULL * 1024ULL) {
            snprintf(buf, bufsize, "%.1fG", (double)bytes / (1024.0 * 1024.0 * 1024.0));
        } else {
            snprintf(buf, bufsize, "%lluG", (bytes + 512ULL * 1024ULL * 1024ULL) / (1024ULL * 1024ULL * 1024ULL));
        }
    } else {
        snprintf(buf, bufsize, "%.1fT", (double)bytes / (1024.0 * 1024.0 * 1024.0 * 1024.0));
    }
}

/* Returns the number of 512-byte blocks used by entry */
static long long get_entry_512_blocks(const FileEntry *entry)
{
    if (entry == NULL || !entry->stat_valid) return 0;
#if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__APPLE__) || (defined(_POSIX_C_SOURCE) && !defined(_WIN32))
    return (long long)entry->st.st_blocks;
#else
    return (entry->st.st_size + 511) / 512;
#endif
}

/* Determines the blocksize setting based on options and environment */
static long get_blocksize_setting(const Options *opts)
{
    if (opts->size_format == SIZE_KB) {
        return 1024;
    }
    if (opts->size_format == SIZE_HUMAN) {
        return 512;
    }
    const char *env = getenv("BLOCKSIZE");
    if (env != NULL && *env != '\0') {
        long bs = strtol(env, NULL, 10);
        if (bs >= 512) {
            return bs;
        }
    }
    return 512;
}

/* Formats block count string (-s option) */
static void format_block_count(const FileEntry *entry, const Options *opts, long bsize, char *buf, size_t bufsize)
{
    long long num_512 = get_entry_512_blocks(entry);
    unsigned long long bytes = (unsigned long long)num_512 * 512ULL;

    if (opts->size_format == SIZE_HUMAN) {
        format_human_size(bytes, buf, bufsize);
    } else {
        if (bsize <= 0) bsize = 512;
        long long blocks = (bytes + bsize - 1) / bsize;
        snprintf(buf, bufsize, "%lld", blocks);
    }
}

/* Looks up owner username or numeric UID */
static void get_owner_string(uid_t uid, char *buf, size_t size, bool numeric)
{
    if (!numeric) {
#ifndef _WIN32
        struct passwd *pw = getpwuid(uid);
        if (pw != NULL && pw->pw_name != NULL) {
            snprintf(buf, size, "%s", pw->pw_name);
            return;
        }
#endif
    }
    snprintf(buf, size, "%u", (unsigned int)uid);
}

/* Looks up group name or numeric GID */
static void get_group_string(gid_t gid, char *buf, size_t size, bool numeric)
{
    if (!numeric) {
#ifndef _WIN32
        struct group *gr = getgrgid(gid);
        if (gr != NULL && gr->gr_name != NULL) {
            snprintf(buf, size, "%s", gr->gr_name);
            return;
        }
#endif
    }
    snprintf(buf, size, "%u", (unsigned int)gid);
}

/* Returns the timestamp to display based on -c, -u, or default mtime */
static time_t get_display_time(const FileEntry *entry, TimeType time_type)
{
    if (entry == NULL || !entry->stat_valid) return 0;
    switch (time_type) {
    case TIME_CTIME:
        return entry->st.st_ctime;
    case TIME_ATIME:
        return entry->st.st_atime;
    case TIME_MTIME:
    default:
        return entry->st.st_mtime;
    }
}

/* Formats modification/status date & time */
static void format_date_time(time_t ftime, char *buf, size_t size)
{
    time_t now = time(NULL);
    time_t six_months_ago = now - (180LL * 86400LL);
    time_t one_hour_future = now + 3600LL;

    struct tm *tm_ptr = NULL;
#if defined(_WIN32)
    tm_ptr = localtime(&ftime);
#else
    struct tm tm_buf;
    tm_ptr = localtime_r(&ftime, &tm_buf);
#endif

    if (tm_ptr == NULL) {
        snprintf(buf, size, "            ");
        return;
    }

    char mon[16];
    strftime(mon, sizeof(mon), "%b", tm_ptr);

    if (ftime < six_months_ago || ftime > one_hour_future) {
        snprintf(buf, size, "%s %2d  %4d", mon, tm_ptr->tm_mday, tm_ptr->tm_year + 1900);
    } else {
        snprintf(buf, size, "%s %2d %02d:%02d", mon, tm_ptr->tm_mday, tm_ptr->tm_hour, tm_ptr->tm_min);
    }
}

/* Returns classification symbol for -F option */
static char get_classify_char(mode_t mode)
{
    if (S_ISLNK(mode)) return '@';
    if (S_ISDIR(mode)) return '/';
    if (S_ISWHT(mode)) return '%';
    if (S_ISSOCK(mode)) return '=';
    if (S_ISFIFO(mode)) return '|';
    if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return '*';
    return '\0';
}

/* Prints name string, replacing non-printable characters if required (-q vs -w) */
static void print_escaped_name(const char *name, bool replace_nonprint)
{
    if (name == NULL) return;
    for (const char *p = name; *p != '\0'; p++) {
        unsigned char c = (unsigned char)*p;
        if (replace_nonprint && !isprint(c)) {
            putchar('?');
        } else {
            putchar(c);
        }
    }
}

void display_entries(const EntryList *list, const Options *opts, bool is_directory)
{
    if (list == NULL || opts == NULL) {
        return;
    }

    long bsize = get_blocksize_setting(opts);

    /* Determine character escaping mode (-q vs -w) */
    bool is_term = isatty(STDOUT_FILENO);
    bool replace_nonprint = false;
    if (opts->char_handling == CHAR_QUESTION) {
        replace_nonprint = true;
    } else if (opts->char_handling == CHAR_RAW) {
        replace_nonprint = false;
    } else {
        replace_nonprint = is_term;
    }

    /* Print total blocks line when appropriate */
    bool print_total = false;
    if (is_directory && (opts->output_format != FMT_SHORT || opts->print_blocks)) {
        print_total = true;
    } else if (!is_directory && opts->print_blocks && is_term) {
        print_total = true;
    }

    if (print_total) {
        long long sum_512 = 0;
        for (size_t i = 0; i < list->count; i++) {
            sum_512 += get_entry_512_blocks(&list->entries[i]);
        }
        unsigned long long sum_bytes = (unsigned long long)sum_512 * 512ULL;
        if (opts->size_format == SIZE_HUMAN) {
            char total_buf[32];
            format_human_size(sum_bytes, total_buf, sizeof(total_buf));
            printf("total %s\n", total_buf);
        } else {
            long long total_blocks = (sum_bytes + bsize - 1) / bsize;
            printf("total %lld\n", total_blocks);
        }
    }

    if (list->count == 0 || list->entries == NULL) {
        return;
    }

    DisplayRow *rows = calloc(list->count, sizeof(DisplayRow));
    if (rows == NULL) {
        fprintf(stderr, "ls: memory allocation failed in display\n");
        return;
    }

    int max_inode_len = 0;
    int max_blocks_len = 0;
    int max_nlink_len = 0;
    int max_owner_len = 0;
    int max_group_len = 0;
    int max_size_len = 0;

    bool is_long = (opts->output_format != FMT_SHORT);
    bool numeric_ids = (opts->output_format == FMT_LONG_NUMERIC);

    for (size_t i = 0; i < list->count; i++) {
        const FileEntry *entry = &list->entries[i];
        DisplayRow *row = &rows[i];
        row->name = entry->name;
        row->link_target = entry->link_target;

        /* Inode column */
        if (opts->print_inode) {
            if (entry->stat_valid) {
                snprintf(row->inode_str, sizeof(row->inode_str), "%llu", (unsigned long long)entry->st.st_ino);
            } else {
                snprintf(row->inode_str, sizeof(row->inode_str), "?");
            }
            int len = (int)strlen(row->inode_str);
            if (len > max_inode_len) max_inode_len = len;
        }

        /* Blocks column */
        if (opts->print_blocks) {
            if (entry->stat_valid) {
                format_block_count(entry, opts, bsize, row->blocks_str, sizeof(row->blocks_str));
            } else {
                snprintf(row->blocks_str, sizeof(row->blocks_str), "?");
            }
            int len = (int)strlen(row->blocks_str);
            if (len > max_blocks_len) max_blocks_len = len;
        }

        /* File classification indicator (-F) */
        if (opts->classify && entry->stat_valid) {
            row->classify_char = get_classify_char(entry->st.st_mode);
        } else {
            row->classify_char = '\0';
        }

        /* Long format fields (-l, -n) */
        if (is_long) {
            if (entry->stat_valid) {
                format_mode(entry->st.st_mode, row->mode_str);
                snprintf(row->nlink_str, sizeof(row->nlink_str), "%u", (unsigned int)entry->st.st_nlink);
                get_owner_string(entry->st.st_uid, row->owner_str, sizeof(row->owner_str), numeric_ids);
                get_group_string(entry->st.st_gid, row->group_str, sizeof(row->group_str), numeric_ids);

                /* Size / Major, Minor device numbers */
                if (S_ISCHR(entry->st.st_mode) || S_ISBLK(entry->st.st_mode)) {
                    snprintf(row->size_str, sizeof(row->size_str), "%3u, %3u",
                             (unsigned int)major(entry->st.st_rdev),
                             (unsigned int)minor(entry->st.st_rdev));
                } else if (opts->size_format == SIZE_HUMAN) {
                    format_human_size((unsigned long long)entry->st.st_size, row->size_str, sizeof(row->size_str));
                } else {
                    snprintf(row->size_str, sizeof(row->size_str), "%lld", (long long)entry->st.st_size);
                }

                time_t t = get_display_time(entry, opts->time_type);
                format_date_time(t, row->time_str, sizeof(row->time_str));
            } else {
                snprintf(row->mode_str, sizeof(row->mode_str), "??????????");
                snprintf(row->nlink_str, sizeof(row->nlink_str), "?");
                snprintf(row->owner_str, sizeof(row->owner_str), "?");
                snprintf(row->group_str, sizeof(row->group_str), "?");
                snprintf(row->size_str, sizeof(row->size_str), "?");
                snprintf(row->time_str, sizeof(row->time_str), "            ");
            }

            int nlink_len = (int)strlen(row->nlink_str);
            if (nlink_len > max_nlink_len) max_nlink_len = nlink_len;

            int owner_len = (int)strlen(row->owner_str);
            if (owner_len > max_owner_len) max_owner_len = owner_len;

            int group_len = (int)strlen(row->group_str);
            if (group_len > max_group_len) max_group_len = group_len;

            int size_len = (int)strlen(row->size_str);
            if (size_len > max_size_len) max_size_len = size_len;
        }
    }

    /* Print all formatted rows */
    for (size_t i = 0; i < list->count; i++) {
        const DisplayRow *row = &rows[i];

        if (opts->print_inode) {
            printf("%*s ", max_inode_len, row->inode_str);
        }

        if (opts->print_blocks) {
            printf("%*s ", max_blocks_len, row->blocks_str);
        }

        if (is_long) {
            printf("%s %*s %-*s  %-*s  %*s %s ",
                   row->mode_str,
                   max_nlink_len, row->nlink_str,
                   max_owner_len, row->owner_str,
                   max_group_len, row->group_str,
                   max_size_len, row->size_str,
                   row->time_str);
        }

        print_escaped_name(row->name, replace_nonprint);

        if (row->classify_char != '\0') {
            putchar(row->classify_char);
        }

        if (is_long && row->link_target != NULL) {
            printf(" -> ");
            print_escaped_name(row->link_target, replace_nonprint);
        }

        putchar('\n');
    }

    free(rows);
}
