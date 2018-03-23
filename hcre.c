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
    printf("USAGE:  %s  wordlist -r rulelist --stdout [--password policies]\n", hcre);
    printf("\n");
    printf("Input words are read from wordlist\n");
    printf("Input rules are read from rulelist\n");
    printf("\n");
    printf("\n");
    printf("EXAMPLE:\n");
    printf("    %s  words.txt -r best64.rule --stdout --length=6\n", hcre);
    printf("\n");
}


int main(int argc, char **argv) {
    // argv[0] is name
    if (argc < 5) { usage(argv[0]); return 0; }


    /* ARGS Check */





    // Allows rule upper/lower/toggle to follow locale
    setlocale(LC_CTYPE, "");


    // Our hash structure's head node
    RuleHash *rules = NULL;

    // Reading/writing to rules hash
    RuleHash *cur_hash = NULL;

    // getline() will read into this
    char   *line     = NULL;
    ssize_t line_len = 0;
    size_t  line_malloc_size = 0;

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
        line_len = getline(&line, &line_malloc_size, rule_file);
        
        #ifdef DEBUG
        printf("%s\n", line);
        printf("%s\n", "read rule");
        #endif
        
        // Stop processing file on error or end of input (we don't care which)
        if (line_len <= 0) { break; }

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
    char rule_output[BLOCK_SIZE];


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
    while ( !feof(word_file) ) {
        line_len = getline(&line, &line_malloc_size, word_file);
        
        #ifdef DEBUG
        printf("%s", line);
        printf("%s\n", "read line");
        #endif

        // Error or end of input
        if (line_len <= 0) { break; }

        // Trim trailing newline
        while (line[line_len - 1] == '\n' || line[line_len - 1] == '\r') {
            line[--line_len] = 0;
        }

        /** Filter Input Word, If too long, skip**/
        if (line_len >= BLOCK_SIZE){
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
                if (rule_rtn == REJECTED) {
                    // Rejections are expected, they're okay
                    fprintf(stderr,
                        "Input word <%s> Rejected <%s> Wrong!\n",
                        line,cur_hash->rule);
                } 
                else {
                    // We missed something in parsing and now our rule broke
                    // We can't "fix" the rule, so our only option is to remove it
                    // If you ever see this message, please contact the developer
                    fprintf(stderr,
                        "Input word <%s> Broke Rule <%s>\n",
                        line,cur_hash->rule);
                    free_hash(cur_hash);
                }

                // Regardless if this was a rejection or error, we're not printing this word
                continue;

            }
            else{
                #ifdef DEBUG
                printf("%s\n", "Inside");
                #endif
                //resultant word
                rule_output[rule_rtn++] = '\t';
                fwrite(rule_output, rule_rtn, 1, stdout);

                //original word
                fwrite(line, line_len, 1, stdout);
                fputc('\n', stdout);
            }

            //Next hash
            cur_hash = cur_hash -> next;

        }

        
    }

    return 0;
}