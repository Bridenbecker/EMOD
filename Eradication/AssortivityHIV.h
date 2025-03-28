
#pragma once

#include <vector>
#include "Assortivity.h"

namespace Kernel 
{
    // AssortivityHIV adds the groups of STI CoInfection Status and HIV Infection Status
    class IDMAPI AssortivityHIV : public Assortivity
    {
        DECLARE_QUERY_INTERFACE();
    public:
        AssortivityHIV( RelationshipType::Enum relType=RelationshipType::TRANSITORY, RANDOMBASE *prng=nullptr );
        virtual ~AssortivityHIV();

        // -------------------------
        // --- Assortivity Methods
        // -------------------------

    protected:
        virtual IIndividualHumanSTI* SelectPartnerForExtendedGroups( AssortivityGroup::Enum group,
                                                                     IIndividualHumanSTI* pPartnerA,
                                                                     const list<IIndividualHumanSTI*>& potentialPartnerList );

        // This routine is called inside Configure() but before the data is completely read.
        virtual void AddConfigurationParameters( AssortivityGroup::Enum group, const Configuration *config ) override;

        virtual void CheckDerivedValues() override;
        virtual void CheckAxesForReceivedResults();
        void SortMatrixReceivedResults();

    private:
        DECLARE_SERIALIZABLE(AssortivityHIV);
    };
}