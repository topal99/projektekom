#include "iver-lexer.h"

#include <stdio.h>
#include <stdlib.h>

enum {
    STS_ACC,
    STS_REJ,
    STS_LET,
};

typedef uint8_t sts_t;

#define TR(st, tr) (*s = (st), (STS_##tr))
#define REJ TR(0, REJ)

#define IS_ALPHA(c)  (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define IS_DIGIT(c)  ((c) >= '0' && (c) <= '9')
#define IS_ALNUM(c)  (IS_ALPHA(c) || IS_DIGIT(c))
#define IS_WSPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

#define TOKEN_DEFINE_1(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, ACC) : REJ; \
    case 1: return REJ; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_2(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, LET) : REJ; \
    case 1: return c == (str)[1] ? TR(2, ACC) : REJ; \
    case 2: return REJ; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_3(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, LET) : REJ; \
    case 1: return c == (str)[1] ? TR(2, LET) : REJ; \
    case 2: return c == (str)[2] ? TR(3, ACC) : REJ; \
    case 3: return REJ; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_4(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, LET) : REJ; \
    case 1: return c == (str)[1] ? TR(2, LET) : REJ; \
    case 2: return c == (str)[2] ? TR(3, LET) : REJ; \
    case 3: return c == (str)[3] ? TR(4, ACC) : REJ; \
    case 4: return REJ; \
    default: abort(); \
    } \
}

#define TOKEN_DEFINE_5(token, str) \
static sts_t token(const uint8_t c, uint8_t *const s) \
{ \
    switch (*s) { \
    case 0: return c == (str)[0] ? TR(1, LET) : REJ; \
    case 1: return c == (str)[1] ? TR(2, LET) : REJ; \
    case 2: return c == (str)[2] ? TR(3, LET) : REJ; \
    case 3: return c == (str)[3] ? TR(4, LET) : REJ; \
    case 4: return c == (str)[4] ? TR(5, ACC) : REJ; \
    case 5: return REJ; \
    default: abort(); \
    } \
}

static sts_t tk_name(const uint8_t c, uint8_t *const s)
{
    enum {
        tk_name_begin,
        tk_name_accum,
    };

    switch (*s) {
    case tk_name_begin:
        return IS_ALPHA(c) || (c == '_') ? TR(tk_name_accum, ACC) : REJ;

    case tk_name_accum:
        return IS_ALNUM(c) || (c == '_') ? STS_ACC : REJ;
    }

    abort();
}

static sts_t tk_nmbr(const uint8_t c, uint8_t *const s)
{
    (void) s;
    return IS_DIGIT(c) ? STS_ACC : STS_REJ;
}

static sts_t tk_strl(const uint8_t c, uint8_t *const s)
{
    enum {
        tk_strl_begin,
        tk_strl_accum,
        tk_strl_end,
    };

    switch (*s) {
    case tk_strl_begin:
        return c == '"' ? TR(tk_strl_accum, LET) : REJ;

    case tk_strl_accum:
        return c != '"' ? STS_LET : TR(tk_strl_end, ACC);

    case tk_strl_end:
        return REJ;
    }

    abort();
}

static sts_t tk_wspc(const uint8_t c, uint8_t *const s)
{
    enum {
        tk_wspc_begin,
        tk_wspc_accum,
    };

    switch (*s) {
    case tk_wspc_begin:
        return IS_WSPACE(c) ? TR(tk_wspc_accum, ACC) : REJ;

    case tk_wspc_accum:
        return IS_WSPACE(c) ? STS_ACC : REJ;
    }

    abort();
}

static sts_t tk_lcom(const uint8_t c, uint8_t *const s)
{
    enum {
        tk_lcom_begin,
        tk_lcom_first_slash,
        tk_lcom_accum,
        tk_lcom_end
    };

    switch (*s) {
    case tk_lcom_begin:
        return c == '/' ? TR(tk_lcom_first_slash, LET) : REJ;

    case tk_lcom_first_slash:
        return c == '/' ? TR(tk_lcom_accum, LET) : REJ;

    case tk_lcom_accum:
        return c == '\n' || c == '\r' ? TR(tk_lcom_end, ACC) : STS_LET;

    case tk_lcom_end:
        return REJ;
    }

    abort();
}

static sts_t tk_bcom(const uint8_t c, uint8_t *const s)
{
    enum {
        tk_bcom_begin,
        tk_bcom_open_slash,
        tk_bcom_accum,
        tk_bcom_close_star,
        tk_bcom_end
    };

    switch (*s) {
    case tk_bcom_begin:
        return c == '/' ? TR(tk_bcom_open_slash, LET) : REJ;

    case tk_bcom_open_slash:
        return c == '*' ? TR(tk_bcom_accum, LET) : REJ;

    case tk_bcom_accum:
        return c != '*' ? STS_LET : TR(tk_bcom_close_star, LET);

    case tk_bcom_close_star:
        return c == '/' ? TR(tk_bcom_end, ACC) : TR(tk_bcom_accum, LET);

    case tk_bcom_end:
        return REJ;
    }

    abort();
}

TOKEN_DEFINE_1(tk_lpar, "(")
TOKEN_DEFINE_1(tk_rpar, ")")
TOKEN_DEFINE_1(tk_lbra, "[")
TOKEN_DEFINE_1(tk_rbra, "]")
TOKEN_DEFINE_1(tk_lbrc, "{")
TOKEN_DEFINE_1(tk_rbrc, "}")
TOKEN_DEFINE_2(tk_cond, "iv")
TOKEN_DEFINE_4(tk_elif, "eliv")
TOKEN_DEFINE_4(tk_else, "elsv")
TOKEN_DEFINE_2(tk_dowh, "dv")
TOKEN_DEFINE_5(tk_whil, "whilv")
TOKEN_DEFINE_1(tk_assn, "=")
TOKEN_DEFINE_2(tk_equl, "==")
TOKEN_DEFINE_2(tk_neql, "!=")
TOKEN_DEFINE_1(tk_lthn, "<")
TOKEN_DEFINE_1(tk_gthn, ">")
TOKEN_DEFINE_2(tk_lteq, "<=")
TOKEN_DEFINE_2(tk_gteq, ">=")
TOKEN_DEFINE_2(tk_conj, "anv")
TOKEN_DEFINE_2(tk_disj, "ov")
TOKEN_DEFINE_1(tk_plus, "+")
TOKEN_DEFINE_1(tk_mins, "-")
TOKEN_DEFINE_1(tk_mult, "*")
TOKEN_DEFINE_1(tk_divi, "/")
TOKEN_DEFINE_1(tk_modu, "%")
TOKEN_DEFINE_1(tk_nega, "!")
TOKEN_DEFINE_5(tk_prnt, "prinv")
TOKEN_DEFINE_1(tk_scol, ";")
TOKEN_DEFINE_1(tk_ques, "?")
TOKEN_DEFINE_1(tk_coln, ":")

static sts_t (*const token_funcs[TK_COUNT])(const uint8_t, uint8_t *const) = {
    tk_name,
    tk_nmbr,
    tk_strl,
    tk_wspc,
    tk_lcom,
    tk_bcom,
    tk_lpar,
    tk_rpar,
    tk_lbra,
    tk_rbra,
    tk_lbrc,
    tk_rbrc,
    tk_cond,
    tk_elif,
    tk_else,
    tk_dowh,
    tk_whil,
    tk_assn,
    tk_equl,
    tk_neql,
    tk_lthn,
    tk_gthn,
    tk_lteq,
    tk_gteq,
    tk_conj,
    tk_disj,
    tk_plus,
    tk_mins,
    tk_mult,
    tk_divi,
    tk_modu,
    tk_nega,
    tk_prnt,
    tk_scol,
    tk_ques,
    tk_coln,
};

static inline int push_token(struct token **const tokens,
    size_t *const ntokens, size_t *const allocated, const tk_t token,
    const uint8_t *const beg, const uint8_t *const end)
{
    if (*ntokens >= *allocated) {
        *allocated = (*allocated ?: 1) * 8;

        struct token *const tmp =
            realloc(*tokens, *allocated * sizeof(struct token));

        if (!tmp) {
            return free(*tokens), *tokens = NULL, LEX_NOMEM;
        }

        *tokens = tmp;
    }

    (*tokens)[(*ntokens)++] = (struct token) {
        .beg = beg,
        .end = end,
        .tk = token
    };

    return LEX_OK;
}

int lex(const uint8_t *const input, const size_t size,
    struct token **const tokens, size_t *const ntokens)
{
    static struct {
        sts_t prev, curr;
    } statuses[TK_COUNT] = {
        [0 ... TK_COUNT - 1] = { STS_LET, STS_REJ }
    };

    uint8_t states[TK_COUNT] = {0};

    const uint8_t *prefix_beg = input, *prefix_end = input;
    tk_t accepted_token;
    size_t allocated = 0;
    *tokens = NULL, *ntokens = 0;

    #define PUSH_OR_NOMEM(tk, beg, end) \
        if (push_token(tokens, ntokens, &allocated, (tk), (beg), (end))) { \
            return LEX_NOMEM; \
        }

    #define foreach_tk \
        for (tk_t tk = 0; tk < TK_COUNT; ++tk)

    PUSH_OR_NOMEM(TK_FBEG, NULL, NULL);

    while (prefix_end < input + size) {
        int did_accept = 0;

        foreach_tk {
            if (statuses[tk].prev != STS_REJ) {
                statuses[tk].curr = token_funcs[tk](*prefix_end, &states[tk]);
            }

            if (statuses[tk].curr != STS_REJ) {
                did_accept = 1;
            }
        }

        if (did_accept) {
            prefix_end++;

            foreach_tk {
                statuses[tk].prev = statuses[tk].curr;
            }
        } else {
            accepted_token = TK_COUNT;

            foreach_tk {
                if (statuses[tk].prev == STS_ACC) {
                    accepted_token = tk;
                }

                statuses[tk].prev = STS_LET;
                statuses[tk].curr = STS_REJ;
            }

            PUSH_OR_NOMEM(accepted_token, prefix_beg, prefix_end);

            if (accepted_token == TK_COUNT) {
                (*tokens)[*ntokens - 1].end++;
                return LEX_UNKNOWN_TOKEN;
            }

            prefix_beg = prefix_end;
        }
    }

    accepted_token = TK_COUNT;

    foreach_tk {
        if (statuses[tk].curr == STS_ACC) {
            accepted_token = tk;
        }

        statuses[tk].prev = STS_LET;
        statuses[tk].curr = STS_REJ;
    }

    PUSH_OR_NOMEM(accepted_token, prefix_beg, prefix_end);

    if (accepted_token == TK_COUNT) {
        return LEX_UNKNOWN_TOKEN;
    }

    PUSH_OR_NOMEM(TK_FEND, NULL, NULL);
    return LEX_OK;

    #undef PUSH_OR_NOMEM
    #undef foreach_tk
}
