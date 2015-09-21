//
// Copyright (c) 2015 CNRS
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

#ifndef __se3_joint_base_hpp__
#define __se3_joint_base_hpp__

#include "pinocchio/spatial/fwd.hpp"
#include "pinocchio/spatial/motion.hpp"
#include "pinocchio/multibody/constraint.hpp"
#include <Eigen/Geometry>
#include <Eigen/StdVector>
#include <boost/variant.hpp>
#include <limits>

namespace se3
{
  // template<class C> struct traits {};

  /* RNEA operations
   *
   * *** FORWARD ***
   * J::calc(q,vq)
   * SE3    = SE3 * J::SE3
   * Motion = J::Motion
   * Motion = J::Constraint*J::JointMotion + J::Bias + Motion^J::Motion
   * Force  = Inertia*Motion  + Inertia.vxiv(Motion)
   *
   * *** BACKWARD *** 
   * J::JointForce = J::Constraint::Transpose*J::Force
   */

  /* CRBA operations
   *
   * *** FORWARD ***
   * J::calc(q)
   * Inertia = Inertia
   *
   * *** BACKWARD *** 
   * Inertia += SE3::act(Inertia)
   * F = Inertia*J::Constraint
   * JointInertia.block = J::Constraint::Transpose*F
   * *** *** INNER ***
   *     F = SE3::act(f)
   *     JointInertia::block = J::Constraint::Transpose*F
   */

  /* Jacobian operations
   *
   * internal::ActionReturn<Constraint>::Type
   * Constraint::se3Action
   */
#ifdef __clang__

#define SE3_JOINT_TYPEDEF_ARG(prefix)					     \
  typedef int Index;						     \
  typedef prefix traits<Joint>::JointData JointData;		     \
  typedef prefix traits<Joint>::JointModel JointModel;	     \
  typedef prefix traits<Joint>::Constraint_t Constraint_t;	     \
  typedef prefix traits<Joint>::Transformation_t Transformation_t; \
  typedef prefix traits<Joint>::Motion_t Motion_t;		     \
  typedef prefix traits<Joint>::Bias_t Bias_t;		     \
  typedef prefix traits<Joint>::F_t F_t;			     \
  enum {							     \
    NQ = traits<Joint>::NQ,					     \
    NV = traits<Joint>::NV					     \
  }

#define SE3_JOINT_TYPEDEF SE3_JOINT_TYPEDEF_ARG()
#define SE3_JOINT_TYPEDEF_TEMPLATE SE3_JOINT_TYPEDEF_ARG(typename)

#elif (__GNUC__ == 4) && (__GNUC_MINOR__ == 4) && (__GNUC_PATCHLEVEL__ == 2)

#define SE3_JOINT_TYPEDEF_NOARG()				\
  typedef int Index;						\
  typedef traits<Joint>::JointData JointData;			\
  typedef traits<Joint>::JointModel JointModel;			\
  typedef traits<Joint>::Constraint_t Constraint_t;		\
  typedef traits<Joint>::Transformation_t Transformation_t;	\
  typedef traits<Joint>::Motion_t Motion_t;			\
  typedef traits<Joint>::Bias_t Bias_t;				\
  typedef traits<Joint>::F_t F_t;				\
  enum {							\
    NQ = traits<Joint>::NQ,					\
    NV = traits<Joint>::NV					\
  }

#define SE3_JOINT_TYPEDEF_ARG(prefix)					\
  typedef int Index;							\
  typedef prefix traits<Joint>::JointData JointData;			\
  typedef prefix traits<Joint>::JointModel JointModel;			\
  typedef prefix traits<Joint>::Constraint_t Constraint_t;		\
  typedef prefix traits<Joint>::Transformation_t Transformation_t;	\
  typedef prefix traits<Joint>::Motion_t Motion_t;			\
  typedef prefix traits<Joint>::Bias_t Bias_t;				\
  typedef prefix traits<Joint>::F_t F_t;				\
  enum {								\
    NQ = traits<Joint>::NQ,						\
    NV = traits<Joint>::NV						\
  }

#define SE3_JOINT_TYPEDEF SE3_JOINT_TYPEDEF_NOARG()
#define SE3_JOINT_TYPEDEF_TEMPLATE SE3_JOINT_TYPEDEF_ARG(typename)

#else

#define SE3_JOINT_TYPEDEF_ARG()              \
  typedef int Index;                 \
  typedef typename traits<Joint>::JointData JointData;         \
  typedef typename traits<Joint>::JointModel JointModel;       \
  typedef typename traits<Joint>::Constraint_t Constraint_t;       \
  typedef typename traits<Joint>::Transformation_t Transformation_t; \
  typedef typename traits<Joint>::Motion_t Motion_t;         \
  typedef typename traits<Joint>::Bias_t Bias_t;         \
  typedef typename traits<Joint>::F_t F_t;           \
  enum {                   \
    NQ = traits<Joint>::NQ,              \
    NV = traits<Joint>::NV               \
  }

#define SE3_JOINT_TYPEDEF SE3_JOINT_TYPEDEF_ARG()
#define SE3_JOINT_TYPEDEF_TEMPLATE SE3_JOINT_TYPEDEF_ARG()

#endif

#define SE3_JOINT_USE_INDEXES \
    typedef JointModelBase<JointModel> Base; \
    using Base::idx_q; \
    using Base::idx_v

  template<typename _JointData>
  struct JointDataBase
  {
    typedef typename traits<_JointData>::Joint Joint;
    SE3_JOINT_TYPEDEF_TEMPLATE;

    JointData& derived() { return *static_cast<JointData*>(this); }
    const JointData& derived() const { return *static_cast<const JointData*>(this); }

    const Constraint_t     & S() const  { return static_cast<const JointData*>(this)->S;   }
    const Transformation_t & M() const  { return static_cast<const JointData*>(this)->M;   }
    const Motion_t         & v() const  { return static_cast<const JointData*>(this)->v;   }
    const Bias_t           & c() const  { return static_cast<const JointData*>(this)->c;   }
    F_t& F()        { return static_cast<      JointData*>(this)->F; }
  };

  template<typename _JointModel>
  struct JointModelBase
  {
    typedef typename traits<_JointModel>::Joint Joint;
    SE3_JOINT_TYPEDEF_TEMPLATE;

    JointModel& derived() { return *static_cast<JointModel*>(this); }
    const JointModel& derived() const { return *static_cast<const JointModel*>(this); }

    JointData createData() const { return static_cast<const JointModel*>(this)->createData(); }
    void calc( JointData& data, 
	       const Eigen::VectorXd & qs ) const
    { return static_cast<const JointModel*>(this)->calc(data,qs); }
    void calc( JointData& data, 
	       const Eigen::VectorXd & qs, 
	       const Eigen::VectorXd & vs ) const
    { return static_cast<const JointModel*>(this)->calc(data,qs,vs); }

  private:
    Index i_id; // ID of the joint in the multibody list.
    int i_q;    // Index of the joint configuration in the joint configuration vector.
    int i_v;    // Index of the joint velocity in the joint velocity vector.

    Eigen::Matrix<double,NQ,1> position_lower;
    Eigen::Matrix<double,NQ,1> position_upper;

    Eigen::Matrix<double,NQ,1> effortMax;
    Eigen::Matrix<double,NV,1> velocityMax;

  public:
          int     nv()    const { return NV; }
          int     nq()    const { return NQ; }
    const int &   idx_q() const { return i_q; }
    const int &   idx_v() const { return i_v; }
    const Index & id()    const { return i_id; }

    const Eigen::Matrix<double,NQ,1> & lowerPosLimit() const { return position_lower;}
    const Eigen::Matrix<double,NQ,1> & upperPosLimit() const { return position_upper;}

    const Eigen::Matrix<double,NQ,1> & maxEffortLimit() const { return effortMax;}
    const Eigen::Matrix<double,NV,1> & maxVelocityLimit() const { return velocityMax;}


    void setIndexes(Index id,int q,int v) { i_id = id, i_q = q; i_v = v; }

    void setLowerPositionLimit(const Eigen::VectorXd & lowerPos)
    {
      if (lowerPos.rows() == NQ)
        position_lower = lowerPos;
      else
        position_lower.fill(-std::numeric_limits<double>::infinity());
    }

    void setUpperPositionLimit(const Eigen::VectorXd & upperPos)
    {
      if (upperPos.rows() == NQ)
        position_upper = upperPos;
      else
        position_upper.fill(std::numeric_limits<double>::infinity());
    }

    void setMaxEffortLimit(const Eigen::VectorXd & effort)
    {
      if (effort.rows() == NQ)
        effortMax = effort;
      else
        effortMax.fill(std::numeric_limits<double>::infinity());
    }

    void setMaxVelocityLimit(const Eigen::VectorXd & v)
    {
      if (v.rows() == NV)
        velocityMax = v;
      else
        velocityMax.fill(std::numeric_limits<double>::infinity());
    }


    template<typename D>
    typename D::template ConstFixedSegmentReturnType<NV>::Type jointMotion(const Eigen::MatrixBase<D>& a) const     { return a.template segment<NV>(i_v); }
    template<typename D>
    typename D::template FixedSegmentReturnType<NV>::Type jointMotion(Eigen::MatrixBase<D>& a) const 
    { return a.template segment<NV>(i_v); }
    template<typename D>
    typename D::template ConstFixedSegmentReturnType<NV>::Type jointForce(const Eigen::MatrixBase<D>& tau) const 
    { return tau.template segment<NV>(i_v); }
    template<typename D>
    typename D::template FixedSegmentReturnType<NV>::Type jointForce(Eigen::MatrixBase<D>& tau) const 
    { return tau.template segment<NV>(i_v); }

    template<typename D>
    typename D::template ConstFixedSegmentReturnType<NQ>::Type jointLimit(const Eigen::MatrixBase<D>& limit) const 
    { return limit.template segment<NQ>(i_q); }
    template<typename D>
    typename D::template FixedSegmentReturnType<NQ>::Type jointLimit(Eigen::MatrixBase<D>& limit) const 
    { return limit.template segment<NQ>(i_q); }

    template<typename D>
    typename D::template ConstFixedSegmentReturnType<NV>::Type jointTangentLimit(const Eigen::MatrixBase<D>& limit) const 
    { return limit.template segment<NV>(i_v); }
    template<typename D>
    typename D::template FixedSegmentReturnType<NV>::Type jointTangentLimit(Eigen::MatrixBase<D>& limit) const 
    { return limit.template segment<NV>(i_v); }

  };

} // namespace se3

#endif // ifndef __se3_joint_base_hpp__