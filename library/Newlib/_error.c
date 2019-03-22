static int err = 0;
int *__errno(void) {
    return &err;
}
