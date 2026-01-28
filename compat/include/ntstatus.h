#pragma once

typedef int NTSTATUS;

#define STATUS_SUCCESS                   ((NTSTATUS) 0x00000000)

#define STATUS_INVALID_PARAMETER         ((NTSTATUS) 0xC000000D)
#define STATUS_PROCEDURE_NOT_FOUND       ((NTSTATUS) 0xC000007A)
#define STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE                                 ((NTSTATUS) 0xC01E0342)