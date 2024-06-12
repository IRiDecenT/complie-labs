int main() {
    char ch = 'a';
    init();
    double x = 10.31;/* some comment */
    int m = 0;
    int y = 310;
    double a = 0.31;
    for(int i = 0; i < 10; i++) {
        m += i;
    }
    while(m < 30) {
        m++;
    }
    if(m == 30) {
        m = 0;
    }
    else {
        m = 1;
    }
    return 0;
}
