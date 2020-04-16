// Copyright (c) 2019-2020, Lawrence Livermore National Security, LLC and
// other Serac Project Developers. See the top-level LICENSE file for
// details.
//
// SPDX-License-Identifier: (BSD-3-Clause)

#ifndef WRAPPER_INTEGRATOR_HPP
#define WRAPPER_INTEGRATOR_HPP

#include <memory>

#include "mfem.hpp"

/// A class to convert linearform integrators into a nonlinear residual-based one
class LinearToNonlinearFormIntegrator : public mfem::NonlinearFormIntegrator {
 public:
  /**
     \brief Recasts, A(u) = F as R(u) = A(u) - F

  \param[in] f A LinearFormIntegrator
   */
  LinearToNonlinearFormIntegrator(std::shared_ptr<mfem::LinearFormIntegrator>  f,
                                  std::shared_ptr<mfem::ParFiniteElementSpace> trial_fes);

  /// Compute the residual vector => -F
  virtual void AssembleElementVector(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                     const mfem::Vector &elfun, mfem::Vector &elvect);

  /// Compute the tangent matrix = 0
  virtual void AssembleElementGrad(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                   const mfem::Vector &elfun, mfem::DenseMatrix &elmat);

 private:
  std::shared_ptr<mfem::LinearFormIntegrator>  m_f;
  std::shared_ptr<mfem::ParFiniteElementSpace> m_trial_fes;
};

/// A class to convert linearform integrators into a nonlinear residual-based one
class BilinearToNonlinearFormIntegrator : public mfem::NonlinearFormIntegrator {
 public:
  /**
     \brief Recasts, A(u) = F as R(u) = A(u) - F

     \param[in] A A BilinearFormIntegrator
   */
  BilinearToNonlinearFormIntegrator(std::shared_ptr<mfem::BilinearFormIntegrator> A);

  /// Compute the residual vector
  virtual void AssembleElementVector(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                     const mfem::Vector &elfun, mfem::Vector &elvect);

  /// Compute the tangent matrix
  virtual void AssembleElementGrad(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                   const mfem::Vector &elfun, mfem::DenseMatrix &elmat);

 private:
  std::shared_ptr<mfem::BilinearFormIntegrator> m_A;
};

/// A class to convert a MixedBiolinearIntegrator into a nonlinear residual-based one
class MixedBilinearToNonlinearFormIntegrator : public mfem::NonlinearFormIntegrator {
 public:
  /**
     \brief Recasts, A(u) = F as R(u) = A(u) - F

     \param[in] A A MixedBilinearFormIntegrator
   */
  MixedBilinearToNonlinearFormIntegrator(std::shared_ptr<mfem::BilinearFormIntegrator> A,
                                         std::shared_ptr<mfem::ParFiniteElementSpace>  trial_fes);

  /// Compute the residual vector
  virtual void AssembleElementVector(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                     const mfem::Vector &elfun, mfem::Vector &elvect);

  /// Compute the tangent matrix
  virtual void AssembleElementGrad(const mfem::FiniteElement &el, mfem::ElementTransformation &Tr,
                                   const mfem::Vector &elfun, mfem::DenseMatrix &elmat);

 private:
  std::shared_ptr<mfem::BilinearFormIntegrator> m_A;
  std::shared_ptr<mfem::ParFiniteElementSpace>  m_trial_fes;
};

#endif
