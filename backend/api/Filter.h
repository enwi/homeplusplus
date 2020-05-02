#pragma once

#include <string>
#include <vector>

#include "../database/DBHandler.h"

class Filter
{
public:
    std::string getSearchString() const;
    int getStartIndex() const;
    int getMaxLength() const;
};
