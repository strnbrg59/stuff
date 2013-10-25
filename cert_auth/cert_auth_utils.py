import os

def dataDir():
    result = "/tmp/cert_auth"
    if not os.path.isdir(result):
        os.mkdir(result)
    return result

def ownCertFile(side):
    assert(side in ("private", "public"))
    return dataDir() + "/" + side + ".cert"
