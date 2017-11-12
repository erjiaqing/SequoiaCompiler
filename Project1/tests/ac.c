float sqr_f(float x)
{
	return x * x;
}

float abs_f(float x)
{
	if (x < 0)
		return -x;
	return x;
}

int main()
{
    int x[5][5];
//	float x = -123.45;
    float xx = 123.45;
	float r = sqr_f(x);
/*	float s = abs_f(x); */
	return 0;
}
