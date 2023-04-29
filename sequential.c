#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>


void md5_hash_string(unsigned char *output, char *input, int input_len)
{
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    MD5_Update(&md5_ctx, input, input_len);
    MD5_Final(output, &md5_ctx);
}

int main(int argc, char *argv[])
{
    clock_t start = clock();

    char *hashed_password = "3a6622c4a9ded6b5884064f98bcd1fec"; // the one we want to crack
    FILE *fptr;
    fptr = fopen("/home/vboxuser/Desktop/parallel/rockyou.txt", "r");

    if (fptr == NULL)
    {
        printf("Error reading file\n");
        return 1;
    }

    // Dynamically allocate memory for password buffer
    char *password = malloc(100 * sizeof(char));
    int found = 0;

    // Read the content of the file line by line
    while (fgets(password, 100, fptr))
    {
        // Remove trailing newline character
        password[strcspn(password, "\n")] = 0;

        int str_len = strlen(password);
        unsigned char md5_hash[MD5_DIGEST_LENGTH];
        md5_hash_string(md5_hash, password, str_len);

        // Initialize hash and md5_hash_password arrays
        char hash[MD5_DIGEST_LENGTH * 2 + 1] = {0};
        char md5_hash_password[3] = {0};

        // Convert hash to string format
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(md5_hash_password, "%02x", md5_hash[i]);
            strcat(hash, md5_hash_password);
        }

        if (strcmp(hash, hashed_password) == 0)
        {
            printf("Match found: %s\n", password);
            found = 1;
            break;
        }
    }

    if (found == 0)
    {
        printf("No match found\n");
    }

    // Free dynamically allocated memory
    free(password);

    // Close the file
    fclose(fptr);

    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;


    printf("Sequential Brute Force Execution time is: %.3lf sec\n",
        time);

    return 0;
}



