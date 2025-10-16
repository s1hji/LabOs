#include <dirent.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifdef __APPLE__
#include <sys/xattr.h>
#endif

#define COLOR_RESET   "\033[0m"
#define COLOR_DIR     "\033[1;34m"
#define COLOR_EXEC    "\033[1;32m"
#define COLOR_LINK    "\033[1;36m"

typedef struct {
    char *name;
    struct stat st;
} FileInfo;

static int compare_files(const void *a, const void *b);
static void print_permissions(mode_t mode, const char *path);
static void print_file_info(const FileInfo *file, int long_format, int use_colors, int max_nlinks_width, int max_owner_width, int max_group_width, int max_size_width, const char *path);
static void list_directory(const char *path, int long_format, int show_all, int multiple_paths);
static char *get_color(const struct stat *st, int use_colors);
static int get_max_name_length(FileInfo *files, int count);
static void print_short_format(FileInfo *files, int count, int use_colors, int term_width);

int main(int argc, char *argv[]) {
    int long_format = 0;
    int show_all = 0;
    int opt;
    int use_colors = isatty(STDOUT_FILENO);

    while ((opt = getopt(argc, argv, "la")) != -1) {
        switch (opt) {
            case 'l':
                long_format = 1;
                break;
            case 'a':
                show_all = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-la] [path...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    if (optind == argc) {
        list_directory(".", long_format, show_all, 0);
        return 0;
    }

    int multiple_paths = (argc - optind > 1);
    for (int i = optind; i < argc; i++) {
        struct stat st;
        if (stat(argv[i], &st) == -1) {
            perror(argv[i]);
            continue;
        }
        if (S_ISDIR(st.st_mode)) {
            list_directory(argv[i], long_format, show_all, multiple_paths);
        } else {
            if (multiple_paths) {
                printf("%s:\n", argv[i]);
            }
            FileInfo file = {.name = argv[i], .st = st};
            if (long_format) {
                char owner[256], group[256];
                struct passwd *pwd = getpwuid(st.st_uid);
                struct group *grp = getgrgid(st.st_gid);
                snprintf(owner, sizeof(owner), "%s", pwd ? pwd->pw_name : "unknown");
                snprintf(group, sizeof(group), "%s", grp ? grp->gr_name : "unknown");
                int nlinks_width = snprintf(NULL, 0, "%lu", (unsigned long)st.st_nlink);
                int owner_width = strlen(owner);
                int group_width = strlen(group);
                int size_width = snprintf(NULL, 0, "%lld", (long long)st.st_size);
                print_file_info(&file, long_format, use_colors, nlinks_width, owner_width, group_width, size_width, argv[i]);
            } else {
                const char *color = get_color(&st, use_colors);
                printf("%s%s%s\n", color, argv[i], use_colors ? COLOR_RESET : "");
            }
            if (multiple_paths) {
                printf("\n");
            }
        }
    }
    return 0;
}

static int compare_files(const void *a, const void *b) {
    const FileInfo *fa = a;
    const FileInfo *fb = b;
    return strcmp(fa->name, fb->name);
}

static void print_permissions(mode_t mode, const char *path) {
    putchar(S_ISDIR(mode) ? 'd' : (S_ISLNK(mode) ? 'l' : '-'));
    putchar(mode & S_IRUSR ? 'r' : '-');
    putchar(mode & S_IWUSR ? 'w' : '-');
    putchar(mode & S_IXUSR ? 'x' : '-');
    putchar(mode & S_IRGRP ? 'r' : '-');
    putchar(mode & S_IWGRP ? 'w' : '-');
    putchar(mode & S_IXGRP ? 'x' : '-');
    putchar(mode & S_IROTH ? 'r' : '-');
    putchar(mode & S_IWOTH ? 'w' : '-');
    putchar(mode & S_IXOTH ? 'x' : '-');
#ifdef __APPLE__
    if (listxattr(path, NULL, 0, XATTR_NOFOLLOW) > 0) {
        putchar('@');
    } else {
        putchar(' ');
    }
#else
    putchar(' ');
#endif
}

static char *get_color(const struct stat *st, int use_colors) {
    if (!use_colors) return "";
    if (S_ISDIR(st->st_mode)) return COLOR_DIR;
    if (S_ISLNK(st->st_mode)) return COLOR_LINK;
    if (st->st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return COLOR_EXEC;
    return "";
}

static void print_file_info(const FileInfo *file, int long_format, int use_colors, int max_nlinks_width, int max_owner_width, int max_group_width, int max_size_width, const char *path) {
    const struct stat *st = &file->st;
    const char *color = get_color(st, use_colors);
    if (long_format) {
        print_permissions(st->st_mode, path);
        printf("%*lu ", max_nlinks_width + 1, (unsigned long)st->st_nlink);
        
        struct passwd *pwd = getpwuid(st->st_uid);
        printf("%-*s  ", max_owner_width, pwd ? pwd->pw_name : "unknown");
        
        struct group *grp = getgrgid(st->st_gid);
        printf("%-*s  ", max_group_width, grp ? grp->gr_name : "unknown");
        
        printf("%*lld ", max_size_width + 1, (long long)st->st_size);
        
        char time_buf[20];
        struct tm *tm = localtime(&st->st_mtime);
        strftime(time_buf, sizeof(time_buf), "%b %e %H:%M", tm);
        printf("%s ", time_buf);
    }
    printf("%s%s%s", color, file->name, use_colors ? COLOR_RESET : "");
    if (long_format) printf("\n");
}

static void list_directory(const char *path, int long_format, int show_all, int multiple_paths) {
    DIR *dir = opendir(path);
    if (!dir) {
        perror(path);
        return;
    }

    FileInfo *files = NULL;
    int count = 0;
    long long total_blocks = 0;
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (!show_all && entry->d_name[0] == '.') continue;
        
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
        
        struct stat st;
        if (lstat(full_path, &st) == -1) continue;
        
        files = realloc(files, (count + 1) * sizeof(FileInfo));
        files[count].name = strdup(entry->d_name);
        files[count].st = st;
        total_blocks += st.st_blocks; // Суммируем блоки
        count++;
    }
    closedir(dir);

    qsort(files, count, sizeof(FileInfo), compare_files);

    if (multiple_paths) {
        printf("%s:\n", path);
    }

    if (long_format) {
        printf("total %lld\n", total_blocks / 2);
        int max_nlinks_width = 1;
        int max_owner_width = 1;
        int max_group_width = 1;
        int max_size_width = 1;
        for (int i = 0; i < count; i++) {
            char buf[256];
            int len;
            
            len = snprintf(buf, sizeof(buf), "%lu", (unsigned long)files[i].st.st_nlink);
            if (len > max_nlinks_width) max_nlinks_width = len;
            
            struct passwd *pwd = getpwuid(files[i].st.st_uid);
            len = strlen(pwd ? pwd->pw_name : "unknown");
            if (len > max_owner_width) max_owner_width = len;
            
            struct group *grp = getgrgid(files[i].st.st_gid);
            len = strlen(grp ? grp->gr_name : "unknown");
            if (len > max_group_width) max_group_width = len;
            
            len = snprintf(buf, sizeof(buf), "%lld", (long long)files[i].st.st_size);
            if (len > max_size_width) max_size_width = len;
        }

        for (int i = 0; i < count; i++) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", path, files[i].name);
            print_file_info(&files[i], long_format, isatty(STDOUT_FILENO), max_nlinks_width, max_owner_width, max_group_width, max_size_width, full_path);
        }
    } else {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_width = w.ws_col > 0 ? w.ws_col : 80;
        
        print_short_format(files, count, isatty(STDOUT_FILENO), term_width);
    }

    if (multiple_paths) {
        printf("\n");
    }

    for (int i = 0; i < count; i++) {
        free(files[i].name);
    }
    free(files);
}

static int get_max_name_length(FileInfo *files, int count) {
    int max = 0;
    for (int i = 0; i < count; i++) {
        int len = strlen(files[i].name);
        if (len > max) max = len;
    }
    return max;
}

static void print_short_format(FileInfo *files, int count, int use_colors, int term_width) {
    if (count == 0) return;

    int max_len = get_max_name_length(files, count);
    int col_width = max_len + 2;
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;

    int rows = (count + cols - 1) / cols;

    for (int row = 0; row < rows; row++) {
        for (int col = 0; col < cols; col++) {
            int idx = col * rows + row;
            if (idx >= count) break;
            print_file_info(&files[idx], 0, use_colors, 0, 0, 0, 0, NULL);
            int name_len = strlen(files[idx].name);
            for (int pad = name_len; pad < max_len + 1; pad++) {
                putchar(' ');
            }
        }
        printf("\n");
    }
}