const addon = require('../build/Release/basic')

try {
    console.log('add(1)')
    addon.add(1)
} catch (e) {
    if (e.message !== 'Wrong number of arguments') {
        throw e
    }
    console.log(e.message)
}
try {
    console.log('add(1, "2")')
    addon.add(1, "2")
} catch (e) {
    if (e.message !== 'Wrong arguments') {
        throw e
    }
    console.log(e.message)
}
console.log('1 + 2 = ', addon.add(1, 2));

console.log('sum() = ', addon.sum())
console.log('sum(1) = ', addon.sum(1))
console.log('sum(1, 2) = ', addon.sum(1, 2))
console.log('sum(1, 2, 3) = ', addon.sum(1, 2, 3))
try {
    addon.sum(1, '2', 3)
} catch (e) {
    if (e.message !== 'Expected number, but saw string') {
        console.log('1 + "2" + 3 = ')
        throw e
    }
}

console.log(addon.callFun0(() => {
    console.log('I\'ve been called!')
}))

console.log(addon.callFun1((n) => 1 + n, 10))

console.log('OK')
