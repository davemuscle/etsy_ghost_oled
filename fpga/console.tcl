#From Alteras System Console Guide, Parallax forums
#Assumes the device is already programmed and ready
set masters [get_service_paths master]
set master [lindex $masters 0]
open_service master $master
puts $master


