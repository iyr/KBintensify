#ifndef textInputFieldClass
#define textInputFieldClass
#include <stdint.h>
#define MAX_INPUTFIELD_SIZE 2048

// Defines a class for getting text input from a user
class textInputField {
  public:
    textInputField(void);
    ~textInputField(void);
    void parseKey(uint16_t key);
    const char* getField(void);
  private:
    uint32_t  cursor = 0;           // text input cursor
    bool      replaceChar = false;  // false == insert characters after cursor
    char      fieldBuff[MAX_INPUTFIELD_SIZE];
};
#endif
