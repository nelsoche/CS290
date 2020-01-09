/*
 * CS 261 Data Structures
 * Assignment 5
 * Name: Chelsea Nelson
 * Date: 10/21/19
 */

#include "hashMap.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Allocates a string for the next word in the file and returns it. This string
 * is null terminated. Returns NULL after reaching the end of the file.
 * @param file
 * @return Allocated string or NULL.
 */
char* nextWord(FILE* file)
{
    int maxLength = 16;
    int length = 0;
    char* word = malloc(sizeof(char) * maxLength);
    while (1)
    {
        char c = fgetc(file);
        if ((c >= '0' && c <= '9') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            c == '\'')
        {
            if (length + 1 >= maxLength)
            {
                maxLength *= 2;
                word = realloc(word, maxLength);
            }
            word[length] = c;
            length++;
        }
        else if (length > 0 || c == EOF)
        {
            break;
        }
    }
    if (length == 0)
    {
        free(word);
        return NULL;
    }
    word[length] = '\0';
    return word;
}

/**
 * Loads the contents of the file into the hash map.
 * @param file
 * @param map
 */
void loadDictionary(FILE* file, HashMap* map)
{
    char* word = nextWord(file);

    while(word != NULL){
        hashMapPut(map, word, 0);
        free(word);
        word = nextWord(file);
    }
    free(word);
}

/**
 * Determines and returns the minimum of three values:
 * @param a
 * @param b
 * @param c
 **/
int minimum(int a, int b, int c){
    if(a <= b && a <= c){
        return a;
    }
    else if(b <= a && b <= c){
        return b;
    }
    else{
        return c;
    }
}


/**
 * Calculates the Levenshtein distance between user's input word and a dictionary
 * word. (Code derived from the Wikipedia page on Levenshtein distance.)
 * @param str1 (user's input string)
 * @param str2 (dictionary word)
 **/
int levenshteinDistance(const char* str1, const char* str2){
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    int equality;
    int dist[len2+1][len1+1];
    dist[0][0] = 0;

    for(int x = 1; x <= len2; x++){
        dist[x][0] = dist[x-1][0] + 1;
    }
    for(int y = 1; y <= len1; y++){
        dist[0][y] = dist[0][y-1] + 1;
    }
    for(int x = 1; x <= len2; x++){
        for(int y = 1; y <= len1; y++){
            if(str1[y-1] == str2[x-1]){
                equality = 0;
            }
            else{
                equality = 1;
            }
            dist[x][y] = minimum(dist[x-1][y]+1, dist[x][y-1]+1, dist[x-1][y-1]+equality);
        }
    }
    return (dist[len2][len1]);
}

/**
 * Reproduced from hashMap.c for creating new hashLinks for the spelling suggestions
 **/
HashLink* newHashLink(const char* key, int value, HashLink* next)
{
    HashLink* link = malloc(sizeof(HashLink));
    link->key = malloc(sizeof(char) * (strlen(key) + 1));
    strcpy(link->key, key);
    link->value = value;
    link->next = next;
    return link;
}

/**
 * Checks the spelling of the word provded by the user. If the word is spelled incorrectly,
 * print the 5 closest words as determined by a metric like the Levenshtein distance.
 * Otherwise, indicate that the provded word is spelled correctly. Use dictionary.txt to
 * create the dictionary.
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, const char** argv)
{
    // FIXME: implement
    HashMap* map = hashMapNew(1000);

    FILE* file = fopen("dictionary.txt", "r");
    clock_t timer = clock();
    loadDictionary(file, map);
    timer = clock() - timer;
    printf("Dictionary loaded in %f seconds\n", (float)timer / (float)CLOCKS_PER_SEC);
    fclose(file);

    char inputBuffer[256];
    int quit = 0;
    while (!quit)
    {
        printf("Enter a word or \"quit\" to quit: ");
        scanf("%s", inputBuffer);

        char c;
        int isWord = 1;
        for(int i = 0; i < strlen(inputBuffer); i++){
            c = inputBuffer[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'))){
                isWord = 0;
                printf("Words do not contain digits or symbols.\n");
                break;
            }
            if(c >= 'A' && c <= 'Z'){
                inputBuffer[i] += 32;
            }
        }

        if(isWord == 1){
            if(hashMapContainsKey(map, inputBuffer)){
                if(strcmp(inputBuffer, "quit") == 0){
                    printf("You spelled 'quit' correctly! Goodbye.\n\n");
                    quit = 1;
                }
                else{
                    printf("The inputted word .... is spelled correctly\n\n");
                }
            }
            else{
                int levDist;
                int* newLev;
                char* dictWord;
                struct HashLink* current;
                struct HashLink** altWords = malloc(sizeof(struct HashLink*) * 5);
                printf("The inputted word .... is spelled incorrectly\n");
                printf("Did you mean...?\n");
                for(int i = 0; i < 5; i++){
                    altWords[i] = NULL;
                }
                for(int i = 0; i < hashMapCapacity(map); i++){
                    current = map->table[i];
                    while(current != NULL){
                        dictWord = current->key;
                        newLev = hashMapGet(map, dictWord);
                        (*newLev) = levenshteinDistance(inputBuffer, dictWord);
                        current = current->next;
                    }
                }
                for(int i = 0; i < hashMapCapacity(map); i++){
                    current = map->table[i];
                    while(current != NULL){
                        levDist = current->value;
                        for(int j = 0; j < 5; j++){
                            if(altWords[j] == NULL){
                                altWords[j] = newHashLink(current->key, current->value, NULL);
                                break;
                            }
                            else if(levDist < altWords[j]->value){
                                free(altWords[j]->key);
                                free(altWords[j]);
                                altWords[j] = newHashLink(current->key, current->value, NULL);
                                break;
                            }
                        }
                        current = current->next;
                    }
                }

                for(int i = 0; i < 5; i++){
                    printf("%s\n", altWords[i]->key);
                }
                printf("\n");

                for(int i = 0; i < 5; i++){
                    free(altWords[i]->key);
                    free(altWords[i]);
                }
                free(altWords);
            }
        }
    }

    hashMapDelete(map);
    return 0;
}
