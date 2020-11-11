#include "../macros/macros.h"
#include "json.h"

//#define T0_JSON_STRING "{456: test123  [] , 123  \"test\""
#define T0_JSON_STRING  "{test:123, test2:556, ___woo :   42, " \
                        " b1: [\"a\",\"b\",\"c\"],"\
                        " nested:{test:123, test2:556, ___woo :   42}, " \
                        " nested2:{test:123, test2:556, ___woo :   42, nested:{test:123, test2:556, ___woo :   42} }, " \
                        " a1: [1,2,3,4,5,6,7,\"test\",{test:123, test2:556, ___woo :   42}, 5335445432343500548,  ]," \
                        " a2: [1,2,3,4,5,6,7,\"test\",5335445432343500548, {test:123, test2:556, ___woo :   42, a1: [1,2,3,4,5,6,7,\"test\",5335445432343500548, {test:123, test2:556, ___woo :   42}, ],}, ]," \
                        "}" \

void T0() {
    PRAI(JSON, json);
    PRAI(JSON, json2);
    PRAI(Str, str);

    CHECK(JSONInit(json, T0_JSON_STRING));

    CHECK(StrInit(str, ""));

    //CHECK(StrInit(str, "Output:\n"));
    CHECK(JSONToStr(json, str, 1));
    //CHECK(StrPush(str, "\n\nOutput Compact:\n"));
    //CHECK(JSONToStr(json, str, 1));

    for(usize i = 0; i < 1024; ++i) {
        printf("%zu: \n%s\n\n", i, StrIng(str));
        CHECK(JSONInit(json2, StrIng(str)));
        StrClear(str);
        CHECK(JSONToStr(json2, str, 1));
        JSONDe(json2); memset(json2, 0, sizeof(JSON));
    }

    printf("%s\n", StrIng(str));

CLEAN:
    JSONDe(json);
    StrDe(str);
    JSONDe(json2);
    return;
FAILED:
    goto CLEAN;
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