#include "iver-parser.h"
#include "iver-lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define RULE_RHS_LAST 7
#define GRAMMAR_SIZE (sizeof(grammar) / sizeof(*grammar))
#define SKIP_TOKEN(t) ((t) == TK_WSPC || (t) == TK_LCOM || (t) == TK_BCOM)

#define n(_nt) { .nt = NT_##_nt, .is_tk = 0, .is_mt = 0 }
#define m(_nt) { .nt = NT_##_nt, .is_tk = 0, .is_mt = 1 }
#define t(_tm) { .tk = TK_##_tm, .is_tk = 1, .is_mt = 0 }
#define no     { .tk = TK_COUNT, .is_tk = 1, .is_mt = 0 }

#define r1(_lhs, t1) \
    { .lhs = NT_##_lhs, .rhs = { no, no, no, no, no, no, no, t1, } },
#define r2(_lhs, t1, t2) \
    { .lhs = NT_##_lhs, .rhs = { no, no, no, no, no, no, t1, t2, } },
#define r3(_lhs, t1, t2, t3) \
    { .lhs = NT_##_lhs, .rhs = { no, no, no, no, no, t1, t2, t3, } },
#define r4(_lhs, t1, t2, t3, t4) \
    { .lhs = NT_##_lhs, .rhs = { no, no, no, no, t1, t2, t3, t4, } },
#define r5(_lhs, t1, t2, t3, t4, t5) \
    { .lhs = NT_##_lhs, .rhs = { no, no, no, t1, t2, t3, t4, t5, } },
#define r6(_lhs, t1, t2, t3, t4, t5, t6) \
    { .lhs = NT_##_lhs, .rhs = { no, no, t1, t2, t3, t4, t5, t6, } },
#define r7(_lhs, t1, t2, t3, t4, t5, t6, t7) \
    { .lhs = NT_##_lhs, .rhs = { no, t1, t2, t3, t4, t5, t6, t7, } },

static const struct rule {
    /* left-hand side of production */
    const nt_t lhs;

    /* array of RULE_RHS_LAST + 1 terms which form the right-hand side */
    const struct term {
        /* a rule RHS term is either a terminal token or a non-terminal */
        union {
            const tk_t tk;
            const nt_t nt;
        };

        /* indicates which field of the above union to use */
        const uint8_t is_tk: 1;

        /* indicates that the non-terminal can be matched multiple times */
        const uint8_t is_mt: 1;
    } rhs[RULE_RHS_LAST + 1];
} grammar[] = {
    r3(Unit, t(FBEG), m(Stmt), t(FEND)                                         )

    r1(Stmt, n(Assn)                                                           )
    r1(Stmt, n(Prnt)                                                           )
    r1(Stmt, n(Ctrl)                                                           )

    r4(Assn, t(NAME), t(ASSN), n(Expr), t(SCOL)                                )
    r4(Assn, n(Aexp), t(ASSN), n(Expr), t(SCOL)                                )

    r3(Prnt, t(PRNT), n(Expr), t(SCOL)                                         )
    r4(Prnt, t(PRNT), t(STRL), n(Expr), t(SCOL)                                )

    r2(Ctrl, n(Cond), m(Elif)                                                  )
    r3(Ctrl, n(Cond), m(Elif), n(Else)                                         )
    r1(Ctrl, n(Dowh)                                                           )
    r1(Ctrl, n(Whil)                                                           )

    r5(Cond, t(COND), n(Expr), t(LBRC), m(Stmt), t(RBRC)                       )
    r5(Elif, t(ELIF), n(Expr), t(LBRC), m(Stmt), t(RBRC)                       )
    r4(Else, t(ELSE), t(LBRC), m(Stmt), t(RBRC)                                )

    r7(Dowh, t(DOWH), t(LBRC), m(Stmt), t(RBRC), t(WHIL), n(Expr), t(SCOL)     )
    r5(Whil, t(WHIL), n(Expr), t(LBRC), m(Stmt), t(RBRC)                       )

    r1(Atom, t(NAME)                                                           )
    r1(Atom, t(NMBR)                                                           )

    r1(Expr, n(Atom)                                                           )
    r1(Expr, n(Pexp)                                                           )
    r1(Expr, n(Bexp)                                                           )
    r1(Expr, n(Uexp)                                                           )
    r1(Expr, n(Texp)                                                           )
    r1(Expr, n(Aexp)                                                           )

    r3(Pexp, t(LPAR), n(Expr), t(RPAR)                                         )

    r3(Bexp, n(Expr), t(EQUL), n(Expr)                                         )
    r3(Bexp, n(Expr), t(NEQL), n(Expr)                                         )
    r3(Bexp, n(Expr), t(LTHN), n(Expr)                                         )
    r3(Bexp, n(Expr), t(GTHN), n(Expr)                                         )
    r3(Bexp, n(Expr), t(LTEQ), n(Expr)                                         )
    r3(Bexp, n(Expr), t(GTEQ), n(Expr)                                         )
    r3(Bexp, n(Expr), t(CONJ), n(Expr)                                         )
    r3(Bexp, n(Expr), t(DISJ), n(Expr)                                         )
    r3(Bexp, n(Expr), t(PLUS), n(Expr)                                         )
    r3(Bexp, n(Expr), t(MINS), n(Expr)                                         )
    r3(Bexp, n(Expr), t(MULT), n(Expr)                                         )
    r3(Bexp, n(Expr), t(DIVI), n(Expr)                                         )
    r3(Bexp, n(Expr), t(MODU), n(Expr)                                         )

    r2(Uexp, t(PLUS), n(Expr)                                                  )
    r2(Uexp, t(MINS), n(Expr)                                                  )
    r2(Uexp, t(NEGA), n(Expr)                                                  )

    r5(Texp, n(Expr), t(QUES), n(Expr), t(COLN), n(Expr)                       )

    r4(Aexp, t(NAME), t(LBRA), n(Expr), t(RBRA)                                )
};

#undef r1
#undef r2
#undef r3
#undef r4
#undef r5
#undef r6
#undef r7

#undef n
#undef m
#undef t
#undef no

static const uint8_t precedence[TK_MODU - TK_EQUL + 1] = {
    4, 4, 3, 3, 3, 3, 5, 6, 2, 2, 1, 1, 1,
};

static struct {
    size_t size, allocated;
    struct node *nodes;
} stack;

static void print_stack(void)
{
    static const char *const nts[NT_COUNT] = {
        "Unit",
        "Stmt",
        "Assn",
        "Prnt",
        "Ctrl",
        "Cond",
        "Elif",
        "Else",
        "Dowh",
        "Whil",
        "Atom",
        "Expr",
        "Pexp",
        "Bexp",
        "Uexp",
        "Texp",
        "Aexp",
    };

    for (size_t i = 0; i < stack.size; ++i) {
        const struct node *const node = &stack.nodes[i];

        if (node->nchildren) {
            printf(YELLOW("%s "), nts[node->nt]);
        } else if (node->token->tk == TK_FBEG) {
            printf(GREEN("^ "));
        } else if (node->token->tk == TK_FEND) {
            printf(GREEN("$ "));
        } else {
            const ptrdiff_t len = node->token->end - node->token->beg;
            printf(GREEN("%.*s "), (int) len, node->token->beg);
        }
    }

    puts("");
}

static void destroy_node(const struct node *const node)
{
    if (node->nchildren) {
        for (size_t child_idx = 0; child_idx < node->nchildren; ++child_idx) {
            destroy_node(node->children[child_idx]);
        }

        free(node->children[0]);
        free(node->children);
    }
}

static void deallocate_stack(void)
{
    free(stack.nodes);
    stack.nodes = NULL;
    stack.size = 0;
    stack.allocated = 0;
}

static void destroy_stack(void)
{
    for (size_t node_idx = 0; node_idx < stack.size; ++node_idx) {
        destroy_node(&stack.nodes[node_idx]);
    }

    deallocate_stack();
}

static inline int term_eq_node(
    const struct term *const term,
    const struct node *const node)
{
    const int node_is_leaf = node->nchildren == 0;

    if (term->is_tk == node_is_leaf) {
        if (node_is_leaf) {
            return term->tk == node->token->tk;
        } else {
            return term->nt == node->nt;
        }
    }

    return 0;
}

static size_t match_rule(const struct rule *const rule, size_t *const at)
{
    const struct term *prev = NULL;
    const struct term *term = &rule->rhs[RULE_RHS_LAST];
    ssize_t st_idx = stack.size - 1;

    do {
        if (term_eq_node(term, &stack.nodes[st_idx])) {
            prev = term->is_mt ? term : NULL;
            --term, --st_idx;
        } else if (prev && term_eq_node(prev, &stack.nodes[st_idx])) {
            --st_idx;
        } else if (term->is_mt) {
            prev = NULL;
            --term;
        } else {
            term = NULL;
            break;
        }
    } while (st_idx >= 0 && !(term->is_tk && term->tk == TK_COUNT));

    const int reached_eor = term && term->is_tk && term->tk == TK_COUNT;
    const size_t reduction_size = stack.size - st_idx - 1;

    return reached_eor && reduction_size ?
        (*at = st_idx + 1, reduction_size) : 0;
}

static inline int shift(const struct token *const token)
{
    if (stack.size >= stack.allocated) {
        stack.allocated = (stack.allocated ?: 1) * 8;

        struct node *const tmp = realloc(stack.nodes,
            stack.allocated * sizeof(struct node));

        if (!tmp) {
            return PARSE_NOMEM;
        }

        stack.nodes = tmp;
    }

    stack.nodes[stack.size++] = (struct node) {
        .nchildren = 0,
        .token = token,
    };

    return PARSE_OK;
}

static inline bool should_shift_pre(
    const struct rule *const rule,
    const struct token *const tokens,
    size_t *const token_idx)
{
    if (rule->lhs == NT_Unit) {
        return false;
    }

    while (SKIP_TOKEN(tokens[*token_idx].tk)) {
        ++*token_idx;
    }

    const struct token *const ahead = &tokens[*token_idx];

    if (rule->lhs == NT_Bexp && ahead->tk >= TK_EQUL && ahead->tk <= TK_MODU) {
        /*
            Check whether the operator ahead has a lower precedence. If it has,
            let the parser shift it before applying the Bexp reduction.
        */
        const uint8_t p1 = precedence[rule->rhs[RULE_RHS_LAST - 1].tk - TK_EQUL];
        const uint8_t p2 = precedence[ahead->tk - TK_EQUL];

        if (p2 < p1) {
            return true;
        }
    } else if (rule->lhs == NT_Atom && rule->rhs[RULE_RHS_LAST].tk == TK_NAME) {
        /*
            Do not allow the left side of an assignment or an array name to
            escalate to Expr.
        */
        if (ahead->tk == TK_ASSN || ahead->tk == TK_LBRA) {
            return true;
        }
    } else if (rule->lhs == NT_Expr && rule->rhs[RULE_RHS_LAST].nt == NT_Aexp) {
        /*
            Do not allow an Aexp on the left side of an assignment to escalate
            to Expr.
        */
        if (ahead->tk == TK_ASSN) {
            return true;
        }
    }

    return false;
}

static inline bool should_shift_post(
    const struct rule *const rule,
    const struct token *const tokens,
    size_t *const token_idx)
{
    if (rule->lhs == NT_Unit) {
        return false;
    }

    while (SKIP_TOKEN(tokens[*token_idx].tk)) {
        ++*token_idx;
    }

    const struct token *const ahead = &tokens[*token_idx];

    if (rule->lhs == NT_Cond || rule->lhs == NT_Elif) {
        /* swallow the next "elif" or "else" in order to parse the whole chain */
        if (ahead->tk == TK_ELIF || ahead->tk == TK_ELSE) {
            return true;
        }
    }

    return false;
}

static int reduce(const struct rule *const rule,
    const size_t at, const size_t size)
{
    struct node *const child_nodes = malloc(size * sizeof(struct node));

    if (!child_nodes) {
        return PARSE_NOMEM;
    }

    struct node *const reduce_at = &stack.nodes[at];
    struct node **const old_children = reduce_at->children;
    reduce_at->children = malloc(size * sizeof(struct node *)) ?: old_children;

    if (reduce_at->children == old_children) {
        return free(child_nodes), PARSE_NOMEM;
    }

    for (size_t child_idx = 0, st_idx = at;
        st_idx < stack.size;
        ++st_idx, ++child_idx) {

        child_nodes[child_idx] = stack.nodes[st_idx];
        reduce_at->children[child_idx] = &child_nodes[child_idx];
    }

    child_nodes[0].children = old_children;
    reduce_at->nchildren = size;
    reduce_at->nt = rule->lhs;
    stack.size = at + 1;
    return PARSE_OK;
}

struct node parse(const struct token *const tokens, const size_t ntokens)
{
    static const struct token
        reject = { .tk = PARSE_REJECT },
        nomem  = { .tk = PARSE_NOMEM  };

    static const struct node
        err_reject = { .nchildren = 0, .token = &reject },
        err_nomem  = { .nchildren = 0, .token = &nomem  };

    #define SHIFT_OR_NOMEM(t) \
        if (shift(t)) { \
            puts(RED("Out of memory on shift!")); \
            return destroy_stack(), err_nomem; \
        }

    #define REDUCE_OR_NOMEM(r, a, s) \
        if (reduce(r, a, s)) { \
            puts(RED("Out of memory on reduce!")); \
            return destroy_stack(), err_nomem; \
        }

    for (size_t token_idx = 0; token_idx < ntokens; ) {
        if (SKIP_TOKEN(tokens[token_idx].tk)) {
            ++token_idx;
            continue;
        }

        SHIFT_OR_NOMEM(&tokens[token_idx++]);
        printf(CYAN("Shift: ")), print_stack();

        try_reduce_again:;
        const struct rule *rule = grammar;

        do {
            size_t reduction_at, reduction_size;

            if ((reduction_size = match_rule(rule, &reduction_at))) {
                const bool do_shift = should_shift_pre(rule, tokens, &token_idx);

                if (!do_shift) {
                    REDUCE_OR_NOMEM(rule, reduction_at, reduction_size);
                    const ptrdiff_t rule_number = rule - grammar + 1;
                    printf(ORANGE("Red%02td: "), rule_number), print_stack();
                }

                if (do_shift || should_shift_post(rule, tokens, &token_idx)) {
                    SHIFT_OR_NOMEM(&tokens[token_idx++]);
                    printf(CYAN("Shift: ")), print_stack();
                }

                goto try_reduce_again;
            }
        } while (++rule != grammar + GRAMMAR_SIZE);
    }

    #undef SHIFT_OR_NOMEM
    #undef REDUCE_OR_NOMEM

    const int accepted = stack.size == 1 &&
        stack.nodes[0].nchildren && stack.nodes[0].nt == NT_Unit;

    printf(accepted ? GREEN("ACCEPT ") : RED("REJECT ")), print_stack();

    if (accepted) {
        const struct node ret = stack.nodes[0];
        return deallocate_stack(), ret;
    } else {
        return destroy_stack(), err_reject;
    }
}

void destroy_tree(const struct node root)
{
    destroy_node(&root);
}
