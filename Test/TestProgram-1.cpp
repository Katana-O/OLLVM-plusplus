#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "AES.h"

char *input;
char enc[100] = "\xce\x6a\x64\x09\x74\x00\xf7\xf0\x86\x31\x5b\x78\xe3\x05\x6f\xfa\xcf\x31\x00\x45\xff\x19\xdf\xeb\xea\xcc\x33\x97\x26\xce\xbc\xa3"; 
void encrypt(unsigned char *dest, char *src){
    int len = strlen(src);
    for(int i = 0;i < len;i ++){
        dest[i] = (src[i] + (32 - i)) ^ i; 
    } 
}

//flag{s1mpl3_11vm_d3m0} 
int main(int argc, char *argv[]){ 
    printf("Welcome to LLVM world...\n");
    if(argc <= 1){
        printf("Input your flag as an argument.\n");
        return 0;
    }
    input = argv[1];
    printf("Your flag is: %s\n", input);
    AES aes((BYTE*)"1234567812345678", 128);
    int outlen;
    unsigned char *dest = aes.EncryptECB((BYTE*)"flag{s1mpl3_11vm_d3m0}", 22, outlen);
    bool result = strlen(input) == 22 && !memcmp(dest, enc, 32); 
    if(result){ 
        printf("Congratulations~\n"); 
    }else{
        printf("Sorry try again.\n"); 
    }
}