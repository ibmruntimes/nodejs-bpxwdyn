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
      console.log("\n\n============This is expected to fail==========\n\n");
  })

  let ddname_allocated = await p1;
}

foo();
