#ifndef FOLDS_H
#define FOLDS_H

#include <functional>
#include <proto.pb.h>
#include <streams.h>

typedef std::function<double(const double&, const double&)> fold_fn_t;

mstream_t fold(const fold_fn_t f, const children_t & children);

#endif
