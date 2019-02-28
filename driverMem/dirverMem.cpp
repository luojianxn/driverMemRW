
#include<ntifs.h>

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING ustrDeviceName;
	UNICODE_STRING ustrSymLinkName;
}DEVICE_EXTENSION, *PDEVICE_EXTENSION;

#define DEVICE_OBJECT_NAME  L"\\Device\\BufferedIODeviceObjectName"
#define DEVICE_LINK_NAME    L"\\DosDevices\\BufferedIODevcieLinkName"
#define CTL_SYS \
	CTL_CODE(FILE_DEVICE_UNKNOWN, 0x830, METHOD_BUFFERED, FILE_ANY_ACCESS)





#ifdef __cplusplus
extern"C"
{
#endif

NTSTATUS DispatchRoutine(IN PDEVICE_OBJECT fdo, IN PIRP Irp)
{
		PAGED_CODE();
		NTSTATUS Status;
		KdPrint(("Enter HelloWDMDispatchRoutine\n"));
		PVOID InputData = NULL;
		ULONG InputDataLength = 0;
		PVOID OutputData = NULL;
		ULONG OutputDataLength = 0;
		ULONG IoControlCode = 0;
		PIO_STACK_LOCATION  IoStackLocation = IoGetCurrentIrpStackLocation(Irp);  //Irp¶ÑÕ» 
		InputData = Irp->AssociatedIrp.SystemBuffer;
		OutputData = Irp->AssociatedIrp.SystemBuffer;	 
		IoControlCode = IoStackLocation->Parameters.DeviceIoControl.IoControlCode;
		InputDataLength = IoStackLocation->Parameters.DeviceIoControl.InputBufferLength;
		OutputDataLength = IoStackLocation->Parameters.DeviceIoControl.OutputBufferLength;
		switch (IoControlCode)
		{
		case CTL_SYS:
		{
						if (InputData != NULL&&InputDataLength > 0)
						{
							DbgPrint("%s\r\n", InputData);
						}
						if (OutputData != NULL&&OutputDataLength >= strlen("Ring0->Ring3") + 1)
						{
							memcpy(OutputData, "Ring0->Ring3", strlen("Ring0->Ring3") + 1);
							Status = STATUS_SUCCESS;
							
						}
						else
						{
							Status = STATUS_INSUFFICIENT_RESOURCES;   //ÄÚ´æ²»¹»
						}
						break;
		}
		default:
			break;
		}


		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		KdPrint(("Leave HelloWDMDispatchRoutine\n"));
		return STATUS_SUCCESS;
}

void driverMemUnload(IN PDRIVER_OBJECT DriverObject)

{
	UNICODE_STRING  DeviceLinkName;
	PDEVICE_OBJECT  v1 = NULL;
	PDEVICE_OBJECT  DeleteDeviceObject = NULL;
	RtlInitUnicodeString(&DeviceLinkName, DEVICE_LINK_NAME);
	IoDeleteSymbolicLink(&DeviceLinkName);
	DeleteDeviceObject = DriverObject->DeviceObject;
	while (DeleteDeviceObject != NULL)
	{
		v1 = DeleteDeviceObject->NextDevice;
		IoDeleteDevice(DeleteDeviceObject);
		DeleteDeviceObject = v1;
	}
}

PVOID MapNewVa(DWORD32 processId, PVOID  userVa, DWORD32 size){

	KAPC_STATE ApcState;
	PEPROCESS Process;
	PMDL mdl=NULL;
	PVOID mapVa=NULL;
	if (!NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)processId, &Process))){
		KdPrint(("PsLookupProcessByProcessId fail"));
		return NULL;
	}
	KeStackAttachProcess(Process, &ApcState);
	__try{
	   
		mdl = IoAllocateMdl(userVa, size, FALSE, FALSE, NULL);
		MmProbeAndLockPages(mdl, UserMode, IoReadAccess);
		MmBuildMdlForNonPagedPool(mdl);	

	}
	__except(EXCEPTION_EXECUTE_HANDLER){
		if (mdl){
			MmUnlockPages(mdl);
			IoFreeMdl(mdl);
		}
		KeUnstackDetachProcess(&ApcState);
		return NULL;	
	}

	KeUnstackDetachProcess(&ApcState);
	__try{
		mapVa=MmMapLockedPagesSpecifyCache(mdl, UserMode, MmNonCached, NULL, false, NormalPagePriority);	
		
	}
	__except (EXCEPTION_EXECUTE_HANDLER){
		MmUnmapLockedPages(mapVa, mdl);	 
	}
	KdPrint(("%c", mapVa));

	return  mapVa;
}

#pragma PAGEDCODE
NTSTATUS AddDevice(IN PDRIVER_OBJECT DriverObject, IN PDEVICE_OBJECT PhysicalDeviceObject)
{
	PAGED_CODE();
	KdPrint(("Enter HelloWDMAddDevice\n"));
	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	UNICODE_STRING devName;
	RtlInitUnicodeString(&devName, DEVICE_OBJECT_NAME);
	status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION),&(UNICODE_STRING)devName,FILE_DEVICE_UNKNOWN,0, FALSE, &fdo);

	if (!NT_SUCCESS(status))
	{
		return status;
	}
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	pdx->fdo = fdo;
	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	UNICODE_STRING symLinkName;
	RtlInitUnicodeString(&symLinkName, DEVICE_LINK_NAME);
	pdx->ustrDeviceName = devName;
	pdx->ustrSymLinkName = symLinkName;
	status = IoCreateSymbolicLink(&(UNICODE_STRING)symLinkName,
		&(UNICODE_STRING)devName);
	if (!NT_SUCCESS(status))
	{
		IoDeleteSymbolicLink(&pdx->ustrSymLinkName);
		status = IoCreateSymbolicLink(&symLinkName, &devName);
		if (!NT_SUCCESS(status))
		{
			return status;
		}
	}
	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;
	KdPrint(("Leaving AddDevice\n"));
	return status;

}


NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,

	IN PUNICODE_STRING pRegistryPath)

{
	
	KdPrint(("Enter DriverEntry\n"));
	pDriverObject->DriverUnload = driverMemUnload;
	pDriverObject->DriverExtension->AddDevice =AddDevice;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchRoutine;
	KdPrint(("Leave DriverEntry\n"));

	return STATUS_SUCCESS;

}
#ifdef __cplusplus

}

#endif