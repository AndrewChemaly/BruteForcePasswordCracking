// mpicc -o mpitest mpiBrute.c -lssl -lcrypto

// mpirun -np 2 ./mpitest




#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
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
    // fptr = fopen("/usr/share/wordlists/rockyou.txt", "r");
        fptr = fopen("/home/vboxuser/Desktop/parallel/rockyou.txt", "r");


    if (fptr == NULL)
    {
        printf("Error reading file\n");
    }
    else
    {
        int myid, numprocs;
        MPI_Init(&argc, &argv);
        MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
        MPI_Comm_rank(MPI_COMM_WORLD, &myid);

        int index = 0;
        int data_tag = 1;
        int terminator_tag = 2;
        int match_tag = 3;

        if (myid == 0)
        {
            int k = 0;
            for (k = 1; k < numprocs; k++)
            {
                MPI_Send(&index, 1, MPI_INT, k, data_tag, MPI_COMM_WORLD);
                // if(myid==2)
                index += 10;
                if (index > 14000000)
                {
                    break;
                }
            }

            MPI_Status status;

            do
            {
                int received_index = 0;
                MPI_Recv(&received_index, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                if (status.MPI_TAG == match_tag)
                {
                    for (k = 1; k < numprocs; k++)
                    {

                        MPI_Send(&index, 1, MPI_INT, k, terminator_tag, MPI_COMM_WORLD);
                    }
                    break;
                }
                else
                {

                    if (index >= 14000000)
                    {
                        printf("%s\n", "no match found");

                        for (k = 1; k < numprocs; k++)
                        {
                            MPI_Send(&index, 1, MPI_INT, k, terminator_tag, MPI_COMM_WORLD);
                        }
                        break;
                    }
                    else
                    {
                        MPI_Send(&index, 1, MPI_INT, status.MPI_SOURCE, data_tag, MPI_COMM_WORLD);
                        index += 10;
                    }
                }

            } while (index <= 14000000);
        }
else
{
    char password[100];

    int max_passwords = 14000000;
    char **passwords = (char**) malloc(max_passwords * sizeof(char*));
    for (int i = 0; i < max_passwords; i++) {
        passwords[i] = (char*) malloc(100 * sizeof(char));
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

    int slave_index;
    MPI_Status status;

    MPI_Recv(&slave_index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    int found = 0;

    while (status.MPI_TAG == data_tag)
    {

        for (int i = slave_index; i < slave_index + 10 && i < max_passwords; i++)
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
                found = 1;
                MPI_Send(&found, 1, MPI_INT, 0, match_tag, MPI_COMM_WORLD);
                break;
            }
        }

        // if not found
        if (found == 0)
        {
            MPI_Send(&slave_index, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
        }
        MPI_Recv(&slave_index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    }

    // free allocated memory
    for (int i = 0; i < max_passwords; i++) {
        free(passwords[i]);
    }
    free(passwords);
}

        if(myid==0)
        {

        clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;


    printf("MPI Brute Force Execution time is: %.3lf sec\n",
        time);
        }

        // Close the file
        MPI_Finalize();
        fclose(fptr);
    }


    return 0;
}