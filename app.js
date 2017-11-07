'use strict';

function isZero(bits)
{
  return !_.any(bits);
}

function lsb(bits)
{
  return _.findIndex(bits);
}

function minimize_mantissa(/*bits*/ Man, /*int*/ BinExp)
{
  var idx = isZero(Man) ? 0 : lsb(Man);
  var adjustment = Math.min(idx, -BinExp);
  if (adjustment <= 0)
    return { coefficient: Man, binary_exponent: BinExp };
  return {
    coefficient: _.drop(Man, adjustment),
    binary_exponent: BinExp + adjustment
  };
}

function reduce_fraction(/*bits*/ Man, /*int*/ BinExp)
{
  if (BinExp >= 0)
    return {
      coefficient: Man,
      binary_exponent: BinExp,
      decimal_exponent: 0
    }
  var FIVE = [true, false, true];
  var coefficient = _.chain([Man]).concat(_.times(-BinExp, _.constant(FIVE))).reduce(multiply);
  return {
    coefficient: coefficient.value(),
    binary_exponent: 0,
    decimal_exponent: BinExp
  };
}

function reduce_binary_exponent(/*bits*/ Man, /*int*/BinExp)
{
  return _.fill(Array(BinExp), false).concat(Man);
}

function float_traits(bits)
{
  if (!(this instanceof float_traits))
    return new float_traits(bits);
  this.bits = bits;
  this.digits = (bits === 32) ? 24 : (bits === 64) ? 53 : 64;
  this.implied_one = bits !== 80;
  this.max_exponent = (bits === 32) ? 128 : (bits === 64) ? 1024 : 16384;
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
function get_negative(rec) {
  return rec[rec.length - 1];
}
float_traits.prototype.get_exponent = function(rec) {
  return _.slice(rec, this.mantissa_bits(), this.mantissa_bits() + this.exponent_bits());
}
float_traits.prototype.get_coefficient = function(rec) {
  return _.slice(rec, 0, this.mantissa_bits());
}
float_traits.prototype.get_number_type = function(rec) {
  var exponent = this.get_exponent(rec);
  var mantissa = this.get_coefficient(rec);
  if (_.every(exponent)) {
    if (isZero(mantissa))
      return "infinity";
    var top_bit = mantissa[mantissa.length - 1];
    if (this.implied_one)
      return top_bit ? "quiet_nan" : "signaling_nan";
    // From here down, we know it's 80 bits.
    if (!top_bit)
      return "signaling_nan";
    if (mantissa[mantissa.length - 2])
      return lsb(mantissa) < this.mantissa_bits() - 2 ? "quiet_nan" : "indefinite";
    return lsb(mantissa) < this.mantissa_bits() - 1 ? "signaling_nan" : "infinity";
  } else if (isZero(exponent)) {
    return isZero(mantissa) ? "zero" : "denormal";
  } else {
    return (!this.implied_one && !mantissa[mantissa.length - 1]) ? "denormal" : "normal";
  }
}

function shift(a, n)
{
  return a << n;
}

function bit_or(a, b)
{
  return a | b;
}

function bits_to_int(bits)
{
  return _.chain(bits).map(shift).reduce(bit_or).value();
}

function int_to_nybble(value)
{
  return [(value & 1) != 0, (value & 2) != 0, (value & 4) != 0, (value & 8) != 0];
}

function digit_to_nybble(value)
{
  return int_to_nybble(parseInt(value, 16));
}

function nybble_to_hex(nybble)
{
  return bits_to_int(nybble).toString(16);
}

function bits_to_hex(bits)
{
  return _.chain(bits).chunk(4).map(nybble_to_hex).reverse().value().join("");
}

function ge(/*bits*/ A, /*bits*/ B)
{
  return _.chain(A).zipWith(B, function(a, b) {
    if (a && !b)
      return true;
    if (b && !a)
      return false;
  }).findLast(_.negate(_.isUndefined)).value() !== false;
}

function subtract(/*bits*/ A, /*bits*/ B)
{
  var borrow = false;
  return _.zipWith(A, B, function(a, b) {
    var result = borrow ^ !!a ^ !!b;
    borrow = !!((!a && (!!b ^ borrow)) || (!!b && borrow));
    return !!result;
  });
}

function divmod(/*bits*/dividend, /*bits*/ divisor)
{
  dividend = _.dropRightWhile(dividend, _.negate(_.identity));
  divisor = _.dropRightWhile(divisor, _.negate(_.identity));

  if (dividend.length < divisor.length)
    return { quotient: [false], remainder: dividend };

  var quotient = _.fill(Array(dividend.length), false);
  var remainder = [false];
  for (var i = dividend.length; i-- > 0; ) {
    remainder.unshift(dividend[i]);
    if (ge(remainder, divisor)) {
      remainder = subtract(remainder, divisor);
      quotient[i] = true;
    }
  }
  return { quotient: quotient, remainder: remainder };
}

function bits_to_digit(bits) {
  return bits_to_int(bits).toString();
}

function bits_to_string(bits)
{
  if (isZero(bits))
    return "0";
  var result = "";
  var TEN = [false, true, false, true];
  var qr;
  while (!isZero(bits)) {
    qr = divmod(bits, TEN);
    result = bits_to_digit(qr.remainder) + result;
    bits = qr.quotient;
  }
  return result;
}

function add(/*bits*/ A, /*bits*/ B)
{
  var carry = false;
  var sum = _.chain(A).zipWith(B, function(a, b) {
    var result = !!a ^ !!b ^ carry;
    carry = !!((!!a && !!b) || (!!a && carry) || (!!b && carry));
    return !!result;
  }).value();
  sum.push(carry);
  return sum;
}

function multiply(/*bits*/ A, /*bits*/ B)
{
  var partial_products = _.map(B, function(value, index) {
    if (value)
      return _.fill(Array(index), false).concat(A);
  });
  return _.reduce(partial_products, add);
}

function exp10(/*int*/ DecExp)
{
  var TEN = [false, true, false, true];
  return _.reduce(_.times(DecExp, _.constant(TEN)), multiply);
}

function bits_to_decimal(bits)
{
  var coefficient, exponent;
  var negative = get_negative(bits);

  var traits = float_traits(bits.length);
  switch (traits.get_number_type(bits)) {
    case 'normal':
      var mantissa_offset = traits.mantissa_bits() - 1 + traits.implied_one;
      coefficient = traits.get_coefficient(bits);
      if (traits.implied_one)
        coefficient[mantissa_offset] = true;
      exponent = bits_to_int(traits.get_exponent(bits)) - traits.exponent_bias() - mantissa_offset;
      return format_number(negative, coefficient, exponent);
      break;
    case 'zero':
      coefficient = traits.get_coefficient(bits);
      exponent = traits.get_exponent(bits);
      return format_number(negative, coefficient, exponent);
      break;
    default:
      return 'unimplemented';
  }
}

function format_number(/*bool*/ negative, /*bits*/ Coefficient, /*int*/ exponent)
{
  var Man = Coefficient;
  var x = minimize_mantissa(Man, exponent);
  Man = x.coefficient;
  var BinExp = x.binary_exponent;

  var DecExp;
  x = reduce_fraction(Man, BinExp);
  Man = x.coefficient;
  BinExp = x.binary_exponent;
  DecExp = x.decimal_exponent;

  Man = reduce_binary_exponent(Man, BinExp);

  var Factor = DecExp < 0 ? exp10(-DecExp) : [true];
  x = divmod(Man, Factor);
  var result = bits_to_string(x.quotient);
  if (!isZero(x.remainder)) {
    result += ".";
    var fraction_str = bits_to_string(x.remainder);
    if (Math.abs(DecExp) > fraction_str.length)
      result += _.padLeft(fraction_str, Math.abs(DecExp), '0');
    else
      result += fraction_str;
  }
  if (negative)
    result = "–" + result;
  return result;
}

var BitView = React.createClass({
  handleChange: function() {
    this.props.onChange(this.props.bit, this.refs.checkbox.getDOMNode().checked);
  },
  render: function() {
    return (<label>
      <input ref="checkbox" type="checkbox" checked={this.props.checked} onChange={this.handleChange}/>
      {this.props.bit}
    </label>);
  }
});
var BitCollection = React.createClass({
  render: function() {
    var bits = _.map(this.props.bits, function(value, index) {
      return (<li key={index}><BitView checked={value} bit={index} onChange={this.props.onBitChange}/></li>);
    }, this);
    return (<ul>{bits}</ul>);
  }
});

var HexView = React.createClass({
  handleChange: function() {
    this.props.onChange(this.refs.edit.getDOMNode().value);
  },
  render: function() {
    return (
      <div>
        <label>Hex <input ref="edit" value={this.props.text} onChange={this.handleChange}/></label>
      </div>
    );
  }
});

var DecimalView = React.createClass({
  render: function() {
    return (
      <div>
        <label>Decimal <input value={this.props.text} readOnly="true"/></label>
      </div>
    );
  }
});
var FloatInfo = React.createClass({
  render: function() {
    var bits = this.props.bits;
    var numBits = bits.length;

    return (<table>
      <tbody>
        <tr>
          <th>Type</th>
          <td>{float_traits(numBits).get_number_type(bits)}</td>
        </tr>
        <tr>
          <th>Minimal value</th>
          <td></td>
        </tr>
        <tr>
          <th>Full value</th>
          <td>{ bits_to_decimal(bits) }</td>
        </tr>
        <tr>
          <th>Hex value</th>
          <td>0x{ bits_to_hex(bits) }</td>
        </tr>
      </tbody>
    </table>);
  }
});
var FloatExplorer = React.createClass({
  getInitialState: function() {
    var new_bits = _.times(this.props.bits, _.constant(false));
    return {
      decimalText: bits_to_decimal(new_bits),
      hexText: bits_to_hex(new_bits),
      bits: new_bits
    };
  },
  handleBitChange: function(bit, newValue) {
    var new_bits = _.map(this.state.bits, function(value, index) {
      if (index === bit)
        return newValue;
      return value
    })

    this.setState({
      decimalText: this.state.decimalText, // bits_to_decimal(new_bits),
      hexText: this.state.hexText, // bits_to_hex(new_bits),
      bits: new_bits
    });
  },
  handleHexChange: function(newValue) {
    var new_bits = _.chain(newValue).takeRight(this.state.bits.length / 4).map(digit_to_nybble).reverse().flatten().value();

    this.setState({
      decimalText: this.state.decimalText,
      hexText: newValue,
      bits: new_bits
    });
  },
  render: function() {
    return (<div>
      <h1>{this.props.bits} bits</h1>
      <DecimalView text={this.state.decimalText}/>
      <HexView text={this.state.hexText} onChange={this.handleHexChange}/>
      <FloatInfo bits={this.state.bits}/>
      <BitCollection bits={this.state.bits} onBitChange={this.handleBitChange}/>
    </div>);
  }
});
React.render(
  <FloatExplorer bits="32"/>,
  document.getElementById('content')
);
