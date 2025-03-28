
#include "stdafx.h"

#include "ConcurrencyParameters.h"
#include "demographic_params.rc"
#include "Debug.h"
#include "Environment.h"
#include "Log.h"
#include "RANDOM.h"
#include "Properties.h"

SETUP_LOGGING( "ConcurrencyParameters" )


#define DEFAULT_PROPERTY "NONE"

namespace Kernel
{
    // ------------------------------------------------------------------------
    // --- ConcurrencyByProperty
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY(ConcurrencyByProperty)
    END_QUERY_INTERFACE_BODY(ConcurrencyByProperty)

    ConcurrencyByProperty::ConcurrencyByProperty( const std::string& propertyKeyValue )
    : JsonConfigurable()
    , m_PropertyValue( propertyKeyValue )
    , m_ProbExtraRelsMale( 0.0f )
    , m_ProbExtraRelsFemale( 0.0f )
    , m_MaxSimultaneiousRelsMale( 0 )
    , m_MaxSimultaneiousRelsFemale( 0 )
    {
    }

    ConcurrencyByProperty::~ConcurrencyByProperty()
    {
    }

    bool ConcurrencyByProperty::Configure( const ::Configuration *json )
    {
        initConfigTypeMap( "Prob_Extra_Relationship_Male",   &m_ProbExtraRelsMale,   Prob_Extra_Relationship_Male_DESC_TEXT,   0.0, 1.0f, 0.0f );
        initConfigTypeMap( "Prob_Extra_Relationship_Female", &m_ProbExtraRelsFemale, Prob_Extra_Relationship_Female_DESC_TEXT, 0.0, 1.0f, 0.0f );

        initConfigTypeMap( "Max_Simultaneous_Relationships_Male",   &m_MaxSimultaneiousRelsMale,   Max_Simultaneous_Relationships_Male_DESC_TEXT,   0, MAX_SLOTS, 1 );
        initConfigTypeMap( "Max_Simultaneous_Relationships_Female", &m_MaxSimultaneiousRelsFemale, Max_Simultaneous_Relationships_Female_DESC_TEXT, 0, MAX_SLOTS, 1 );

        bool ret = JsonConfigurable::Configure( json );
        return ret;
    }

    // ------------------------------------------------------------------------
    // --- ConcurrencyParameters
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY(ConcurrencyParameters)
    END_QUERY_INTERFACE_BODY(ConcurrencyParameters)

    ConcurrencyParameters::ConcurrencyParameters()
    : JsonConfigurable()
    , m_PropertyValueToConcurrencyMap()
    {
    }

    ConcurrencyParameters::~ConcurrencyParameters()
    {
        for( auto& entry : m_PropertyValueToConcurrencyMap )
        {
            delete entry.second;
        }
    }

    void ConcurrencyParameters::Initialize( const std::string& rRelTypeName,
                                            const std::string& rConcurrencyProperty, 
                                            const ::Configuration *json )
    {
        if( rConcurrencyProperty == DEFAULT_PROPERTY )
        {
            ConcurrencyByProperty* p_cbp = new ConcurrencyByProperty( DEFAULT_PROPERTY );
            m_PropertyValueToConcurrencyMap[ DEFAULT_PROPERTY ] = p_cbp;

            initConfigTypeMap( DEFAULT_PROPERTY, p_cbp, Concurrency_By_Property_NONE_DESC_TEXT );
        }
        else
        {
            const IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( rConcurrencyProperty );

            for( auto kv : p_ip->GetValues<IPKeyValueContainer>()  )
            {
                const std::string& prop_value = kv.GetValueAsString();

                //if( !json->Exist( prop_value ) )
                //{
                //    std::stringstream ss;
                //    ss << "Each type of relationship's Concurrency_Parameters must have all of the Individual Properties defined.  Property Key = " << rConcurrencyProperty;
                //    ss << ", Not Defined Property Value = " << prop_value << " in element = " << rRelTypeName ;
                //    throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                //}

                ConcurrencyByProperty* p_cbp = new ConcurrencyByProperty( prop_value );
                m_PropertyValueToConcurrencyMap[ prop_value ] = p_cbp;

                initConfigTypeMap( prop_value.c_str(), p_cbp, Concurrency_By_Property_IP_DESC_TEXT );
            }
        }
        try
        {
            JsonConfigurable::Configure( json );
        }
        catch( DetailedException& re )
        {
            std::stringstream ss ;
            ss << re.GetMsg();
            ss << " Occured while reading the 'Concurrency_Parameters' in '" << rRelTypeName << "' from the demographics.  ";
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
    }

    float ConcurrencyParameters::GetProbExtra( const char* prop_value, Gender::Enum gender ) const
    {
        if( gender == Gender::MALE )
        {
            return m_PropertyValueToConcurrencyMap.at( prop_value )->GetProbExtraRelsMale();
        }
        else
        {
           return  m_PropertyValueToConcurrencyMap.at( prop_value )->GetProbExtraRelsFemale();
        }
    }

    float ConcurrencyParameters::GetMaxRels( const char* prop_value, Gender::Enum gender ) const
    {
        if( gender == Gender::MALE )
        {
            return m_PropertyValueToConcurrencyMap.at( prop_value )->GetMaxSimultaneiousRelsMale();
        }
        else
        {
            return m_PropertyValueToConcurrencyMap.at( prop_value )->GetMaxSimultaneiousRelsFemale();
        }
    }

    // ------------------------------------------------------------------------
    // --- ConcurrencyConfigurationByProperty
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY(ConcurrencyConfigurationByProperty)
    END_QUERY_INTERFACE_BODY(ConcurrencyConfigurationByProperty)

    ConcurrencyConfigurationByProperty::ConcurrencyConfigurationByProperty( const std::string& propertyValue )
    : JsonConfigurable()
    , m_PropretyValue( propertyValue )
    , m_ExtraRelFlag( ExtraRelationalFlagType::Independent )
    , m_RelTypeOrder()
    {
    }

    ConcurrencyConfigurationByProperty::~ConcurrencyConfigurationByProperty()
    {
    }

    bool ConcurrencyConfigurationByProperty::Configure( const ::Configuration *json )
    {
        initConfig( "Extra_Relational_Flag_Type", 
                    m_ExtraRelFlag,
                    json, 
                    MetadataDescriptor::Enum("Extra_Relational_Flag_Type", Extra_Relational_Flag_Type_DESC_TEXT, MDD_ENUM_ARGS( ExtraRelationalFlagType ) ) );

        bool ret = true;
        if( m_ExtraRelFlag == ExtraRelationalFlagType::Correlated )
        {
            std::set<std::string> allowable_relationship_types = GetAllowableRelationshipTypes();
            std::vector<std::string> rel_type_strings ;
            initConfigTypeMap( "Correlated_Relationship_Type_Order", &rel_type_strings, Correlated_Relationship_Type_Order_DESC_TEXT, nullptr, allowable_relationship_types );

            ret = JsonConfigurable::Configure( json );
            if( ret )
            {
                if( rel_type_strings.size() != RelationshipType::COUNT )
                {
                    std::stringstream ss;
                    ss << "Reading '" << m_PropretyValue << ": 'Correlated_Relationship_Type_Order' has " << rel_type_strings.size() << " types and must have " << RelationshipType::COUNT;
                    throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
                }

                m_RelTypeOrder = ConvertStringsToRelationshipTypes( "Correlated_Relationship_Type_Order",
                                                                    rel_type_strings );
            }
        }
        else
        {
            m_RelTypeOrder = GetRelationshipTypes();
        }
        return ret;
    }

    // ------------------------------------------------------------------------
    // --- ConcurrencyConfiguration
    // ------------------------------------------------------------------------
    BEGIN_QUERY_INTERFACE_BODY(ConcurrencyConfiguration)
    END_QUERY_INTERFACE_BODY(ConcurrencyConfiguration)

    ConcurrencyConfiguration::ConcurrencyConfiguration()
    : JsonConfigurable()
    , m_PropertyKey( DEFAULT_PROPERTY )
    , m_PropertyValueToConfig()
    , m_RelTypeToParametersMap()
    , m_ProbSuperSpreader(0.001f)
    {
    }

    ConcurrencyConfiguration::~ConcurrencyConfiguration()
    {
        for( auto& entry : m_PropertyValueToConfig )
        {
            delete entry.second;
        }
        for( auto& entry : m_RelTypeToParametersMap )
        {
            delete entry.second;
        }
    }

    void ConcurrencyConfiguration::Initialize( const ::Configuration *json )
    {
        initConfigTypeMap( "Probability_Person_Is_Behavioral_Super_Spreader", &m_ProbSuperSpreader, Probability_Person_Is_Behavioral_Super_Spreader_DESC_TEXT, 0.0, 1.0f, 0.001f );
        initConfigTypeMap( "Individual_Property_Name", &m_PropertyKey, Individual_Property_Name_DESC_TEXT, DEFAULT_PROPERTY );
        try
        {
            JsonConfigurable::Configure( json );
        }
        catch( DetailedException& re )
        {
            std::stringstream ss ;
            ss << re.GetMsg();
            ss << " Occured while reading 'Concurrency_Configuration' from the demographics.  ";
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }

        if( m_PropertyKey.empty() )
        {
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, "'Individual_Property_Name' cannot be empty string." );
        }
        else if( m_PropertyKey == DEFAULT_PROPERTY )
        {
            ConcurrencyConfigurationByProperty* p_ccbp = new ConcurrencyConfigurationByProperty( DEFAULT_PROPERTY );
            m_PropertyValueToConfig[ DEFAULT_PROPERTY ] = p_ccbp;
            initConfigTypeMap( DEFAULT_PROPERTY, p_ccbp, Concurrency_Configuration_By_Property_NONE_DESC_TEXT );
        }
        else
        {
            std::set<std::string> keys = IPFactory::GetInstance()->GetKeysAsStringSet();
            if( keys.count( m_PropertyKey ) == 0 )
            {
                std::stringstream ss;
                ss << "'Individual_Property_Name' (='" << m_PropertyKey << "') in 'Concurrency_Configuration' in the demograhics is not a defined Individual Property.\n";
                ss << "Valid Individual Properties are: ";
                if( keys.size() == 0 )
                {
                    ss << "(No properties are defined.)";
                }
                else
                {
                    for( auto& k : keys )
                    {
                        ss << "'" << k << "' ";
                    }
                }
                throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
            }

            const IndividualProperty* p_ip = IPFactory::GetInstance()->GetIP( m_PropertyKey );
            for( auto kv : p_ip->GetValues<IPKeyValueContainer>() )
            {
                const std::string& prop_value = kv.GetValueAsString();

                ConcurrencyConfigurationByProperty* p_ccbp = new ConcurrencyConfigurationByProperty( prop_value );
                m_PropertyValueToConfig[ prop_value ] = p_ccbp ;

                initConfigTypeMap( prop_value.c_str(), p_ccbp, Concurrency_Configuration_By_Property_IP_DESC_TEXT );
            }
        }
        try
        {
            JsonConfigurable::Configure( json );
        }
        catch( DetailedException& re )
        {
            std::stringstream ss ;
            ss << re.GetMsg();
            ss << " Occured while reading 'Concurrency_Configuration' from the demographics.  ";
            throw InvalidInputDataException( __FILE__, __LINE__, __FUNCTION__, ss.str().c_str() );
        }
    }

    float ConcurrencyConfiguration::GetProbSuperSpreader() const
    {
        return m_ProbSuperSpreader;
    }

    bool ConcurrencyConfiguration::IsConcurrencyProperty( const char* prop ) const
    {
        return ( (prop != nullptr) && (m_PropertyKey == prop) );
    }

    const char* ConcurrencyConfiguration::GetConcurrencyPropertyValue( const IPKeyValueContainer& rProperties, 
                                                                       const char* prop, 
                                                                       const char* prop_value ) const
    {
        if( m_PropertyKey == DEFAULT_PROPERTY )
        {
            return DEFAULT_PROPERTY;
        }
        else if( (prop == nullptr) || (prop_value == nullptr) )
        {
            IPKey key( m_PropertyKey );

            return rProperties.Get( key ).GetValueAsString().c_str();
        }
        else
        {
            release_assert( m_PropertyKey == prop );
            return prop_value;
        }
    }

    unsigned char ConcurrencyConfiguration::GetProbExtraRelationalBitMask( RANDOMBASE* pRNG,
                                                                           const char* prop,
                                                                           const char* prop_value, 
                                                                           Gender::Enum gender,
                                                                           bool isSuperSpreader ) const
    {
        release_assert( m_PropertyKey == prop );

        unsigned char ret = 0;

        if( isSuperSpreader ) 
        {
            for( int rel = 0; rel < RelationshipType::COUNT; rel++ )
            {
                ret |= EXTRA_RELATIONAL_ALLOWED( rel );
            }
        }
        else
        {
            release_assert( m_PropertyValueToConfig.count( prop_value ) > 0 );

            const ConcurrencyConfigurationByProperty* p_ccbp = m_PropertyValueToConfig.at( prop_value );

            for( int rel = 0; rel < RelationshipType::COUNT; rel++ )
            {
                RelationshipType::Enum rel_type = p_ccbp->GetRelationshipTypeOrder()[rel];
                float prob = m_RelTypeToParametersMap.at( rel_type )->GetProbExtra( prop_value, gender );

                if( pRNG->SmartDraw( prob ) )
                {
                    ret |= EXTRA_RELATIONAL_ALLOWED(rel_type);
                }
                else if( p_ccbp->GetExtraRelationshipFlag() != ExtraRelationalFlagType::Independent )
                {
                    return ret ;
                }
            }
        }
        return ret;
    }

    int ConcurrencyConfiguration::GetMaxAllowableRelationships( RANDOMBASE* pRNG,
                                                                const char* prop, 
                                                                const char* prop_value, 
                                                                Gender::Enum gender,
                                                                RelationshipType::Enum rel_type ) const
    {
        release_assert( m_PropertyKey == prop );

        float max_num = m_RelTypeToParametersMap.at( rel_type )->GetMaxRels( prop_value, gender );

        // -----------------------------------------------------------------------
        // --- randomly round to the nearest integer such that if max_num is 2.3, 
        // --- it returns two 70% of the time and three 30%.
        // -----------------------------------------------------------------------
        int max_rel = pRNG->randomRound( max_num );

        return max_rel;
    }

    const std::string& ConcurrencyConfiguration::GetPropertyKey() const
    {
        return m_PropertyKey;
    }

    void ConcurrencyConfiguration::AddParameters( RelationshipType::Enum relType, ConcurrencyParameters* pCP )
    {
        m_RelTypeToParametersMap[ relType ] = pCP ;
    }
}
