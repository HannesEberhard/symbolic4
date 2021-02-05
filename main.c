
#include <unistd.h>
#include "symbolic4.h"

int debug_mode(const char* query) {
    
    char result[result_size];
    memset(result, 0, result_size);
    
    if (symbolic4(result, query) == RETS_SUCCESS) {
        printf("\nResult:\n%s\n\n", result);
        return 0;
    } else {
        printf("\nError:\n%s\n\n", result);
        return 1;
    }
    
}

void interactive_mode() {
    
    char query[result_size];
    char result[result_size];
    
    while (true) {
        
        memset(query, 0, query_size);
        memset(result, 0, result_size);
        
        printf("Query:\n");
        scanf("%s", query);
        
        if (symbolic4(result, query) == RETS_SUCCESS) {
            printf("%s\n\n", result);
        } else {
            printf("ERROR\n\n");
        }
        
    }
    
}

int arguments_mode(int argc, const char* argv[]) {
    
    uint8_t i;
    char result[result_size];
    int status = 0;
    
    for (i = 1; i < argc; i++) {
        
        memset(result, 0, result_size);
        
        if (symbolic4(result, argv[i]) == RETS_SUCCESS) {
            printf("%s\n", result);
        } else {
            status = 1;
            printf("ERROR\n");
        }
        
    }
    
    return status;
    
}

int stdin_mode() {
    
    char query[query_size];
    char result[result_size];
    int status = 0;

    memset(query, 0, query_size);
    memset(result, 0, result_size);
    
    while (fgets(query, query_size, stdin) != NULL) {
        if (strlen(query) > 1 && query[0] != '#') {
            
            query[strlen(query) - 1] = '\0';
            
            if (symbolic4(result, query) == RETS_SUCCESS) {
                printf("%s\n", result);
            } else {
                status = 1;
                printf("ERROR\n");
            }

            memset(query, 0, query_size);
            memset(result, 0, result_size);
            
        }
    }
    
    return status;
    
}

int main(int argc, const char* argv[]) {
    
    return debug_mode("sin(x+x + 0)+sin(x+x)");
    
#ifdef DEBUG
    return debug_mode("1+2");
#endif
    
    if (!isatty(STDIN_FILENO)) {
        return stdin_mode();
    } else if (argc == 1) {
        interactive_mode();
    } else if (argc > 1) {
        return arguments_mode(argc, argv);
    }
    
    return EXIT_SUCCESS;
    
}
