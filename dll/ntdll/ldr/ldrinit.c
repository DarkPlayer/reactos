/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS NT User-Mode Library
 * FILE:            dll/ntdll/ldr/ldrinit.c
 * PURPOSE:         User-Mode Process/Thread Startup
 * PROGRAMMERS:     Alex Ionescu (alex@relsoft.net)
 *                  Aleksey Bragin (aleksey@reactos.org)
 */

/* INCLUDES *****************************************************************/

#include <ntdll.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS *******************************************************************/

HKEY ImageExecOptionsKey;
HKEY Wow64ExecOptionsKey;
UNICODE_STRING ImageExecOptionsString = RTL_CONSTANT_STRING(L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options");
UNICODE_STRING Wow64OptionsString = RTL_CONSTANT_STRING(L"");

BOOLEAN LdrpInLdrInit;
LONG LdrpProcessInitialized;

PLDR_DATA_TABLE_ENTRY LdrpImageEntry;
PUNICODE_STRING LdrpTopLevelDllBeingLoaded;
extern PTEB LdrpTopLevelDllBeingLoadedTeb; // defined in rtlsupp.c!
PLDR_DATA_TABLE_ENTRY LdrpCurrentDllInitializer;

//RTL_BITMAP TlsBitMap;
//RTL_BITMAP TlsExpansionBitMap;
//RTL_BITMAP FlsBitMap;
BOOLEAN LdrpImageHasTls;
LIST_ENTRY LdrpTlsList;
ULONG LdrpNumberOfTlsEntries;
ULONG LdrpNumberOfProcessors;

RTL_CRITICAL_SECTION LdrpLoaderLock;

BOOLEAN ShowSnaps;

ULONG LdrpFatalHardErrorCount;

VOID RtlpInitializeVectoredExceptionHandling(VOID);
VOID NTAPI RtlpInitDeferedCriticalSection(VOID);

/* FUNCTIONS *****************************************************************/

/*
 * @implemented
 */
NTSTATUS
NTAPI
LdrOpenImageFileOptionsKey(IN PUNICODE_STRING SubKey,
                           IN BOOLEAN Wow64,
                           OUT PHKEY NewKeyHandle)
{
    PHKEY RootKeyLocation;
    HANDLE RootKey;
    UNICODE_STRING SubKeyString;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    PWCHAR p1;

    /* Check which root key to open */
    if (Wow64)
        RootKeyLocation = &Wow64ExecOptionsKey;
    else
        RootKeyLocation = &ImageExecOptionsKey;

    /* Get the current key */
    RootKey = *RootKeyLocation;

    /* Setup the object attributes */
    InitializeObjectAttributes(&ObjectAttributes,
                               Wow64 ? 
                               &Wow64OptionsString : &ImageExecOptionsString,
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    /* Open the root key */
    Status = ZwOpenKey(&RootKey, KEY_ENUMERATE_SUB_KEYS, &ObjectAttributes);
    if (NT_SUCCESS(Status))
    {
        /* Write the key handle */
        if (_InterlockedCompareExchange((LONG*)RootKeyLocation, (LONG)RootKey, 0) != 0)
        {
            /* Someone already opened it, use it instead */
            NtClose(RootKey);
            RootKey = *RootKeyLocation;
        }

        /* Extract the name */
        SubKeyString = *SubKey;
        p1 = (PWCHAR)((ULONG_PTR)SubKeyString.Buffer + SubKeyString.Length);
        while (SubKey->Length)
        {
            if (p1[-1] == L'\\') break;
            p1--;
            SubKeyString.Length -= sizeof(*p1);
        }
        SubKeyString.Buffer = p1;
        SubKeyString.Length = SubKeyString.MaximumLength - SubKeyString.Length - sizeof(WCHAR);

        /* Setup the object attributes */
        InitializeObjectAttributes(&ObjectAttributes,
                                   &SubKeyString,
                                   OBJ_CASE_INSENSITIVE,
                                   RootKey,
                                   NULL);

        /* Open the setting key */
        Status = ZwOpenKey((PHANDLE)NewKeyHandle, GENERIC_READ, &ObjectAttributes);
    }

    /* Return to caller */
    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
LdrQueryImageFileKeyOption(IN HKEY KeyHandle,
                           IN PCWSTR ValueName,
                           IN ULONG Type,
                           OUT PVOID Buffer,
                           IN ULONG BufferSize,
                           OUT PULONG ReturnedLength OPTIONAL)
{
    ULONG KeyInfo[256];
    UNICODE_STRING ValueNameString, IntegerString;
    ULONG KeyInfoSize, ResultSize;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION)&KeyInfo;
    BOOLEAN FreeHeap = FALSE;
    NTSTATUS Status;

    /* Build a string for the value name */
    Status = RtlInitUnicodeStringEx(&ValueNameString, ValueName);
    if (!NT_SUCCESS(Status)) return Status;

    /* Query the value */
    Status = NtQueryValueKey(KeyHandle,
                             &ValueNameString,
                             KeyValuePartialInformation,
                             KeyValueInformation,
                             sizeof(KeyInfo),
                             &ResultSize);
    if (Status == STATUS_BUFFER_OVERFLOW)
    {
        /* Our local buffer wasn't enough, allocate one */
        KeyInfoSize = sizeof(KEY_VALUE_PARTIAL_INFORMATION) +
                      KeyValueInformation->DataLength;
        KeyValueInformation = RtlAllocateHeap(RtlGetProcessHeap(),
                                              0,
                                              KeyInfoSize);
        if (KeyInfo == NULL)
        {
            /* Give up this time */
            Status = STATUS_NO_MEMORY;
        }

        /* Try again */
        Status = NtQueryValueKey(KeyHandle,
                                 &ValueNameString,
                                 KeyValuePartialInformation,
                                 KeyValueInformation,
                                 KeyInfoSize,
                                 &ResultSize);
        FreeHeap = TRUE;
    }

    /* Check for success */
    if (NT_SUCCESS(Status))
    {
        /* Handle binary data */
        if (KeyValueInformation->Type == REG_BINARY)
        {
            /* Check validity */
            if ((Buffer) && (KeyValueInformation->DataLength <= BufferSize))
            {
                /* Copy into buffer */
                RtlMoveMemory(Buffer,
                              &KeyValueInformation->Data,
                              KeyValueInformation->DataLength);
            }
            else
            {
                Status = STATUS_BUFFER_OVERFLOW;
            }

            /* Copy the result length */
            if (ReturnedLength) *ReturnedLength = KeyValueInformation->DataLength;
        }
        else if (KeyValueInformation->Type == REG_DWORD)
        {
            /* Check for valid type */
            if (KeyValueInformation->Type != Type)
            {
                /* Error */
                Status = STATUS_OBJECT_TYPE_MISMATCH;
            }
            else
            {
                /* Check validity */
                if ((Buffer) &&
                    (BufferSize == sizeof(ULONG)) &&
                    (KeyValueInformation->DataLength <= BufferSize))
                {
                    /* Copy into buffer */
                    RtlMoveMemory(Buffer,
                                  &KeyValueInformation->Data,
                                  KeyValueInformation->DataLength);
                }
                else
                {
                    Status = STATUS_BUFFER_OVERFLOW;
                }

                /* Copy the result length */
                if (ReturnedLength) *ReturnedLength = KeyValueInformation->DataLength;
            }
        }
        else if (KeyValueInformation->Type != REG_SZ)
        {
            /* We got something weird */
            Status = STATUS_OBJECT_TYPE_MISMATCH;
        }
        else
        {
            /*  String, check what you requested */
            if (Type == REG_DWORD)
            {
                /* Validate */
                if (BufferSize != sizeof(ULONG))
                {
                    /* Invalid size */
                    BufferSize = 0;
                    Status = STATUS_INFO_LENGTH_MISMATCH;
                }
                else
                {
                    /* OK, we know what you want... */
                    IntegerString.Buffer = (PWSTR)KeyValueInformation->Data;
                    IntegerString.Length = KeyValueInformation->DataLength -
                                           sizeof(WCHAR);
                    IntegerString.MaximumLength = KeyValueInformation->DataLength;
                    Status = RtlUnicodeStringToInteger(&IntegerString, 0, (PULONG)Buffer);
                }
            }
            else
            {
                /* Validate */
                if (KeyValueInformation->DataLength > BufferSize)
                {
                    /* Invalid */
                    Status = STATUS_BUFFER_OVERFLOW;
                }
                else
                {
                    /* Set the size */
                    BufferSize = KeyValueInformation->DataLength;
                }

                /* Copy the string */
                RtlMoveMemory(Buffer, &KeyValueInformation->Data, BufferSize);
            }

            /* Copy the result length */
            if (ReturnedLength) *ReturnedLength = KeyValueInformation->DataLength;
        }
    }

    /* Check if buffer was in heap */
    if (FreeHeap) RtlFreeHeap(RtlGetProcessHeap(), 0, KeyValueInformation);

    /* Close key and return */
    NtClose(KeyHandle);
    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
LdrQueryImageFileExecutionOptionsEx(IN PUNICODE_STRING SubKey,
                                    IN PCWSTR ValueName,
                                    IN ULONG Type,
                                    OUT PVOID Buffer,
                                    IN ULONG BufferSize,
                                    OUT PULONG ReturnedLength OPTIONAL,
                                    IN BOOLEAN Wow64)
{
    NTSTATUS Status;
    HKEY KeyHandle;

    /* Open a handle to the key */
    Status = LdrOpenImageFileOptionsKey(SubKey, Wow64, &KeyHandle);

    /* Check for success */
    if (NT_SUCCESS(Status))
    {
        /* Query the data */
        Status = LdrQueryImageFileKeyOption(KeyHandle,
                                            ValueName,
                                            Type,
                                            Buffer,
                                            BufferSize,
                                            ReturnedLength);

        /* Close the key */
        NtClose(KeyHandle);
    }

    /* Return to caller */
    return Status;
}

/*
 * @implemented
 */
NTSTATUS
NTAPI
LdrQueryImageFileExecutionOptions(IN PUNICODE_STRING SubKey,
                                  IN PCWSTR ValueName,
                                  IN ULONG Type,
                                  OUT PVOID Buffer,
                                  IN ULONG BufferSize,
                                  OUT PULONG ReturnedLength OPTIONAL)
{
    /* Call the newer function */
    return LdrQueryImageFileExecutionOptionsEx(SubKey,
                                               ValueName,
                                               Type,
                                               Buffer,
                                               BufferSize,
                                               ReturnedLength,
                                               FALSE);
}

VOID
NTAPI
LdrpEnsureLoaderLockIsHeld()
{
    // Ignored atm
}

PVOID
NTAPI
LdrpFetchAddressOfSecurityCookie(PVOID BaseAddress, ULONG SizeOfImage)
{
    PIMAGE_LOAD_CONFIG_DIRECTORY ConfigDir;
    ULONG DirSize;
    PVOID Cookie = NULL;

    /* Check NT header first */
    if (!RtlImageNtHeader(BaseAddress)) return NULL;

    /* Get the pointer to the config directory */
    ConfigDir = RtlImageDirectoryEntryToData(BaseAddress,
                                             TRUE,
                                             IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                             &DirSize);

    /* Check for sanity */
    if (!ConfigDir ||
        (DirSize != 64 && ConfigDir->Size != DirSize) ||
        (ConfigDir->Size < 0x48))
        return NULL;

    /* Now get the cookie */
    Cookie = (PVOID)ConfigDir->SecurityCookie;

    /* Check this cookie */
    if (Cookie == NULL ||
        (PCHAR)Cookie <= (PCHAR)BaseAddress ||
        (PCHAR)Cookie >= (PCHAR)BaseAddress + SizeOfImage)
    {
        Cookie = NULL;
    }

    /* Return validated security cookie */
    return Cookie;
}

PVOID
NTAPI
LdrpInitSecurityCookie(PLDR_DATA_TABLE_ENTRY LdrEntry)
{
    PVOID Cookie;

    /* Fetch address of the cookie */
    Cookie = LdrpFetchAddressOfSecurityCookie(LdrEntry->DllBase, LdrEntry->SizeOfImage);

    if (Cookie)
    {
        UNIMPLEMENTED;
        Cookie = NULL;
    }

    return Cookie;
}

NTSTATUS
NTAPI
LdrpRunInitializeRoutines(IN PCONTEXT Context OPTIONAL)
{
    PLDR_DATA_TABLE_ENTRY LocalArray[16];
    PLIST_ENTRY ListHead;
    PLIST_ENTRY NextEntry;
    PLDR_DATA_TABLE_ENTRY LdrEntry, *LdrRootEntry, OldInitializer;
    PVOID EntryPoint;
    ULONG Count, i;
    //ULONG BreakOnInit;
    NTSTATUS Status = STATUS_SUCCESS;
    PPEB Peb = NtCurrentPeb();
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_EXTENDED ActCtx;
    ULONG BreakOnDllLoad;
    PTEB OldTldTeb;
    BOOLEAN DllStatus;

    DPRINT("LdrpRunInitializeRoutines() called for %wZ\n", &LdrpImageEntry->BaseDllName);

    /* Check the Loader Lock */
    LdrpEnsureLoaderLockIsHeld();

     /* Get the number of entries to call */
    if ((Count = LdrpClearLoadInProgress()))
    {
        /* Check if we can use our local buffer */
        if (Count > 16)
        {
            /* Allocate space for all the entries */
            LdrRootEntry = RtlAllocateHeap(RtlGetProcessHeap(),
                                           0,
                                           Count * sizeof(LdrRootEntry));
            if (!LdrRootEntry) return STATUS_NO_MEMORY;
        }
        else
        {
            /* Use our local array */
            LdrRootEntry = LocalArray;
        }
    }
    else
    {
        /* Don't need one */
        LdrRootEntry = NULL;
    }

    /* Show debug message */
    if (ShowSnaps)
    {
        DPRINT1("[%x,%x] LDR: Real INIT LIST for Process %wZ pid %u %0x%x\n",
                NtCurrentTeb()->RealClientId.UniqueThread,
                NtCurrentTeb()->RealClientId.UniqueProcess,
                Peb->ProcessParameters->ImagePathName,
                NtCurrentTeb()->RealClientId.UniqueThread,
                NtCurrentTeb()->RealClientId.UniqueProcess);
    }

    /* Loop in order */
    ListHead = &Peb->Ldr->InInitializationOrderModuleList;
    NextEntry = ListHead->Flink;
    i = 0;
    while (NextEntry != ListHead)
    {
        /* Get the Data Entry */
        LdrEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InInitializationOrderModuleList);

        /* Check if we have a Root Entry */
        if (LdrRootEntry)
        {
            /* Check flags */
            if (!(LdrEntry->Flags & LDRP_ENTRY_PROCESSED))
            {
                /* Setup the Cookie for the DLL */
                LdrpInitSecurityCookie(LdrEntry);

                /* Check for valid entrypoint */
                if (LdrEntry->EntryPoint)
                {
                    /* Write in array */
                    LdrRootEntry[i] = LdrEntry;

                    /* Display debug message */
                    if (ShowSnaps)
                    {
                        DPRINT1("[%x,%x] LDR: %wZ init routine %p\n",
                                NtCurrentTeb()->RealClientId.UniqueThread,
                                NtCurrentTeb()->RealClientId.UniqueProcess,
                                &LdrEntry->FullDllName,
                                LdrEntry->EntryPoint);
                    }
                    i++;
                }
            }
        }

        /* Set the flag */
        LdrEntry->Flags |= LDRP_ENTRY_PROCESSED;
        NextEntry = NextEntry->Flink;
    }

    /* If we got a context, then we have to call Kernel32 for TS support */
    if (Context)
    {
        /* Check if we have one */
        //if (Kernel32ProcessInitPostImportfunction)
        //{
            /* Call it */
            //Kernel32ProcessInitPostImportfunction();
        //}

        /* Clear it */
        //Kernel32ProcessInitPostImportfunction = NULL;
        UNIMPLEMENTED;
    }

    /* No root entry? return */
    if (!LdrRootEntry) return STATUS_SUCCESS;

    /* Set the TLD TEB */
    OldTldTeb = LdrpTopLevelDllBeingLoadedTeb;
    LdrpTopLevelDllBeingLoadedTeb = NtCurrentTeb();

    /* Loop */
    i = 0;
    while (i < Count)
    {
        /* Get an entry */
        LdrEntry = LdrRootEntry[i];

        /* FIXME: Verify NX Compat */

        /* Move to next entry */
        i++;

        /* Get its entrypoint */
        EntryPoint = LdrEntry->EntryPoint;

        /* Are we being debugged? */
        BreakOnDllLoad = 0;
        if (Peb->BeingDebugged || Peb->ReadImageFileExecOptions)
        {
            /* Check if we should break on load */
            Status = LdrQueryImageFileExecutionOptions(&LdrEntry->BaseDllName,
                                                       L"BreakOnDllLoad",
                                                       REG_DWORD,
                                                       &BreakOnDllLoad,
                                                       sizeof(ULONG),
                                                       NULL);
            if (!NT_SUCCESS(Status)) BreakOnDllLoad = 0;
        }

        /* Break if aksed */
        if (BreakOnDllLoad)
        {
            /* Check if we should show a message */
            if (ShowSnaps)
            {
                DPRINT1("LDR: %wZ loaded.", &LdrEntry->BaseDllName);
                DPRINT1(" - About to call init routine at %p\n", EntryPoint);
            }

            /* Break in debugger */
            DbgBreakPoint();
        }

        /* Make sure we have an entrypoint */
        if (EntryPoint)
        {
            /* Save the old Dll Initializer and write the current one */
            OldInitializer = LdrpCurrentDllInitializer;
            LdrpCurrentDllInitializer = LdrEntry;

            /* Set up the Act Ctx */
            ActCtx.Size = sizeof(ActCtx);
            ActCtx.Frame.Flags = ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID;
            RtlZeroMemory(&ActCtx, sizeof(ActCtx));

            /* Activate the ActCtx */
            RtlActivateActivationContextUnsafeFast(&ActCtx,
                                                   LdrEntry->EntryPointActivationContext);

            /* Check if it has TLS */
            if (LdrEntry->TlsIndex && Context)
            {
                /* Call TLS */
                LdrpTlsCallback(LdrEntry->DllBase, DLL_PROCESS_ATTACH);
            }

            /* Call the Entrypoint */
            if (ShowSnaps)
            {
                DPRINT1("%wZ - Calling entry point at %p for DLL_PROCESS_ATTACH\n",
                        &LdrEntry->BaseDllName, EntryPoint);
            }
            DllStatus = LdrpCallDllEntry(EntryPoint,
                                         LdrEntry->DllBase,
                                         DLL_PROCESS_ATTACH,
                                         Context);

            /* Deactivate the ActCtx */
            RtlDeactivateActivationContextUnsafeFast(&ActCtx);

            /* Save the Current DLL Initializer */
            LdrpCurrentDllInitializer = OldInitializer;

            /* Mark the entry as processed */
            LdrEntry->Flags |= LDRP_PROCESS_ATTACH_CALLED;

            /* Fail if DLL init failed */
            if (!DllStatus)
            {
                DPRINT1("LDR: DLL_PROCESS_ATTACH for dll \"%wZ\" (InitRoutine: %p) failed\n",
                    &LdrEntry->BaseDllName, EntryPoint);

                Status = STATUS_DLL_INIT_FAILED;
                goto Quickie;
            }
        }
    }

    /* Loop in order */
    ListHead = &Peb->Ldr->InInitializationOrderModuleList;
    NextEntry = NextEntry->Flink;
    while (NextEntry != ListHead)
    {
        /* Get the Data Entrry */
        LdrEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InInitializationOrderModuleList);

        /* FIXME: Verify NX Compat */
        // LdrpCheckNXCompatibility()

        /* Next entry */
        NextEntry = NextEntry->Flink;
    }

    /* Check for TLS */
    if (LdrpImageHasTls && Context)
    {
        /* Set up the Act Ctx */
        ActCtx.Size = sizeof(ActCtx);
        ActCtx.Frame.Flags = ACTCTX_FLAG_PROCESSOR_ARCHITECTURE_VALID;
        RtlZeroMemory(&ActCtx, sizeof(ActCtx));

        /* Activate the ActCtx */
        RtlActivateActivationContextUnsafeFast(&ActCtx,
                                               LdrpImageEntry->EntryPointActivationContext);

        /* Do TLS callbacks */
        LdrpTlsCallback(Peb->ImageBaseAddress, DLL_PROCESS_ATTACH);

        /* Deactivate the ActCtx */
        RtlDeactivateActivationContextUnsafeFast(&ActCtx);
    }

Quickie:
    /* Restore old TEB */
    LdrpTopLevelDllBeingLoadedTeb = OldTldTeb;

    /* Check if the array is in the heap */
    if (LdrRootEntry != LocalArray)
    {
        /* Free the array */
        RtlFreeHeap(RtlGetProcessHeap(), 0, LdrRootEntry);
    }

    /* Return to caller */
    DPRINT("LdrpRunInitializeRoutines() done\n");
    return Status;
}

NTSTATUS
NTAPI
LdrpInitializeTls(VOID)
{
    PLIST_ENTRY NextEntry, ListHead;
    PLDR_DATA_TABLE_ENTRY LdrEntry;
    PIMAGE_TLS_DIRECTORY TlsDirectory;
    PLDRP_TLS_DATA TlsData;
    ULONG Size;

    /* Initialize the TLS List */
    InitializeListHead(&LdrpTlsList);

    /* Loop all the modules */
    ListHead = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    NextEntry = ListHead->Flink;
    while (ListHead != NextEntry)
    {
        /* Get the entry */
        LdrEntry = CONTAINING_RECORD(NextEntry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        NextEntry = NextEntry->Flink;

        /* Get the TLS directory */
        TlsDirectory = RtlImageDirectoryEntryToData(LdrEntry->DllBase,
                                                    TRUE,
                                                    IMAGE_DIRECTORY_ENTRY_TLS,
                                                    &Size);

        /* Check if we have a directory */
        if (!TlsDirectory) continue;

        /* Check if the image has TLS */
        if (!LdrpImageHasTls) LdrpImageHasTls = TRUE;

        /* Show debug message */
        if (ShowSnaps)
        {
            DPRINT1("LDR: Tls Found in %wZ at %p\n",
                    &LdrEntry->BaseDllName,
                    TlsDirectory);
        }

        /* Allocate an entry */
        TlsData = RtlAllocateHeap(RtlGetProcessHeap(), 0, sizeof(LDRP_TLS_DATA));
        if (!TlsData) return STATUS_NO_MEMORY;

        /* Lock the DLL and mark it for TLS Usage */
        LdrEntry->LoadCount = -1;
        LdrEntry->TlsIndex = -1;

        /* Save the cached TLS data */
        TlsData->TlsDirectory = *TlsDirectory;
        InsertTailList(&LdrpTlsList, &TlsData->TlsLinks);

        /* Update the index */
        *(PLONG)TlsData->TlsDirectory.AddressOfIndex = LdrpNumberOfTlsEntries;
        TlsData->TlsDirectory.Characteristics = LdrpNumberOfTlsEntries++;
    }

    /* Done setting up TLS, allocate entries */
    return LdrpAllocateTls();
}

NTSTATUS
NTAPI
LdrpAllocateTls(VOID)
{
    PTEB Teb = NtCurrentTeb();
    PLIST_ENTRY NextEntry, ListHead;
    PLDRP_TLS_DATA TlsData;
    ULONG TlsDataSize;
    PVOID *TlsVector;

    /* Check if we have any entries */
    if (LdrpNumberOfTlsEntries)
        return 0;

    /* Allocate the vector array */
    TlsVector = RtlAllocateHeap(RtlGetProcessHeap(),
                                    0,
                                    LdrpNumberOfTlsEntries * sizeof(PVOID));
    if (!TlsVector) return STATUS_NO_MEMORY;
    Teb->ThreadLocalStoragePointer = TlsVector;

    /* Loop the TLS Array */
    ListHead = &LdrpTlsList;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead)
    {
        /* Get the entry */
        TlsData = CONTAINING_RECORD(NextEntry, LDRP_TLS_DATA, TlsLinks);
        NextEntry = NextEntry->Flink;

        /* Allocate this vector */
        TlsDataSize = TlsData->TlsDirectory.EndAddressOfRawData - 
                      TlsData->TlsDirectory.StartAddressOfRawData;
        TlsVector[TlsData->TlsDirectory.Characteristics] = RtlAllocateHeap(RtlGetProcessHeap(),
                                                                           0,
                                                                           TlsDataSize);
        if (!TlsVector[TlsData->TlsDirectory.Characteristics])
        {
            /* Out of memory */
            return STATUS_NO_MEMORY;
        }

        /* Show debug message */
        if (ShowSnaps)
        {
            DPRINT1("LDR: TlsVector %x Index %d = %x copied from %x to %x\n",
                    TlsVector,
                    TlsData->TlsDirectory.Characteristics,
                    &TlsVector[TlsData->TlsDirectory.Characteristics],
                    TlsData->TlsDirectory.StartAddressOfRawData,
                    TlsVector[TlsData->TlsDirectory.Characteristics]);
        }

        /* Copy the data */
        RtlCopyMemory(TlsVector[TlsData->TlsDirectory.Characteristics],
                      (PVOID)TlsData->TlsDirectory.StartAddressOfRawData,
                      TlsDataSize);
    }

    /* Done */
    return STATUS_SUCCESS;
}

VOID
NTAPI
LdrpFreeTls(VOID)
{
    PLIST_ENTRY ListHead, NextEntry;
    PLDRP_TLS_DATA TlsData;
    PVOID *TlsVector;
    PTEB Teb = NtCurrentTeb();

    /* Get a pointer to the vector array */
    TlsVector = Teb->ThreadLocalStoragePointer;
    if (!TlsVector) return;

    /* Loop through it */
    ListHead = &LdrpTlsList;
    NextEntry = ListHead->Flink;
    while (NextEntry != ListHead)
    {
        TlsData = CONTAINING_RECORD(NextEntry, LDRP_TLS_DATA, TlsLinks);
        NextEntry = NextEntry->Flink;

        /* Free each entry */
        if (TlsVector[TlsData->TlsDirectory.Characteristics])
        {
            RtlFreeHeap(RtlGetProcessHeap(),
                        0,
                        TlsVector[TlsData->TlsDirectory.Characteristics]);
        }
    }

    /* Free the array itself */
    RtlFreeHeap(RtlGetProcessHeap(),
                0,
                TlsVector);
}

VOID
NTAPI
LdrpInitFailure(NTSTATUS Status)
{
    ULONG Response;

    /* Print a debug message */
    DPRINT1("LDR: Process initialization failure; NTSTATUS = %08lx\n", Status);

    /* Raise a hard error */
    if (!LdrpFatalHardErrorCount)
    {
        ZwRaiseHardError(STATUS_APP_INIT_FAILURE, 1, 0, (PULONG_PTR)&Status, OptionOk, &Response);
    }
}

VOID
NTAPI
LdrpInit(PCONTEXT Context,
         PVOID SystemArgument1,
         PVOID SystemArgument2)
{
    LARGE_INTEGER Timeout;
    PTEB Teb = NtCurrentTeb();
    NTSTATUS Status, LoaderStatus = STATUS_SUCCESS;
    MEMORY_BASIC_INFORMATION MemoryBasicInfo;
    PPEB Peb = NtCurrentPeb();

    DPRINT("LdrpInit()\n");

    /* Check if we have a deallocation stack */
    if (!Teb->DeallocationStack)
    {
        /* We don't, set one */
        Status = NtQueryVirtualMemory(NtCurrentProcess(),
                                      Teb->NtTib.StackLimit,
                                      MemoryBasicInformation,
                                      &MemoryBasicInfo,
                                      sizeof(MEMORY_BASIC_INFORMATION),
                                      NULL);
        if (!NT_SUCCESS(Status))
        {
            /* Fail */
            LdrpInitFailure(Status);
            RtlRaiseStatus(Status);
            return;
        }

        /* Set the stack */
        Teb->DeallocationStack = MemoryBasicInfo.AllocationBase;
    }

    /* Now check if the process is already being initialized */
    while (_InterlockedCompareExchange(&LdrpProcessInitialized,
                                      1,
                                      0) == 1)
    {
        /* Set the timeout to 30 seconds */
        Timeout.QuadPart = Int32x32To64(30, -10000);

        /* Make sure the status hasn't changed */
        while (!LdrpProcessInitialized)
        {
            /* Do the wait */
            ZwDelayExecution(FALSE, &Timeout);
        }
    }

    /* Check if we have already setup LDR data */
    if (!Peb->Ldr)
    {
        /* Setup the Loader Lock */
        Peb->LoaderLock = &LdrpLoaderLock;

        /* Let other code know we're initializing */
        LdrpInLdrInit = TRUE;

        /* Initialize Critical Section Data */
        RtlpInitDeferedCriticalSection();

        /* Initialize VEH Call lists */
        RtlpInitializeVectoredExceptionHandling();

        /* Protect with SEH */
        _SEH2_TRY
        {
            /* Initialize the Process */
            LoaderStatus = LdrpInitializeProcess(Context,
                                                 SystemArgument1);

            /* Check for success and if MinimumStackCommit was requested */
            if (NT_SUCCESS(LoaderStatus) && Peb->MinimumStackCommit)
            {
                /* Enforce the limit */
                //LdrpTouchThreadStack(Peb->MinimumStackCommit);
                UNIMPLEMENTED;
            }
        }
        _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
        {
            /* Fail with the SEH error */
            LoaderStatus = _SEH2_GetExceptionCode();
        }
        _SEH2_END;

        /* We're not initializing anymore */
        LdrpInLdrInit = FALSE;

        /* Check if init worked */
        if (NT_SUCCESS(LoaderStatus))
        {
            /* Set the process as Initialized */
            _InterlockedIncrement(&LdrpProcessInitialized);
        }
    }
    else
    {
        /* Loader data is there... is this a fork() ? */
        if(Peb->InheritedAddressSpace)
        {
            /* Handle the fork() */
            //LoaderStatus = LdrpForkProcess();
            LoaderStatus = STATUS_NOT_IMPLEMENTED;
            UNIMPLEMENTED;
        }
        else
        {
            /* This is a new thread initializing */
            LdrpInitializeThread(Context);
        }
    }

    /* All done, test alert the thread */
    NtTestAlert();

    /* Return */
    if (!NT_SUCCESS(LoaderStatus))
    {
        /* Fail */
        LdrpInitFailure(LoaderStatus);
        RtlRaiseStatus(LoaderStatus);
    }
}

/* EOF */
