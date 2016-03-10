//-*-C++-*-

#ifndef LEAST_SQUARE_PLAN_COMPLEX_H
#define LEAST_SQUARE_PLAN_COMPLEX_H

/*!
 *   \author: peter staar
 */
template<typename scalartype>
class least_square_plan<std::complex<scalartype> >
{

 public:

  least_square_plan(int m, int n, int nrhs, std::complex<scalartype>* a, std::complex<scalartype>* b);
  ~least_square_plan();

  int execute_plan();

 public:

  int M;
  int N;

  int NRHS;

  std::complex<scalartype>* A;
  int LDA;

  std::complex<scalartype>* B;
  int LDB;

  scalartype* S;

  scalartype RCOND;
  int RANK;

  std::complex<scalartype>* WORK;
  int LWORK;

  scalartype* RWORK;

//   int* IWORK;

  int INFO;

};

template<typename scalartype>
least_square_plan<std::complex<scalartype> >::least_square_plan(int m, int n, int nrhs, std::complex<scalartype>* a, std::complex<scalartype>* b):
  M(m),
  N(n),
  NRHS(nrhs),

  A(a),
  LDA(m),

  B(b),
  LDB(m),

  RCOND(-1),

  LWORK(2*(2*min(M,N) + max(M,max(N,NRHS))))
{
  S     = new scalartype[min(N,M)];
  WORK  = new std::complex<scalartype>[LWORK];
  RWORK = new scalartype[5*(5*min(N,M))];
}

template<typename scalartype>
least_square_plan<std::complex<scalartype> >::~least_square_plan()
{
  delete [] S;
  delete [] WORK;
  delete [] RWORK;
}

template<typename scalartype>
int least_square_plan<std::complex<scalartype> >::execute_plan()
{
  throw std::logic_error(__FUNCTION__);
}

template<>
int least_square_plan<std::complex<float> >::execute_plan()
{
  LAPACK::cgelss_( &M, &N, &NRHS, A, &LDA, B, &LDB, S, &RCOND, &RANK, WORK, &LWORK, RWORK, &INFO);
  return INFO;
}

template<>
int least_square_plan<std::complex<double> >::execute_plan()
{
  LAPACK::zgelss_( &M, &N, &NRHS, A, &LDA, B, &LDB, S, &RCOND, &RANK, WORK, &LWORK, RWORK, &INFO);
  return INFO;
}

#endif