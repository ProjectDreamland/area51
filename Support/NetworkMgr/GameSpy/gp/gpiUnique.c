/*
gpiUnique.c
GameSpy Presence SDK 
Dan "Mr. Pants" Schoenblum

Copyright 1999-2003 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

***********************************************************************
Please see the GameSpy Presence SDK documentation for more information
**********************************************************************/

//INCLUDES
//////////
#include "gpi.h"

//FUNCTIONS
///////////
static GPResult
gpiSendRegisterUniqueNick(
  GPConnection * connection,
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char cdkey[GP_CDKEY_LEN],
  int operationid
)
{
	GPIConnection * iconnection = (GPIConnection*)*connection;

	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\registernick\\\\sesskey\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, iconnection->sessKey);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\uniquenick\\");
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, uniquenick);
	if(cdkey)
	{
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\cdkey\\");
		gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, cdkey);
	}
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\id\\");
	gpiAppendIntToBuffer(connection, &iconnection->outputBuffer, operationid);
	gpiAppendStringToBuffer(connection, &iconnection->outputBuffer, "\\final\\");
	
	return GP_NO_ERROR;
}

GPResult gpiRegisterUniqueNick(
  GPConnection * connection,
  const char uniquenick[GP_UNIQUENICK_LEN],
  const char cdkey[GP_CDKEY_LEN],
  GPEnum blocking,
  GPCallback callback,
  void * param
)
{
	GPIOperation * operation = NULL;
	GPResult result;

	// Add the operation.
	/////////////////////
	CHECK_RESULT(gpiAddOperation(connection, GPI_REGISTER_UNIQUENICK, NULL, &operation, blocking, callback, param));

	// Send a request for info.
	///////////////////////////
	result = gpiSendRegisterUniqueNick(connection, uniquenick, cdkey, operation->id);
	CHECK_RESULT(result);

	// Process it if blocking.
	//////////////////////////
	if(blocking)
	{
		result = gpiProcess(connection, operation->id);
		CHECK_RESULT(result);
	}

	return GP_NO_ERROR;
}

GPResult gpiProcessRegisterUniqueNick(
  GPConnection * connection,
  GPIOperation * operation,
  const char * input
)
{
	GPICallback callback;

	// Check for an error.
	//////////////////////
	if(gpiCheckForError(connection, input, GPITrue))
		return GP_SERVER_ERROR;

	// This should be \rn\.
	///////////////////////
	if(strncmp(input, "\\rn\\", 4) != 0)
		CallbackFatalError(connection, GP_NETWORK_ERROR, GP_PARSE, "Unexpected data was received from the server.");

	// Call the callback.
	/////////////////////
	callback = operation->callback;
	if(callback.callback != NULL)
	{
		GPRegisterUniqueNickResponseArg * arg;
		arg = (GPRegisterUniqueNickResponseArg *)gsimalloc(sizeof(GPRegisterUniqueNickResponseArg));
		if(arg == NULL)
			Error(connection, GP_MEMORY_ERROR, "Out of memory.");

		arg->result = GP_NO_ERROR;

		CHECK_RESULT(gpiAddCallback(connection, callback, arg, operation, 0));
	}

	// This operation is complete.
	//////////////////////////////
	gpiRemoveOperation(connection, operation);

	return GP_NO_ERROR;
}
