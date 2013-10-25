import sys
import random
import linecache
import testme

def traceit(frame, event, arg):
    if event == "line":
        lineno = frame.f_lineno
        filename = frame.f_globals["__file__"]
        if filename == "<stdin>":
            filename = "traceit.py"
        if (filename.endswith(".pyc") or
            filename.endswith(".pyo")):
            filename = filename[:-1]
        name = frame.f_globals["__name__"]
        line = linecache.getline(filename, lineno)
        print "%s:%s: %s" % (name, lineno, line.rstrip())
    return traceit


def main():
    print "In main"
    for i in range(5):
        print i, random.randrange(0, 10)
    print "Done."
    sys.settrace(traceit)
    testme.func()
    sys.settrace(None)

main()
