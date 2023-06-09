#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libcoro.h"

void merge(
        int arr[],
        const int left[],
        int leftSize,
        const int right[],
        int rightSize
) {

    int i = 0, j = 0, k = 0;

    while (i < leftSize && j < rightSize)
        if (left[i] <= right[j])
            arr[k++] = left[i++];
        else
            arr[k++] = right[j++];

    while (i < leftSize)
        arr[k++] = left[i++];

    while (j < rightSize)
        arr[k++] = right[j++];
}

void mergeSort(
        int arr[],
        int size
) {

    if (size < 2)
        return;

    int mid = size / 2;
    int left[mid];
    int right[size - mid];

    for (int i = 0; i < mid; i++)
        left[i] = arr[i];

    for (int i = mid; i < size; i++)
        right[i - mid] = arr[i];

    mergeSort(left, mid);
    coro_yield();
    mergeSort(right, size - mid);
    coro_yield();
    merge(arr, left, mid, right, size - mid);
    coro_yield();
}

void mergeFiles(
        const char *outputFile,
        char *inputFiles[],
        int numFiles
) {

    FILE *in_files[numFiles];
    FILE *out_file;
    int i, j, min_val, min_val_file;

    for (i = 0; i < numFiles; i++) {
        in_files[i] = fopen(inputFiles[i], "r");
        if (in_files[i] == NULL) {
            printf("Error opening file %s.\n", inputFiles[i]);
            exit(1);
        }
    }

    out_file = fopen(outputFile, "w");
    if (out_file == NULL) {
        printf("Error creating output file.\n");
        exit(1);
    }

    while (1) {
        min_val = -1;
        min_val_file = -1;
        for (i = 0; i < numFiles; i++) {
            if (!feof(in_files[i])) {
                fscanf(in_files[i], "%d", &j);
                if (min_val == -1 || j < min_val) {
                    min_val = j;
                    min_val_file = i;
                }
            }
        }
        if (min_val_file == -1) {
            break;
        }
        fprintf(out_file, "%d\n", min_val);
    }

    for (i = 0; i < numFiles; i++) {
        fclose(in_files[i]);
    }
    fclose(out_file);

    printf("Files merged successfully into merged.txt.\n");
}

/**
 * Coroutine body. This code is executed by all the coroutines. Here you
 * implement your solution, sort each individual file.
 */
static int
coroutine_func_f(void *context)
{
    FILE *file;
    int num, count = 0;

    file = fopen(context, "r");
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return 1;
    }

    while (fscanf(file, "%d", &num) == 1)
        count++;

    int *arr = (int *)malloc(count * sizeof(int));
    if (arr == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return 1;
    }

    fseek(file, 0, SEEK_SET);

    for (int i = 0; i < count; i++) {
        if (fscanf(file, "%d", &arr[i]) != 1) {
            printf("Failed to read integer from the file.\n");
            fclose(file);
            free(arr);
            return 1;
        }
    }

    fclose(file);

    mergeSort(arr, count);

    file = fopen(context, "w");
    if (file == NULL) {
        printf("Failed to open the file for writing.\n");
        free(arr);
        return 1;
    }

    for (int i = 0; i < count; i++)
        fprintf(file, "%d ", arr[i]);

    fclose(file);
    free(arr);

    return 0;
}

int
main(int argc, char **argv)
{
    if (argc < 2) {
        printf("No files for sorting provided\n");
        return 1;
    }

    coro_sched_init();

    for (int i = 1; i < argc; ++i) {
        printf("%s\n", argv[i]);

        char name[16];
        sprintf(name, "coro_%d", i);

        coro_new(coroutine_func_f, strdup(argv[i]));
    }

    struct coro *c;
    while ((c = coro_sched_wait()) != NULL) {
        printf("Finished %d\n", coro_status(c));
        coro_delete(c);
    }

    char *inputFiles[argc];
    const char *outputFile = "output.txt";

    for (int i = 0; i < argc - 1; i++)
        inputFiles[i] = argv[i + 1];

    mergeFiles(outputFile, inputFiles, argc - 1);

}
