#ifndef IOPNET_CONFIG_H
#define IOPNET_CONFIG_H

#define MAX_CONFIG_NAME_COUNT   8
#define MAX_CONFIG_NAME_LENGTH  128


typedef struct s_net_config_list
{
    s32     Count;
    s8      Available[MAX_CONFIG_NAME_COUNT];
    char    Name[MAX_CONFIG_NAME_COUNT][MAX_CONFIG_NAME_LENGTH];
} net_config_list;

enum
{
    CR_NET_SET_CONFIG,
    CR_NET_ACTIVATE_CONFIG,
};

void    NetGetConfig        (char *pPath,s32 type,net_config_list *pConfigList);
s32     NetSetConfig        (char *pPath,s32 ConfigIndex);
s32     NetActivateConfig   (s32 on);
s32     NetGetAttachStatus  (void);

#endif