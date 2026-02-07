func test_func(name) {
    print(name);
    return 1;
}

let x = 5;

while (x < 10) {
    x = x + test_func("Tom");
}

print(x);
