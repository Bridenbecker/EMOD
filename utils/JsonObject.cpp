#include <stdafx.h>
#include <string>
#include <iostream>

#include "RapidJsonImpl.h"
#include "Log.h"

////////////////////////////////////////////////////////////////////////////
// Implements class factory (sort of)
// The assumption is at any given time, there is only one Json library is 
// compiled in as the underlying impl class 
SETUP_LOGGING( "JsonObject" )

namespace Kernel {

    IJsonObjectAdapter* CreateJsonObjAdapter(JsonLibType jsLib)
    {
        IJsonObjectAdapter* jsObj = nullptr;

        switch(jsLib)
        {
            case JS_RAPIDJSON:
                jsObj = _new_ RapidJsonObj();
                break;

            default:
                LOG_INFO_F("The Json library type %d is not supported yet. Only rapidjson library for Json-based serialization\n", jsLib);
                break;
        }

        return jsObj;
    }
}