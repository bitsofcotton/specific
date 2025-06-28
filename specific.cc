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

template <typename T> static inline int countLSBIdx(const T& x) {
  myuint m(x);
  int idx(0);
  for( ; idx < sizeof(myuint) * 8 && int(idx ? m >> idx : m) & 1; idx ++) ;
  return idx;
}

template <typename T> static inline T reverseMantissa(const T& x, const int& block = 1, const int& offset = 0) {
  myuint m(x);
  myuint res(int(0));
  m >>= offset;
  for(int i = 0; i < sizeof(myuint) * 8; i --) {
    res <<= block;
    res |= myuint(m >> (block * i)) & ((myuint(int(1)) << block) - myuint(int(1)) );
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
  for(int i0 = 0; i0 < invs.rows() - 1; i0 ++) {
    op = invs * op0;
    std::vector<int> lsbs;
    lsbs.reserve(op.rows() * op.cols());
    for(int i = 0; i < op.rows(); i ++)
      for(int j = 0; j < op.cols(); j ++)
        lsbs.emplace_back(countLSBIdx<num_t>(op(i, j) << myint(int(_FLOAT_BITS_))));
    std::sort(lsbs.begin(), lsbs.end());
    lsbs.erase(std::unique(lsbs.begin(), lsbs.end()), lsbs.end());
    for(int i = 0; i < op.rows(); i ++)
      for(int j = 0; j < op.cols(); j ++)
        op(i, j)  =
          reverseMantissa<num_t>(op(i, j) << myint(int(_FLOAT_BITS_)),
            lsbs.size() <= 1 ? 1 : lsbs[1] - lsbs[0], lsbs[0]);
    invs.row(i0)  = linearInvariant<num_t>(op * op.transpose());
    invs.row(i0) /= sqrt(invs.row(i0).dot(invs.row(i0)));
  }
  {
    SimpleVector<num_t> lastinv(invs.rows());
    lastinv.O(num_t(int(1)));
    lastinv -= invs * lastinv;
    invs.row(invs.rows() - 1) = lastinv / sqrt(lastinv.dot(lastinv));
  }
  std::cout << (op = invs * op0) << std::endl;
  for(int i = 1; i < argc; i ++) {
    vector<SimpleMatrix<num_t> > work;
    if(! loadp2or3<num_t>(work, argv[i])) continue;
    SimpleVector<num_t> in(work.size() * work[0].rows() * work[0].cols());
    in.O();
    for(int j = 0; j < work.size(); j ++)
      for(int k = 0; k < work[j].rows(); k ++)
        in.setVector(j * work[0].rows() * work[0].cols() +
          k * work[0].cols(), work[j].row(k));
    vector<SimpleMatrix<num_t> > out;
    out.resize(1, SimpleMatrix<num_t>(1, 4));
    out[0].row(0).O() = revertProgramInvariant<num_t>(make_pair(
      op * makeProgramInvariant<num_t>(in).first, num_t(int(1)) ) );
    if(! savep2or3<num_t>((std::string(argv[i]) + std::string("-specific.pgm")).c_str(), out) )
      cerr << "failed to save." << endl;
  }
  return 0;
 usage:
  cerr << "Usage:" << endl;
  cerr << argv[0] << " <input0.ppm> ... < ..." << endl;
  return -1;
}

