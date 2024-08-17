#pragma once

#include <stdint.h>
#include <string>
#include "knx_types.h"

class CemiFrame;

/**
 * This class represents an Application Protocol Data Unit. It is part of a CemiFrame.
 */
class APDU
{
        friend class CemiFrame;

    public:
        /**
         * Get the type of the APDU.
         */
        ApduType type() const;
        /**
         * Set the type of the APDU.
         */
        void type(ApduType atype);
        /**
         * Get a pointer to the data.
         */
        uint8_t* data();
        /**
         * Get the CemiFrame this APDU is part of.
         */
        CemiFrame& frame();
        /**
         * Get the length of the APDU. (This is actually the octet count of the NPDU.)
         */
        uint8_t length() const;
        /**
         * Convert APDU to string.
         */
        operator std::string() const;

    protected:
        /**
         * The constructor.
         * @param data The data of the APDU. Encoding depends on the ::ApduType. The class doesn't
         *             take possession of this pointer.
         * @param frame The CemiFrame this APDU is part of.
         */
        APDU(uint8_t* data, CemiFrame& frame);

    private:
        uint8_t* _data = 0;
        CemiFrame& _frame;
};