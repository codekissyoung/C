
int max(int a, int b){
	if(a > b)
		return a;
	else
		return b;
}

int swap(int *a,int *b){
	int temp;
	temp = *a;
	*a = *b;
	*b = temp;
	return *a + *b;
}
