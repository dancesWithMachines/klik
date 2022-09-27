#ifndef DICTIONARY_H
#define DICTIONARY_H

typedef struct
{
    uint8_t entry;
    char *string;
} dictionary_t;

char *dictionaryGetString(dictionary_t *dictionary, uint8_t entry);
uint8_t dictionaryGetEntry(dictionary_t *dictionary, char *string);

#endif