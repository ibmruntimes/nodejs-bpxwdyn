/*
 ** Licensed Materials - Property of IBM
 ** (C) Copyright IBM Corp. 2022. All Rights Reserved.
 ** US Government Users Restricted Rights - Use, duplication or disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 **/

const bpxwdyn = require("../build/Release/bpxwdyn.node");
const expect = require('chai').expect;
const should = require('chai').should;
const assert = require('chai').assert;

describe("nodejs-bpxwdyn Validation", function() {
  it("check nodejs-bpxwdyn allocate DOES.NOT.EXIST with diag(2) and msg(2)", function(done) {
    bpxwdyn.execute_rtvars(
      "alloc diag(2) shr msg(2) da('DOES.NOT.EXIST')",
      function(err, rc, ddname, dsname, volser) {
        console.log(
          "\n\n-----ALLOC DA('DOES.NOT.EXIST')---------------------");
        if (rc == 0) {
          console.log("ddname-received", ddname);
          console.log("dsname-received", dsname);
          console.log("volser-received", volser);
          console.log("FAIL")
          expect(false).to.be.true
          done()
        } else {
          console.log("error-string", err);
          console.log("SUCCESS")
          expect(true).to.be.true
          done()
        }
      });
  });
  it("check nodejs-bpxwdyn allocate SYS1.MACLIB(GETMAIN)", function(done) {
    bpxwdyn.execute_rtvars(
      "alloc shr msg(2) da('SYS1.MACLIB(GETMAIN)')",
      function(err, rc, ddname, dsname, volser) {
        console.log("\n\nALLOC SHR DA('SYS1.MACLIB(GETMAIN)' and free");
        if (rc == 0) {
          console.log("ddname-received", ddname);
          console.log("dsname-received", dsname);
          console.log("volser-received", volser);
          if (dsname === "SYS1.MACLIB") {
            bpxwdyn.execute("free dd(" + ddname + ")", function(err, rc) {
              console.log("\n\n----- FREE DD('" + ddname + "')-----");
              if (rc == 0) {
                console.log("FREE'd", ddname);
                console.log("SUCCESS")
                expect(true).to.be.true
                done()
              } else {
                console.log("error-string", err);
                console.log("FAIL")
                expect(false).to.be.true
                done()
              }
            });
          } else {
            console.log("error-string", err);
            console.log("FAIL")
            expect(false).to.be.true
            done()
          }
        } else {
          console.log("error-string", err);
          console.log("FAIL")
          expect(false).to.be.true
          done()
        }
      });
  });

});
