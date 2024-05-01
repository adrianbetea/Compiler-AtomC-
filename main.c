#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "ad.h"
#include "at.h"
#include "vm.h"

int main()
{
    char *inbuf = loadFile("tests/testad.c");
    Token *tokens = tokenize(inbuf);

    free(inbuf);
    showTokensByCode(tokens);

    pushDomain();
    parse(tokens);


    showDomain(symTable,"global"); // afisare domeniu global
    dropDomain(); // sterge domeniul global


    return 0;
}
