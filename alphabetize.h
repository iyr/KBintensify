#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <ctype.h>

#include "alphabetize.c"

// Compare function that "ignores" case when comparing strings
static int cmpIgnoreCase(const void* a, const void* b);

// Function to sort the array 
void alphabetize(char* strings[], int numStrings);
