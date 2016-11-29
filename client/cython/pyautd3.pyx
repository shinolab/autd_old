cimport autd3
from libcpp.memory cimport shared_ptr
from cython.operator cimport dereference as deref
cimport eigen

cdef class Pyautd:
	cdef autd3.Controller *_ctrler
	def __cinit__(self):
		self._ctrler = new autd3.Controller()

	def open(self, addr):
		#  ETHERCAT only
		self._ctrler.Open(autd3.ETHERCAT, addr)

	def add_device(self, position, rotation):
		cdef eigen.Vector3f *pos = new eigen.Vector3f(position[0],position[1],position[2])
		cdef eigen.Vector3f *rot = new eigen.Vector3f(rotation[0],rotation[1],rotation[2])
		return deref(self._ctrler.geometry()).AddDevice(deref(pos), deref(rot))

	def del_device(self, device_id):
		return deref(self._ctrler.geometry()).DelDevice(device_id)

	def focal_point(self, position):
		cdef eigen.Vector3f *pos = new eigen.Vector3f(position[0],position[1],position[2])
		cdef shared_ptr[autd3.Gain] g = autd3.FocalPointGain.Create(deref(pos))
		deref(self._ctrler).AppendGain(g)

	def stop(self):
		cdef shared_ptr[autd3.Gain] g = autd3.Gain.Create()
		deref(self._ctrler).AppendGain(g)