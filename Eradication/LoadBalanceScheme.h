
#pragma once

#include <string>
#include "IdmApi.h"

namespace Kernel
{
    struct IInitialLoadBalanceScheme
    {
        virtual ~IInitialLoadBalanceScheme() {};

        virtual void Initialize( const std::string& rFilename, 
                                 uint32_t expectedNumNodes, 
                                 uint32_t numTasks ) = 0;

        virtual int GetInitialRankFromNodeId( uint32_t node_id ) = 0;
    };

    class IDMAPI LoadBalanceSchemeFactory
    {
    public:
        static IInitialLoadBalanceScheme* Create( const std::string& rFilename, 
                                                  uint32_t expectedNumNodes, 
                                                  uint32_t numTasks );
    };
}