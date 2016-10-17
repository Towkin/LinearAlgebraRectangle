#include "UniqueID.h"

unsigned int UniqueID::sID = 0;

UniqueID::UniqueID() : ID(sID++) {}

UniqueID::UniqueID(const UniqueID & aCopy) : UniqueID() {}

UniqueID::~UniqueID() {}
