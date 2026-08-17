#pragma once

void SetHardwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp);
void RemoveHardwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp);
void SetSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp);
void RemoveSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp);
void ReadSoftwareBreakpoint(IN PUSER_DATA userData, IN PIRP pIrp);
