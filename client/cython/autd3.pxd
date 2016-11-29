from libcpp cimport bool
from libcpp.string cimport string
from libcpp.memory cimport shared_ptr
from cython.operator cimport dereference
cimport eigen

cdef extern from "autd3.hpp" namespace "autd":
    ctypedef enum LinkType:
        ETHERCAT
    cdef cppclass Geometry "autd::Geometry":
        Geomatry()
        int AddDevice(eigen.Vector3f, eigen.Vector3f)
        void DelDevice(int)
    cdef cppclass Gain "autd::Gain":
        @staticmethod
        shared_ptr[Gain] Create()
    cdef cppclass FocalPointGain "autd::FocalPointGain":
        @staticmethod
        shared_ptr[Gain] Create(eigen.Vector3f)
    cdef cppclass Modulation "autd::Modulation":
        @staticmethod
        shared_ptr[Modulation] Create(int)
    cdef cppclass Modulation "autd::SineModulation":
        @staticmethod
        shared_ptr[Modulation] Create(float)
    cdef cppclass Controller "autd::Controller":
        Controller()
        void Open(LinkType type, string location)
        bool isOpen()
        shared_ptr[Geometry] geometry()
        void AppendGain(shared_ptr[Gain])
        void AppendModulation(shared_ptr[Modulation])

