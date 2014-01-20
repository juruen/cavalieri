#ifndef FOLDS_H
#define FOLDS_H

#include <functional>
#include <proto.pb.h>
#include <streams.h>

typedef std::function<double(const double&, const double&)> fold_fn_t;

mstream_t fold(const fold_fn_t f, const children_t & children);

mstream_t sum(const children_t);

mstream_t product(const children_t);

mstream_t difference(const children_t);

mstream_t mean(const children_t);

mstream_t minimum(const children_t);

mstream_t maximum(const children_t);

#endif
