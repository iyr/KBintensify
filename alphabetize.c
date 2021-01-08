//#include "alphabetize.h"
#ifndef MAX_FILENAME_LENGTH
#define MAX_FILENAME_LENGTH 32
#endif

// Compare function that "ignores" case when comparing strings
static int cmpIgnoreCase(const void* a, const void* b) 
{ 

   // Allocate memory for temporary strings
   char x[MAX_FILENAME_LENGTH];
   char y[MAX_FILENAME_LENGTH];

   // Convert input strings to same case, copy to tmp strings
   int j = 0;
   while((*(const char**)a)[j]) {
      x[j] = toupper((*(const char**)a)[j]);
      j++;
   }
   j = 0;
   while((*(const char**)b)[j]) {
      y[j] = toupper((*(const char**)b)[j]);
      j++;
   }
  
   // Get tmp string difference
   j = strcmp(x, y);

   return j;
} 
  
// Function to sort the array 
void alphabetize(char* strings[], int numStrings) 
{ 
   qsort(strings, numStrings, sizeof(const char*), cmpIgnoreCase); 
}
