
#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <ctype.h>

typedef struct S_FileType
{
    const char *description;
    const char *ext[4];
} FileType;

static const FileType g_filetypes[] = {
    {"C source", {".c"}},
    {"C++ source", {".cpp", ".cc", ".cxx", ".c++"}},
    {"C/C++ headers", {".h", ".hpp"}},
    {"CMake file", {"CMakeLists.txt"}},
    {"GNU Makefile", {"Makefile"}},
    {"Linux Kconfig", {"Kconfig"}}};

typedef struct S_FileTypeStat
{
    size_t file_count;

    size_t total_size;
    size_t total_lines;

    size_t actual_lines;
    size_t comment_lines;
    size_t empty_lines;
} FileTypeStat;

static FileTypeStat *g_filestats;

static int is_excluded(const char *path, char **excludes, size_t exclude_count)
{
    for (size_t i = 0; i < exclude_count; ++i)
    {
        if (strcmp(path, excludes[i]) == 0)
            return 1;
    }

    return 0;
}

static float bytes_to_human_readable(size_t bytes, char unit[2])
{
    float fbytes = bytes;
    int i = 0;
    while (fbytes >= 1024)
    {
        ++i;
        fbytes /= 1024.f;
    }

    if (unit)
    {
        switch (i)
        {
        case 0:
            strcpy(unit, "B");
            break;
        case 1:
            strcpy(unit, "K");
            break;
        case 2:
            strcpy(unit, "M");
            break;
        case 3:
            strcpy(unit, "G");
            break;
        }
    }
    return fbytes;
}

static int line_is_empty(const char *line)
{
    while (*line)
    {
        if (!isspace(*line))
            return 0;

        ++line;
    }

    return 1;
}

static int stat_file_lines(const char *path, FileTypeStat *fts)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        printf("Could not open \"%s\" for reading!\n", path);
        return 1;
    }

    char *line = NULL;
    size_t linelen = 0;
    size_t read;

    while ((read = getline(&line, &linelen, f)) != -1)
    {
        ++fts->total_lines;
        if (line_is_empty(line))
            ++fts->empty_lines;
        else
            ++fts->actual_lines;

        fts->total_size += read * sizeof(char);
    }

    if (line)
        free(line);

    fclose(f);
    return 0;
}

static int stat_file(const char *fullpath)
{
    for (size_t i = 0; i < sizeof(g_filetypes) / sizeof(FileType); ++i)
    {
        const FileType *ft = &g_filetypes[i];

        for (size_t k = 0; k < sizeof(ft->ext) / sizeof(char *); ++k)
        {
            const char *ext = ft->ext[k];
            char *start = NULL;
            if (ext && (start = strstr(fullpath, ext)))
            {
                size_t fulllen = strlen(fullpath);
                size_t extlen = strlen(ext);
                if (start - fullpath + extlen != fulllen)
                    continue;

                FileTypeStat *fts = &g_filestats[i];
                ++fts->file_count;
                stat_file_lines(fullpath, fts);
            }
        }
    }
}

static void do_loading_animation()
{
    static struct timeval tv;
    static size_t i = 0;
    static size_t last_ms = 0;

    gettimeofday(&tv, NULL);
    size_t ms = (long long)tv.tv_sec * 1000 + tv.tv_usec / 1000;

    if (ms < last_ms + 100)
        return;

    char c = "-\\|/"[i];
    i = i == 3 ? 0 : i + 1;
    last_ms = ms;

    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bThinking... [%c] ", c);
    fflush(stdout);
}

int init()
{
    size_t filetype_count = sizeof(g_filetypes) / sizeof(FileType);
    g_filestats = malloc(filetype_count * sizeof(FileTypeStat));
    if (!g_filestats)
        exit(2);

    memset(g_filestats, 0, filetype_count * sizeof(FileTypeStat));
}

int run(char **paths, size_t path_count, char **excludes, size_t exclude_count)
{
    for (size_t i = 0; i < path_count; ++i)
    {
        struct dirent *ent;
        DIR *d = opendir(paths[i]);
        if (!d)
        {
            printf("Could not open dir %s!\n", paths[i]);
            continue;
        }

        while ((ent = readdir(d)) != NULL)
        {
            do_loading_animation();

            if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
                continue;

            if (is_excluded(ent->d_name, excludes, exclude_count))
                continue;

            size_t maxlen = strlen(paths[i]) + strlen(ent->d_name) + 2;
            char *fullpath = malloc(maxlen);
            if (!fullpath)
                exit(2);

            snprintf(fullpath, maxlen, "%s/%s", paths[i], ent->d_name);

            if (ent->d_type == DT_DIR)
            {
                char *tmp[1] = {
                    fullpath};

                run(tmp, 1, excludes, exclude_count);
            }
            else if (ent->d_type == DT_UNKNOWN)
            {
                continue;
            }
            else
            {
                stat_file(fullpath);
            }

            free(fullpath);
        }

        closedir(d);
    }

    return 0;
}

int dump()
{
    size_t filetype_count = sizeof(g_filetypes) / sizeof(FileType);

    for (size_t i = 0; i < 20; ++i)
        printf("\b");

    printf(",-----------------------------------------------------------------------------------------,\n");
    printf("|   Description   |  No. of files  |  Size  |   Lines   |  Empty lines  |  Comment lines  |\n");
    printf("|-----------------+----------------+--------+-----------+---------------+-----------------|\n");

    for (size_t i = 0; i < filetype_count; ++i)
    {
        const FileType *ft = &g_filetypes[i];
        FileTypeStat *fts = &g_filestats[i];
        if (!fts->file_count)
            continue;

        char unit[2];
        float size = bytes_to_human_readable(fts->total_size, unit);
        char fmt[20];
        snprintf(fmt, 20, "%.1f%s", size, unit);

        printf("| %-15s | %-14u | %-6s | %-9u | %-13u | %-15u |\n", ft->description, fts->file_count, fmt, fts->actual_lines, fts->empty_lines, fts->comment_lines);

        // printf("%s has %d files\n", ft->description, fts->file_count);
    }

    printf("`-----------------------------------------------------------------------------------------`\n");
}

int destroy()
{
    free(g_filestats);
}
