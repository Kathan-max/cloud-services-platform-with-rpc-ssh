#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void splitFile(char *inputFileName, const char *username, int numParts) {
    FILE *inputFile = fopen(inputFileName, "r");
    if (inputFile == NULL) {
        perror("Error opening input file");
        exit(1);
    }
    
    
    char fileNames[10][100];
    int numFiles = 0;
    const char *ipAddresses[] = {"10.20.24.81"};
    
    // Find the total number of lines in the file
    char buffer[1024];
    int totalLines = 0;
    while (fgets(buffer, sizeof(buffer), inputFile)) {
        totalLines++;
    }
    fseek(inputFile, 0L, SEEK_SET);
    
    
    int linesPerPart = totalLines / numParts;
    int remainingLines = totalLines % numParts;

    FILE *outputFile = NULL;
    char outputFileName[100];
    char savedFile[100];
    int partCounter = 1;

    // Extract the extension from inputFileName
    const char *extension = strrchr(inputFileName, '.');
    if (!extension) {
        extension = ""; // Use empty string if no extension is found
    }

    numFiles = 0; // Initialize the number of files to be transferred
    while (numParts--) {
        sprintf(outputFileName, "data_files/%s_data_part%d%s", username, partCounter, extension);
        outputFile = fopen(outputFileName, "w");
        if (outputFile == NULL) {
            perror("Error opening output file");
            exit(1);
        }
        printf("Created file: %s\n", outputFileName);

        int linesToWrite = linesPerPart + (remainingLines > 0 ? 1 : 0);
        while (linesToWrite-- && fgets(buffer, sizeof(buffer), inputFile)) {
            fputs(buffer, outputFile);
        }

        fclose(outputFile);

        if (partCounter != 1) { // Store file names to be transferred
            strcpy(fileNames[numFiles], outputFileName);
            numFiles++;
        }else{
            sprintf(savedFile,"%s",outputFileName); 
        }
        partCounter++;
        if (remainingLines > 0) {
            remainingLines--;
        }
    }
    
   
    
    
    FILE *logFile = fopen("distribution_log.txt", "w");
    if (logFile == NULL) {
        perror("Error opening log file");
        exit(1);
    }
    
    char dd[256];
    snprintf(dd, sizeof(dd), "%s stored in M1 (local machine)\n", savedFile);
    fprintf(logFile, "%s", dd); // Corrected usage
    
    char command[256];
    int k = 2;
    for (int i = 0; i < numFiles; i++) {
        printf("File%d: %s", i, fileNames[i]);
        int requiredSize = snprintf(NULL, 0, "ssh cloud-lab@%s 'cat > /home/cloud-lab/Desktop/Cloud_C_proj/%s' < %s", 
                                ipAddresses[i], fileNames[i], fileNames[i]);

	if (requiredSize >= sizeof(command)) {
	    fprintf(stderr, "Error: Command string too long, buffer size exceeded.\n");
	    continue; // Skip this iteration
	}

    	snprintf(command, sizeof(command), "ssh cloud-lab@%s 'cat > /home/cloud-lab/Desktop/Cloud_C_proj/%s' < %s", ipAddresses[i], fileNames[i], fileNames[i]); // Use snprintf safely


        
        snprintf(dd, sizeof(dd), "%s stored in M%d (%s)\n", fileNames[i], k, ipAddresses[i]);
        system(command);
        fprintf(logFile, "%s", dd); // Corrected usage
        remove(fileNames[i]);
        k++;
    }
    
 
    fclose(inputFile);
    fclose(logFile);
}

