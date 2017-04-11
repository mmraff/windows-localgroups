var os = require('os')
  , expect = require('chai').expect
  , mod = require('../')

describe('windows-localgroups module', function() {
  it('should export functions: list, getComment, getMembers', function() {
    expect(mod.list).to.be.a('function')
    expect(mod.getComment).to.be.a('function')
    expect(mod.getMembers).to.be.a('function')
  })
})

// TODO:
// * tests for getComment, synchronous
// * tests for getComment, asynchronous
// * tests for getMembers, synchronous
// * tests for getMembers, asynchronous

var refList

describe('list() synchronous call', function() {

  // Here the reference data is collected, as a side effect of testing the
  // basic form of the function:
  it('should return an array when no arguments are given', function() {
    refList = mod.list()
    expect(refList).to.be.instanceof(Array)
  })

  it('results should contain only objects, ' +
     'each with string-value properties "name" and "comment"', function() {

    var list = refList
    for (var i = 0; i < list.length; i++) {
      expect(list[i]).to.be.an('object')
      expect(list[i]).to.have.property('name').that.is.a('string').that.is.not.empty
      expect(list[i]).to.have.property('comment')
      if (list[i].comment !== null)
        expect(list[i].comment).to.be.a('string')
    }
  })

  it('results should be the same as for no-arg call when passed a single' +
     ' empty arg (undefined, null, empty string)', function() {

    var firstArgs = [ undefined, null, '', new String() ]

    firstArgs.forEach(function(el) {
      expect(mod.list(el)).to.deep.equal(refList)
    })
    // This would fail if the order of elements returned by NetLocalGroupEnum
    // varied from one call to the next; but this has not been seen.
  })

  it('should throw an exception when first argument is invalid', function() {
    var firstArgs = [ 42, true, {}, [] ]
    firstArgs.forEach(function(el) {
      expect(function(){ mod.list(el) })
        .to.throw(Error, /Invalid type for first argument/)
    })
  })

  it('should throw an exception when non-empty servername arg is given' +
     ' without passing a callback', function() {
    expect(function(){ mod.list(os.hostname()) })
      .to.throw(Error, /Must provide callback when giving a hostname/)
  })
})

describe('list() asynchronous call', function() {

  it('callback results should be the same as for no-arg call when passed a' +
     ' callback, with or without empty arg preceding', function(done) {

    var firstArgs = [ undefined, null, '', new String() ]

    function nextTest(i) {
      if (firstArgs.length <= i) return done()

      mod.list(firstArgs[i], function(err, data) {
        if (err) return done(err)
        expect(data).to.deep.equal(refList)

        nextTest(i + 1)
      })
    }

    mod.list(function(err, data) { // The Kickoff
      if (err) return done(err)
      expect(data).to.deep.equal(refList)
      nextTest(0)
    })
  })

  it('should return an exception when unknown hostname passed', function(done) {

    var badName = "IWILLNOTCALLMYCOMPUTERANINAPPROPRIATENAME"
    // Error passed back, and you're to blame - you give list() a badName
    mod.list(badName, function(err, data) {
      expect(err).to.be.an('error')
      expect(err.message)
        .to.match(/(access is denied)|(hostname is unrecognized)|(no RPC server available)/)
      // NOTES:
      // * I have not yet seen access denial, but I expect there can be situations...
      // * No RPC Server case can be when the host is disconnected from the network,
      //   and not running such a server itself
      done()
    })
  })
  // It doesn't make sense to include any other tests with a hostname (other than
  //   the current host) because
  // a) This script doesn't know what other hosts are defined on an arbitrary
  //   local network
  // b) This script can't know in advance whether the query to another host will
  //   fail, even if its name is known.

})

