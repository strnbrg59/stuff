def createCounter():
    i = 0
    while True:
        yield i
        i += 1

c = createCounter()
print dir(c)
n = createCounter().next
print n()
print n()
print n()
