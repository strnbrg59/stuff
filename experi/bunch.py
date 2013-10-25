class Bunch:
    def __init__(self,**kw):
        for i in kw.keys():
            print i
            self.__dict__[i] = kw[i]

if __name__ == '__main__':
    b = Bunch(a=1, b=2, c=3)

    print b.a
    print b.b
    print b.c
