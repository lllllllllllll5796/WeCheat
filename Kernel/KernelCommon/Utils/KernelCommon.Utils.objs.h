#pragma once

namespace KernelCommon
{
	namespace Utils
	{
		BOOL ObjectTypesSnapShot(eastl::unordered_map<eastl::wstring, POBJECT_TYPE>& ObjTypes);
		BOOL RPCControlSnapShot(eastl::unordered_map<eastl::wstring, PVOID>& RPCObjs);
		POBJECT_TYPE GetObjectType(const eastl::wstring& Name);
		PVOID GetRPCControl(const eastl::wstring& Name);
	}
}