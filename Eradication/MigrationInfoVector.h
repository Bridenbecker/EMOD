
#pragma once

#include "stdafx.h"
#include "IMigrationInfoVector.h"
#include "Migration.h"
#include "EnumSupport.h"

namespace Kernel
{
    struct IVectorSimulationContext;

    ENUM_DEFINE(ModiferEquationType,
        ENUM_VALUE_SPEC(LINEAR       , 1)
        ENUM_VALUE_SPEC(EXPONENTIAL  , 2))

    // ----------------------------------
    // --- MigrationInfoNullVector
    // ----------------------------------
    class IDMAPI MigrationInfoNullVector : public MigrationInfoNull, public IMigrationInfoVector
    {
    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()
        DECLARE_QUERY_INTERFACE()

    public:
        //IMigrationInfoVector
        virtual void UpdateRates( const suids::suid& rThisNodeId,
                                  const std::string& rSpeciesID,
                                  IVectorSimulationContext* pivsc ) {};

    protected:
        friend class MigrationInfoFactoryVector;
        friend class MigrationInfoFactoryVectorDefault;

        MigrationInfoNullVector();
        virtual ~MigrationInfoNullVector();
    };

    // ----------------------------------
    // --- MigrationInfoVector
    // ----------------------------------

    class IDMAPI MigrationInfoVector : public MigrationInfoFixedRate, public IMigrationInfoVector
    {
    public:
        IMPLEMENT_NO_REFERENCE_COUNTING()  
        DECLARE_QUERY_INTERFACE()
    public:
        virtual ~MigrationInfoVector();

        virtual void PickMigrationStep( RANDOMBASE* pRNG,
                                        IIndividualHumanContext * traveler, 
                                        float migration_rate_modifier, 
                                        suids::suid &destination, 
                                        MigrationType::Enum &migration_type,
                                        float &timeUntilTrip ) override;

        // IMigrationInfoVector
        virtual void UpdateRates( const suids::suid& rThisNodeId, 
                                  const std::string& rSpeciesID, 
                                  IVectorSimulationContext* pivsc ) override;

    protected:
        friend class MigrationInfoFactoryVector;
        friend class MigrationInfoFactoryVectorDefault;

        MigrationInfoVector( INodeContext* _parent,
                             ModiferEquationType::Enum equation,
                             float habitatModifier,
                             float foodModifier,
                             float stayPutModifier );

        virtual void Initialize( const std::vector<std::vector<MigrationRateData>>& rRateData ) override;
        virtual void SaveRawRates( std::vector<float>& r_rate_cdf ) override;
        float CalculateModifiedRate( const suids::suid& rNodeId, 
                                     float rawRate, 
                                     float populationRatio, 
                                     float habitatRatio );

        typedef std::function<int( const suids::suid& rNodeId, 
                                   const std::string& rSpeciesID, 
                                   IVectorSimulationContext* pivsc )> tGetValueFunc;

        std::vector<float> GetRatios( const std::vector<suids::suid>& rReachableNodes, 
                                      const std::string& rSpeciesID, 
                                      IVectorSimulationContext* pivsc, 
                                      tGetValueFunc getValueFunc );

    private:
        std::vector<float>        m_RawMigrationRate;
        suids::suid               m_ThisNodeId;
        ModiferEquationType::Enum m_ModifierEquation;
        float                     m_ModifierHabitat;
        float                     m_ModifierFood;
        float                     m_ModifierStayPut;
    };


    // ----------------------------------
    // --- MigrationInfoFactoryVector
    // ----------------------------------

    class MigrationInfoFactoryVector : public IMigrationInfoFactoryVector
    {
    public:
        MigrationInfoFactoryVector( bool enableVectorMigration );
        virtual ~MigrationInfoFactoryVector();

        // IMigrationInfoFactoryVector
        virtual void ReadConfiguration( JsonConfigurable* pParent, const ::Configuration* config ) override;
        virtual IMigrationInfoVector* CreateMigrationInfoVector( 
            const std::string& idreference,
            INodeContext *parent_node, 
            const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override;

    private:
        MigrationInfoFile         m_InfoFileVector;
        ModiferEquationType::Enum m_ModifierEquation;
        float                     m_ModifierHabitat;
        float                     m_ModifierFood;
        float                     m_ModifierStayPut;
    };

    // ----------------------------------
    // --- MigrationInfoFactoryVectorDefault
    // ----------------------------------

    class IDMAPI MigrationInfoFactoryVectorDefault : public IMigrationInfoFactoryVector
    {
    public:
        MigrationInfoFactoryVectorDefault( bool enableVectorMigration, int defaultTorusSize );
        virtual ~MigrationInfoFactoryVectorDefault();

        // IMigrationInfoFactoryVector
        virtual void ReadConfiguration( JsonConfigurable* pParent, const ::Configuration* config ) {};
        virtual IMigrationInfoVector* CreateMigrationInfoVector( 
            const std::string& idreference,
            INodeContext *parent_node, 
            const boost::bimap<ExternalNodeId_t, suids::suid>& rNodeIdSuidMap ) override;

    protected:

    private:
        bool m_IsVectorMigrationEnabled;
        int  m_TorusSize;
    };
}
