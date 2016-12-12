#include <stdio.h>

int main(int argc, char const *argv[])
{
    FILE *f = fopen("nameFile", "w");
    fprintf(f, "Connor Peet\n");
    fclose(f);


    f = fopen("nameFile", "r");
    int data;
    int sum = 0;
    while (fread(&data, 4, 1, f) == 1) {
        printf("%d\n", data);
        sum += data;
    }
    printf("%d\n", sum);

    return 0;
}
