#ifndef JC_TEMPLATE_H
#define JC_TEMPLATE_H
#include "../macros/macros.h"
// #include "../../lib/macros/maros.h"

//// Data
typedef struct Template {

} Template;

//// Face
static Ret TemplateInit(Template *R() template);
static void TemplateDe(Template *R() template);

//// Imp
static Ret TemplateInit(Template *template) {
    // RAI(), PRAI()

    // Init()

    // Logic()

    Ret ret = 0;
CLEAN:
    // De()
    return ret;
FAILED:
    ret = -1;
    goto CLEAN;
}
static void TemplateDe(Template *template) {
    // safe to call on zero'd memory logic
}

#endif