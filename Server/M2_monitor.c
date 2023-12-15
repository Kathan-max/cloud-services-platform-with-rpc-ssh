#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_COMMAND_LENGTH 256

int main() {
    FILE *file, *file2;
    char lastCommand[MAX_COMMAND_LENGTH];
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        file = fopen("msg.txt", "r");
        if (file == NULL) {
            perror("Error opening file");
            return 1;
        }

        while (fgets(command, MAX_COMMAND_LENGTH, file) != NULL) {
            // Check if the command is different from the last executed command
            if (strcmp(lastCommand, command) != 0) {
                strcpy(lastCommand, command);
                system(command);
            }
        }

        file2 = fopen("kathan_me_data_an2.txt", "r");
        if (file2 == NULL) {
            perror("Error opening file");
            return 1;
        }
        char scp_command[256];
        snprintf(scp_command, sizeof(scp_command), "ssh cloud-lab@10.20.24.87 'cat > /home/cloud-lab/Desktop/Cloud_Computing_proj/kathan_me_data_an2.txt' < kathan_me_data_an2.txt"); // Use snprintf safely
        system(scp_command);

        fclose(file);
        fclose(file2);

        // Sleep for a bit to avoid consuming too much CPU
        sleep(1);
    }

    return 0;
}

