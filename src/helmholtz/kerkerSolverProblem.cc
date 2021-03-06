// ---------------------------------------------------------------------
//
// Copyright (c) 2019-2020 The Regents of the University of Michigan and DFT-FE authors.
//
// This file is part of the DFT-FE code.
//
// The DFT-FE code is free software; you can use it, redistribute
// it, and/or modify it under the terms of the GNU Lesser General
// Public License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// The full text of the license can be found in the file LICENSE at
// the top level of the DFT-FE distribution.
//
// ---------------------------------------------------------------------
//
// @author Phani Motamarri
//

#include <kerkerSolverProblem.h>
#include <constants.h>
#include <dftParameters.h>

namespace dftfe {
  //
  //constructor
  //
  template<unsigned int FEOrder>
  kerkerSolverProblem<FEOrder>::kerkerSolverProblem(const  MPI_Comm &mpi_comm):
    mpi_communicator (mpi_comm),
    n_mpi_processes (dealii::Utilities::MPI::n_mpi_processes(mpi_comm)),
    this_mpi_process (dealii::Utilities::MPI::this_mpi_process(mpi_comm)),
    pcout (std::cout, (dealii::Utilities::MPI::this_mpi_process(MPI_COMM_WORLD) == 0))
  {

  }


  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::init(dealii::MatrixFree<3,double> & matrixFreeDataPRefined,
					  dealii::ConstraintMatrix & constraintMatrixPRefined,
					  vectorType & x,
					  double kerkerMixingParameter)
  {
    d_matrixFreeDataPRefinedPtr = &matrixFreeDataPRefined;
    d_constraintMatrixPRefinedPtr = &constraintMatrixPRefined;
    d_gamma = kerkerMixingParameter;

    matrixFreeDataPRefined.initialize_dof_vector(x);
    computeDiagonalA();
  }
  

  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::reinit(vectorType & x,
					    const std::map<dealii::CellId,std::vector<double> > & quadPointValues)
  {
    d_xPtr = &x;
    d_quadGradResidualValuesPtr = &quadPointValues;
  }

  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::distributeX()
  {
    d_constraintMatrixPRefinedPtr->distribute(*d_xPtr);
  }

  template<unsigned int FEOrder>
  vectorType & kerkerSolverProblem<FEOrder>::getX()
  {
    return *d_xPtr;
  }

  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::computeRhs(vectorType  & rhs)
  {

    rhs.reinit(*d_xPtr);

    const dealii::DoFHandler<3> & dofHandler=
      d_matrixFreeDataPRefinedPtr->get_dof_handler();

    dealii::QGauss<3>  quadrature(C_num1DQuad<FEOrder>());
    dealii::FEValues<3> fe_values (dofHandler.get_fe(), quadrature, dealii::update_values | dealii::update_gradients | dealii::update_JxW_values);
    const unsigned int dofs_per_cell = dofHandler.get_fe().dofs_per_cell;
    const unsigned int num_quad_points = quadrature.size();
    dealii::Vector<double>  elementalRhs(dofs_per_cell);
    std::vector<dealii::types::global_dof_index> local_dof_indices (dofs_per_cell);

    //rhs contribution from static condensation of dirichlet boundary conditions
    typename dealii::DoFHandler<3>::active_cell_iterator cell = dofHandler.begin_active(), endc = dofHandler.end();
	
    //rhs contribution from electronic charge
    cell = dofHandler.begin_active();
    for(; cell!=endc; ++cell)
      {
	if (cell->is_locally_owned())
	  {
	    fe_values.reinit (cell);
	    elementalRhs=0.0;

	    const std::vector<double>& tempVec=d_quadGradResidualValuesPtr->find(cell->id())->second;
	    for (unsigned int i=0; i<dofs_per_cell; ++i)
	      {
		for (unsigned int q_point=0; q_point<num_quad_points; ++q_point)
		  {
		    dealii::Tensor<1,3,double> shapeFuncGrad = fe_values.shape_grad(i,q_point);
		    elementalRhs(i) += (shapeFuncGrad[0]*tempVec[3*q_point+0] + shapeFuncGrad[1]*tempVec[3*q_point+1] + shapeFuncGrad[2]*tempVec[3*q_point+2])*fe_values.JxW(q_point);
		  }
	      }

	    //assemble to global data structures
	    cell->get_dof_indices (local_dof_indices);
	    d_constraintMatrixPRefinedPtr->distribute_local_to_global(elementalRhs, local_dof_indices, rhs);
	  }
      }


    //MPI operation to sync data
    rhs.compress(dealii::VectorOperation::add);

    //FIXME: check if this is really required
    d_constraintMatrixPRefinedPtr->set_zero(rhs);

  }

  //Matrix-Free Jacobi preconditioner application
  template<unsigned int FEOrder>
  void  kerkerSolverProblem<FEOrder>::precondition_Jacobi(vectorType& dst,
								  const vectorType& src,
								  const double omega) const
  {
    dst = src;
    dst.scale(d_diagonalA);
  }

  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::computeDiagonalA()
  {

    const dealii::DoFHandler<3> & dofHandler=
      d_matrixFreeDataPRefinedPtr->get_dof_handler();
  
    d_matrixFreeDataPRefinedPtr->initialize_dof_vector(d_diagonalA);
    d_diagonalA = 0.0;

    dealii::QGauss<3>  quadrature(C_num1DQuad<FEOrder>());
    dealii::FEValues<3> fe_values (dofHandler.get_fe(), quadrature, dealii::update_values | dealii::update_gradients | dealii::update_JxW_values);
    const unsigned int   dofs_per_cell = dofHandler.get_fe().dofs_per_cell;
    const unsigned int   num_quad_points = quadrature.size();
    dealii::Vector<double>  elementalDiagonalA(dofs_per_cell);
    std::vector<dealii::types::global_dof_index> local_dof_indices (dofs_per_cell);


    //parallel loop over all elements
    typename dealii::DoFHandler<3>::active_cell_iterator cell = dofHandler.begin_active(), endc = dofHandler.end();
    for(; cell!=endc; ++cell)
      if (cell->is_locally_owned())
	{
	  fe_values.reinit (cell);

	  cell->get_dof_indices (local_dof_indices);

	  elementalDiagonalA=0.0;
	  for (unsigned int i = 0; i < dofs_per_cell; ++i)
	    for (unsigned int q_point = 0; q_point < num_quad_points; ++q_point)
	      elementalDiagonalA(i) += (fe_values.shape_grad(i, q_point)*fe_values.shape_grad (i, q_point)+ 4*M_PI*d_gamma*fe_values.shape_value(i,q_point)*fe_values.shape_value(i,q_point))*fe_values.JxW(q_point);

	  d_constraintMatrixPRefinedPtr->distribute_local_to_global(elementalDiagonalA,
								    local_dof_indices,
								    d_diagonalA);
	}

    //MPI operation to sync data
    d_diagonalA.compress(dealii::VectorOperation::add);

    for(dealii::types::global_dof_index i = 0; i < d_diagonalA.size(); ++i)
      if(d_diagonalA.in_local_range(i))
	if(!d_constraintMatrixPRefinedPtr->is_constrained(i))
	  d_diagonalA(i) = 1.0/d_diagonalA(i);

    d_diagonalA.compress(dealii::VectorOperation::insert);
  }

  //Ax
  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::AX(const dealii::MatrixFree<3,double> & matrixFreeData,
						vectorType &dst,
						const vectorType &src,
						const std::pair<unsigned int,unsigned int> &cell_range) const
  {

    dealii::FEEvaluation<3,FEOrder,C_num1DQuad<FEOrder>()> fe_eval(matrixFreeData,
								   0,
								   0);
    //double gamma = dftParameters::kerkerParameter;

    dealii::VectorizedArray<double>  kerkerConst = dealii::make_vectorized_array(4*M_PI*d_gamma);


    for (unsigned int cell=cell_range.first; cell<cell_range.second; ++cell)
      {
	fe_eval.reinit(cell);
	fe_eval.read_dof_values(src);
	fe_eval.evaluate(true,true,false);
	for (unsigned int q=0; q<fe_eval.n_q_points; ++q)
	  {
	    fe_eval.submit_gradient(fe_eval.get_gradient(q), q);
	    fe_eval.submit_value(fe_eval.get_value(q)*kerkerConst,q);
	  }
	fe_eval.integrate(true, true);
	fe_eval.distribute_local_to_global(dst);
      }
  }


  template<unsigned int FEOrder>
  void kerkerSolverProblem<FEOrder>::vmult(vectorType &Ax,const vectorType &x) const
  {
    Ax=0.0;
    d_matrixFreeDataPRefinedPtr->cell_loop (&kerkerSolverProblem<FEOrder>::AX, this, Ax, x);
  }


  template class kerkerSolverProblem<1>;
  template class kerkerSolverProblem<2>;
  template class kerkerSolverProblem<3>;
  template class kerkerSolverProblem<4>;
  template class kerkerSolverProblem<5>;
  template class kerkerSolverProblem<6>;
  template class kerkerSolverProblem<7>;
  template class kerkerSolverProblem<8>;
  template class kerkerSolverProblem<9>;
  template class kerkerSolverProblem<10>;
  template class kerkerSolverProblem<11>;
  template class kerkerSolverProblem<12>;
  template class kerkerSolverProblem<13>;
  template class kerkerSolverProblem<14>;
  template class kerkerSolverProblem<15>;
  template class kerkerSolverProblem<16>;
  template class kerkerSolverProblem<17>;
  template class kerkerSolverProblem<18>;
  template class kerkerSolverProblem<19>;
  template class kerkerSolverProblem<20>;
  template class kerkerSolverProblem<21>;
  template class kerkerSolverProblem<22>;
  template class kerkerSolverProblem<23>;
  template class kerkerSolverProblem<24>;
}
