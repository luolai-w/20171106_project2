module("Main",package.seeall)

local is_reload = ...

print('check reload',is_reload)
errlog('check reload',is_reload)
sg_print('check reload',is_reload)

package.path = package.path .. ";../scripts/?.lua"

package.loaded['Timer'] = nil

require('Timer')


errlog(tostring_r(package.loaded['Timer'],3))
