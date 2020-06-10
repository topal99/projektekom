#include <stdint.h>

#define HASH_SIZE 20
#define MULTIPLIER 31

typedef enum
{
   ATRIB,
   ADD,
   SUB,
   MUL,
   DIV,
   IF_I,
   PRINT,
   READ,
   GOTO_I,
   LABEL,
   QUIT
} OpKind;

typedef enum
{
   EMPTY,
   INT_CONST,
   STRING
} ElemKind;

typedef struct
{
   ElemKind kind;
   union {
      int val;
      char *name;
   } contents;
} Elem;

typedef struct
{
   OpKind op;
   Elem *first, *second, *third;
} Instr;

typedef struct List
{
   char *key;
   intptr_t value;
   struct List *next;
} List;

typedef struct _InstrList
{
   Instr *instr;
   struct _InstrList *next;
} InstrList;

List *table[HASH_SIZE];
InstrList *instrList;

Instr *parser(char *line);
void run(InstrList *instrList);

Elem *mkVar(char *s);
Elem *mkInt(int x);
Elem *mkEmpty();
Instr *mkInstr(OpKind op, Elem *x, Elem *y, Elem *z);
InstrList *mkList(Instr *instr, InstrList **startInstrList, InstrList *endInstrList);

void runATRIB(char *name, int val);
void runADD(char *name, int a, int b);
void runSUB(char *name, int a, int b);
void runMUL(char *name, int a, int b);
void runDIV(char *name, int a, int b);
InstrList *runIF_I(char *var, char *label);
void runPRINT(char *var);
void runREAD(char *var);
InstrList *runGOTO_I(char *label);
void runQUIT();

void initHashTable();
unsigned int hash(char *str);
List *lookup(char *key);
void insert(char *key, intptr_t value);
