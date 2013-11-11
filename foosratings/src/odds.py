# Print table of winning odds, based on ratings spread.
from math import pow, exp

def factorial(n):
    if n <= 1:
        return 1
    result = 2
    for i in range(3,n+1):
        result *= i
    return result

#for n in range(1,10): print factorial(n)

def comb(n,m):
    if n < m: return 0
    result = n
    for i in range(m+1,n):
        result *= i
    result /= factorial(n-m)
    return result

# Probability of a 5-m game when stronger player's rating is greater by spread.
def likelihood(m, spread):
    p = exp(spread)/(1+exp(spread)) # prob of winning any point    
    l = comb(m+5-1,m)*pow(p,5)*pow(1-p,m)
    #print "p=", p, ", lik", (m, spread), "=", l
    return l

# Prob of stronger player winning.
def probwin(spread):
    probwin = 0
    for m in range(0,5):
        probwin += likelihood(m, spread)
    return probwin

# Expected winning margin by stronger player.
def expected_margin(spread):
    mw = 0 # Expected losing score conditional on stronger player winning.
    pw = probwin(spread)
    ml = 0 # Expected losing score conditional on stronger player losing.
    for m in range(0,5):
        mw += m * likelihood(m, spread)
        ml += m * likelihood(m, -spread)
    return pw*(5 - mw) - (1-pw)*(5 - ml)


print "(spread, probwin, exp_margin)"
for s in range(0,31):
    ss = s/10.0
    print '%6.1f'%ss, '%8.3f'%probwin(ss), '%8.2f'%expected_margin(s/10.0)
