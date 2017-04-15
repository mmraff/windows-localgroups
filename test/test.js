var assert = require('assert')
  , os = require('os')
  , expect = require('chai').expect
  , mod = require('../')
  // This will be the results returned by mod.list(), used throughout the suite:
  , refList

if (process.platform !== 'win32') {
  console.error('This module is only meant for Windows platforms.\n' +
    'Aborting tests.\n');
  return
}

describe('windows-localgroups module', function() {
  it('should export functions: list, getComment, getMembers', function() {
    expect(mod.list).to.be.a('function')
    expect(mod.getComment).to.be.a('function')
    expect(mod.getMembers).to.be.a('function')
  })

var emptyArgs   = [ undefined, null, '', new String() ]
  , invalidArgs = [ 42, true, {}, [] ]
  , badName = "IWILLNOTCALLMYCOMPUTERANINAPPROPRIATENAME"
  , badGrpName = "Nobody Would Give A Group A Stupid Name Like This"

  , re_errNoGrpName = /Must provide name of local group/
  , re_errEmptyGrpName = /Group name cannot be empty/
  , re_errGrpNameType = /Invalid type for group name argument/
  , re_errArg1Type = /Invalid type for first argument/
  , re_errArg2Type = /Invalid type for second argument/
  , re_errNoCb = /Must provide callback when giving a hostname/
  , re_errNotFound = /(could not be found)|(does not exist)/
  , re_errHostnameAccess =
     /(access is denied)|(hostname is unrecognized)|(no RPC server available)/

function dummyFunc(err, data) {
  assert(false, 'This dummy function should never get called!')
}

describe('list() synchronous call', function() {

  // Here the reference data is collected, as a side effect of testing the
  // basic form of the function:
  before('should return an array when no arguments are given', function() {
    refList = mod.list()
    expect(refList).to.be.instanceof(Array)
    if (refList.length == 0) {
      console.warn(
        'NO LOCAL GROUPS DEFINED ON THIS SYSTEM!\n' +
        'NO MEANINGFUL TESTS CAN BE DONE, SO TEST SUITE WILL BE ABORTED.\n' +
        'SORRY!'
      );
      process.exit()
    }
  })

  it('results should contain only objects, ' +
     'each with string-value properties "name" and "comment"', function() {

    for (var i = 0; i < refList.length; i++) {
      var item = refList[i]
      expect(item).to.be.an('object')
      expect(item).to.have.property('name').that.is.a('string').that.is.not.empty
      expect(item).to.have.property('comment')
      if (item.comment !== null)
        expect(item.comment).to.be.a('string')
    }
  })

  it('results should be the same as for no-arg call when passed a single' +
     ' empty arg (undefined, null, empty string)', function() {

    emptyArgs.forEach(function(el) {
      expect(mod.list(el)).to.deep.equal(refList)
    })
    // This would fail if the order of elements returned by NetLocalGroupEnum
    // varied from one call to the next; but this has not been seen.
  })

  it('should throw an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.list(invalidArgs[i]) })
        .to.throw(Error, re_errArg1Type)
    }
  })

  it('should throw an exception when non-empty servername arg is given' +
     ' without passing a callback', function() {

    // Name doesn't matter - if given without callback, it is rejected
    expect(function(){ mod.list('BATCAVE') })
      .to.throw(Error, re_errNoCb)
  })
})

describe('list() asynchronous call', function() {

  it('should pass back an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.list(invalidArgs[i], dummyFunc) })
        .to.throw(Error, re_errArg1Type)
    }
  })

  it('callback results should be the same as for no-arg call when passed a' +
     ' callback, with or without empty arg preceding', function(done) {

    function nextTest(i) {
      if (emptyArgs.length <= i) return done()

      mod.list(emptyArgs[i], function(err, data) {
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

  it('should pass back an error when unknown hostname passed', function(done) {

    // Error passed back, and you're to blame - you give list() a badName
    mod.list(badName, function(err, data) {
      expect(err).to.be.an('error')
      expect(err.message)
        .to.match(re_errHostnameAccess)
      // NOTES:
      // * I have not yet seen access denial, but I expect there can be situations...
      // * 'No RPC Server' case can be when the host is disconnected from the network,
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

describe('getComment() synchronous call', function() {

  it('should throw an exception when no argument given', function() {
    expect(function(){ mod.getComment() }).to.throw(Error, re_errNoGrpName)
  })

  it('should throw an exception when passed a single empty value', function() {

    expect(function(){ mod.getComment(undefined) })
      .to.throw(Error, re_errNoGrpName)
    expect(function(){ mod.getComment(null) })
      .to.throw(Error, re_errNoGrpName)

    expect(function(){ mod.getComment('') })
      .to.throw(Error, re_errEmptyGrpName)
    expect(function(){ mod.getComment(new String()) })
      .to.throw(Error, re_errEmptyGrpName)
  })

  it('should throw an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getComment(invalidArgs[i]) })
        .to.throw(Error, re_errGrpNameType)
    }
  })

  it('should throw an exception when 2nd argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getComment(refList[0].name, invalidArgs[i]) })
        .to.throw(Error, re_errArg2Type)
    }
  })

  it('should throw an exception when non-empty hostname arg is given' +
     ' without passing a callback', function() {

    // Here we'll use a group name we know is valid; but that, and the hostname
    // used, doesn't matter - if hostname given without callback, it is rejected
    expect(function(){ mod.getComment(refList[0].name, 'BATCAVE') })
      .to.throw(Error, re_errNoCb)
  })

  it('should throw an exception when given an unknown group name', function() {

    expect(function(){ mod.getComment(badGrpName) })
      .to.throw(Error, re_errNotFound)
  })

  it('should return a matching comment for each group enumerated by list()', function() {

    for (var i = 0; i < refList.length; i++) {
      expect(mod.getComment(refList[i].name)).to.equal(refList[i].comment)
    }

    for (var i = 0; i < refList.length; i++) {
      emptyArgs.forEach(function(el) {
        expect(mod.getComment(refList[i].name, el)).to.equal(refList[i].comment)
      })
    }
  })

})

describe('getComment() asynchronous call', function() {

  it('should throw an exception when no group name given', function() {
    // NOTE: because there is a 1st argument (the callback) given,
    // the error is not the same as when there are no arguments given.
    expect(function(){ mod.getComment(dummyFunc) })
      .to.throw(Error, re_errGrpNameType)
  })

  it('should throw an exception when passed an empty arg for group name', function() {

    expect(function(){ mod.getComment(undefined, dummyFunc) })
      .to.throw(Error, re_errNoGrpName)
    expect(function(){ mod.getComment(null, dummyFunc) })
      .to.throw(Error, re_errNoGrpName)

    expect(function(){ mod.getComment('', dummyFunc) })
      .to.throw(Error, re_errEmptyGrpName)
    expect(function(){ mod.getComment(new String(), dummyFunc) })
      .to.throw(Error, re_errEmptyGrpName)
  })

  it('should throw an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getComment(invalidArgs[i], dummyFunc) })
        .to.throw(Error, re_errGrpNameType)
    }
  })

  it('should throw an exception when 2nd argument has invalid type', function() {

    var grpName0 = refList[0].name
    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getComment(grpName0, invalidArgs[i], dummyFunc) })
        .to.throw(Error, re_errArg2Type)
    }
  })

  it('should pass back an error when given an unknown group name', function(done) {

    mod.getComment(badGrpName, function(err1, data) {
      expect(err1).to.be.an('error')
      expect(err1.message).to.match(re_errNotFound)

      mod.getComment(badGrpName, '', function(err2, data) {
        expect(err2).to.be.an('error')
        expect(err2.message).to.match(re_errNotFound)

        mod.getComment(badGrpName, os.hostname(), function(err3, data) {
          expect(err3).to.be.an('error')
          expect(err3.message).to.match(re_errNotFound)
          done()
        })
      })
    })
  })

  it('should pass back an error when unknown hostname passed', function(done) {

    mod.getComment(refList[0].name, badName, function(err, data) {
      expect(err).to.be.an('error')
      expect(err.message).to.match(re_errHostnameAccess)
      done()
    })
  })

  it('should pass back a matching comment for each group enumerated by list()'
     + ' (given no hostname)', function(done) {

    function nextTest(i) {
      if (i >= refList.length) return done()

      mod.getComment(refList[i].name, function(err, data) {
        if (err) return done(err)
        expect(data).to.equal(refList[i].comment)
        return nextTest(i + 1)
      })
    }

    nextTest(0)
  })

  it('should pass back a matching comment for each group enumerated by list()'
     + ' (given null for hostname)', function(done) {

    function nextTestEmptyHostname(i) {
      if (i >= refList.length) return done()

      mod.getComment(refList[i].name, null, function(err, data) {
        if (err) return done(err)
        expect(data).to.equal(refList[i].comment)
        return nextTestEmptyHostname(i + 1)
      })
    }

    nextTestEmptyHostname(0)
  })

  it('should pass back a matching comment for each group enumerated by list()'
     + ' (given local hostname)', function(done) {

    function nextTestRealHostname(i) {
      if (i >= refList.length) return done()

      mod.getComment(refList[i].name, os.hostname(), function(err, data) {
        if (err) return done(err)
        expect(data).to.equal(refList[i].comment)
        return nextTestRealHostname(i + 1)
      })
    }

    nextTestRealHostname(0)
  })

})

describe('getMembers() synchronous call', function() {

  before('TODO?', function() {
  })

  it('should throw an exception when no argument given', function() {
    expect(function(){ mod.getMembers() }).to.throw(Error, re_errNoGrpName)
  })

  it('should throw an exception when passed a single empty value', function() {

    expect(function(){ mod.getMembers(undefined) })
      .to.throw(Error, re_errNoGrpName)
    expect(function(){ mod.getMembers(null) })
      .to.throw(Error, re_errNoGrpName)

    expect(function(){ mod.getMembers('') })
      .to.throw(Error, re_errEmptyGrpName)
    expect(function(){ mod.getMembers(new String()) })
      .to.throw(Error, re_errEmptyGrpName)
  })

  it('should throw an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getMembers(invalidArgs[i]) })
        .to.throw(Error, re_errGrpNameType)
    }
  })

  it('should throw an exception when 2nd argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getMembers(refList[0].name, invalidArgs[i]) })
        .to.throw(Error, re_errArg2Type)
    }
  })

  it('should throw an exception when non-empty hostname arg is given' +
     ' without passing a callback', function() {

    expect(function(){ mod.getMembers(refList[0].name, 'BATCAVE') })
      .to.throw(Error, re_errNoCb)
  })

  it('should throw an exception when given an unknown group name', function() {

    expect(function(){ mod.getMembers(badGrpName) })
      .to.throw(Error, re_errNotFound)
  })

  it('should return an array for each group enumerated by list()', function() {

    // Here we collect the member lists as a side effect of the test, so that
    // we retain the data for the next test
    for (var i = 0; i < refList.length; i++) {
      refList[i].members = mod.getMembers(refList[i].name)
      expect(refList[i].members).to.be.instanceof(Array)
    }
  })

  it('results should contain only objects, each with string-value properties' +
     ' "sid", "accountType" and "domainAndName"', function() {

    for (var i = 0; i < refList.length; i++) {
      var item = refList[i]
      for (var n = 0; n < item.members.length; n++) {
        var member = item.members[n]
        expect(member).to.be.an('object')
        expect(member).to.have.property('sid').that.is.a('string').that.is.not.empty
        expect(member).to.have.property('accountType').that.is.a('string').that.is.not.empty
        expect(member).to.have.property('domainAndName').that.is.a('string').that.is.not.empty
      }
    }
  })
})

describe('getMembers() asynchronous call', function() {

  it('should throw an exception when no group name given', function() {
    // NOTE: because there is a 1st argument (the callback) given,
    // the error is not the same as when there are no arguments given.
    expect(function(){ mod.getMembers(dummyFunc) })
      .to.throw(Error, re_errGrpNameType)
  })

  it('should throw an exception when passed an empty arg for group name', function() {

    expect(function(){ mod.getMembers(undefined, dummyFunc) })
      .to.throw(Error, re_errNoGrpName)
    expect(function(){ mod.getMembers(null, dummyFunc) })
      .to.throw(Error, re_errNoGrpName)

    expect(function(){ mod.getMembers('', dummyFunc) })
      .to.throw(Error, re_errEmptyGrpName)
    expect(function(){ mod.getMembers(new String(), dummyFunc) })
      .to.throw(Error, re_errEmptyGrpName)
  })

  it('should throw an exception when 1st argument has invalid type', function() {

    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getMembers(invalidArgs[i], dummyFunc) })
        .to.throw(Error, re_errGrpNameType)
    }
  })

  it('should throw an exception when 2nd argument has invalid type', function() {

    var grpName0 = refList[0].name
    for (var i = 0; i < invalidArgs.length; i++) {
      expect(function(){ mod.getMembers(grpName0, invalidArgs[i], dummyFunc) })
        .to.throw(Error, re_errArg2Type)
    }
  })

  it('should pass back an error when given an unknown group name', function(done) {

    mod.getMembers(badGrpName, function(err1, data) {
      expect(err1).to.be.an('error')
      expect(err1.message).to.match(re_errNotFound)

      mod.getMembers(badGrpName, '', function(err2, data) {
        expect(err2).to.be.an('error')
        expect(err2.message).to.match(re_errNotFound)

        mod.getMembers(badGrpName, os.hostname(), function(err3, data) {
          expect(err3).to.be.an('error')
          expect(err3.message).to.match(re_errNotFound)
          done()
        })
      })
    })
  })

  it('should pass back an error when unknown hostname passed', function(done) {

    mod.getMembers(refList[0].name, badName, function(err, data) {
      expect(err).to.be.an('error')
      expect(err.message).to.match(re_errHostnameAccess)
      done()
    })
  })

  function validateMemberData(list) {
    expect(list).to.be.instanceof(Array)

    for (var n = 0; n < list.length; n++) {
      expect(list[n]).to.be.an('object')
      expect(list[n]).to.have.property('sid').that.is.a('string').that.is.not.empty
      expect(list[n]).to.have.property('accountType').that.is.a('string').that.is.not.empty
      expect(list[n]).to.have.property('domainAndName').that.is.a('string').that.is.not.empty
    }
  }

  it('should pass back valid member data for each group enumerated by list()'
     + ' (given no hostname)', function(done) {

    function nextTest(i) {
      if (i >= refList.length) return done()

      mod.getMembers(refList[i].name, function(err, data) {
        if (err) return done(err)
        validateMemberData(data)
        return nextTest(i + 1)
      })
    }

    nextTest(0)
  })

  it('should pass back valid member data for each group enumerated by list()'
     + ' (given null for hostname)', function(done) {

    function nextTestEmptyHostname(i) {
      if (i >= refList.length) return done()

      mod.getMembers(refList[i].name, null, function(err, data) {
        if (err) return done(err)
        validateMemberData(data)
        return nextTestEmptyHostname(i + 1)
      })
    }

    nextTestEmptyHostname(0)
  })

  it('should pass back valid member data for each group enumerated by list()'
     + ' (given local hostname)', function(done) {

    function nextTestRealHostname(i) {
      if (i >= refList.length) return done()

      mod.getMembers(refList[i].name, os.hostname(), function(err, data) {
        if (err) return done(err)
        validateMemberData(data)
        return nextTestRealHostname(i + 1)
      })
    }

    nextTestRealHostname(0)
  })
})

})

