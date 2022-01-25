const rainfall = require('../build/Release/rainfall')
const assert = require('assert')

const location = {
    latitude: 40.71, longitude: -74.01,
    samples: [
        { date: "2016-06-07", rainfall: 2.1 },
        { date: "2016-08-12", rainfall: 0.5 },
        { date: "2015-06-21", rainfall: 1.5 },
        { date: "2015-06-28", rainfall: 1.3 },
        { date: "2015-07-05", rainfall: 0.9 }
    ]
}

const avg = rainfall.avgRainfall(location)
assert.equal(avg, 1.26)
console.log(`Avg rain fall = ${avg}cm`)

const data = rainfall.dataRainfall(location)
console.log(`Mean = ${data.mean.toFixed(2)}`)
console.log(`Median = ${data.median.toFixed(2)}`)
console.log(`StDev = ${data.stdev.toFixed(2)}`)
console.log(`N = ${data.n}`)
console.log()

// carried func
const printResult = locations => (result, i) => {
    console.log(`Result for Location ${i}`)
    console.log('--------------------------')
    console.log(`\tLatitude: ${locations[i].latitude.toFixed(2)}`)
    console.log(`\tLongitude: ${locations[i].longitude.toFixed(2)}`)
    console.log(`\tMean Rainfall: ${result.mean.toFixed(2)}cm`)
    console.log(`\tMedian Rainfall: ${result.median.toFixed(2)}cm`)
    console.log(`\tStDev Rainfall: ${result.stdev.toFixed(2)}cm`)
    console.log(`\tNumber Samples: ${result.n}`)
}

const locations = [location]

console.log('+ calcResults')
const results = rainfall.calcResults(locations)
results.forEach(printResult(locations))
console.log('- calcResults')
console.log()

console.log('+ calcResultsSync')
rainfall.calcResultsSync(locations, (results) => {
    results.forEach(printResult(locations))
})
console.log('- calcResultsSync')
console.log()

console.log('+ calcResultsAsync')
rainfall.calcResultsAsync(locations, (err, results) => {
    if (err) {
        console.error(err)
        return;
    }
    results.forEach(printResult(locations))
})
console.log('- calcResultsAsync')
console.log()
