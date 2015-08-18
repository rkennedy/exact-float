"use strict";

module.exports.exact = (function() {
    var BigInteger = require("../lib/javascript-bignum/biginteger.js").BigInteger;

    function lsb(x)
    {
        return 0;
    }

    function minimize_mantissa(/*BigInteger*/ Man, /*int*/ BinExp)
    {
        Man = BigInteger(Man);
        var idx = Man.is_zero() ? 0 : lsb(Man);
        var adjustment = Math.min(idx, -BinExp);
        if (adjustment <= 0)
            return [Man, BinExp];
        return [Man >> adjustment, BinExp + adjustment];
    }

    function reduce_fraction(/*BigInteger*/ Man, /*int*/ BinExp)
    {
        if (BinExp >= 0)
            return [Man, BinExp, 0];
        return [Man.multiply(BigInteger(5).pow(-BinExp)), 0, BinExp];
    }

    function reduce_binary_exponent(a, b)
    {
    }

    function Float32(s)
    {
    }

    function Float64(s)
    {
    }

    function Float80(s)
    {
    }

    return {
        minimize_mantissa: minimize_mantissa,
        reduce_fraction: reduce_fraction,
	Float32: Float32,
        Float64: Float64,
        Float80: Float80
    };
})();
