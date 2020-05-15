// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#include "thermal_operators.hpp"

DynamicConductionOperator::DynamicConductionOperator(std::shared_ptr<mfem::ParFiniteElementSpace> fespace,
                                                     const LinearSolverParameters &               params)
    : mfem::TimeDependentOperator(fespace->GetTrueVSize(), 0.0),
      m_fespace(fespace),
      m_z(fespace->GetTrueVSize()),
      m_y(fespace->GetTrueVSize()),
      m_x(fespace->GetTrueVSize()),
      m_old_dt(-1.0)
{
  // Set the mass solver options (CG and Jacobi for now)
  m_M_solver = std::make_unique<mfem::CGSolver>(m_fespace->GetComm());
  m_M_prec   = std::make_unique<mfem::HypreSmoother>();

  m_M_solver->iterative_mode = false;
  m_M_solver->SetRelTol(params.rel_tol);
  m_M_solver->SetAbsTol(params.abs_tol);
  m_M_solver->SetMaxIter(params.max_iter);
  m_M_solver->SetPrintLevel(params.print_level);
  m_M_prec->SetType(mfem::HypreSmoother::Jacobi);
  m_M_solver->SetPreconditioner(*m_M_prec);

  // Use the same options for the T (= M + dt K) solver
  m_T_solver = std::make_unique<mfem::CGSolver>(m_fespace->GetComm());
  m_T_prec   = std::make_unique<mfem::HypreSmoother>();

  m_T_solver->iterative_mode = false;
  m_T_solver->SetRelTol(params.rel_tol);
  m_T_solver->SetAbsTol(params.abs_tol);
  m_T_solver->SetMaxIter(params.max_iter);
  m_T_solver->SetPrintLevel(params.print_level);
  m_T_solver->SetPreconditioner(*m_T_prec);

  m_state_gf = std::make_shared<mfem::ParGridFunction>(m_fespace.get());
  m_bc_rhs   = std::make_shared<mfem::Vector>(fespace->GetTrueVSize());
}

void DynamicConductionOperator::SetMMatrix(std::shared_ptr<mfem::HypreParMatrix> M_mat,
                                           std::shared_ptr<mfem::HypreParMatrix> M_e_mat)
{
  // Set the mass matrix
  m_M_mat   = M_mat;
  m_M_e_mat = M_e_mat;
}

void DynamicConductionOperator::SetKMatrix(std::shared_ptr<mfem::HypreParMatrix> K_mat,
                                           std::shared_ptr<mfem::HypreParMatrix> K_e_mat)
{
  // Set the stiffness matrix and RHS
  m_K_mat   = K_mat;
  m_K_e_mat = K_e_mat;
}

void DynamicConductionOperator::SetLoadVector(std::shared_ptr<mfem::Vector> rhs) { m_rhs = rhs; }

void DynamicConductionOperator::SetEssentialBCs(std::shared_ptr<mfem::Coefficient> ess_bdr_coef,
                                                const mfem::Array<int> &ess_bdr, const mfem::Array<int> &ess_tdof_list)
{
  m_ess_bdr_coef  = ess_bdr_coef;
  m_ess_bdr       = ess_bdr;
  m_ess_tdof_list = ess_tdof_list;
}

// TODO: allow for changing thermal essential boundary conditions
void DynamicConductionOperator::Mult(const mfem::Vector &u, mfem::Vector &du_dt) const
{
  MFEM_ASSERT(m_M_mat != nullptr, "Mass matrix not set in ConductionSolver::Mult!");
  MFEM_ASSERT(m_K_mat != nullptr, "Stiffness matrix not set in ConductionSolver::Mult!");

  m_y = u;
  m_M_solver->SetOperator(*m_M_mat);

  *m_bc_rhs = *m_rhs;
  mfem::EliminateBC(*m_K_mat, *m_K_e_mat, m_ess_tdof_list, m_y, *m_bc_rhs);

  // Compute:
  //    du_dt = M^{-1}*-K(u)
  // for du_dt
  m_K_mat->Mult(m_y, m_z);
  m_z.Neg();  // z = -zw  m_z.Add(1.0, *m_bc_rhs);
  m_z.Add(1.0, *m_bc_rhs);
  m_M_solver->Mult(m_z, du_dt);
}

void DynamicConductionOperator::ImplicitSolve(const double dt, const mfem::Vector &u, mfem::Vector &du_dt)
{
  MFEM_ASSERT(m_M_mat != nullptr, "Mass matrix not set in ConductionSolver::ImplicitSolve!");
  MFEM_ASSERT(m_K_mat != nullptr, "Stiffness matrix not set in ConductionSolver::ImplicitSolve!");

  // Save a copy of the current state vector
  m_y = u;

  // Solve the equation:
  //    du_dt = M^{-1}*[-K(u + dt*du_dt)]
  // for du_dt
  if (dt != m_old_dt) {
    m_T_mat.reset(mfem::Add(1.0, *m_M_mat, dt, *m_K_mat));

    // Eliminate the essential DOFs from the T matrix
    m_T_e_mat.reset(m_T_mat->EliminateRowsCols(m_ess_tdof_list));
    m_T_solver->SetOperator(*m_T_mat);
  }

  // Apply the boundary conditions
  *m_bc_rhs = *m_rhs;
  m_x       = 0.0;

  if (m_ess_bdr_coef != nullptr) {
    m_ess_bdr_coef->SetTime(t);
    m_state_gf->SetFromTrueDofs(m_y);
    m_state_gf->ProjectBdrCoefficient(*m_ess_bdr_coef, m_ess_bdr);
    m_state_gf->GetTrueDofs(m_y);

    mfem::EliminateBC(*m_K_mat, *m_K_e_mat, m_ess_tdof_list, m_y, *m_bc_rhs);
  }

  m_K_mat->Mult(m_y, m_z);
  m_z.Neg();
  m_z.Add(1.0, *m_bc_rhs);
  m_T_solver->Mult(m_z, du_dt);

  // Save the dt used to compute the T matrix
  m_old_dt = dt;
}

DynamicConductionOperator::~DynamicConductionOperator() {}