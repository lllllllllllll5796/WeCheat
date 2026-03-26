#include "WeVt.pch.h"
#include "WeVt.Hypervisor.h"
#include "WeVt.Global.h"
//#include "VT/VT.Interface.h"

namespace Hypervisor
{
    //BOOL SuportVT()
    //{
    //    return VtInterface::QuerySupport() ? TRUE : FALSE;
    //}

    //BOOL EnableVT()
    //{
    //    if (!VtInterface::StartHypervisor())
    //    {
    //        LOG_DEBUG("[-] VtInterface::StartHypervisor failed\r\n");
    //        return FALSE;
    //    }

    //    Global::g_SuportVT    = TRUE;
    //    Global::g_HypervisorRunning = TRUE;
    //    LOG_DEBUG("[+] VT Hypervisor started successfully\r\n");
    //    return TRUE;
    //}

    //void DisableVT()
    //{
    //    if (!Global::g_HypervisorRunning)
    //        return;

    //    VtInterface::StopHypervisor();

    //    Global::g_HypervisorRunning = FALSE;
    //    Global::g_SuportVT          = FALSE;
    //    LOG_DEBUG("[+] VT Hypervisor stopped\r\n");
    //}
}
