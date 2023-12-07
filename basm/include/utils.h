#ifndef UTILS_H
#define UTILS_H 1

/*
 * Destructively makes a string uppercase.
 *
 * @param string Pointer to the string to make upper case.
 * @returns char* Pointer to the uppercase string.
 */
char* toUpperString(char* string);

int roundUp(float);
int str2num(char*);

#endif