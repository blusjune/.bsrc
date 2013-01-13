#include <stdio.h>



int add(int a, int b);
int sub(int a, int b);

struct calc {
	int (*opr)(struct calc *);
	int a;
	int b;
};
int calc_add(struct calc * c);
int calc_sub(struct calc * c);




int add(int a, int b)
{
	printf ("add: %d = %d + %d\n", a+b, a, b);
	return a+b;
}

int sub(int a, int b)
{
	printf ("sub: %d = %d - %d\n", a-b, a, b);
	return a-b;
}

int test1(void)
{
	int (*fp)(int, int);

	fp = &add;
	(*fp)(4, 3);
	fp = &sub;
	(*fp)(2, 1);

	return 0;
}




int calc_add(struct calc * c)
{
	int result = c->a + c->b;
	printf ("calc_add: %d = %d + %d\n", result, c->a, c->b);
	return result;
}

int calc_sub(struct calc * c)
{
	int result = c->a - c->b;
	printf ("calc_sub: %d = %d - %d\n", result, c->a, c->b);
	return result;
}

int test2(void)
{
	struct calc *c;
	struct calc add_this;
	struct calc sub_this = {
		&calc_sub,
		2,
		1
	};

	add_this.opr = &calc_add;
	add_this.a = 4;
	add_this.b = 3;

#if 0
	sub_this.opr = &calc_sub;
	sub_this.a = 2;
	sub_this.b = 1;
#endif

	c=&add_this;
	(*(c->opr))(c);
	c=&sub_this;
	(*(c->opr))(c);

	return 0;
}

int main(void)
{
	test1();
	test2();
	return 0;
}
