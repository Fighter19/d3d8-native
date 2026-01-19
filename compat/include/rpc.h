#pragma once
typedef void* RPC_AUTH_IDENTITY_HANDLE;
typedef void* RPC_IF_HANDLE;
#define __RPC_STUB
typedef void* RPC_MESSAGE;
typedef RPC_MESSAGE* PRPC_MESSAGE;
// This is essentially a no-op because we don't use any special calling conventions
#define __RPC_USER __stdcall