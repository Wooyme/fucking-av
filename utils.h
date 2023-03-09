#ifdef _WIN32

#include <windows.h>
#include <direct.h>

#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <stdio.h>
#include <unistd.h>

bool mjs_path_exists(char *path) {
#ifdef _WIN32
    DWORD attribs = GetFileAttributesA(path);
    return (attribs != INVALID_FILE_ATTRIBUTES);
#else
    struct stat info;
    return (stat(path, &info) == 0)
#endif
}

int mjs_mk_dirs(const char *path) {
    char buffer[256];
    size_t length = strlen(path);

    // Create each subdirectory in the given path recursively
    for (size_t i = 0; i < length; i++) {
        if (path[i] == '/' || path[i] == '\\') {
            strncpy(buffer, path, i);
            buffer[i] = '\0';
#ifdef _WIN32
            _mkdir(buffer);
#else
            mode_t mode = 0755;
            mkdir(buffer, mode);
#endif
        }
    }

    // Create the last subdirectory in the given path
#ifdef _WIN32
    _mkdir(path);
#else
    mode_t mode = 0755;
    mkdir(path, mode);
#endif

    return 0;
}

const char *mjs_get_home() {
#ifdef _WIN32
    const char *homeDir = getenv("ProgramFiles");
#else
    const char* homeDir = getenv("HOME");
#endif
    return homeDir;
}

char *mjs_get_tmp_dir() {
    static char *temp_dir = nullptr;
    if (temp_dir == nullptr) {
#ifdef _WIN32
        DWORD length = GetTempPathA(0, nullptr);
        temp_dir = (char *) malloc(length + 1);
        GetTempPathA(length + 1, temp_dir);
#else
        temp_dir = getenv("TMPDIR");
        if (temp_dir == NULL) {
            temp_dir = "/tmp";
        }
#endif
    }
    return temp_dir;
}


char *mjs_get_current_path() {
    static char *current_path = nullptr;
    if (current_path == nullptr) {
        current_path = (char *) malloc(1024);
        getcwd(current_path, 1024);
    }
    return current_path;
}

void mjs_copy(const char *src, const char *dest) {
    FILE *source_file, *dest_file;
    source_file = fopen(src, "rb");
    if (source_file == nullptr) {
        log_info("fopen source_file");
        return;
    }
    dest_file = fopen(dest, "wb");
    if (dest_file == nullptr) {
        log_info("fopen destination_file");
        return;
    }
    char buffer[4096];
    size_t read_size;
    while ((read_size = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
        if (fwrite(buffer, 1, read_size, dest_file) != read_size) {
            log_info("Failed to write to destination file.\n");
            fclose(source_file);
            fclose(dest_file);
            return;
        }
        memset(buffer, 0, 4096);
    }
    fclose(source_file);
    fclose(dest_file);
    log_info("File copied successfully.\n");
}