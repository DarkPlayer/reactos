/*
 * COPYRIGHT:         See COPYING in the top level directory
 * PROJECT:           ReactOS Runtime Library
 * PURPOSE:           Network Address Translation implementation
 * PROGRAMMER:        Alex Ionescu (alexi@tinykrnl.org)
 *                    Thomas Faber (thomas.faber@reactos.org)
 */

/* INCLUDES *****************************************************************/

#include <rtl.h>

#define NDEBUG
#include <debug.h>

/* maximum length of an ipv4 address expressed as a string */
#define IPV4_ADDR_STRING_MAX_LEN 16

/* maximum length of an ipv4 port expressed as a string */
#define IPV4_PORT_STRING_MAX_LEN 7 /* with the leading ':' */

/* network to host order conversion for little endian machines */
#define WN2H(w) (((w & 0xFF00) >> 8) | ((w & 0x00FF) << 8))

/* PRIVATE FUNCTIONS **********************************************************/

static
NTSTATUS
RtlpStringToUlong(
    _In_ PCWSTR String,
    _In_ BOOLEAN Strict,
    _Out_ PCWSTR *Terminator,
    _Out_ PULONG Out)
{
    /* If we never see any digits, we'll return this */
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    ULONG Result = 0;
    ULONG Base = 10;
    ULONG Digit;

    if (String[0] == L'0')
    {
        if (String[1] == L'x' || String[1] == L'X')
        {
            /* 0x/0X prefix -- hex */
            String += 2;
            Base = 16;
        }
        else if (String[1] >= L'0' && String[1] <= L'9')
        {
            /* 0 prefix -- octal */
            String++;
            Base = 8;
        }
    }

    /* Strict forbids anything but decimal */
    if (Strict && Base != 10)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Done;
    }

    while (1)
    {
        if (*String >= L'0' && *String <= L'7')
            Digit = *String - L'0';
        else if (*String >= L'0' && *String <= L'9' && Base >= 10)
            Digit = *String - L'0';
        else if (*String >= L'A' && *String <= L'F' && Base >= 16)
            Digit = 10 + (*String - L'A');
        else if (*String >= L'a' && *String <= L'f' && Base >= 16)
            Digit = 10 + (*String - L'a');
        else
            break;

        Status = RtlULongMult(Result, Base, &Result);
        if (!NT_SUCCESS(Status))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        Status = RtlULongAdd(Result, Digit, &Result);
        if (!NT_SUCCESS(Status))
        {
            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        String++;
    }

Done:
    *Terminator = String;
    *Out = Result;
    return Status;
}

/* PUBLIC FUNCTIONS ***********************************************************/

/*
 * @implemented
 */
LPSTR
NTAPI
RtlIpv4AddressToStringA(IN struct in_addr *Addr,
                        OUT PCHAR S)
{
    INT Length;

    if (!S) return (LPSTR)~0;

    Length = sprintf(S, "%u.%u.%u.%u", Addr->S_un.S_un_b.s_b1,
                                            Addr->S_un.S_un_b.s_b2,
                                            Addr->S_un.S_un_b.s_b3,
                                            Addr->S_un.S_un_b.s_b4);

    return S + Length;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlIpv4AddressToStringExA(IN struct in_addr *Address,
                          IN USHORT Port,
                          OUT PCHAR AddressString,
                          IN OUT PULONG AddressStringLength)
{
    CHAR Buffer[IPV4_ADDR_STRING_MAX_LEN+IPV4_PORT_STRING_MAX_LEN];
    ULONG Length;

    if (!Address || !AddressString || !AddressStringLength)
        return STATUS_INVALID_PARAMETER;

    Length = sprintf(Buffer, "%u.%u.%u.%u", Address->S_un.S_un_b.s_b1,
                                            Address->S_un.S_un_b.s_b2,
                                            Address->S_un.S_un_b.s_b3,
                                            Address->S_un.S_un_b.s_b4);

    if (Port) Length += sprintf(Buffer + Length, ":%u", WN2H(Port));

    if (*AddressStringLength > Length)
    {
        *AddressStringLength = Length + 1;
        strcpy(AddressString, Buffer);
        return STATUS_SUCCESS;
    }

    *AddressStringLength = Length + 1;
    return STATUS_INVALID_PARAMETER;
}

/*
 * @implemented
 */
LPWSTR
NTAPI
RtlIpv4AddressToStringW(IN struct in_addr *Addr,
                        OUT PWCHAR S)
{
    INT Length;

    if (!S) return (LPWSTR)~0;

    Length = swprintf(S, L"%u.%u.%u.%u", Addr->S_un.S_un_b.s_b1,
                                         Addr->S_un.S_un_b.s_b2,
                                         Addr->S_un.S_un_b.s_b3,
                                         Addr->S_un.S_un_b.s_b4);
    return S + Length;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlIpv4AddressToStringExW(IN struct in_addr *Address,
                          IN USHORT Port,
                          OUT PWCHAR AddressString,
                          IN OUT PULONG AddressStringLength)
{
    WCHAR Buffer[IPV4_ADDR_STRING_MAX_LEN+IPV4_PORT_STRING_MAX_LEN];
    ULONG Length;

    if (!Address || !AddressString || !AddressStringLength)
        return STATUS_INVALID_PARAMETER;

    Length = swprintf(Buffer, L"%u.%u.%u.%u", Address->S_un.S_un_b.s_b1,
                                              Address->S_un.S_un_b.s_b2,
                                              Address->S_un.S_un_b.s_b3,
                                              Address->S_un.S_un_b.s_b4);

    if (Port) Length += swprintf(Buffer + Length, L":%u", WN2H(Port));

    if (*AddressStringLength > Length)
    {
        *AddressStringLength = Length + 1;
        wcscpy(AddressString, Buffer);
        return STATUS_SUCCESS;
    }

    *AddressStringLength = Length + 1;
    return STATUS_INVALID_PARAMETER;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlIpv4StringToAddressA(
    _In_ PCSTR String,
    _In_ BOOLEAN Strict,
    _Out_ PCSTR *Terminator,
    _Out_ struct in_addr *Addr)
{
    NTSTATUS Status;
    ANSI_STRING AddressA;
    UNICODE_STRING AddressW;
    PCWSTR TerminatorW = NULL;

    Status = RtlInitAnsiStringEx(&AddressA, String);
    if (!NT_SUCCESS(Status))
        return Status;

    Status = RtlAnsiStringToUnicodeString(&AddressW, &AddressA, TRUE);
    if (!NT_SUCCESS(Status))
        return Status;

    Status = RtlIpv4StringToAddressW(AddressW.Buffer,
                                     Strict,
                                     &TerminatorW,
                                     Addr);

    ASSERT(TerminatorW >= AddressW.Buffer);
    *Terminator = String + (TerminatorW - AddressW.Buffer);

    RtlFreeUnicodeString(&AddressW);

    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlIpv4StringToAddressExA(
    _In_ PCSTR AddressString,
    _In_ BOOLEAN Strict,
    _Out_ struct in_addr *Address,
    _Out_ PUSHORT Port)
{
    NTSTATUS Status;
    ANSI_STRING AddressA;
    UNICODE_STRING AddressW;

    Status = RtlInitAnsiStringEx(&AddressA, AddressString);
    if (!NT_SUCCESS(Status))
        return Status;

    Status = RtlAnsiStringToUnicodeString(&AddressW, &AddressA, TRUE);
    if (!NT_SUCCESS(Status))
        return Status;

    Status = RtlIpv4StringToAddressExW(AddressW.Buffer, Strict, Address, Port);

    RtlFreeUnicodeString(&AddressW);

    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
RtlIpv4StringToAddressW(
    _In_ PCWSTR String,
    _In_ BOOLEAN Strict,
    _Out_ PCWSTR *Terminator,
    _Out_ struct in_addr *Addr)
{
    NTSTATUS Status;
    ULONG Values[4];
    ULONG Result;
    INT Parts = 0;
    INT i;

    do
    {
        Status = RtlpStringToUlong(String, Strict, &String, &Values[Parts]);
        Parts++;

        if (*String != L'.')
            break;

        /* Already four parts, but a dot follows? */
        if (Parts == 4)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Done;
        }

        /* Skip the dot */
        String++;
    } while (NT_SUCCESS(Status));

    if (Strict && Parts < 4)
        Status = STATUS_INVALID_PARAMETER;

    if (!NT_SUCCESS(Status))
        goto Done;

    /* Combine the parts */
    Result = Values[Parts - 1];
    for (i = 0; i < Parts - 1; i++)
    {
        INT Shift = CHAR_BIT * (3 - i);

        if (Values[i] > 0xFF || (Result & (0xFF << Shift)) != 0)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Done;
        }
        Result |= Values[i] << Shift;
    }

    Addr->S_un.S_addr = RtlUlongByteSwap(Result);

Done:
    *Terminator = String;
    return Status;
}

/*
* @implemented
*/
NTSTATUS
NTAPI
RtlIpv4StringToAddressExW(
    _In_ PCWSTR AddressString,
    _In_ BOOLEAN Strict,
    _Out_ struct in_addr *Address,
    _Out_ PUSHORT Port)
{
    PCWSTR CurrentChar;
    ULONG ConvertedPort;
    NTSTATUS Status;

    if (!AddressString || !Address || !Port)
        return STATUS_INVALID_PARAMETER;

    Status = RtlIpv4StringToAddressW(AddressString,
                                     Strict,
                                     &CurrentChar,
                                     Address);
    if (!NT_SUCCESS(Status))
        return Status;

    if (!*CurrentChar)
    {
        *Port = 0;
        return STATUS_SUCCESS;
    }

    if (*CurrentChar != L':')
        return STATUS_INVALID_PARAMETER;
    ++CurrentChar;

    Status = RtlpStringToUlong(CurrentChar,
                               FALSE,
                               &CurrentChar,
                               &ConvertedPort);
    if (!NT_SUCCESS(Status))
        return Status;

    if (*CurrentChar || !ConvertedPort || ConvertedPort > 0xffff)
        return STATUS_INVALID_PARAMETER;

    *Port = WN2H(ConvertedPort);
    return STATUS_SUCCESS;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6AddressToStringA(IN struct in6_addr *Addr,
                        OUT PCHAR S)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6AddressToStringExA(IN struct in6_addr *Address,
                          IN ULONG ScopeId,
                          IN ULONG Port,
                          OUT PCHAR AddressString,
                          IN OUT PULONG AddressStringLength)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6AddressToStringW(IN struct in6_addr *Addr,
                        OUT PWCHAR S)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6AddressToStringExW(IN struct in6_addr *Address,
                          IN ULONG ScopeId,
                          IN USHORT Port,
                          IN OUT PWCHAR AddressString,
                          IN OUT PULONG AddressStringLength)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6StringToAddressA(IN PCHAR Name,
                        OUT PCHAR *Terminator,
                        OUT struct in6_addr *Addr)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6StringToAddressExA(IN PCHAR AddressString,
                          OUT struct in6_addr *Address,
                          OUT PULONG ScopeId,
                          OUT PUSHORT Port)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6StringToAddressW(IN PWCHAR Name,
                        OUT PCHAR *Terminator,
                        OUT struct in6_addr *Addr)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/*
* @unimplemented
*/
NTSTATUS
NTAPI
RtlIpv6StringToAddressExW(IN PWCHAR AddressName,
                          OUT struct in6_addr *Address,
                          OUT PULONG ScopeId,
                          OUT PUSHORT Port)
{
    UNIMPLEMENTED;
    return STATUS_NOT_IMPLEMENTED;
}

/* EOF */
