/* ssh_constants.h */

#ifndef SSH_CONSTANTS_H_FILE
#define SSH_CONSTANTS_H_FILE

/* messages */
#define SSH_MSG_DISCONNECT                       1
#define SSH_MSG_IGNORE                           2
#define SSH_MSG_UNIMPLEMENTED                    3
#define SSH_MSG_DEBUG                            4
#define SSH_MSG_SERVICE_REQUEST                  5
#define SSH_MSG_SERVICE_ACCEPT                   6
#define SSH_MSG_KEXINIT                         20
#define SSH_MSG_NEWKEYS                         21
#define SSH_MSG_USERAUTH_REQUEST                50
#define SSH_MSG_USERAUTH_FAILURE                51
#define SSH_MSG_USERAUTH_SUCCESS                52
#define SSH_MSG_USERAUTH_BANNER                 53
#define SSH_MSG_GLOBAL_REQUEST                  80
#define SSH_MSG_REQUEST_SUCCESS                 81
#define SSH_MSG_REQUEST_FAILURE                 82
#define SSH_MSG_CHANNEL_OPEN                    90
#define SSH_MSG_CHANNEL_OPEN_CONFIRMATION       91
#define SSH_MSG_CHANNEL_OPEN_FAILURE            92
#define SSH_MSG_CHANNEL_WINDOW_ADJUST           93
#define SSH_MSG_CHANNEL_DATA                    94
#define SSH_MSG_CHANNEL_EXTENDED_DATA           95
#define SSH_MSG_CHANNEL_EOF                     96
#define SSH_MSG_CHANNEL_CLOSE                   97
#define SSH_MSG_CHANNEL_REQUEST                 98
#define SSH_MSG_CHANNEL_SUCCESS                 99
#define SSH_MSG_CHANNEL_FAILURE                100

/* DH key exchange */
#define SSH_MSG_KEXDH_INIT             30
#define SSH_MSG_KEXDH_REPLY            31

/* publickey USERAUTH */
#define SSH_MSG_USERAUTH_PK_OK         60

/* SSH_MSG_CHANNEL_EXTENDED_DATA data_type_code */
#define SSH_EXTENDED_DATA_STDERR       1

/* SSH_MSG_DISCONNECT reason codes */
#define SSH_DISCONNECT_HOST_NOT_ALLOWED_TO_CONNECT          1
#define SSH_DISCONNECT_PROTOCOL_ERROR                       2
#define SSH_DISCONNECT_KEY_EXCHANGE_FAILED                  3
#define SSH_DISCONNECT_RESERVED                             4
#define SSH_DISCONNECT_MAC_ERROR                            5
#define SSH_DISCONNECT_COMPRESSION_ERROR                    6
#define SSH_DISCONNECT_SERVICE_NOT_AVAILABLE                7
#define SSH_DISCONNECT_PROTOCOL_VERSION_NOT_SUPPORTED       8
#define SSH_DISCONNECT_HOST_KEY_NOT_VERIFIABLE              9
#define SSH_DISCONNECT_CONNECTION_LOST                     10
#define SSH_DISCONNECT_BY_APPLICATION                      11
#define SSH_DISCONNECT_TOO_MANY_CONNECTIONS                12
#define SSH_DISCONNECT_AUTH_CANCELLED_BY_USER              13
#define SSH_DISCONNECT_NO_MORE_AUTH_METHODS_AVAILABLE      14
#define SSH_DISCONNECT_ILLEGAL_USER_NAME                   15

/* SSH_MSG_CHANNEL_OPEN_FAILURE reason codes */
#define SSH_OPEN_ADMINISTRATIVELY_PROHIBITED                1
#define SSH_OPEN_CONNECT_FAILED                             2
#define SSH_OPEN_UNKNOWN_CHANNEL_TYPE                       3
#define SSH_OPEN_RESOURCE_SHORTAGE                          4

/* SSH_MSG_CHANNEL_EXTENDED_DATA data type codes */
#define SSH_EXTENDED_DATA_STDERR                   1


const char *ssh_const_get_msg_name(unsigned char msg_type);
const char *ssh_const_get_disconnect_reason(uint32_t reason_code);

#endif /* SSH_CONSTANTS_H_FILE */
