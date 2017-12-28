int main()
{
	int x, y;
	x = read();
	if (x < 0)
	{
		x = 0 - x;
	} else {
		x = x * 2;
	}
	write(x);
	y = read();
	if (y < 0)
		y = 0 - y;
	write(y);
	return 0;
}
