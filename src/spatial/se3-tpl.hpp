//
// Copyright (c) 2015-2018 CNRS
// Copyright (c) 2016 Wandercraft, 86 rue de Paris 91400 Orsay, France.
//
// This file is part of Pinocchio
// Pinocchio is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// Pinocchio is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// General Lesser Public License for more details. You should have
// received a copy of the GNU Lesser General Public License along with
// Pinocchio If not, see
// <http://www.gnu.org/licenses/>.

#ifndef __pinocchio_se3_tpl_hpp__
#define __pinocchio_se3_tpl_hpp__

#include <Eigen/Geometry>
#include "pinocchio/math/quaternion.hpp"

namespace pinocchio
{
  template<typename _Scalar, int _Options>
  struct traits< SE3Tpl<_Scalar,_Options> >
  {
    enum {
      Options = _Options,
      LINEAR = 0,
      ANGULAR = 3
    };
    typedef _Scalar Scalar;
    typedef Eigen::Matrix<Scalar,3,1,Options> Vector3;
    typedef Eigen::Matrix<Scalar,4,1,Options> Vector4;
    typedef Eigen::Matrix<Scalar,6,1,Options> Vector6;
    typedef Eigen::Matrix<Scalar,3,3,Options> Matrix3;
    typedef Eigen::Matrix<Scalar,4,4,Options> Matrix4;
    typedef Eigen::Matrix<Scalar,6,6,Options> Matrix6;
    typedef Matrix3 AngularType;
    typedef typename EIGEN_REF_TYPE(Matrix3) AngularRef;
    typedef typename EIGEN_REF_CONSTTYPE(Matrix3) ConstAngularRef;
    typedef Vector3 LinearType;
    typedef typename EIGEN_REF_TYPE(Vector3) LinearRef;
    typedef typename EIGEN_REF_CONSTTYPE(Vector3) ConstLinearRef;
    typedef Matrix6 ActionMatrixType;
    typedef Matrix4 HomogeneousMatrixType;
  }; // traits SE3Tpl
  
  template<typename _Scalar, int _Options>
  struct SE3Tpl : public SE3Base< SE3Tpl<_Scalar,_Options> >
  {
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    SE3_TYPEDEF_TPL(SE3Tpl);
    typedef SE3Base< SE3Tpl<_Scalar,_Options> > Base;
    typedef Eigen::Quaternion<Scalar,Options> Quaternion;
    typedef typename traits<SE3Tpl>::Vector3 Vector3;
    typedef typename traits<SE3Tpl>::Matrix3 Matrix3;
    typedef typename traits<SE3Tpl>::Vector4 Vector4;
    typedef typename traits<SE3Tpl>::Matrix6 Matrix6;
    
    using Base::rotation;
    using Base::translation;
    
    SE3Tpl(): rot(), trans() {};
    
    template<typename Matrix3Like,typename Vector3Like>
    SE3Tpl(const Eigen::MatrixBase<Matrix3Like> & R,
           const Eigen::MatrixBase<Vector3Like> & p)
    : rot(R), trans(p)
    {
      EIGEN_STATIC_ASSERT_VECTOR_SPECIFIC_SIZE(Vector3Like,3)
      EIGEN_STATIC_ASSERT_MATRIX_SPECIFIC_SIZE(Matrix3Like,3,3)
    }
    
    template<typename Matrix4Like>
    explicit SE3Tpl(const Eigen::MatrixBase<Matrix4Like> & m)
    : rot(m.template block<3,3>(LINEAR,LINEAR))
    , trans(m.template block<3,1>(LINEAR,ANGULAR))
    {
      EIGEN_STATIC_ASSERT_MATRIX_SPECIFIC_SIZE(Matrix4Like,4,4);
    }
    
    SE3Tpl(int)
    : rot(AngularType::Identity())
    , trans(LinearType::Zero())
    {}
    
    template<int O2>
    SE3Tpl(const SE3Tpl<Scalar,O2> & clone)
    : rot(clone.rotation()),trans(clone.translation()) {}
    
    template<int O2>
    SE3Tpl & operator=(const SE3Tpl<Scalar,O2> & other)
    {
      rot = other.rotation();
      trans = other.translation();
      return *this;
    }
    
    static SE3Tpl Identity()
    {
      return SE3Tpl(1);
    }
    
    SE3Tpl & setIdentity()
    { rot.setIdentity (); trans.setZero (); return *this;}
    
    /// aXb = bXa.inverse()
    SE3Tpl inverse() const
    {
      return SE3Tpl(rot.transpose(), -rot.transpose()*trans);
    }
    
    static SE3Tpl Random()
    {
      return SE3Tpl().setRandom();
    }
    
    SE3Tpl & setRandom()
    {
      Quaternion q; quaternion::uniformRandom(q);
      rot = q.matrix();
      trans.setRandom();
      
      return *this;
    }
    
    HomogeneousMatrixType toHomogeneousMatrix_impl() const
    {
      HomogeneousMatrixType M;
      M.template block<3,3>(LINEAR,LINEAR) = rot;
      M.template block<3,1>(LINEAR,ANGULAR) = trans;
      M.template block<1,3>(ANGULAR,LINEAR).setZero();
      M(3,3) = 1;
      return M;
    }
    
    /// Vb.toVector() = bXa.toMatrix() * Va.toVector()
    ActionMatrixType toActionMatrix_impl() const
    {
      typedef Eigen::Block<ActionMatrixType,3,3> Block3;
      ActionMatrixType M;
      M.template block<3,3>(ANGULAR,ANGULAR)
      = M.template block<3,3>(LINEAR,LINEAR) = rot;
      M.template block<3,3>(ANGULAR,LINEAR).setZero();
      Block3 B = M.template block<3,3>(LINEAR,ANGULAR);
      
      B.col(0) = trans.cross(rot.col(0));
      B.col(1) = trans.cross(rot.col(1));
      B.col(2) = trans.cross(rot.col(2));
      return M;
    }
    
    ActionMatrixType toDualActionMatrix_impl() const
    {
      typedef Eigen::Block<ActionMatrixType,3,3> Block3;
      ActionMatrixType M;
      M.template block<3,3>(ANGULAR,ANGULAR)
      = M.template block<3,3>(LINEAR,LINEAR) = rot;
      M.template block<3,3>(LINEAR,ANGULAR).setZero();
      Block3 B = M.template block<3,3>(ANGULAR,LINEAR);
      
      B.col(0) = trans.cross(rot.col(0));
      B.col(1) = trans.cross(rot.col(1));
      B.col(2) = trans.cross(rot.col(2));
      return M;
    }
    
    void disp_impl(std::ostream & os) const
    {
      os
      << "  R =\n" << rot << std::endl
      << "  p = " << trans.transpose() << std::endl;
    }
    
    /// --- GROUP ACTIONS ON M6, F6 and I6 ---
    
    /// ay = aXb.act(by)
    template<typename D>
    typename internal::SE3GroupAction<D>::ReturnType
    act_impl(const D & d) const
    {
      return d.se3Action(*this);
    }
    
    /// by = aXb.actInv(ay)
    template<typename D> typename internal::SE3GroupAction<D>::ReturnType
    actInv_impl(const D & d) const
    {
      return d.se3ActionInverse(*this);
    }
    
    template<typename EigenDerived>
    typename EigenDerived::PlainObject
    actOnEigenObject(const Eigen::MatrixBase<EigenDerived> & p) const
    { return (rotation()*p+translation()).eval(); }
    
    template<typename MapDerived>
    Vector3 actOnEigenObject(const Eigen::MapBase<MapDerived> & p) const
    { return Vector3(rotation()*p+translation()); }
    
    template<typename EigenDerived>
    typename EigenDerived::PlainObject
    actInvOnEigenObject(const Eigen::MatrixBase<EigenDerived> & p) const
    { return (rotation().transpose()*(p-translation())).eval(); }
    
    template<typename MapDerived>
    Vector3 actInvOnEigenObject(const Eigen::MapBase<MapDerived> & p) const
    { return Vector3(rotation().transpose()*(p-translation())); }
    
    Vector3 act_impl(const Vector3 & p) const
    { return Vector3(rotation()*p+translation()); }
    
    Vector3 actInv_impl(const Vector3 & p) const
    { return Vector3(rotation().transpose()*(p-translation())); }
    
    template<int O2>
    SE3Tpl act_impl(const SE3Tpl<Scalar,O2> & m2) const
    { return SE3Tpl(rot*m2.rotation()
                    ,translation()+rotation()*m2.translation());}
    
    template<int O2>
    SE3Tpl actInv_impl(const SE3Tpl<Scalar,O2> & m2) const
    { return SE3Tpl(rot.transpose()*m2.rotation(),
                    rot.transpose()*(m2.translation()-translation()));}
    
    template<int O2>
    SE3Tpl __mult__(const SE3Tpl<Scalar,O2> & m2) const
    { return this->act_impl(m2);}
    
    template<int O2>
    bool __equal__(const SE3Tpl<Scalar,O2> & m2) const
    {
      return (rotation() == m2.rotation() && translation() == m2.translation());
    }
    
    template<int O2>
    bool isApprox_impl(const SE3Tpl<Scalar,O2> & m2,
                       const Scalar & prec = Eigen::NumTraits<Scalar>::dummy_precision()) const
    {
      return rotation().isApprox(m2.rotation(), prec)
      && translation().isApprox(m2.translation(), prec);
    }
    
    bool isIdentity(const Scalar & prec = Eigen::NumTraits<Scalar>::dummy_precision()) const
    {
      return rotation().isIdentity(prec) && translation().isZero(prec);
    }
    
    ConstAngularRef rotation_impl() const { return rot; }
    AngularRef rotation_impl() { return rot; }
    void rotation_impl(const AngularType & R) { rot = R; }
    ConstLinearRef translation_impl() const { return trans;}
    LinearRef translation_impl() { return trans;}
    void translation_impl(const LinearType & p) { trans = p; }
    
    /// \returns An expression of *this with the Scalar type casted to NewScalar.
    template<typename NewScalar>
    SE3Tpl<NewScalar,Options> cast() const
    {
      typedef SE3Tpl<NewScalar,Options> ReturnType;
      ReturnType res(rot.template cast<NewScalar>(),
                     trans.template cast<NewScalar>());
      return res;
    }
    
  protected:
    AngularType rot;
    LinearType trans;
    
  }; // class SE3Tpl
  
} // namespace pinocchio

#endif // ifndef __pinocchio_se3_tpl_hpp__

