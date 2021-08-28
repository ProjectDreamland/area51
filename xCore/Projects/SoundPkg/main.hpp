#ifndef __MAIN_HPP
#define __MAIN_HPP

enum target_type
{
    TARGET_TYPE_UNDEFINED = -1,
    TARGET_TYPE_PS2,
    TARGET_TYPE_PC,
    TARGET_TYPE_XBOX,
    TARGET_TYPE_GCN,
};

target_type     GetTargetPlatform(void);
void            SetTargetPlatform(target_type TargetType);

s8      TargetEndian8(s8 h);
s16     TargetEndian16(s16 h);
s32     TargetEndian32(s32 h);
#endif