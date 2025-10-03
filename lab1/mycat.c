#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

void print_usage() {
    printf("Usage: mycat [-n] [-b] [-E] [file...]\n");
    printf("Options:\n");
    printf("  -n    number all output lines\n");
    printf("  -b    number non-empty output lines\n");
    printf("  -E    display $ at end of each line\n");
}

void process_file(FILE *file, int show_line_numbers, int number_non_empty, int show_end) {
    char line[1024];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), file)) {
    
        size_t len = strlen(line);
        if (len > 0 && line[len-1] == '\n') {
            line[len-1] = '\0';
            len--;
        }
        
        
        if (number_non_empty) {
            if (len > 0) {
                printf("%6d\t", line_number++);
            } else if (show_line_numbers) {
                printf("%6d\t", line_number++);
            }
        }
        
        else if (show_line_numbers) {
            printf("%6d\t", line_number++);
        }
        
        
        printf("%s", line);
        
       
        if (show_end) {
            printf("$");
        }
        
        printf("\n");
    }
}

int main(int argc, char *argv[]) {
    int show_line_numbers = 0;
    int number_non_empty = 0;
    int show_end = 0;
    int opt;
    
    // Parse command line options using getopt
    while ((opt = getopt(argc, argv, "nbE")) != -1) {
        switch (opt) {
            case 'n':
                show_line_numbers = 1;
                break;
            case 'b':
                number_non_empty = 1;
                break;
            case 'E':
                show_end = 1;
                break;
            default:
                print_usage();
                return 1;
        }
    }
    
    
    if (optind >= argc) {
        process_file(stdin, show_line_numbers, number_non_empty, show_end);
    } else {
        // Process each file
        for (int i = optind; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                fprintf(stderr, "mycat: cannot open file '%s'\n", argv[i]);
                continue;
            }
            
            process_file(file, show_line_numbers, number_non_empty, show_end);
            fclose(file);
        }
    }
    
    return 0;
}
