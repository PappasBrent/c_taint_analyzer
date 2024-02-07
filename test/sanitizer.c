void sanitize(char *s) __attribute__((__annotate__("sanitizes(s)"))) {
        (void)s;
}

int main(void) {
        char *s = (char *)0;
        sanitize(s);
        return 0;
}