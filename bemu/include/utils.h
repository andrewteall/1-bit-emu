#ifndef UTILS_H
    #define UTILS_H 1

/**
 * @brief Converts a number string to an integer.
 * @param numStr The String to parse into and integer.
 * @return int The integer value of the string. -1 if Failure.
 */
int str2num(char *numStr);

/**
 * @brief Rounds a float up to the nearest whole number.
 * @param floatNumber The number to round up.
 * @return int Number that has been rounded up.
 */
int roundUp(float floatNumber);

#endif