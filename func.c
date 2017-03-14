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
extern int a1; // 说明这个变量来着外部文件
void f(){
	a1 = 23.29;
}
