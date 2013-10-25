import Numeric as nu
import LinearAlgebra as la
import RandomArray as ra
import math

def TestOls():
    n = 6
    ones = nu.ones([n,1])
    ndx = nu.reshape( nu.arange(n), [n,1])
    ra.seed(0)
    noise = ra.random(shape=[n,1])/1.0
#   print "ones=\n", ones
#   print "ndx=\n", ndx
#   print "noise=\n", noise

    x = nu.concatenate( (ones,ndx), 1 )
#   print "x=\n", x

    beta = nu.array( [[2.2],[3.3]] )
#   print "beta=\n", beta

    y = nu.dot(x,beta) + noise
#   print "y=\n", y

    ols_results = Ols(y,x)
    print "ols_results['beta']=", nu.transpose(ols_results['beta'])
    print "ols_results['tstats']=", ols_results['tstats']

def SD( v ):
    n = nu.shape(nu.ravel(v))[0]
    sx = nu.sum(v)[0]
    sxx = nu.sum(v*v)[0]
    return math.pow( (1.0/(n-1)) * (sxx - sx*sx/n), 0.5 )

def Ols( y, x ):
    xpx = nu.dot( nu.transpose(x),x )
    xpxinv = la.inverse(xpx)
    xpy = nu.dot( nu.transpose(x),y )
    beta = nu.dot( xpxinv, xpy )

    yhat = nu.dot(x,beta)
    res = y - yhat
    sdres = SD(res)

    vcvbeta = sdres*sdres * xpxinv
    sdbeta = nu.sqrt( nu.diagonal(vcvbeta) )

    tstats = nu.transpose(beta) / sdbeta

    result = {'beta':beta, 'yhat':yhat, 'res':res,
              'vcvbeta':vcvbeta, 'sdres':sdres, 'tstats':tstats }
    return result

if __name__ == "__main__":
    TestOls()
