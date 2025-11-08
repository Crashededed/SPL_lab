#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int addr5;
int addr6;

int foo()
{
    return -1;
}
void point_at(void *p);
void foo1();
char g = 'g';
void foo2();

int secondary(int x)
{
    int addr2;
    int addr3;
    char *yos = "ree";
    int *addr4 = (int *)(malloc(50));

    printf("- &addr2: %p\n", &addr2);
    printf("- &addr3: %p\n", &addr3);
    printf("- foo: %p\n", &foo);
    printf("- &addr5: %p\n", &addr5);
    printf("Print distances:\n");
    point_at(&addr5);

    printf("Print more addresses:\n");
    printf("- &addr6: %p\n", &addr6);
    printf("- yos: %p\n", yos);
    printf("- gg: %p\n", &g);
    printf("- addr4: %p\n", addr4);
    printf("- &addr4: %p\n", &addr4);

    printf("- &foo1: %p\n", &foo1);
    printf("- &foo1: %p\n", &foo2);

    printf("Print another distance:\n");
    printf("- &foo2 - &foo1: %ld\n", (long)(&foo2 - &foo1));

    /* task 1 b here */
    printf("Arrays Mem Layout (T1b):\n");

    int iarray[3];
    float farray[3];
    double darray[3];
    char carray[3];

    printf("- &iarray[0] = %p, &iarray[1] = %p \n", &iarray, &iarray + 1);
    printf("- &farray[0] = %p, &farray[1] = %p \n", &farray, &farray + 1);
    printf("- &darray[0] = %p, &darray[1] = %p \n", &darray, &darray + 1);
    printf("- &carray[0] = %p, &carray[1] = %p \n", &carray, &carray + 1);

    printf("Pointers and arrays (T1d): \n");

    /* task 1 d here */
    int iarray2[] = {1, 2, 3};
    char carray2[] = {'a', 'b', 'c'};
    int *iarray2Ptr = iarray2;
    char *carray2Ptr = carray2;

    for (int i = 0; i < 3; i++)
    {
        printf("- iarray2[i] = %d\n", iarray2Ptr[i]);
    }

    for (int i = 0; i < 3; i++)
    {
        printf("- carray2Ptr[i] = %c\n", carray2Ptr[i]);
    }

    int *p;
    printf("- uninitialized pointer p: %p\n", p);
}

int main(int argc, char **argv)
{

    printf("Print function argument addresses:\n");

    printf("- &argc %p\n", &argc);
    printf("- argv %p\n", argv);
    printf("- &argv %p\n", &argv);

    secondary(0);

    printf("Command line arg addresses (T1e):\n");
    /* task 1 e here */

    for (int i = 1; i < argc; i++)
    {
        printf("- argv[%d] = %s, &argv[%d] = %p\n", i, argv[i], i, (void *)&argv[i]);
    }

    return 0;
}

void point_at(void *p)
{
    int local;
    static int addr0 = 2;
    static int addr1;

    long dist1 = (size_t)&addr6 - (size_t)p;
    long dist2 = (size_t)&local - (size_t)p;
    long dist3 = (size_t)&foo - (size_t)p;

    printf("- dist1: (size_t)&addr6 - (size_t)p: %ld\n", dist1);
    printf("- dist2: (size_t)&local - (size_t)p: %ld\n", dist2);
    printf("- dist3: (size_t)&foo - (size_t)p:  %ld\n", dist3);

    /* task 1 a here */
    printf("Check long type mem size (T1a):\n");
    printf("- sizeof(long): %zu\n", sizeof(long));
    printf("- sizeof(void*): %zu\n", sizeof(p));

    printf("- addr0: %p\n", &addr0);
    printf("- addr1: %p\n", &addr1);
}

void foo1()
{
    printf("foo1\n");
}

void foo2()
{
    printf("foo2\n");
}