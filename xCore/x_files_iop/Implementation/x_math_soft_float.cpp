#include "x_target.hpp"
#include "x_files.hpp"
#include "x_math_soft_float.hpp"


static char s_print_buff[256];
static s32	s_print_buff_index;

//-----------------------------------------------------------
// Constructors
//-----------------------------------------------------------

f32::f32(float V)

{
	s32 mantissa;
	s32 exponent;
	x_ieee_float f;
	f.f = V;
	mantissa = f.parts.mantissa;
	exponent =  (23-X_FIXED_ACCURACY) - f.parts.exponent + 127;
	if (f.parts.exponent != 0)
		mantissa |= (1<<23);
	if (exponent < 0)
		whole = mantissa << -exponent;
	else						
		whole = mantissa >> exponent;
	if (f.parts.sign)			
		whole = -whole;
}

static char* s_PrintPart( char* ptr, u32 v)
{
	s32		r;
	u32		mult;
	xbool	flag;
	u32		start;


	start = 1;

	while (v > start*10)
	{
		start*=10;
	}

	mult = start;
	flag = FALSE;

	while (mult >= 10)
	{
		r = v/mult;
		if ( r || flag)
		{
			*ptr++ = '0'+r;
			v-= r*mult;
			flag = TRUE;
		}
		mult /= 10;
	}
	*ptr++ = '0' + v;
	return ptr;
}

f32::operator char*( void ) const
{
	char* ptr;
	char* pString;
	f32 t;

	t.whole = whole;

	// Max of 16 characters to be displayed per cast
	if ( (s_print_buff_index > 256-16) || (s_print_buff_index < 0) )
	{
		s_print_buff_index = 0;
	}

	pString = ptr = s_print_buff+s_print_buff_index;

	// Do integer portion of conversion
	if (t.parts.i < 0)
	{
		*ptr++='-';
		t.whole = -t.whole;
	}

	ptr = s_PrintPart(ptr,t.parts.i);
	if (t.parts.f)
	{
		*ptr++='.';
		ptr = s_PrintPart(ptr,1000*t.parts.f/256);
	}
	*ptr = 0x0;

	s_print_buff_index += x_strlen(pString)+1;
	return pString;
}
