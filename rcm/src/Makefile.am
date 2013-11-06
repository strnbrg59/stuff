lib_LTLIBRARIES = librcm.la
librcm_la_SOURCES = amoeba.cpp stepwiseRegression.cpp logic.cpp efficientFrontier.cpp smooth.cpp shuffle.cpp diffs.cpp kernelDensity.cpp rowmax.cpp util.cpp eigen.cpp cgram.cpp round.cpp arithmetic.cpp regression.cpp quantiles.cpp sort.cpp indexx.cpp cumulate.cpp zero.cpp urand.cpp nrand.cpp iota.cpp matmult.cpp specialFuncs.cpp inverse.cpp varcovar.cpp correlmat.cpp

include_HEADERS = rcm.h rcm-template-classes.h rcm-template-classes.hImpl rcm-template-functions.h rcm-template-functions.hImpl rcm-non-template-functions.h rcm-util.h rcm-regression.h

librcm_la_CPPFLAGS = -DRCM_MANUAL_COW=$(RCM_MANUAL_COW) -DRCM_DEBUG=$(RCM_DEBUG)

bin_PROGRAMS = testsuite
testsuite_SOURCES = testsuite.cpp
testsuite_LDADD = librcm.la
