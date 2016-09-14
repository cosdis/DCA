//-*-C++-*-                                                                                                                                                                                                                                                                                                                                                        

#ifndef LINALG_PSEUDO_INVERSE_CPU_H
#define LINALG_PSEUDO_INVERSE_CPU_H

namespace LIN_ALG 
{  
  template<>
  class PSEUDO_INVERSE<CPU>
  {
  public:

      template<typename scalartype>
      static void execute(matrix<scalartype, CPU>& A,
			  matrix<scalartype, CPU>& A_inv,
			  scalartype EPSILON=1.e-6)
      {
	int M = A.size().first;
	int N = A.size().second;

	if(M<=N)
	  {
	    // A_inv = A'*inv(A*A')

	    matrix<scalartype, CPU> A_At("A_At", std::pair<int,int>(M,M));

	    dca::linalg::matrixop::gemm('N', 'C', A, A, A_At);

	    dca::linalg::Vector<scalartype, CPU> S ("S" , M);
	    matrix<scalartype, CPU> V ("V" , std::pair<int,int>(M,M));
	    matrix<scalartype, CPU> Vt("Vt", std::pair<int,int>(M,M));

	    {
	      // inv(A*A') = V*inv(D)*V';   [D, V] = eig(A*A')

	      
	      GEEV<CPU>::execute('V', 'U', A_At, S, V);	      

	      Vt.copy_from(V);
	      
	      for(int j=0; j<M; j++)
		{
		  scalartype one_over_sigma=0;
		  
		  if(S[j]>EPSILON*S[M-1])
		    one_over_sigma = 1./S[j];
		  
		  for(int i=0; i<M; i++)
		    V(i,j) *= one_over_sigma;
		}
	    }
	      
	    dca::linalg::matrixop::gemm('N', 'C', V, Vt  , A_At);
	    dca::linalg::matrixop::gemm('C', 'N', A, A_At, A_inv);	 
	  }
	else
	  {
	    // A_inv = inv(A'*A)*A'

	    matrix<scalartype, CPU> At_A("At_A", std::pair<int,int>(N,N));

	    dca::linalg::matrixop::gemm('C', 'N', A, A, At_A);

	    dca::linalg::Vector<scalartype, CPU> S ("S" , N);
	    matrix<scalartype, CPU> V ("V" , std::pair<int,int>(N,N));
	    matrix<scalartype, CPU> Vt("Vt", std::pair<int,int>(N,N));

	    {
	      // inv(A'*A) = V*inv(D)*V';   [D, V] = eig(A'*A)	    
	      
	      GEEV<CPU>::execute('V', 'U', At_A, S, V);
	      
	      Vt.copy_from(V);
	      
	      for(int j=0; j<N; j++)
		{
		  scalartype one_over_sigma=0;
		  
		  if(S[j]>EPSILON*S[M-1])
		    one_over_sigma = 1./S[j];
		  
		  for(int i=0; i<N; i++)
		    V(i,j) *= one_over_sigma;
		}
	    }

	    dca::linalg::matrixop::gemm('N', 'C', V   , Vt, At_A);
	    dca::linalg::matrixop::gemm('N', 'C', At_A, A , A_inv);	    
	  }
      }

      template<typename scalartype>
      static void execute(matrix<std::complex<scalartype>, CPU>& A,
			  matrix<std::complex<scalartype>, CPU>& A_inv,
			  scalartype EPSILON=1.e-6)
      {
	int M = A.size().first;
	int N = A.size().second;

	if(M<=N)
	  {
	    // A_inv = A'*inv(A*A')

	    matrix<std::complex<scalartype>, CPU> A_At("A_At", std::pair<int,int>(M,M));

	    dca::linalg::matrixop::gemm('N', 'C', A, A, A_At);

	    dca::linalg::Vector<scalartype, CPU> S ("S" , M);
	    matrix<std::complex<scalartype>, CPU> V ("V" , std::pair<int,int>(M,M));
	    matrix<std::complex<scalartype>, CPU> Vt("Vt", std::pair<int,int>(M,M));

	    {
	      // inv(A*A') = V*inv(D)*V';   [D, V] = eig(A*A')

	      GEEV<CPU>::execute('V', 'U', A_At, S, V);

	      Vt.copy_from(V);
	      
	      for(int j=0; j<M; j++)
		{
		  scalartype one_over_sigma=0;
		  
		  if(S[j]>EPSILON*S[M-1]){
		    one_over_sigma = 1./S[j];

		    //cout << j << "\t" << S[j] << endl;
		  }

		  for(int i=0; i<M; i++)
		    V(i,j) *= one_over_sigma;
		}
	    }

	    dca::linalg::matrixop::gemm('N', 'C', V, Vt, A_At);

	    dca::linalg::matrixop::gemm('C', 'N', A, A_At, A_inv);	    
	  }
	else
	  {
	    // A_inv = inv(A'*A)*A'

	    matrix<std::complex<scalartype>, CPU> At_A("At_A", std::pair<int,int>(N,N));

	    dca::linalg::matrixop::gemm('C', 'N', A, A, At_A);

	    dca::linalg::Vector<scalartype, CPU> S ("S" , N);
	    matrix<std::complex<scalartype>, CPU> V ("V" , std::pair<int,int>(N,N));
	    matrix<std::complex<scalartype>, CPU> Vt("Vt", std::pair<int,int>(N,N));

	    {
	      // inv(A'*A) = V*inv(D)*V';   [D, V] = eig(A'*A)	    
	      
	      GEEV<CPU>::execute('V', 'U', At_A, S, V);
	      
	      Vt.copy_from(V);
	      
	      for(int j=0; j<N; j++)
		{
		  scalartype one_over_sigma=0;
		  
		  if(S[j]>EPSILON*S[N-1])
		    one_over_sigma = 1./S[j];
		  
		  for(int i=0; i<N; i++)
		    V(i,j) *= one_over_sigma;
		}
	    }

	    dca::linalg::matrixop::gemm('N', 'C', V   , Vt, At_A);
	    dca::linalg::matrixop::gemm('N', 'C', At_A, A , A_inv);	    
	  }
      }

    };
}

#endif
