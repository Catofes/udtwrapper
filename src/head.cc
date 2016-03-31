#include <log.hh>
#include <error.hh>
#include "head.hh"
#include "string.h"

int Head::Read(const char *data, uint16_t size)
{
    const Head *head = (const Head *) data;

    if (size == 0) { // uint16_t would never be less than 0
        throw HeadSpace::ParseZero();

    } else if (head->type >= HeadTypeNum) {
        Log::Log("Error Head Type.", 3);
        throw HeadSpace::UnkownType();

    } else if (size < sizeOfType[head->type]) {
        throw HeadSpace::Partial();
    }

    *this = *head;
    return 0;
}

int Head::Pack(char *data) const
{
    Head *head = (Head *) data;
    *head = *this;
    return sizeOfType[type];
}
