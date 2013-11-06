
/** Abstract class for linear models. */
class Regression
{
public:

    Regression( Vd y, Md x );
    virtual Regression* clone() const = 0;
    virtual Regression* clone( const Vd& y, const Md& x ) const = 0;
    virtual ~Regression();
    //
    // Getters.  Some do lazy evaluation.  Subclasses implement
    // the compute_* functions.
    //
    Md x();               // independent variables
    Vd y();               // dependent variable
    Vd yhat();            // fitted y
    Vd beta();            // estimated parameters
    Vd res();             // residual errors
    double r2();          // R-squared
    Vd tstats();          // beta / sd(beta)
    Md betaVcv();         // sd(res)*inv(x'x)
    Md betaCorrelmat();   // normalized betaVcv.
    Md displayAll();      // beta, tstats, betaCorrelmat

protected:
    bool yhatIsComputed;
    bool betaIsComputed;
    bool resIsComputed;
    bool r2IsComputed;
    bool tstatsIsComputed;
    bool betaVcvIsComputed;
    bool betaCorrelmatIsComputed;

    // These guys set the corresponding var_ variables, e.g. yhat_, beta_,...
    virtual void compute_yhat()=0;
    virtual void compute_beta()=0;
    virtual void compute_res()=0;
    virtual void compute_r2()=0;
    virtual void compute_tstats()=0;
    virtual void compute_betaVcv()=0;
    virtual void compute_betaCorrelmat()=0;

    Md x_;
    Vd y_;
    Vd yhat_;
    Vd beta_;
    Vd res_;
    double r2_;
    Vd tstats_;
    Md betaVcv_;
    Md betaCorrelmat_;
};

class Ols : public Regression
{
public:

    Ols( Vd y, Md x );
    Regression* clone() const;
    Regression* clone( const Vd& y, const Md& x ) const;
private:

    void compute_yhat();
    void compute_beta();
    void compute_res();
    void compute_r2();
    void compute_tstats();
    void compute_betaVcv();
    void compute_betaCorrelmat();
};

class Tls : public Regression
{
public:

    Tls( Vd y, Md x, double trimFactor );
    Regression* clone() const;
    Regression* clone( const Vd& y, const Md& x ) const;

private:

    void compute_yhat();
    void compute_beta();
    void compute_res();
    void compute_r2();
    void compute_tstats();
    void compute_betaVcv();
    void compute_betaCorrelmat();

    Md estimate( const void* data );
    const Md* amoebaData[3]; // y, x, trimFactor

    double m_trimFactor;
};
