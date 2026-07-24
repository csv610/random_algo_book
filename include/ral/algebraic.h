#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <tuple>
#include <stdexcept>

#include "freivalds.h"
#include "polynomial_chapter8.h"
#include "rabin_karp.h"
#include "polynomial.h"
#include "number_theory.h"
#include "crypto.h"

namespace ral {

// Freivalds verification
using chapter8::freivalds_verify;

// Schwartz-Zippel PIT
using chapter8::poly_eval;
using chapter8::polynomial_identity_test;
using chapter8::poly_degree;

// Rabin-Karp String Matching
using chapter8::string_fingerprint;
using chapter8::power_mod;
using chapter8::rabin_karp_search;

// Number Theory
using randalgo::mod_pow;
using randalgo::gcd;
using randalgo::extended_gcd;
using randalgo::ExtendedGCDResult;
using randalgo::mod_inverse;
using randalgo::is_prime_trial;
using randalgo::Congruence;
using randalgo::chinese_remainder_theorem;
using randalgo::euler_totient;

// Cryptography & Fields
using randalgo::is_quadratic_residue;
using randalgo::legendre_symbol;
using randalgo::jacobi_symbol;
using randalgo::RSAKeyPair;
using randalgo::generate_rsa_keypair;
using randalgo::rsa_encrypt;
using randalgo::rsa_decrypt;

// Polynomial Arithmetic
using Polynomial = randalgo::Polynomial;

} // namespace ral
