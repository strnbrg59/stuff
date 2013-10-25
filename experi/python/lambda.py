def f(x):
    print x

def g(func, x):
    func(x)

if __name__ == '__main__':
    func = lambda x: g(f, x);
    func(31)
