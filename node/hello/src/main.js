const addon = require('../build/Release/hello.node')

console.log(addon.hello())

console.log('0 = ', addon.sum())
console.log('1 = ', addon.sum(1))
console.log('1 + 2 = ', addon.sum(1, 2))
console.log('1 + 2 + 3 = ', addon.sum(1, 2, 3))
try {
    console.log('1 + "2" + 3 = ')
    addon.sum(1, '2', 3)
} catch (e) {
    if (e.message !== 'Expected number, but saw string') {
        throw e
    }
    console.error(e.message)
}

console.log('OK')
