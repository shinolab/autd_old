from pyautd3 import Pyautd 

ctl = Pyautd()
#ctl.open("127.0.0.1")
ctl.add_device([0,0,0], [0,0,0])
ctl.focal_point([0,0,0])
ctl.stop()