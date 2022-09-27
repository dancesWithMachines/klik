#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"

#include "dictionary.h"

/**
 * @brief Get string from dictionary by entry.
 *
 * @param dictionary    dictionary.
 * @param entry         entry.
 * @return char*        string.
 */
char *dictionaryGetString(dictionary_t *dictionary, uint8_t entry)
{
    while (dictionary->entry)
    {
        if (dictionary->entry == entry)
            return dictionary->string;
        dictionary++;
    }
    return NULL;
}

/**
 * @brief Get dictionary entry by string.
 *
 * @param dictionary    dictionary.
 * @param string        string.
 * @return uint8_t      dictionary entry.
 */
uint8_t dictionaryGetEntry(dictionary_t *dictionary, char *string)
{
    while (dictionary->string && strcmp(dictionary->string, string) != 0)
        dictionary++;

    return dictionary->entry;
}