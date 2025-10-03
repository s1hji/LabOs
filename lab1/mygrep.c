#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void print_usage() {
    printf("Usage: mygrep pattern [file]\n");
    printf("If no file is specified, reads from stdin\n");
}

void search_pattern(FILE *file, const char *pattern) {
    char line[1024];
    int line_number = 1;
    
    while (fgets(line, sizeof(line), file)) {
        
        if (strstr(line, pattern) != NULL) {
            printf("%s", line);
        }
        line_number++;
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    const char *pattern = argv[1];
    
    
    if (argc == 2) {
        search_pattern(stdin, pattern);
    } else {
      
        for (int i = 2; i < argc; i++) {
            FILE *file = fopen(argv[i], "r");
            if (file == NULL) {
                fprintf(stderr, "mygrep: cannot open file '%s'\n", argv[i]);
                continue;
            }
            
            search_pattern(file, pattern);
            fclose(file);
        }
    }
    
    return 0;
}