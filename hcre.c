#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <locale.h>
#include <string.h>
#include "rules.h"
//#define DEBUG

typedef struct RuleHash {
    // The Rule struct itself
    char *rule;
    int rule_len;

    struct RuleHash *next;

} RuleHash;


void free_hash(RuleHash *hash) {
    if (hash == NULL)      { return; }
    if (hash->rule       ) { free(hash->rule);   }
    memset(hash, 0, sizeof(RuleHash));
}


void usage(char *hcre) {
    printf("\n");
    printf("USAGE:  %s  wordlist -r rulelist --stdout [--no_filter_input] [--count_only] [--password policies]\n", hcre);
    printf("\n");
    printf("Input words are read from wordlist\n");
    printf("Input rules are read from rulelist\n");
    printf("\n");
    printf("\n");
    printf("EXAMPLE:\n");
    printf("    %s  words.txt -r best64.rule --stdout --no_filter_input --count_only --length=6\n", hcre);
    printf("\n");
}

int check_length = -1;
int check_digit = 0;
int check_letter = 0;
int check_lower = 0;
int check_upper = 0;

// flags
int filter_input = 1; // filter input_stream, onlt return ASCII printables
int count_only = 0; // only return a number, count guesses made by this specific configuration.

static int check_password_policy(char *pw_buf, const int pw_len)
{
  /*
  printf("%d\n", check_length);
  printf("%d\n", check_digit);
  printf("%d\n", check_lower);
  printf("%d\n", check_letter);
  printf("%d\n", check_upper);
  printf("%d\n", check_symbol);
  printf("The End\n");
  */
  if (check_length != -1 && check_length >= 0)
  {
    //if pw_len < check_length, reject
    if (pw_len < check_length)
    {
      return 0;
    }
  }



  int digit_flag = check_digit != 0? 1:0;
  int lower_flag = check_lower != 0? 1:0;
  int letter_flag = check_letter != 0? 1:0;
  int upper_flag = check_upper != 0? 1:0;
  int flag_count = digit_flag + lower_flag + letter_flag + upper_flag;
  //symbol_flag = check_symbol != 0? 1:0;

  for (int i = 0; i < pw_len; i ++)
  {
    /* PRINTABLE ASCII CHECK */
    if (filter_input == 1){
        if ((pw_buf[i]) < 32 || (pw_buf[i]) > 126)
        {
            return 0;
        }
    }
    else{
        /* Cannot short-circuit if filtering non-ascii*/
        if (flag_count == 0)
        {
            return 1;
        }
    }
    
    /* Rejection Policy Check */
    // If contains digit, 
    if ( ( digit_flag == 1 ) && (pw_buf[i]) >= '0' && (pw_buf[i]) <= '9')
    {
        digit_flag = 0; // satisfy digit flag
        flag_count -= 1;
    }
    if ( (lower_flag == 1) && (pw_buf[i]) >= 'a' && (pw_buf[i]) <= 'z')
    {
        lower_flag = 0; // satisfy lower_flag
        flag_count -= 1;
    }
    if ( (upper_flag == 1) && (pw_buf[i]) >= 'A' && (pw_buf[i]) <= 'Z')
    {
        upper_flag = 0; // satisfy upper_flag
        flag_count -=1;
    }
    if ( (letter_flag == 1) && (((pw_buf[i]) >= 'a' && (pw_buf[i]) <= 'z') || ((pw_buf[i]) >= 'A' && (pw_buf[i]) <= 'Z')) )
    {
        letter_flag = 0; // satisfy letter_flag
        flag_count -= 1;
    }
    /*
    if ( *(pw_buf[i] >= 'A' && *(pw_buf[i]) <= 'Z')
    {
      if (symbol_flag == 1)
      {
        printf("symbol_flag Not Implemented");
      }
    }
    */
  }
  if (flag_count > 0)
  {
    return 0;
  }

  return 1;
}

//int check_symbol = 0;
bool prefix(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

int main(int argc, char **argv) {
    int new_argc = argc;
  for (int i = 0; i < argc; i++)
  {
    //printf("%s\n",argv[i]);
    if (prefix("--length=",argv[i]))
    {
      new_argc --;
      check_length = atoi(argv[i]+9);
    }
    else if (prefix("--digit",argv[i]))
    {
      new_argc --;
      check_digit = 1;
    }
    else if (prefix("--upper",argv[i]))
    {
      new_argc --;
      check_upper = 1;
    }
    else if (prefix("--lower",argv[i]))
    {
      new_argc --;
      check_lower = 1;
    }
    //else if (prefix("--symbol",argv[i]))
    //{
      //new_argc --;
      //check_symbol = 1;
    //}
    else if (prefix("--letter",argv[i]))
    {
      new_argc --;
      check_letter = 1;
    }
    else if (prefix("--no_filter_input",argv[i]))
    {
      new_argc --;
      filter_input = 0;
    }
    else if (prefix("--count_only",argv[i]))
    {
      new_argc --;
      count_only = 1;
    }
  }

    argc = new_argc;
    // argv[0] is name
    if (argc < 5) { usage(argv[0]); return 0; }


    /* ARGS Check */
    if (prefix("-r",argv[2]) == 0)
    {
        usage(argv[0]); return 0;
    }
    if (prefix("--stdout",argv[4]) == 0)
    {
        usage(argv[0]); return 0;
    }




    // Allows rule upper/lower/toggle to follow locale
    setlocale(LC_CTYPE, "");


    // Our hash structure's head node
    RuleHash *rules = NULL;

    // Reading/writing to rules hash
    RuleHash *cur_hash = NULL;

    // fgets() will read into this
    char    line[RP_PASSWORD_SIZE] = { 0 };

    #ifdef DEBUG
    printf("initialize\n");
    #endif
    // Rule File
    char *file_name = argv[3];
    FILE *rule_file = fopen(file_name, "rb");

    // Make sure we successfully opened the rule file
    if (rule_file == NULL) {
        fprintf(stderr, "ERROR: Failed to open input file <%s>\n", file_name);
        return -1;
    }

    // Read each rule from the input file

    while ( !feof(rule_file) ) {
        f_res = fgets(line, RP_PASSWORD_SIZE, rule_file);
        int line_len = strlen(line);
        
        #ifdef DEBUG
        printf("%s\n", line);
        printf("%s\n", "read rule");
        #endif
        
        // Stop processing file on error or end of input (we don't care which)
        if (f_res == NULL) { break; }

        // Trim trailing newline by overwriting it will a NULL terminator
        while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r') {
            line[--line_len] = 0;
        }

        #ifdef DEBUG
        printf("%s\n", "Finished Triming");
        #endif

        // Skip empty lines and commented lines
        if (line_len ==  0 ) { continue; }
        if (line[0]  == '#') { continue; }

        // Make a new hash entry from the current rule
        cur_hash = (RuleHash *)calloc(1, sizeof(RuleHash));
        
        //Copy data
        cur_hash->rule = (char *)calloc(1, (line_len + 1) * sizeof(char));
        if (cur_hash->rule == NULL) {
            fprintf(stderr, "clone_rule() failed to calloc() a string of length.\n");
            return -1;
        }
        memcpy(cur_hash->rule, line, line_len);
        cur_hash->rule_len    = line_len;
        cur_hash->next        = NULL;

        #ifdef DEBUG
        printf("%s\n", "Finished Parsing");
        #endif

        // If clone_rule() failed, we have a major problem
        if (!cur_hash->rule) {
            fprintf(stderr, "Failed to clone rule, aborting\n");
            return -1;
        }

        //Link to head
        RuleHash *tmp = rules;
        if (tmp == NULL)
        {
            rules = cur_hash;
        }
        else
        {
            while (tmp -> next != NULL){
            tmp = tmp -> next;
            }
            tmp -> next = cur_hash;
        }

        #ifdef DEBUG
        printf("%s\n", "Linked To HEAD");
        #endif
    }

    #ifdef DEBUG
    printf("%s\n", "End of Reading Rules");
    #endif

    fclose(rule_file);



    // Our mangled text ends up here
    char rule_output[RP_PASSWORD_SIZE];


    // Increase the output buffering
    if (setvbuf(stdout, NULL, _IOFBF, 1024 * 1024 * 4) != 0) {
        fprintf(stderr, "Failed to adjust stdout buffer size\n");
    }

    char *input_file_name = argv[1];
    FILE *word_file = fopen(input_file_name, "rb");

    // Make sure we successfully opened the rule file
    if (word_file == NULL) {
        fprintf(stderr, "ERROR: Failed to open input file <%s>\n", input_file_name);
        return -1;
    }

    #ifdef DEBUG
    printf("%s\n", "Open File");
    #endif

    // Main processing loop, runs for each line of input
    int guess_count = 0; //count guesses made by this configuration
    while ( !feof(word_file) ) {
        f_res = fgets(line, RP_PASSWORD_SIZE, word_file);
        int line_len = strlen(line);
        
        #ifdef DEBUG
        printf("%s", line);
        printf("%s\n", "read line");
        #endif

        // Error or end of input
        if (f_res == NULL) { break; }

        // Trim trailing newline
        while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r') {
            line[--line_len] = 0;
        }

        /** Filter Input Word, If too long, skip**/
        if (line_len >= RP_PASSWORD_SIZE){
            continue;
        } 

        // Safely iterate the rules hash
        // For iterating rules hash with HASH_ITER
        cur_hash = rules;
        
        #ifdef DEBUG
        printf("%s\n", "Start Apply Rules");
        #endif

        while (cur_hash != NULL)
        {
            // Apply the rule operations
            int rule_rtn = apply_rule(cur_hash->rule, cur_hash->rule_len, line, line_len, rule_output);
            #ifdef DEBUG
            printf("%s\n", "Apply Rules");
            #endif

            // Something broke?
            if (rule_rtn < 0) {
                if (rule_rtn == RULE_RC_REJECT_ERROR) {
                    // Rejections are expected, they're okay
                    /*
                    fprintf(stderr,
                        "Input word <%s> Rejected <%s> Wrong!\n",
                        line,cur_hash->rule);
                        */
                    break;
                } 
                else {
                    //printf("error: %d\n", rule_rtn);
                    // We missed something in parsing and now our rule broke
                    // We can't "fix" the rule, so our only option is to remove it
                    // If you ever see this message, please contact the developer
                    fprintf(stderr,
                        "Input word <%s> Broke Rule <%s>\n",
                        line,cur_hash->rule);
                    
                    //free_hash(cur_hash);
                }

                // Regardless if this was a rejection or error, we're not printing this word
                break;

            }
            else{


                #ifdef DEBUG
                printf("%s\n", "Inside");
                #endif

                if (check_password_policy(rule_output, rule_rtn) == 1)
                {
                    if (count_only == 0) // output actual pwd
                    {
                        //resultant word
                        rule_output[rule_rtn++] = '\t';
                        fwrite(rule_output, rule_rtn, 1, stdout);

                        //original word
                        fwrite(line, line_len, 1, stdout);
                        fputc('\n', stdout);
                    }
                    else //output a number only
                    {
                        guess_count += 1;
                    }
                    
                }

            }

            //Next hash
            cur_hash = cur_hash -> next;

        }

    }
    // at the end of program, if requires only a number, output this number.
    if (count_only == 1){
            fprintf(stdout, "%d\n", guess_count);
    }

    return 0;
}