const assert = require('assert')
const { Polynomial, add } = require('../build/Release/polynomial')

const p1 = new Polynomial(1,  3, 2)
console.log(p1)
const p2 =     Polynomial(8, -2, 4) // w/o new
console.log(p2)

assert.deepStrictEqual(p1.roots(), [-1, -2])
assert.equal(p1.at(4), 30)
p1.c = 0
assert.equal(p1.at(4), 28)

const p3 = add(p1, p2)
assert.equal(p3.a, 9)
assert.equal(p3.b, 1)
assert.equal(p3.c, 4) // due to p1.c = 0
