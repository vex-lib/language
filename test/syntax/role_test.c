#include "syntax/role.h"
#include <assert.h>
#include <string.h>

int main(void) {
    assert(strcmp(SyntaxRole_name(SYNTAX_ROLE_DEREFERENCE), "operator.deref") == 0);
    assert(strcmp(SyntaxRole_styleKey(SYNTAX_ROLE_DEREFERENCE), "operator.deref") == 0);
    assert(SyntaxRole_isValid(SYNTAX_ROLE_MACRO_PARAMETER));
    assert(!SyntaxRole_isValid(SYNTAX_ROLE_NONE));
    assert(SyntaxRole_isValid(SYNTAX_ROLE_IDENTIFIER));
    assert(strcmp(SyntaxRole_name(SYNTAX_ROLE_IDENTIFIER), "identifier") == 0);
    assert(strcmp(SyntaxRole_styleKey(SYNTAX_ROLE_IDENTIFIER), "text") == 0);
    for (unsigned value = 0u; value <= (unsigned) SYNTAX_ROLE_IDENTIFIER; ++value) {
        const char *name = SyntaxRole_name((SyntaxRole) value);
        const char *key = SyntaxRole_styleKey((SyntaxRole) value);
        assert(name != nullptr && name[0] != '\0');
        assert(key != nullptr && key[0] != '\0');
        if (value != (unsigned) SYNTAX_ROLE_INVALID)
            assert(strcmp(name, "invalid") != 0);
    }
    assert(!SyntaxRole_isValid((SyntaxRole) ((unsigned) SYNTAX_ROLE_IDENTIFIER + 1u)));
    assert(strcmp(SyntaxRole_name((SyntaxRole) 999u), "invalid") == 0);
    return 0;
}
