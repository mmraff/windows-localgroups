var addon = require('bindings')('localgroups')

// addon.path might prove useful, but we don't want to export it
for (var prop in addon) {
  if ('function' === typeof addon[prop])
    module.exports[prop] = addon[prop]
}

