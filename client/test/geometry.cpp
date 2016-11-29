#include <gtest/gtest.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "autd3.hpp"

using namespace std;
using namespace Eigen;
using namespace autd;

float randf() {
    return 100.0*rand()/RAND_MAX;
}

// XXX: What is proper coefficient of epsilon?
inline void ASSERT_EQ_VECTOR3F(Vector3f a, Vector3f b) {
    ASSERT_LT((a-b).norm(), 3*FLT_EPSILON);
}

TEST(Geometry, DeviceOrigin) {
    GeometryPtr geo = Geometry::Create();
    geo->AddDevice(Vector3f(0,0,0), Vector3f(0,0,0));
    ASSERT_EQ_VECTOR3F(geo->position(0), Vector3f(0,0,0));
}

TEST(Geometry, DeviceRandomTranslation) {
    srand(NULL);
    auto pos = Vector3f(randf(),randf(),randf());
    GeometryPtr geo = Geometry::Create();
    geo->AddDevice(pos, Vector3f(0,0,0));
    ASSERT_EQ_VECTOR3F(geo->position(0), pos);
}

TEST(Geometry, DeviceDirection0) {
    srand(NULL);
    auto rot = Vector3f(M_PI_2, M_PI_2, 0);
    GeometryPtr geo = Geometry::Create();
    geo->AddDevice(Vector3f::Zero(), rot);
    ASSERT_EQ_VECTOR3F(geo->direction(0), Vector3f(0, 1, 0));
    ASSERT_EQ_VECTOR3F(geo->direction(1), Vector3f(0, 1, 0));
}

TEST(Geometry, DeviceDirection1) {
    srand(NULL);
    auto rot = Vector3f(M_PI_4, M_PI_4, M_PI_4);
    GeometryPtr geo = Geometry::Create();
    geo->AddDevice(Vector3f::Zero(), rot);
    auto ref = Vector3f(0.5, 0.5, sqrt(2.0f)/2).normalized();
    ASSERT_EQ_VECTOR3F(geo->direction(0), ref);
    ASSERT_EQ_VECTOR3F(geo->direction(1), ref);
}