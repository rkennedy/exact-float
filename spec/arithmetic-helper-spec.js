"use strict";

var exact = require("../src/exact-float.js").exact;
var BigInteger = require("../lib/javascript-bignum/biginteger.js").BigInteger;

var compareBigInteger = function(first, second) {
    if (first instanceof BigInteger)
        return first.compare(second) == 0;
    return undefined;
};

describe("The arithmatic helper functions", function() {
    beforeEach(function() {
        jasmine.addCustomEqualityTester(compareBigInteger);
    });

    it("include a minimize_mantissa method", function() {
        var func = exact.minimize_mantissa;
        expect(func).toBeDefined();

        describe("The minimize_mantissa function", function() {
            it("does nothing to positive mantissas", function() {
                expect(func(0b011000, 3)[0]).toEqual(0b11000);
                expect(func(0b011000, 3)[1]).toBe(3);
            });
            it("does something to smaller mantissas", function() {
                expect(func(0b0100, -3)[0]).toEqual(0b1);
                expect(func(0b0100, -3)[1]).toBe(-1);
            });
            it("reduces larger mantissas as far as they'll go", function() {
                expect(func(0b110000, -3)[0]).toEqual(0b000110);
                expect(func(0b110000, -3)[1]).toBe(0);
            });
        });
    });

    it("include a reduce_fraction function", function() {
        var func = exact.reduce_fraction;
        expect(func).toBeDefined();

        describe("The reduce_fraction function", function() {
            it("does something with small values", function() {
                expect(func(BigInteger(2), -4)[0]).toEqual(5*5*5*5*2);
                expect(func(BigInteger(2), -4)[1]).toBe(0);
                expect(func(BigInteger(2), -4)[2]).toBe(-4);
            });
            it("does nothing to unit mantissas", function() {
                expect(func(BigInteger(3), 0)[0]).toEqual(3);
                expect(func(BigInteger(3), 0)[1]).toBe(0);
                expect(func(BigInteger(3), 0)[2]).toBe(0);
            });
            it("does nothing to larger mantissas", function() {
                expect(func(BigInteger(4), 6)[0]).toEqual(4);
                expect(func(BigInteger(4), 6)[1]).toBe(6);
                expect(func(BigInteger(4), 6)[2]).toBe(0);
            });
        });
    });

    it("include a reduce_binary_exponent function", function() {
        var func = exact.reduce_binary_exponent;
        expect(func).toBeDefined();

        describe("The reduce_binary_exponent function", function() {
            it("shifts mantissas by their binary exponents", function() {
                expect(func(0b1100, 4)).toEqual(0b11000000);
                expect(func(0b1100, 1)).toEqual(0b11000);
                expect(func(0b110, 0)).toEqual(0b110);
            });
        });
    });
});
