"use strict";

var exact = require("../src/exact-float.js").exact;
var BigInteger = require("../lib/javascript-bignum/biginteger.js").BigInteger;

var compareBigInteger = function(first, second) {
    if (first instanceof BigInteger)
        return first.compare(second) == 0;
    return undefined;
};

describe("A FloatInfo object", function() {
    beforeEach(function() {
        jasmine.addCustomEqualityTester(compareBigInteger);
    });

    it("can be constructed", function() {
        expect(exact.FloatInfo).toBeDefined();
    });

    function constructFloatInfo(value, bits) {
        return function() {
            exact.FloatInfo(value, bits);
        };
    }

    describe("when constructed with a hex string", function() {
        it("does not require the bit-count parameter", function() {
            expect(constructFloatInfo("0x00000000")).not.toThrow();
        });

        it("accepts only certain bit counts", function() {
            expect(constructFloatInfo("0x000000000")).toThrow();  // 36
            expect(constructFloatInfo("0x00000000")).not.toThrow();  // 32
            expect(constructFloatInfo("0x0000000")).toThrow();  // 28

            expect(constructFloatInfo("0x0000000000000000")).not.toThrow();  // 64
            expect(constructFloatInfo("0x00000000000000000000")).not.toThrow();  // 80
            expect(constructFloatInfo("0x00000000000000000000000000")).toThrow();  // 92
            expect(constructFloatInfo("0x00000000000000000000000000000000")).toThrow(); // 128
        });

        it("checks that the bit count matches the string", function() {
            expect(constructFloatInfo("0x00000000", 31)).toThrow();
            expect(constructFloatInfo("0x00000000", 32)).not.toThrow();
            expect(constructFloatInfo("0x00000000", 33)).toThrow();
            expect(constructFloatInfo("0x0000000000000000", 64)).not.toThrow();
            expect(constructFloatInfo("0x00000000000000000000", 80)).not.toThrow();
        });

        it("detects the sign of the input", function() {
            expect(exact.FloatInfo("0x00000000000000000000").negative).toBe(false);
            expect(exact.FloatInfo("0x80000000000000000000").negative).toBe(true);
            expect(exact.FloatInfo("0x7fff0000000000000000").negative).toBe(false);
            expect(exact.FloatInfo("0xffff0000000000000000").negative).toBe(true);
            expect(exact.FloatInfo("0x3333ffffffffffffffff").negative).toBe(false);
            expect(exact.FloatInfo("0xb333ffffffffffffffff").negative).toBe(true);
            expect(exact.FloatInfo("0x0000000000000000").negative).toBe(false);
            expect(exact.FloatInfo("0x8000000000000000").negative).toBe(true);
            expect(exact.FloatInfo("0x7770000000000000").negative).toBe(false);
            expect(exact.FloatInfo("0xf770000000000000").negative).toBe(true);
            expect(exact.FloatInfo("0x001fffffffffffff").negative).toBe(false);
            expect(exact.FloatInfo("0x801fffffffffffff").negative).toBe(true);
            expect(exact.FloatInfo("0x00000000").negative).toBe(false);
            expect(exact.FloatInfo("0x80000000").negative).toBe(true);
            expect(exact.FloatInfo("0x7fbfffff").negative).toBe(false);
            expect(exact.FloatInfo("0xffbfffff").negative).toBe(true);
        });

        it("detects the exponent of the input", function() {
            pending("new understanding of exponents");
            expect(exact.FloatInfo("0x00000000000000000000").exponent).toEqual(0x0000);
            expect(exact.FloatInfo("0x00007fffffffffffffff").exponent).toEqual(0x0000);
            expect(exact.FloatInfo("0x00004aaaaaaaaaaaaaaa").exponent).toEqual(0x0000);
            expect(exact.FloatInfo("0x00008000000000000000").exponent).toEqual(0x0000);
            expect(exact.FloatInfo("0x0000ffffffffffffffff").exponent).toEqual(0x0000);
            expect(exact.FloatInfo("0x7fff0000000000000000").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fff3fffffffffffffff").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fff4000000000000000").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fff8000000000000000").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fffbfffffffffffffff").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fff8000000000000001").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fffc000000000000000").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fffffffffffffffffff").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x7fffc000000000000001").exponent).toEqual(0x7fff);
            expect(exact.FloatInfo("0x77770000000000000000").exponent).toEqual(0x7777);
            expect(exact.FloatInfo("0x44447fffffffffffffff").exponent).toEqual(0x4444);
            expect(exact.FloatInfo("0x33338000000000000000").exponent).toEqual(0x3333);
            expect(exact.FloatInfo("0x3333ffffffffffffffff").exponent).toEqual(0x3333);
            expect(exact.FloatInfo("0x0000000000000000").exponent).toEqual(0x000);
            expect(exact.FloatInfo("0x000fffffffffffff").exponent).toEqual(0x000);
            expect(exact.FloatInfo("0x0008000000000000").exponent).toEqual(0x000);
            expect(exact.FloatInfo("0x0000000000000001").exponent).toEqual(0x000);
            expect(exact.FloatInfo("0x7ff0000000000000").exponent).toEqual(0x7ff);
            expect(exact.FloatInfo("0x7ff8000000000000").exponent).toEqual(0x7ff);
            expect(exact.FloatInfo("0x7fffffffffffffff").exponent).toEqual(0x7ff);
            expect(exact.FloatInfo("0x7ff0000000000001").exponent).toEqual(0x7ff);
            expect(exact.FloatInfo("0x7ff7ffffffffffff").exponent).toEqual(0x7ff);
            expect(exact.FloatInfo("0x7770000000000000").exponent).toEqual(0x777);
            expect(exact.FloatInfo("0x001fffffffffffff").exponent).toEqual(0x001);
            expect(exact.FloatInfo("0x4008000000000000").exponent).toEqual(0x400);
            expect(exact.FloatInfo("0x00000000").exponent).toEqual(0x00);
            expect(exact.FloatInfo("0x007fffff").exponent).toEqual(0x00);
            expect(exact.FloatInfo("0x00400000").exponent).toEqual(0x00);
            expect(exact.FloatInfo("0x00000001").exponent).toEqual(0x00);
            expect(exact.FloatInfo("0x7f800000").exponent).toEqual(0xff);
            expect(exact.FloatInfo("0x7fc00000").exponent).toEqual(0xff);
            expect(exact.FloatInfo("0x7fffffff").exponent).toEqual(0xff);
            expect(exact.FloatInfo("0x7fbfffff").exponent).toEqual(0xff);
            expect(exact.FloatInfo("0x7f800001").exponent).toEqual(0xff);
            expect(exact.FloatInfo("0x40000000").exponent).toEqual(0x80);
            expect(exact.FloatInfo("0x00800000").exponent).toEqual(0x01);
            expect(exact.FloatInfo("0x3bffffff").exponent).toEqual(0x77);
            expect(exact.FloatInfo("0x7f7fffff").exponent).toEqual(0xfe);
        });

        it("detects the mantissa of the input", function() {
            pending("new understanding of mantissas");
            expect(exact.FloatInfo("0x00000000000000000000").mantissa).toEqual(BigInteger("0x0000000000000000"));
            expect(exact.FloatInfo("0x00007fffffffffffffff").mantissa).toEqual(BigInteger("0x7fffffffffffffff"));
            expect(exact.FloatInfo("0x00004aaaaaaaaaaaaaaa").mantissa).toEqual(BigInteger("0x4aaaaaaaaaaaaaaa"));
            expect(exact.FloatInfo("0x00008000000000000000").mantissa).toEqual(BigInteger("0x8000000000000000"));
            expect(exact.FloatInfo("0x0000ffffffffffffffff").mantissa).toEqual(BigInteger("0xffffffffffffffff"));
            expect(exact.FloatInfo("0x7fff0000000000000000").mantissa).toEqual(BigInteger("0x0000000000000000"));
            expect(exact.FloatInfo("0x7fff3fffffffffffffff").mantissa).toEqual(BigInteger("0x3fffffffffffffff"));
            expect(exact.FloatInfo("0x7fff4000000000000000").mantissa).toEqual(BigInteger("0x4000000000000000"));
            expect(exact.FloatInfo("0x7fff8000000000000000").mantissa).toEqual(BigInteger("0x8000000000000000"));
            expect(exact.FloatInfo("0x7fffbfffffffffffffff").mantissa).toEqual(BigInteger("0xbfffffffffffffff"));
            expect(exact.FloatInfo("0x7fff8000000000000001").mantissa).toEqual(BigInteger("0x8000000000000001"));
            expect(exact.FloatInfo("0x7fffc000000000000000").mantissa).toEqual(BigInteger("0xc000000000000000"));
            expect(exact.FloatInfo("0x7fffffffffffffffffff").mantissa).toEqual(BigInteger("0xffffffffffffffff"));
            expect(exact.FloatInfo("0x7fffc000000000000001").mantissa).toEqual(BigInteger("0xc000000000000001"));
            expect(exact.FloatInfo("0x77770000000000000000").mantissa).toEqual(BigInteger("0x0000000000000000"));
            expect(exact.FloatInfo("0x44447fffffffffffffff").mantissa).toEqual(BigInteger("0x7fffffffffffffff"));
            expect(exact.FloatInfo("0x33338000000000000000").mantissa).toEqual(BigInteger("0x8000000000000000"));
            expect(exact.FloatInfo("0x3333ffffffffffffffff").mantissa).toEqual(BigInteger("0xffffffffffffffff"));
            expect(exact.FloatInfo("0x0000000000000000").mantissa).toEqual(BigInteger("0x0000000000000"));
            expect(exact.FloatInfo("0x000fffffffffffff").mantissa).toEqual(BigInteger("0xfffffffffffff"));
            expect(exact.FloatInfo("0x0008000000000000").mantissa).toEqual(BigInteger("0x8000000000000"));
            expect(exact.FloatInfo("0x0000000000000001").mantissa).toEqual(BigInteger("0x0000000000001"));
            expect(exact.FloatInfo("0x7ff0000000000000").mantissa).toEqual(BigInteger("0x0000000000000"));
            expect(exact.FloatInfo("0x7ff8000000000000").mantissa).toEqual(BigInteger("0x8000000000000"));
            expect(exact.FloatInfo("0x7fffffffffffffff").mantissa).toEqual(BigInteger("0xfffffffffffff"));
            expect(exact.FloatInfo("0x7ff0000000000001").mantissa).toEqual(BigInteger("0x0000000000001"));
            expect(exact.FloatInfo("0x7ff7ffffffffffff").mantissa).toEqual(BigInteger("0x7ffffffffffff"));
            expect(exact.FloatInfo("0x7770000000000000").mantissa).toEqual(BigInteger("0x0000000000000"));
            expect(exact.FloatInfo("0x001fffffffffffff").mantissa).toEqual(BigInteger("0xfffffffffffff"));
            expect(exact.FloatInfo("0x4008000000000000").mantissa).toEqual(BigInteger("0x8000000000000"));
            expect(exact.FloatInfo("0x00000000").mantissa).toEqual(BigInteger("0"));
            expect(exact.FloatInfo("0x007fffff").mantissa).toEqual(BigInteger("0x7fffff"));
            expect(exact.FloatInfo("0x00400000").mantissa).toEqual(BigInteger("0x400000"));
            expect(exact.FloatInfo("0x00000001").mantissa).toEqual(BigInteger("0x000001"));
            expect(exact.FloatInfo("0x7f800000").mantissa).toEqual(BigInteger("0x000000"));
            expect(exact.FloatInfo("0x7fc00000").mantissa).toEqual(BigInteger("0x400000"));
            expect(exact.FloatInfo("0x7fffffff").mantissa).toEqual(BigInteger("0x7fffff"));
            expect(exact.FloatInfo("0x7fbfffff").mantissa).toEqual(BigInteger("0x3fffff"));
            expect(exact.FloatInfo("0x7f800001").mantissa).toEqual(BigInteger("0x000001"));
            expect(exact.FloatInfo("0x40000000").mantissa).toEqual(BigInteger("0x000000"));
            expect(exact.FloatInfo("0x00800000").mantissa).toEqual(BigInteger("0x000000"));
            expect(exact.FloatInfo("0x3bffffff").mantissa).toEqual(BigInteger("0x7fffff"));
            expect(exact.FloatInfo("0x7f7fffff").mantissa).toEqual(BigInteger("0x7fffff"));
        });

        it("detects the number type of the input", function() {
            expect(exact.FloatInfo("0x00000000000000000000").number_type).toBe("zero");
            expect(exact.FloatInfo("0x00007fffffffffffffff").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x00004aaaaaaaaaaaaaaa").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x00008000000000000000").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x0000ffffffffffffffff").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x7fff0000000000000000").number_type).toBe("infinity");
            expect(exact.FloatInfo("0x7fff3fffffffffffffff").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7fff4000000000000000").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7fff8000000000000000").number_type).toBe("infinity");
            expect(exact.FloatInfo("0x7fffbfffffffffffffff").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7fff8000000000000001").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7fffc000000000000000").number_type).toBe("indefinite");
            expect(exact.FloatInfo("0x7fffffffffffffffffff").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x7fffc000000000000001").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x77770000000000000000").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x44447fffffffffffffff").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x33338000000000000000").number_type).toBe("normal");
            expect(exact.FloatInfo("0x3333ffffffffffffffff").number_type).toBe("normal");
            expect(exact.FloatInfo("0x0000000000000000").number_type).toBe("zero");
            expect(exact.FloatInfo("0x000fffffffffffff").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x0008000000000000").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x0000000000000001").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x7ff0000000000000").number_type).toBe("infinity");
            expect(exact.FloatInfo("0x7ff8000000000000").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x7fffffffffffffff").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x7ff0000000000001").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7ff7ffffffffffff").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7770000000000000").number_type).toBe("normal");
            expect(exact.FloatInfo("0x001fffffffffffff").number_type).toBe("normal");
            expect(exact.FloatInfo("0x4008000000000000").number_type).toBe("normal");
            expect(exact.FloatInfo("0x00000000").number_type).toBe("zero");
            expect(exact.FloatInfo("0x007fffff").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x00400000").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x00000001").number_type).toBe("denormal");
            expect(exact.FloatInfo("0x7f800000").number_type).toBe("infinity");
            expect(exact.FloatInfo("0x7fc00000").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x7fffffff").number_type).toBe("quiet_nan");
            expect(exact.FloatInfo("0x7fbfffff").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x7f800001").number_type).toBe("signaling_nan");
            expect(exact.FloatInfo("0x40000000").number_type).toBe("normal");
            expect(exact.FloatInfo("0x00800000").number_type).toBe("normal");
            expect(exact.FloatInfo("0x3bffffff").number_type).toBe("normal");
            expect(exact.FloatInfo("0x7f7fffff").number_type).toBe("normal");
        });
    });

    describe("when constructed with a numeric string", function() {
        it("requires the bit-count parameter", function() {
            expect(constructFloatInfo("1")).toThrow();
            expect(constructFloatInfo("1", 32)).not.toThrow();
            expect(constructFloatInfo("1", 64)).not.toThrow();
            expect(constructFloatInfo("1", 80)).not.toThrow();
        });

        it("accepts only supported bit counts", function() {
            expect(constructFloatInfo("1", 31)).toThrow();
            expect(constructFloatInfo("1", 96)).toThrow();
            expect(constructFloatInfo("1", 128)).toThrow();
        });

        it("acts equivalently to the corresponding hex value", function() {
            expect(exact.FloatInfo("0", 32)).toEqual(exact.FloatInfo("0x00000000"));
            expect(exact.FloatInfo("0", 64)).toEqual(exact.FloatInfo("0x0000000000000000"));
            expect(exact.FloatInfo("0", 80)).toEqual(exact.FloatInfo("0x00000000000000000000"));
            expect(exact.FloatInfo("10.5", 32)).toEqual(exact.FloatInfo("0x41280000"));
            expect(exact.FloatInfo("10.5", 64)).toEqual(exact.FloatInfo("0x4025000000000000"));
            expect(exact.FloatInfo("10.5", 80)).toEqual(exact.FloatInfo("0x4002a800000000000000"));
        });

        it("accepts scientific notation", function() {
            expect(exact.FloatInfo(".105e2", 32)).toEqual(exact.FloatInfo("10.5", 32));
            expect(exact.FloatInfo("10500e-3", 32)).toEqual(exact.FloatInfo("10.5", 32));
        });
    });

    describe("when formatted", function() {
        it("appears as a string", function() {
            expect(exact.FloatInfo("0", 80).toString()).toEqual("0");
            expect(exact.FloatInfo("1.", 80).toString()).toEqual("1");
            expect(exact.FloatInfo("-1.5", 80).toString()).toEqual("-1.5");
            expect(exact.FloatInfo("87.285", 80).toString()).toEqual("87.28500" + "00000" + "00000" + "00333" + "06690" + "73875" + "46962" + "12708" + "95004" + "27246" + "09375");
            expect(exact.FloatInfo("0.0625", 80).toString()).toEqual("0.0625");

            expect(exact.FloatInfo("0", 64).toString()).toEqual("0");
            expect(exact.FloatInfo("1.", 64).toString()).toEqual("1");
            expect(exact.FloatInfo("-1.5", 64).toString()).toEqual("-1.5");
            expect(exact.FloatInfo("87.285", 64).toString()).toEqual("87.28499" + "99999" + "99996" + "58939" + "48683" + "51519" + "10781" + "86035" + "15625");
            expect(exact.FloatInfo("0.0625", 64).toString()).toEqual("0.0625");

            expect(exact.FloatInfo("0", 32).toString()).toEqual("0");
            expect(exact.FloatInfo("1.", 32).toString()).toEqual("1");
            expect(exact.FloatInfo("-1.5", 32).toString()).toEqual("-1.5");
            expect(exact.FloatInfo("87.285", 32).toString()).toEqual("87.28500" + "36621" + "09375");
            expect(exact.FloatInfo("0.0625", 32).toString()).toEqual("0.0625");
        });
    });
});
