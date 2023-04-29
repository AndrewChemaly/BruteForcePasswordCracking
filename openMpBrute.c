#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
// #include <time.h>

#define num_threads 5

void md5_hash_string(unsigned char *output, char *input, int input_len)
{
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    MD5_Update(&md5_ctx, input, input_len);
    MD5_Final(output, &md5_ctx);
}

int main(int argc, char *argv[])
{
    double start_time = omp_get_wtime();

    // clock_t start = clock();
    omp_set_num_threads(num_threads);

    char *hashed_password = "3a6622c4a9ded6b5884064f98bcd1fec"; // the one we want to crack
    FILE *fptr;
        fptr = fopen("/home/vboxuser/Desktop/parallel/rockyou.txt", "r");

    if (fptr == NULL)
    {
        printf("%s", "Error reading file\n");
        return 1;
    }

    char password[100];

    int max_passwords = 14000000;
    char **passwords = (char **)malloc(max_passwords * sizeof(char *));
    for (int i = 0; i < max_passwords; i++)
    {
        passwords[i] = (char *)malloc(100 * sizeof(char));
    }

    int i = 0;

    // Store the content of the file
    // Read the content
    while (fgets(password, 100, fptr) && i < max_passwords)
    {
        // break;
        strcpy(passwords[i], password);
        i++;
    }


#pragma omp parallel for schedule(static, 10)
    for (i = 0; i < max_passwords; i++)
    {

        // printf("%s", passwords[i]);
        // Remove trailing newline character
        passwords[i][strcspn(passwords[i], "\n")] = 0;

        int str_len = strlen(passwords[i]);
        unsigned char md5_hash[MD5_DIGEST_LENGTH];
        md5_hash_string(md5_hash, passwords[i], str_len);

        // Initialize hash and md5_hash_password arrays
        char hash[MD5_DIGEST_LENGTH * 2 + 1] = {0};
        char md5_hash_password[3] = {0};

        // Convert hash to string format
        for (int j = 0; j < MD5_DIGEST_LENGTH; j++)
        {
            sprintf(md5_hash_password, "%02x", md5_hash[j]);
            strcat(hash, md5_hash_password);
        }

        if (strcmp(hash, hashed_password) == 0)
        {
            printf("Match found: %s\n", passwords[i]);
            max_passwords=i;
        }
    }

    // Close the file
    fclose(fptr);

    // clock_t end = clock();
    // double time = (double)(end - start) / CLOCKS_PER_SEC;

    // printf("OpenMP Brute Force Execution time is: %.3lf sec\n",
    //        time);

    double end_time = omp_get_wtime();

    printf("OPENMP Elapsed time = % 5.3f seconds\nUsing %d Threads\n", end_time - start_time, num_threads);

    return 0;
}