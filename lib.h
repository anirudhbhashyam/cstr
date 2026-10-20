#ifndef STR_H
#define STR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <ctype.h>
#include <stdint.h>

typedef struct {
    const char* data;
    uint64_t size;
} str;

str str_create(const char* data);
str str_from_parts(const char* data, uint64_t size);
uint64_t str_size(str* string);
int32_t str_get(str* string, uint64_t i, char* buffer);
void str_substr(str* string, uint64_t start, uint64_t end);
int32_t str_endswith(str* string, const char* s);
int32_t str_startswith(str* string, const char* s);
int32_t str_equals(str* s1, str* s2);
int32_t str_find(str* string, const char* s, uint64_t* position);
void str_drop_right(str* string, uint64_t n);
void str_drop_left(str* string, uint64_t n);
void str_take_left(str* string, uint64_t n);
void str_take_right(str* string, uint64_t n);
str str_chop_by_delim(str* string, const char delim);
int32_t str_to_cstr(str* string, char* buffer, uint64_t size);
void str_trim_left(str* string);
void str_trim_right(str* string);

#ifdef __cplusplus
}
#endif

#endif // STR_H

#ifdef STR_IMPLEMENTATION
#undef STR_IMPLEMENTATION

#include <string.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define str_println(string) printf("%.*s\n", (int32_t) (string).size, (string).data)
#define str_foreach(string) for (const char* it = (string)->data, *it##_end = it + (string)->size; it < it##_end; ++it)
#define STR_FMT "%.*s"
#define STR_ARG(s) (int32_t)(s).size, (s).data

str str_create(const char* data) {
    return (str) {
        .data = data,
        .size = strlen(data)
    };
}

str str_from_buffer(const char* data, uint64_t size) {
    return (str) {
        .data = data,
        .size = size
    };
}

uint64_t str_size(str* string) {
    return string->size;
}

int32_t str_get(str* string, uint64_t i, char* buffer) {
    if (i >= string->size) return -1;
    *buffer = string->data[i];
    return 0;
}

void str_substr(str* string, uint64_t start, uint64_t end) {
    if (start > end) start = end;
    if (start >= string->size) start = string->size;
    if (end >= string->size) end = string->size;
    string->data += start;
    string->size = end - start;
}

int32_t str_endswith(str* string, const char* s) {
    uint64_t s_size = strlen(s);
    if (s_size > string->size) return 1;
    uint64_t start = string->size - s_size;
    if ( memcmp(string->data + start, s, s_size) == 0 ) {
        return 0;
    }
    return 1;
}

int32_t str_startswith(str* string, const char* s) {
    uint64_t s_size = strlen(s);
    if (s_size > string->size) return 1;
    if ( memcmp(string->data, s, s_size) == 0 ) {
        return 0;
    }
    return 1;
}

int32_t str_equals(str* s1, str* s2) {
    if (s1->size != s2->size) return 1;
    if ( memcmp(s1->data, s2->data, s1->size) == 0 ) return 0;
    return 1;
}

int32_t str_find(str* string, const char* s, uint64_t* position) {
    uint64_t s_size = strlen(s);
    if (s_size > string->size) return -1;
    if (s_size == 0) {
        *position = 0;
        return 0;
    }

    const char* ptr = string->data;
    const char* last = string->data + string->size - s_size;
    while (ptr <= last) {
        ptr = (const char*) memchr(ptr, s[0], last - ptr + 1);
        if (ptr == NULL) return -1;
        if ( memcmp(ptr, s, s_size) == 0 ) {
            *position = ptr - string->data;
            return 0;
        }
        ++ptr;
    }
    return -1;
}

void str_drop_right(str* string, uint64_t n) {
    if (n >= string->size) n = string->size;
    string->size -= n;
}

void str_drop_left(str* string, uint64_t n) {
    if (n >= string->size) n = string->size;
    string->data += n;
    string->size -= n;
}

void str_take_left(str* string, uint64_t n) {
    if (n >= string->size) n = string->size;
    str_drop_right(string, string->size - n);
}

void str_take_right(str* string, uint64_t n) {
    if (n >= string->size) n = string->size;
    str_drop_left(string, string->size - n);
}

str str_chop_by_delim(str* string, const char delim) {
    str chopped = *string;
    str_foreach(string) {
        if (*it != delim) continue;
        uint64_t chopped_size = it - string->data;
        chopped.size = chopped_size;
        str_drop_left(string, chopped_size + 1);
        return chopped;
    }
    str_drop_left(string, string->size);
    return chopped;
}

int32_t str_to_cstr(str* string, char* buffer, uint64_t size) {
    if (string->size >= size) return -1;
    memcpy(buffer, string->data, string->size);
    buffer[string->size] = '\0';
    return 0;
}

void str_trim_left(str* string) {
    str_foreach(string) {
        if (isspace((unsigned char) *it)) continue;
        str_drop_left(string, it - string->data);
        return;
    }
    str_drop_left(string, string->size);
}

void str_trim_right(str* string) {
    uint64_t i = string->size;
    while (i > 0 && isspace((unsigned char) string->data[i - 1])) --i;
    str_drop_right(string, string->size - i);
}

#ifdef __cplusplus
}
#endif

#endif
