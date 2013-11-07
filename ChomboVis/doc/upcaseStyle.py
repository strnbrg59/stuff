import sys

def upcaseStyle( var_name ):
    """
    Turn foo_bar into FooBar.
    """
    result = var_name[0].upper()
    pos = 1
    for c in var_name[1:]:
        if var_name[pos] == '_' : pass
        elif var_name[pos-1] == '_' :
            result = result + c.upper()
        else:
            result = result + c
        pos = pos + 1

    return result

if __name__ == '__main__':
    print upcaseStyle( sys.argv[1] )
