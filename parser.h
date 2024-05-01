#pragma once
#include <stdbool.h>
#include "lexer.h"
#include "ad.h"
void parse(Token *tokens);

bool unit(); // 1
bool structDef(); // 2
bool varDef(); // 3
bool typeBase(Type *t); // 4
bool arrayDecl(Type *t); // 5
bool fnType(); // functie ajutatoare pt fnDef
bool fnDef(); // 6
bool fnParam(); // 7
bool stm(); // 8
bool stmCompound(bool newDomain); // 9
bool expr(); // 10
bool exprAssign(); // 11
bool exprOrPrim(); // functie ajutatoare pt exprOR
bool exprOr(); // 12 - recursivitate la stanga
bool exprAndPrim(); // functie ajutatoare pt exprAND
bool exprAnd(); // 13 - recursivitate la stanga
bool exprEqPrim(); // functie ajutatoare pt exprEq
bool exprEq(); // 14 - recursivitate la stanga
bool exprRelPrim(); // fuctie ajutatoare pentru exprRel
bool exprRel(); // 15 - recursivitate la staga
bool exprAddPrim(); // fuctie ajutatoare pentru exprAdd
bool exprAdd(); // 16 - recursivitate la stanga
bool exprMulPrim(); // functie ajutatoare pentru exprMul
bool exprMul(); // 17 - recursicitate la stanga
bool exprCast(); // 18
bool exprUnary(); // 19
bool exprPostfixPrim(); // functie ajutatoare pentru exprPostfix
bool exprPostfix(); // 20 - recursivitate la stanga
bool exprPrimary(); // 21

