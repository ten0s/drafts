const assert = require('assert')
const types = require('../build/Release/types')

const tap = (x) => {
    console.log(x)
    return x
}

//
// PassNumber
//
assert.equal(types.passNumber(), 0) // explicite checked for argc

assert.ok(isNaN(types.passNumber(undefined)))
assert.ok(isNaN(types.passNumber({})))
assert.ok(isNaN(types.passNumber({x: 1})))
assert.ok(isNaN(types.passNumber('abc')))

assert.equal(types.passNumber(null), 42)

assert.equal(types.passNumber(10), 52)
assert.equal(types.passNumber(10.0), 52.0)

assert.equal(types.passNumber(''), 42)
assert.equal(types.passNumber('10'), 52)

//
// PassInt32
//
assert.equal(types.passInt32(undefined), 42)
assert.equal(types.passInt32({}), 42)
assert.equal(types.passInt32({x: 1}), 42)
assert.equal(types.passInt32('abc'), 42)

assert.equal(types.passInt32(null), 42)

assert.equal(types.passInt32(10), 52)
assert.equal(types.passInt32(10.0), 52)

assert.equal(types.passInt32(''), 42)
assert.equal(types.passInt32('10'), 52)

//
// PassBoolean
//
assert.ok(types.passBoolean(true))
assert.ok(! types.passBoolean(false))

assert.ok(! types.passBoolean(undefined))
assert.ok(! types.passBoolean(null))
assert.ok(! types.passBoolean(''))
assert.ok(! types.passBoolean(0))

assert.ok(types.passBoolean(1))
assert.ok(types.passBoolean({}))
assert.ok(types.passBoolean([]))

//
// PassString
//
assert.equal(types.passString(null), null)
assert.equal(types.passString(undefined), undefined)
assert.deepStrictEqual(types.passString({}), {})
assert.deepStrictEqual(types.passString({x: 1}), {x: 1})

assert.equal(types.passString(''), '')
assert.equal(types.passString('abc'), 'cba')

assert.equal(types.passString(10), '01')
assert.equal(types.passString(10.0), '01')
assert.equal(types.passString(10.1), '1.01')

//
// PassObject
//
assert.deepStrictEqual(types.passObject({x: 1}), {x: 1, y: 42})
assert.deepStrictEqual(types.passObject({x: 1, y: 2}), {x: 1, y: 44})
assert.deepStrictEqual(types.passObject({x: 1, y: "hello"}), {x: 1, y: NaN})
