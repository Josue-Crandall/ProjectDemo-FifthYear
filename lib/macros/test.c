#include "../macros/macros.h"
#include "template.h"

void T0() {
    
    return;
FAILED:
    exit(0);
}
void T1() {
    
    return;
FAILED:
    exit(0);
}
void T2() {
    
    return;
FAILED:
    exit(0);
}

int main(void) {
    T0(); T1(); T2();
    return 0;
}