#define STR_IMPLEMENTATION
#include "lib.h"

#include <stdio.h>
#include <string.h>

static int32_t test_failures = 0;

#define TEST_ASSERT(condition) \
do { \
    if ((condition)) { \
        fprintf(stderr, "\e[0;32m[PASS]\e[0m %s:%d:\t%s\n", __func__, __LINE__, #condition); \
    } else { \
        fprintf(stderr, "\e[0;31m[FAIL]\e[0m %s:%d:\t%s\n", __func__, __LINE__, #condition); \
        ++test_failures; \
        return; \
    } \
} while(0)

void test_str_create() {
    str hello = str_create("Hello!");
    TEST_ASSERT(strcmp(hello.data, "Hello!") == 0);
    TEST_ASSERT(hello.size == 6);
}

void test_str_size() {
    str hello = str_create("Hello!");
    TEST_ASSERT(str_size(&hello) == 6);

    str nothing = str_create("");
    TEST_ASSERT(str_size(&nothing) == 0);
}

void test_str_get() {
    const char* source = "Hello, World!";
    str hello = str_create(source);
    char c;

    TEST_ASSERT(str_get(&hello, 0, &c) == 0);
    TEST_ASSERT(c == 'H');
    TEST_ASSERT(str_get(&hello, 7, &c) == 0);
    TEST_ASSERT(c == 'W');
    TEST_ASSERT(str_get(&hello, 12, &c) == 0);
    TEST_ASSERT(c == '!');

    // Out-of-range indices return -1 and leave the output untouched
    c = '?';
    TEST_ASSERT(str_get(&hello, 13, &c) == -1);
    TEST_ASSERT(str_get(&hello, 1000, &c) == -1);
    TEST_ASSERT(str_get(&hello, UINT64_MAX, &c) == -1);
    TEST_ASSERT(c == '?');

    // Indices are relative to the slice and stop at its size
    str world = str_create(source);
    str_substr(&world, 7, 12);
    TEST_ASSERT(str_get(&world, 0, &c) == 0);
    TEST_ASSERT(c == 'W');
    TEST_ASSERT(str_get(&world, 4, &c) == 0);
    TEST_ASSERT(c == 'd');
    TEST_ASSERT(str_get(&world, 5, &c) == -1);

    // A '\0' byte inside the string is still a valid result
    str with_nul = { .data = "a\0b", .size = 3 };
    TEST_ASSERT(str_get(&with_nul, 1, &c) == 0);
    TEST_ASSERT(c == '\0');
    TEST_ASSERT(str_get(&with_nul, 2, &c) == 0);
    TEST_ASSERT(c == 'b');

    str empty = str_create("");
    TEST_ASSERT(str_get(&empty, 0, &c) == -1);
}

void test_str_substr() {
    const char* source = "Hello, World!";

    str prefix = str_create(source);
    str_substr(&prefix, 0, 5);
    TEST_ASSERT(prefix.data == source);
    TEST_ASSERT(prefix.size == 5);
    TEST_ASSERT(strncmp(prefix.data, "Hello", prefix.size) == 0);

    str middle = str_create(source);
    str_substr(&middle, 7, 12);
    TEST_ASSERT(middle.data == source + 7);
    TEST_ASSERT(middle.size == 5);
    TEST_ASSERT(strncmp(middle.data, "World", middle.size) == 0);

    str whole = str_create(source);
    str_substr(&whole, 0, 13);
    TEST_ASSERT(whole.data == source);
    TEST_ASSERT(whole.size == 13);

    str empty = str_create(source);
    str_substr(&empty, 4, 4);
    TEST_ASSERT(empty.size == 0);

    str end_clamped = str_create(source);
    str_substr(&end_clamped, 7, 100);
    TEST_ASSERT(end_clamped.size == 6);
    TEST_ASSERT(strncmp(end_clamped.data, "World!", end_clamped.size) == 0);

    str start_clamped = str_create(source);
    str_substr(&start_clamped, 100, 200);
    TEST_ASSERT(start_clamped.data == source + 13);
    TEST_ASSERT(start_clamped.size == 0);

    str reversed = str_create(source);
    str_substr(&reversed, 8, 3);
    TEST_ASSERT(reversed.data == source + 3);
    TEST_ASSERT(reversed.size == 0);

    str reversed_clamped = str_create(source);
    str_substr(&reversed_clamped, 200, 100);
    TEST_ASSERT(reversed_clamped.data == source + 13);
    TEST_ASSERT(reversed_clamped.size == 0);
}

void test_str_endswith() {
    const char* source = "Hello, World!";
    str hello = str_create(source);

    TEST_ASSERT(str_endswith(&hello, "World!") == 0);
    TEST_ASSERT(str_endswith(&hello, "!") == 0);
    TEST_ASSERT(str_endswith(&hello, "Hello, World!") == 0);
    TEST_ASSERT(str_endswith(&hello, "") == 0);

    TEST_ASSERT(str_endswith(&hello, "World") == 1);
    TEST_ASSERT(str_endswith(&hello, "world!") == 1);
    TEST_ASSERT(str_endswith(&hello, "Oh, Hello, World!") == 1);

    // Checking a suffix must not modify the string
    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 13);

    // A slice is not null-terminated, so only its own bytes may count
    str slice = str_create(source);
    str_substr(&slice, 0, 5);
    TEST_ASSERT(str_endswith(&slice, "llo") == 0);
    TEST_ASSERT(str_endswith(&slice, "World!") == 1);
    TEST_ASSERT(str_endswith(&slice, "Hello, World!") == 1);

    str empty = str_create("");
    TEST_ASSERT(str_endswith(&empty, "") == 0);
    TEST_ASSERT(str_endswith(&empty, "a") == 1);
}

void test_str_startswith() {
    const char* source = "Hello, World!";
    str hello = str_create(source);

    TEST_ASSERT(str_startswith(&hello, "Hello") == 0);
    TEST_ASSERT(str_startswith(&hello, "H") == 0);
    TEST_ASSERT(str_startswith(&hello, "Hello, World!") == 0);
    TEST_ASSERT(str_startswith(&hello, "") == 0);

    TEST_ASSERT(str_startswith(&hello, "World") == 1);
    TEST_ASSERT(str_startswith(&hello, "hello") == 1);
    TEST_ASSERT(str_startswith(&hello, "Hello, World!!") == 1);

    // Checking a prefix must not modify the string
    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 13);

    // A slice is not null-terminated, so only its own bytes may count
    str slice = str_create(source);
    str_substr(&slice, 7, 10);
    TEST_ASSERT(str_startswith(&slice, "Wor") == 0);
    TEST_ASSERT(str_startswith(&slice, "World") == 1);
    TEST_ASSERT(str_startswith(&slice, "Hello") == 1);

    str empty = str_create("");
    TEST_ASSERT(str_startswith(&empty, "") == 0);
    TEST_ASSERT(str_startswith(&empty, "a") == 1);
}

void test_str_equals() {
    const char* source = "Hello, World!";
    str hello = str_create(source);
    str hello_again = str_create(source);

    TEST_ASSERT(str_equals(&hello, &hello) == 0);
    TEST_ASSERT(str_equals(&hello, &hello_again) == 0);

    // Equality compares contents, not pointers
    char copy[] = "Hello, World!";
    str hello_copy = str_create(copy);
    TEST_ASSERT(str_equals(&hello, &hello_copy) == 0);

    str shorter = str_create("Hello");
    TEST_ASSERT(str_equals(&hello, &shorter) == 1);
    TEST_ASSERT(str_equals(&shorter, &hello) == 1);

    str same_size = str_create("Hello, World?");
    TEST_ASSERT(str_equals(&hello, &same_size) == 1);

    // A slice equals a string with the same bytes, even though
    // the slice's memory continues past its size
    str slice = str_create(source);
    str_substr(&slice, 0, 5);
    TEST_ASSERT(str_equals(&slice, &shorter) == 0);
    TEST_ASSERT(str_equals(&slice, &hello) == 1);

    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 13);

    str empty = str_create("");
    str empty_slice = str_create(source);
    str_substr(&empty_slice, 4, 4);
    TEST_ASSERT(str_equals(&empty, &empty_slice) == 0);
    TEST_ASSERT(str_equals(&empty, &hello) == 1);
}

void test_str_find() {
    const char* source = "Hello, World!";
    str hello = str_create(source);
    uint64_t position;

    TEST_ASSERT(str_find(&hello, "World", &position) == 0);
    TEST_ASSERT(position == 7);
    TEST_ASSERT(str_find(&hello, "Hello", &position) == 0);
    TEST_ASSERT(position == 0);
    TEST_ASSERT(str_find(&hello, "!", &position) == 0);
    TEST_ASSERT(position == 12);
    TEST_ASSERT(str_find(&hello, "Hello, World!", &position) == 0);
    TEST_ASSERT(position == 0);
    TEST_ASSERT(str_find(&hello, "", &position) == 0);
    TEST_ASSERT(position == 0);

    // The first of several matches is returned
    TEST_ASSERT(str_find(&hello, "l", &position) == 0);
    TEST_ASSERT(position == 2);

    // The first 'o' (at 4) is a false start; the match is at 8
    TEST_ASSERT(str_find(&hello, "orl", &position) == 0);
    TEST_ASSERT(position == 8);

    // Not found returns -1 and leaves the output untouched
    position = 99;
    TEST_ASSERT(str_find(&hello, "xyz", &position) == -1);
    TEST_ASSERT(str_find(&hello, "world", &position) == -1);
    TEST_ASSERT(str_find(&hello, "!!", &position) == -1);
    TEST_ASSERT(str_find(&hello, "Hello, World!!", &position) == -1);
    TEST_ASSERT(position == 99);

    // Searching must not modify the string
    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 13);

    // Positions are relative to the slice, and matches must fit inside it
    str world = str_create(source);
    str_substr(&world, 7, 12);
    TEST_ASSERT(str_find(&world, "rl", &position) == 0);
    TEST_ASSERT(position == 2);
    TEST_ASSERT(str_find(&world, "d", &position) == 0);
    TEST_ASSERT(position == 4);
    TEST_ASSERT(str_find(&world, "!", &position) == -1);
    TEST_ASSERT(str_find(&world, "ld!", &position) == -1);
    TEST_ASSERT(str_find(&world, "Hello", &position) == -1);

    // A '\0' byte inside the string does not end the search
    str with_nul = { .data = "a\0b", .size = 3 };
    TEST_ASSERT(str_find(&with_nul, "b", &position) == 0);
    TEST_ASSERT(position == 2);

    str empty = str_create("");
    TEST_ASSERT(str_find(&empty, "", &position) == 0);
    TEST_ASSERT(position == 0);
    TEST_ASSERT(str_find(&empty, "a", &position) == -1);
}

void test_str_drop_left() {
    const char* source = "Hello, World!";

    str world = str_create(source);
    str_drop_left(&world, 7);
    TEST_ASSERT(world.data == source + 7);
    TEST_ASSERT(world.size == 6);
    TEST_ASSERT(strncmp(world.data, "World!", world.size) == 0);

    str unchanged = str_create(source);
    str_drop_left(&unchanged, 0);
    TEST_ASSERT(unchanged.data == source);
    TEST_ASSERT(unchanged.size == 13);

    str exact = str_create(source);
    str_drop_left(&exact, 13);
    TEST_ASSERT(exact.data == source + 13);
    TEST_ASSERT(exact.size == 0);

    // Dropping more than the size clamps to the end of the string
    str overflow = str_create(source);
    str_drop_left(&overflow, 100);
    TEST_ASSERT(overflow.data == source + 13);
    TEST_ASSERT(overflow.size == 0);

    str max = str_create(source);
    str_drop_left(&max, UINT64_MAX);
    TEST_ASSERT(max.data == source + 13);
    TEST_ASSERT(max.size == 0);

    // Repeated drops add up, and work on slices
    str repeated = str_create(source);
    str_drop_left(&repeated, 3);
    str_drop_left(&repeated, 4);
    TEST_ASSERT(repeated.data == source + 7);
    TEST_ASSERT(repeated.size == 6);

    str slice = str_create(source);
    str_substr(&slice, 0, 5);
    str_drop_left(&slice, 3);
    TEST_ASSERT(slice.data == source + 3);
    TEST_ASSERT(slice.size == 2);
    TEST_ASSERT(strncmp(slice.data, "lo", slice.size) == 0);

    str empty = str_create("");
    str_drop_left(&empty, 1);
    TEST_ASSERT(empty.size == 0);
}

void test_str_drop_right() {
    const char* source = "Hello, World!";

    str hello = str_create(source);
    str_drop_right(&hello, 8);
    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 5);
    TEST_ASSERT(strncmp(hello.data, "Hello", hello.size) == 0);

    str unchanged = str_create(source);
    str_drop_right(&unchanged, 0);
    TEST_ASSERT(unchanged.data == source);
    TEST_ASSERT(unchanged.size == 13);

    str exact = str_create(source);
    str_drop_right(&exact, 13);
    TEST_ASSERT(exact.data == source);
    TEST_ASSERT(exact.size == 0);

    // Dropping more than the size clamps to an empty string
    str overflow = str_create(source);
    str_drop_right(&overflow, 100);
    TEST_ASSERT(overflow.data == source);
    TEST_ASSERT(overflow.size == 0);

    str max = str_create(source);
    str_drop_right(&max, UINT64_MAX);
    TEST_ASSERT(max.data == source);
    TEST_ASSERT(max.size == 0);

    // Repeated drops add up, and work on slices
    str repeated = str_create(source);
    str_drop_right(&repeated, 1);
    str_drop_right(&repeated, 7);
    TEST_ASSERT(repeated.data == source);
    TEST_ASSERT(repeated.size == 5);

    str slice = str_create(source);
    str_substr(&slice, 7, 13);
    str_drop_right(&slice, 1);
    TEST_ASSERT(slice.data == source + 7);
    TEST_ASSERT(slice.size == 5);
    TEST_ASSERT(strncmp(slice.data, "World", slice.size) == 0);

    str empty = str_create("");
    str_drop_right(&empty, 1);
    TEST_ASSERT(empty.size == 0);
}

void test_str_take_left() {
    const char* source = "Hello, World!";

    str hello = str_create(source);
    str_take_left(&hello, 5);
    TEST_ASSERT(hello.data == source);
    TEST_ASSERT(hello.size == 5);
    TEST_ASSERT(strncmp(hello.data, "Hello", hello.size) == 0);

    str none = str_create(source);
    str_take_left(&none, 0);
    TEST_ASSERT(none.data == source);
    TEST_ASSERT(none.size == 0);

    str exact = str_create(source);
    str_take_left(&exact, 13);
    TEST_ASSERT(exact.data == source);
    TEST_ASSERT(exact.size == 13);

    // Taking more than the size keeps the whole string
    str overflow = str_create(source);
    str_take_left(&overflow, 100);
    TEST_ASSERT(overflow.data == source);
    TEST_ASSERT(overflow.size == 13);

    str max = str_create(source);
    str_take_left(&max, UINT64_MAX);
    TEST_ASSERT(max.data == source);
    TEST_ASSERT(max.size == 13);

    str slice = str_create(source);
    str_substr(&slice, 7, 13);
    str_take_left(&slice, 3);
    TEST_ASSERT(slice.data == source + 7);
    TEST_ASSERT(slice.size == 3);
    TEST_ASSERT(strncmp(slice.data, "Wor", slice.size) == 0);

    str empty = str_create("");
    str_take_left(&empty, 1);
    TEST_ASSERT(empty.size == 0);
}

void test_str_take_right() {
    const char* source = "Hello, World!";

    str world = str_create(source);
    str_take_right(&world, 6);
    TEST_ASSERT(world.data == source + 7);
    TEST_ASSERT(world.size == 6);
    TEST_ASSERT(strncmp(world.data, "World!", world.size) == 0);

    str none = str_create(source);
    str_take_right(&none, 0);
    TEST_ASSERT(none.data == source + 13);
    TEST_ASSERT(none.size == 0);

    str exact = str_create(source);
    str_take_right(&exact, 13);
    TEST_ASSERT(exact.data == source);
    TEST_ASSERT(exact.size == 13);

    // Taking more than the size keeps the whole string
    str overflow = str_create(source);
    str_take_right(&overflow, 100);
    TEST_ASSERT(overflow.data == source);
    TEST_ASSERT(overflow.size == 13);

    str max = str_create(source);
    str_take_right(&max, UINT64_MAX);
    TEST_ASSERT(max.data == source);
    TEST_ASSERT(max.size == 13);

    str slice = str_create(source);
    str_substr(&slice, 0, 5);
    str_take_right(&slice, 3);
    TEST_ASSERT(slice.data == source + 2);
    TEST_ASSERT(slice.size == 3);
    TEST_ASSERT(strncmp(slice.data, "llo", slice.size) == 0);

    str empty = str_create("");
    str_take_right(&empty, 1);
    TEST_ASSERT(empty.size == 0);
}

void test_str_chop_by_delim() {
    const char* source = "a,bc,def";
    str line = str_create(source);
    str field;

    // Each chop returns the part before the delimiter and moves past it
    field = str_chop_by_delim(&line, ',');
    TEST_ASSERT(field.data == source);
    TEST_ASSERT(field.size == 1);
    TEST_ASSERT(line.data == source + 2);
    TEST_ASSERT(line.size == 6);

    field = str_chop_by_delim(&line, ',');
    TEST_ASSERT(field.data == source + 2);
    TEST_ASSERT(field.size == 2);
    TEST_ASSERT(strncmp(field.data, "bc", field.size) == 0);
    TEST_ASSERT(line.data == source + 5);
    TEST_ASSERT(line.size == 3);

    // Without a delimiter the rest is returned and the string is emptied
    field = str_chop_by_delim(&line, ',');
    TEST_ASSERT(field.data == source + 5);
    TEST_ASSERT(field.size == 3);
    TEST_ASSERT(strncmp(field.data, "def", field.size) == 0);
    TEST_ASSERT(line.data == source + 8);
    TEST_ASSERT(line.size == 0);

    str no_delim = str_create("abc");
    field = str_chop_by_delim(&no_delim, ',');
    TEST_ASSERT(field.size == 3);
    TEST_ASSERT(no_delim.size == 0);

    // Delimiters at the start or next to each other give empty fields
    str leading = str_create(",a");
    field = str_chop_by_delim(&leading, ',');
    TEST_ASSERT(field.size == 0);
    TEST_ASSERT(leading.size == 1);

    str doubled = str_create("a,,b");
    str_chop_by_delim(&doubled, ',');
    field = str_chop_by_delim(&doubled, ',');
    TEST_ASSERT(field.size == 0);
    TEST_ASSERT(doubled.size == 1);
    TEST_ASSERT(strncmp(doubled.data, "b", doubled.size) == 0);

    // A trailing delimiter leaves an empty remainder
    str trailing = str_create("a,");
    field = str_chop_by_delim(&trailing, ',');
    TEST_ASSERT(field.size == 1);
    TEST_ASSERT(trailing.size == 0);

    str only_delim = str_create(",");
    field = str_chop_by_delim(&only_delim, ',');
    TEST_ASSERT(field.size == 0);
    TEST_ASSERT(only_delim.size == 0);

    str empty = str_create("");
    field = str_chop_by_delim(&empty, ',');
    TEST_ASSERT(field.size == 0);
    TEST_ASSERT(empty.size == 0);

    // A delimiter just past the end of a slice is not found
    str slice = str_create("ab,cd");
    str_substr(&slice, 0, 2);
    field = str_chop_by_delim(&slice, ',');
    TEST_ASSERT(field.size == 2);
    TEST_ASSERT(slice.size == 0);

    // Chopping in a loop splits the whole string
    str words = str_create("the quick brown fox");
    const char* expected[] = { "the", "quick", "brown", "fox" };
    uint64_t count = 0;
    while (str_size(&words) > 0 && count < 4) {
        field = str_chop_by_delim(&words, ' ');
        TEST_ASSERT(field.size == strlen(expected[count]));
        TEST_ASSERT(strncmp(field.data, expected[count], field.size) == 0);
        count++;
    }
    TEST_ASSERT(count == 4);
    TEST_ASSERT(words.size == 0);
}

void test_str_drop_both() {
    // Dropping from both ends leaves the middle
    const char* source = "Hello, World!";
    str middle = str_create(source);
    str_drop_left(&middle, 7);
    str_drop_right(&middle, 1);
    TEST_ASSERT(middle.data == source + 7);
    TEST_ASSERT(middle.size == 5);
    TEST_ASSERT(strncmp(middle.data, "World", middle.size) == 0);
}

void test_str_foreach() {
    const char* source = "Hello, World!";
    char seen[32];
    uint64_t count;

    str hello = str_create(source);
    count = 0;
    str_foreach(&hello) {
        if (count < sizeof(seen)) seen[count] = *it;
        count++;
    }
    TEST_ASSERT(count == 13);
    TEST_ASSERT(strncmp(seen, source, count) == 0);

    str world = str_create(source);
    str_substr(&world, 7, 12);
    count = 0;
    str_foreach(&world) {
        if (count < sizeof(seen)) seen[count] = *it;
        count++;
    }
    TEST_ASSERT(count == 5);
    TEST_ASSERT(strncmp(seen, "World", count) == 0);

    const char* first = NULL;
    str_foreach(&world) {
        first = it;
        break;
    }
    TEST_ASSERT(first == source + 7);

    str empty = str_create("");
    count = 0;
    str_foreach(&empty) count++;
    TEST_ASSERT(count == 0);
}

void test_str_to_cstr() {
    char buffer[16];

    // Fits with room to spare
    str hello = str_create("Hello");
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&hello, buffer, sizeof(buffer)) == 0);
    TEST_ASSERT(strcmp(buffer, "Hello") == 0);
    TEST_ASSERT(buffer[5] == '\0');
    TEST_ASSERT(buffer[6] == 'X');  // nothing written past the terminator

    // Exact fit: size + 1 bytes
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&hello, buffer, 6) == 0);
    TEST_ASSERT(strcmp(buffer, "Hello") == 0);
    TEST_ASSERT(buffer[6] == 'X');

    // One byte short: no room for '\0', buffer untouched
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&hello, buffer, 5) == -1);
    TEST_ASSERT(buffer[0] == 'X');
    TEST_ASSERT(buffer[4] == 'X');
    TEST_ASSERT(buffer[5] == 'X');

    // Much too small
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&hello, buffer, 2) == -1);
    TEST_ASSERT(buffer[0] == 'X');

    // Zero-sized buffer
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&hello, buffer, 0) == -1);
    TEST_ASSERT(buffer[0] == 'X');

    // Empty string needs exactly one byte
    str empty = str_create("");
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&empty, buffer, 1) == 0);
    TEST_ASSERT(buffer[0] == '\0');
    TEST_ASSERT(buffer[1] == 'X');

    // Empty string, zero-sized buffer
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&empty, buffer, 0) == -1);
    TEST_ASSERT(buffer[0] == 'X');

    // Slice in the middle of a string: terminator must cut it off
    const char* source = "Hello, World!";
    str world = str_create(source);
    str_substr(&world, 7, 12);
    memset(buffer, 'X', sizeof(buffer));
    TEST_ASSERT(str_to_cstr(&world, buffer, sizeof(buffer)) == 0);
    TEST_ASSERT(strcmp(buffer, "World") == 0);
    TEST_ASSERT(strlen(buffer) == world.size);
    TEST_ASSERT(buffer[6] == 'X');

    // Source is not modified
    TEST_ASSERT(world.data == source + 7);
    TEST_ASSERT(world.size == 5);

    // Converting each chopped field
    str csv = str_create("ab,cde");
    str field = str_chop_by_delim(&csv, ',');
    TEST_ASSERT(str_to_cstr(&field, buffer, sizeof(buffer)) == 0);
    TEST_ASSERT(strcmp(buffer, "ab") == 0);
    field = str_chop_by_delim(&csv, ',');
    TEST_ASSERT(str_to_cstr(&field, buffer, sizeof(buffer)) == 0);
    TEST_ASSERT(strcmp(buffer, "cde") == 0);
}

void test_str_trim_left() {
    // Leading spaces
    const char* source = "   abc";
    str s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 3);
    TEST_ASSERT(s.size == 3);

    // Nothing to trim
    source = "abc";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Every whitespace character isspace knows about
    source = " \t\n\v\f\rabc";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 6);
    TEST_ASSERT(s.size == 3);

    // Trailing and interior whitespace are kept
    source = "  a b  ";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 5);
    TEST_ASSERT(memcmp(s.data, "a b  ", 5) == 0);

    // First non-space character is not dropped
    source = " x";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 1);
    TEST_ASSERT(s.size == 1);
    TEST_ASSERT(s.data[0] == 'x');

    // All whitespace
    source = " \t \n";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source + 4);

    // Empty
    source = "";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source);

    // Trimming twice changes nothing
    source = "  abc";
    s = str_create(source);
    str_trim_left(&s);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 3);

    // Bytes >= 0x80 (UTF-8 "é") are not whitespace
    source = "  \xC3\xA9";
    s = str_create(source);
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 2);

    // Slice: whitespace past the end of the slice is never reached
    source = "  ab  cd";
    s = str_create(source);
    str_take_left(&s, 4);           // "  ab"
    str_trim_left(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 2);

    // Slice that is all whitespace, followed by text outside it
    source = "   abc";
    s = str_create(source);
    str_take_left(&s, 3);           // "   "
    str_trim_left(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source + 3);
}

void test_str_trim_right() {
    // Trailing spaces
    const char* source = "abc   ";
    str s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Nothing to trim
    source = "abc";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Every whitespace character isspace knows about
    source = "abc \t\n\v\f\r";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Leading and interior whitespace are kept
    source = "  a b  ";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 5);
    TEST_ASSERT(memcmp(s.data, "  a b", 5) == 0);

    // Last non-space character is not dropped
    source = "x ";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 1);
    TEST_ASSERT(s.data[0] == 'x');

    // Single character, no whitespace
    source = "x";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 1);

    // All whitespace
    source = " \t \n";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source);

    // Empty
    source = "";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source);

    // Trimming twice changes nothing
    source = "abc  ";
    s = str_create(source);
    str_trim_right(&s);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 3);

    // Bytes >= 0x80 (UTF-8 "é") are not whitespace
    source = "\xC3\xA9  ";
    s = str_create(source);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 2);

    // Slice: the text after the slice doesn't stop the trim
    source = "ab  cd";
    s = str_create(source);
    str_take_left(&s, 4);           // "ab  "
    str_trim_right(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 2);

    // Slice that is all whitespace, preceded by text outside it
    source = "abc   ";
    s = str_create(source);
    str_drop_left(&s, 3);           // "   "
    str_trim_right(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source + 3);

    // Both sides
    source = "  hello world \n";
    s = str_create(source);
    str_trim_left(&s);
    str_trim_right(&s);
    TEST_ASSERT(s.size == 11);
    TEST_ASSERT(memcmp(s.data, "hello world", 11) == 0);
}

void test_str_from_buffer() {
    // Buffer with no null terminator
    const char buffer[3] = { 'a', 'b', 'c' };
    str s = str_from_buffer(buffer, 3);
    TEST_ASSERT(s.data == buffer);
    TEST_ASSERT(s.size == 3);

    // Size is taken as given, not from strlen
    const char* source = "Hello, World!";
    s = str_from_buffer(source, 5);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 5);
    TEST_ASSERT(str_endswith(&s, "Hello") == 0);

    // Embedded '\0' is part of the string
    const char embedded[] = "a\0b";
    s = str_from_buffer(embedded, 3);
    TEST_ASSERT(s.size == 3);
    TEST_ASSERT(s.data[1] == '\0');
    TEST_ASSERT(s.data[2] == 'b');

    // Zero size
    s = str_from_buffer(source, 0);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 0);

    // Same result as str_create over the whole string
    str a = str_from_buffer(source, strlen(source));
    str b = str_create(source);
    TEST_ASSERT(a.data == b.data);
    TEST_ASSERT(a.size == b.size);
    TEST_ASSERT(str_equals(&a, &b) == 0);

    // Works with the other functions
    s = str_from_buffer(buffer, 3);
    uint64_t position;
    TEST_ASSERT(str_find(&s, "bc", &position) == 0);
    TEST_ASSERT(position == 1);
    TEST_ASSERT(str_find(&s, "cd", &position) == -1);
}

void test_str_fmt() {
    char buffer[64];

    // Whole string
    str hello = str_create("Hello");
    snprintf(buffer, sizeof(buffer), STR_FMT, STR_ARG(hello));
    TEST_ASSERT(strcmp(buffer, "Hello") == 0);

    // Slice prints only its own bytes
    str world = str_create("Hello, World!");
    str_substr(&world, 7, 12);
    snprintf(buffer, sizeof(buffer), STR_FMT, STR_ARG(world));
    TEST_ASSERT(strcmp(buffer, "World") == 0);

    // Inside a larger format string
    snprintf(buffer, sizeof(buffer), "[" STR_FMT "]", STR_ARG(world));
    TEST_ASSERT(strcmp(buffer, "[World]") == 0);

    // Several in one call
    snprintf(buffer, sizeof(buffer), STR_FMT ", " STR_FMT "!", STR_ARG(hello), STR_ARG(world));
    TEST_ASSERT(strcmp(buffer, "Hello, World!") == 0);

    // Empty
    str empty = str_create("");
    snprintf(buffer, sizeof(buffer), "<" STR_FMT ">", STR_ARG(empty));
    TEST_ASSERT(strcmp(buffer, "<>") == 0);

    // Buffer without a null terminator
    const char raw[3] = { 'x', 'y', 'z' };
    str s = str_from_buffer(raw, 3);
    snprintf(buffer, sizeof(buffer), STR_FMT, STR_ARG(s));
    TEST_ASSERT(strcmp(buffer, "xyz") == 0);

    // Return value is the printed length
    TEST_ASSERT(snprintf(buffer, sizeof(buffer), STR_FMT, STR_ARG(world)) == 5);
}

void test_str_try_chop_by_delim() {
    const char* source = "a,bc,def";
    str s = str_create(source);
    str chopped;

    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.data == source);
    TEST_ASSERT(chopped.size == 1);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 6);

    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.data == source + 2);
    TEST_ASSERT(chopped.size == 2);
    TEST_ASSERT(s.data == source + 5);
    TEST_ASSERT(s.size == 3);

    // No delimiter left: both arguments untouched
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(chopped.data == source + 2);
    TEST_ASSERT(chopped.size == 2);
    TEST_ASSERT(s.data == source + 5);
    TEST_ASSERT(s.size == 3);

    // No delimiter at all
    source = "abc";
    s = str_create(source);
    chopped = str_create("sentinel");
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);
    TEST_ASSERT(chopped.size == 8);

    // Empty
    source = "";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 0);

    // Leading delimiter gives an empty field
    source = ",a";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.data == source);
    TEST_ASSERT(chopped.size == 0);
    TEST_ASSERT(s.data == source + 1);
    TEST_ASSERT(s.size == 1);

    // Doubled delimiter gives an empty field between
    source = "a,,b";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.size == 1);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.data == source + 2);
    TEST_ASSERT(chopped.size == 0);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(s.data == source + 3);
    TEST_ASSERT(s.size == 1);

    // Trailing delimiter vs no delimiter: now distinguishable
    source = "a,";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.size == 1);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);

    source = "a";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(s.size == 1);

    // Only a delimiter
    source = ",";
    s = str_create(source);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == 0);
    TEST_ASSERT(chopped.data == source);
    TEST_ASSERT(chopped.size == 0);
    TEST_ASSERT(s.data == source + 1);
    TEST_ASSERT(s.size == 0);

    // Delimiter just past the end of a slice is not found
    source = "ab,cd";
    s = str_create(source);
    str_take_left(&s, 2);
    TEST_ASSERT(str_try_chop_by_delim(&s, ',', &chopped) == -1);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 2);

    // Buffer without a null terminator
    const char raw[4] = { 'x', ';', 'y', 'z' };
    s = str_from_buffer(raw, 4);
    TEST_ASSERT(str_try_chop_by_delim(&s, ';', &chopped) == 0);
    TEST_ASSERT(chopped.data == raw);
    TEST_ASSERT(chopped.size == 1);
    TEST_ASSERT(s.data == raw + 2);
    TEST_ASSERT(s.size == 2);

    // Split loop: delimiters + 1 fields, remainder is the last one
    s = str_create("x,y,,z");
    const char* expected[] = { "x", "y", "" };
    uint64_t count = 0;
    while (count < 4 && str_try_chop_by_delim(&s, ',', &chopped) == 0) {
        TEST_ASSERT(count < 3);
        TEST_ASSERT(chopped.size == strlen(expected[count]));
        TEST_ASSERT(memcmp(chopped.data, expected[count], chopped.size) == 0);
        ++count;
    }
    TEST_ASSERT(count == 3);
    TEST_ASSERT(s.size == 1);
    TEST_ASSERT(s.data[0] == 'z');
}

void test_str_trim() {
    // Both sides
    const char* source = "  abc  ";
    str s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 3);

    // Nothing to trim
    source = "abc";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Left side only
    source = " \t abc";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 3);
    TEST_ASSERT(s.size == 3);

    // Right side only
    source = "abc \r\n";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source);
    TEST_ASSERT(s.size == 3);

    // Every whitespace character isspace knows about, on both sides
    source = " \t\n\v\f\rabc \t\n\v\f\r";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 6);
    TEST_ASSERT(s.size == 3);

    // Interior whitespace is kept
    source = "  hello   world  ";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 13);
    TEST_ASSERT(memcmp(s.data, "hello   world", 13) == 0);

    // Single character surrounded by whitespace
    source = " x ";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 1);
    TEST_ASSERT(s.size == 1);
    TEST_ASSERT(s.data[0] == 'x');

    // All whitespace: empty, positioned at the end
    source = " \t \n ";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source + 5);

    // Empty
    source = "";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.size == 0);
    TEST_ASSERT(s.data == source);

    // Trimming twice changes nothing
    source = "  abc  ";
    s = str_create(source);
    str_trim(&s);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 2);
    TEST_ASSERT(s.size == 3);

    // Bytes >= 0x80 (UTF-8 "é") are not whitespace
    source = " \xC3\xA9 ";
    s = str_create(source);
    str_trim(&s);
    TEST_ASSERT(s.data == source + 1);
    TEST_ASSERT(s.size == 2);

    // Slice: text outside the slice doesn't stop the trim
    source = "ab  cd  ef";
    s = str_create(source);
    str_substr(&s, 2, 8);           // "  cd  "
    str_trim(&s);
    TEST_ASSERT(s.data == source + 4);
    TEST_ASSERT(s.size == 2);

    // Same result as trim_left then trim_right
    source = "\t key = value \n";
    str a = str_create(source);
    str b = str_create(source);
    str_trim(&a);
    str_trim_left(&b);
    str_trim_right(&b);
    TEST_ASSERT(a.data == b.data);
    TEST_ASSERT(a.size == b.size);

    // Typical use: trimming fields after a chop
    str line = str_create("  key = value \n");
    str key = str_chop_by_delim(&line, '=');
    str_trim(&key);
    str_trim(&line);
    TEST_ASSERT(key.size == 3);
    TEST_ASSERT(memcmp(key.data, "key", 3) == 0);
    TEST_ASSERT(line.size == 5);
    TEST_ASSERT(memcmp(line.data, "value", 5) == 0);
}

int32_t main() {
    test_str_create();
    test_str_size();
    test_str_get();
    test_str_substr();
    test_str_endswith();
    test_str_startswith();
    test_str_equals();
    test_str_find();
    test_str_drop_left();
    test_str_drop_right();
    test_str_take_left();
    test_str_take_right();
    test_str_drop_both();
    test_str_chop_by_delim();
    test_str_foreach();
    test_str_to_cstr();
    test_str_trim_left();
    test_str_trim_right();
    test_str_from_buffer();
    test_str_fmt();
    test_str_try_chop_by_delim();
    test_str_trim();
    printf("\e[1mTest failures: %d\e[0m\n", test_failures);
    return test_failures == 0 ? 0 : 1;
}
