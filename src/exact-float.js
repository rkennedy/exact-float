"use strict";

module.exports.exact = (function() {
    var BigInteger = require("../lib/javascript-bignum/biginteger.js").BigInteger;

    function assert(condition, message) {
        if (!condition)
            throw new Error(message);
    }

    function lsb(/*BigInteger*/x)
    {
        assert(x instanceof BigInteger, "unexpected " + typeof x);
        var result = 0;
        for ( ; x.isEven(); x = x.divide(2))
            result++;
        return result;
    }

    function minimize_mantissa(/*BigInteger*/ Man, /*int*/ BinExp)
    {
        Man = BigInteger(Man);
        var idx = Man.isZero() ? 0 : lsb(Man);
        var adjustment = Math.min(idx, -BinExp);
        if (adjustment <= 0)
            return [Man, BinExp];
        return [Man.divide(BigInteger.small[2].pow(adjustment)), BinExp + adjustment];
    }

    function reduce_fraction(/*BigInteger*/ Man, /*int*/ BinExp)
    {
        assert(Man instanceof BigInteger, "unexpected " + typeof Man);
        if (BinExp >= 0)
            return [Man, BinExp, 0];
        return [Man.multiply(BigInteger.small[5].pow(-BinExp)), 0, BinExp];
    }

    function reduce_binary_exponent(Man, BinExp)
    {
        Man = BigInteger(Man);
        return Man.multiply(Math.pow(2, BinExp));
    }

    function bit_test(value, bit)
    {
        return value.remainder(BigInteger.small[2].pow(bit + 1)).compare(BigInteger.small[2].pow(bit)) !== -1;
    }

    var beta = BigInteger.small[2];

    function float_traits(bits)
    {
        if (!(this instanceof float_traits))
            return new float_traits(bits);

        this.bits = bits;
        this.digits = (bits === 32) ? 24 : (bits === 64) ? 53 : 64;
        this.implied_one = bits !== 80;
        this.max_exponent = (bits === 32) ? 128 : (bits === 64) ? 1024 : 16384;
        this.betan_1 = beta.pow(this.digits - 1);
        this.betan = beta.pow(this.digits);

    }
    float_traits.prototype.mantissa_bits = function() {
        return this.digits - this.implied_one;
    }
    float_traits.prototype.exponent_bits = function() {
        return this.bits - 1 - this.mantissa_bits();
    }
    float_traits.prototype.exponent_bias = function() {
        return this.max_exponent - 1;
    }
    float_traits.prototype.get_negative = function(rec) {
        return !rec.subtract(BigInteger.small[2].pow(this.bits - 1)).isNegative();
    }
    float_traits.prototype.get_exponent = function(rec) {
        return rec.divide(BigInteger.small[2].pow(this.mantissa_bits())).remainder(BigInteger.small[2].pow(this.exponent_bits())).valueOf();
    }
    float_traits.prototype.get_mantissa = function(rec) {
        return rec.remainder(BigInteger.small[2].pow(this.mantissa_bits()));
    }
    float_traits.prototype.get_number_type = function(exponent, mantissa) {
        var exponent_mask = BigInteger.small[2].pow(this.exponent_bits()).subtract(1);
        if (BigInteger(exponent).compare(exponent_mask) == 0) {
            if (mantissa.isZero())
                return "infinity";
            var top_bit = bit_test(mantissa, this.mantissa_bits() - 1);
            if (this.implied_one)
                return top_bit ? "quiet_nan" : "signaling_nan";
            // From here down, we know it's 80 bits.
            if (!top_bit)
                return "signaling_nan";
            if (bit_test(mantissa, this.mantissa_bits() - 2))
                return lsb(mantissa) < this.mantissa_bits() - 2 ? "quiet_nan" : "indefinite";
            return lsb(mantissa) < this.mantissa_bits() - 1 ? "signaling_nan" : "infinity";
        } else if (exponent === 0) {
            return mantissa.isZero() ? "zero" : "denormal";
        } else {
            return (!this.implied_one && !bit_test(mantissa, this.mantissa_bits() - 1)) ? "denormal" : "normal";
        }
    }

    // Return array containing sign, mantissa, and DecExp
    function parseFloat(value)
    {
        var place_counter = 0;
        var negative;
        var Mantissa = BigInteger.ZERO;
        var DecExp = 0;
        var current_places = 0;
        var current_number = BigInteger.ZERO;
        var current_negative;
        var got_exponent = false;
        for (var i = 0; i < value.length; ++i) {
            var c = value[i];
            switch (c) {
                case '-':
                case '+':
                    current_negative = c === '-';
                    break;
                case ',':
                    // Ignore thousands separators
                    continue;
                case '.':
                    // Decimal separator
                    place_counter = 1;
                    continue;
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    current_number = current_number.exp10(1).add(c.valueOf());
                    current_places -= place_counter;
                    break;
                case 'e':
                case 'E':
                    got_exponent = true;
                    negative = current_negative;
                    current_negative = undefined;
                    Mantissa = current_number;
                    current_number = BigInteger.ZERO;
                    DecExp = current_places;
                    current_places = 0;
                    place_counter = 0;
                    break;
            }
        }
        if (got_exponent) {
            if (!!current_negative)
                current_number = current_number.negate();
            DecExp += current_number.valueOf();
        } else {
            Mantissa = current_number;
            negative = current_negative;
            DecExp = current_places;
        }
        return [!!negative, Mantissa, DecExp];
    }

    float_traits.prototype.nextfloat = function(z)
    {
        var m = z[0];
        var k = z[1];
        if (m.compare(this.betan.subtract(1)) === 0)
            return [this.betan_1, k + 1];
        return [m.add(1), k];
    }

    float_traits.prototype.ratio_float = function(u, v, k) {
        var qr = u.divRem(v);
        var q = qr[0];
        var r = qr[1];
        var v_r = v.subtract(r);
        var z = [q, k];
        switch (r.compare(v_r)) {
            case -1:
                return z;
            case 1:
                return this.nextfloat(z);
            default:
                if (q.isEven())
                    return z;
                return this.nextfloat(z);
        }
    }

    // Given exact integer f and e, with f non-negative, returns the
    // floating-point number closest to f * 10^e.
    float_traits.prototype.AlgorithmM = function(/*BigInteger*/f, /*BigInteger*/e)
    {
        var self = this;
        function loop(/*BigInteger*/u, /*BigInteger*/v, /*int*/k) {
            var x = u.divide(v);
            if (x.compare(self.betan_1) === -1)
                return loop(beta.multiply(u), v, k - 1);
            if (self.betan.compare(x) !== 1)
                return loop(u, beta.multiply(v), k + 1);
            return self.ratio_float(u, v, k);
        }
        if (e.isNegative())
            return loop(f, BigInteger.ONE.exp10(e.negate()), 0);
        else
            return loop(f.exp10(e), BigInteger.ONE, 0);
    }

    var hex_re = new RegExp(/^0x([0-9a-fA-F]+)$/);

    function FloatInfo(value, bitCount)
    {
        if (!(this instanceof FloatInfo))
            return new FloatInfo(value, bitCount);

        var rec;
        if (hex_re.test(value)) {
            // Looks like a hex string.
            var num_digits = value.length - 2;  // Only count digits, not leading "0x"
            switch (num_digits * 4) {
                case 32:
                case 64:
                case 80:
                    if (typeof bitCount !== "undefined" && bitCount !== num_digits * 4)
                        throw new Error("explicit bit count does not agree with string length");
                    bitCount = num_digits * 4;
                    rec = BigInteger(value);
                    break;
                default:
                    throw new Error("string length implies invalid bit count");
            }

            var traits = float_traits(bitCount);

            var rec_exponent = traits.get_exponent(rec);
            var rec_mantissa = traits.get_mantissa(rec);

            this.negative = traits.get_negative(rec);
            this.number_type = traits.get_number_type(rec_exponent, rec_mantissa);
            switch (this.number_type) {
                case "normal":
                    var mantissa_offset = traits.mantissa_bits() - 1 + traits.implied_one;
                    this.coefficient = rec_mantissa;
                    if (traits.implied_one)
                        this.coefficient = this.coefficient.add(BigInteger.small[2].pow(mantissa_offset));
                    this.exponent = rec_exponent - traits.exponent_bias() - mantissa_offset;
                    break;
                case "zero":
                    this.coefficient = rec_mantissa;
                    this.exponent = rec_exponent;
                    break;
            }
        } else {
            // Not hex, or not a string.
            if (typeof bitCount === "undefined")
                throw new Error("non-hex parameter requires bit count");
            if (bitCount !== 32 && bitCount !== 64 && bitCount !== 80)
                throw new Error("invalid bit count");
            var traits = float_traits(bitCount);

            var decimal = parseFloat(value);
            this.negative = decimal[0];
            if (decimal[1].isZero()) {
                this.number_type = "zero";
                this.coefficient = BigInteger.ZERO;
                this.exponent = 0;
            } else {
                var binary = traits.AlgorithmM(decimal[1], BigInteger(decimal[2]));
                this.number_type = "normal";  // TODO
                this.coefficient = binary[0];
                this.exponent = binary[1];
            }
        }
    }
    FloatInfo.prototype.toString = function() {
        var Man = this.coefficient;
        var x = minimize_mantissa(Man, this.exponent);
        Man = x[0];
        var BinExp = x[1];

        var DecExp;
        x = reduce_fraction(Man, BinExp);
        Man = x[0];
        BinExp = x[1];
        DecExp = x[2];

        Man = reduce_binary_exponent(Man, BinExp);

        var Factor = DecExp < 0 ? BigInteger.ONE.exp10(-DecExp) : BigInteger.ONE;
        x = Man.divRem(Factor);
        var result = x[0].toString();
        if (!x[1].isZero()) {
            result += ".";
            var fraction_str = x[1].toString();
            var zeros_needed = Math.abs(DecExp) - fraction_str.length;
            if (zeros_needed > 0)
                result += (new Array(zeros_needed + 1)).join('0');
            result += fraction_str;
        }
        if (this.negative)
            result = "-" + result;
        return result;
    };

    return {
        minimize_mantissa: minimize_mantissa,
        reduce_fraction: reduce_fraction,
        reduce_binary_exponent: reduce_binary_exponent,
        FloatInfo: FloatInfo
    };
})();
