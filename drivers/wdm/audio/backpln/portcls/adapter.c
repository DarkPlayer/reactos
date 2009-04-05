/*
 * COPYRIGHT:       See COPYING in the top level directory
 * PROJECT:         ReactOS Kernel Streaming
 * FILE:            drivers/wdm/audio/backpln/portcls/api.c
 * PURPOSE:         Port Class driver / DriverEntry and IRP handlers
 * PROGRAMMER:      Andrew Greenwood
 *
 * HISTORY:
 *                  27 Jan 07   Created
 */

#include "private.h"

/*
    This is called from DriverEntry so that PortCls can take care of some
    IRPs and map some others to the main KS driver. In most cases this will
    be the first function called by an audio driver.

    First 2 parameters are from DriverEntry.

    The AddDevice parameter is a driver-supplied pointer to a function which
    typically then calls PcAddAdapterDevice (see below.)
*/
NTSTATUS NTAPI
PcInitializeAdapterDriver(
    IN  PDRIVER_OBJECT DriverObject,
    IN  PUNICODE_STRING RegistryPathName,
    IN  PDRIVER_ADD_DEVICE AddDevice)
{
    /*
        (* = implement here, otherwise KS default)
        IRP_MJ_CLOSE
        * IRP_MJ_CREATE
        IRP_MJ_DEVICE_CONTROL
        IRP_MJ_FLUSH_BUFFERS
        * IRP_MJ_PNP
        * IRP_MJ_POWER
        IRP_MJ_QUERY_SECURITY
        IRP_MJ_READ
        IRP_MJ_SET_SECURITY
        * IRP_MJ_SYSTEM_CONTROL
        IRP_MJ_WRITE
    */

    //NTSTATUS status;
    //ULONG i;

    DPRINT1("PcInitializeAdapterDriver\n");

    /* Our IRP handlers */
    DPRINT1("Setting IRP handlers\n");
    DriverObject->MajorFunction[IRP_MJ_CREATE] = PcDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_PNP] = PcDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_POWER] = PcDispatchIrp;
    DriverObject->MajorFunction[IRP_MJ_SYSTEM_CONTROL] = PcDispatchIrp;

    /* The driver-supplied AddDevice */
    DriverObject->DriverExtension->AddDevice = AddDevice;

    /* KS handles these */
    DPRINT("Setting KS function handlers\n");
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_CLOSE);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_DEVICE_CONTROL);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_FLUSH_BUFFERS);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_QUERY_SECURITY);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_READ);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_SET_SECURITY);
    KsSetMajorFunctionHandler(DriverObject, IRP_MJ_WRITE);

    DPRINT("PortCls has finished initializing the adapter driver\n");

    return STATUS_SUCCESS;
}

/*
    Typically called by a driver's AddDevice function, which is set when
    calling PcInitializeAdapterDriver. This performs some common driver
    operations, such as creating a device extension.

    The StartDevice parameter is a driver-supplied function which gets
    called in response to IRP_MJ_PNP / IRP_MN_START_DEVICE.
*/
NTSTATUS NTAPI
PcAddAdapterDevice(
    IN  PDRIVER_OBJECT DriverObject,
    IN  PDEVICE_OBJECT PhysicalDeviceObject,
    IN  PCPFNSTARTDEVICE StartDevice,
    IN  ULONG MaxObjects,
    IN  ULONG DeviceExtensionSize)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    PDEVICE_OBJECT fdo = NULL;
    PDEVICE_OBJECT PrevDeviceObject;
    PPCLASS_DEVICE_EXTENSION portcls_ext;

    DPRINT1("PcAddAdapterDevice called\n");

    if (!DriverObject || !PhysicalDeviceObject || !StartDevice)
    {
        return STATUS_INVALID_PARAMETER;
    }

    /* check if the DeviceExtensionSize is provided */
    if ( DeviceExtensionSize < PORT_CLASS_DEVICE_EXTENSION_SIZE )
    {
        /* driver does not need a device extension */
        if ( DeviceExtensionSize != 0 )
        {
            /* DeviceExtensionSize must be zero*/
            return STATUS_INVALID_PARAMETER;
        }
        /* set size to our extension size */
        DeviceExtensionSize = PORT_CLASS_DEVICE_EXTENSION_SIZE;
    }

    /* create the device */
    status = IoCreateDevice(DriverObject,
                            DeviceExtensionSize,
                            NULL,
                            FILE_DEVICE_KS,
                            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &fdo);

    if (!NT_SUCCESS(status))
    {
        DPRINT("IoCreateDevice() failed with status 0x%08lx\n", status);
        return status;
    }

    /* Obtain the new device extension */
    portcls_ext = (PPCLASS_DEVICE_EXTENSION) fdo->DeviceExtension;
    /* initialize the device extension */
    RtlZeroMemory(portcls_ext, DeviceExtensionSize);
    /* allocate create item */
    portcls_ext->CreateItems = AllocateItem(NonPagedPool, MaxObjects * sizeof(KSOBJECT_CREATE_ITEM), TAG_PORTCLASS);

    if (!portcls_ext->CreateItems)
    {
        /* not enough resources */
        IoDeleteDevice(fdo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    /* store the physical device object */
    portcls_ext->PhysicalDeviceObject = PhysicalDeviceObject;
    /* set up the start device function */
    portcls_ext->StartDevice = StartDevice;
    /* prepare the subdevice list */
    InitializeListHead(&portcls_ext->SubDeviceList);
    /* prepare the physical connection list */
    InitializeListHead(&portcls_ext->PhysicalConnectionList);

    /* set io flags */
    fdo->Flags |= DO_DIRECT_IO | DO_POWER_PAGABLE;
    /* clear initializing flag */
    fdo->Flags &= ~ DO_DEVICE_INITIALIZING;

    /* allocate work item */
    portcls_ext->WorkItem = IoAllocateWorkItem(fdo);

    if (!portcls_ext->WorkItem)
    {
        /* not enough resources */
        FreeItem(portcls_ext->CreateItems, TAG_PORTCLASS);
        /* delete created fdo */
        IoDeleteDevice(fdo);
        /* return error code */
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    /* allocate the device header */
    status = KsAllocateDeviceHeader(&portcls_ext->KsDeviceHeader, MaxObjects, portcls_ext->CreateItems);
    /* did we succeed */
    if (!NT_SUCCESS(status))
    {
        /* free previously allocated create items */
        FreeItem(portcls_ext->CreateItems, TAG_PORTCLASS);
        /* free allocated work item */
        IoFreeWorkItem(portcls_ext->WorkItem);
        /* delete created fdo */
        IoDeleteDevice(fdo);
        /* return error code */
        return status;
    }

    /* attach device to device stack */
    PrevDeviceObject = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
    /* did we succeed */
    if (PrevDeviceObject)
    {
        /* store the device object in the device header */
        //KsSetDevicePnpBaseObject(portcls_ext->KsDeviceHeader, PrevDeviceObject, fdo);
        portcls_ext->PrevDeviceObject = PrevDeviceObject;
    }
    else
    {
        /* free the device header */
        KsFreeDeviceHeader(portcls_ext->KsDeviceHeader);
        /* free previously allocated create items */
        FreeItem(portcls_ext->CreateItems, TAG_PORTCLASS);
        /* free allocated work item */
        IoFreeWorkItem(portcls_ext->WorkItem);
        /* delete created fdo */
        IoDeleteDevice(fdo);
        /* return error code */
        return STATUS_UNSUCCESSFUL;
    }

    return status;
}

NTSTATUS
NTAPI
PcRegisterSubdevice(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PWCHAR Name,
    IN  PUNKNOWN Unknown)
{
    PPCLASS_DEVICE_EXTENSION DeviceExt;
    NTSTATUS Status;
    ISubdevice *SubDevice;
    UNICODE_STRING SymbolicLinkName;
    SUBDEVICE_DESCRIPTOR * SubDeviceDescriptor;
    ULONG Index;

    DPRINT1("PcRegisterSubdevice DeviceObject %p Name %S Unknown %p\n", DeviceObject, Name, Unknown);

    /* check if all parameters are valid */
    if (!DeviceObject || !Name || !Unknown)
    {
        DPRINT("PcRegisterSubdevice invalid parameter\n");
        return STATUS_INVALID_PARAMETER;
    }

    /* get device extension */
    DeviceExt = (PPCLASS_DEVICE_EXTENSION)DeviceObject->DeviceExtension;

    if (!DeviceExt)
    {
        /* should not happen */
        KeBugCheck(0);
        return STATUS_UNSUCCESSFUL;
    }

    /* look up our undocumented interface */
    Status = Unknown->lpVtbl->QueryInterface(Unknown, &IID_ISubdevice, (LPVOID)&SubDevice);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("No ISubdevice interface\n");
        /* the provided port driver doesnt support ISubdevice */
        return STATUS_INVALID_PARAMETER;
    }

    /* get the subdevice descriptor */
    Status = SubDevice->lpVtbl->GetDescriptor(SubDevice, &SubDeviceDescriptor);
    if (!NT_SUCCESS(Status))
    {
        DPRINT1("Failed to get subdevice descriptor %x\n", Status);
        SubDevice->lpVtbl->Release(SubDevice);
        return STATUS_UNSUCCESSFUL;
    }

    /* add an create item to the device header */
    Status = KsAddObjectCreateItemToDeviceHeader(DeviceExt->KsDeviceHeader, PcCreateItemDispatch, (PVOID)SubDevice, Name, NULL);
    if (!NT_SUCCESS(Status))
    {
        /* failed to attach */
        SubDevice->lpVtbl->Release(SubDevice);
        DPRINT1("KsAddObjectCreateItemToDeviceHeader failed with %x\n", Status);
        return Status;
    }

    /* increment reference count */
    SubDevice->lpVtbl->AddRef(SubDevice);

    for(Index = 0; Index < SubDeviceDescriptor->InterfaceCount; Index++)
    {
        //FIXME
        // Use a reference string such as Wave0001 / Topology0001
        //

        Status = IoRegisterDeviceInterface(DeviceExt->PhysicalDeviceObject,
                                           &SubDeviceDescriptor->Interfaces[Index],
                                           NULL,
                                           &SymbolicLinkName);
        if (NT_SUCCESS(Status))
        {
            IoSetDeviceInterfaceState(&SymbolicLinkName, TRUE);
            RtlFreeUnicodeString(&SymbolicLinkName);
        }
    }

    return STATUS_SUCCESS;
}
