from libcpp cimport bool
from libcpp.string cimport string

cdef extern from "Eigen/Core" namespace "Eigen":
    cdef cppclass Vector3f "Eigen::Vector3f":
        Vector3f()
        Vector3f(float, float, float)
        Vector3f(Vector3f)