/*==========================================================================;
 *
 *  xonline.h -- This module defines the XBox Live APIs
 *
 *  Copyright (C) Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#ifndef __XONLINE__
#define __XONLINE__

#ifdef __cplusplus
extern "C" {
#endif


//
// XOnline Startup & Cleanup
//

typedef struct _XONLINE_STARTUP_PARAMS {

    DWORD           dwMaxPrivatePool;

} XONLINE_STARTUP_PARAMS, *PXONLINE_STARTUP_PARAMS;

XBOXAPI
HRESULT
WINAPI
XOnlineStartup(
    IN const XONLINE_STARTUP_PARAMS*  pxosp
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCleanup();


//
// XOnline Error Codes
//

#define FACILITY_XONLINE                                21

// Generic Errors                                       = 0x80150XXX
#define XONLINE_E_OVERFLOW                              _HRESULT_TYPEDEF_(0x80150001L)
#define XONLINE_E_NO_SESSION                            _HRESULT_TYPEDEF_(0x80150002L)
#define XONLINE_E_USER_NOT_LOGGED_ON                    _HRESULT_TYPEDEF_(0x80150003L)
#define XONLINE_E_NO_GUEST_ACCESS                       _HRESULT_TYPEDEF_(0x80150004L)
#define XONLINE_E_NOT_INITIALIZED                       _HRESULT_TYPEDEF_(0x80150005L)
#define XONLINE_E_NO_USER                               _HRESULT_TYPEDEF_(0x80150006L)
#define XONLINE_E_INTERNAL_ERROR                        _HRESULT_TYPEDEF_(0x80150007L)
#define XONLINE_E_OUT_OF_MEMORY                         _HRESULT_TYPEDEF_(0x80150008L)
#define XONLINE_E_TASK_BUSY                             _HRESULT_TYPEDEF_(0x80150009L)
#define XONLINE_E_SERVER_ERROR                          _HRESULT_TYPEDEF_(0x8015000AL)
#define XONLINE_E_IO_ERROR                              _HRESULT_TYPEDEF_(0x8015000BL)
#define XONLINE_E_BAD_CONTENT_TYPE                      _HRESULT_TYPEDEF_(0x8015000CL)
#define XONLINE_E_USER_NOT_PRESENT                      _HRESULT_TYPEDEF_(0x8015000DL)
#define XONLINE_E_PROTOCOL_MISMATCH                     _HRESULT_TYPEDEF_(0x8015000EL)
#define XONLINE_E_INVALID_SERVICE_ID                    _HRESULT_TYPEDEF_(0x8015000FL)
#define XONLINE_E_INVALID_REQUEST                       _HRESULT_TYPEDEF_(0x80150010L)
#define XONLINE_E_TASK_THROTTLED                        _HRESULT_TYPEDEF_(0x80150011L)
#define XONLINE_E_TASK_ABORTED_BY_DUPLICATE             _HRESULT_TYPEDEF_(0x80150012L)
#define XONLINE_E_INVALID_TITLE_ID                      _HRESULT_TYPEDEF_(0x80150013L)

// Failures from XOnlineLogon                           = 0x801510XX
#define XONLINE_E_LOGON_NO_NETWORK_CONNECTION           _HRESULT_TYPEDEF_(0x80151000L)

// XOnlineLogon task successful return states
#define XONLINE_S_LOGON_CONNECTION_ESTABLISHED          _HRESULT_TYPEDEF_(0x001510F0L)

// XOnlineLogon task failure return values
#define XONLINE_E_LOGON_CANNOT_ACCESS_SERVICE           _HRESULT_TYPEDEF_(0x80151001L)
#define XONLINE_E_LOGON_UPDATE_REQUIRED                 _HRESULT_TYPEDEF_(0x80151002L)
#define XONLINE_E_LOGON_SERVERS_TOO_BUSY                _HRESULT_TYPEDEF_(0x80151003L)
#define XONLINE_E_LOGON_CONNECTION_LOST                 _HRESULT_TYPEDEF_(0x80151004L)
#define XONLINE_E_LOGON_KICKED_BY_DUPLICATE_LOGON       _HRESULT_TYPEDEF_(0x80151005L)
#define XONLINE_E_LOGON_INVALID_USER                    _HRESULT_TYPEDEF_(0x80151006L)

// Failures from XOnlineSilentLogon
#define XONLINE_E_SILENT_LOGON_DISABLED                 _HRESULT_TYPEDEF_(0x80151080L)
#define XONLINE_E_SILENT_LOGON_NO_ACCOUNTS              _HRESULT_TYPEDEF_(0x80151081L)
#define XONLINE_E_SILENT_LOGON_PASSCODE_REQUIRED        _HRESULT_TYPEDEF_(0x80151082L)

// Service errors after XOnlineLogon task completion    = 0x801511XX
#define XONLINE_E_LOGON_SERVICE_NOT_REQUESTED           _HRESULT_TYPEDEF_(0x80151100L)
#define XONLINE_E_LOGON_SERVICE_NOT_AUTHORIZED          _HRESULT_TYPEDEF_(0x80151101L)
#define XONLINE_E_LOGON_SERVICE_TEMPORARILY_UNAVAILABLE _HRESULT_TYPEDEF_(0x80151102L)

// User warnings after XOnlineLogon task completion     = 0x801512XX
#define XONLINE_S_LOGON_USER_HAS_MESSAGE                _HRESULT_TYPEDEF_(0x001512F0L)

// User errors after XOnlineLogon task completion
#define XONLINE_E_LOGON_USER_ACCOUNT_REQUIRES_MANAGEMENT _HRESULT_TYPEDEF_(0x80151200L)

// XOnlineChangeLogonUsers task successful return states = 0x801513XX
#define XONLINE_S_LOGON_COMMIT_USER_CHANGE              _HRESULT_TYPEDEF_(0x001513F0L)
#define XONLINE_S_LOGON_USER_CHANGE_COMPLETE            _HRESULT_TYPEDEF_(0x001513F1L)

// XOnlineChangeLogonUsers task failure return values   
#define XONLINE_E_LOGON_CHANGE_USER_FAILED              _HRESULT_TYPEDEF_(0x80151300L)

// Other generic auth related errors                    = 0x801518XX
#define XONLINE_E_LOGON_MU_NOT_MOUNTED                  _HRESULT_TYPEDEF_(0x80151800L)
#define XONLINE_E_LOGON_MU_IO_ERROR                     _HRESULT_TYPEDEF_(0x80151801L)
#define XONLINE_E_LOGON_NOT_LOGGED_ON                   _HRESULT_TYPEDEF_(0x80151802L)


// Errors returned by Presence/Notification             = 0x801520XX
#define XONLINE_E_NOTIFICATION_SERVER_BUSY              _HRESULT_TYPEDEF_(0x80152001L)
#define XONLINE_E_NOTIFICATION_LIST_FULL                _HRESULT_TYPEDEF_(0x80152002L)
#define XONLINE_E_NOTIFICATION_BLOCKED                  _HRESULT_TYPEDEF_(0x80152003L)
#define XONLINE_E_NOTIFICATION_FRIEND_PENDING           _HRESULT_TYPEDEF_(0x80152004L)
#define XONLINE_E_NOTIFICATION_FLUSH_TICKETS            _HRESULT_TYPEDEF_(0x80152005L)
#define XONLINE_E_NOTIFICATION_TOO_MANY_REQUESTS        _HRESULT_TYPEDEF_(0x80152006L)
#define XONLINE_E_NOTIFICATION_USER_ALREADY_EXISTS      _HRESULT_TYPEDEF_(0x80152007L)
#define XONLINE_E_NOTIFICATION_USER_NOT_FOUND           _HRESULT_TYPEDEF_(0x80152008L)
#define XONLINE_E_NOTIFICATION_OTHER_LIST_FULL          _HRESULT_TYPEDEF_(0x80152009L)
#define XONLINE_E_NOTIFICATION_SELF                     _HRESULT_TYPEDEF_(0x8015200AL)
#define XONLINE_E_NOTIFICATION_SAME_TITLE               _HRESULT_TYPEDEF_(0x8015200BL)
#define XONLINE_E_NOTIFICATION_NO_TASK                  _HRESULT_TYPEDEF_(0x8015200CL)

// Errors returned by teams                             = 0x801521XX
#define XONLINE_E_TEAMS_SERVER_BUSY                     _HRESULT_TYPEDEF_(0x80152100L)
#define XONLINE_E_TEAMS_TEAM_FULL                       _HRESULT_TYPEDEF_(0x80152101L)
#define XONLINE_E_TEAMS_MEMBER_PENDING                  _HRESULT_TYPEDEF_(0x80152102L)
#define XONLINE_E_TEAMS_TOO_MANY_REQUESTS               _HRESULT_TYPEDEF_(0x80152103L)
#define XONLINE_E_TEAMS_USER_ALREADY_EXISTS             _HRESULT_TYPEDEF_(0x80152104L)
#define XONLINE_E_TEAMS_USER_NOT_FOUND                  _HRESULT_TYPEDEF_(0x80152105L)
#define XONLINE_E_TEAMS_USER_TEAMS_FULL                 _HRESULT_TYPEDEF_(0x80152106L)
#define XONLINE_E_TEAMS_SELF                            _HRESULT_TYPEDEF_(0x80152107L)
#define XONLINE_E_TEAMS_NO_TASK                         _HRESULT_TYPEDEF_(0x80152108L)
#define XONLINE_E_TEAMS_TOO_MANY_TEAMS                  _HRESULT_TYPEDEF_(0x80152109L)
#define XONLINE_E_TEAMS_TEAM_ALREADY_EXISTS             _HRESULT_TYPEDEF_(0x8015210AL)
#define XONLINE_E_TEAMS_TEAM_NOT_FOUND                  _HRESULT_TYPEDEF_(0x8015210BL)
#define XONLINE_E_TEAMS_INSUFFICIENT_PRIVILEGES         _HRESULT_TYPEDEF_(0x8015210CL)
#define XONLINE_E_TEAMS_NAME_CONTAINS_BAD_WORDS         _HRESULT_TYPEDEF_(0x8015210DL)
#define XONLINE_E_TEAMS_DESCRIPTION_CONTAINS_BAD_WORDS  _HRESULT_TYPEDEF_(0x8015210EL)
#define XONLINE_E_TEAMS_MOTTO_CONTAINS_BAD_WORDS        _HRESULT_TYPEDEF_(0x8015210FL)
#define XONLINE_E_TEAMS_URL_CONTAINS_BAD_WORDS          _HRESULT_TYPEDEF_(0x80152110L)
#define XONLINE_E_TEAMS_NOT_A_MEMBER                    _HRESULT_TYPEDEF_(0x80152111L)
#define XONLINE_E_TEAMS_NO_ADMIN                        _HRESULT_TYPEDEF_(0x80152112L)

// Errors returned by offering service                  = 0x801530XX + 0x801531XX
#define XONLINE_S_OFFERING_NEW_CONTENT                  _HRESULT_TYPEDEF_(0x00153101L)  // new content is available
#define XONLINE_S_OFFERING_NO_NEW_CONTENT               _HRESULT_TYPEDEF_(0x00153102L)  // no new content is available
#define XONLINE_E_OFFERING_BAD_REQUEST                  _HRESULT_TYPEDEF_(0x80153001L)  // server received incorrectly formatted request
#define XONLINE_E_OFFERING_INVALID_USER                 _HRESULT_TYPEDEF_(0x80153002L)  // cannot find account for this user
#define XONLINE_E_OFFERING_INVALID_OFFER_ID             _HRESULT_TYPEDEF_(0x80153003L)  // offer does not exist
#define XONLINE_E_OFFERING_INELIGIBLE_FOR_OFFER         _HRESULT_TYPEDEF_(0x80153004L)  // )] private /title not allowed to purchase offer
#define XONLINE_E_OFFERING_OFFER_EXPIRED                _HRESULT_TYPEDEF_(0x80153005L)  // offer no longer available
#define XONLINE_E_OFFERING_SERVICE_UNREACHABLE          _HRESULT_TYPEDEF_(0x80153006L)  // apparent connectivity problems
#define XONLINE_E_OFFERING_PURCHASE_BLOCKED             _HRESULT_TYPEDEF_(0x80153007L)  // this user is not allowed to make purchases
#define XONLINE_E_OFFERING_PURCHASE_DENIED              _HRESULT_TYPEDEF_(0x80153008L)  // this user's payment is denied by billing provider
#define XONLINE_E_OFFERING_BILLING_SERVER_ERROR         _HRESULT_TYPEDEF_(0x80153009L)  // nonspecific billing provider error
#define XONLINE_E_OFFERING_OFFER_NOT_CANCELABLE         _HRESULT_TYPEDEF_(0x8015300AL)  // either this offer doesn't exist, or it's marked as un-cancelable
#define XONLINE_E_OFFERING_NOTHING_TO_CANCEL            _HRESULT_TYPEDEF_(0x8015300BL)  // this user doesn't have one of these anyways
#define XONLINE_E_OFFERING_ALREADY_OWN_MAX              _HRESULT_TYPEDEF_(0x8015300CL)  // this user already owns the maximum allowed
#define XONLINE_E_OFFERING_NO_CHARGE                    _HRESULT_TYPEDEF_(0x8015300DL)  // this is a free offer; no purchase is necessary
#define XONLINE_E_OFFERING_PERMISSION_DENIED            _HRESULT_TYPEDEF_(0x8015300EL)  // permission denied
#define XONLINE_E_OFFERING_NAME_TAKEN                   _HRESULT_TYPEDEF_(0x8015300FL)  // Name given to XOnlineVerifyNickname is taken (dosen't vet)

//  Errors returned by xcbk service                     = 0x801535XX

//  Errors returned by uacs service                     = 0x801540XX

// Errors returned by Notification                      = 0x801550XX
#define XONLINE_E_NOTIFICATION_BAD_CONTENT_TYPE         _HRESULT_TYPEDEF_(0x80155000L)
#define XONLINE_E_NOTIFICATION_REQUEST_TOO_SMALL        _HRESULT_TYPEDEF_(0x80155001L)
#define XONLINE_E_NOTIFICATION_INVALID_MESSAGE_TYPE     _HRESULT_TYPEDEF_(0x80155002L)
#define XONLINE_E_NOTIFICATION_NO_ADDRESS               _HRESULT_TYPEDEF_(0x80155003L)
#define XONLINE_E_NOTIFICATION_INVALID_PUID             _HRESULT_TYPEDEF_(0x80155004L)
#define XONLINE_E_NOTIFICATION_NO_CONNECTION            _HRESULT_TYPEDEF_(0x80155005L)
#define XONLINE_E_NOTIFICATION_SEND_FAILED              _HRESULT_TYPEDEF_(0x80155006L)
#define XONLINE_E_NOTIFICATION_RECV_FAILED              _HRESULT_TYPEDEF_(0x80155007L)
#define XONLINE_E_NOTIFICATION_MESSAGE_TRUNCATED        _HRESULT_TYPEDEF_(0x80155008L)
#define XONLINE_E_NOTIFICATION_INVALID_TITLE_ID         _HRESULT_TYPEDEF_(0x80155009L)

// Errors returned by Messages                          = 0x80155AXX
#define XONLINE_E_MESSAGE_INVALID_MESSAGE_ID            _HRESULT_TYPEDEF_(0x80155A01L)  // the specified message was not found
#define XONLINE_E_MESSAGE_PROPERTY_DOWNLOAD_REQUIRED    _HRESULT_TYPEDEF_(0x80155A02L)  // the property was too large to fit into the details block, it must be retrieved separately using XOnlineMessageDownloadAttachmentxxx
#define XONLINE_E_MESSAGE_PROPERTY_NOT_FOUND            _HRESULT_TYPEDEF_(0x80155A03L)  // the specified property tag was not found
#define XONLINE_E_MESSAGE_NO_VALID_SENDS_TO_REVOKE      _HRESULT_TYPEDEF_(0x80155A04L)  // no valid sends to revoke were found 
#define XONLINE_E_MESSAGE_NO_MESSAGE_DETAILS            _HRESULT_TYPEDEF_(0x80155A05L)  // the specified message does not have any details
#define XONLINE_E_MESSAGE_INVALID_TITLE_ID              _HRESULT_TYPEDEF_(0x80155A06L)  // an invalid title ID was specified
#define XONLINE_E_MESSAGE_SENDER_BLOCKED                _HRESULT_TYPEDEF_(0x80155A07L)  // a send failed because the recipient has blocked the sender
#define XONLINE_E_MESSAGE_MAX_DETAILS_SIZE_EXCEEDED     _HRESULT_TYPEDEF_(0x80155A08L)  // the property couldn't be added because the maximum details size would be exceeded
// Success codes returned by Messages                   = 0x00155AXX
#define XONLINE_S_MESSAGE_PENDING_SYNC                  _HRESULT_TYPEDEF_(0x00155A01L)  // updated message list is currently being retrieved (after logon or disabling summary refresh), returned results may be out of date


//  Errors returned by matchmaking                      = 0x801551XX
#define XONLINE_E_MATCH_INVALID_SESSION_ID              _HRESULT_TYPEDEF_(0x80155100L)  // specified session id does not exist
#define XONLINE_E_MATCH_INVALID_TITLE_ID                _HRESULT_TYPEDEF_(0x80155101L)  // specified title id is zero, or does not exist
#define XONLINE_E_MATCH_INVALID_DATA_TYPE               _HRESULT_TYPEDEF_(0x80155102L)  // attribute ID or parameter type specifies an invalid data type
#define XONLINE_E_MATCH_REQUEST_TOO_SMALL               _HRESULT_TYPEDEF_(0x80155103L)  // the request did not meet the minimum length for a valid request
#define XONLINE_E_MATCH_REQUEST_TRUNCATED               _HRESULT_TYPEDEF_(0x80155104L)  // the self described length is greater than the actual buffer size
#define XONLINE_E_MATCH_INVALID_SEARCH_REQ              _HRESULT_TYPEDEF_(0x80155105L)  // the search request was invalid
#define XONLINE_E_MATCH_INVALID_OFFSET                  _HRESULT_TYPEDEF_(0x80155106L)  // one of the attribute/parameter offsets in the request was invalid.  Will be followed by the zero based offset number.
#define XONLINE_E_MATCH_INVALID_ATTR_TYPE               _HRESULT_TYPEDEF_(0x80155107L)  // the attribute type was something other than user or session
#define XONLINE_E_MATCH_INVALID_VERSION                 _HRESULT_TYPEDEF_(0x80155108L)  // bad protocol version in request
#define XONLINE_E_MATCH_OVERFLOW                        _HRESULT_TYPEDEF_(0x80155109L)  // an attribute or parameter flowed past the end of the request
#define XONLINE_E_MATCH_INVALID_RESULT_COL              _HRESULT_TYPEDEF_(0x8015510AL)  // referenced stored procedure returned a column with an unsupported data type
#define XONLINE_E_MATCH_INVALID_STRING                  _HRESULT_TYPEDEF_(0x8015510BL)  // string with length-prefix of zero, or string with no terminating null
#define XONLINE_E_MATCH_STRING_TOO_LONG                 _HRESULT_TYPEDEF_(0x8015510CL)  // string exceeded 400 characters
#define XONLINE_E_MATCH_BLOB_TOO_LONG                   _HRESULT_TYPEDEF_(0x8015510DL)  // blob exceeded 800 bytes
#define XONLINE_E_MATCH_INVALID_ATTRIBUTE_ID            _HRESULT_TYPEDEF_(0x80155110L)  // attribute id is invalid
#define XONLINE_E_MATCH_SESSION_ALREADY_EXISTS          _HRESULT_TYPEDEF_(0x80155112L)  // session id already exists in the db
#define XONLINE_E_MATCH_CRITICAL_DB_ERR                 _HRESULT_TYPEDEF_(0x80155115L)  // critical error in db
#define XONLINE_E_MATCH_NOT_ENOUGH_COLUMNS              _HRESULT_TYPEDEF_(0x80155116L)  // search result set had too few columns
#define XONLINE_E_MATCH_PERMISSION_DENIED               _HRESULT_TYPEDEF_(0x80155117L)  // incorrect permissions set on search sp
#define XONLINE_E_MATCH_INVALID_PART_SCHEME             _HRESULT_TYPEDEF_(0x80155118L)  // title specified an invalid partitioning scheme
#define XONLINE_E_MATCH_INVALID_PARAM                   _HRESULT_TYPEDEF_(0x80155119L)  // bad parameter passed to sp
#define XONLINE_E_MATCH_DATA_TYPE_MISMATCH              _HRESULT_TYPEDEF_(0x8015511DL)  // data type specified in attr id did not match type of attr being set
#define XONLINE_E_MATCH_SERVER_ERROR                    _HRESULT_TYPEDEF_(0x8015511EL)  // error on server not correctable by client
#define XONLINE_E_MATCH_NO_USERS                        _HRESULT_TYPEDEF_(0x8015511FL)  // no authenticated users in search request.
#define XONLINE_E_MATCH_INVALID_BLOB                    _HRESULT_TYPEDEF_(0x80155120L)  // invalid blob attribute
#define XONLINE_E_MATCH_TOO_MANY_USERS                  _HRESULT_TYPEDEF_(0x80155121L)  // too many users in search request
#define XONLINE_E_MATCH_INVALID_FLAGS                   _HRESULT_TYPEDEF_(0x80155122L)  // invalid flags were specified in a search request

// Errors returned by uodb procs                        = 0x801560XX
#define XONLINE_E_UODB_KEY_ALREADY_EXISTS               _HRESULT_TYPEDEF_(0x80156000L)  // service key already exists when attempting to insert key

// Errors returned by Query service                     = 0x801561XX
#define XONLINE_E_QUERY_QUOTA_FULL                      _HRESULT_TYPEDEF_(0x80156101L)  // this user or team's quota for the dataset is full.  you must remove an entity first.
#define XONLINE_E_QUERY_ENTITY_NOT_FOUND                _HRESULT_TYPEDEF_(0x80156102L)  // the requested entity didn't exist in the provided dataset.
#define XONLINE_E_QUERY_PERMISSION_DENIED               _HRESULT_TYPEDEF_(0x80156103L)  // the user tried to update or delete an entity that he didn't own.
#define XONLINE_E_QUERY_ATTRIBUTE_TOO_LONG              _HRESULT_TYPEDEF_(0x80156104L)  // attribute passed exceeds schema definition
#define XONLINE_E_QUERY_UNEXPECTED_ATTRIBUTE            _HRESULT_TYPEDEF_(0x80156105L)  // attribute passed was a bad param for the database operation
#define XONLINE_E_QUERY_INVALID_ACTION                  _HRESULT_TYPEDEF_(0x80156107L)  // the specified action (or dataset) doesn't have a select action associated with it. 
#define XONLINE_E_QUERY_SPEC_COUNT_MISMATCH             _HRESULT_TYPEDEF_(0x80156108L)  // the provided number of QUERY_ATTRIBUTE_SPECs doesn't match the number returned by the procedure
#define XONLINE_E_QUERY_DATASET_NOT_FOUND               _HRESULT_TYPEDEF_(0x80156109L)  // The specified dataset id was not found.
#define XONLINE_E_QUERY_PROCEDURE_NOT_FOUND             _HRESULT_TYPEDEF_(0x8015610AL)  // The specified proc index was not found.


// Errors returned by Competitions service              = 0x801562XX
#define XONLINE_E_COMP_ACCESS_DENIED                    _HRESULT_TYPEDEF_(0x80156202L)  // The specified source (client) is not permitted to execute this method
#define XONLINE_E_COMP_REGISTRATION_CLOSED              _HRESULT_TYPEDEF_(0x80156203L)  // The competition is closed to registration
#define XONLINE_E_COMP_FULL                             _HRESULT_TYPEDEF_(0x80156204L)  // The competition has reached it's max enrollment
#define XONLINE_E_COMP_NOT_REGISTERED                   _HRESULT_TYPEDEF_(0x80156205L)  // The user or team isn't registered for the competition
#define XONLINE_E_COMP_CANCELLED                        _HRESULT_TYPEDEF_(0x80156206L)  // The competition has been cancelled, and the operation is invalid.
#define XONLINE_E_COMP_CHECKIN_TIME_INVALID             _HRESULT_TYPEDEF_(0x80156207L)  // The user is attempting to checkin to an event outside the allowed time.
#define XONLINE_E_COMP_CHECKIN_BAD_EVENT                _HRESULT_TYPEDEF_(0x80156208L)  // The user is attempting to checkin to an event in which they are not a valid participant.
#define XONLINE_E_COMP_EVENT_SCORED                     _HRESULT_TYPEDEF_(0x80156209L)  // The user is attempting to checkin to an event which has already been scored by the service (user has forfeited or been ejected)
#define XONLINE_S_COMP_EVENT_SCORED                     _HRESULT_TYPEDEF_(0x00156209L)  // The user is attempting to checkin to an event but the users event has been updated. Re-query for a new event
#define XONLINE_E_COMP_UNEXPECTED                       _HRESULT_TYPEDEF_(0x80156210L)  // Results from the Database are unexpected or inconsistent with the current operation.
#define XONLINE_E_COMP_TOPOLOGY_ERROR                   _HRESULT_TYPEDEF_(0x80156216L)  // The topology request cannot be fulfilled by the server
#define XONLINE_E_COMP_TOPOLOGY_PENDING                 _HRESULT_TYPEDEF_(0x80156217L)  // The topology request has not completed yet
#define XONLINE_E_COMP_CHECKIN_TOO_EARLY                _HRESULT_TYPEDEF_(0x80156218L)  // The user is attempting to checkin to an event before the allowed time.
#define XONLINE_E_COMP_ALREADY_REGISTERED               _HRESULT_TYPEDEF_(0x80156219L)  // The user has already registered for this competition.
#define XONLINE_E_COMP_INVALID_ENTRANT_TYPE             _HRESULT_TYPEDEF_(0x8015621AL)  // dwTeamId was non-0 for a user competition, or dwTeamId was 0 for a team competition

// Errors returned by the v1 Message Service            = 0x801570XX
#define XONLINE_E_MSGSVR_INVALID_REQUEST                _HRESULT_TYPEDEF_(0x80157001L)  // an invalid request type was received

// Errors returned by the String Service                = 0x801571XX
#define XONLINE_E_STRING_ALREADY_EXISTS                 _HRESULT_TYPEDEF_(0x80157100L)  // an attempt was made to add a string that already exists
#define XONLINE_E_STRING_NOT_FOUND                      _HRESULT_TYPEDEF_(0x80157101L)  // an attempt was made to update a string that doesn't exist
#define XONLINE_E_STRING_OFFENSIVE_TEXT                 _HRESULT_TYPEDEF_(0x80157102L)  // the string contains offensive text

// Errors returned by the Feedback Service              = 0x801580XX
#define XONLINE_E_FEEDBACK_NULL_TARGET                  _HRESULT_TYPEDEF_(0x80158001L) // target PUID of feedback is NULL
#define XONLINE_E_FEEDBACK_BAD_TYPE                     _HRESULT_TYPEDEF_(0x80158002L) // bad feedback type
#define XONLINE_E_FEEDBACK_CANNOT_LOG                   _HRESULT_TYPEDEF_(0x80158006L) // cannot write to feedback log

// Errors returned by the Statistics Service            = 0x80159XXX
#define XONLINE_E_STAT_BAD_REQUEST                      _HRESULT_TYPEDEF_(0x80159001L)   // server received incorrectly formatted request.
#define XONLINE_E_STAT_INVALID_TITLE_OR_LEADERBOARD     _HRESULT_TYPEDEF_(0x80159002L)   // title or leaderboard id were not recognized by the server.
#define XONLINE_E_STAT_TOO_MANY_SPECS                   _HRESULT_TYPEDEF_(0x80159004L)   // too many stat specs in a request.
#define XONLINE_E_STAT_TOO_MANY_STATS                   _HRESULT_TYPEDEF_(0x80159005L)   // too many stats in a spec or already stored for the user.
#define XONLINE_E_STAT_USER_NOT_FOUND                   _HRESULT_TYPEDEF_(0x80159003L)   // user not found.
#define XONLINE_E_STAT_SET_FAILED_0                     _HRESULT_TYPEDEF_(0x80159100L)   // set operation failed on spec index 0
#define XONLINE_E_STAT_PERMISSION_DENIED                _HRESULT_TYPEDEF_(0x80159200L)   // operation failed because of credentials. UserId is not logged in or this operation is not supported in production (e.g. userId=0 in XOnlineStatReset)
#define XONLINE_E_STAT_LEADERBOARD_WAS_RESET            _HRESULT_TYPEDEF_(0x80159201L)   // operation failed because user was logged on before the leaderboard was reset.
#define XONLINE_E_STAT_INVALID_ATTACHMENT               _HRESULT_TYPEDEF_(0x80159202L)   // attachment is invalid.
#define XONLINE_S_STAT_CAN_UPLOAD_ATTACHMENT            _HRESULT_TYPEDEF_(0x00159203L)   // Use XOnlineStatWriteGetResults to get a handle to upload a attachment.
#define XONLINE_E_STAT_TOO_MANY_PARAMETERS              _HRESULT_TYPEDEF_(0x80159204L) 
#define XONLINE_E_STAT_TOO_MANY_PROCEDURES              _HRESULT_TYPEDEF_(0x80159205L) 
#define XONLINE_E_STAT_STAT_POST_PROC_ERROR             _HRESULT_TYPEDEF_(0x80159206L)

//  Errors returned by xsuppapi service                 = 0x8015A0XX

// Errors returned by Signature Service                 = 0x8015B0XX
#define XONLINE_E_SIGNATURE_VER_INVALID_SIGNATURE       _HRESULT_TYPEDEF_(0x8015B001L)  // presented signature does not match
#define XONLINE_E_SIGNATURE_VER_UNKNOWN_KEY_VER         _HRESULT_TYPEDEF_(0x8015B002L)  // signature key version specified is not found among the valid signature keys
#define XONLINE_E_SIGNATURE_VER_UNKNOWN_SIGNATURE_VER   _HRESULT_TYPEDEF_(0x8015B003L)  // signature version is unknown, currently only version 1 is supported
#define XONLINE_E_SIGNATURE_BANNED_XBOX                 _HRESULT_TYPEDEF_(0x8015B004L)  // signature is not calculated or revoked because Xbox is banned
#define XONLINE_E_SIGNATURE_BANNED_USER                 _HRESULT_TYPEDEF_(0x8015B005L)  // signature is not calculated or revoked because at least one user is banned
#define XONLINE_E_SIGNATURE_BANNED_TITLE                _HRESULT_TYPEDEF_(0x8015B006L)  // signature is not calculated or revoked because the given title and version is banned
#define XONLINE_E_SIGNATURE_BANNED_DIGEST               _HRESULT_TYPEDEF_(0x8015B007L)  // signature is not calculated or revoked because the digest is banned
#define XONLINE_E_SIGNATURE_GET_BAD_AUTH_DATA           _HRESULT_TYPEDEF_(0x8015B008L)  // fail to retrieve AuthData from SG, returned by GetSigningKey api
#define XONLINE_E_SIGNATURE_SERVICE_UNAVAILABLE         _HRESULT_TYPEDEF_(0x8015B009L)  // fail to retrieve a signature server master key, returned by GetSigningKey or SignOnBehalf api

// Errors returned by Arbitration Service                          = 0x8015B1XX
#define XONLINE_E_ARBITRATION_SERVICE_UNAVAILABLE                  _HRESULT_TYPEDEF_(0x8015B101L)   // Service temporarily unavailable
#define XONLINE_E_ARBITRATION_INVALID_REQUEST                      _HRESULT_TYPEDEF_(0x8015B102L)   // The request is invalidly formatted
#define XONLINE_E_ARBITRATION_SESSION_NOT_FOUND                    _HRESULT_TYPEDEF_(0x8015B103L)   // The session is not found or has expired
#define XONLINE_E_ARBITRATION_REGISTRATION_FLAGS_MISMATCH          _HRESULT_TYPEDEF_(0x8015B104L)   // The session was registered with different flags by another Xbox
#define XONLINE_E_ARBITRATION_REGISTRATION_SESSION_TIME_MISMATCH   _HRESULT_TYPEDEF_(0x8015B105L)   // The session was registered with a different session time by another Xbox
#define XONLINE_E_ARBITRATION_REGISTRATION_TOO_LATE                _HRESULT_TYPEDEF_(0x8015B106L)   // Registration came too late, the session has already been arbitrated
#define XONLINE_E_ARBITRATION_NEED_TO_REGISTER_FIRST               _HRESULT_TYPEDEF_(0x8015B107L)   // Must register in seesion first, before any other activity
#define XONLINE_E_ARBITRATION_TIME_EXTENSION_NOT_ALLOWED           _HRESULT_TYPEDEF_(0x8015B108L)   // Time extension of this session not allowed, or session is already arbitrated
#define XONLINE_E_ARBITRATION_INCONSISTENT_FLAGS                   _HRESULT_TYPEDEF_(0x8015B109L)   // Inconsistent flags are used in the request
#define XONLINE_E_ARBITRATION_INCONSISTENT_COMPETITION_STATUS      _HRESULT_TYPEDEF_(0x8015B10AL)   // Whether the session is a competition is inconsistent between registration and report
#define XONLINE_E_ARBITRATION_REPORT_ALREADY_CALLED                _HRESULT_TYPEDEF_(0x8015b10BL)   // Report call for this session already made by this client
#define XONLINE_E_ARBITRATION_TOO_MANY_XBOXES_IN_SESSION           _HRESULT_TYPEDEF_(0x8015b10CL)   // Only up to 255 Xboxes can register in a session
#define XONLINE_E_ARBITRATION_1_XBOX_1_USER_SESSION_NOT_ALLOWED    _HRESULT_TYPEDEF_(0x8015b10DL)   // Single Xbox single user sessions should not be arbitrated
#define XONLINE_E_ARBITRATION_REPORT_TOO_LARGE                     _HRESULT_TYPEDEF_(0x8015b10EL)   // The stats or query submission is too large
#define XONLINE_E_ARBITRATION_INVALID_TEAMTICKET                   _HRESULT_TYPEDEF_(0x8015b10FL)   // An invalid team ticket was submitted
// Arbitration success HRESULTS
#define XONLINE_S_ARBITRATION_INVALID_XBOX_SPECIFIED               _HRESULT_TYPEDEF_(0x0015b1F0L)   // Invalid/duplicate Xbox specified in lost connectivity or suspicious info. Never the less, this report is accepted
#define XONLINE_S_ARBITRATION_INVALID_USER_SPECIFIED               _HRESULT_TYPEDEF_(0x0015b1F1L)   // Invalid/duplicate user specified in lost connectivity or suspicious info. Never the less, this report is accepted
#define XONLINE_S_ARBITRATION_DIFFERENT_RESULTS_DETECTED           _HRESULT_TYPEDEF_(0x0015b1F2L)   // Differing result submissions have been detected in this session. Never the less, this report submission is accepted

// Errors returned by the Storage services              = 0x8015C0XX
#define XONLINE_E_STORAGE_INVALID_REQUEST               _HRESULT_TYPEDEF_(0x8015c001L)  // Request is invalid
#define XONLINE_E_STORAGE_ACCESS_DENIED                 _HRESULT_TYPEDEF_(0x8015c002L)  // Client doesn't have the rights to upload the file
#define XONLINE_E_STORAGE_FILE_IS_TOO_BIG               _HRESULT_TYPEDEF_(0x8015c003L)  // File is too big
#define XONLINE_E_STORAGE_FILE_NOT_FOUND                _HRESULT_TYPEDEF_(0x8015c004L)  // File not found
#define XONLINE_E_STORAGE_INVALID_ACCESS_TOKEN          _HRESULT_TYPEDEF_(0x8015c005L)  // Access token signature is invalid
#define XONLINE_E_STORAGE_CANNOT_FIND_PATH              _HRESULT_TYPEDEF_(0x8015c006L)  // name resolution failed
#define XONLINE_E_STORAGE_FILE_IS_ELSEWHERE             _HRESULT_TYPEDEF_(0x8015c007L)  // redirection request
#define XONLINE_E_STORAGE_INVALID_STORAGE_PATH          _HRESULT_TYPEDEF_(0x8015c008L)  // Invalid storage path
#define XONLINE_E_STORAGE_INVALID_FACILITY              _HRESULT_TYPEDEF_(0x8015c009L)  // Invalid facility code
#define XONLINE_E_STORAGE_UNKNOWN_DOMAIN                _HRESULT_TYPEDEF_(0x8015c00AL)  // Bad pathname
#define XONLINE_E_STORAGE_SYNC_TIME_SKEW                _HRESULT_TYPEDEF_(0x8015c00BL)  // SyncDomain timestamp skew
#define XONLINE_E_STORAGE_SYNC_TIME_SKEW_LOCALTIME      _HRESULT_TYPEDEF_(0x8015c00CL)  // SyncDomain timestamp appears to be localtime
#define XONLINE_E_STORAGE_UNSUPPORTED_CONTENT_TYPE      _HRESULT_TYPEDEF_(0x8015c00DL)  // The type of the content is not supported by this API

// Errors returned by billing services                      = 0x80162XXX - 0x8016EXXX
#define XONLINE_E_BILLING_AUTHORIZATION_FAILED              _HRESULT_TYPEDEF_(0x80167611) // Credit card authorization failed; user should update credit card info in Dash.
#define XONLINE_E_BILLING_CREDIT_CARD_EXPIRED               _HRESULT_TYPEDEF_(0x80167531) // The credit card has expired or will expire this month; user should update card info in Dash.
#define XONLINE_E_BILLING_NON_ACTIVE_ACCOUNT                _HRESULT_TYPEDEF_(0x80169d94) // The account specified is no longer active; user should call customer service
#define XONLINE_E_BILLING_INVALID_PAYMENT_INSTRUMENT_STATUS _HRESULT_TYPEDEF_(0x80169e7f) // User's payment instrument is in a bad state. They should call customer service to rectify the issue.









//
// XOnline Task Pump
//

DECLARE_HANDLE(XONLINETASK_HANDLE);
typedef XONLINETASK_HANDLE* PXONLINETASK_HANDLE;

#define XONLINETASK_S_RUNNING                   (S_OK)
#define XONLINETASK_S_SUCCESS                   _HRESULT_TYPEDEF_(0x001500F0L)
#define XONLINETASK_S_RESULTS_AVAIL             _HRESULT_TYPEDEF_(0x001500F1L)
#define XONLINETASK_S_RUNNING_IDLE              _HRESULT_TYPEDEF_(0x001500F2L)


XBOXAPI
HRESULT
WINAPI
XOnlineTaskContinue(
    IN XONLINETASK_HANDLE hTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTaskClose(
    IN XONLINETASK_HANDLE hTask
    );



//
// XOnline Authentication
//

#define XONLINE_GAMERTAG_SIZE                   16
#define XONLINE_MAX_GAMERTAG_LENGTH             (XONLINE_GAMERTAG_SIZE - 1)
#define XONLINE_PASSCODE_LENGTH                  4
#define XONLINE_MAX_LOGON_USERS                  4

typedef enum {
    XONLINE_PASSCODE_DPAD_UP = 1,
    XONLINE_PASSCODE_DPAD_DOWN,
    XONLINE_PASSCODE_DPAD_LEFT,
    XONLINE_PASSCODE_DPAD_RIGHT,
    XONLINE_PASSCODE_GAMEPAD_X,
    XONLINE_PASSCODE_GAMEPAD_Y,
    XONLINE_PASSCODE_GAMEPAD_LEFT_TRIGGER = 9,
    XONLINE_PASSCODE_GAMEPAD_RIGHT_TRIGGER
} XONLINE_PASSCODE_TYPE;

typedef enum {
    XONLINE_NAT_OPEN = 1,
    XONLINE_NAT_MODERATE,
    XONLINE_NAT_STRICT
} XONLINE_NAT_TYPE;


#define XONLINE_STRING_SERVICE                  ((DWORD)2)
#define XONLINE_CONTENT_AVAILABLE_SERVICE       ((DWORD)4)
#define XONLINE_MATCHMAKING_SERVICE             ((DWORD)6)
#define XONLINE_STATISTICS_SERVICE              ((DWORD)7)
#define XONLINE_FEEDBACK_SERVICE                ((DWORD)8)
#define XONLINE_BILLING_OFFERING_SERVICE        ((DWORD)9)
#define XONLINE_NICKNAME_VERIFICATION_SERVICE   ((DWORD)9)
#define XONLINE_SIGNATURE_SERVICE               ((DWORD)12)
#define XONLINE_QUERY_SERVICE                   ((DWORD)13)
#define XONLINE_STORAGE_SERVICE                 ((DWORD)15)
#define XONLINE_ARBITRATION_SERVICE             ((DWORD)16)
#define XONLINE_USAGE_DATA_SERVICE              ((DWORD)17)
#define XONLINE_MESSAGING_SERVICE               ((DWORD)18)
#define XONLINE_TEAM_SERVICE                    ((DWORD)19)
#define XONLINE_NAT_TYPE_DETECTION_SERVICE      ((DWORD)20)

#define XONLINE_INVALID_SERVICE                 ((DWORD)0)


#define XONLINE_USER_GUEST_MASK                 0x00000003
#define XONLINE_USER_NOSHOW_RATING_MASK         0x0000001C
#define XONLINE_USER_DISCONNECT_RATING_MASK     0x000000E0

#define XONLINE_USER_COUNTRY_MASK               0x0000ff00

#define XONLINE_USER_VOICE_NOT_ALLOWED          0x00010000
#define XONLINE_USER_PURCHASE_NOT_ALLOWED       0x00020000
#define XONLINE_USER_NICKNAME_NOT_ALLOWED       0x00040000
#define XONLINE_USER_SHARED_CONTENT_NOT_ALLOWED 0x00080000

#define XOnlineUserCountryId(dwUserFlags) ((BYTE)(((dwUserFlags) & XONLINE_USER_COUNTRY_MASK) >> 8))

#define XOnlineIsUserVoiceAllowed(dwUserFlags) (((dwUserFlags) & XONLINE_USER_VOICE_NOT_ALLOWED) == 0)

#define XOnlineIsUserPurchaseAllowed(dwUserFlags) (((dwUserFlags) & XONLINE_USER_PURCHASE_NOT_ALLOWED) == 0)

#define XOnlineIsUserNicknameAllowed(dwUserFlags) (((dwUserFlags) & XONLINE_USER_NICKNAME_NOT_ALLOWED) == 0)

#define XOnlineIsUserSharedContentAllowed(dwUserFlags) (((dwUserFlags) & XONLINE_USER_SHARED_CONTENT_NOT_ALLOWED) == 0)

#define XOnlineUserNoShowRating(dwUserFlags) (((dwUserFlags) & XONLINE_USER_NOSHOW_RATING_MASK) >> 2)

#define XOnlineUserDisconnectRating(dwUserFlags) (((dwUserFlags) & XONLINE_USER_DISCONNECT_RATING_MASK) >> 5)

#define XOnlineIsUserGuest(dwUserFlags) (((dwUserFlags) & XONLINE_USER_GUEST_MASK) != 0)

#define XOnlineUserGuestNumber(dwUserFlags) ((dwUserFlags) & XONLINE_USER_GUEST_MASK)

#define XOnlineSetUserGuestNumber(dwUserFlags,guestNumber) ((dwUserFlags) = ((dwUserFlags) & ~XONLINE_USER_GUEST_MASK) | (guestNumber & XONLINE_USER_GUEST_MASK))


#pragma pack(push, 4)

typedef struct _XUID {
    union
    {
        ULONGLONG qwUserID;
        ULONGLONG qwTeamID;
    };
    DWORD dwUserFlags;
} XUID;

#pragma pack(pop)

#define XOnlineAreUsersIdentical(pXUID1, pXUID2) (((pXUID1)->qwUserID == (pXUID2)->qwUserID) && \
                (XOnlineUserGuestNumber((pXUID1)->dwUserFlags) == XOnlineUserGuestNumber((pXUID2)->dwUserFlags)))

#define XOnlineXUIDIsTeam(pxuid) (((pxuid)->qwUserID & 0xFF00000000000000) == 0xFE00000000000000)


typedef ULONGLONG XOFFERING_ID;


#pragma pack(push, 4)

#define XONLINE_USER_RESERVED_SIZE              72
#define XONLINE_MAX_STORED_ONLINE_USERS         16


#define XONLINE_USER_OPTION_REQUIRE_PASSCODE    0x00000001
#define XONLINE_USER_OPTION_CAME_FROM_MU        0x80000000
#define XONLINE_USER_OPTION_MU_PORT_MASK        0x60000000
#define XONLINE_USER_OPTION_MU_PORT_SHIFT               29
#define XONLINE_USER_OPTION_MU_SLOT_MASK        0x10000000
#define XONLINE_USER_OPTION_MU_SLOT_SHIFT               28

typedef struct _XONLINE_USER {
    XUID xuid;
    CHAR szGamertag[XONLINE_GAMERTAG_SIZE];
    DWORD dwUserOptions;
    BYTE passcode[XONLINE_PASSCODE_LENGTH];
    BYTE reserved[XONLINE_USER_RESERVED_SIZE];
    HRESULT hr;
} XONLINE_USER, *PXONLINE_USER;

typedef struct _XONLINE_SERVICE_INFO{
    DWORD          dwServiceID;
    IN_ADDR        serviceIP;
    WORD           wServicePort;
    WORD           wReserved;
} XONLINE_SERVICE_INFO, *PXONLINE_SERVICE_INFO;

#pragma pack(pop)


XBOXAPI
HRESULT
WINAPI
XOnlineGetUsers(
    OUT PXONLINE_USER pUsers,
    OUT DWORD *pdwUsers
    );

XBOXAPI
HRESULT
WINAPI
XOnlineLogon(
    IN const XONLINE_USER *pUsers,
    IN const DWORD *pdwServiceIDs,
    IN DWORD dwServices,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE pHandle
    );

XBOXAPI
HRESULT
WINAPI
XOnlineLogonTaskGetResults(
    IN XONLINETASK_HANDLE hLogonTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineChangeLogonUsers(
    IN const XONLINE_USER *pUsers,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE pHandle
    );

XBOXAPI
HRESULT
WINAPI
XOnlineChangeLogonUsersTaskGetResults(
    IN XONLINETASK_HANDLE hLogonTask,
    OUT HRESULT *phr
    );

XBOXAPI
PXONLINE_USER
WINAPI
XOnlineGetLogonUsers();

XBOXAPI
XONLINE_NAT_TYPE
WINAPI
XOnlineGetNatType();

XBOXAPI
HRESULT
WINAPI
XOnlineGetServiceInfo(
    IN DWORD dwServiceID,
    OUT PXONLINE_SERVICE_INFO pServiceInfo
    );

XBOXAPI
HRESULT
WINAPI
XOnlineSilentLogon(
    IN const DWORD *pdwServiceIDs,
    IN DWORD dwServices,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE pHandle
    );

//
// Preserving state across reboots
//

// This should be same as XONLINE_MAX_NUMBER_SERVICE
#define XONLINE_MAX_LOGON_STATE_SERVICES    16
#define XONLINE_LOGON_STATE_SIZE    (XONLINE_MAX_LOGON_USERS * sizeof(XONLINE_USER) + XONLINE_MAX_LOGON_STATE_SERVICES * sizeof(DWORD))

#define XONLINE_LOGON_STATE_TYPE 0x4C
#define XONLINE_LOGON_STATE_VERSION 1

typedef struct _XONLINE_LOGON_STATE {
    BYTE bType;
    BYTE bVersion;
    WORD cbSize;
    BYTE Data[XONLINE_LOGON_STATE_SIZE];
} XONLINE_LOGON_STATE, *PXONLINE_LOGON_STATE;

//
// Launch data passed to Downloader.XBE
//

#define LAUNCH_DATA_DOWNLOADER_ID  'dl01'

typedef struct _LD_DOWNLOADER {
    DWORD dwID;
    DWORD dwBitFilter;
    BYTE  bPremiumLogonPort;
    BYTE  Reserved1[3];
    DWORD Reserved2;
    XONLINE_LOGON_STATE LogonState;
    BYTE  UserDefined[MAX_LAUNCH_DATA_SIZE - (20 + XONLINE_LOGON_STATE_SIZE)];
} LD_DOWNLOADER, *PLD_DOWNLOADER;


XBOXAPI
HRESULT
WINAPI
XOnlineSaveLogonState(
    OUT PXONLINE_LOGON_STATE pLogonState
    );

XBOXAPI
HRESULT
WINAPI
XOnlineRetrieveLogonState(
    IN const XONLINE_LOGON_STATE *pLogonState,
    OUT PXONLINE_USER pUsers,
    OUT DWORD *pdwServiceIDs,
    IN OUT DWORD *pdwServices
    );


//
// XOnline Title Update (Security updates)
//

XBOXAPI
HRESULT
WINAPI
XOnlineTitleUpdate(
    IN DWORD dwContext
    );

//
// Signature Service APIs
//

typedef struct
{
    DWORD   cbDigest;
    PBYTE   pbDigest;
    DWORD   cbOnlineSignature;
    PBYTE   pbOnlineSignature;

} XONLINE_SIGNATURE_TO_VERIFY;

typedef XONLINE_SIGNATURE_TO_VERIFY *PXONLINE_SIGNATURE_TO_VERIFY;

XBOXAPI
HRESULT
WINAPI
XOnlineSignatureVerify(
    IN const XONLINE_SIGNATURE_TO_VERIFY *rgSignaturesToVerify,
    IN DWORD dwSignaturesToVerify,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineSignatureVerifyGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT HRESULT **prgHresults,
    OUT DWORD *pdwHresults
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageUpload(
    IN HANDLE hServerFileReference,
    IN LPCSTR szDirectory,
    IN DWORD dwUploadFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

typedef enum
{
    XONLINESTORAGE_FACILITY_INVALID = 0,
    XONLINESTORAGE_FACILITY_STATS,
    XONLINESTORAGE_FACILITY_MESSAGING,
    XONLINESTORAGE_FACILITY_TEAMS,
    XONLINESTORAGE_FACILITY_PER_TITLE,
    XONLINESTORAGE_FACILITY_PER_USER_TITLE,
    XONLINESTORAGE_FACILITY_MAX

} XONLINESTORAGE_FACILITY;

#define XONLINESTORAGE_MAX_PATH                 256

XBOXAPI
HRESULT
WINAPI
XOnlineStorageCreateServerPath(
    IN DWORD dwFacility,
    IN ULONGLONG qwUserID,
    IN ULONGLONG qwTeamID,
    IN LPCWSTR wszStorageFileName,
    OUT LPWSTR wszStorageServerPath,
    IN OUT DWORD *pcchStorageServerPath
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageUploadByServerPath(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStorageFileName,
    IN FILETIME ftServerExpirationDate,
    IN LPCSTR szDirectory,
    IN DWORD dwUploadFlags,
    IN HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageDownload(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStoragePath,
    IN LPCSTR szInstallDirectory,
    IN DWORD dwDownloadFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageUploadFromMemory(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStorageFileName,
    IN FILETIME ftServerExpirationDate,
    IN PBYTE pbDataToUpload,
    IN DWORD cbDataToUpload,
    IN DWORD dwUploadFlags,
    IN HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageDownloadToMemory(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStoragePath,
    IN PBYTE pbReceiveBuffer,
    IN DWORD cbReceiveBuffer,
    IN DWORD dwDownloadFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageDownloadToMemoryGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT OPTIONAL PBYTE *ppbReceivedData,
    OUT OPTIONAL DWORD *pcbReceivedData,
    OUT OPTIONAL DWORD *pcbDataTotal,
    OUT OPTIONAL ULONGLONG *pqwOwnerPuid,
    OUT OPTIONAL FILETIME *pftCreationDate
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageDeleteFile(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStorageFileName,
    IN HANDLE hWorkEvent,
    OUT XONLINETASK_HANDLE *phTask
    );

XBOXAPI
HRESULT 
WINAPI
XOnlineStorageEnumerate(
    IN DWORD dwFacility,
    IN DWORD dwUserIndex,
    IN LPCWSTR wszStorageEnumerationPath,
    IN DWORD dwStartingIndex,
    IN DWORD cMaxResultsToReturn,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

typedef enum
{
    XONLINESTORAGE_CONTENT_TYPE_PACKAGE = 0,
    XONLINESTORAGE_CONTENT_TYPE_BLOB = 1,
    
} XONLINESTORAGE_CONTENT_TYPE;

#pragma pack(push, 1)

typedef struct 
{
    DWORD dwTitleID;
    DWORD dwTitleVersion;
    ULONGLONG qwOwnerPUID;
    BYTE bCountryID;
    ULONGLONG qwReserved;
    DWORD dwContentType;
    DWORD dwStorageSize;
    DWORD dwInstalledSize;
    FILETIME ftCreated;
    FILETIME ftLastModified;
    WORD wAttributesSize;
    WORD cchPathName;
    LPCWSTR wszPathName;
    PBYTE pbAttributes;
    
} XONLINESTORAGE_FILE_INFO, *PXONLINESTORAGE_FILE_INFO;

#pragma pack(pop)

XBOXAPI
HRESULT 
WINAPI
XOnlineStorageEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTotalResults,
    OUT DWORD *pdwResultsReturned,
    OUT PXONLINESTORAGE_FILE_INFO **prgpStorageFileInfo
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageGetProgress(
    IN XONLINETASK_HANDLE hTask,
    OUT OPTIONAL DWORD *pdwPercentDone,
    OUT OPTIONAL ULONGLONG *pqwNumerator,
    OUT OPTIONAL ULONGLONG *pqwDenominator
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageGetInstallLocation(
    IN DWORD dwFacility,
    IN LPCWSTR wszStoragePath,
    OUT LPSTR szLocation,
    IN OUT DWORD *pdwLocationSize
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStorageSetFamilyTitleID(
    IN DWORD dwTitleID
    );


typedef struct
{
    DWORD   dwContext;
    DWORD   dwReserved[6];
    BYTE    Data[MAX_LAUNCH_DATA_SIZE - 28];

} LD_UPDATE, *PLD_UPDATE;

XBOXAPI
HRESULT
WINAPI
XOnlineTitleUpdateEx(
    IN const LD_UPDATE *pldUpdate
    );

//
// XOnline Offerings
//

#define XO_CURRENCY_EUR     1
#define XO_CURRENCY_GBP     2
#define XO_CURRENCY_JPY     4
#define XO_CURRENCY_KRW     8


typedef enum {
    NO_TAX = 0,
    DEFAULT,
    GST,
    VAT,
    TAX_NOT_APPLICABLE
} XONLINE_TAX_TYPE;



typedef struct _XONLINE_PRICE{
    DWORD               dwWholePart;
    DWORD               dwFractionalPart;
    WCHAR               rgwchISOCurrencyCode[3];
    BOOL                fOfferingIsFree;
    XONLINE_TAX_TYPE    Tax;
    BYTE                bCurrencyFormat;
} XONLINE_PRICE, *PXONLINE_PRICE;



typedef enum {
    ONE_TIME_CHARGE = 0,
    MONTHLY,
    QUARTERLY,
    BIANNUALLY,
    ANNUALLY
} XONLINE_OFFERING_FREQUENCY;


typedef struct _XONLINEOFFERING_DETAILS{
    PBYTE                       pbDetailsBuffer;            // Pointer to buffer of details blob
    DWORD                       dwDetailsBuffer;            // Length of details blob
    DWORD                       dwInstances;                // Count of currently-owned instances
    XONLINE_PRICE               Price;                      // price strcture
    DWORD                       dwFreeMonthsBeforeCharge;   // free months before charge begins
    DWORD                       dwDuration;                 // duration of the recurring charge (months)
    XONLINE_OFFERING_FREQUENCY  Frequency;                  // how often charges are made
} XONLINEOFFERING_DETAILS, *PXONLINEOFFERING_DETAILS;


XBOXAPI
HRESULT
WINAPI
XOnlineOfferingPurchase(
    IN DWORD dwUserIndex,
    IN XOFFERING_ID OfferingId,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineOfferingCancel(
    IN DWORD dwUserIndex,
    IN XOFFERING_ID OfferingId,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineOfferingDetails(
    IN DWORD dwUserIndex,
    IN XOFFERING_ID OfferingId,
    IN DWORD dwLanguage,
    IN DWORD dwDescriptionIndex,
    OUT OPTIONAL PBYTE pbBuffer,
    IN DWORD dwBufferSize,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineOfferingDetailsGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINEOFFERING_DETAILS pDetails
    );

XBOXAPI
DWORD
WINAPI
XOnlineOfferingDetailsMaxSize(
    IN DWORD dwTitleSpecificDataMaxSize
    );

XBOXAPI
HRESULT
WINAPI
XOnlineOfferingPriceFormat(
    IN OUT XONLINE_PRICE *pPrice,
    IN OUT LPWSTR lpFormattedPrice,
    IN OUT DWORD *pdwLength,
    IN DWORD dwExtendedCharsFilter
    );




//
// Offering enumeration APIs
//




#define XONLINE_COUNTRY_UNITED_ARAB_EMIRATES 1
#define XONLINE_COUNTRY_ALBANIA              2
#define XONLINE_COUNTRY_ARMENIA              3
#define XONLINE_COUNTRY_ARGENTINA            4
#define XONLINE_COUNTRY_AUSTRIA              5
#define XONLINE_COUNTRY_AUSTRALIA            6
#define XONLINE_COUNTRY_AZERBAIJAN           7
#define XONLINE_COUNTRY_BELGIUM              8
#define XONLINE_COUNTRY_BULGARIA             9
#define XONLINE_COUNTRY_BAHRAIN              10
#define XONLINE_COUNTRY_BRUNEI_DARUSSALAM    11
#define XONLINE_COUNTRY_BOLIVIA              12
#define XONLINE_COUNTRY_BRAZIL               13
#define XONLINE_COUNTRY_BELARUS              14
#define XONLINE_COUNTRY_BELIZE               15
#define XONLINE_COUNTRY_CANADA               16
#define XONLINE_COUNTRY_SWITZERLAND          18
#define XONLINE_COUNTRY_CHILE                19
#define XONLINE_COUNTRY_CHINA                20
#define XONLINE_COUNTRY_COLOMBIA             21
#define XONLINE_COUNTRY_COSTA_RICA           22
#define XONLINE_COUNTRY_CZECH_REPUBLIC       23
#define XONLINE_COUNTRY_GERMANY              24
#define XONLINE_COUNTRY_DENMARK              25
#define XONLINE_COUNTRY_DOMINICAN_REPUBLIC   26
#define XONLINE_COUNTRY_ALGERIA              27
#define XONLINE_COUNTRY_ECUADOR              28
#define XONLINE_COUNTRY_ESTONIA              29
#define XONLINE_COUNTRY_EGYPT                30
#define XONLINE_COUNTRY_SPAIN                31
#define XONLINE_COUNTRY_FINLAND              32
#define XONLINE_COUNTRY_FAROE_ISLANDS        33
#define XONLINE_COUNTRY_FRANCE               34
#define XONLINE_COUNTRY_GREAT_BRITAIN        35
#define XONLINE_COUNTRY_GEORGIA              36
#define XONLINE_COUNTRY_GREECE               37
#define XONLINE_COUNTRY_GUATEMALA            38
#define XONLINE_COUNTRY_HONG_KONG            39
#define XONLINE_COUNTRY_HONDURAS             40
#define XONLINE_COUNTRY_CROATIA              41
#define XONLINE_COUNTRY_HUNGARY              42
#define XONLINE_COUNTRY_INDONESIA            43
#define XONLINE_COUNTRY_IRELAND              44
#define XONLINE_COUNTRY_ISRAEL               45
#define XONLINE_COUNTRY_INDIA                46
#define XONLINE_COUNTRY_IRAQ                 47
#define XONLINE_COUNTRY_IRAN                 48
#define XONLINE_COUNTRY_ICELAND              49
#define XONLINE_COUNTRY_ITALY                50
#define XONLINE_COUNTRY_JAMAICA              51
#define XONLINE_COUNTRY_JORDAN               52
#define XONLINE_COUNTRY_JAPAN                53
#define XONLINE_COUNTRY_KENYA                54
#define XONLINE_COUNTRY_KYRGYZSTAN           55
#define XONLINE_COUNTRY_KOREA                56
#define XONLINE_COUNTRY_KUWAIT               57
#define XONLINE_COUNTRY_KAZAKHSTAN           58
#define XONLINE_COUNTRY_LEBANON              59
#define XONLINE_COUNTRY_LIECHTENSTEIN        60
#define XONLINE_COUNTRY_LITHUANIA            61
#define XONLINE_COUNTRY_LUXEMBOURG           62
#define XONLINE_COUNTRY_LATVIA               63
#define XONLINE_COUNTRY_LIBYA                64
#define XONLINE_COUNTRY_MOROCCO              65
#define XONLINE_COUNTRY_MONACO               66
#define XONLINE_COUNTRY_MACEDONIA            67
#define XONLINE_COUNTRY_MONGOLIA             68
#define XONLINE_COUNTRY_MACAU                69
#define XONLINE_COUNTRY_MALDIVES             70
#define XONLINE_COUNTRY_MEXICO               71
#define XONLINE_COUNTRY_MALAYSIA             72
#define XONLINE_COUNTRY_NICARAGUA            73
#define XONLINE_COUNTRY_NETHERLANDS          74
#define XONLINE_COUNTRY_NORWAY               75
#define XONLINE_COUNTRY_NEW_ZEALAND          76
#define XONLINE_COUNTRY_OMAN                 77
#define XONLINE_COUNTRY_PANAMA               78
#define XONLINE_COUNTRY_PERU                 79
#define XONLINE_COUNTRY_PHILIPPINES          80
#define XONLINE_COUNTRY_PAKISTAN             81
#define XONLINE_COUNTRY_POLAND               82
#define XONLINE_COUNTRY_PUERTO_RICO          83
#define XONLINE_COUNTRY_PORTUGAL             84
#define XONLINE_COUNTRY_PARAGUAY             85
#define XONLINE_COUNTRY_QATAR                86
#define XONLINE_COUNTRY_ROMANIA              87
#define XONLINE_COUNTRY_RUSSIAN_FEDERATION   88
#define XONLINE_COUNTRY_SAUDI_ARABIA         89
#define XONLINE_COUNTRY_SWEDEN               90
#define XONLINE_COUNTRY_SINGAPORE            91
#define XONLINE_COUNTRY_SLOVENIA             92
#define XONLINE_COUNTRY_SLOVAK_REPUBLIC      93
#define XONLINE_COUNTRY_EL_SALVADOR          95
#define XONLINE_COUNTRY_SYRIA                96
#define XONLINE_COUNTRY_THAILAND             97
#define XONLINE_COUNTRY_TUNISIA              98
#define XONLINE_COUNTRY_TURKEY               99
#define XONLINE_COUNTRY_TRINIDAD_AND_TOBAGO  100
#define XONLINE_COUNTRY_TAIWAN               101
#define XONLINE_COUNTRY_UKRAINE              102
#define XONLINE_COUNTRY_UNITED_STATES        103
#define XONLINE_COUNTRY_URUGUAY              104
#define XONLINE_COUNTRY_UZBEKISTAN           105
#define XONLINE_COUNTRY_VENEZUELA            106
#define XONLINE_COUNTRY_VIET_NAM             107
#define XONLINE_COUNTRY_YEMEN                108
#define XONLINE_COUNTRY_SOUTH_AFRICA         109
#define XONLINE_COUNTRY_ZIMBABWE             110





#define    XONLINE_OFFERING_SUBSCRIPTION        0x1
#define    XONLINE_OFFERING_CONTENT             0x2




#define XONLINE_OFFERING_BITFILTER_ALL          0xffffffff
#define XONLINE_OFFERING_TYPE_ALL               0xffffffff



#define    XONLINE_OFFERING_IS_NOT_FREE         0x1


#define XOnlineOfferingIsFree(x)    (((x) & XONLINE_OFFERING_IS_NOT_FREE) == 0)


#pragma pack(push, 1)
typedef struct _XONLINEOFFERING_ENUM_PARAMS
{
    DWORD       dwOfferingType;    // Filter on offering type
    DWORD       dwBitFilter;       // Bitfield for filtering offerings
    DWORD       dwDescriptionIndex;// Publisher-specific index
    WORD        wStartingIndex;    // Starting index to enumerate
    WORD        wMaxResults;       // Desired max number of results
} XONLINEOFFERING_ENUM_PARAMS, *PXONLINEOFFERING_ENUM_PARAMS;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct _XONLINEOFFERING_INFO{
    XOFFERING_ID        OfferingId;             // Offering ID
    DWORD               dwOfferingType;         // Offering type
    DWORD               dwBitFlags;             // Package-specific flags
    DWORD               dwPackageSize;          // Package wire size (bytes)
    DWORD               dwInstallSize;          // Installed size (blocks)
    FILETIME            ftActivationDate;       // Activation date of package
    DWORD               dwRating;               // Package rating
    WORD                fOfferingFlags;         // Per-offering flags
    DWORD               dwTitleSpecificData;    // Size of data blob (bytes)
    PBYTE               pbTitleSpecificData;    // Pointer to data blob
} XONLINEOFFERING_INFO, *PXONLINEOFFERING_INFO;
#pragma pack(pop)

#define XONLINEOFFERING_ENUM_MAX_TITLE_DATA_SIZE    (8000)



XBOXAPI
HRESULT
WINAPI
XOnlineOfferingIsNewContentAvailable(
    IN DWORD dwBitFilter,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );



XBOXAPI
HRESULT
WINAPI
XOnlineOfferingEnumerate(
    IN DWORD dwUserIndex,
    IN const XONLINEOFFERING_ENUM_PARAMS *pEnumParams,
    OUT OPTIONAL PBYTE pbBuffer,
    IN DWORD dwBufferSize,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineOfferingEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINEOFFERING_INFO **prgpOfferingInfo,
    OUT DWORD *pdwReturnedResults,
    OUT BOOL *pfMoreResults
    );


XBOXAPI
DWORD
WINAPI
XOnlineOfferingEnumerateMaxSize(
    IN const XONLINEOFFERING_ENUM_PARAMS *pEnumParams,
    IN OPTIONAL DWORD dwTitleSpecificDataMaxSize
    );


XBOXAPI
HRESULT
WINAPI
XOnlineContentInstall(
    IN XOFFERING_ID OfferingId,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineContentInstallGetProgress(
    IN XONLINETASK_HANDLE hTask,
    OUT OPTIONAL DWORD *pdwPercentDone,
    OUT OPTIONAL ULONGLONG *pqwNumerator,
    OUT OPTIONAL ULONGLONG *pqwDenominator
    );

XBOXAPI
HRESULT
WINAPI
XOnlineContentInstallGetSize(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTotalInstalledSizeInBlocks,
    OUT DWORD *pdwAdditionalBlocksRequired
    );    

XBOXAPI
HRESULT
WINAPI
XOnlineContentSetSecurityKey(
    IN const BYTE *pbSecretKey
    );



//
// Messaging
//

#define XONLINE_MAX_NUM_MESSAGES        125
#define XONLINE_MAX_MESSAGE_RECIPIENTS  100
#define XONLINE_MAX_MESSAGE_DETAILS     4096

DECLARE_HANDLE(XONLINE_MSG_HANDLE);
typedef XONLINE_MSG_HANDLE* PXONLINE_MSG_HANDLE;

//
// Message Flags
//
#define XONLINE_MSG_FLAG_REQUIRED               0x00000001 // The user is required to read this message
#define XONLINE_MSG_FLAG_RECOMMENDED            0x00000002 // The user has a system recommended message
#define XONLINE_MSG_FLAG_HAS_VOICE              0x00000004 // This message contains a voice attachment
#define XONLINE_MSG_FLAG_HAS_TEXT               0x00000008 // This message contains a text body
#define XONLINE_MSG_FLAG_READ                   0x00000010 // This message has been read
#define XONLINE_MSG_FLAG_NON_EXPORTABLE         0x00000020 // This message should only be displayed on Xbox consoles, not the web
#define XONLINE_MSG_FLAG_TEAM_CONTEXT           0x00000040 // This message's sender context refers to a team ID
#define XONLINE_MSG_FLAG_COMP_CONTEXT           0x00000080 // This message's sender context refers to a competition event or entity ID
#define XONLINE_MSG_FLAG_ALTERNATE_TITLE        0x00000100 // This message is from an alternate Title
#define XONLINE_MSG_FLAGS_TITLE_RESERVED        0xFF000000 // Flags reserved for title custom messages

//
// Message Property Types
//
#define XONLINE_MSG_PROP_TYPE_NULL         ((BYTE)  1) // The property contains no data
#define XONLINE_MSG_PROP_TYPE_I1           ((BYTE)  2) // The property value points to 8-bits of data
#define XONLINE_MSG_PROP_TYPE_I2           ((BYTE)  3) // The property value points to 16-bits of data
#define XONLINE_MSG_PROP_TYPE_I4           ((BYTE)  4) // The property value points to 32-bits of data
#define XONLINE_MSG_PROP_TYPE_I8           ((BYTE)  5) // The property value points to 64-bits of data
#define XONLINE_MSG_PROP_TYPE_STRING       ((BYTE)  6) // The property value points to a NULL-terminated wide character string
#define XONLINE_MSG_PROP_TYPE_FILETIME     ((BYTE)  7) // The property value points to a time value
#define XONLINE_MSG_PROP_TYPE_BINARY       ((BYTE)  8) // The property value points to a binary blob that fits in the message details
#define XONLINE_MSG_PROP_TYPE_ATTACHMENT   ((BYTE)  9) // The property value points to a binary blob (or directory path, depending on attachment flags) that will be uploaded to storage during XOnlineMessageSend
#define XONLINE_MSG_PROP_TYPE_BOOL         ((BYTE) 10) // The property value points to a boolean value (1=TRUE, 0=FALSE)
#define XONLINE_MSG_PROP_TYPE_STRING_ID    ((BYTE) 11) // The property value contains a string ID whose text can be requested from the string service

//
// Attachment Flags
//
#define XONLINE_MSG_ATTACHMENT_FLAG_NON_EXPORTABLE  0x00000001 // This attachment should not be visible when the message is displayed on the web
#define XONLINE_MSG_ATTACHMENT_FLAG_DIRECTORY       0x00000002 // The property value points to an ANSI path string to a local directory that will be uploaded to storage, instead of a binary blob


//
// Message Property Tags
// Property tags occupy a word where the upper byte indicates the property's data type, and 
// the lower byte indicates a unique identifier for the property.  Property identifiers only
// need to be unique within a given message type.
//
#define XONLINE_MSG_PROP_TAG(type, id)      ((BYTE)(type) << 8 | (BYTE)(id))
#define XOnlineMessageGetPropId(tag)           ((WORD)(tag) & 0xFF)
#define XOnlineMessageGetPropType(tag)         ((WORD)(tag) >> 8)

//
// Message Property IDs 
// Property IDs should adhere to the following ranges:
//
// Range           Purpose
// 0x00-0x7F       Reserved for use by titles
// 0x80-0xBF       Reserved for use by Microsoft for non-global properties
// 0xC0-0xFF       Reserved for use by Microsoft for properties that span all message types
#define XONLINE_MSG_PROP_ID_BUILTIN        0x80 // Used for properties defined by Microsoft
#define XONLINE_MSG_PROP_ID_GLOBAL         0x40 // Used for properties that can span all message types


//
// Message Types
//

#define XONLINE_MSG_TYPE_TITLE_CUSTOM           ((BYTE) 1) // context: title defined;    required props: title defined
#define XONLINE_MSG_TYPE_FRIEND_REQUEST         ((BYTE) 2) // context: 0;                required props: none
#define XONLINE_MSG_TYPE_GAME_INVITE            ((BYTE) 3) // context: see msg flags;    required props: XONLINE_MSG_PROP_SESSION_ID
#define XONLINE_MSG_TYPE_TEAM_RECRUIT           ((BYTE) 4) // context: inviting team ID; required props: none
#define XONLINE_MSG_TYPE_COMP_REMINDER          ((BYTE) 5) // context: comp event ID;    required props: XONLINE_MSG_PROP_COMP_NAME, XONLINE_MSG_PROP_COMP_EVENT_START
#define XONLINE_MSG_TYPE_COMP_REQUEST           ((BYTE) 6) // context: comp entity ID;   required props: XONLINE_MSG_PROP_COMP_NAME, XONLINE_MSG_PROP_COMP_START, XONLINE_MSG_PROP_COMP_REG_CLOSE
#define XONLINE_MSG_TYPE_LIVE_MESSAGE           ((BYTE) 7) // context: 0                 required props: XONLINE_MSG_PROP_SYSTEM_TEXT


//
// Global property tags, allowed in any message type
//
#define XONLINE_MSG_PROP_VOICE_DATA               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_ATTACHMENT, XONLINE_MSG_PROP_ID_GLOBAL | XONLINE_MSG_PROP_ID_BUILTIN | 1)
#define XONLINE_MSG_PROP_VOICE_DATA_CODEC         XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_GLOBAL | XONLINE_MSG_PROP_ID_BUILTIN | 2)
#define XONLINE_MSG_PROP_VOICE_DATA_DURATION      XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I4,         XONLINE_MSG_PROP_ID_GLOBAL | XONLINE_MSG_PROP_ID_BUILTIN | 3)
#define XONLINE_MSG_PROP_TEXT                     XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING,     XONLINE_MSG_PROP_ID_GLOBAL | XONLINE_MSG_PROP_ID_BUILTIN | 4)
#define XONLINE_MSG_PROP_TEXT_LANGUAGE            XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I4,         XONLINE_MSG_PROP_ID_GLOBAL | XONLINE_MSG_PROP_ID_BUILTIN | 5)

//
// Invite message properties
//
#define XONLINE_MSG_PROP_SESSION_ID               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I8,         XONLINE_MSG_PROP_ID_BUILTIN | 1)

//
// Live system message properties
//
#define XONLINE_MSG_PROP_SYSTEM_TEXT              XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING_ID,  XONLINE_MSG_PROP_ID_BUILTIN | 1)

//
// Competitions message properties
//
#define XONLINE_MSG_PROP_COMP_DATASET             XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I4,         XONLINE_MSG_PROP_ID_BUILTIN | 0x1 )
#define XONLINE_MSG_PROP_COMP_NAME                XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING,     XONLINE_MSG_PROP_ID_BUILTIN | 0x2 )
#define XONLINE_MSG_PROP_COMP_START               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_FILETIME,   XONLINE_MSG_PROP_ID_BUILTIN | 0x3 )
#define XONLINE_MSG_PROP_COMP_ROUND               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0x4 )
#define XONLINE_MSG_PROP_COMP_OPPONENT            XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING_ID,  XONLINE_MSG_PROP_ID_BUILTIN | 0x5 )
#define XONLINE_MSG_PROP_COMP_ADMIN               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I8,         XONLINE_MSG_PROP_ID_BUILTIN | 0x6 )
#define XONLINE_MSG_PROP_COMP_REG_CLOSE           XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_FILETIME,   XONLINE_MSG_PROP_ID_BUILTIN | 0x7 )
#define XONLINE_MSG_PROP_COMP_PRIVATE_SLOTS       XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0x8 )
#define XONLINE_MSG_PROP_COMP_PUBLIC_SLOTS        XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0x9 )
#define XONLINE_MSG_PROP_COMP_UNITS               XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0xA )
#define XONLINE_MSG_PROP_COMP_INTERVAL            XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0xB )
#define XONLINE_MSG_PROP_COMP_DAYMASK             XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I2,         XONLINE_MSG_PROP_ID_BUILTIN | 0xC )
#define XONLINE_MSG_PROP_COMP_DESCRIPTION         XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING,     XONLINE_MSG_PROP_ID_BUILTIN | 0xD )
#define XONLINE_MSG_PROP_COMP_URL                 XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_STRING,     XONLINE_MSG_PROP_ID_BUILTIN | 0xE )
#define XONLINE_MSG_PROP_COMP_EVENT_ID            XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_I8,         XONLINE_MSG_PROP_ID_BUILTIN | 0x10 )
#define XONLINE_MSG_PROP_COMP_EVENT_START         XONLINE_MSG_PROP_TAG(XONLINE_MSG_PROP_TYPE_FILETIME,   XONLINE_MSG_PROP_ID_BUILTIN | 0x11 )

//
// Voice Codec types (values for XONLINE_MSG_PROP_VOICE_DATA_CODEC)
//
#define XONLINE_PROP_VOICE_DATA_CODEC_WMAVOICE_V90      1



#pragma pack(push, 8)
typedef struct _XONLINE_MSG_SUMMARY
{
    XUID                xuidSender;                          // User ID of sender
    BYTE                bMsgType;                            // Type of the message
    ULONGLONG           qwMessageContext;                    // Message specific context value
    FILETIME            ftSentTime;                          // Time at which message was sent, in Coordinated Universal Time (UTC) format
    DWORD               dwMessageID;                         // ID of message
    DWORD               dwMessageFlags;                      // Flags describing message
    DWORD               dwSenderTitleID;                     // ID of title in which message was sent
    WORD                wExpireMinutes;                      // An offset in minutes from the sent time
    WORD                cbDetails;                           // Size of details blob, excluding downloadable content
    char                szSenderName[XONLINE_GAMERTAG_SIZE]; // Gamer tag of sender
} XONLINE_MSG_SUMMARY, *PXONLINE_MSG_SUMMARY;
#pragma pack(pop)

typedef struct _XONLINE_MSG_SEND_RESULT
{
    XUID        xuidRecipient;  // User ID of recipient
    HRESULT     hr;             // Result code of send to recipient
    DWORD       dwMessageID;    // Message ID of send to recipient, if successful
} XONLINE_MSG_SEND_RESULT, *PXONLINE_MSG_SEND_RESULT;


XBOXAPI
HRESULT
WINAPI
XOnlineMessageEnumerate(
    IN DWORD dwUserIndex,
    OUT XONLINE_MSG_SUMMARY *pMsgSummaries,
    OUT DWORD *pdwNumMsgSummaries
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageSummary(
    IN DWORD dwUserIndex,
    IN DWORD dwMessageID,
    OUT XONLINE_MSG_SUMMARY *pMsgSummary
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDetails(
    IN DWORD dwUserIndex,
    IN DWORD dwMessageID,
    IN DWORD dwMessageFlagsToSet,
    IN DWORD dwMessageFlagsToClear,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDetailsGetResultsSummary(
    IN XONLINETASK_HANDLE hTask,
    OUT OPTIONAL XONLINE_MSG_SUMMARY *pMsgSummary,
    OUT OPTIONAL DWORD *pdwNumProperties,
    OUT OPTIONAL ULONGLONG *pqwAttachmentsSize
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDetailsGetResultsProperty(
    IN XONLINETASK_HANDLE hTask,
    IN WORD wPropTag,
    IN DWORD dwPropValueBufferSize,
    IN OUT OPTIONAL PVOID pPropValue,
    OUT OPTIONAL DWORD *pdwPropValueSize,
    OUT OPTIONAL DWORD *pdwAttachmentFlags
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDownloadAttachmentToMemory(
    IN XONLINETASK_HANDLE hDetailsTask,
    IN WORD wPropTag,
    IN OUT PBYTE pbBuffer,
    IN DWORD dwBufferSize,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phDownloadTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDownloadAttachmentToMemoryGetResults(
    IN XONLINETASK_HANDLE hDownloadTask,
    OUT OPTIONAL PBYTE *ppbReceivedData,
    OUT OPTIONAL DWORD *pdwReceivedDataSize,
    OUT OPTIONAL DWORD *pdwTotalDataSize
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDownloadAttachmentToDirectory(
    IN XONLINETASK_HANDLE hDetailsTask,
    IN WORD wPropTag,
    IN LPCSTR lpLocalPath,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phDownloadTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDownloadAttachmentGetProgress(
    IN XONLINETASK_HANDLE hDownloadTask,
    OUT OPTIONAL DWORD *pdwPercentDone,
    OUT OPTIONAL ULONGLONG *pqwNumerator,
    OUT OPTIONAL ULONGLONG *pqwDenominator
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageSetFlags(
    IN DWORD dwUserIndex,
    IN DWORD dwMessageID,
    IN DWORD dwMessageFlagsToSet,
    IN DWORD dwMessageFlagsToClear,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDelete(
    IN DWORD dwUserIndex,
    IN DWORD dwMessageID,
    IN BOOL fBlockSender
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageCreate(
    IN BYTE bMsgType,
    IN WORD wNumProperties,
    IN WORD wExpectedValuesSize,
    IN ULONGLONG qwMessageContext,
    IN DWORD dwMessageFlags,
    IN WORD wExpireMinutes,
    OUT XONLINE_MSG_HANDLE *phMsg
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageDestroy(
    IN XONLINE_MSG_HANDLE hMsg
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageSetProperty(
    IN XONLINE_MSG_HANDLE hMsg,
    IN WORD wPropTag,
    IN DWORD dwPropValueSize,
    IN const VOID *pPropValue,
    IN DWORD dwAttachmentFlags
    );
    

XBOXAPI
HRESULT
WINAPI
XOnlineMessageSend(
    IN DWORD dwUserIndexSender,
    IN XONLINE_MSG_HANDLE hMsg,
    IN DWORD dwRecipientCount,
    IN const XUID *pxuidRecipients,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageSendGetProgress(
    IN XONLINETASK_HANDLE hTask,
    OUT OPTIONAL DWORD *pdwPercentDone,
    OUT OPTIONAL ULONGLONG *pqwNumerator,
    OUT OPTIONAL ULONGLONG *pqwDenominator
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageSendGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT XONLINE_MSG_SEND_RESULT *pMsgSendResults
    );


XBOXAPI
HRESULT
WINAPI
XOnlineMessageRevoke(
    IN DWORD dwUserIndex,
    IN DWORD dwNumMsgSendResults,
    IN const XONLINE_MSG_SEND_RESULT *pMsgSendResults,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
BOOL
WINAPI
XOnlineMessageSetSummaryRefresh(
    IN BOOL fEnable
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMessageEnableReceivingFamilyTitleIDs(
    IN DWORD dwNumTitleIDs,
    IN const DWORD *pdwTitleIDs
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMessageSetSendingFamilyTitleID(
    IN DWORD dwTitleID
    );




//
// XOnline Matchmaking
//

typedef struct _XONLINE_ATTRIBUTE {
    DWORD dwAttributeID;
    BOOL fChanged;
    union {
        struct {
            ULONGLONG      qwValue;
        } integer;
        struct {
            LPWSTR         lpValue;
        } string;
        struct {
            PVOID          pvValue;
            DWORD          dwLength;
        } blob;
    } info;
} XONLINE_ATTRIBUTE, *PXONLINE_ATTRIBUTE;

typedef struct _XONLINE_ATTRIBUTE_SPEC {
    DWORD            dwType;
    DWORD            dwLength;
} XONLINE_ATTRIBUTE_SPEC, *PXONLINE_ATTRIBUTE_SPEC;

#define X_MAX_STRING_ATTRIBUTE_LEN         400
#define X_MAX_BLOB_ATTRIBUTE_LEN           800

#define X_ATTRIBUTE_SCOPE_TITLE_SPECIFIC   0x00000000

#define X_ATTRIBUTE_DATATYPE_MASK          0x00F00000
#define X_ATTRIBUTE_DATATYPE_INTEGER       0x00000000
#define X_ATTRIBUTE_DATATYPE_STRING        0x00100000
#define X_ATTRIBUTE_DATATYPE_BLOB          0x00200000
#define X_ATTRIBUTE_DATATYPE_NULL          0x00F00000

#define X_ATTRIBUTE_ID_MASK                0x0000FFFF

#pragma pack(push, 1)

typedef struct _XONLINE_MATCH_SEARCHRESULT
{
    DWORD     dwReserved;
    XNKID     SessionID;
    XNADDR    HostAddress;
    XNKEY     KeyExchangeKey;
    DWORD     dwPublicOpen;
    DWORD     dwPrivateOpen;
    DWORD     dwPublicFilled;
    DWORD     dwPrivateFilled;
    DWORD     dwNumAttributes;
} XONLINE_MATCH_SEARCHRESULT, *PXONLINE_MATCH_SEARCHRESULT;

#pragma pack(pop)

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSessionCreate(
    IN DWORD dwPublicFilled,
    IN DWORD dwPublicOpen,
    IN DWORD dwPrivateFilled,
    IN DWORD dwPrivateOpen,
    IN DWORD dwNumAttributes,
    IN OUT PXONLINE_ATTRIBUTE pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSessionUpdate(
    IN XNKID SessionID,
    IN DWORD dwPublicFilled,
    IN DWORD dwPublicOpen,
    IN DWORD dwPrivateFilled,
    IN DWORD dwPrivateOpen,
    IN DWORD dwNumAttributes,
    IN OUT PXONLINE_ATTRIBUTE pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSessionGetInfo(
    IN XONLINETASK_HANDLE hTask,
    OUT XNKID *pSessionID,
    OUT XNKEY *pKeyExchangeKey
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSessionDelete(
    IN XNKID SessionID,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSessionFindFromID(
    IN XNKID SessionID,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSearch(
    IN DWORD dwProcedureIndex,
    IN DWORD dwNumResults,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN DWORD dwResultsLen,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSearchGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINE_MATCH_SEARCHRESULT **prgpSearchResults,
    OUT DWORD *pdwReturnedResults
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMatchSearchParse(
    IN const XONLINE_MATCH_SEARCHRESULT *pSearchResult,
    IN DWORD dwNumSessionAttributes,
    IN const XONLINE_ATTRIBUTE_SPEC *pSessionAttributeSpec,
    OUT PVOID pQuerySession
    );

XBOXAPI
DWORD
WINAPI
XOnlineMatchSearchResultsLen(
    IN DWORD dwNumResults,
    IN DWORD dwNumSessionAttributes,
    IN const XONLINE_ATTRIBUTE_SPEC *pSessionAttributeSpec
    );


//
// Title Name
//

#define MAX_TITLENAME_LEN 40
#define MAX_TITLENAME_SIZE (MAX_TITLENAME_LEN * sizeof(WCHAR))


BOOL
WINAPI
XOnlineTitleIdIsSameTitle(
    IN DWORD dwTitleID
    );

BOOL
WINAPI
XOnlineTitleIdIsSamePublisher(
    IN DWORD dwTitleID
    );

//
// Notification
//

typedef enum {
    XONLINE_NOTIFICATION_FRIEND_REQUEST             = 0,
    XONLINE_NOTIFICATION_GAME_INVITE                = 1,
    XONLINE_NOTIFICATION_NEW_GAME_INVITE            = 2,
    XONLINE_NOTIFICATION_GAME_INVITE_ANSWER         = 3,
    XONLINE_NOTIFICATION_NUM                        = 4
} XONLINE_NOTIFICATION_TYPE;

XBOXAPI
HRESULT
WINAPI
XOnlineNotificationSetState(
    IN DWORD dwUserIndex,
    IN DWORD dwStateFlags,
    IN XNKID sessionID,
    IN DWORD dwStateData,
    IN const BYTE *pbStateData
    );

XBOXAPI
BOOL
WINAPI
XOnlineGetNotification(
    IN DWORD dwUserIndex,
    IN XONLINE_NOTIFICATION_TYPE NotificationType
    );

//
// Special State flags for XOnlineGetNotificationEx
//
#define XONLINE_NOTIFICATION_STATE_FLAG_PENDING_SYNC          (0x80000000)
#define XONLINE_NOTIFICATION_STATE_FLAG_MORE_ITEMS            (0x00000001)
#define XONLINE_NOTIFICATION_STATE_FLAG_OVERFLOW_ITEMS        (0x00000002)

#pragma pack(push, 1)

typedef struct 
{
    BYTE bMessageType;
    DWORD dwMessageID;
    DWORD dwNotifyFlags;
} XONLINE_NOTIFICATION_EX_INFO, *PXONLINE_NOTIFICATION_EX_INFO;

#pragma pack(pop)

XBOXAPI
BOOL
WINAPI
XOnlineGetNotificationEx(
    IN DWORD dwUserIndex,
    OUT PXONLINE_NOTIFICATION_EX_INFO pNotificationInfo,
    OUT DWORD *pdwStateFlags
    );


//
// Friends
//

#define XONLINE_FRIENDSTATE_FLAG_NONE              0x00000000
#define XONLINE_FRIENDSTATE_FLAG_ONLINE            0x00000001
#define XONLINE_FRIENDSTATE_FLAG_PLAYING           0x00000002
#define XONLINE_FRIENDSTATE_FLAG_VOICE             0x00000008
#define XONLINE_FRIENDSTATE_FLAG_JOINABLE          0x00000010
#define XONLINE_FRIENDSTATE_MASK_GUESTS            0x00000060
#define XONLINE_FRIENDSTATE_FLAG_RESERVED0         0x00000080
#define XONLINE_FRIENDSTATE_FLAG_SENTINVITE        0x04000000
#define XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE    0x08000000
#define XONLINE_FRIENDSTATE_FLAG_INVITEACCEPTED    0x10000000
#define XONLINE_FRIENDSTATE_FLAG_INVITEREJECTED    0x20000000
#define XONLINE_FRIENDSTATE_FLAG_SENTREQUEST       0x40000000
#define XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST   0x80000000

#define XOnlineGetGuests(dwState) ((dwState & XONLINE_FRIENDSTATE_MASK_GUESTS) >> 5)

typedef enum {
    XONLINE_REQUEST_NO = 0,
    XONLINE_REQUEST_YES = 1,
    XONLINE_REQUEST_BLOCK = 2
} XONLINE_REQUEST_ANSWER_TYPE;

typedef enum {
    XONLINE_GAMEINVITE_NO = 0,
    XONLINE_GAMEINVITE_YES = 1,
    XONLINE_GAMEINVITE_REMOVE = 2
} XONLINE_GAMEINVITE_ANSWER_TYPE;

#define MAX_FRIENDS         100
#define MAX_STATEDATA_SIZE  32
#define MAX_USERDATA_SIZE   0

#pragma pack(push, 1)
typedef struct _XONLINE_FRIEND {
    XUID                    xuid;
    CHAR                    szGamertag[XONLINE_GAMERTAG_SIZE];
    DWORD                   dwFriendState;
    FILETIME                gameinviteTime;
    XNKID                   sessionID;
    DWORD                   dwTitleID;
    BYTE                    StateDataSize;
    BYTE                    StateData[MAX_STATEDATA_SIZE];
    BYTE                    bReserved;
} XONLINE_FRIEND, *PXONLINE_FRIEND;
#pragma pack(pop)

typedef struct _XONLINE_ACCEPTED_GAMEINVITE{
    XONLINE_FRIEND InvitingFriend;
    XUID           xuidAcceptedFriend;
    FILETIME       InviteAcceptTime;
    XUID           xuidLogonUsers[XONLINE_MAX_LOGON_USERS];
} XONLINE_ACCEPTED_GAMEINVITE, *PXONLINE_ACCEPTED_GAMEINVITE;

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsStartup(
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsEnumerate(
    IN DWORD dwUserIndex,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsEnumerateFinish(
    IN XONLINETASK_HANDLE hTask
    );

XBOXAPI
DWORD
WINAPI
XOnlineFriendsGetLatest(
    IN DWORD dwUserIndex,
    IN DWORD dwFriendBufferCount,
    OUT PXONLINE_FRIEND pFriendBuffer
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsGetLatestByRange(
    IN DWORD dwUserIndex,
    IN DWORD dwRangeStart,
    IN OUT DWORD *pdwFriendBuffer,
    OUT PXONLINE_FRIEND pFriendBuffer,
    OUT DWORD *pdwFriendsBefore,
    OUT DWORD *pdwFriendsAfter
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsGetLatestByFocus(
    IN DWORD dwUserIndex,
    IN XUID xuidFriendFocus,
    IN DWORD dwBeforeFocus,
    IN OUT DWORD *pdwFriendBuffer,
    OUT PXONLINE_FRIEND pFriendBuffer,
    OUT DWORD *pdwFriendsBefore,
    OUT DWORD *pdwFriendsAfter
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsGetTitleName(
    IN DWORD dwTitleId,
    IN DWORD dwLanguage,
    IN DWORD dwMaxTitleNameChars,
    OUT LPWSTR lpTitleName
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRemove(
    IN DWORD dwUserIndex,
    IN const XONLINE_FRIEND *pFriend
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRequest(
    IN DWORD dwUserIndex,
    IN XUID xuidToUser
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRequestEx(
    IN DWORD dwUserIndex,
    IN XUID xuidToUser,
    IN XONLINE_MSG_HANDLE hMsg,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRequestByName(
    IN DWORD dwUserIndex,
    IN LPCSTR lpUserName,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRequestByNameEx(
    IN DWORD dwUserIndex,
    IN LPCSTR lpUserName,
    IN XONLINE_MSG_HANDLE hMsg,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsGameInvite(
    IN DWORD dwUserIndex,
    IN XNKID SessionID,
    IN DWORD dwFriendListCount,
    IN const XONLINE_FRIEND *pToFriendList
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsRevokeGameInvite(
    IN DWORD dwUserIndex,
    IN XNKID SessionID,
    IN DWORD dwFriendListCount,
    IN const XONLINE_FRIEND *pToFriendList
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsAnswerRequest(
    IN DWORD dwUserIndex,
    IN const XONLINE_FRIEND *pToFriend,
    IN XONLINE_REQUEST_ANSWER_TYPE Answer
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsAnswerGameInvite(
    IN DWORD dwUserIndex,
    IN const XONLINE_FRIEND *pToFriend,
    IN XONLINE_GAMEINVITE_ANSWER_TYPE Answer
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsJoinGame(
    IN DWORD dwUserIndex,
    IN const XONLINE_FRIEND *pToFriend
    );

XBOXAPI
HRESULT
WINAPI
XOnlineFriendsGetAcceptedGameInvite(
    OUT PXONLINE_ACCEPTED_GAMEINVITE pAcceptedGameInvite
    );


//
// Mute List
//

#define MAX_MUTELISTUSERS      250


typedef struct _XONLINE_MUTELISTUSER {
    XUID                    xuid;
    DWORD                   dwReserved;
} XONLINE_MUTELISTUSER, *PXONLINE_MUTELISTUSER;

XBOXAPI
HRESULT
WINAPI
XOnlineMutelistGet(
    IN DWORD dwUserIndex,
    IN DWORD dwMutelistUserBufferCount,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask,
    OUT PXONLINE_MUTELISTUSER pMutelistUsersBuffer,
    OUT DWORD *pdwNumMustlistUsers
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMutelistStartup(
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMutelistAdd(
    IN DWORD dwUserIndex,
    IN XUID xUserID
    );

XBOXAPI
HRESULT
WINAPI
XOnlineMutelistRemove(
    IN DWORD dwUserIndex,
    IN XUID xUserID
    );



//
// XOnline Nicknames
//

#define XONLINE_MAX_NICKNAME_SIZE 63

XBOXAPI
HRESULT
WINAPI
XOnlineVerifyNickname(
    IN LPCWSTR lpNickname,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


//
// XOnline Feedback
//

typedef enum {
    XONLINE_FEEDBACK_NEG_NICKNAME,
    XONLINE_FEEDBACK_NEG_GAMEPLAY,
    XONLINE_FEEDBACK_NEG_SCREAMING,
    XONLINE_FEEDBACK_NEG_HARASSMENT,
    XONLINE_FEEDBACK_NEG_LEWDNESS,
    XONLINE_FEEDBACK_POS_ATTITUDE,
    XONLINE_FEEDBACK_POS_SESSION,
    XONLINE_FEEDBACK_POS_STATS_ATTACHMENT,
    XONLINE_FEEDBACK_NEG_STATS_ATTACHMENT,
    XONLINE_FEEDBACK_NEG_STATS_ATTACHMENT_CHEATING,

    // Add new public types here

    NUM_XONLINE_FEEDBACK_TYPES,


} XONLINE_FEEDBACK_TYPE;

typedef struct
{
    LPCWSTR lpStringParam;
} XONLINE_FEEDBACK_PARAMS, *PXONLINE_FEEDBACK_PARAMS;

XBOXAPI
HRESULT
WINAPI
XOnlineFeedbackSend(
    IN DWORD dwUserIndex,
    IN XUID xTargetUser,
    IN XONLINE_FEEDBACK_TYPE FeedbackType,
    IN const XONLINE_FEEDBACK_PARAMS *pParams,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


//
// String service
//

XBOXAPI
HRESULT
WINAPI
XOnlineStringVerify(
    IN WORD wNumStrings,
    IN LPCWSTR* ppwStrings,
    IN DWORD dwLanguage,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStringVerifyGetResults(
    IN XONLINETASK_HANDLE hTask,
    IN WORD wNumResults,
    IN OUT HRESULT *pResults
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStringLookup(
    IN WORD wNumStringIDs,
    IN DWORD *pdwStringIDs,
    IN DWORD dwLanguage,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStringLookupGetResults(
    IN XONLINETASK_HANDLE hTask,
    IN OUT BYTE *pbBuffer,
    IN OUT DWORD *pdwBufferSize,
    IN OUT WCHAR **ppwszStrings,
    IN OUT WORD *pwNumStrings
    );



// 
// Query service
//

#define XENTITY_ID ULONGLONG

#define XONLINE_QUERY_MAX_ATTRIBUTES                    20
#define XONLINE_QUERY_MAX_STRING_ATTRIBUTE_LEN         400
#define XONLINE_QUERY_MAX_BLOB_ATTRIBUTE_LEN           800
#define XONLINE_QUERY_MAX_PAGE                         255
#define XONLINE_QUERY_MAX_PAGE_SIZE                    255
#define XONLINE_QUERY_MAX_FIND_NUM_ENTITYIDS            10
#define X_ATTRIBUTE_DATATYPE_ENTITY_ID                  X_ATTRIBUTE_DATATYPE_INTEGER         


XBOXAPI
HRESULT
WINAPI
XOnlineQueryAdd(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineQueryAddGetResults(
    IN XONLINETASK_HANDLE hTask, 
    OUT XENTITY_ID *pEntityId
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryUpdate(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN DWORD dwProcIndex,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryUpdateId(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN DWORD dwProcIndex,
    IN XENTITY_ID entityId,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
DWORD 
WINAPI
XOnlineQueryGetResultsBufferSize(
    IN DWORD dwNumResults,
    IN DWORD dwNumSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pSpecs
    );

XBOXAPI
HRESULT
WINAPI
XOnlineQuerySearch(
    IN DWORD dwDatasetId,
    IN DWORD dwProcIndex,
    IN DWORD dwPage,
    IN DWORD dwResultsPerPage,
    IN DWORD dwNumResultSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pAttributeSpecs,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQuerySearchGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTotalResults,
    OUT DWORD *pdwReturnedResults,
    IN OUT DWORD *pdwResultsSize,
    OUT PBYTE pbResults
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryFindFromIds(
    IN DWORD dwDatasetId,
    IN DWORD dwProcIndex,
    IN DWORD dwNumResultSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pAttributeSpecs,
    IN DWORD dwNumEntityIds,
    IN const XENTITY_ID *pEntityIds,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryFindFromIdsGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwReturnedResults,
    OUT PVOID pResults
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryRemove(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN DWORD dwProcIndex,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQueryRemoveId(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN XENTITY_ID entityId,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


XBOXAPI
HRESULT
WINAPI
XOnlineQuerySelect(
    IN DWORD dwUserIndex,
    IN ULONGLONG qwTeamId,
    IN DWORD dwDatasetId,
    IN XENTITY_ID entityId,
    IN DWORD dwAction,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );





//
// Game Usage Data
//

XBOXAPI
HRESULT
WINAPI
XOnlineGameDataUpload(
    IN LPCSTR szRelativeURL,
    IN DWORD dwFlags,
    IN DWORD dwTimeout,
    IN const BYTE *pbUsageData,
    IN DWORD dwUsageDataSize, 
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


//
// Peer gaming
//

#define XONLINE_ACCEPTED_GAMEINVITE_EXPIRATION_INTERVAL     (15 * 60) // seconds

typedef enum {
    XONLINE_PEER_ANSWER_NO,
    XONLINE_PEER_ANSWER_YES,
    XONLINE_PEER_ANSWER_NEVER
} XONLINE_PEER_ANSWER_TYPE;

typedef struct _PXONLINE_PEER_SESSION_RESULTS {
    XUID   xuid;
    DWORD  dwTitleID;
    DWORD  dwTitleVersion;
    DWORD  dwTitleRegion;
    XNADDR xnaddr;
    XNKID  xkid;
    XNKEY  xnkey;
} XONLINE_PEER_SESSION_RESULTS, *PXONLINE_PEER_SESSION_RESULTS;

typedef struct _XONLINE_GAMEINVITE_ANSWER_INFO{
    XUID     xuidInvitingUser;
    CHAR     szInvitingUserGamertag[XONLINE_GAMERTAG_SIZE];
    DWORD    dwTitleID;
    XNKID    SessionID;
    FILETIME GameInviteTime;
} XONLINE_GAMEINVITE_ANSWER_INFO, *PXONLINE_GAMEINVITE_ANSWER_INFO;

typedef struct _XONLINE_LATEST_ACCEPTED_GAMEINVITE{
    XUID     xuidAcceptedUser;
    XUID     xuidInvitingUser;
    CHAR     szInvitingUserGamertag[XONLINE_GAMERTAG_SIZE];
    XNKID    SessionID;
    FILETIME InviteAcceptTime;
    XUID     xuidLogonUsers[XONLINE_MAX_LOGON_USERS];
} XONLINE_LATEST_ACCEPTED_GAMEINVITE, *PXONLINE_LATEST_ACCEPTED_GAMEINVITE;

typedef struct _XONLINE_GAME_JOIN_INFO{
    XUID  xuidJoinedUser;
    CHAR  szJoinedUserGamertag[XONLINE_GAMERTAG_SIZE];
    DWORD dwTitleID;
    XNKID SessionID;
} XONLINE_GAME_JOIN_INFO, *PXONLINE_GAME_JOIN_INFO;


XBOXAPI
HRESULT
WINAPI
XOnlineGetSession(
    OUT XNADDR *pxnaddr,
    OUT XNKID *pxnkid,
    OUT XNKEY *pxnkey
    );

XBOXAPI
HRESULT
WINAPI
XOnlineGetUserSession(
    IN DWORD dwUserIndex, 
    IN XUID xuidPeer,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask, 
    OUT PXONLINE_PEER_SESSION_RESULTS pResults
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineGameInviteSend(
    IN DWORD dwUserIndex,
    IN DWORD dwPeerCount,
    IN const XUID *pxuidPeersToInvite,
    IN XNKID SessionID,
    IN DWORD dwFlags,
    IN XONLINE_MSG_HANDLE hMsg,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineGameInviteAnswer(
    IN DWORD dwUserIndex,
    IN const XONLINE_GAMEINVITE_ANSWER_INFO *pGameInviteAnswerInfo,
    IN XONLINE_PEER_ANSWER_TYPE GameInviteAnswer,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineGameInviteRevoke(
    IN DWORD dwUserIndex,
    IN DWORD dwPeerCount,
    IN const XUID *pxuidPeersToRevoke,
    IN XNKID SessionID,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineGameInviteGetLatestAccepted(
    OUT XONLINE_LATEST_ACCEPTED_GAMEINVITE *pLatestAcceptedGameInvite
    );    

XBOXAPI
HRESULT
WINAPI
XOnlineGameJoin(
    IN DWORD dwUserIndex,
    IN const XONLINE_GAME_JOIN_INFO *pGameJoinInfo
    );

    



//
// Teams 
//

#define XONLINE_MAX_TEAM_COUNT                   8
#define XONLINE_MAX_TEAM_MEMBER_COUNT            100

//
// Unicode zero-teminated strings length 
//

#define XONLINE_MAX_TEAM_NAME_SIZE               16
#define XONLINE_MAX_TEAM_DESCRIPTION_SIZE        256
#define XONLINE_MAX_TEAM_MOTTO_SIZE              256
#define XONLINE_MAX_TEAM_URL_SIZE                256

//
// Custom data(bytes)
//

#define XONLINE_MAX_TEAM_DATA_SIZE               100
#define XONLINE_MAX_TEAM_MEMBER_DATA_SIZE        100



#define XONLINE_TEAM_MSG_RECRUIT                 0x00000001
#define XONLINE_TEAM_MSG_GAME_INVITE             0x00000002

typedef enum {
    XONLINE_TEAM_DELETE                    = 0x00000001,
    XONLINE_TEAM_MODIFY_DATA               = 0x00000002,
    XONLINE_TEAM_MODIFY_MEMBER_PERMISSIONS = 0x00000004,
    XONLINE_TEAM_DELETE_MEMBER             = 0x00000008,
    XONLINE_TEAM_RECRUIT_MEMBERS           = 0x00000010,
    
    XONLINE_TEAM_LIVE_PERMISSIONS_FORCE_DWORD = 0xFFFFFFFF
} XONLINE_TEAM_LIVE_PERMISSIONS;

#pragma pack(push, 1)
typedef struct _XONLINE_TEAM_PROPERTIES {
    WCHAR    wszTeamName[XONLINE_MAX_TEAM_NAME_SIZE];
    WCHAR    wszDescription[XONLINE_MAX_TEAM_DESCRIPTION_SIZE];
    WCHAR    wszMotto[XONLINE_MAX_TEAM_MOTTO_SIZE];
    WCHAR    wszURL[XONLINE_MAX_TEAM_URL_SIZE];
    WORD     TeamDataSize;
    BYTE     TeamData[XONLINE_MAX_TEAM_DATA_SIZE];
} XONLINE_TEAM_PROPERTIES, *PXONLINE_TEAM_PROPERTIES;

typedef struct _XONLINE_TEAM {
    XUID     xuidTeam;
    XONLINE_TEAM_PROPERTIES TeamProperties;
    DWORD    dwFlags; // XONLINE_TEAM_MSG_* combinations
    FILETIME CreationTime;
    DWORD    dwMemberCount;
} XONLINE_TEAM, *PXONLINE_TEAM;

typedef struct _XONLINE_TEAM_MEMBER_PROPERTIES {
  DWORD    dwPrivileges;
  WORD     TeamMemberDataSize;
  BYTE     TeamMemberData[XONLINE_MAX_TEAM_MEMBER_DATA_SIZE];
} XONLINE_TEAM_MEMBER_PROPERTIES, *PXONLINE_TEAM_MEMBER_PROPERTIES;
                                    
typedef struct _XONLINE_TEAM_MEMBER {
    XUID                 xuidTeamMember;
    CHAR                 szGamertag[XONLINE_GAMERTAG_SIZE];
    XONLINE_TEAM_MEMBER_PROPERTIES TeamMemberProperties;
    DWORD                dwFlags; // XONLINE_TEAM_MSG_* combinations
    FILETIME             JoinDate;
} XONLINE_TEAM_MEMBER, *PXONLINE_TEAM_MEMBER;
#pragma pack(pop)

//
// Teams Managing
//

XBOXAPI
HRESULT
WINAPI
XOnlineTeamCreate(
    IN DWORD dwUserIndex,
    IN const XONLINE_TEAM_PROPERTIES *pTeamProperties,
    IN const XONLINE_TEAM_MEMBER_PROPERTIES *pFirstTeamMemberProperties,
    IN DWORD dwMaxTeamMemberCount,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamCreateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT XONLINE_TEAM *pTeam
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamSetProperties(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN const XONLINE_TEAM_PROPERTIES *pTeamProperties,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamDelete(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
   
XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberSetProperties(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN XUID xuidTeamMember,
    IN const XONLINE_TEAM_MEMBER_PROPERTIES *pTeamMemberProperties,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
       
XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberRemove(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN XUID xuidTeamMember,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberRecruit(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN XUID xuidPeer,
    IN const XONLINE_TEAM_MEMBER_PROPERTIES *pPeerTeamMemberInfo,
    IN XONLINE_MSG_HANDLE hMsg,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberRecruitByName(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN LPCSTR lpPeerName,
    IN const XONLINE_TEAM_MEMBER_PROPERTIES *pPeerTeamMemberInfo,
    IN XONLINE_MSG_HANDLE hMsg,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberAnswerRecruit(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN XONLINE_PEER_ANSWER_TYPE RecruitAnswer,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

//
// Teams Listing
//

XBOXAPI
HRESULT
WINAPI
XOnlineTeamEnumerate(
    IN DWORD dwUserIndex,
    IN DWORD dwTeamCount,
    IN const XUID *pxuidTeams,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTeamEnumerateByUserXUID(
    IN DWORD dwUserIndex,
    IN XUID xuidUser,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTeamEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTeamCount,
    OUT XUID *pxuidTeams
    );

XBOXAPI
HRESULT
WINAPI
XOnlineTeamGetDetails(
    IN XONLINETASK_HANDLE hTask,
    IN XUID xuidTeam,
    OUT XONLINE_TEAM *pTeamInfo
    );      

XBOXAPI
HRESULT
WINAPI
XOnlineTeamMembersEnumerate(
    IN DWORD dwUserIndex,
    IN XUID xuidTeam,
    IN DWORD dwFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamMembersEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTeamMemberCount,
    OUT XUID *pxuidTeamMembers
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlineTeamMemberGetDetails(
    IN XONLINETASK_HANDLE hTask,
    IN XUID xuidTeamMember,
    OUT XONLINE_TEAM_MEMBER *pTeamMemberInfo
    ); 

XBOXAPI
HRESULT
WINAPI
XOnlineTeamSetFamilyTitleID(
    IN DWORD dwTitleID
    );

//
// Presence
//  

#define XONLINE_MAX_PRESENCE_USERS_COUNT                1000

#define XONLINE_PRESENCE_FLAG_NONE                      XONLINE_FRIENDSTATE_FLAG_NONE
#define XONLINE_PRESENCE_FLAG_ONLINE                    XONLINE_FRIENDSTATE_FLAG_ONLINE
#define XONLINE_PRESENCE_FLAG_PLAYING                   XONLINE_FRIENDSTATE_FLAG_PLAYING
#define XONLINE_PRESENCE_FLAG_VOICE                     XONLINE_FRIENDSTATE_FLAG_VOICE
#define XONLINE_PRESENCE_FLAG_JOINABLE                  XONLINE_FRIENDSTATE_FLAG_JOINABLE
#define XONLINE_PRESENCE_MASK_GUESTS                    XONLINE_FRIENDSTATE_MASK_GUESTS
#define XONLINE_PRESENCE_FLAG_RECEIVEDINVITE            XONLINE_FRIENDSTATE_FLAG_RECEIVEDINVITE
#define XONLINE_PRESENCE_FLAG_RECEIVEDREQUEST           XONLINE_FRIENDSTATE_FLAG_RECEIVEDREQUEST
#define XONLINE_PRESENCE_FLAG_RECEIVEDTEAMRECRUIT       0x00000100
#define XONLINE_PRESENCE_FLAG_RECEIVEDCOMPREMINDER      0x00000200
#define XONLINE_PRESENCE_FLAG_RECEIVEDCOMPREQUEST       0x00000400
#define XONLINE_PRESENCE_FLAG_RECEIVEDTITLECUSTOM       0x00000800

#pragma pack(push, 1)
typedef struct _XONLINE_PRESENCE {
    XUID     xuid;
    DWORD    dwUserState;
    XNKID    SessionID;
    DWORD    dwTitleID;
    BYTE     StateDataSize;
    BYTE     StateData[MAX_STATEDATA_SIZE];
} XONLINE_PRESENCE, *PXONLINE_PRESENCE;
#pragma pack(pop)

XBOXAPI
HRESULT
WINAPI
XOnlinePresenceInit(
    IN DWORD dwUserIndex,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlinePresenceAdd(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwGroupID,
    IN DWORD dwUserCount,
    IN XUID *pxuidUsers
    );

XBOXAPI
HRESULT
WINAPI
XOnlinePresenceClear(
    IN XONLINETASK_HANDLE hTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlinePresenceSubmit(
    IN XONLINETASK_HANDLE hTask
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlinePresenceGetLatest(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwGroupID,
    IN DWORD dwUserPresenceBufferCount,
    OUT XONLINE_PRESENCE *pUserPresence
    );
    
XBOXAPI
HRESULT
WINAPI
XOnlinePresenceGetTitleName(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwTitleID,
    IN DWORD dwLanguage,
    IN DWORD dwTitleNameSize,
    OUT LPWSTR wszTitleName
    );


//
// Statistics
//

#define XONLINE_STAT_MAX_SPECS_IN_WRITE_REQUEST 20
#define XONLINE_STAT_MAX_STATS_IN_SPEC          64
#define XONLINE_STAT_MAX_NICKNAME_LENGTH        32
#define XONLINE_STAT_MAX_PROCEDURE_COUNT        100
#define XONLINE_STAT_MAX_MEMBERS_IN_UNIT        4
#define XONLINE_STAT_MAX_UNITS                  100
#define XONLINE_STAT_MAX_PARAM_COUNT            256
#define XONLINE_STAT_MAX_NUM_UNIT_READ_SPECS    5

#define XONLINE_STAT_RANK                       ((WORD)0xFFFF)
#define XONLINE_STAT_RATING                     ((WORD)0xFFFE)
#define XONLINE_STAT_NICKNAME                   ((WORD)0xFFFD)
#define XONLINE_STAT_LEADERBOARD_SIZE           ((WORD)0xFFFC)
#define XONLINE_STAT_ATTACHMENT_PATH            ((WORD)0xFFFB)
#define XONLINE_STAT_ATTACHMENT_SIZE            ((WORD)0xFFFA)
#define XONLINE_STAT_UNIT_ACTIVITY_COUNTER      ((WORD)0xFFF9)
#define XONLINE_STAT_UNIT_LAST_ACTIVITY_DATE    ((WORD)0xFFF8)

#define XONLINE_STAT_COMPTYPE_EQUAL             1 // if the current stored stat value equals the specified value
#define XONLINE_STAT_COMPTYPE_GREATER           2 // if the current stored stat value is greater than the specified value
#define XONLINE_STAT_COMPTYPE_GREATER_OR_EQUAL  3 // if the current stored stat value is greater than or equal to the specified value
#define XONLINE_STAT_COMPTYPE_LESS              4 // if the current stored stat value is less than the specified value
#define XONLINE_STAT_COMPTYPE_LESS_OR_EQUAL     5 // if the current stored stat value is less than or equal to the specified value
#define XONLINE_STAT_COMPTYPE_EXISTS            6 // if the current stored stat value exists (specified value is ignored)
#define XONLINE_STAT_COMPTYPE_NOT_EXISTS        7 // if the current stored stat value does not exist (specified value is ignored)
#define XONLINE_STAT_COMPTYPE_NOT_EQUAL         8 // if the current stored stat value does not equal the specified value

#define XONLINE_STAT_PROCID_UPDATE_REPLACE          0x8001 // use XONLINE_STAT_UPDATE structure
#define XONLINE_STAT_PROCID_UPDATE_REPLACE_UNIT     0x8002 // use XONLINE_STAT_UPDATE_UNIT structure
#define XONLINE_STAT_PROCID_UPDATE_INCREMENT        0x8003 // use XONLINE_STAT_UPDATE structure
#define XONLINE_STAT_PROCID_UPDATE_INCREMENT_UNIT   0x8004 // use XONLINE_STAT_UPDATE_UNIT structure
#define XONLINE_STAT_PROCID_ELO                     0x8005 // use XONLINE_STAT_ELO structure
#define XONLINE_STAT_PROCID_ELO_UNIT                0x8006 // use XONLINE_STAT_ELO_UNIT structure
#define XONLINE_STAT_PROCID_CONDITIONAL             0x8007 // use XONLINE_STAT_CONDITIONAL structure
#define XONLINE_STAT_PROCID_CONDITIONAL_UNIT        0x8008 // use XONLINE_STAT_CONDITIONAL_UNIT structure
// All other procedure IDs are custom and use the XONLINE_STAT_CUSTOM structure

typedef enum _XONLINE_STAT_TYPE {
    XONLINE_STAT_NONE,
    XONLINE_STAT_LONG,
    XONLINE_STAT_LONGLONG,
    XONLINE_STAT_DOUBLE,
    XONLINE_STAT_LPCWSTR
} XONLINE_STAT_TYPE;

typedef enum _XONLINE_STAT_PARAM_TYPE {
    XONLINE_STAT_PARAM_NONE,
    XONLINE_STAT_PARAM_BYTE,
    XONLINE_STAT_PARAM_WORD,
    XONLINE_STAT_PARAM_LONG,
    XONLINE_STAT_PARAM_LONGLONG,
    XONLINE_STAT_PARAM_DOUBLE,
    XONLINE_STAT_PARAM_LPCWSTR,
    XONLINE_STAT_PARAM_XUID
} XONLINE_STAT_PARAM_TYPE;

typedef enum _XONLINE_STAT_SORTORDER {
    XONLINE_STAT_SORTORDER_LASTACTIVITY,
    XONLINE_STAT_SORTORDER_RATING
} XONLINE_STAT_SORTORDER;

typedef struct _XONLINE_STAT {
    WORD wID;
    XONLINE_STAT_TYPE type;
    union {
        LONG     lValue;
        LONGLONG llValue;
        double   dValue;
        LPCWSTR  lpString;
    };
} XONLINE_STAT, *PXONLINE_STAT;

typedef struct _XONLINE_STAT_SPEC {
    XUID          xuidUser;
    DWORD         dwLeaderBoardID;
    DWORD         dwNumStats;
    PXONLINE_STAT pStats;
} XONLINE_STAT_SPEC, *PXONLINE_STAT_SPEC;

typedef struct _XONLINE_STAT_SPEC_UNIT {
    DWORD         dwLeaderBoardID;
    DWORD         dwNumStats;
    PXONLINE_STAT pStats;
} XONLINE_STAT_SPEC_UNIT, *PXONLINE_STAT_SPEC_UNIT;

typedef struct _XONLINE_STAT_USER {
    XUID xuidUser;
    union {
        CHAR  szGamertag[XONLINE_GAMERTAG_SIZE];
        WCHAR wszTeamName[XONLINE_MAX_TEAM_NAME_SIZE];
    };
} XONLINE_STAT_USER, *PXONLINE_STAT_USER;

typedef struct _XONLINE_STAT_NAME {
    union
    {
        CHAR  szGamertag[XONLINE_GAMERTAG_SIZE];
        WCHAR wszTeamName[XONLINE_MAX_TEAM_NAME_SIZE];
    };
} XONLINE_STAT_NAME, *PXONLINE_STAT_NAME;

typedef struct _XONLINE_STAT_UNIT {
    XUID               xuidUnitMembers[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
    XONLINE_STAT_NAME  UnitMemberNames[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
} XONLINE_STAT_UNIT, *PXONLINE_STAT_UNIT;

#pragma pack(push, 1)

typedef struct {
    DWORD     dwLeaderboardIndex;
    ULONGLONG qwUserPuid;
} XONLINE_STAT_ATTACHMENT_REFERENCE, *PXONLINE_STAT_ATTACHMENT_REFERENCE;

#pragma pack(pop)

typedef struct _XONLINE_STAT_CUSTOM_PARAM {
    XONLINE_STAT_PARAM_TYPE type;
    union {
        BYTE     bValue;
        WORD     wValue;
        LONG     lValue;
        LONGLONG llValue;
        double   dValue;
        LPCWSTR  lpString;
        XUID     xuidValue;
    };
} XONLINE_STAT_CUSTOM_PARAM, *PXONLINE_STAT_CUSTOM_PARAM;

typedef struct _XONLINE_STAT_UPDATE {
    XUID          xuid;
    DWORD         dwLeaderBoardID;
    DWORD         dwConditionalIndex; // one-based index of XONLINE_STAT_CONDITIONAL procedure that determines whether this update occurs, or 0 if always updated
    DWORD         dwNumStats;
    PXONLINE_STAT pStats;
} XONLINE_STAT_UPDATE, *PXONLINE_STAT_UPDATE;

typedef struct _XONLINE_STAT_UPDATE_UNIT {
    XUID          xuidUnitMembers[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
    DWORD         dwLeaderBoardID;
    DWORD         dwConditionalUnitIndex; // one-based index of XONLINE_STAT_CONDITIONAL_UNIT procedure that determines whether this update occurs, or 0 if always updated
    DWORD         dwNumStats;
    PXONLINE_STAT pStats;
} XONLINE_STAT_UPDATE_UNIT, *PXONLINE_STAT_UPDATE_UNIT;

typedef struct _XONLINE_STAT_CONDITIONAL {
    XUID         xuid;
    DWORD        dwLeaderBoardID;
    BYTE         bComparisonType;
    XONLINE_STAT StatToCompare;
} XONLINE_STAT_CONDITIONAL, *PXONLINE_STAT_CONDITIONAL;

typedef struct _XONLINE_STAT_CONDITIONAL_UNIT {
    XUID         xuidUnitMembers[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
    DWORD        dwLeaderBoardID;
    BYTE         bComparisonType;
    XONLINE_STAT StatToCompare;
} XONLINE_STAT_CONDITIONAL_UNIT, *PXONLINE_STAT_CONDITIONAL_UNIT;

typedef struct _XONLINE_STAT_ELO {
    XUID   xuid1;
    XUID   xuid2;
    DWORD  dwLeaderBoardID;
    DWORD  dwConditionalIndex; // one-based index of XONLINE_STAT_CONDITIONAL procedure that determines whether this update occurs, or 0 if always updated
    double W;
    double C1;
    double C2;
} XONLINE_STAT_ELO, *PXONLINE_STAT_ELO;

typedef struct _XONLINE_STAT_ELO_UNIT {
    XUID   xuidUnit1Members[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
    XUID   xuidUnit2Members[XONLINE_STAT_MAX_MEMBERS_IN_UNIT];
    DWORD  dwLeaderBoardID;
    DWORD  dwConditionalUnitIndex; // one-based index of XONLINE_STAT_CONDITIONAL_UNIT procedure that determines whether this update occurs, or 0 if always updated
    double W;
    double C1;
    double C2;
} XONLINE_STAT_ELO_UNIT, *PXONLINE_STAT_ELO_UNIT;

typedef struct _XONLINE_STAT_CUSTOM {
    DWORD                       dwNumParams;
    XONLINE_STAT_CUSTOM_PARAM *pParams;
} XONLINE_STAT_CUSTOM, *PXONLINE_STAT_CUSTOM;

typedef struct _XONLINE_STAT_PROC {
    WORD wProcedureID;
    union {
        XONLINE_STAT_UPDATE              Update;
        XONLINE_STAT_UPDATE_UNIT         UpdateUnit;
        XONLINE_STAT_CONDITIONAL         Conditional;
        XONLINE_STAT_CONDITIONAL_UNIT    ConditionalUnit;
        XONLINE_STAT_ELO                 Elo;
        XONLINE_STAT_ELO_UNIT            EloUnit;
        XONLINE_STAT_CUSTOM              Custom;
    };
} XONLINE_STAT_PROC, *PXONLINE_STAT_PROC;


XBOXAPI
HRESULT
WINAPI
XOnlineStatWrite(
    IN DWORD dwNumStatSpecs,
    IN const XONLINE_STAT_SPEC *pStatSpecs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatWriteEx(
    IN DWORD dwNumStatProcs,
    IN const XONLINE_STAT_PROC *pStatProcs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatWriteGetResult(
    IN XONLINETASK_HANDLE hTask,
    OUT HANDLE *phServerFileReference,
    OUT PXONLINE_STAT_ATTACHMENT_REFERENCE *prgReferences,
    OUT DWORD *pdwReferences
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatRead(
    IN DWORD dwNumStatSpecs,
    IN const XONLINE_STAT_SPEC *pStatSpecs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatReadGetResult(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwNumStatSpecs,
    OUT PXONLINE_STAT_SPEC pStatSpecs,
    IN DWORD dwExtraBufferSize,
    IN OUT BYTE *pExtraBuffer
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatUnitRead(
    IN const XUID *pxuidUnitMembers,
    IN DWORD dwNumStatSpecUnits,
    IN const XONLINE_STAT_SPEC_UNIT *pStatSpecUnits,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatUnitReadGetResult(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwNumStatSpecUnits,
    OUT XONLINE_STAT_SPEC_UNIT *pStatSpecUnits
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatLeaderEnumerate(
    IN const XUID *pxuidPagePivot,
    IN DWORD dwPageStart,
    IN DWORD dwPageSize,
    IN DWORD dwLeaderboardID,
    IN DWORD dwNumStatsPerUser,
    IN const WORD *pStatsPerUser,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatLeaderEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwUserCount,
    OUT PXONLINE_STAT_USER pUsers,
    IN DWORD dwStatCount,
    OUT PXONLINE_STAT pStats,
    OUT DWORD *pdwLeaderboardSize,
    OUT DWORD *pdwReturnedResults,
    IN DWORD dwExtraBufferSize,
    IN OUT BYTE *pExtraBuffer
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatUnitEnumerate(
    IN XUID xuidUnitMember,
    IN DWORD dwLeaderboardID,
    IN XONLINE_STAT_SORTORDER SortOrder,
    IN DWORD dwMaxUnitsToReturn,
    IN DWORD dwNumStatsPerUnit,
    IN const WORD *pStatsPerUnit,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatUnitEnumerateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINE_STAT_UNIT pUnits,
    IN DWORD dwStatCount,
    OUT XONLINE_STAT *pStats,
    OUT DWORD *pdwReturnedResults
    );

XBOXAPI
HRESULT
WINAPI
XOnlineStatReset(
    XUID xuid,
    DWORD dwLeaderBoardId,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );



//
// Arbitration
//

#define XONLINE_ARB_MAX_SUSPICIOUS_INFO_MESSAGE_LENGTH   256 // max characters, not including null termination

#define XONLINE_ARB_REGISTER_FLAG_USER_COMPETITION       0x00000001 // Arbitrated round is part of a user-organized competition
#define XONLINE_ARB_REGISTER_FLAG_PUBLISHER_COMPETITION  0x00000002 // Arbitrated round is part of a publisher-organized competition
#define XONLINE_ARB_REGISTER_FLAG_TIME_EXTENDABLE        0x00000004 // Arbitrated round duration can be extended with XOnlineArbitrationExtendRound
#define XONLINE_ARB_REGISTER_FLAG_HOST_FORWARDS_PACKETS  0x00000008 // Host will forward packets during arbitrated round
#define XONLINE_ARB_REGISTER_FLAG_TEAMS                  0x00000010 // Arbitrated round includes team participants
#define XONLINE_ARB_REGISTER_FLAG_FFA                    0x00000020 // Arbitrated round is free-for-all style of gameplay

#define XONLINE_ARB_REPORT_FLAG_WAS_HOST                 0x00000001 // Caller was the host for the arbitrated round
#define XONLINE_ARB_REPORT_FLAG_VOLUNTARILY_QUITTING     0x00000002 // Caller accepts disconnect penalty and stats submitted by other participants, but wants to report connectivity or suspicious info before leaving


typedef struct _XONLINE_ARB_ID {
    XNKID         SessionID;            // Session ID
    ULONGLONG     qwRoundID;            // Arbitration round ID
} XONLINE_ARB_ID, *PXONLINE_ARB_ID;

typedef struct _XONLINE_ARB_REGISTRANT {
    ULONGLONG qwMachineID;                        // Machine ID for this registrant
    XUID      xuidUsers[XONLINE_MAX_LOGON_USERS]; // Array of users logged on by this registrant
    BYTE      bReliabilityValue;                  // Relative reliability value for this registrant
} XONLINE_ARB_REGISTRANT, *PXONLINE_ARB_REGISTRANT;

#pragma pack(push, 4)
typedef struct _XONLINE_ARB_SUSPICIOUS_INFO {
    CHAR   *pszMessage;              // Pointer to suspicious activity message string, up to XONLINE_ARB_MAX_SUSPICIOUS_INFO_MESSAGE_LENGTH characters
    BYTE    bNumRelatedAddresses;    // Number of addresses involved in suspicious activity
    XNADDR *pxnaddrRelatedAddresses; // Array of addresses involved in suspicious activity
    BYTE    bNumRelatedUsers;        // Number of users involved in suspicious activity
    XUID   *pxuidRelatedUsers;       // Array of users involved in suspicious activity
} XONLINE_ARB_SUSPICIOUS_INFO, *PXONLINE_ARB_SUSPICIOUS_INFO;

typedef struct _XONLINE_ARB_REPORT_DATA {
    BYTE                         bNumLostConnectivityAddresses;    // Number of addresses in lost-connectivity array
    XNADDR                      *pxnaddrLostConnectivityAddresses; // Array of addresses to whom connectivity was lost
    XONLINE_ARB_SUSPICIOUS_INFO *pSuspiciousInfoType1;             // Pointer to type 1 suspicious activity information
    XONLINE_ARB_SUSPICIOUS_INFO *pSuspiciousInfoType2;             // Pointer to type 2 suspicious activity information
    XONLINE_ARB_SUSPICIOUS_INFO *pSuspiciousInfoType3;             // Pointer to type 3 suspicious activity information
} XONLINE_ARB_REPORT_DATA, *PXONLINE_ARB_REPORT_DATA;
#pragma pack(pop)



XBOXAPI
HRESULT
WINAPI
XOnlineArbitrationCreateRoundID(
    OUT ULONGLONG *pqwRoundID
    );

XBOXAPI
HRESULT
WINAPI
XOnlineArbitrationRegister(
    IN const XONLINE_ARB_ID *pArbID,
    IN WORD wMaxRoundSeconds,
    IN DWORD dwFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineArbitrationRegisterGetResults(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwRegistrantsBufferCount,
    IN OUT XONLINE_ARB_REGISTRANT *pRegistrants,
    OUT DWORD *pdwNumRegistrants
    );

XBOXAPI
HRESULT
WINAPI
XOnlineArbitrationExtendRound(
    IN const XONLINE_ARB_ID *pArbID,
    IN WORD wMaxSecondsFromNow,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineArbitrationReport(
    IN const XONLINE_ARB_ID *pArbID,
    IN DWORD dwNumStatProcs,
    IN OPTIONAL const XONLINE_STAT_PROC *pStatProcs,
    IN OPTIONAL const XONLINE_ARB_REPORT_DATA *pReportData,
    IN DWORD dwFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );


//
// Competitions
//

//
// Attribute IDs
//
#define XONLINE_QUERY_ENTITY_ID                   0x80000000
#define XONLINE_QUERY_OWNER_PUID                  0x80000001

#define XONLINE_COMP_ATTR_ID                      0x80010000
#define XONLINE_COMP_ATTR_REG_OPEN                0x80010001
#define XONLINE_COMP_ATTR_REG_CLOSE               0x80010002
#define XONLINE_COMP_ATTR_COMP_START              0x80010003
#define XONLINE_COMP_ATTR_COMP_CLEANUP            0x80010004
#define XONLINE_COMP_ATTR_ROUND_FREQUENCY         0x80010005
#define XONLINE_COMP_ATTR_ROUND_INTERVAL          0x80010006
#define XONLINE_COMP_ATTR_ROUND_DAY_MASK          0x80010007
#define XONLINE_COMP_ATTR_ROUNDS                  0x80010008
#define XONLINE_COMP_ATTR_CURRENT_ROUND           0x80010009
#define XONLINE_COMP_ATTR_CURRENT_ROUND_START     0x8001000A
#define XONLINE_COMP_ATTR_CURRENT_ROUND_END       0x8001000B
#define XONLINE_COMP_ATTR_ROUND_DURATION_MIN      0x8001000B
#define XONLINE_COMP_ATTR_ROUND_ONE_START         0x8001000C
#define XONLINE_COMP_ATTR_ROUND_ONE_END           0x8001000D
#define XONLINE_COMP_ATTR_AUTOPROGRESS_DEADLINE   0x8001000F
#define XONLINE_COMP_ATTR_IS_TEAM_COMP            0x80010010
#define XONLINE_COMP_ATTR_TEAM_SIZE               0x80010011
#define XONLINE_COMP_ATTR_MAX_PUBLIC_SLOTS        0x80010012
#define XONLINE_COMP_ATTR_MAX_PRIVATE_SLOTS       0x80010013
#define XONLINE_COMP_ATTR_MIN_SLOTS               0x80010014
#define XONLINE_COMP_ATTR_BYES                    0x80010016
#define XONLINE_COMP_ATTR_BYES_GRANTED            0x80010017
#define XONLINE_COMP_ATTR_PUBLIC_ENTRANTS         0x80010018
#define XONLINE_COMP_ATTR_PRIVATE_ENTRANTS        0x80010019        
#define XONLINE_COMP_ATTR_REMINDER_MIN            0x8001001A        
#define XONLINE_COMP_ATTR_PLAY_BEFORE_MIN         0x8001001B        
#define XONLINE_COMP_ATTR_PLAY_AFTER_MIN          0x8001001C                
#define XONLINE_COMP_ATTR_STATUS                  0x8001001D 
#define XONLINE_COMP_ATTR_ROUND0_LEADERBOARD_ID   0x80010020 
#define XONLINE_COMP_ATTR_DEBUG_ADVANCE_TIME      0x80010023
       
#define XONLINE_COMP_ATTR_RESULTS                 0x80210000


#define XONLINE_COMP_ATTR_ENTRANT_PUID            0x80020000
#define XONLINE_COMP_ATTR_ENTRANT_STATUS          0x80020001
#define XONLINE_COMP_ATTR_ENTRANT_TRUST           0x80020002
#define XONLINE_COMP_ATTR_ENTRANT_CURRENT_ROUND   0x80020003
#define XONLINE_COMP_ATTR_ENTRANT_PRIVATE_SLOT    0x80020004
#define XONLINE_COMP_ATTR_ENTRANT_SEED            0x80020005
#define XONLINE_COMP_ATTR_ENTRANT_ELIMINATED      0x80020006
#define XONLINE_COMP_ATTR_ENTRANT_CURRENT_EVENT   0x80020007
#define XONLINE_COMP_ATTR_ENTRANT_CURRENT_START   0x80020008
#define XONLINE_COMP_ATTR_ENTRANT_COMP_SORT       0x80020009


#define XONLINE_COMP_ATTR_EVENT_ENTITY_ID         0x80030000
#define XONLINE_COMP_ATTR_EVENT_TOPOLOGY_ID       0x80030001
#define XONLINE_COMP_ATTR_EVENT_ROUND             0x80030002
#define XONLINE_COMP_ATTR_EVENT_START             0x80030003
#define XONLINE_COMP_ATTR_EVENT_NEXT_ENTITY       0x80030006
#define XONLINE_COMP_ATTR_EVENT_NEXT_START        0x80030007
#define XONLINE_COMP_ATTR_EVENT_P1                0x80030008
#define XONLINE_COMP_ATTR_EVENT_P1_GAMERTAG       0x81130008
#define XONLINE_COMP_ATTR_EVENT_P1_CHECKIN        0x80030009
#define XONLINE_COMP_ATTR_EVENT_P1_TRUST          0x8023000A
#define XONLINE_COMP_ATTR_EVENT_P2                0x8003000B
#define XONLINE_COMP_ATTR_EVENT_P2_GAMERTAG       0x8113000B
#define XONLINE_COMP_ATTR_EVENT_P2_CHECKIN        0x8003000C
#define XONLINE_COMP_ATTR_EVENT_P2_TRUST          0x8023000D
#define XONLINE_COMP_ATTR_EVENT_WINNER            0x8003000E
#define XONLINE_COMP_ATTR_EVENT_LOSER             0x8003000F
#define XONLINE_COMP_ATTR_EVENT_MIN               0x80030010
#define XONLINE_COMP_ATTR_EVENT_MAX               0x80030011


#define XONLINE_COMP_ATTR_BRACKET_ID              0x80040001
#define XONLINE_COMP_ATTR_BRACKET_SLOTS           0x80040002
#define XONLINE_COMP_ATTR_BRACKET_ENTRANTS        0x80040003
#define XONLINE_COMP_ATTR_BRACKET_BYES            0x80040004
#define XONLINE_COMP_ATTR_BRACKET_BYES_GRANTED    0x80040005
#define XONLINE_COMP_ATTR_BRACKET_ROUND_START     0x80040006
#define XONLINE_COMP_ATTR_BRACKET_APD             0x80040007


//
// Competition status codes
//
#define XONLINE_COMP_STATUS_PRE_INIT              0
#define XONLINE_COMP_STATUS_ACTIVE                1
#define XONLINE_COMP_STATUS_COMPLETE              2
#define XONLINE_COMP_STATUS_CANCELED              0xFFFFFFFF

//
// Entrant status codes
//
#define XONLINE_COMP_STATUS_ENTRANT_REGISTERED    0
#define XONLINE_COMP_STATUS_ENTRANT_PLAYING       1
#define XONLINE_COMP_STATUS_ENTRANT_FORFEIT       2
#define XONLINE_COMP_STATUS_ENTRANT_ELIMINATED    0xFFFFFFFF

//
// Action IDs
//
#define XONLINE_COMP_ACTION_JOIN                  1
#define XONLINE_COMP_ACTION_JOIN_PRIVATE          2
#define XONLINE_COMP_ACTION_WITHDRAW              3
#define XONLINE_COMP_ACTION_CHECKIN               4
#define XONLINE_COMP_ACTION_REQUEST_BYE           5
#define XONLINE_COMP_ACTION_REQUEST_PASS          6
#define XONLINE_COMP_ACTION_SUBMIT_RESULTS        7
#define XONLINE_COMP_ACTION_FORFEIT               8
#define XONLINE_COMP_ACTION_CANCEL                9
#define XONLINE_COMP_ACTION_EJECT                 10
#define XONLINE_COMP_ACTION_DEBUG_ADVANCE_TIME    99


//
// Limits
//
#define XONLINE_COMP_MAX_FREQUENCY              60
#define XONLINE_COMP_MIN_DURATION_MINS          5
#define XONLINE_COMP_MIN_JOB_DELAY              1
#define XONLINE_COMP_CHECKIN_WIN_SECS           5 * 60
#define XONLINE_COMP_MIN_CLEANUP_DAYS           1
#define XONLINE_COMP_MAX_CLEANUP_DAYS           365
#define XONLINE_COMP_DEFAULT_CLEANUP_DAYS       5        


//
// Defined types of competition intervals
//
typedef enum
{
    XONLINE_COMP_INTERVAL_MINUTE = 2,
    XONLINE_COMP_INTERVAL_HOUR = 3,
    XONLINE_COMP_INTERVAL_DAILY = 4,
    XONLINE_COMP_INTERVAL_WEEKLY = 5

} XONLINE_COMP_INTERVAL_UNIT;

// Day mask data type
typedef DWORD XONLINE_COMP_DAY_MASK;

//
// Day Mask Constants
//
#define XONLINE_COMP_DAY_MASK_ALL             0x007F
#define XONLINE_COMP_DAY_MASK_SUNDAY          0x0001
#define XONLINE_COMP_DAY_MASK_MONDAY          0x0002
#define XONLINE_COMP_DAY_MASK_TUESDAY         0x0004
#define XONLINE_COMP_DAY_MASK_WEDNESDAY       0x0008
#define XONLINE_COMP_DAY_MASK_THURSDAY        0x0010
#define XONLINE_COMP_DAY_MASK_FRIDAY          0x0020
#define XONLINE_COMP_DAY_MASK_SATURDAY        0x0040

typedef union
{
    DWORD                   dwUnitsOfTime;
    XONLINE_COMP_DAY_MASK   DayMask;

} XONLINE_COMP_UNITS_OR_MASK;
    
//
// User-friendly structure for Single Eliminations
//
typedef struct
{
    DWORD       dwPrivateSlots;
    DWORD       dwPublicSlots;
    DWORD       dwMinimumPlayers;

    FILETIME    ftRegistrationOpen;
    FILETIME    ftRegistrationClose;
    FILETIME    ftCompetitionStart;
    FILETIME    ftRoundOneStart;
    FILETIME    ftRoundOneEnd;

    DWORD       dwMatchReminderAdvanceMinutes;

    XONLINE_COMP_INTERVAL_UNIT      Interval;   
    XONLINE_COMP_UNITS_OR_MASK      UnitOrMask; // Mask when Interval = Day, otherwise Units of time of type Interval

    BOOL        fTeamCompetition;
    DWORD       dwTeamSize;
    
} XONLINE_COMP_SINGLE_ELIMINATION_ATTRIBUTES;

typedef XONLINE_COMP_SINGLE_ELIMINATION_ATTRIBUTES *PXONLINE_COMP_SINGLE_ELIMINATION_ATTRIBUTES;

//
// Structure returning results for a created competition
//
typedef struct
{
    ULONGLONG   qwCompetitionID;

} XONLINE_COMP_CREATE_RESULTS;

typedef XONLINE_COMP_CREATE_RESULTS *PXONLINE_COMP_CREATE_RESULTS;

//
// Topology results structure
//
typedef struct
{
    DWORD   dwBaseWidth;
    DWORD   dwRoundsReturned;
    DWORD   dwTotalResultEntries;
    DWORD   dwResultsSize;
    PBYTE   pbResults;
    DWORD   dwNumResultAttributeSpecs;
    PXONLINE_ATTRIBUTE_SPEC pResultAttributeSpecs;

} XONLINE_COMP_TOPOLOGY_SE_RESULTS;

typedef XONLINE_COMP_TOPOLOGY_SE_RESULTS *PXONLINE_COMP_TOPOLOGY_SE_RESULTS;

#define TOPOLOGY_ID(roundNumber, eventNumber) (((roundNumber) << 16) + (eventNumber))


XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionCreate(
    IN DWORD dwUserIndex,
    IN DWORD dwTemplate,
    IN ULONGLONG qwTeamID,
    IN DWORD dwNumCompetitionAttributes,
    IN const XONLINE_ATTRIBUTE *pCompetitionAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionCreateGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINE_COMP_CREATE_RESULTS pCompResults
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionCreateSingleElimination(
    IN DWORD dwUserIndex,
    IN DWORD dwTemplate,
    IN ULONGLONG qwTeamID,
    IN const XONLINE_COMP_SINGLE_ELIMINATION_ATTRIBUTES *pDefaultAttributes,
    IN DWORD dwNumAdditionalAttributes,
    IN const XONLINE_ATTRIBUTE *pAdditionalAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionSearch(
    IN DWORD dwQueryID,
    IN DWORD dwTarget,
    IN DWORD dwPage,
    IN DWORD dwResultsPerPage,
    IN DWORD dwNumSearchAttributes,
    IN const XONLINE_ATTRIBUTE *pSearchAttributes,
    IN DWORD dwNumResultAttributeSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pResultAttributeSpecs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionSearchGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTotalItemsInSearchResult,
    OUT DWORD *pdwItemsReturned,
    IN OUT DWORD *pdwResultBufferSize,
    IN OUT PBYTE pbResultBuffer            
    );

XBOXAPI
DWORD
WINAPI
XOnlineCompetitionGetResultsBufferSize(
    IN DWORD dwResultsPerPage,
    IN DWORD dwNumSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC* pSpecs
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionManageEntrant(
    IN DWORD dwAction,
    IN DWORD dwUserIndex,
    IN DWORD dwTemplate,
    IN ULONGLONG qwTeamID,
    IN ULONGLONG qwCompetitionID,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionCheckin(
    IN DWORD dwUserIndex,
    IN DWORD dwTemplate,
    IN ULONGLONG qwTeamID,
    IN ULONGLONG qwCompetitionID,
    IN ULONGLONG qwEventID,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionCancel(
    IN DWORD dwUserIndex,
    IN DWORD dwTemplate,
    IN ULONGLONG qwTeamID,
    IN ULONGLONG qwCompetitionID,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionSessionRegister(
    IN const XONLINE_ARB_ID *pArbitrationID,
    IN WORD wMaxRoundSeconds,
    IN DWORD dwArbitrationRegisterFlags,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionSessionRegisterGetResults(
    IN XONLINETASK_HANDLE hTask,
    IN DWORD dwRegistrantsBufferCount,
    OUT XONLINE_ARB_REGISTRANT *pRegistrants,
    OUT DWORD *pdwNumRegistrants
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionSubmitResults(
    IN DWORD dwTemplate,
    IN ULONGLONG qwCompetitionID,
    IN const XONLINE_ARB_ID *pArbitrationID,
    IN DWORD dwArbitrationReportFlags,
    IN const XONLINE_ARB_REPORT_DATA *pArbitrationReportData,
    IN DWORD dwNumStatProcs,
    IN const XONLINE_STAT_PROC *pStatProcs,
    IN DWORD dwNumAttributes,
    IN const XONLINE_ATTRIBUTE *pAttributes,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionTopology(
    IN DWORD dwTemplate,
    IN ULONGLONG qwCompetitionID,
    IN DWORD dwPage,
    IN DWORD dwResultsPerPage,
    IN DWORD dwStartingEventTopologyID,
    IN DWORD dwEndingEventTopologyID,
    IN DWORD dwNumResultAttributeSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pResultAttributeSpecs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionTopologyGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT DWORD *pdwTotalItemsInSearchResult,
    OUT DWORD *pdwItemsReturned,
    IN OUT DWORD *pdwResultBufferSize,
    IN OUT PBYTE pbResultBuffer            
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionTopologySingleElimination(
    IN DWORD dwTemplate,
    IN ULONGLONG qwCompetitionID,
    IN DWORD dwOriginEventTopologyID,
    IN DWORD dwRoundsForward,
    IN DWORD dwRoundsBackward,
    IN DWORD dwTopWidth,
    IN DWORD dwNumResultAttributeSpecs,
    IN const XONLINE_ATTRIBUTE_SPEC *pResultAttributeSpecs,
    IN OPTIONAL HANDLE hWorkEvent,
    OUT PXONLINETASK_HANDLE phTask
    );

XBOXAPI
HRESULT
WINAPI
XOnlineCompetitionTopologySingleEliminationGetResults(
    IN XONLINETASK_HANDLE hTask,
    OUT PXONLINE_COMP_TOPOLOGY_SE_RESULTS *ppTopologyResults
    );

//
// Live Server Platform (LSP) 
//

#define XONLINE_LSP_ATTR_TSADDR         0x80200001
#define XONLINE_LSP_ATTR_XNKID          0x80200002
#define XONLINE_LSP_ATTR_KEK            0x80200003
    
#define XONLINE_LSP_DEFAULT_DATASET_ID  0x0000AAAA





// ====================================================================
// Throttling Functions
//

//
// Throttle type tags
//


// XONLINE_STATISTICS_SERVICE
#define XONLINE_THROTTLE_TAG_XOSTATLEADERENUMERATE      "XOnlineStatLeaderEnumerate"
#define XONLINE_THROTTLE_TAG_XOSTATREAD                 "XOnlineStatRead"
#define XONLINE_THROTTLE_TAG_XOSTATRESET                "XOnlineStatReset"
#define XONLINE_THROTTLE_TAG_XOSTATUNITREAD             "XOnlineStatUnitRead"
#define XONLINE_THROTTLE_TAG_XOSTATUNITENUMERATE        "XOnlineStatUnitEnumerate"
#define XONLINE_THROTTLE_TAG_XOSTATWRITE                "XOnlineStatWrite"
#define XONLINE_THROTTLE_TAG_XOSTATWRITEEX              "XOnlineStatWriteEx"

// XONLINE_QUERY_SERVICE
#define XONLINE_THROTTLE_TAG_XOQUERYSEARCH              "XOnlineQuerySearch"

// XONLINE_ARBITRATION_SERVICE
#define XONLINE_THROTTLE_TAG_XOARBITRATIONEXTENDROUND   "XOnlineArbitrationExtendRound"
#define XONLINE_THROTTLE_TAG_XOARBITRATIONREGISTER      "XOnlineArbitrationRegister"
#define XONLINE_THROTTLE_TAG_XOARBITRATIONREPORT        "XOnlineArbitrationReport"

// XONLINE_MESSAGING_SERVICE
#define XONLINE_THROTTLE_TAG_XOMESSAGESEND              "XOnlineMessageSend"

// XONLINE_TEAM_SERVICE
#define XONLINE_THROTTLE_TAG_TEAM_ENUMERATION           "Team Enumeration"


//
// Throttle flags
//
#define XONLINE_THROTTLE_FLAG_DELAY             0x00000001 // delay starting each new operation until the throttle delay period expires
#define XONLINE_THROTTLE_FLAG_FAIL              0x00000002 // return XONLINE_E_TASK_THROTTLED for new tasks that are started during the throttle delay period
#define XONLINE_THROTTLE_FLAG_CANCEL_PREVIOUS   0x00000004 // force the previous task of the same type to fail with XONLINE_E_TASK_ABORTED_BY_DUPLICATE when possible
#define XONLINE_THROTTLE_FLAG_RIP               0x00000008 // RIP in debug
#define XONLINE_THROTTLE_FLAG_IGNORE_SERVER     0x00000010 // ignore any changes in the delay or flags that the server requests, debug only

#define XONLINE_VALID_THROTTLE_FLAGS            (XONLINE_THROTTLE_FLAG_DELAY | \
                                                 XONLINE_THROTTLE_FLAG_FAIL | \
                                                 XONLINE_THROTTLE_FLAG_CANCEL_PREVIOUS | \
                                                 XONLINE_THROTTLE_FLAG_RIP | \
                                                 XONLINE_THROTTLE_FLAG_IGNORE_SERVER)

XBOXAPI
HRESULT
WINAPI
XOnlineThrottleSet(
    IN DWORD dwServiceID,
    IN LPCSTR szThrottleTag,
    IN DWORD dwThrottleFlags,
    IN DWORD dwDelay
    );



XBOXAPI
HRESULT
WINAPI
XOnlineThrottleGet(
    IN DWORD dwServiceID,
    IN LPCSTR szThrottleTag,
    OUT DWORD *pdwThrottleFlags,
    OUT DWORD *pdwDelay
    );

    


#ifdef __cplusplus
}
#endif

#endif
