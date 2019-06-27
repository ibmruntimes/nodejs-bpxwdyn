# nodejs-bpxwdyn
Node.js interface to the BPXWDYN program (svc 99 service with text commands)

## Installation

<!--
This is a [Node.js](https://nodejs.org/en/) module available through the
[npm registry](https://www.npmjs.com/).
-->

Before installing, [download and install Node.js](https://developer.ibm.com/node/sdk/ztp/).
Node.js 8.16 for z/OS or higher is required.

### Install

```bash
npm install nodejs-bpxwdyn
```

### Use

#### bpxwdyn.execute_rtvars

##### Arguments:
1. Command to BPXWDYN [Reference 1](#ref1)
2. Second argument, is the call-back function for asynchronous call.

##### Returns:
This function requests BPXWDYN to return DDNAME, DSNAME and VOLSER
The callback function receive five return objects, they 
are respectively:
 
1. Error String, if Return code is not 0 
2. Return code
3. DDNAME if Return code is 0
4. DSNAME if Return code is 0
5. VOLSER if Return code is 0

#### bpxwdyn.execute

##### Arguments:
1. Command to BPXWDYN [Reference 1](#ref1)
2. Second argument, is the call-back function for asynchronous call.

##### Returns:
This function does not requests BPXWDYN to return variables
The callback function receive two return objects, they 
are respectively:
 
1. Error String, if Return code is not 0 
2. Return code

### Example:
```js


const bpxwdyn = require('nodejs-bpxwdyn');

async function foo() {

  console.log("\n\n------------------------------------------------------TESTING  alloc shr msg(2) da('SYS1.MACLIB(GETMAIN)')");
  let p1 = new Promise((resolve, reject) => {
    bpxwdyn.execute_rtvars(
        "alloc shr msg(2) da('SYS1.MACLIB(GETMAIN)')",
        function(err, rc, ddname, dsname, volser) {
          console.log(
              "\n\n----- ALLOC SHR DA('SYS1.MACLIB(GETMAIN)') ---------");
          if (rc == 0) {
            console.log("ddname-received", ddname);
            console.log("dsname-received", dsname);
            console.log("volser-received", volser);
            resolve(ddname);
          } else {
            console.log("error-string", err);
            reject("allocate failed");
          }
        });
  }).catch((error) => {
      console.log("Error", error);
      return;
  })


  let ddname_allocated = await p1;

  console.log("\n\n------------------------------------------------------TESTING  free dd(" + ddname_allocated + ")");
  let p2 = new Promise((resolve, reject) => {
    bpxwdyn.execute("free dd(" + ddname_allocated + ")", function(err, rc) {
      console.log("\n\n----- FREE DD('" + ddname_allocated + "')-----");
      if (rc == 0) {
        resolve("OK");
      } else {
        console.log("error-string", err);
        reject("free failed");
      }
    });
  }).catch((error) => {
      console.log("Error", error);
      return;
  })

  let freed = await p2;

  console.log("\n\n------------------------------------------------------TESTING alloc diag(2) shr msg(2) da('DOES.NOT.EXIST')" );
  bar()
}

async function bar() {

  let p1 = new Promise((resolve, reject) => {
    bpxwdyn.execute_rtvars(
        "alloc diag(2) shr msg(2) da('DOES.NOT.EXIST')",
        function(err, rc, ddname, dsname, volser) {
          console.log(
              "\n\n-----ALLOC DA('DOES.NOT.EXIST')---------------------");
          if (rc == 0) {
            console.log("ddname-received", ddname);
            console.log("dsname-received", dsname);
            console.log("volser-received", volser);
            resolve(ddname);
          } else {
            console.log("error-string", err);
            reject("allocate failed");
          }
        });
  }).catch((error) => {
      console.log("Error", error);
  })

  let ddname_allocated = await p1;
}

foo();


```

### Test

```bash

node test.js
```

#### reference
<a name='ref1'></a>[BPXWDYN: a text interface to dynamic allocation](https://www.ibm.com/support/knowledgecenter/en/SSLTBW_2.3.0/com.ibm.zos.v2r3.bpxb600/wdyn.htm)
