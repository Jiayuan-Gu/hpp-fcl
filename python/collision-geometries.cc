//
// Software License Agreement (BSD License)
//
//  Copyright (c) 2019-2022 CNRS-LAAS INRIA
//  Author: Joseph Mirabel
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//   * Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//   * Redistributions in binary form must reproduce the above
//     copyright notice, this list of conditions and the following
//     disclaimer in the documentation and/or other materials provided
//     with the distribution.
//   * Neither the name of CNRS-LAAS. nor the names of its
//     contributors may be used to endorse or promote products derived
//     from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
//  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
//  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
//  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
//  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
//  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
//  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
//  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.

#include <eigenpy/eigenpy.hpp>
#include <eigenpy/eigen-to-python.hpp>

#include "fcl.hh"
#include "deprecation.hh"

#include <hpp/fcl/fwd.hh>
#include <hpp/fcl/shape/geometric_shapes.h>
#include <hpp/fcl/shape/convex.h>
#include <hpp/fcl/BVH/BVH_model.h>
#include <hpp/fcl/hfield.h>
#include <hpp/fcl/serialization/memory.h>
#include <hpp/fcl/serialization/BVH_model.h>

#ifdef HPP_FCL_HAS_DOXYGEN_AUTODOC
// FIXME for a reason I do not understand, doxygen fails to understand that
// BV_splitter is not defined in hpp/fcl/BVH/BVH_model.h
#include <hpp/fcl/internal/BV_splitter.h>
#include <hpp/fcl/broadphase/detail/hierarchy_tree.h>

#include "doxygen_autodoc/hpp/fcl/BVH/BVH_model.h"
#include "doxygen_autodoc/hpp/fcl/BV/AABB.h"
#include "doxygen_autodoc/hpp/fcl/hfield.h"
#include "doxygen_autodoc/hpp/fcl/shape/geometric_shapes.h"
#include "doxygen_autodoc/functions.h"
#endif

using namespace boost::python;
using namespace hpp::fcl;
namespace dv = doxygen::visitor;
namespace bp = boost::python;

using boost::noncopyable;

typedef std::vector<Vec3f> Vec3fs;
typedef std::vector<Triangle> Triangles;

struct BVHModelBaseWrapper
{
  typedef Eigen::Matrix<double,Eigen::Dynamic,3,Eigen::RowMajor> RowMatrixX3;
  typedef Eigen::Map<RowMatrixX3> MapRowMatrixX3;
  typedef Eigen::Ref<RowMatrixX3> RefRowMatrixX3;
  
  static Vec3f & vertex (BVHModelBase & bvh, unsigned int i)
  {
    if (i >= bvh.num_vertices) throw std::out_of_range("index is out of range");
    return bvh.vertices[i];
  }

  static RefRowMatrixX3 vertices(BVHModelBase & bvh)
  {
    return MapRowMatrixX3(bvh.vertices[0].data(),bvh.num_vertices,3);
  }
  
  static Triangle tri_indices (const BVHModelBase& bvh, unsigned int i)
  {
    if (i >= bvh.num_tris) throw std::out_of_range("index is out of range");
    return bvh.tri_indices[i];
  }
};

template <typename BV>
void exposeBVHModel (const std::string& bvname)
{
  typedef BVHModel<BV> BVH;

  const std::string type_name = "BVHModel" + bvname;
  class_ <BVH, bases<BVHModelBase>, shared_ptr<BVH> >
    (type_name.c_str(), doxygen::class_doc<BVH>(), no_init)
    .def (dv::init<BVH>())
    .def (dv::init<BVH, const BVH &>())
    .DEF_CLASS_FUNC(BVH,getNumBVs)
    .DEF_CLASS_FUNC(BVH,makeParentRelative)
    .DEF_CLASS_FUNC(BVHModelBase,memUsage)
    .def ("clone", &BVH::clone,
          doxygen::member_func_doc(&BVH::clone),
          return_value_policy<manage_new_object>())
    ;
}

template <typename BV>
void exposeHeightField (const std::string & bvname)
{
  typedef HeightField<BV> Geometry;
  typedef typename Geometry::Base Base;
  typedef typename Geometry::Node Node;

  const std::string type_name = "HeightField" + bvname;
  class_ <Geometry, bases<Base>, shared_ptr<Geometry> >
    (type_name.c_str(), doxygen::class_doc<Geometry>(), no_init)
    .def (dv::init< HeightField<BV> >())
    .def (dv::init< HeightField<BV> , const HeightField<BV> &>())
    .def (dv::init< HeightField<BV> , FCL_REAL, FCL_REAL, const MatrixXf &, bp::optional<FCL_REAL> >())
  
    .DEF_CLASS_FUNC(Geometry,getXDim)
    .DEF_CLASS_FUNC(Geometry,getYDim)
    .DEF_CLASS_FUNC(Geometry,getMinHeight)
    .DEF_CLASS_FUNC(Geometry,getMaxHeight)
    .DEF_CLASS_FUNC(Geometry,getNodeType)
    .DEF_CLASS_FUNC(Geometry,updateHeights)
  
    .def("clone", &Geometry::clone,
         doxygen::member_func_doc(&Geometry::clone),
         return_value_policy<manage_new_object>())
    .def("getXGrid", &Geometry::getXGrid,
         doxygen::member_func_doc(&Geometry::getXGrid),
         bp::return_value_policy<bp::copy_const_reference>())
    .def("getYGrid", &Geometry::getYGrid,
         doxygen::member_func_doc(&Geometry::getYGrid),
         bp::return_value_policy<bp::copy_const_reference>())
    .def("getHeights", &Geometry::getHeights,
         doxygen::member_func_doc(&Geometry::getHeights),
         bp::return_value_policy<bp::copy_const_reference>())
    .def("getBV", (Node & (Geometry::*)(unsigned int))&Geometry::getBV,
         doxygen::member_func_doc((Node & (Geometry::*)(unsigned int))&Geometry::getBV),
         bp::return_internal_reference<>())
    ;
}

struct ConvexBaseWrapper
{
  typedef Eigen::Matrix<double,Eigen::Dynamic,3,Eigen::RowMajor> RowMatrixX3;
  typedef Eigen::Map<RowMatrixX3> MapRowMatrixX3;
  typedef Eigen::Ref<RowMatrixX3> RefRowMatrixX3;
  
  static Vec3f & point (const ConvexBase& convex, unsigned int i)
  {
    if (i >= convex.num_points) throw std::out_of_range("index is out of range");
    return convex.points[i];
  }
  
  static RefRowMatrixX3 points (const ConvexBase& convex)
  {
    return MapRowMatrixX3(convex.points[0].data(),convex.num_points,3);
  }

  static list neighbors (const ConvexBase& convex, unsigned int i)
  {
    if (i >= convex.num_points) throw std::out_of_range("index is out of range");
    list n;
    for (unsigned char j = 0; j < convex.neighbors[i].count(); ++j)
      n.append (convex.neighbors[i][j]);
    return n;
  }

  static ConvexBase* convexHull (const Vec3fs& points, bool keepTri,
      const char* qhullCommand)
  {
    return ConvexBase::convexHull (points.data(), (unsigned int)points.size(), keepTri,
        qhullCommand);
  }
};

template <typename PolygonT>
struct ConvexWrapper
{
  typedef Convex<PolygonT> Convex_t;

  static PolygonT polygons (const Convex_t& convex, unsigned int i)
  {
    if (i >= convex.num_polygons) throw std::out_of_range("index is out of range");
    return convex.polygons[i];
  }

  static shared_ptr<Convex_t> constructor (const Vec3fs& _points, const Triangles& _tris)
  {
    Vec3f* points = new Vec3f[_points.size()];
    for (std::size_t i = 0; i < _points.size(); ++i) points[i] = _points[i];
    Triangle* tris = new Triangle[_tris.size()];
    for (std::size_t i = 0; i < _tris.size(); ++i) tris[i] = _tris[i];
    return shared_ptr<Convex_t>(new Convex_t(true, points, (unsigned int)_points.size(),
          tris, (unsigned int)_tris.size()));
  }
};


template<typename T>
void defComputeMemoryFootprint()
{
  doxygen::def("computeMemoryFootprint",&computeMemoryFootprint<T>);
}

void exposeComputeMemoryFootprint()
{
  defComputeMemoryFootprint<Sphere>();
  defComputeMemoryFootprint<Ellipsoid>();
  defComputeMemoryFootprint<Cone>();
  defComputeMemoryFootprint<Capsule>();
  defComputeMemoryFootprint<Cylinder>();
  defComputeMemoryFootprint<Box>();
  defComputeMemoryFootprint<Plane>();
  defComputeMemoryFootprint<Halfspace>();
  defComputeMemoryFootprint<TriangleP>();
  
  defComputeMemoryFootprint< BVHModel<OBB> >();
  defComputeMemoryFootprint< BVHModel<RSS> >();
  defComputeMemoryFootprint< BVHModel<OBBRSS> >();
}

void exposeShapes ()
{
  class_ <ShapeBase, bases<CollisionGeometry>, shared_ptr<ShapeBase>, noncopyable>
    ("ShapeBase", doxygen::class_doc<ShapeBase>(), no_init)
    //.def ("getObjectType", &CollisionGeometry::getObjectType)
    ;

  class_ <Box, bases<ShapeBase>, shared_ptr<Box> >
    ("Box", doxygen::class_doc<ShapeBase>(), no_init)
    .def (dv::init<Box>())
    .def (dv::init<Box, FCL_REAL,FCL_REAL,FCL_REAL>())
    .def (dv::init<Box, const Vec3f&>())
    .DEF_RW_CLASS_ATTRIB(Box,halfSide)
    .def ("clone", &Box::clone,
        doxygen::member_func_doc(&Box::clone),
        return_value_policy<manage_new_object>())
    ;

  class_ <Capsule, bases<ShapeBase>, shared_ptr<Capsule> >
    ("Capsule", doxygen::class_doc<Capsule>(), no_init)
    .def (dv::init<Capsule, FCL_REAL, FCL_REAL>())
    .DEF_RW_CLASS_ATTRIB (Capsule, radius)
    .DEF_RW_CLASS_ATTRIB (Capsule, halfLength)
    .def ("clone", &Capsule::clone,
      doxygen::member_func_doc(&Capsule::clone),
      return_value_policy<manage_new_object>())
    ;

  class_ <Cone, bases<ShapeBase>, shared_ptr<Cone> >
    ("Cone", doxygen::class_doc<Cone>(), no_init)
    .def (dv::init<Cone, FCL_REAL, FCL_REAL>())
    .DEF_RW_CLASS_ATTRIB (Cone, radius)
    .DEF_RW_CLASS_ATTRIB (Cone, halfLength)
    .def ("clone", &Cone::clone,
      doxygen::member_func_doc(&Cone::clone),
      return_value_policy<manage_new_object>())
    ;

  class_ <ConvexBase, bases<ShapeBase>, shared_ptr<ConvexBase >, noncopyable>
    ("ConvexBase", doxygen::class_doc<ConvexBase>(), no_init)
    .DEF_RO_CLASS_ATTRIB (ConvexBase, center)
    .DEF_RO_CLASS_ATTRIB (ConvexBase, num_points)
    .def ("point", &ConvexBaseWrapper::point,
          bp::args("self","index"),"Retrieve the point given by its index.",
          bp::return_internal_reference<>())
    .def ("points", &ConvexBaseWrapper::point,
          bp::args("self","index"),"Retrieve the point given by its index.",
          ::hpp::fcl::python::deprecated_member< bp::return_internal_reference<> >())
    .def ("points", &ConvexBaseWrapper::points,
          bp::args("self"),"Retrieve all the points.",
          bp::with_custodian_and_ward_postcall<0,1>())
  //    .add_property ("points",
  //                   bp::make_function(&ConvexBaseWrapper::points,bp::with_custodian_and_ward_postcall<0,1>()),
  //                   "Points of the convex.")
    .def ("neighbors", &ConvexBaseWrapper::neighbors)
    .def ("convexHull", &ConvexBaseWrapper::convexHull,
        doxygen::member_func_doc(&ConvexBase::convexHull),
        return_value_policy<manage_new_object>())
    .staticmethod("convexHull")
    .def ("clone", &ConvexBase::clone,
      doxygen::member_func_doc(&ConvexBase::clone),
      return_value_policy<manage_new_object>())
    ;

  class_ <Convex<Triangle>, bases<ConvexBase>, shared_ptr<Convex<Triangle> >, noncopyable>
    ("Convex", doxygen::class_doc< Convex<Triangle> >(), no_init)
    .def("__init__", make_constructor(&ConvexWrapper<Triangle>::constructor))
    .DEF_RO_CLASS_ATTRIB (Convex<Triangle>, num_polygons)
    .def ("polygons", &ConvexWrapper<Triangle>::polygons)
    ;

  class_ <Cylinder, bases<ShapeBase>, shared_ptr<Cylinder> >
    ("Cylinder", doxygen::class_doc<Cylinder>(), no_init)
    .def (dv::init<Cylinder, FCL_REAL, FCL_REAL>())
    .DEF_RW_CLASS_ATTRIB (Cylinder, radius)
    .DEF_RW_CLASS_ATTRIB (Cylinder, halfLength)
    .def ("clone", &Cylinder::clone,
          doxygen::member_func_doc(&Cylinder::clone),
          return_value_policy<manage_new_object>())
    ;

  class_ <Halfspace, bases<ShapeBase>, shared_ptr<Halfspace> >
    ("Halfspace", doxygen::class_doc<Halfspace>(), no_init)
    .def (dv::init<Halfspace, const Vec3f&, FCL_REAL>())
    .def (dv::init<Halfspace, FCL_REAL,FCL_REAL,FCL_REAL,FCL_REAL>())
    .def (dv::init<Halfspace>())
    .DEF_RW_CLASS_ATTRIB (Halfspace, n)
    .DEF_RW_CLASS_ATTRIB (Halfspace, d)
    .def ("clone", &Halfspace::clone,
          doxygen::member_func_doc(&Halfspace::clone),
          return_value_policy<manage_new_object>())
    ;

  class_ <Plane, bases<ShapeBase>, shared_ptr<Plane> >
    ("Plane", doxygen::class_doc<Plane>(), no_init)
    .def (dv::init<Plane, const Vec3f&, FCL_REAL>())
    .def (dv::init<Plane, FCL_REAL,FCL_REAL,FCL_REAL,FCL_REAL>())
    .def (dv::init<Plane>())
    .DEF_RW_CLASS_ATTRIB (Plane, n)
    .DEF_RW_CLASS_ATTRIB (Plane, d)
    .def ("clone", &Plane::clone,
          doxygen::member_func_doc(&Plane::clone),
          return_value_policy<manage_new_object>())
    ;

  class_ <Sphere, bases<ShapeBase>, shared_ptr<Sphere> >
    ("Sphere", doxygen::class_doc<Sphere>(), no_init)
    .def (dv::init<Sphere, FCL_REAL>())
    .DEF_RW_CLASS_ATTRIB (Sphere, radius)
    .def ("clone", &Sphere::clone,
          doxygen::member_func_doc(&Sphere::clone),
          return_value_policy<manage_new_object>())
    ;

  class_ <Ellipsoid, bases<ShapeBase>, shared_ptr<Ellipsoid> >
    ("Ellipsoid", doxygen::class_doc<Ellipsoid>(), no_init)
    .def (dv::init<Ellipsoid, FCL_REAL, FCL_REAL, FCL_REAL>())
    .DEF_RW_CLASS_ATTRIB (Ellipsoid, radii)
    .def ("clone", &Ellipsoid::clone,
          doxygen::member_func_doc(&Ellipsoid::clone),
          return_value_policy<manage_new_object>())
    ;

  class_ <TriangleP, bases<ShapeBase>, shared_ptr<TriangleP> >
    ("TriangleP", doxygen::class_doc<TriangleP>(), no_init)
    .def (dv::init<TriangleP, const Vec3f&, const Vec3f&, const Vec3f&>())
    .DEF_RW_CLASS_ATTRIB (TriangleP, a)
    .DEF_RW_CLASS_ATTRIB (TriangleP, b)
    .DEF_RW_CLASS_ATTRIB (TriangleP, c)
    .def ("clone", &TriangleP::clone,
          doxygen::member_func_doc(&TriangleP::clone),
          return_value_policy<manage_new_object>())
    ;

}

boost::python::tuple AABB_distance_proxy(const AABB & self, const AABB & other)
{
  Vec3f P,Q;
  FCL_REAL distance = self.distance(other,&P,&Q);
  return boost::python::make_tuple(distance,P,Q);
}

void exposeCollisionGeometries ()
{
  
  enum_<BVHModelType>("BVHModelType")
    .value ("BVH_MODEL_UNKNOWN"    , BVH_MODEL_UNKNOWN)
    .value ("BVH_MODEL_TRIANGLES"  , BVH_MODEL_TRIANGLES)
    .value ("BVH_MODEL_POINTCLOUD" , BVH_MODEL_POINTCLOUD)
    .export_values()
  ;
  
  enum_<BVHBuildState>("BVHBuildState")
  .value("BVH_BUILD_STATE_EMPTY",BVH_BUILD_STATE_EMPTY)
  .value("BVH_BUILD_STATE_BEGUN",BVH_BUILD_STATE_BEGUN)
  .value("BVH_BUILD_STATE_PROCESSED",BVH_BUILD_STATE_PROCESSED)
  .value("BVH_BUILD_STATE_UPDATE_BEGUN",BVH_BUILD_STATE_UPDATE_BEGUN)
  .value("BVH_BUILD_STATE_UPDATED",BVH_BUILD_STATE_UPDATED)
  .value("BVH_BUILD_STATE_REPLACE_BEGUN",BVH_BUILD_STATE_REPLACE_BEGUN)
  .export_values()
  ;
  
  if(!eigenpy::register_symbolic_link_to_registered_type<OBJECT_TYPE>())
  {
    enum_<OBJECT_TYPE>("OBJECT_TYPE")
      .value ("OT_UNKNOWN", OT_UNKNOWN)
      .value ("OT_BVH"    , OT_BVH)
      .value ("OT_GEOM"   , OT_GEOM)
      .value ("OT_OCTREE" , OT_OCTREE)
      .value ("OT_HFIELD" , OT_HFIELD)
      .export_values()
      ;
  }
  
  if(!eigenpy::register_symbolic_link_to_registered_type<NODE_TYPE>())
  {
    enum_<NODE_TYPE>("NODE_TYPE")
      .value ("BV_UNKNOWN", BV_UNKNOWN)
      .value ("BV_AABB"  , BV_AABB)
      .value ("BV_OBB"   , BV_OBB)
      .value ("BV_RSS"   , BV_RSS)
      .value ("BV_kIOS"  , BV_kIOS)
      .value ("BV_OBBRSS", BV_OBBRSS)
      .value ("BV_KDOP16", BV_KDOP16)
      .value ("BV_KDOP18", BV_KDOP18)
      .value ("BV_KDOP24", BV_KDOP24)
      .value ("GEOM_BOX"      , GEOM_BOX)
      .value ("GEOM_SPHERE"   , GEOM_SPHERE)
      .value ("GEOM_ELLIPSOID"   , GEOM_ELLIPSOID)
      .value ("GEOM_CAPSULE"  , GEOM_CAPSULE)
      .value ("GEOM_CONE"     , GEOM_CONE)
      .value ("GEOM_CYLINDER" , GEOM_CYLINDER)
      .value ("GEOM_CONVEX"   , GEOM_CONVEX)
      .value ("GEOM_PLANE"    , GEOM_PLANE)
      .value ("GEOM_HALFSPACE", GEOM_HALFSPACE)
      .value ("GEOM_TRIANGLE" , GEOM_TRIANGLE)
      .value ("GEOM_OCTREE"   , GEOM_OCTREE)
      .value ("HF_AABB"       , HF_AABB)
      .value ("HF_OBBRSS"     , HF_OBBRSS)
      .export_values()
      ;
  }
  
  class_<AABB>("AABB",
               "A class describing the AABB collision structure, which is a box in 3D space determined by two diagonal points",
               no_init)
    .def(init<>(bp::arg("self"), "Default constructor"))
    .def(init<Vec3f>(bp::args("self","v"),"Creating an AABB at position v with zero size."))
    .def(init<Vec3f,Vec3f>(bp::args("self","a","b"),"Creating an AABB with two endpoints a and b."))
    .def(init<AABB,Vec3f>(bp::args("self","core","delta"),"Creating an AABB centered as core and is of half-dimension delta."))
    .def(init<Vec3f,Vec3f,Vec3f>(bp::args("self","a","b","c"),"Creating an AABB contains three points."))
  
    .def("contain",
         (bool (AABB::*)(const Vec3f &) const)&AABB::contain,
         bp::args("self","p"),
         "Check whether the AABB contains a point p.")
    .def("contain",
         (bool (AABB::*)(const AABB &) const)&AABB::contain,
         bp::args("self","other"),
         "Check whether the AABB contains another AABB.")
  
    .def("overlap",
         (bool (AABB::*)(const AABB &) const)&AABB::overlap,
         bp::args("self","other"),
         "Check whether two AABB are overlap.")
    .def("overlap",
         (bool (AABB::*)(const AABB&, AABB&) const)&AABB::overlap,
         bp::args("self","other","overlapping_part"),
         "Check whether two AABB are overlaping and return the overloaping part if true.")
  
    .def("distance",
         (FCL_REAL (AABB::*)(const AABB &) const)&AABB::distance,
         bp::args("self","other"),
         "Distance between two AABBs.")
//    .def("distance",
//         AABB_distance_proxy,
//         bp::args("self","other"),
//         "Distance between two AABBs.")
  
    .def(bp::self + bp::self)
    .def(bp::self += bp::self)
    .def(bp::self += Vec3f())
  
    .def("size",&AABB::volume,bp::arg("self"),"Size of the AABB.")
    .def("center",&AABB::center,bp::arg("self"),"Center of the AABB.")
    .def("width",&AABB::width,bp::arg("self"),"Width of the AABB.")
    .def("height",&AABB::height,bp::arg("self"),"Height of the AABB.")
    .def("depth",&AABB::depth,bp::arg("self"),"Depth of the AABB.")
    .def("volume",&AABB::volume,bp::arg("self"),"Volume of the AABB.")
  
    .def("expand",
         static_cast<AABB& (AABB::*)(const AABB &, FCL_REAL)>(&AABB::expand),
//         doxygen::member_func_doc(static_cast<AABB& (AABB::*)(const AABB &, FCL_REAL)>(&AABB::expand)),
//         doxygen::member_func_args(static_cast<AABB& (AABB::*)(const AABB &, FCL_REAL)>(&AABB::expand)),
         bp::return_internal_reference<>())
    .def("expand",
         static_cast<AABB& (AABB::*)(const FCL_REAL)>(&AABB::expand),
//         doxygen::member_func_doc(static_cast<AABB& (AABB::*)(const FCL_REAL)>(&AABB::expand)),
//         doxygen::member_func_args(static_cast<AABB& (AABB::*)(const FCL_REAL)>(&AABB::expand)),
         bp::return_internal_reference<>())
    .def("expand",
         static_cast<AABB& (AABB::*)(const Vec3f &)>(&AABB::expand),
//         doxygen::member_func_doc(static_cast<AABB& (AABB::*)(const Vec3f &)>(&AABB::expand)),
//         doxygen::member_func_args(static_cast<AABB& (AABB::*)(const Vec3f &)>(&AABB::expand)),
         bp::return_internal_reference<>())
  ;
  
  def("translate",
      (AABB (*)(const AABB&, const Vec3f&))&translate,
      bp::args("aabb","t"),
      "Translate the center of AABB by t");
  
  def("rotate",
      (AABB (*)(const AABB&, const Matrix3f&))&rotate,
      bp::args("aabb","R"),
      "Rotate the AABB object by R");

  if(!eigenpy::register_symbolic_link_to_registered_type<CollisionGeometry>())
  {
    class_ <CollisionGeometry, CollisionGeometryPtr_t, noncopyable>
      ("CollisionGeometry", no_init)
      .def ("getObjectType", &CollisionGeometry::getObjectType)
      .def ("getNodeType", &CollisionGeometry::getNodeType)

      .def ("computeLocalAABB", &CollisionGeometry::computeLocalAABB)

      .def ("computeCOM", &CollisionGeometry::computeCOM)
      .def ("computeMomentofInertia", &CollisionGeometry::computeMomentofInertia)
      .def ("computeVolume", &CollisionGeometry::computeVolume)
      .def ("computeMomentofInertiaRelatedToCOM", &CollisionGeometry::computeMomentofInertiaRelatedToCOM)

      .def_readwrite("aabb_radius",&CollisionGeometry::aabb_radius,"AABB radius.")
      .def_readwrite("aabb_center",&CollisionGeometry::aabb_center,"AABB center in local coordinate.")
      .def_readwrite("aabb_local",&CollisionGeometry::aabb_local,"AABB in local coordinate, used for tight AABB when only translation transform.")
    
      .def("isOccupied",&CollisionGeometry::isOccupied,bp::arg("self"),"Whether the object is completely occupied.")
      .def("isFree",&CollisionGeometry::isFree,bp::arg("self"),"Whether the object is completely free.")
      .def("isUncertain",&CollisionGeometry::isUncertain,bp::arg("self"),"Whether the object has some uncertainty.")

      .def_readwrite("cost_density",&CollisionGeometry::cost_density,"Collision cost for unit volume.")
      .def_readwrite("threshold_occupied",&CollisionGeometry::threshold_occupied,"Threshold for occupied ( >= is occupied).")
      .def_readwrite("threshold_free",&CollisionGeometry::threshold_free,"Threshold for free (<= is free).")
      ;
  }

  exposeShapes();

  class_ <BVHModelBase, bases<CollisionGeometry>, BVHModelPtr_t, noncopyable>
    ("BVHModelBase", no_init)
    .def ("vertex", &BVHModelBaseWrapper::vertex,
          bp::args("self","index"),"Retrieve the vertex given by its index.",
          bp::return_internal_reference<>())
    .def ("vertices", &BVHModelBaseWrapper::vertex,
          bp::args("self","index"),"Retrieve the vertex given by its index.",
          ::hpp::fcl::python::deprecated_member< bp::return_internal_reference<> >())
    .def ("vertices", &BVHModelBaseWrapper::vertices,
          bp::args("self"),"Retrieve all the vertices.",
          bp::with_custodian_and_ward_postcall<0,1>())
//    .add_property ("vertices",
//                   bp::make_function(&BVHModelBaseWrapper::vertices,bp::with_custodian_and_ward_postcall<0,1>()),
//                   "Vertices of the BVH.")
    .def ("tri_indices", &BVHModelBaseWrapper::tri_indices,
          bp::args("self","index"),"Retrieve the triangle given by its index.")
    .def_readonly ("num_vertices", &BVHModelBase::num_vertices)
    .def_readonly ("num_tris", &BVHModelBase::num_tris)
    .def_readonly ("build_state", &BVHModelBase::build_state)

    .def_readonly ("convex", &BVHModelBase::convex)

    .DEF_CLASS_FUNC(BVHModelBase, buildConvexRepresentation)
    .DEF_CLASS_FUNC(BVHModelBase, buildConvexHull)

    // Expose function to build a BVH
    .def(dv::member_func("beginModel", &BVHModelBase::beginModel))
    .def(dv::member_func("addVertex", &BVHModelBase::addVertex))
    .def(dv::member_func("addVertices", &BVHModelBase::addVertices))
    .def(dv::member_func("addTriangle", &BVHModelBase::addTriangle))
    .def(dv::member_func("addTriangles", &BVHModelBase::addTriangles))
    .def(dv::member_func<int (BVHModelBase::*)(const Vec3fs&, const Triangles&)>("addSubModel", &BVHModelBase::addSubModel))
    .def(dv::member_func<int (BVHModelBase::*)(const Vec3fs&                  )>("addSubModel", &BVHModelBase::addSubModel))
    .def(dv::member_func("endModel", &BVHModelBase::endModel))
    // Expose function to replace a BVH
    .def(dv::member_func("beginReplaceModel", &BVHModelBase::beginReplaceModel))
    .def(dv::member_func("replaceVertex", &BVHModelBase::replaceVertex))
    .def(dv::member_func("replaceTriangle", &BVHModelBase::replaceTriangle))
    .def(dv::member_func("replaceSubModel", &BVHModelBase::replaceSubModel))
    .def(dv::member_func("endReplaceModel", &BVHModelBase::endReplaceModel))
    .def(dv::member_func("getModelType", &BVHModelBase::getModelType))
    ;
  exposeBVHModel<OBB    >("OBB"    );
  exposeBVHModel<OBBRSS >("OBBRSS" );
  exposeHeightField<OBBRSS>("OBBRSS");
  exposeHeightField<AABB>("AABB");
  exposeComputeMemoryFootprint();
}

void exposeCollisionObject ()
{
  namespace bp = boost::python;

  if(!eigenpy::register_symbolic_link_to_registered_type<CollisionObject>())
  {
    class_ <CollisionObject, CollisionObjectPtr_t>
      ("CollisionObject", no_init)
      .def (dv::init<CollisionObject, const CollisionGeometryPtr_t&, bp::optional<bool> >())
      .def (dv::init<CollisionObject, const CollisionGeometryPtr_t&, const Transform3f&, bp::optional<bool> >())
      .def (dv::init<CollisionObject, const CollisionGeometryPtr_t&, const Matrix3f&, const Vec3f&, bp::optional<bool> >())

      .DEF_CLASS_FUNC(CollisionObject, getObjectType)
      .DEF_CLASS_FUNC(CollisionObject, getNodeType)
      .DEF_CLASS_FUNC(CollisionObject, computeAABB)
      .def(dv::member_func("getAABB",
         static_cast<AABB & (CollisionObject::*)()>(&CollisionObject::getAABB), bp::return_internal_reference<>()))

      .DEF_CLASS_FUNC2(CollisionObject, getTranslation, bp::return_value_policy<bp::copy_const_reference>())
      .DEF_CLASS_FUNC(CollisionObject, setTranslation)
      .DEF_CLASS_FUNC2(CollisionObject, getRotation, bp::return_value_policy<bp::copy_const_reference>())
      .DEF_CLASS_FUNC(CollisionObject, setRotation)
      .DEF_CLASS_FUNC2(CollisionObject, getTransform, bp::return_value_policy<bp::copy_const_reference>())
      .def(dv::member_func("setTransform",
          static_cast<void (CollisionObject::*) (const Transform3f&)>(&CollisionObject::setTransform)))

      .DEF_CLASS_FUNC(CollisionObject, isIdentityTransform)
      .DEF_CLASS_FUNC(CollisionObject, setIdentityTransform)
      .DEF_CLASS_FUNC2(CollisionObject, setCollisionGeometry, (bp::with_custodian_and_ward_postcall<1,2>()))
      
      .def(dv::member_func("collisionGeometry",
          static_cast<const CollisionGeometryPtr_t& (CollisionObject::*) ()>(&CollisionObject::collisionGeometry),
          bp::return_value_policy<bp::copy_const_reference>()))
      ;
  }
}
