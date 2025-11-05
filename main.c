#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_COUNT 50000
#define MAX_SUCCESSOR_COUNT 64

char *tokens[MAX_WORD_COUNT];
size_t tokens_size = 0;

char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
size_t succs_sizes[MAX_WORD_COUNT];

static void replace_non_printable_chars_with_space(char *book) {
    size_t n = strlen(book);
    for (size_t i = 0; i < n; i++) {
        unsigned char c = (unsigned char)book[i];
        if (!isprint(c)) {
            book[i] = ' ';
        }
    }
}

static size_t token_id(char *token) {
    for (size_t i = 0; i < tokens_size; i++) {
        if (strcmp(tokens[i], token) == 0)
            return i;
    }
    if (tokens_size >= MAX_WORD_COUNT) {
        return tokens_size ? (tokens_size - 1) : 0; // clamp if full
    }
#ifdef _WIN32
    char *dup = _strdup(token);
#else
    char *dup = strdup(token);
#endif
    if (!dup) {
        return tokens_size ? (tokens_size - 1) : 0; // allocation failed
    }
    tokens[tokens_size] = dup;
    return tokens_size++;
}

static void append_to_succs(char *token, char *succ) {
    size_t id = token_id(token);
    if (id < MAX_WORD_COUNT && succs_sizes[id] < MAX_SUCCESSOR_COUNT) {
#ifdef _WIN32
        char *dup = _strdup(succ);
#else
        char *dup = strdup(succ);
#endif
        if (!dup) return;
        succs[id][succs_sizes[id]++] = dup;
    }
}

static void tokenize_and_fill_succs(char *delimiters, char *str) {
    char *prev = NULL;
    char *tok = strtok(str, delimiters);
    while (tok) {
        token_id(tok);
        if (prev)
            append_to_succs(prev, tok);
        prev = tok;
        tok = strtok(NULL, delimiters);
    }
}

static char last_char(char *str) {
    size_t len = strlen(str);
    return len > 0 ? str[len - 1] : '\0';
}

static bool token_ends_a_sentence(char *token) {
    char c = last_char(token);
    return (c == '.' || c == '?' || c == '!');
}

static size_t random_token_id_that_starts_a_sentence(void) {
    if (tokens_size == 0)
        return 0;
    size_t id;
    do {
        id = (size_t)(rand() % (int)tokens_size);
    } while (!isupper((unsigned char)tokens[id][0]));
    return id;
}

static char *generate_sentence(char *sentence, size_t sentence_size) {
    sentence[0] = '\0';
    if (tokens_size == 0)
        return sentence;

    size_t current_token_id = random_token_id_that_starts_a_sentence();
    strcat(sentence, tokens[current_token_id]);
    size_t cur_len = strlen(sentence);

    if (token_ends_a_sentence(tokens[current_token_id]))
        return sentence;

    size_t sentence_len_next;
    do {
        if (succs_sizes[current_token_id] == 0)
            break;

        size_t next_index = (size_t)(rand() % (int)succs_sizes[current_token_id]);
        char *next_token = succs[current_token_id][next_index];

        size_t next_len = strlen(next_token);
        sentence_len_next = cur_len + 1 + next_len; // space + token
        if (sentence_len_next >= sentence_size - 1)
            break;

        sentence[cur_len++] = ' ';
        sentence[cur_len] = '\0';
        strcat(sentence, next_token);
        cur_len += next_len;
        current_token_id = token_id(next_token);
    } while (!token_ends_a_sentence(tokens[current_token_id]));

    return sentence;
}

int main(void) {
    const char *filename = "pg84.txt";
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Could not open file '%s'\n", filename);
        return 1;
    }

    if (fseek(f, 0, SEEK_END) != 0) {
        fprintf(stderr, "Failed to seek in file '%s'\n", filename);
        fclose(f);
        return 1;
    }
    long fsize = ftell(f);
    if (fsize < 0) {
        fprintf(stderr, "Failed to determine file size for '%s'\n", filename);
        fclose(f);
        return 1;
    }
    rewind(f);

    char *book = (char *)malloc((size_t)fsize + 1);
    if (!book) {
        fprintf(stderr, "Memory allocation failed.\n");
        fclose(f);
        return 1;
    }

    size_t nread = fread(book, 1, (size_t)fsize, f);
    if (nread != (size_t)fsize) {
        fprintf(stderr, "Failed to read file '%s' (read %zu of %ld bytes)\n", filename, nread, fsize);
        fclose(f);
        free(book);
        return 1;
    }
    book[fsize] = '\0';
    fclose(f);

    replace_non_printable_chars_with_space(book);

    char *delimiters = " \n\r\t";
    tokenize_and_fill_succs(delimiters, book);

    srand((unsigned)time(NULL));

    char sentence[1000];

    do {
        generate_sentence(sentence, sizeof sentence);
    } while (last_char(sentence) != '?');
    puts(sentence);
    puts("");

    do {
        generate_sentence(sentence, sizeof sentence);
    } while (last_char(sentence) != '!');
    puts(sentence);

    // Cleanup allocated memory
    for (size_t i = 0; i < tokens_size; ++i) {
        for (size_t j = 0; j < succs_sizes[i]; ++j) {
            free(succs[i][j]);
        }
    }
    for (size_t i = 0; i < tokens_size; ++i) {
        free(tokens[i]);
    }
    free(book);
    return 0;
}