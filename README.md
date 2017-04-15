# windows-localgroups
A native addon for node.js to query local groups on Windows platforms

## Install
<pre>
C:\Users\myUser><b>npm install windows-localgroups</b>
</pre>

## Usage
```js
var localgroups = require('windows-localgroups')
```
For each of the three available functions, the user may expect the result of
querying the localhost as the synchronous return value *if no callback function
is added to the argument list*. Simply supply a callback function as the last
argument to turn the call asynchronous.  
A callback function is *required* if a host other than the localhost is to be
queried.  

**Note:** In some environments, the synchronous functions may throw an `Error` of
`"access is denied to current user"`, and any of the asynchronous functions may
pass this error to the callback. This would be a consequence of the user's
permissions versus the particular security configuration of the domain.

---
## API

### localgroups.list()
*Synchronous.* Returns a list of all local groups defined on the local system.
- Return: {Array} array of objects, each with two properties:  
  * **name** {String} Name of local group
  * **comment** {String | `null`} Descriptive comment

### localgroups.list([hostname,] callback)
*Asynchronous.* Passes a list of all local groups defined on the local system
(or, optionally, on the system named by `hostname`) to the `callback` function.
If `hostname` is given and it is unknown or cannot be accessed, an `Error` is
passed back.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Array} as described for return value of synchronous version,
  if no error.

### localgroups.getMembers(groupName)
*Synchronous.* Returns a list of member data for the named group if it is known
on the local system; otherwise an `Error` is thrown.
- `groupName` {String} Name of local group.
- Return: {Array} array of objects, each with three properties:  
  * **sid** {String} The unique security identifier (SID) of the member **TODO: MSDN link**
  * **accountType** {String} one of the following:  
    `"user"`, `"group"`, `"well-known group"`, `"deleted"`, `"unknown"`
  * **domainAndName** {String} The domain-qualified name of the member  
    (i.e. `"DomainName\\AccountName"`)

### localgroups.getMembers(groupName [, hostname], callback)
*Asynchronous.* Passes a list of all member data for the named group (optionally
on the system named by `hostname`) to the `callback` function, if the local group
name is known; otherwise an `Error` is passed to the callback. If `hostname` is
given and it is unknown or cannot be accessed, an `Error` is passed back.
- `groupName` {String} Name of local group.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {Array} as described for return value of synchronous version,
    if no error.

### localgroups.getComment(groupName)
*Synchronous.* Returns the descriptive comment (or `null` if none) for the named
group if it is known on the local system; otherwise an `Error` is thrown.
- `groupName` {String} Name of local group.
- Return: {String | `null`} The comment associated with the named group

### localgroups.getComment(groupName [, hostname], callback)
*Asynchronous.* Passes the descriptive comment (or `null` if none) for the named
group (optionally on the system named by `hostname`) to the `callback` function,
if the local group name is known; otherwise an `Error` is passed to the callback.
If `hostname` is given and it is unknown or cannot be accessed, an `Error` is
passed back.
- `groupName` {String} Name of local group.
- `hostname` {String} *Optional.*  
  The name of a host in the domain to query. An empty value (`undefined`, `null`,
  or empty string) may be passed to get the same effect as omitting this argument.
- `callback` {Function}  
  * **error** {Error | `null`}
  * **data** {String | `null`} The comment associated with the named group,
    if no error.

------

**License: MIT**

