#include <stdio.h>
#include <assert.h>
#include <vx_print.h>

short d[100];

int main()
{
    // Should produce an out of bounds error before buffer
	d[0] = 0;
	for (int i = 0; i < 100; i++)
		d[i] = d[i-1] + 2;
	for (int i = 1; i < 100; i++)
		assert(d[i] == d[i-1] + 2);

	int a[10];
	char b[10];

    // Should produce an out of bounds error after buffer
	for (int i = 0; i <= 10; i++)
		a[i] = i;
	for (int i = 0; i < 10; i++)
		assert(a[i] == i);

    // Should produce an out of bonds error after buffer
	char *c = "hello world!";
	for (int i = 0; i < 13; i++)
		b[i] = c[i];
	for (int i = 0; i < 10; i++)
		assert(b[i] == c[i]);

    // Should produce an access error within buffer
    c[0] = "?";
    c[1] = "?";    

	vx_printf("Success!\n");
	vx_printf("Buffer base VAs: a=%p b=%p c=%p d=%p\n", a, b, c, d);
	vx_printf("Buffer sizes: a=%x b=%x c=%x d=%x\n", 
				10*sizeof(a[0]), 
				10*sizeof(b[0]), 
				13*sizeof(c[0]), 
				100*sizeof(d[0]));

	return 0;
}

// RBT layout using program output
// The base addresses might not be deterministic, run make to double check them
// Buffer ID (14 bits) | Base Address (48 bits) | Size (32 bits) | Read-only (1 bit) | Kernel ID (12 bit)
// 0x1 | 0xFEFFFFC8 | 0x28 | 0x0 | 0x0
// 0x2 | 0xFEFFFFBC | 0xA | 0x0 | 0x0
// 0x3 | 0x8000A814 | 0xD | 0x1 | 0x0
// 0x4 | 0x8000BA5C | 0xC8 | 0x0 | 0x0