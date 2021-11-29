#include <cstdio>
#include <cstdint>

int add(int a, int b){
    return a + b;
}

uint8_t x = 1, y, z = add(1,2);

int main(){
    y = x + 2;
    printf("%d\n", y);
}