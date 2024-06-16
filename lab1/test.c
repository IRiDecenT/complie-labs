int main() {
    init();
    double x = 10.31;/* some comment */
    int m = 0;
    int y = 310;
    double a = 0.31;
    for(int i = 0; i < 10; i++) {
        if(i % 2 == 0) {
            y = m + x;
        } else {
            y = m * x + 1;
        }
    }
    return 0;
}
