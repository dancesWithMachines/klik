/*
 * File: dictionary.h
 * Project: Klik
 * -----
 * This source code is released under BSD-3 license.
 * Check LICENSE file for full list of conditions and disclaimer.
 * -----
 * Copyright 2022 - 2022 M.Kusiak (timax)
 */

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