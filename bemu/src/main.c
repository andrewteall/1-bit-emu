#include <stdio.h>

#include "bemu.h"

int main(int argc, char* argv[]){
    int error = 0;
    if (argc == 1){
        printUsage();
        printf("ERROR: Filename to open must be specified. Exiting now\n");
        return 1;
    } else {
        /*********************************************************************/
        /************************* Init Program ******************************/
        struct OPTIONS sOptions;
        error = parseCommandLineOptions(&sOptions,argc,argv);
        /*********************************************************************/
        
        error = run(&sOptions);

        if(error){
            printf("ERROR: Exited due to error...\n");
        }
        return error;
    }
}