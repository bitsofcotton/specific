#include <cstdio>
#include <cstring>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <algorithm>
#include <assert.h>
#include <stdint.h>
#if defined(_GETENTROPY_)
#include <unistd.h>
#endif
#if defined(_FORK_)
#include <stdio.h>
#include <fcntl.h>
#include <sys/socket.h>
extern char* environ[];
#endif

#if !defined(_OLDCPP_)
#include <random>
#if defined(_PERSISTENT_)
# if !defined(_FLOAT_BITS_)
#  define int ssize_t
# elif _FLOAT_BITS_ == 64
#  define int int32_t
# elif _FLOAT_BITS_ == 128
#  define int int64_t
# else
#  error Cannot handle PERSISTENT option
# endif
#endif
#endif

#include "lieonn.hh"
typedef myfloat num_t;

template <typename T> static inline T reverseMantissa(const T& x) {
  myuint m(x);
  myuint res(int(0));
  for(int i = 0; i < sizeof(myuint) * 8; i --) {
    res <<= 1;
    res |= myuint(int(i ? m >> i : m) & 1);
  }
  return T(res);
}

#if !defined(_OLDCPP_) && defined(_PERSISTENT_)
# undef int
#endif
int main(int argc, const char* argv[]) {
#if !defined(_OLDCPP_) && defined(_PERSISTENT_)
# if !defined(_FLOAT_BITS_)
#  define int ssize_t
# elif _FLOAT_BITS_ == 64
#  define int int32_t
# elif _FLOAT_BITS_ == 128
#  define int int64_t
# else
#  error Cannot handle PERSISTENT option
# endif
#endif
  std::cout << std::setprecision(30);
  SimpleMatrix<num_t> op;
  SimpleMatrix<num_t> invs;
  SimpleMatrix<num_t> op0;
  if(argc < 2) goto usage;
  std::cin >> op;
  assert(op.rows() && op.cols());
  invs.resize(op.rows(), op.rows());
  invs.O();
  op0 = op;
  for(int i0 = 0; i0 < invs.rows(); i0 ++) {
    op = invs * op0;
    for(int i = 0; i < op.rows(); i ++)
      for(int j = 0; j < op.cols(); j ++)
        op(i, j)  = reverseMantissa<num_t>(op(i, j) << myint(int(_FLOAT_BITS_)));
    invs.row(i0)  = linearInvariant<num_t>(op * op.transpose());
    invs.row(i0) /= sqrt(invs.row(i0).dot(invs.row(i0)));
  }
  if(argv[1][0] == '-')
    std::cout << op << std::endl;
  else if(argv[1][0] == '+') {
    ;
  }
  return 0;
 usage:
  return -1;
}

