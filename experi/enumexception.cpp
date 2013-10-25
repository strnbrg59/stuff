typedef enum {
    lc_err_not_found = 14004
} ERROR_CODE;

void func1(ERROR_CODE e) {
    throw e;
}

void func2(int e) {
    throw e;
}

int main() {
    try {
//        func1(2);
        func2(lc_err_not_found);
    }
    catch(ERROR_CODE err) {
        return err;
    }
    return 0;
}
