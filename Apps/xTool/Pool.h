// Pool.h
//

/////////////////////////////////////////////////////////////////////////////
// CPool
//
//  Simple pooled allocator of objects that are never deallocated

#define CPOOL_BANK_SIZE 1000

template <class T>
class CPool
{
    xarray<T*>  m_Banks;
    s32         m_BankSize;
    s32         m_iNextInBank;

    void NewBank( void )
    {
        T* pBank = new T[m_BankSize];
        ASSERT( pBank );
        m_Banks.Append( pBank );
        m_iNextInBank = 0;
    }

public:
    CPool( void )
    {
        m_BankSize = CPOOL_BANK_SIZE;
        NewBank();
    }

    ~CPool( void )
    {
        while( m_Banks.GetCount() > 0 )
        {
            T* pBank = m_Banks[0];
            ASSERT( pBank );
            delete[] pBank;
            m_Banks.Delete( 0 );
        }
    }

    T* New( void )
    {
        T* pItem = &m_Banks[m_Banks.GetCount()-1][m_iNextInBank];

        if( ++m_iNextInBank == m_BankSize )
            NewBank();

        return pItem;
    }

    T* NewArray( s32 Count, s32& Index )
    {
        ASSERT( Count <= m_BankSize );

        if( (m_BankSize - m_iNextInBank) < Count )
            NewBank();

        T* pItem = &m_Banks[m_Banks.GetCount()-1][m_iNextInBank];

        Index = (m_Banks.GetCount()-1)*m_BankSize + m_iNextInBank;

        m_iNextInBank += Count;

        return pItem;
    }

    T& operator[]( s32 Index )
    {
        s32 iBank   = Index / m_BankSize;
        s32 iEntry  = Index % m_BankSize;
        return m_Banks[iBank][iEntry];
    }

    void Serialize( CArchive& ar )
    {
        if( ar.IsStoring() )
        {
            ar << (s32)m_BankSize;

            ar << (s32)m_Banks.GetCount();
            for( s32 i=0 ; i<m_Banks.GetCount() ; i++ )
            {
                ar.Write( m_Banks[i], sizeof(T)*m_BankSize );
            }

            ar << m_iNextInBank;
        }
        else
        {
            s32 nBanks;

            while( m_Banks.GetCount() > 0 )
            {
                T* pBank = m_Banks[0];
                ASSERT( pBank );
                delete[] pBank;
                m_Banks.Delete( 0 );
            }

            ar >> m_BankSize;

            ar >> nBanks;
            for( s32 i=0 ; i<nBanks ; i++ )
            {
                T* pBank = new T[m_BankSize];
                ASSERT( pBank );
                m_Banks.Append( pBank );
                ar.Read( pBank, sizeof(T)*m_BankSize );
            }

            ar >> m_iNextInBank;
        }
    }
};

/////////////////////////////////////////////////////////////////////////////
