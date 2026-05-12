module;
#include <stdint.h>

export module fcl.core.git_revision;

export namespace fcl {

extern const char* const git_revision_sha;
extern const uint32_t git_revision_unix_timestamp;

} // end namespace fcl
