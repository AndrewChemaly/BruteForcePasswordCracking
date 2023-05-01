
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/md5.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#define max_passwords 100500

void md5_hash_string(unsigned char* output, char* input, int input_len)

{

    MD5_CTX md5_ctx;

    MD5_Init(&md5_ctx);

    MD5_Update(&md5_ctx, input, input_len);

    MD5_Final(output, &md5_ctx);

}

/*
__device__ void md5_hash_string(unsigned char* output, char* input, int input_len, int (*MD5_Init)(MD5_CTX* c), int (*MD5_Update)(MD5_CTX* c, const void* data, size_t len), int (*MD5_Final)(unsigned char* md, MD5_CTX* c))
{
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    MD5_Update(&md5_ctx, input, input_len);
    MD5_Final(output, &md5_ctx);
}

*/

__device__ int strcmp_device(const char* str1, const char* str2)
{
    for (; *str1 == *str2; str1++, str2++)
        if (*str1 == '\0') 
            return 0;
    return (*(unsigned char*)str1 < *(unsigned char*)str2) ? -1 : 1;
}


__device__ size_t strcspn_device(const char* str1, const char* str2)
{
    size_t count = 0;
    while (*str1)
    {
        const char* s = str2;
        while (*s)
        {
            if (*str1 == *s)
                return count;
            s++;
        }
        str1++;
        count++;
    }
    return count;
}

__device__ int found;
__device__ char* hashed_password = "845fe25803ec3078dc795ec7187e3fd2";
//sagar at 99999
//sagara at 19275

__global__ void crackPassword(char** passwords, int (*MD5_Final)(unsigned char* md, MD5_CTX* c), int (*MD5_Init)(MD5_CTX* c), int (*MD5_Update)(MD5_CTX* c, const void* data, size_t len))
{
    int max_to_check = max_passwords;
    int i = threadIdx.x + blockDim.x * blockIdx.x;
    // Remove trailing newline character

    if (i < max_passwords && found == 0 && i < max_to_check)
    {
        passwords[i][strcspn_device(passwords[i], "\n")] = 0;


        if (strcmp_device(passwords[i], hashed_password) == 0)
        {
            printf("Match found: %s at Index %d\n", passwords[i], i);
            found = i;
            max_to_check = i;
      //      max_passwords = i;

        }
    }
}

int main(int argc, char* argv[])
{
    // double start_time = omp_get_wtime();

    clock_t start = clock();

    FILE* fptr;
    // fptr = fopen("/content/drive/MyDrive/parallel2/rockyou.txt", "r");
    fptr = fopen("D:/Desktop/rockyou.txt", "r");

    if (fptr == NULL)
    {
        printf("%s", "Error reading file\n");
        return 1;
    }


    char** original_passwords = (char**)malloc(max_passwords * sizeof(char*));
    for (int i = 0; i < max_passwords; i++)
    {
        original_passwords[i] = (char*)malloc(100 * sizeof(char));
    }



    char password[100];

    char** passwords = (char**)malloc(max_passwords * sizeof(char*));

    for (int i = 0; i < max_passwords; i++)
    {
        passwords[i] = (char*)malloc(100 * sizeof(char));
    }

    char** device_passwords;

    int size = max_passwords * sizeof(char*);

    cudaMalloc((void**)&device_passwords, size);

    // float *h_temp[max_passwords];
    char** h_temp[max_passwords];

    int i = 0;

    // Store the content of the file
    // Read the content
    while (fgets(password, 100, fptr) && i < max_passwords)
    {
        password[strcspn(password, "\n")] = 0;
        strcpy(original_passwords[i], password);

        // break;
        int str_len = strlen(password);

        unsigned char md5_hash[MD5_DIGEST_LENGTH];

        md5_hash_string(md5_hash, password, str_len);

        // Initialize hash and md5_hash_password arrays
        char hash[MD5_DIGEST_LENGTH * 2 + 1] = { 0 };
        char md5_hash_password[3] = { 0 };

        // Convert hash to string format
        for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
        {
            sprintf(md5_hash_password, "%02x", md5_hash[i]);
            strcat(hash, md5_hash_password);
        }
        strcpy(passwords[i], hash);

        i++;
    }


    for (int i = 0; i < max_passwords; i++)
    {
        cudaMalloc((void**)&h_temp[i], 100 * sizeof(char));
    }

    cudaMemcpy(device_passwords, h_temp, size, cudaMemcpyHostToDevice);

    for (int i = 0; i < max_passwords; i++)

    {

        cudaMemcpy(h_temp[i], passwords[i], 100 * sizeof(char), cudaMemcpyHostToDevice);
    }


    // crackPassword<<(max_passwords-1)/256 + 1, 256>>(device_passwords);
    crackPassword << <(max_passwords - 1) / 256 + 1, 128 >> > (device_passwords, MD5_Final, MD5_Init, MD5_Update);
    cudaError_t error;
    error = cudaGetLastError();
    if (error != cudaSuccess)
    {
        printf("CUDA error: %s\n", cudaGetErrorString(error));
        exit(-1);
    }



    // Close the file
    fclose(fptr);

    int found_index = -1;
    cudaMemcpyFromSymbol(&found_index, found, sizeof(int), 0, cudaMemcpyDeviceToHost);
    if (found_index != -1)
    {
        printf("Original password: %s at Index %d\n", original_passwords[found_index], found_index);
    }

    cudaFree(device_passwords);
    cudaFree(h_temp);
    // free allocated memory
    for (int i = 0; i < max_passwords; i++)
    {
        free(passwords[i]);
    }
    free(passwords);

    clock_t end = clock();
    double time = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Cuda C Brute Force Execution time is: %.3lf sec\n",
        time);

    return 0;
}