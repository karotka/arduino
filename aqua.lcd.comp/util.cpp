#include "util.h"
#include <Arduino.h>

int getlen(const char* buffer) {
    int len = 0;
    char c = buffer[len];
    while(c != '\0'){
        ++len;
        c = buffer[len];
    }
    return len;
}

int split(char* buffer, const char delimiter, char** strs, int n) {

    //obtain len
    int len = getlen(buffer);
    int split = 0;
    strs[split] = buffer;

    for(int i = 0; i < len; ++i){
        if (buffer[i] == delimiter){
            buffer[i] = '\0';
            if( i + 1 != len){
                ++split;
                strs[split] = &buffer[i+1];
                if(split == n - 1)
                    break;
            }
        }
    }
    return split + 1;
}
