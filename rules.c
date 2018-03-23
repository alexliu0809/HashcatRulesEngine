// =============================================================================
// Original Author: Jens Steube <jens.steube@gmail.com>
// Rewritten By:    llamasoft   <llamasoft@users.noreply.github.com>
// License: MIT
// =============================================================================

/*
Important Test Case:
1. e/e
2. r
3. {/}

*/


#include "rules.h"

// Increment rule position, return syntax error on premature end
#define NEXT_RULEPOS(rule_pos)             \
    do {                                   \
        if ( ++(rule_pos) == rule_len ) {  \
            errno = PREMATURE_END_OF_RULE; \
            break;                         \
        }                                  \
    } while (0)

// Read current rule position as positional value into out_var
#define NEXT_RPTOI(rule, rule_pos, out_var)          \
    do {                                             \
        (out_var) = conv_ctoi( (rule)[(rule_pos)] ); \
        if ( (out_var) == -1 ) {                     \
            errno = INVALID_POSITIONAL;              \
            break;                                   \
        }                                            \
    } while (0)


static inline bool class_num(char c)   { return( (c >= '0') && (c <= '9') ); }
static inline bool class_lower(char c) { return( (c >= 'a') && (c <= 'z') ); }
static inline bool class_upper(char c) { return( (c >= 'A') && (c <= 'Z') ); }
static inline bool class_alpha(char c) { return(class_lower(c) || class_upper(c)); }

// Single character to integer value
// 0 .. 9 =>  0 ..  9
// A .. Z => 10 .. 35
// else -1 (error)
static inline int conv_ctoi(char c)
{
    if (class_num(c)) {
        return ((int)(c - '0'));

    } else if (class_upper(c)) {
        return ((int)(c - 'A' + (char)10));

    } else {
        return(-1);
    }
}

// NOTE: toggle/lower/upper/switch functions used to be macros
// To prevent breakage, the signatures haven't been changed
// This also means that they have no return values and do no safety checks
// The functions are only used internally, so it shouldn't be an issue

// Toggle a character uppercase/lowercase at a given offset
void mangle_toggle_at(char str[BLOCK_SIZE], int offset) {
    if ( class_alpha(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to lowercase
void mangle_lower_at(char str[BLOCK_SIZE], int offset) {
    if ( class_upper(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to uppercase
void mangle_upper_at(char str[BLOCK_SIZE], int offset) {
    if ( class_lower(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Swap the characters at offsets left and right
void mangle_switch(char str[BLOCK_SIZE], int left, int right) {
    char temp  = str[left];
    str[left]  = str[right];
    str[right] = temp;
}


int mangle_lrest (char arr[BLOCK_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_LOWER_AT (arr, pos);

  return (arr_len);
}

int mangle_urest (char arr[BLOCK_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_UPPER_AT (arr, pos);

  return (arr_len);
}

int mangle_trest (char arr[BLOCK_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_TOGGLE_AT (arr, pos);

  return (arr_len);
}

int mangle_reverse (char arr[BLOCK_SIZE], int arr_len)
{
  int l;

  for (l = 0; l < arr_len; l++)
  {
    int r = arr_len - 1 - l;

    if (l >= r) break;

    MANGLE_SWITCH (arr, l, r);
  }

  return (arr_len);
}

int mangle_double (char arr[BLOCK_SIZE], int arr_len)
{
  if ((arr_len * 2) >= BLOCK_SIZE) return (arr_len);

  memcpy (&arr[arr_len], arr, (size_t) arr_len);

  return (arr_len * 2);
}

int mangle_double_times (char arr[BLOCK_SIZE], int arr_len, int times)
{
  if (((arr_len * times) + arr_len) >= BLOCK_SIZE) return (arr_len);

  int orig_len = arr_len;

  int i;

  for (i = 0; i < times; i++)
  {
    memcpy (&arr[arr_len], arr, orig_len);

    arr_len += orig_len;
  }

  return (arr_len);
}

int mangle_reflect (char arr[BLOCK_SIZE], int arr_len)
{
  if ((arr_len * 2) >= BLOCK_SIZE) return (arr_len);

  mangle_double (arr, arr_len);

  mangle_reverse (arr + arr_len, arr_len);

  return (arr_len * 2);
}

int mangle_rotate_left (char arr[BLOCK_SIZE], int arr_len)
{
  int l;
  int r;

  for (l = 0, r = arr_len - 1; r > 0; r--)
  {
    MANGLE_SWITCH (arr, l, r);
  }

  return (arr_len);
}

int mangle_rotate_right (char arr[BLOCK_SIZE], int arr_len)
{
  int l;
  int r;

  for (l = 0, r = arr_len - 1; l < r; l++)
  {
    MANGLE_SWITCH (arr, l, r);
  }

  return (arr_len);
}

int mangle_append (char arr[BLOCK_SIZE], int arr_len, char c)
{
  if ((arr_len + 1) >= BLOCK_SIZE) return (arr_len);

  arr[arr_len] = c;

  return (arr_len + 1);
}

int mangle_prepend (char arr[BLOCK_SIZE], int arr_len, char c)
{
  if ((arr_len + 1) >= BLOCK_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--)
  {
    arr[arr_pos + 1] = arr[arr_pos];
  }

  arr[0] = c;

  return (arr_len + 1);
}

int mangle_delete_at (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  int arr_pos;

  for (arr_pos = upos; arr_pos < arr_len - 1; arr_pos++)
  {
    arr[arr_pos] = arr[arr_pos + 1];
  }

  return (arr_len - 1);
}

int mangle_extract (char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
  if (upos >= arr_len) return (arr_len);

  if ((upos + ulen) > arr_len) return (arr_len);

  int arr_pos;

  for (arr_pos = 0; arr_pos < ulen; arr_pos++)
  {
    arr[arr_pos] = arr[upos + arr_pos];
  }

  return (ulen);
}

int mangle_omit (char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
  if (upos >= arr_len) return (arr_len);

  if ((upos + ulen) > arr_len) return (arr_len);

  int arr_pos;

  for (arr_pos = upos; arr_pos < arr_len - ulen; arr_pos++)
  {
    arr[arr_pos] = arr[arr_pos + ulen];
  }

  return (arr_len - ulen);
}

int mangle_insert (char arr[BLOCK_SIZE], int arr_len, int upos, char c)
{
  if (upos > arr_len) return (arr_len);

  if ((arr_len + 1) >= BLOCK_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > upos - 1; arr_pos--)
  {
    arr[arr_pos + 1] = arr[arr_pos];
  }

  arr[upos] = c;

  return (arr_len + 1);
}

int mangle_insert_multi (char arr[BLOCK_SIZE], int arr_len, int arr_pos, char arr2[BLOCK_SIZE], int arr2_len, int arr2_pos, int arr2_cpy)
{
  if ((arr_len + arr2_cpy) > BLOCK_SIZE) return (RULE_RC_REJECT_ERROR);

  if (arr_pos > arr_len) return (RULE_RC_REJECT_ERROR);

  if (arr2_pos > arr2_len) return (RULE_RC_REJECT_ERROR);

  if ((arr2_pos + arr2_cpy) > arr2_len) return (RULE_RC_REJECT_ERROR);

  if (arr2_cpy < 1) return (RULE_RC_SYNTAX_ERROR);

  memcpy (arr2, arr2 + arr2_pos, arr2_len - arr2_pos);

  memcpy (arr2 + arr2_cpy, arr + arr_pos, arr_len - arr_pos);

  memcpy (arr + arr_pos, arr2, arr_len - arr_pos + arr2_cpy);

  return (arr_len + arr2_cpy);
}

int mangle_overstrike (char arr[BLOCK_SIZE], int arr_len, int upos, char c)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] = c;

  return (arr_len);
}

int mangle_truncate_at (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  memset (arr + upos, 0, arr_len - upos);

  return (upos);
}

int mangle_replace (char arr[BLOCK_SIZE], int arr_len, char oldc, char newc)
{
  int arr_pos;

  for (arr_pos = 0; arr_pos < arr_len; arr_pos++)
  {
    if (arr[arr_pos] != oldc) continue;

    arr[arr_pos] = newc;
  }

  return (arr_len);
}

int mangle_purgechar (char arr[BLOCK_SIZE], int arr_len, char c)
{
  int arr_pos;

  int ret_len;

  for (ret_len = 0, arr_pos = 0; arr_pos < arr_len; arr_pos++)
  {
    if (arr[arr_pos] == c) continue;

    arr[ret_len] = arr[arr_pos];

    ret_len++;
  }

  return (ret_len);
}

int mangle_dupeblock_prepend (char arr[BLOCK_SIZE], int arr_len, int ulen)
{
  if (ulen > arr_len) return (arr_len);

  if ((arr_len + ulen) >= BLOCK_SIZE) return (arr_len);

  char cs[100] = { 0 };

  memcpy (cs, arr, ulen);

  int i;

  for (i = 0; i < ulen; i++)
  {
    char c = cs[i];

    arr_len = mangle_insert (arr, arr_len, i, c);
  }

  return (arr_len);
}

int mangle_dupeblock_append (char arr[BLOCK_SIZE], int arr_len, int ulen)
{
  if (ulen > arr_len) return (arr_len);

  if ((arr_len + ulen) >= BLOCK_SIZE) return (arr_len);

  int upos = arr_len - ulen;

  int i;

  for (i = 0; i < ulen; i++)
  {
    char c = arr[upos + i];

    arr_len = mangle_append (arr, arr_len, c);
  }

  return (arr_len);
}

int mangle_dupechar_at (char arr[BLOCK_SIZE], int arr_len, int upos, int ulen)
{
  if ( arr_len         ==  0) return (arr_len);
  if ((arr_len + ulen) >= BLOCK_SIZE) return (arr_len);

  char c = arr[upos];

  int i;

  for (i = 0; i < ulen; i++)
  {
    arr_len = mangle_insert (arr, arr_len, upos, c);
  }

  return (arr_len);
}

int mangle_dupechar (char arr[BLOCK_SIZE], int arr_len)
{
  if ( arr_len            ==  0) return (arr_len);
  if ((arr_len + arr_len) >= BLOCK_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--)
  {
    int new_pos = arr_pos * 2;

    arr[new_pos] = arr[arr_pos];

    arr[new_pos + 1] = arr[arr_pos];
  }

  return (arr_len * 2);
}

int mangle_switch_at_check (char arr[BLOCK_SIZE], int arr_len, int upos, int upos2)
{
  if (upos  >= arr_len) return (arr_len);
  if (upos2 >= arr_len) return (arr_len);

  MANGLE_SWITCH (arr, upos, upos2);

  return (arr_len);
}

int mangle_switch_at (char arr[BLOCK_SIZE], int arr_len, int upos, int upos2)
{
  MANGLE_SWITCH (arr, upos, upos2);

  return (arr_len);
}

int mangle_chr_shiftl (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] <<= 1;

  return (arr_len);
}

int mangle_chr_shiftr (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] >>= 1;

  return (arr_len);
}

int mangle_chr_incr (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] += 1;

  return (arr_len);
}

int mangle_chr_decr (char arr[BLOCK_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] -= 1;

  return (arr_len);
}

int mangle_title_sep (char arr[BLOCK_SIZE], int arr_len, char c)
{
  int upper_next = 1;

  int pos;

  for (pos = 0; pos < arr_len; pos++)
  {
    if (arr[pos] == c)
    {
      upper_next = 1;

      continue;
    }

    if (upper_next)
    {
      upper_next = 0;

      MANGLE_UPPER_AT (arr, pos);
    }
    else
    {
      MANGLE_LOWER_AT (arr, pos);
    }
  }

  return (arr_len);
}



Rule *clone_rule(Rule *source) {
    if (source       == NULL) { return NULL; }
    if (source->text == NULL) { return NULL; }

    Rule *rtn = (Rule *)calloc(1, sizeof(Rule));
    if (rtn == NULL) {
        fprintf(stderr, "clone_rule() failed to calloc() a Rule of size %zu\n", sizeof(Rule));
        return NULL;
    }

    rtn->text = (char *)calloc(1, (source->length + 1) * sizeof(char));
    if (rtn->text == NULL) {
        fprintf(stderr, "clone_rule() failed to calloc() a string of length %zu\n", source->length + 1);
        return NULL;
    }
    memcpy(rtn->text, source->text, source->length);

    rtn->length = source->length;
    return rtn;
}


void free_rule(Rule *rule) {
    if (rule == NULL) { return; }
    if (rule->text) { free(rule->text); }
    memset(rule, 0, sizeof(Rule));
}



// Doesn't actually run the rule, but checks it for validity and does some preprocessing
//   e.g. removing noops, validating positionals, parameter count checking
// In theory, this will make apply_rule be faster as it will require fewer validations
// It also assists in detecting duplicate rules (e.g. 'lu' == 'l:u')
int parse_rule(char *rule, int rule_len, Rule **output_rule) {
    if (rule == NULL) { return(INVALID_INPUT); }

    // Allocate a Rule if our user was lazy
    if ((*output_rule) == NULL) {
        (*output_rule) = (Rule *)calloc(1, sizeof(Rule));
    }

    // Our new rule is guaranteed to be no larger than the unparsed rule
    // Note: realloc(NULL, size) is the same as malloc(size)
    char *new_rule     = (char *)realloc((*output_rule)->text, (rule_len + 1) * sizeof(char));
    int   new_rule_len = 0;
    memset(new_rule, 0, rule_len + 1);

    int errno = 0;
    int mem_len = 0;
    int temp_int;

    // Our cursor positions for the rule
    int rule_pos = 0;
    for (rule_pos = 0; rule_pos < rule_len; rule_pos++) {
        // Always copy the operation
        new_rule[new_rule_len++] = rule[rule_pos];

        switch (rule[rule_pos]) {
            // Whitespace and no-ops are skipped
            case ' ':
            case '\t':
            case '\r':
            case RULE_OP_MANGLE_NOOP:
                // Un-copy the operation
                new_rule[new_rule_len--] = 0;
                break;

            // No parameters
            case RULE_OP_MANGLE_LREST:
            case RULE_OP_MANGLE_UREST:
            case RULE_OP_MANGLE_LREST_UFIRST:
            case RULE_OP_MANGLE_UREST_LFIRST:
            case RULE_OP_MANGLE_TREST:
            case RULE_OP_MANGLE_REVERSE:
            case RULE_OP_MANGLE_DUPEWORD:
            case RULE_OP_MANGLE_REFLECT:
            case RULE_OP_MANGLE_ROTATE_LEFT:
            case RULE_OP_MANGLE_ROTATE_RIGHT:
            case RULE_OP_MANGLE_DELETE_FIRST:
            case RULE_OP_MANGLE_DELETE_LAST:
            case RULE_OP_MANGLE_DUPECHAR_ALL:
            case RULE_OP_MANGLE_SWITCH_FIRST:
            case RULE_OP_MANGLE_SWITCH_LAST:
            case RULE_OP_MANGLE_TITLE:
                // Operation already copied, nothing to do
                break;

            // Integer
            case RULE_OP_MANGLE_TOGGLE_AT:
            case RULE_OP_MANGLE_DUPEWORD_TIMES:
            case RULE_OP_MANGLE_DELETE_AT:
            case RULE_OP_MANGLE_TRUNCATE_AT:
            case RULE_OP_MANGLE_DUPECHAR_FIRST:
            case RULE_OP_MANGLE_DUPECHAR_LAST:
            case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
            case RULE_OP_MANGLE_DUPEBLOCK_LAST:
            case RULE_OP_MANGLE_CHR_SHIFTL:
            case RULE_OP_MANGLE_CHR_SHIFTR:
            case RULE_OP_MANGLE_CHR_INCR:
            case RULE_OP_MANGLE_CHR_DECR:
            case RULE_OP_MANGLE_REPLACE_NP1:
            case RULE_OP_MANGLE_REPLACE_NM1:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Character
            case RULE_OP_MANGLE_TITLE_SEP:
            case RULE_OP_MANGLE_APPEND:
            case RULE_OP_MANGLE_PREPEND:
            case RULE_OP_MANGLE_PURGECHAR:
                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Character + Character
            case RULE_OP_MANGLE_REPLACE:
                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Integer + Integer
            case RULE_OP_MANGLE_EXTRACT:
            case RULE_OP_MANGLE_OMIT:
            case RULE_OP_MANGLE_SWITCH_AT:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;

            // Integer + Character
            case RULE_OP_MANGLE_INSERT:
            case RULE_OP_MANGLE_OVERSTRIKE:
                NEXT_RULEPOS(rule_pos);
                NEXT_RPTOI(rule, rule_pos, temp_int);
                new_rule[new_rule_len++] = rule[rule_pos];

                NEXT_RULEPOS(rule_pos);
                new_rule[new_rule_len++] = rule[rule_pos];
                break;


            default:
                errno = UNKNOWN_RULE_OP;
                break;
        }

        if (errno != 0) { break; }
    }


    // Check for processing errors and create an error message
    if (errno != 0) {
        // We use asprintf() to dynamically allocate a buffer for our error message
        // We don't need our old rule buffer any more because asprintf will create a new one
        free(new_rule);

        if (errno == PREMATURE_END_OF_RULE) {
            new_rule_len = asprintf(&new_rule,
                "premature end of rule, expected char or positional value"
            );

        } else if (errno == UNKNOWN_RULE_OP) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) is not a valid operation",
                rule[rule_pos], rule_pos
            );

        } else if (errno == INVALID_POSITIONAL) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) is not a valid position or length value",
                rule[rule_pos], rule_pos
            );

        } else if (errno == MEMORY_ERROR) {
            new_rule_len = asprintf(&new_rule,
                "'%c' (offset %d) cannot be used before memorize operation",
                rule[rule_pos], rule_pos
            );

        } else {
            new_rule_len = asprintf(&new_rule,
                "unknown error %d at operation '%c' (offset %d)",
                errno, rule[rule_pos], rule_pos
            );
            errno = UNKNOWN_ERROR;
        }
    }


    new_rule[new_rule_len] = 0;
    (*output_rule)->text   = new_rule;
    (*output_rule)->length = new_rule_len;
    return (errno < 0 ? errno : new_rule_len);
}



int apply_rule(Rule *input_rule, char *input_word, int input_len, char out[BLOCK_SIZE])
{
    if (input_rule       == NULL) { return(INVALID_INPUT); }
    if (input_rule->text == NULL) { return(INVALID_INPUT); }
    // Rule length not checked, noop rules are valid but empty
    char *rule     = input_rule->text;
    int   rule_len = input_rule->length;

    if (input_word == NULL) { return(INVALID_INPUT); }
    if (input_len  <     0 ||   in_len >= BLOCK_SIZE) { return(INVALID_INPUT); }

    //int out_len = (input_len < BLOCK_SIZE ? input_len : BLOCK_SIZE - 1);

    // Operation parameters
    int errno, rule_pos;
    for (rule_pos = 0, errno = 0; rule_pos < rule_len && !errno; rule_pos++) {
        switch (rule[rule_pos]) {
            case ' ':
            case '\t':
            case '\r':
            case RULE_OP_MANGLE_NOOP: {
                // After validation, noops shouldn't exist in the rule
                // Just in case though, we'll skip them
                break;
            }
            case RULE_OP_MANGLE_LREST: {
                mangle_lower_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_UREST: {
                mangle_upper_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_LREST_UFIRST: {
                mangle_lower_all(&out[1], out_len - 1);
                if (out_len > 0) { mangle_upper_at(out, 0); }
                break;
            }
            case RULE_OP_MANGLE_UREST_LFIRST: {
                mangle_upper_all(&out[1], out_len - 1);
                if (out_len > 0) { mangle_lower_at(out, 0); }
                break;
            }
            case RULE_OP_MANGLE_TREST: {
                mangle_toggle_all(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_TOGGLE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if (out_len > offset) { mangle_toggle_at(out, offset); }
                break;
            }
            case RULE_OP_MANGLE_REVERSE: {
                mangle_reverse(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEWORD: {
                out_len = mangle_double(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEWORD_TIMES: {
                int times = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_double_times(out, out_len, times);
                break;
            }
            case RULE_OP_MANGLE_REFLECT: {
                out_len = mangle_reflect(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_ROTATE_LEFT: {
                mangle_rotate_left(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_ROTATE_RIGHT: {
                mangle_rotate_right(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_APPEND: {
                char chr = rule[++rule_pos];

                out_len = mangle_append(out, out_len, chr);
                break;
            }
            case RULE_OP_MANGLE_PREPEND: {
                char chr = rule[++rule_pos];

                out_len = mangle_prepend(out, out_len, chr);
                break;
            }
            case RULE_OP_MANGLE_DELETE_FIRST: {
                out_len = mangle_delete_at(out, out_len, 0);
                break;
            }
            case RULE_OP_MANGLE_DELETE_LAST: {
                out_len = mangle_delete_at(out, out_len, out_len - 1);
                break;
            }
            case RULE_OP_MANGLE_DELETE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_delete_at(out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_EXTRACT: {
                int offset     = conv_ctoi(rule[++rule_pos]);
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_extract(out, out_len, offset, substr_len);
                break;
            }
            case RULE_OP_MANGLE_OMIT: {
                int offset     = conv_ctoi(rule[++rule_pos]);
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_omit(out, out_len, offset, substr_len);
                break;
            }
            case RULE_OP_MANGLE_INSERT: {
                int  offset = conv_ctoi(rule[++rule_pos]);
                char chr    = rule[++rule_pos];

                out_len = mangle_insert(out, out_len, offset, chr);
                break;
            }
            case RULE_OP_MANGLE_OVERSTRIKE: {
                int  offset = conv_ctoi(rule[++rule_pos]);
                char chr    = rule[++rule_pos];

                mangle_overstrike(out, out_len, offset, chr);
                break;
            }
            case RULE_OP_MANGLE_TRUNCATE_AT: {
                int offset = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_truncate_at(out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_REPLACE: {
                char search  = rule[++rule_pos];
                char replace = rule[++rule_pos];

                mangle_replace(out, out_len, search, replace);
                break;
            }
            case RULE_OP_MANGLE_PURGECHAR: {
                char search = rule[++rule_pos];

                out_len = mangle_purgechar(out, out_len, search);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_FIRST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupechar_at(out, out_len, 0, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_LAST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupechar_at(out, out_len, out_len - 1, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPECHAR_ALL: {
                out_len = mangle_dupechar(out, out_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEBLOCK_FIRST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupeblock_prepend(out, out_len, substr_len);
                break;
            }
            case RULE_OP_MANGLE_DUPEBLOCK_LAST: {
                int substr_len = conv_ctoi(rule[++rule_pos]);

                out_len = mangle_dupeblock_append(out, out_len, substr_len);
                break;
            }
            case RULE_OP_MANGLE_SWITCH_FIRST: {
                if (out_len > 2) { mangle_switch_at(out, out_len, 0, 1); }
                break;
            }
            case RULE_OP_MANGLE_SWITCH_LAST: {
                if (out_len > 2) { mangle_switch_at(out, out_len, out_len - 1, out_len - 2); }
                break;
            }
            case RULE_OP_MANGLE_SWITCH_AT: {
                int offset1 = conv_ctoi(rule[++rule_pos]);
                int offset2 = conv_ctoi(rule[++rule_pos]);

                mangle_switch_at_check(out, out_len, offset1, offset2);
                break;
            }
            case RULE_OP_MANGLE_CHR_SHIFTL: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_shiftl((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_SHIFTR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_shiftr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_INCR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_incr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_CHR_DECR: {
                int offset = conv_ctoi(rule[++rule_pos]);

                mangle_chr_decr((uint8_t *) out, out_len, offset);
                break;
            }
            case RULE_OP_MANGLE_REPLACE_NP1: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if ((offset + 1) < out_len) {
                    mangle_overstrike(out, out_len, offset, out[offset + 1]);
                }
                break;
            }
            case RULE_OP_MANGLE_REPLACE_NM1: {
                int offset = conv_ctoi(rule[++rule_pos]);

                if (offset >= 1 && offset < out_len) {
                    mangle_overstrike(out, out_len, offset, out[offset - 1]);
                }
                break;
            }
            case RULE_OP_MANGLE_TITLE_SEP:{
                char chr = rule[++rule_pos];
                out_len = mangle_title_sep(out, out_len, chr);
            }
            case RULE_OP_MANGLE_TITLE: {
                mangle_title_sep(out, out_len,' ');
                break;
            }
            default:
                errno = UNKNOWN_RULE_OP;
                break;
        }
    }

    if (errno != 0) { return(errno); }

    // Add the null terminator and null extra bytes (just in case)
    memset(out + out_len, 0, BLOCK_SIZE - out_len);
    return(out_len);
}