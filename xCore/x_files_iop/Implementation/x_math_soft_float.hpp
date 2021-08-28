#ifndef X_MATH_SOFT_FLOAT_HPP
#define X_MATH_SOFT_FLOAT_HPP

#include "x_target.hpp"
#include "x_types.hpp"

#if !defined(TARGET_PS2_IOP)
//#error "This should only be compiled for the PS2 software float emulation"
#endif


#define X_FIXED_ACCURACY	8
#define f64 float


	// Union with the float values all split in to pieces!
union x_ieee_float
{
	float f;
	struct
	{
		unsigned int mantissa:23;
		unsigned int exponent:8;
		unsigned int sign:1;
	} parts;
};


class f32
{

public:
		//-----------------------------------------------------------
		// Constructors
		//-----------------------------------------------------------
		f32(s32 V)																		{ parts.i = V; parts.f = 0;				};
		f32(s64 V)																		{ parts.i = V; parts.f = 0;				};
		f32()																			{										};
		f32(float);
		f32(double)																		{										}

		//-----------------------------------------------------------
		// Unary operators
		//-----------------------------------------------------------
				
				inline	f32			operator -		( void							) const		{ f32 T; T.whole = -whole; return T;	};
				inline	f32			operator +		( void							) const		{ f32 T; T.whole = whole; return T;		};

		//-----------------------------------------------------------
		// Assignment Operators
		//-----------------------------------------------------------

		//-----------------------------------------------------------
		// Arithmetic assignment operators
		//-----------------------------------------------------------
		inline const	f32&		operator --     ( void							)			{ parts.i--;			return *this;	};		
		inline const	f32&		operator ++     ( void							)			{ parts.i++;			return *this;	};		

		// Operators for f32 right parameter
		inline const	f32&		operator +=     ( const f32 V1					)			{ whole += V1.whole;	return *this;	};		
		inline const	f32&		operator -=     ( const f32 V1					)			{ whole -= V1.whole;	return *this;	};		
        inline const	f32&        operator *=		( const f32 V1  				)			{ *this = *this * V1;	return *this;	};
        inline const	f32&        operator /=		( const f32 V1  				)			{ *this = *this / V1;	return *this;	};

		// Operators for s32 right parameter
		inline const	f32&		operator +=     ( const s32 V1					)			{ parts.i += V1;		return *this;	};
		inline const	f32&		operator -=     ( const s32 V1					)			{ parts.i -= V1;		return *this;	};
        inline const	f32&        operator *=		( const s32 V1  				)			{ whole *= V1;			return *this;	};
        inline const	f32&        operator /=		( const s32 V1  				)			{ whole /= V1;			return *this;	};

		// Operators for float right parameter
		inline const	f32&		operator +=     ( const float V1				)			{ f32 T(V1); *this += T; return *this;	};
		inline const	f32&		operator -=     ( const float V1				)			{ f32 T(V1); *this -= T; return *this;	};
        inline const	f32&        operator *=		( const float V1  				)			{ f32 T(V1); *this *= T; return *this;  };
        inline const	f32&        operator /=		( const float V1  				)			{ f32 T(V1); *this /= T; return *this;  };



		//-----------------------------------------------------------
		// Arithmetic operators (defined in x_math_soft_float.cpp)
		//-----------------------------------------------------------
		// Operators for f32 right parameter
				inline	f32			operator +		( const f32& V					) const		{ f32 T; T.whole = whole + V.whole; return T; };
				inline	f32			operator -		( const f32& V					) const		{ f32 T; T.whole = whole - V.whole; return T; };
				inline	f32			operator *		( const f32& V					) const		{ f32 T; T.whole = (whole * V.whole) >> X_FIXED_ACCURACY; return T;};
				inline	f32			operator /		( const f32& V					) const		{ f32 T; T.whole = (whole << X_FIXED_ACCURACY) / V.whole; return T;};

		// Operators for float left parameter
		inline friend	f32			operator +		( const float V1, const f32& V2 ) 			{ f32 T(V1); T = T+V2;	return T; };
		inline friend	f32			operator -		( const float V1, const f32& V2 )			{ f32 T(V1); T = T-V2;	return T; };
		inline friend	f32			operator *		( const float V1, const f32& V2 )			{ f32 T(V1); T = T*V2;	return T; };
		inline friend	f32			operator /		( const float V1, const f32& V2 )			{ f32 T(V1); T = T/V2;	return T; };

		// Operators for float right parameter
		inline friend	f32			operator +		( const f32   V1, const float& V2 )			{ f32 T(V2); T = V1+T;	return T; };
		inline friend	f32			operator -		( const f32   V1, const float& V2 )			{ f32 T(V2); T = V1-T;	return T; };
		inline friend	f32			operator *		( const f32	  V1, const float& V2 )			{ f32 T(V2); T = V1*T;	return T; };
		inline friend	f32			operator /		( const f32   V1, const float& V2 )			{ f32 T(V2); T = V1/T;	return T; };


		//-----------------------------------------------------------
		// Comparison operators
		//-----------------------------------------------------------
		friend  xbool       operator ==     ( const f32&  V1, const f32&  V2 )			{ return V1.whole == V2.whole;	};
		friend  xbool       operator !=     ( const f32&  V1, const f32&  V2 )			{ return V1.whole != V2.whole;	};
		friend  xbool       operator <      ( const f32&  V1, const f32&  V2 )			{ return V1.whole <  V2.whole;	};
		friend  xbool       operator >      ( const f32&  V1, const f32&  V2 )			{ return V1.whole >  V2.whole;	};
		friend  xbool       operator <=     ( const f32&  V1, const f32&  V2 )			{ return V1.whole <= V2.whole;	};
		friend  xbool       operator >=     ( const f32&  V1, const f32&  V2 )			{ return V1.whole >= V2.whole;	};
                                    
		friend  xbool       operator ==     ( const f32&  V1, s32     V2 )				{ return V1.whole == V2;		};
		friend  xbool       operator !=     ( const f32&  V1, s32     V2 )				{ return V1.whole != V2;		};
		friend  xbool       operator <      ( const f32&  V1, s32     V2 )				{ return V1.whole <  V2;		};
		friend  xbool       operator >      ( const f32&  V1, s32     V2 )				{ return V1.whole >  V2;		};
		friend  xbool       operator <=     ( const f32&  V1, s32     V2 )				{ return V1.whole <= V2;		};
		friend  xbool       operator >=     ( const f32&  V1, s32     V2 )				{ return V1.whole >= V2;		};
                                    
		friend  xbool       operator ==     ( s32     V1, const f32&  V2 )				{ return V1	== V2.whole;		};
		friend  xbool       operator !=     ( s32     V1, const f32&  V2 )				{ return V1 != V2.whole;		};
		friend  xbool       operator <      ( s32     V1, const f32&  V2 )				{ return V1 <  V2.whole;		};
		friend  xbool       operator >      ( s32     V1, const f32&  V2 )				{ return V1 >  V2.whole;		};
		friend  xbool       operator <=     ( s32     V1, const f32&  V2 )				{ return V1 <= V2.whole;		};
		friend  xbool       operator >=     ( s32     V1, const f32&  V2 )				{ return V1 >= V2.whole;		};
                                    
		//-----------------------------------------------------------
		// Type casting operators						    
		//-----------------------------------------------------------
							operator s32	( void							) const		{ return parts.i;				};
							operator bool	( void							) const		{ return whole != 0;			};
							operator float	( void							) const		{ return whole;					};
							operator char*	( void							) const;

private:
	union
	{
		s32 whole;			// 32 bit version
		struct
		{
			unsigned int	f:X_FIXED_ACCURACY;			// Fractional part
			int				i:(32-X_FIXED_ACCURACY);			// Integer part
		} parts;
	};
};


inline f32 x_pow(f32 val, f32 exp)
{
	return (val+exp /0.0f);
}
#endif

