#pragma once

NTSYSAPI
BOOLEAN
KeAddSystemServiceTable(
    IN PULONG64 Base,
    IN PULONG Count OPTIONAL,
    IN ULONG Limit,
    IN PUCHAR Number,
    IN ULONG Index
    );


