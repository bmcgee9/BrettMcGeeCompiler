extern int read();
extern void print(int);

int func(int i){
	int a;
	int b;
	int c;

	if (i > 100){
		a = 10;
		b = 20;
	}
	else{
		a = 100;
		b = 200;
	}
	
	c = i + a;

	return (c);
}

