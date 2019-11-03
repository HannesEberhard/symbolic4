
#include "src/symbolic4.h"

int main(int argc, const char * argv[]) {
    
    char query[150];
    char buffer[500];

    memset(buffer, 0, 500);
    memset(query, 0, 150);
    
#ifdef DEBUG_MODE
    if (symbolic4(buffer, "Deriv(x)") == RETS_SUCCESS) {
        printf("%s\n\n", buffer);
        return 0;
    } else {
        printf("ERROR");
        return 1;
    }
#endif
    
    if (argc == 1) {
        while (true) {
            memset(buffer, 0, 500);
            memset(query, 0, 150);
            printf("Query:\n");
            scanf("%s", query);
            if (symbolic4(buffer, query) == RETS_SUCCESS) {
                printf("%s\n\n", buffer);
            } else {
                printf("ERROR\n\n");
            }
        }
    } else if (argc == 2) {
        if (symbolic4(buffer, (char*) argv[1]) == RETS_SUCCESS) {
            printf("%s", buffer);
            return 0;
        } else {
            printf("ERROR");
            return 1;
        }
    }
    
    return 0;
    
}
