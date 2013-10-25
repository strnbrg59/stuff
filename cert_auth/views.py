from django.http import HttpResponse
from django.shortcuts import render
from django.core.context_processors import csrf
from django.shortcuts import render_to_response
from django.views.decorators.csrf import csrf_exempt
import datetime
import os
import cert_auth_utils


""" Return the CMC's public key.
    No input required.
"""
def showOwnCertificate(request):
    infilename = cert_auth_utils.ownCertFile("public")
    if not os.path.isfile(infilename):
        return HttpResponse("The CMC has no certificate yet.\n")
    else:
        infile = open(infilename, "rb")
        cert = infile.read(os.path.getsize(infilename))
        return HttpResponse(cert)

""" This is the alternative to acceptKeyPair().
"""
def generateOwnKeyPair(request):
    for side in ("private", "public"):
        outfile = open(cert_auth_utils.ownCertFile(side), "w")
        outfile.write("This is the self-generated " + side + " key, haha.\n")

    return HttpResponse("ok\n")

""" Takes POST input with keys "public" and "private", and saves them to two
    files.
    Test with ../client/accept_key_pair.sh.
"""
@csrf_exempt
def acceptKeyPair(request):
    sides = ("public", "private")
    for k in sides:
        if not k in request.POST:
            result = "Error: acceptHigherAuthority(): missing key: " +\
                    k + "<br>\n"
            return HttpResponse(result)

    # Sanity checks passed, do real work now.
    for side in sides:
        outfile = open(cert_auth_utils.ownCertFile(side), "w")
        outfile.write(request.POST.get(side))
    return HttpResponse("ok\n")


""" Returns signed certificate.
    Saves (unsigned) certificate.

    Takes POST input with keys "name" and "cert".
"""
@csrf_exempt
def signCertificate(request):
    # Verify that the CMC has a private key installed.  (Even though at this
    # point we're not actually using it.)
    infilename = cert_auth_utils.ownCertFile("private")
    if not os.path.isfile(infilename):
        return HttpResponse("The CMC has no keypair installed.\n")

    result = ''
    required_keys = ("name", "cert")
    for k in required_keys:
        if not k in request.POST:
            for kk in request.POST.keys():
                result += kk + ' : ' + request.POST.get(kk) + '<br>\n'
            result += "Error: signCertificate(): missing key: " +\
                    kk + "<br>\n"
            return HttpResponse(result)

    # Save unsigned certificate.
    outfilename = cert_auth_utils.dataDir() + "/" + request.POST.get("name") +\
         ".cert"
    outfile = open(outfilename, "w");
    outfile.write(request.POST.get("cert"))

    # Sign certificate and return that.
    result = request.POST.get("cert") + "<BR>\nSigned by CMC\n"
    return HttpResponse(result)
