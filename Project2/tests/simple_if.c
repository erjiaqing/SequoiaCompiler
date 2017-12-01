int main()
{
	x = read();
	if (x < 0)
	{
		x = -x;
	} else {
		x = x * 2;
	}
	write(x);
	y = read();
	if (y < 0)
		y = -y;
	write(y);
}
