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
#define NEXT_RULEPOS(rp)      if (++(rp) == rule_len) return (RULE_RC_SYNTAX_ERROR)

// Read current rule position as positional value into out_var
#define NEXT_RPTOI(r,rp,up)   if (((up) = conv_pos ((r)[(rp)], pos_mem)) == -1) return (RULE_RC_SYNTAX_ERROR)

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

// conv poisition to int
static int conv_pos (char c, const int pos_mem)
{
  if (c == RULE_LAST_REJECTED_SAVED_POS)
  {
    return pos_mem;
  }

  return conv_ctoi (c);
}

// NOTE: toggle/lower/upper/switch functions used to be macros
// To prevent breakage, the signatures haven't been changed
// This also means that they have no return values and do no safety checks
// The functions are only used internally, so it shouldn't be an issue

// Toggle a character uppercase/lowercase at a given offset
void MANGLE_TOGGLE_AT(char str[RP_PASSWORD_SIZE], int offset) {
    if ( class_alpha(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to lowercase
void MANGLE_LOWER_AT(char str[RP_PASSWORD_SIZE], int offset) {
    if ( class_upper(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Convert a character at offset to uppercase
void MANGLE_UPPER_AT(char str[RP_PASSWORD_SIZE], int offset) {
    if ( class_lower(str[offset]) ) {
        str[offset] ^= 0x20;
    }
}


// Swap the characters at offsets left and right
void MANGLE_SWITCH(char str[RP_PASSWORD_SIZE], int left, int right) {
    char temp  = str[left];
    str[left]  = str[right];
    str[right] = temp;
}


int mangle_lrest (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_LOWER_AT (arr, pos);

  return (arr_len);
}

int mangle_urest (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_UPPER_AT (arr, pos);

  return (arr_len);
}

int mangle_trest (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  int pos;

  for (pos = 0; pos < arr_len; pos++) MANGLE_TOGGLE_AT (arr, pos);

  return (arr_len);
}

int mangle_reverse (char arr[RP_PASSWORD_SIZE], int arr_len)
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

int mangle_double (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  if ((arr_len * 2) >= RP_PASSWORD_SIZE) return (arr_len);

  memcpy (&arr[arr_len], arr, (size_t) arr_len);

  return (arr_len * 2);
}

int mangle_double_times (char arr[RP_PASSWORD_SIZE], int arr_len, int times)
{
  if (((arr_len * times) + arr_len) >= RP_PASSWORD_SIZE) return (arr_len);

  int orig_len = arr_len;

  int i;

  for (i = 0; i < times; i++)
  {
    memcpy (&arr[arr_len], arr, orig_len);

    arr_len += orig_len;
  }

  return (arr_len);
}

int mangle_reflect (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  if ((arr_len * 2) >= RP_PASSWORD_SIZE) return (arr_len);

  mangle_double (arr, arr_len);

  mangle_reverse (arr + arr_len, arr_len);

  return (arr_len * 2);
}

int mangle_rotate_left (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  int l;
  int r;

  for (l = 0, r = arr_len - 1; r > 0; r--)
  {
    MANGLE_SWITCH (arr, l, r);
  }

  return (arr_len);
}

int mangle_rotate_right (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  int l;
  int r;

  for (l = 0, r = arr_len - 1; l < r; l++)
  {
    MANGLE_SWITCH (arr, l, r);
  }

  return (arr_len);
}

int mangle_append (char arr[RP_PASSWORD_SIZE], int arr_len, char c)
{
  if ((arr_len + 1) >= RP_PASSWORD_SIZE) return (arr_len);

  arr[arr_len] = c;

  return (arr_len + 1);
}

int mangle_prepend (char arr[RP_PASSWORD_SIZE], int arr_len, char c)
{
  if ((arr_len + 1) >= RP_PASSWORD_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--)
  {
    arr[arr_pos + 1] = arr[arr_pos];
  }

  arr[0] = c;

  return (arr_len + 1);
}

int mangle_delete_at (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  int arr_pos;

  for (arr_pos = upos; arr_pos < arr_len - 1; arr_pos++)
  {
    arr[arr_pos] = arr[arr_pos + 1];
  }

  return (arr_len - 1);
}

int mangle_extract (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, int ulen)
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

int mangle_omit (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, int ulen)
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

int mangle_insert (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, char c)
{
  if (upos > arr_len) return (arr_len);

  if ((arr_len + 1) >= RP_PASSWORD_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > upos - 1; arr_pos--)
  {
    arr[arr_pos + 1] = arr[arr_pos];
  }

  arr[upos] = c;

  return (arr_len + 1);
}

int mangle_insert_multi (char arr[RP_PASSWORD_SIZE], int arr_len, int arr_pos, char arr2[RP_PASSWORD_SIZE], int arr2_len, int arr2_pos, int arr2_cpy)
{
  if ((arr_len + arr2_cpy) > RP_PASSWORD_SIZE) return (RULE_RC_REJECT_ERROR);

  if (arr_pos > arr_len) return (RULE_RC_REJECT_ERROR);

  if (arr2_pos > arr2_len) return (RULE_RC_REJECT_ERROR);

  if ((arr2_pos + arr2_cpy) > arr2_len) return (RULE_RC_REJECT_ERROR);

  if (arr2_cpy < 1) return (RULE_RC_SYNTAX_ERROR);

  memmove (arr2, arr2 + arr2_pos, arr2_len - arr2_pos);

  memcpy  (arr2 + arr2_cpy, arr + arr_pos, arr_len - arr_pos);

  memcpy  (arr + arr_pos, arr2, arr_len - arr_pos + arr2_cpy);

  return (arr_len + arr2_cpy);
}


int mangle_overstrike (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, char c)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] = c;

  return (arr_len);
}

int mangle_truncate_at (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  memset (arr + upos, 0, arr_len - upos);

  return (upos);
}

int mangle_replace (char arr[RP_PASSWORD_SIZE], int arr_len, char oldc, char newc)
{
  int arr_pos;

  for (arr_pos = 0; arr_pos < arr_len; arr_pos++)
  {
    if (arr[arr_pos] != oldc) continue;

    arr[arr_pos] = newc;
  }

  return (arr_len);
}

int mangle_purgechar (char arr[RP_PASSWORD_SIZE], int arr_len, char c)
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

int mangle_dupeblock_prepend (char arr[RP_PASSWORD_SIZE], int arr_len, int ulen)
{
  if (ulen > arr_len) return (arr_len);

  if ((arr_len + ulen) >= RP_PASSWORD_SIZE) return (arr_len);

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

int mangle_dupeblock_append (char arr[RP_PASSWORD_SIZE], int arr_len, int ulen)
{
  if (ulen > arr_len) return (arr_len);

  if ((arr_len + ulen) >= RP_PASSWORD_SIZE) return (arr_len);

  int upos = arr_len - ulen;

  int i;

  for (i = 0; i < ulen; i++)
  {
    char c = arr[upos + i];

    arr_len = mangle_append (arr, arr_len, c);
  }

  return (arr_len);
}

int mangle_dupechar_at (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, int ulen)
{
  if ( arr_len         ==  0) return (arr_len);
  if ((arr_len + ulen) >= RP_PASSWORD_SIZE) return (arr_len);

  char c = arr[upos];

  int i;

  for (i = 0; i < ulen; i++)
  {
    arr_len = mangle_insert (arr, arr_len, upos, c);
  }

  return (arr_len);
}

int mangle_dupechar (char arr[RP_PASSWORD_SIZE], int arr_len)
{
  if ( arr_len            ==  0) return (arr_len);
  if ((arr_len + arr_len) >= RP_PASSWORD_SIZE) return (arr_len);

  int arr_pos;

  for (arr_pos = arr_len - 1; arr_pos > -1; arr_pos--)
  {
    int new_pos = arr_pos * 2;

    arr[new_pos] = arr[arr_pos];

    arr[new_pos + 1] = arr[arr_pos];
  }

  return (arr_len * 2);
}

int mangle_switch_at_check (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, int upos2)
{
  if (upos  >= arr_len) return (arr_len);
  if (upos2 >= arr_len) return (arr_len);

  MANGLE_SWITCH (arr, upos, upos2);

  return (arr_len);
}

int mangle_switch_at (char arr[RP_PASSWORD_SIZE], int arr_len, int upos, int upos2)
{
  MANGLE_SWITCH (arr, upos, upos2);

  return (arr_len);
}

int mangle_chr_shiftl (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] <<= 1;

  return (arr_len);
}

int mangle_chr_shiftr (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] >>= 1;

  return (arr_len);
}

int mangle_chr_incr (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] += 1;

  return (arr_len);
}

int mangle_chr_decr (char arr[RP_PASSWORD_SIZE], int arr_len, int upos)
{
  if (upos >= arr_len) return (arr_len);

  arr[upos] -= 1;

  return (arr_len);
}

int mangle_title_sep (char arr[RP_PASSWORD_SIZE], int arr_len, char c)
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

char *str_contains_chr(char arr[RP_PASSWORD_SIZE], int arr_len, char c){
  int pos;
  for (pos = 0; pos < arr_len; pos++)
  {
    
    if (arr[pos] == c)
    {
      return &arr[pos];
    }
  }
  return NULL;
}



int apply_rule(char *rule, int rule_len, char in[RP_PASSWORD_SIZE], int in_len, char out[RP_PASSWORD_SIZE])
{
    char mem[RP_PASSWORD_SIZE] = { 0 };

    int pos_mem = -1;

    if (rule       == NULL) { return(RULE_RC_REJECT_ERROR); }
    if (rule_len < 1)       { return(RULE_RC_REJECT_ERROR); }

    if (in == NULL)         { return(RULE_RC_REJECT_ERROR); }
    if (out == NULL)        { return(RULE_RC_REJECT_ERROR); }

    if (in_len  <     0 ||   in_len >= RP_PASSWORD_SIZE) { return(RULE_RC_REJECT_ERROR); }

    //int out_len = (input_len < RP_PASSWORD_SIZE ? input_len : RP_PASSWORD_SIZE - 1);
    int out_len = in_len;
    memset (out, 0, RP_PASSWORD_SIZE);
    memcpy (out, in, out_len);

    memcpy (mem, out, out_len);
    int mem_len = out_len;

    // Operation parameters
    int errno, rule_pos;
    for (rule_pos = 0, errno = 0; rule_pos < rule_len && !errno; rule_pos++) {
        int upos, upos2;
        int ulen;
        switch (rule[rule_pos]) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
            case RULE_OP_MANGLE_NOOP: {
                // After validation, noops shouldn't exist in the rule
                // Just in case though, we'll skip them
                break;
            }
            case RULE_OP_MANGLE_LREST:
              out_len = mangle_lrest (out, out_len);
              break;

            case RULE_OP_MANGLE_UREST:
              out_len = mangle_urest (out, out_len);
              break;

            case RULE_OP_MANGLE_LREST_UFIRST:
              out_len = mangle_lrest (out, out_len);
              if (out_len) MANGLE_UPPER_AT (out, 0);
              break;

            case RULE_OP_MANGLE_UREST_LFIRST:
              out_len = mangle_urest (out, out_len);
              if (out_len) MANGLE_LOWER_AT (out, 0);
              break;

            case RULE_OP_MANGLE_TREST:
              out_len = mangle_trest (out, out_len);
              break;

            case RULE_OP_MANGLE_TOGGLE_AT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if (upos < out_len) MANGLE_TOGGLE_AT (out, upos);
              break;

            case RULE_OP_MANGLE_REVERSE:
              out_len = mangle_reverse (out, out_len);
              break;

            case RULE_OP_MANGLE_DUPEWORD:
              out_len = mangle_double (out, out_len);
              break;

            case RULE_OP_MANGLE_DUPEWORD_TIMES:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_double_times (out, out_len, ulen);
              break;

            case RULE_OP_MANGLE_REFLECT:
              out_len = mangle_reflect (out, out_len);
              break;

            case RULE_OP_MANGLE_ROTATE_LEFT:
              mangle_rotate_left (out, out_len);
              break;

            case RULE_OP_MANGLE_ROTATE_RIGHT:
              mangle_rotate_right (out, out_len);
              break;

            case RULE_OP_MANGLE_APPEND:
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_append (out, out_len, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_PREPEND:
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_prepend (out, out_len, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_DELETE_FIRST:
              out_len = mangle_delete_at (out, out_len, 0);
              break;

            case RULE_OP_MANGLE_DELETE_LAST:
              out_len = mangle_delete_at (out, out_len, (out_len) ? out_len - 1 : 0);
              break;

            case RULE_OP_MANGLE_DELETE_AT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              out_len = mangle_delete_at (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_EXTRACT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_extract (out, out_len, upos, ulen);
              break;

            case RULE_OP_MANGLE_OMIT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_omit (out, out_len, upos, ulen);
              break;

            case RULE_OP_MANGLE_INSERT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_insert (out, out_len, upos, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_OVERSTRIKE:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_overstrike (out, out_len, upos, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_TRUNCATE_AT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              out_len = mangle_truncate_at (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_REPLACE:
              NEXT_RULEPOS (rule_pos);
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_replace (out, out_len, rule[rule_pos - 1], rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_PURGECHAR:
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_purgechar (out, out_len, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_DUPECHAR_FIRST:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_dupechar_at (out, out_len, 0, ulen);
              break;

            case RULE_OP_MANGLE_DUPECHAR_LAST:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_dupechar_at (out, out_len, out_len - 1, ulen);
              break;

            case RULE_OP_MANGLE_DUPECHAR_ALL:
              out_len = mangle_dupechar (out, out_len);
              break;

            case RULE_OP_MANGLE_DUPEBLOCK_FIRST:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_dupeblock_prepend (out, out_len, ulen);
              break;

            case RULE_OP_MANGLE_DUPEBLOCK_LAST:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              out_len = mangle_dupeblock_append (out, out_len, ulen);
              break;

            case RULE_OP_MANGLE_SWITCH_FIRST:
              if (out_len >= 2) mangle_switch_at (out, out_len, 0, 1);
              break;

            case RULE_OP_MANGLE_SWITCH_LAST:
              if (out_len >= 2) mangle_switch_at (out, out_len, out_len - 1, out_len - 2);
              break;

            case RULE_OP_MANGLE_SWITCH_AT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos2);
              out_len = mangle_switch_at_check (out, out_len, upos, upos2);
              break;

            case RULE_OP_MANGLE_CHR_SHIFTL:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              mangle_chr_shiftl (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_CHR_SHIFTR:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              mangle_chr_shiftr (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_CHR_INCR:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              mangle_chr_incr (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_CHR_DECR:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              mangle_chr_decr (out, out_len, upos);
              break;

            case RULE_OP_MANGLE_REPLACE_NP1:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if ((upos >= 0) && ((upos + 1) < out_len)) mangle_overstrike (out, out_len, upos, out[upos + 1]);
              break;

            case RULE_OP_MANGLE_REPLACE_NM1:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if ((upos >= 1) && ((upos + 0) < out_len)) mangle_overstrike (out, out_len, upos, out[upos - 1]);
              break;

            case RULE_OP_MANGLE_TITLE_SEP:
              NEXT_RULEPOS (rule_pos);
              out_len = mangle_title_sep (out, out_len, rule[rule_pos]);
              break;

            case RULE_OP_MANGLE_TITLE:
              out_len = mangle_title_sep (out, out_len, ' ');
              break;

            case RULE_OP_MANGLE_EXTRACT_MEMORY:
              if (mem_len < 1) return (RULE_RC_REJECT_ERROR);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, ulen);
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos2);
              if ((out_len = mangle_insert_multi (out, out_len, upos2, mem, mem_len, upos, ulen)) < 1) return (out_len);
              break;

            case RULE_OP_MANGLE_APPEND_MEMORY:
              if (mem_len < 1) return (RULE_RC_REJECT_ERROR);
              if ((out_len + mem_len) >= RP_PASSWORD_SIZE) return (RULE_RC_REJECT_ERROR);
              memcpy (out + out_len, mem, mem_len);
              out_len += mem_len;
              break;

            case RULE_OP_MANGLE_PREPEND_MEMORY:
              if (mem_len < 1) return (RULE_RC_REJECT_ERROR);
              if ((mem_len + out_len) >= RP_PASSWORD_SIZE) return (RULE_RC_REJECT_ERROR);
              memcpy (mem + mem_len, out, out_len);
              out_len += mem_len;
              memcpy (out, mem, out_len);
              break;

            case RULE_OP_MEMORIZE_WORD:
              memcpy (mem, out, out_len);
              mem_len = out_len;
              break;

            case RULE_OP_REJECT_LESS:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if (out_len > upos) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_GREATER:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if (out_len < upos) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_EQUAL:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if (out_len != upos) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_CONTAIN:
              NEXT_RULEPOS (rule_pos);
              if (str_contains_chr (out, out_len, rule[rule_pos]) != NULL) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_NOT_CONTAIN:
              NEXT_RULEPOS (rule_pos);
              char *match = str_contains_chr (out, out_len, rule[rule_pos]);
              //char *match = strchr (out, rule[rule_pos]); // another buggy hc
              if (match != NULL)
              {
                pos_mem = (int)(match - out);
              }
              else
              {
                return (RULE_RC_REJECT_ERROR);
              }
              break;

            case RULE_OP_REJECT_EQUAL_FIRST:
              NEXT_RULEPOS (rule_pos);
              if (out[0] != rule[rule_pos]) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_EQUAL_LAST:
              NEXT_RULEPOS (rule_pos);
              if (out[out_len - 1] != rule[rule_pos]) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_EQUAL_AT:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if ((upos + 1) > out_len) return (RULE_RC_REJECT_ERROR);
              NEXT_RULEPOS (rule_pos);
              if (out[upos] != rule[rule_pos]) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_CONTAINS:
              NEXT_RULEPOS (rule_pos);
              NEXT_RPTOI (rule, rule_pos, upos);
              if (upos > out_len) return (RULE_RC_REJECT_ERROR); // should be this, buggy hc
              NEXT_RULEPOS (rule_pos);
              int c; int cnt;
              for (c = 0, cnt = 0; c < out_len && cnt < upos; c++)
              {
                if (out[c] == rule[rule_pos])
                {
                  cnt++;
                  pos_mem = c;
                }
              }
              if (cnt < upos) return (RULE_RC_REJECT_ERROR);
              break;

            case RULE_OP_REJECT_MEMORY:
              if ((out_len == mem_len) && (memcmp (out, mem, out_len) == 0)) return (RULE_RC_REJECT_ERROR);
              break;

            default:
              errno = RULE_RC_SYNTAX_ERROR;
              break;
              }
    }

    if (errno != 0) { return(errno); }

    // Add the null terminator and null extra bytes (just in case)
    memset(out + out_len, 0, RP_PASSWORD_SIZE - out_len);
    return(out_len);
}