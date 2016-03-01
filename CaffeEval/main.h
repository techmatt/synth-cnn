
#include "mLibInclude.h"

#include <leveldb/db.h>
#include <leveldb/write_batch.h>

#include <stdint.h>
#include <sys/stat.h>
#include <direct.h>

#include <fstream>  // NOLINT(readability/streams)
#include <string>

#include "boost/algorithm/string.hpp"
#include "caffe/caffe.hpp"
#include "caffe/util/signal_handler.h"

using namespace caffe;  // NOLINT(build/namespaces)
using namespace google;

#include "../common/constants.h"
#include "../common/netflixDatabase.h"

#include "util.h"
#include "networkProcessor.h"
