typedef boost::multi_array<double, 1> cvector;   // column vector
typedef cvector::index cvector_index;
typedef boost::multi_array<double, 2> matrix;
typedef matrix::index matrix_index;

cvector Ols( cvector lhs, matrix rhs );
matrix MatMatMult( matrix const & mat1, matrix const & mat2 );
cvector MatCvectMult( matrix const & mat, cvector const & cvect );
matrix MatTranspose( matrix const & mat );
matrix MatInverse( matrix const & mat );
cvector CvectMinus( cvector const & v1, cvector const & v2 );
void gaussj( matrix & a, int n, matrix & b, int m);
