#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libfiles.h"

static int is_memory_error(void *s, const char *msg) 
{ 
    if (s == NULL) {
        fprintf(stderr, "Memory allocation error in %s!\nDetail: ", msg);
        perror(NULL);            
    }
    return !!!s;
}

static void file_err_handler(FILE *f, long result) {
    if (result == FILE_ERR || ferror(f)) {
        fprintf(stderr, "File error!\n");
        perror(NULL);
        fclose(f);
        exit(EXIT_FAILURE);
    }
}

size_t search_string(size_t *length, FILE *f, int mode, 
                     void (*compar)(size_t, size_t*, size_t*, size_t), int *status) { 
    char *buf, *cur_line, *prev_line;
    size_t bufsize = STR_BUF_SIZE * sizeof(buf[0]);
    size_t cur_len = 0, res_len = mode, res_strpos = 0, cur_strpos = 0;
    buf = malloc(bufsize);
    if (is_memory_error(buf, "search_string()")) {
        *status = MEMORY_ERR; 
        return 0;
    }
    long cursor_before = ftell(f);
    file_err_handler(f, cursor_before);
    file_err_handler(f, fseek(f, 0L, SEEK_SET));
    do {
        buf = memset(buf, '\0', bufsize);
        if (fgets(buf, bufsize, f) == NULL && !feof(f)) {
            free(buf);
            file_err_handler(f, FILE_ERR);
        }
        prev_line = buf;
        while ((cur_line = strchr(prev_line, '\n')) != NULL) {
            cur_len += cur_line - prev_line;
            cur_strpos += cur_line - prev_line + 1;
            compar(cur_len, &res_len, &res_strpos, cur_strpos);
            cur_len = 0;
            prev_line = cur_line + 1;
        }
        if (prev_line != buf) {
            cur_len = strlen(prev_line);
            cur_strpos += cur_len;
        } else {
            cur_len += strlen(buf);
            cur_strpos += strlen(buf);
        }
    } while (!feof(f));
    cur_strpos += 1;
    compar(cur_len, &res_len, &res_strpos, cur_strpos);
    free(buf);
    file_err_handler(f, fseek(f, cursor_before, SEEK_SET));
    *length = res_len;
    if (res_strpos == 0 && res_len == 0) {
        return 0;
    }
    return res_strpos - res_len - 1;
}

static void lshift(size_t bufsize, size_t blocks, size_t left, 
            size_t last_bufsize, FILE *f, size_t mod_shift) {
    char sh_buf[STR_BUF_SIZE], buf[STR_BUF_SIZE];
    size_t bufsize_mod = bufsize;
    if (mod_shift) {
        bufsize_mod = mod_shift;
    }
    file_err_handler(f, fseek(f, left, SEEK_SET));
    fread(buf, bufsize_mod, sizeof(*buf), f);
    left += bufsize_mod;
    for (int i = blocks; i > 0; i--) {
        file_err_handler(f, fseek(f, left, SEEK_SET));
        fread(sh_buf, bufsize, sizeof(*sh_buf), f);
        left -= bufsize_mod;
        file_err_handler(f, fseek(f, left, SEEK_SET));
        fwrite(sh_buf, bufsize, sizeof(*sh_buf), f);
        left += bufsize + bufsize_mod;
    }
    file_err_handler(f, fseek(f, left, SEEK_SET));
    if (last_bufsize) {
        fread(sh_buf, last_bufsize, sizeof(*sh_buf), f);
        file_err_handler(f, fseek(f, left - bufsize_mod, SEEK_SET));
        fwrite(sh_buf, last_bufsize, sizeof(*sh_buf), f);
    } else {
        file_err_handler(f, fseek(f, left - bufsize_mod, SEEK_SET));
    }
    fwrite(buf, bufsize_mod, sizeof(*buf), f);
}

void process_shift(size_t left, size_t right, size_t shift, FILE *f) {
    long cursor_before = ftell(f);
    int bufsize = STR_BUF_SIZE;
    char buf[bufsize];
    file_err_handler(f, fseek(f, left, SEEK_SET));
    long delta = right - left;
    if (delta != 0) {
        shift %= delta;
    }
    if (delta <= bufsize) {
        fread(buf, delta, sizeof(*buf), f);
        file_err_handler(f, fseek(f, left, SEEK_SET));
        for (int i = 0; i < delta; i++) {
            fputc(buf[(i + shift) % delta], f);
        }
        file_err_handler(f, fseek(f, cursor_before, SEEK_SET));
        return;
    }
    size_t blocks = delta / bufsize - 1;
    size_t last_bufsize = delta - (blocks + 1) * bufsize;
    size_t old_left = left;
    size_t iters = shift / bufsize;
    while (iters--) {
        lshift(bufsize, blocks, old_left, last_bufsize, f, 0);
    }
    size_t mod_shift = shift % bufsize;
    if (mod_shift) {
        blocks = (delta - mod_shift) / bufsize;
        last_bufsize = delta - blocks * bufsize - mod_shift;
        lshift(bufsize, blocks, old_left, last_bufsize, f, mod_shift);
    }
    file_err_handler(f, fseek(f, cursor_before, SEEK_SET));
}

void fprntline(FILE *f, size_t line_pos, size_t length) {
    char buf[STR_BUF_SIZE], *line_end;
    size_t bufsize= STR_BUF_SIZE;
    file_err_handler(f, fseek(f, line_pos, SEEK_SET));
    while (fgets(buf, bufsize, f) != NULL && length / bufsize) {
        printf("%s", buf);
        length -= bufsize;
    }
    if (ferror(f)) {    
        fclose(f);
        exit(EXIT_FAILURE);
    }
    if ((line_end = strchr(buf, '\n')) != NULL) {
        buf[line_end - buf] = '\0';
    }
    puts(buf);
}