int count_digits(char* s) {
    int count = 0;
    while (*s) {
        if (*s >= '0' && *s <= '9')
            count++;
        s++;
    }
    return count;
}
