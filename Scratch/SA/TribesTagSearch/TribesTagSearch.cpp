// TribesTagSearch.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "string.h"


const char* ClanMembers[16] = 
{
	"RivDog.POW",
	"Nytstalker.POW",
	"D-Railer.POW",
	"Gentle.POW",
	"JDog23.POW",
	"Mobias One.POW",
	"The One.POW",
	"Harrigan.POW"
/*
"EC.SkyWalk3r",
"EC.JAXON",
"EC.ME VOILA", 
"EC.GUNNER",
"EC.BOOGIEMAN",
"EC.SKELLINGTON", 
"EC.HIGHWIND",
"EC.KT SLAYER",
*/
};
/*
	"ResidntEvil.POW",
	"ASSAULT.POW",
	"Slayer.POW",
	"WILLY.POW",
	"Shin0da.POW",
	"crickett.POW",
	"MAYHEM.POW",
	"SKULLI.POW"

"KRILL-DOG!-DBZ"
"KAKAROT!-DBZ"
"SANCHO!-DBZ"
"KNOWONE!-DBZ" 
"ORBIC!-DBZ"
"ADMUSHI!-DBZ"
"DEFENDER!-DBZ" 
"CLONEFAN94!-DBZ"
"INVISABO!-DBZ"
"Assasin!-DBZ"
"Relic!-DBZ"
"TheFid!-DBZ" 
"OptiKal!-DBZ"

"EC.B CAMARO",
"EC.YUNG GUNZ",
"EC.MASTER YOD",


};
*/
typedef int					s32;
typedef bool				xbool;
#define MAX_PLAYER_COUNT	8

//====================================================================

s32 GetMatchCount( char pTag[8][3], s32 Col, s32 Row, char* pFinalTag, s32 FinalLen )
{
	s32   TagMatchCount = 0;
	s32 LastMatchingIndex = 0;
	for( s32 i = 0; i < Col; i++ )
	{
		for( s32 j = i+1; j < Col; j++ )
		{
			if( !strncmp( pTag[i], pTag[j], Row ) )
			{
				TagMatchCount++;
				LastMatchingIndex = i;
			}
		}
		if( TagMatchCount >= 6 )
		{
			strncpy( pFinalTag, pTag[ LastMatchingIndex ], FinalLen );
			return TagMatchCount;
		}
		else if( TagMatchCount >= 3 || TagMatchCount < 6 )
		{
			return 0;
		}
	}
	return 0;
}

//====================================================================

s32 FirstandLastCharacterCheck( void )
{
	char CheckTag[3]   = { 0, 0, 0 };
	s32  CheckTagCount = 0;

	// Check the first character to match.
	for( s32 i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		if( CheckTagCount == 3 )
			break;
		
		xbool MatchCheck = false;
		for( s32 j = 0; j <= CheckTagCount; j++ )
		{
			if( CheckTag[ j ] == ClanMembers[i][0] )
				MatchCheck = true;
		}

		if( !MatchCheck )
		{
			CheckTag[ CheckTagCount ] = ClanMembers[i][0];
			CheckTagCount++;
		}
	}
	
	if( CheckTagCount < 3 )
		return 1;

	CheckTagCount = 0;
	CheckTag[0]   = 0;
	CheckTag[1]   = 0;
	CheckTag[2]   = 0;
	// If the first character check fails check the last character to match.
	// Check the first character to match.
	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		if( CheckTagCount == 3 )
			return 0;
		
		xbool MatchCheck = false;
		for( s32 j = 0; j <= CheckTagCount; j++ )
		{
			if( CheckTag[ j ] == ClanMembers[i][ strlen(ClanMembers[i]) - 1] )
				MatchCheck = true;
		}

		if( !MatchCheck )
		{
			CheckTag[ CheckTagCount ] = ClanMembers[i][ strlen(ClanMembers[i])-1 ];
			CheckTagCount++;
		}
	}
	
	return -1;
}

//====================================================================

int main(int argc, char* argv[])
{
	// if( player_count < 4 )
	// return;
		
	s32 StartIndex = FirstandLastCharacterCheck();
	char  Tag[8][3];
	char  FinalTag[3];
	s32 i = 0;
	
	if( !StartIndex )
		return 0;

	
	for( i = 0; i < MAX_PLAYER_COUNT; i++ )
	{
		s32 SpaceIncrement = 0;			
		s32 NameStartIndex = strlen( ClanMembers[i] ) - 1;
 
		for( s32 j = 0; j < 3; j++ )
		{				
			s32 Index = j;
			
			if( StartIndex == -1 )
            {
				Index = NameStartIndex - (2-j);

			    // We don't care about the spaces so just skip over them.
			    while( ClanMembers[i][ Index - SpaceIncrement ] == ' ' )
				    SpaceIncrement--;

                Tag[i][j] = ClanMembers[i][ Index - SpaceIncrement ];
            }
            else
            {
			    // We don't care about the spaces so just skip over them.
			    while( ClanMembers[i][ Index - SpaceIncrement ] == ' ' )
				    SpaceIncrement++;
                
                Tag[i][j] = ClanMembers[i][ Index + SpaceIncrement ];
            }
			
			
		}

	}
	
	s32 MatchCount = GetMatchCount( Tag, 8, 3, FinalTag, 3 );
	if( MatchCount )
	{
		s32* NameIndex = new s32 [MatchCount];
		s32 Start = 0;
		s32 i = 0;
		for( i = 0; i < MAX_PLAYER_COUNT; i++ )
		{
			if( !strncmp( FinalTag, Tag[i], 3) )
			{
				NameIndex[ Start ] = i;
				Start++;
			}
		}
		
		xbool TagEnd = false;
		s32 TagStartIndex = 4;
		i = 4;
		while( !TagEnd )
		{
			TagStartIndex = i;

			s32 NameLen = strlen( ClanMembers[NameIndex[0]] );
			// This should never happen.
			if( NameLen <= i )
				break;

			if( StartIndex == -1 )
				TagStartIndex = NameLen - i;

			char TagChar = ClanMembers[ NameIndex[0] ][ TagStartIndex ];

			for( s32 j = 0; j < MatchCount; j++ )
			{
				NameLen = strlen( ClanMembers[NameIndex[j]] );

				// This should never happen.
				if( NameLen <= i )
				{
					TagEnd = true;
					break;
				}

				if( StartIndex == -1 )
					TagStartIndex =  NameLen - i;

				if( TagChar != ClanMembers[ NameIndex[j] ][ TagStartIndex ] )
				{
					TagEnd = true;
					break;
				}
			}
			i++;
		}

        char* ClanTag = ClanMembers[NameIndex[MatchCount-1]
	}

	return 0;
}

