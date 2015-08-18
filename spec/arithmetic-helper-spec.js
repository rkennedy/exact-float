//"use strict";

var jasmine = require("jasmine-node");
var exact = require("../src/exact-float.js").exact;
var BigInteger = require("../lib/javascript-bignum/biginteger.js").BigInteger;

beforeEach(function() {
    var compareBigInteger = function(first, second) {
        if (typeof first == 'BigInteger')
            return first.compare(second) == 0;
        return undefined;
    };

    console.log(jasmine.getEnv().addCustomEqualityTester);
    jasmine.getEnv().addCustomEqualityTester(compareBigInteger);
});

describe("The exact module", function() {
    it("exists", function() {
        expect(exact).toBeDefined();
    });

    it("has a minimize_mantissa method", function() {
        expect(exact.minimize_mantissa).toBeDefined();
    });
});

/*
describe("The minimize_mantissa function", function() {
    it("does nothing to positive mantissas", function() {
        expect(exact.minimize_mantissa(0b011000, 3)[0]).toBe(0b11000);
        expect(exact.minimize_mantissa(0b011000, 3)[1]).toBe(3);
    });
    it("does something to smaller mantissas", function() {
        expect(exact.minimize_mantissa(0b0100, -3)[0]).toBe(0b1);
        expect(exact.minimize_mantissa(0b0100, -3)[1]).toBe(-1);
    });
    it("reduces larger mantissas as far as they'll go", function() {
        expect(exact.minimize_mantissa(0b110000, -3)[0]).toBe(0b000110);
        expect(exact.minimize_mantissa(0b110000, -3)[1]).toBe(0);
    });
});

describe("The reduce_fraction function", function() {
    it("does something with small values", function() {
        expect(exact.reduce_fraction(BigInteger(2), -4)[0]).toBe(BigInteger(5*5*5*5*2));
        expect(exact.reduce_fraction(BigInteger(2), -4)[1]).toBe(0);
        expect(exact.reduce_fraction(BigInteger(2), -4)[2]).toBe(-4);
    });
    it("does nothing to unit mantissas", function() {
        expect(exact.reduce_fraction(BigInteger(3), 0)[0]).toBe(3);
        expect(exact.reduce_fraction(BigInteger(3), 0)[1]).toBe(0);
        expect(exact.reduce_fraction(BigInteger(3), 0)[2]).toBe(0);
    });
    it("does nothing to larger mantissas", function() {
        expect(exact.reduce_fraction(BigInteger(4), 6)[0]).toBe(4);
        expect(exact.reduce_fraction(BigInteger(4), 6)[1]).toBe(6);
        expect(exact.reduce_fraction(BigInteger(4), 6)[2]).toBe(0);
    });
});

describe("The reduce_binary_exponent function", function() {
    it("shifts mantissas by their binary exponents", function() {
        expect(exact.reduce_binary_exponent(0b1100, 4)).toBe(0b11000000);
        expect(exact.reduce_binary_exponent(0b1100, 1)).toBe(0b11000);
        expect(exact.reduce_binary_exponent(0b110, 0)).toBe(0b110);
    });
});
*/
