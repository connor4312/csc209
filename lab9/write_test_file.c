#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
    FILE *fp = fopen(argv[1], "wb");
    int r;

    for (int i = 0; i < 100; i++) {
        r = rand() % 100;
        fwrite(&r, sizeof(int), 1, fp);
    }
    fclose(fp);

    return 0;
}
